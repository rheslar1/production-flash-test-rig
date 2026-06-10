#ifndef FLASH_RIG_FLASH_RIG_HPP_
#define FLASH_RIG_FLASH_RIG_HPP_

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

namespace flash_rig {

enum class EvidenceStatus {
  Pass,
  Fail
};

std::string toString(EvidenceStatus status);

struct FirmwareImage {
  std::string version;
  std::string targetHardwareRevision;
  std::string sha256;
  std::size_t sizeBytes{};
};

struct BoardSlot {
  std::string fixtureId;
  std::string slotId;
  std::string serialNumber;
  std::string hardwareRevision;
  double supplyVoltage{};
  double idleCurrentMa{};
  std::string expectedSocId;
};

struct AcceptanceLimits {
  double minSupplyVoltage{4.75};
  double maxSupplyVoltage{5.25};
  double maxIdleCurrentMa{120.0};
  double maxRunCurrentMa{300.0};
  double maxBoardTempC{70.0};
  std::uint32_t maxBootTimeMs{2500};
  std::uint32_t minSoakCycles{8};
};

struct FlashAttempt {
  bool erased{};
  bool programmed{};
  bool verified{};
  std::string programmerSerial;
  std::string detail;
};

struct BootObservation {
  bool responded{};
  std::string firmwareVersion;
  std::uint32_t bootTimeMs{};
  std::string socId;
  std::string detail;
};

struct SoakObservation {
  std::uint32_t cycles{};
  std::uint32_t failures{};
  double maxBoardTempC{};
  double maxCurrentMa{};
  std::string detail;
};

struct EvidenceEvent {
  std::string step;
  EvidenceStatus status{EvidenceStatus::Fail};
  std::string detail;
};

struct BoardResult {
  std::string serialNumber;
  bool accepted{};
  std::string failureReason;
  FlashAttempt flash;
  BootObservation boot;
  SoakObservation soak;
  std::vector<EvidenceEvent> evidence;
};

class IProgrammer {
 public:
  virtual ~IProgrammer() = default;
  virtual FlashAttempt flashAndVerify(const BoardSlot& board,
                                      const FirmwareImage& image) = 0;
};

class IBootProbe {
 public:
  virtual ~IBootProbe() = default;
  virtual BootObservation probeBoot(const BoardSlot& board,
                                    const FirmwareImage& image) = 0;
};

class ISoakRunner {
 public:
  virtual ~ISoakRunner() = default;
  virtual SoakObservation runSoak(const BoardSlot& board,
                                  const FirmwareImage& image,
                                  const AcceptanceLimits& limits) = 0;
};

class IReportSink {
 public:
  virtual ~IReportSink() = default;
  virtual void publish(const BoardResult& result) = 0;
};

class ProductionTestRig {
 public:
  ProductionTestRig(IProgrammer& programmer,
                    IBootProbe& bootProbe,
                    ISoakRunner& soakRunner,
                    IReportSink& reportSink);

  BoardResult runBoard(const BoardSlot& board,
                       const FirmwareImage& image,
                       const AcceptanceLimits& limits = AcceptanceLimits{});

 private:
  IProgrammer& programmer_;
  IBootProbe& bootProbe_;
  ISoakRunner& soakRunner_;
  IReportSink& reportSink_;
};

struct SimulatedProgrammerConfig {
  std::string programmerSerial{"FACTORY-PROG-01"};
  bool programmerOnline{true};
  bool forceVerifyFailure{false};
};

class SimulatedProgrammer final : public IProgrammer {
 public:
  explicit SimulatedProgrammer(SimulatedProgrammerConfig config = {});

  FlashAttempt flashAndVerify(const BoardSlot& board,
                              const FirmwareImage& image) override;

 private:
  SimulatedProgrammerConfig config_;
};

struct SimulatedBootConfig {
  bool bootResponds{true};
  std::string observedFirmwareVersion;
  std::string observedSocId;
  std::uint32_t bootTimeMs{1180};
};

class SimulatedBootProbe final : public IBootProbe {
 public:
  explicit SimulatedBootProbe(SimulatedBootConfig config = {});

  BootObservation probeBoot(const BoardSlot& board,
                            const FirmwareImage& image) override;

 private:
  SimulatedBootConfig config_;
};

struct SimulatedSoakConfig {
  std::uint32_t cycles{12};
  std::uint32_t failures{0};
  double maxBoardTempC{48.5};
  double maxCurrentMa{181.0};
};

class SimulatedSoakRunner final : public ISoakRunner {
 public:
  explicit SimulatedSoakRunner(SimulatedSoakConfig config = {});

  SoakObservation runSoak(const BoardSlot& board,
                          const FirmwareImage& image,
                          const AcceptanceLimits& limits) override;

 private:
  SimulatedSoakConfig config_;
};

class TextReportSink final : public IReportSink {
 public:
  explicit TextReportSink(std::ostream& stream);

  void publish(const BoardResult& result) override;

 private:
  std::ostream& stream_;
};

}  // namespace flash_rig

#endif  // FLASH_RIG_FLASH_RIG_HPP_
