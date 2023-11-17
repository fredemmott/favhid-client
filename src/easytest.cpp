// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#include "favhid/descriptors.hpp"
#include "favhid/io.hpp"
#include "favhid/protocol.hpp"

#include <Windows.h>

#include <winrt/base.h>

#include <cstdint>
#include <format>
#include <iostream>

#include <wbemidl.h>

using namespace FAVHID;

constexpr uint8_t REPORT_ID = 0x03;

#pragma pack(push, 1)
struct Report {
  uint8_t x {};
  uint8_t y {};
  uint8_t buttons {};
};
#pragma pack(pop)

static void WriteDescriptor(const THandle& handle, uint8_t reportID) {
  using namespace FAVHID::Descriptors;

  Descriptor desc {
    UsagePage::GenericDesktop(),
    Usage::GamePad(),
    Collection::Application(Collection::Physical(
      ReportID(reportID),
      UsagePage::GenericDesktop(),
      Usage::X(),
      Usage::Y(),
      LogicalMinimum::ForType<int8_t>(),
      LogicalMaximum::ForType<int8_t>(),
      ReportSize(8),
      ReportCount(2),
      Input::DataVariableAbsolute(),
      UsagePage::Button(),
      UsageMinimum(1),
      UsageMaximum(8),
      LogicalMinimum(0),
      LogicalMaximum(1),
      ReportSize(1),
      ReportCount(8),
      Input::DataVariableAbsolute()))};

  Report report {};
  WriteDescriptor(
    handle, reportID, desc.data(), desc.size(), &report, sizeof(report));
}

int main() {
  std::cout << "Opening port..." << std::endl;
  auto f = OpenArduino();

  if (!f) {
    std::cout << "Failed to find arduino." << std::endl;
    return 1;
  }

  WriteDescriptor(f, REPORT_ID);

  f.close();
  std::cout << "Waiting to re-open" << std::endl;
  Sleep(500);
  f = OpenArduino();
  std::cout << "Re-opened, feeding" << std::endl;

  if (!f) {
    return 1;
  }

  Report report {.buttons = 1};
  while (true) {
    Sleep(250);
    report.buttons <<= 1;
    if (!report.buttons) {
      report.buttons = 1;
    }
    const auto result = WriteReport(f, REPORT_ID, &report, sizeof(report));
    if (!result.IsOK()) {
      __debugbreak();
    }
  }

  return 0;
}