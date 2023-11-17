// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#include "favhid/descriptors.hpp"
#include "favhid/Arduino.hpp"
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

static void WriteDescriptor(Arduino* arduino, uint8_t reportID) {
  using namespace FAVHID::Descriptors;

  Descriptor desc {
    UsagePage::GenericDesktop(),
    Usage::Gamepad(),
    Collection::Application {
      Collection::Physical {
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
        Input::DataVariableAbsolute(),
      },
    },
  };

  arduino->PushDescriptor(desc.data(), desc.size());
}

int main() {
  std::cout << "Opening port..." << std::endl;

  {
    auto maybeArduino = Arduino::Open();

    if (!maybeArduino) {
      std::cout << "Failed to find Arduino." << std::endl;
      return 1;
    }

    WriteDescriptor(&*maybeArduino, REPORT_ID);
  }

  std::cout << "Waiting to re-open" << std::endl;
  Sleep(500);
  auto a = Arduino::Open();
  if (!a) {
      std::cout << "Failed to re-open Arduino." << std::endl;
      return 1;
  }
  std::cout << "Re-opened, feeding" << std::endl;


  Report report {.buttons = 1};
  while (true) {
    Sleep(250);
    report.buttons <<= 1;
    if (!report.buttons) {
      report.buttons = 1;
    }
    const auto result = a->WriteReport(REPORT_ID, &report, sizeof(report));
    if (!result.IsOK()) {
      __debugbreak();
    }
  }

  return 0;
}