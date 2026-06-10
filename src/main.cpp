#include "flash_rig/FlashRig.hpp"

#include <iostream>

namespace {

flash_rig::FirmwareImage releaseImage() {
  return flash_rig::FirmwareImage{
      "2026.06.10-production",
      "REV-C",
      "3b7e1f0c9d4a5b682817263544e5f60718c9aabbccddeeff0011223344556677",
      4U * 1024U * 1024U};
}

flash_rig::BoardSlot passingBoard() {
  return flash_rig::BoardSlot{
      "FIXTURE-A",
      "SLOT-01",
      "PFT-000427",
      "REV-C",
      5.03,
      38.5,
      "AM3358-2BGA"};
}

flash_rig::BoardSlot quarantineBoard() {
  return flash_rig::BoardSlot{
      "FIXTURE-A",
      "SLOT-02",
      "PFT-000428",
      "REV-B",
      5.01,
      41.0,
      "AM3358-2BGA"};
}

}  // namespace

int main() {
  flash_rig::SimulatedProgrammer programmer;
  flash_rig::SimulatedBootProbe bootProbe;
  flash_rig::SimulatedSoakRunner soakRunner;
  flash_rig::TextReportSink reportSink(std::cout);
  flash_rig::ProductionTestRig rig(
      programmer, bootProbe, soakRunner, reportSink);

  std::cout << "Production Flash and Test Rig\n";
  std::cout << "Release image: " << releaseImage().version << "\n\n";

  const auto accepted = rig.runBoard(passingBoard(), releaseImage());
  std::cout << '\n';
  const auto rejected = rig.runBoard(quarantineBoard(), releaseImage());

  std::cout << "\nstation_summary accepted=" << (accepted.accepted ? 1 : 0)
            << " quarantined=" << (!rejected.accepted ? 1 : 0) << '\n';

  return accepted.accepted && !rejected.accepted ? 0 : 1;
}
