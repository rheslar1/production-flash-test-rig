#include "flash_rig/FlashRig.hpp"

#include <cassert>
#include <sstream>
#include <string>

namespace {

flash_rig::FirmwareImage image() {
  return flash_rig::FirmwareImage{
      "2026.06.10-production",
      "REV-C",
      "3b7e1f0c9d4a5b682817263544e5f60718c9aabbccddeeff0011223344556677",
      4U * 1024U * 1024U};
}

flash_rig::BoardSlot board() {
  return flash_rig::BoardSlot{
      "FIXTURE-A",
      "SLOT-01",
      "PFT-000427",
      "REV-C",
      5.02,
      39.0,
      "AM3358-2BGA"};
}

bool contains(const std::string& value, const std::string& needle) {
  return value.find(needle) != std::string::npos;
}

void acceptsNominalBoard() {
  std::ostringstream report;
  flash_rig::SimulatedProgrammer programmer;
  flash_rig::SimulatedBootProbe bootProbe;
  flash_rig::SimulatedSoakRunner soakRunner;
  flash_rig::TextReportSink sink(report);
  flash_rig::ProductionTestRig rig(
      programmer, bootProbe, soakRunner, sink);

  const auto result = rig.runBoard(board(), image());

  assert(result.accepted);
  assert(result.failureReason == "accepted");
  assert(result.flash.verified);
  assert(result.boot.firmwareVersion == image().version);
  assert(result.soak.failures == 0U);
  assert(result.evidence.size() == 6U);
  assert(contains(report.str(), "result=PASS"));
  assert(contains(report.str(), "release-decision"));
}

void rejectsWrongHardwareRevisionBeforeProgramming() {
  std::ostringstream report;
  flash_rig::SimulatedProgrammer programmer;
  flash_rig::SimulatedBootProbe bootProbe;
  flash_rig::SimulatedSoakRunner soakRunner;
  flash_rig::TextReportSink sink(report);
  flash_rig::ProductionTestRig rig(
      programmer, bootProbe, soakRunner, sink);
  auto wrongBoard = board();
  wrongBoard.hardwareRevision = "REV-B";

  const auto result = rig.runBoard(wrongBoard, image());

  assert(!result.accepted);
  assert(!result.flash.programmed);
  assert(contains(result.failureReason, "image targets hardware"));
  assert(contains(report.str(), "result=FAIL"));
}

void detectsBootVersionMismatch() {
  std::ostringstream report;
  flash_rig::SimulatedProgrammer programmer;
  flash_rig::SimulatedBootProbe bootProbe(
      flash_rig::SimulatedBootConfig{true, "2025.12.rollback", "", 900});
  flash_rig::SimulatedSoakRunner soakRunner;
  flash_rig::TextReportSink sink(report);
  flash_rig::ProductionTestRig rig(
      programmer, bootProbe, soakRunner, sink);

  const auto result = rig.runBoard(board(), image());

  assert(!result.accepted);
  assert(result.flash.verified);
  assert(contains(result.failureReason, "booted firmware"));
  assert(contains(report.str(), "boot-health"));
}

void detectsSoakCurrentLimit() {
  std::ostringstream report;
  flash_rig::SimulatedProgrammer programmer;
  flash_rig::SimulatedBootProbe bootProbe;
  flash_rig::SimulatedSoakRunner soakRunner(
      flash_rig::SimulatedSoakConfig{12, 0, 52.0, 341.0});
  flash_rig::TextReportSink sink(report);
  flash_rig::ProductionTestRig rig(
      programmer, bootProbe, soakRunner, sink);

  const auto result = rig.runBoard(board(), image());

  assert(!result.accepted);
  assert(result.soak.maxCurrentMa > flash_rig::AcceptanceLimits{}.maxRunCurrentMa);
  assert(contains(result.failureReason, "run current"));
  assert(contains(report.str(), "power-thermal-soak"));
}

void rejectsInvalidDigest() {
  std::ostringstream report;
  flash_rig::SimulatedProgrammer programmer;
  flash_rig::SimulatedBootProbe bootProbe;
  flash_rig::SimulatedSoakRunner soakRunner;
  flash_rig::TextReportSink sink(report);
  flash_rig::ProductionTestRig rig(
      programmer, bootProbe, soakRunner, sink);
  auto badImage = image();
  badImage.sha256 = "not-a-real-digest";

  const auto result = rig.runBoard(board(), badImage);

  assert(!result.accepted);
  assert(!result.flash.erased);
  assert(contains(result.failureReason, "sha256"));
  assert(contains(report.str(), "firmware-manifest"));
}

}  // namespace

int main() {
  acceptsNominalBoard();
  rejectsWrongHardwareRevisionBeforeProgramming();
  detectsBootVersionMismatch();
  detectsSoakCurrentLimit();
  rejectsInvalidDigest();
  return 0;
}
