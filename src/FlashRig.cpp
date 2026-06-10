#include "flash_rig/FlashRig.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string_view>
#include <utility>

namespace flash_rig {
namespace {

bool isValidSha256(std::string_view digest) {
  return digest.size() == 64U &&
         std::all_of(digest.begin(), digest.end(), [](const char value) {
           return std::isxdigit(static_cast<unsigned char>(value)) != 0;
         });
}

std::string yesNo(const bool value) {
  return value ? "yes" : "no";
}

std::string fixed1(const double value) {
  std::ostringstream stream;
  stream << std::fixed << std::setprecision(1) << value;
  return stream.str();
}

std::string fixed2(const double value) {
  std::ostringstream stream;
  stream << std::fixed << std::setprecision(2) << value;
  return stream.str();
}

void addEvidence(BoardResult& result,
                 std::string step,
                 const bool passed,
                 std::string detail) {
  result.evidence.push_back(
      EvidenceEvent{std::move(step),
                    passed ? EvidenceStatus::Pass : EvidenceStatus::Fail,
                    std::move(detail)});
}

bool preflightBoard(BoardResult& result,
                    const BoardSlot& board,
                    const AcceptanceLimits& limits) {
  if (board.fixtureId.empty() || board.slotId.empty() ||
      board.serialNumber.empty()) {
    addEvidence(result,
                "fixture-preflight",
                false,
                "fixture id, slot id, and board serial are required");
    return false;
  }

  if (board.supplyVoltage < limits.minSupplyVoltage ||
      board.supplyVoltage > limits.maxSupplyVoltage) {
    addEvidence(result,
                "fixture-preflight",
                false,
                "supply voltage " + fixed2(board.supplyVoltage) +
                    " V is outside " + fixed2(limits.minSupplyVoltage) +
                    "-" + fixed2(limits.maxSupplyVoltage) + " V");
    return false;
  }

  if (board.idleCurrentMa > limits.maxIdleCurrentMa) {
    addEvidence(result,
                "fixture-preflight",
                false,
                "idle current " + fixed1(board.idleCurrentMa) +
                    " mA exceeds " + fixed1(limits.maxIdleCurrentMa) + " mA");
    return false;
  }

  addEvidence(result,
              "fixture-preflight",
              true,
              board.fixtureId + "/" + board.slotId + " stable at " +
                  fixed2(board.supplyVoltage) + " V and " +
                  fixed1(board.idleCurrentMa) + " mA idle");
  return true;
}

bool validateManifest(BoardResult& result,
                      const BoardSlot& board,
                      const FirmwareImage& image) {
  if (image.version.empty()) {
    addEvidence(result,
                "firmware-manifest",
                false,
                "firmware version must be populated");
    return false;
  }

  if (image.targetHardwareRevision != board.hardwareRevision) {
    addEvidence(result,
                "firmware-manifest",
                false,
                "image targets hardware " + image.targetHardwareRevision +
                    " but board is " + board.hardwareRevision);
    return false;
  }

  if (!isValidSha256(image.sha256)) {
    addEvidence(result,
                "firmware-manifest",
                false,
                "firmware sha256 must be a 64-character hex digest");
    return false;
  }

  if (image.sizeBytes == 0U) {
    addEvidence(result,
                "firmware-manifest",
                false,
                "firmware image size must be non-zero");
    return false;
  }

  addEvidence(result,
              "firmware-manifest",
              true,
              "version " + image.version + " for hardware " +
                  image.targetHardwareRevision + " digest " +
                  image.sha256.substr(0U, 12U));
  return true;
}

bool bootPassed(const BoardSlot& board,
                const FirmwareImage& image,
                const AcceptanceLimits& limits,
                const BootObservation& boot,
                std::string& reason) {
  if (!boot.responded) {
    reason = "boot probe did not receive a response";
    return false;
  }

  if (boot.firmwareVersion != image.version) {
    reason = "booted firmware " + boot.firmwareVersion + " but expected " +
             image.version;
    return false;
  }

  if (boot.bootTimeMs > limits.maxBootTimeMs) {
    reason = "boot time " + std::to_string(boot.bootTimeMs) +
             " ms exceeds " + std::to_string(limits.maxBootTimeMs) + " ms";
    return false;
  }

  if (!board.expectedSocId.empty() && boot.socId != board.expectedSocId) {
    reason = "observed SoC id " + boot.socId + " but expected " +
             board.expectedSocId;
    return false;
  }

  reason = "firmware " + boot.firmwareVersion + " responded in " +
           std::to_string(boot.bootTimeMs) + " ms";
  return true;
}

bool soakPassed(const AcceptanceLimits& limits,
                const SoakObservation& soak,
                std::string& reason) {
  if (soak.cycles < limits.minSoakCycles) {
    reason = "soak completed " + std::to_string(soak.cycles) +
             " cycles, below required " +
             std::to_string(limits.minSoakCycles);
    return false;
  }

  if (soak.failures != 0U) {
    reason = "soak reported " + std::to_string(soak.failures) + " failures";
    return false;
  }

  if (soak.maxBoardTempC > limits.maxBoardTempC) {
    reason = "board temperature " + fixed1(soak.maxBoardTempC) +
             " C exceeds " + fixed1(limits.maxBoardTempC) + " C";
    return false;
  }

  if (soak.maxCurrentMa > limits.maxRunCurrentMa) {
    reason = "run current " + fixed1(soak.maxCurrentMa) + " mA exceeds " +
             fixed1(limits.maxRunCurrentMa) + " mA";
    return false;
  }

  reason = std::to_string(soak.cycles) + " cycles, max " +
           fixed1(soak.maxBoardTempC) + " C, " +
           fixed1(soak.maxCurrentMa) + " mA";
  return true;
}

}  // namespace

std::string toString(const EvidenceStatus status) {
  return status == EvidenceStatus::Pass ? "PASS" : "FAIL";
}

ProductionTestRig::ProductionTestRig(IProgrammer& programmer,
                                     IBootProbe& bootProbe,
                                     ISoakRunner& soakRunner,
                                     IReportSink& reportSink)
    : programmer_(programmer),
      bootProbe_(bootProbe),
      soakRunner_(soakRunner),
      reportSink_(reportSink) {}

BoardResult ProductionTestRig::runBoard(const BoardSlot& board,
                                        const FirmwareImage& image,
                                        const AcceptanceLimits& limits) {
  BoardResult result;
  result.serialNumber = board.serialNumber;

  auto fail = [&](std::string reason) {
    result.accepted = false;
    result.failureReason = std::move(reason);
    reportSink_.publish(result);
    return result;
  };

  if (!preflightBoard(result, board, limits)) {
    return fail(result.evidence.back().detail);
  }

  if (!validateManifest(result, board, image)) {
    return fail(result.evidence.back().detail);
  }

  result.flash = programmer_.flashAndVerify(board, image);
  const bool flashed =
      result.flash.erased && result.flash.programmed && result.flash.verified;
  addEvidence(result, "erase-program-verify", flashed, result.flash.detail);
  if (!flashed) {
    return fail(result.flash.detail);
  }

  result.boot = bootProbe_.probeBoot(board, image);
  std::string bootReason;
  const bool bootOk = bootPassed(board, image, limits, result.boot, bootReason);
  addEvidence(result, "boot-health", bootOk, bootReason);
  if (!bootOk) {
    return fail(bootReason);
  }

  result.soak = soakRunner_.runSoak(board, image, limits);
  std::string soakReason;
  const bool soakOk = soakPassed(limits, result.soak, soakReason);
  addEvidence(result, "power-thermal-soak", soakOk, soakReason);
  if (!soakOk) {
    return fail(soakReason);
  }

  result.accepted = true;
  result.failureReason = "accepted";
  addEvidence(result,
              "release-decision",
              true,
              "board released for packaging with complete evidence");
  reportSink_.publish(result);
  return result;
}

SimulatedProgrammer::SimulatedProgrammer(SimulatedProgrammerConfig config)
    : config_(std::move(config)) {}

FlashAttempt SimulatedProgrammer::flashAndVerify(const BoardSlot& board,
                                                 const FirmwareImage& image) {
  if (!config_.programmerOnline) {
    return FlashAttempt{
        false, false, false, config_.programmerSerial, "programmer offline"};
  }

  if (board.hardwareRevision != image.targetHardwareRevision) {
    return FlashAttempt{true,
                        false,
                        false,
                        config_.programmerSerial,
                        "hardware revision guard blocked programming"};
  }

  if (!isValidSha256(image.sha256) || image.sizeBytes == 0U) {
    return FlashAttempt{
        true, false, false, config_.programmerSerial, "invalid image manifest"};
  }

  if (config_.forceVerifyFailure) {
    return FlashAttempt{true,
                        true,
                        false,
                        config_.programmerSerial,
                        "readback digest did not match manifest"};
  }

  return FlashAttempt{true,
                      true,
                      true,
                      config_.programmerSerial,
                      "erased, programmed " + std::to_string(image.sizeBytes) +
                          " bytes, verified " + image.sha256.substr(0U, 12U)};
}

SimulatedBootProbe::SimulatedBootProbe(SimulatedBootConfig config)
    : config_(std::move(config)) {}

BootObservation SimulatedBootProbe::probeBoot(const BoardSlot& board,
                                              const FirmwareImage& image) {
  if (!config_.bootResponds) {
    return BootObservation{false, "", 0U, "", "boot UART did not respond"};
  }

  const std::string firmwareVersion =
      config_.observedFirmwareVersion.empty() ? image.version
                                              : config_.observedFirmwareVersion;
  const std::string socId = config_.observedSocId.empty() ? board.expectedSocId
                                                          : config_.observedSocId;

  return BootObservation{true,
                         firmwareVersion,
                         config_.bootTimeMs,
                         socId,
                         "captured boot banner and health endpoint"};
}

SimulatedSoakRunner::SimulatedSoakRunner(SimulatedSoakConfig config)
    : config_(std::move(config)) {}

SoakObservation SimulatedSoakRunner::runSoak(const BoardSlot&,
                                             const FirmwareImage&,
                                             const AcceptanceLimits&) {
  return SoakObservation{config_.cycles,
                         config_.failures,
                         config_.maxBoardTempC,
                         config_.maxCurrentMa,
                         "ran update reboot loop with power telemetry"};
}

TextReportSink::TextReportSink(std::ostream& stream) : stream_(stream) {}

void TextReportSink::publish(const BoardResult& result) {
  stream_ << "serial=" << result.serialNumber << " result="
          << (result.accepted ? "PASS" : "FAIL") << " reason=\""
          << result.failureReason << "\"\n";

  for (const auto& event : result.evidence) {
    stream_ << "  [" << toString(event.status) << "] " << event.step << ": "
            << event.detail << '\n';
  }

  if (!result.flash.programmerSerial.empty()) {
    stream_ << "  flash: programmer=" << result.flash.programmerSerial
            << " erased=" << yesNo(result.flash.erased)
            << " programmed=" << yesNo(result.flash.programmed)
            << " verified=" << yesNo(result.flash.verified) << '\n';
  }

  if (result.boot.responded) {
    stream_ << "  boot: version=" << result.boot.firmwareVersion
            << " soc=" << result.boot.socId
            << " time_ms=" << result.boot.bootTimeMs << '\n';
  }

  if (result.soak.cycles != 0U) {
    stream_ << "  soak: cycles=" << result.soak.cycles
            << " failures=" << result.soak.failures
            << " max_temp_c=" << fixed1(result.soak.maxBoardTempC)
            << " max_current_ma=" << fixed1(result.soak.maxCurrentMa)
            << '\n';
  }
}

}  // namespace flash_rig
