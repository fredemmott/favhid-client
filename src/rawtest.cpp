// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#include "favhid/Arduino.hpp"
#include "favhid/descriptors.hpp"
#include "favhid/protocol.hpp"

#include <Windows.h>

#include <winrt/base.h>

#include <cstdint>
#include <format>
#include <iostream>
#include <chrono>

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

  const auto response = arduino->PushDescriptor(desc.data(), desc.size());
  if (!response.IsOK()) {
    __debugbreak();
  }
}

int main() {
  std::cout << "Opening port..." << std::endl;

  // {7561c7b8-7e1a-419b-98df-d24cd684a92e}
  constexpr OpaqueID MY_ID {
    0x7561c7b8,
    0x7e1a,
    0x419b,
    {0x98, 0xdf, 0xd2, 0x4c, 0xd6, 0x84, 0xa9, 0x2e},
  };

  auto device = Arduino::Open();

  if (!device) {
    std::cout << "Failed to find Arduino." << std::endl;
    return 1;
  }

  const auto activeConfig = device->GetVolatileConfigID();
  if (activeConfig != MY_ID) {
    if (!activeConfig.IsZero()) {
      std::cout << "Rebooting Arduino to get a clean config" << std::endl;

      if (!device->HardReset()) {
        std::cout << "Failed to find Arduino after hard reset" << std::endl;
        return 1;
      }
    }

    std::cout << "Pushing HID descriptor..." << std::endl;
    WriteDescriptor(&*device, REPORT_ID);
    device->SetVolatileConfigID(MY_ID);

    if (!device->ResetUSB()) {
      std::cout << "Failed to find Arduino after descriptor change"
                << std::endl;
      return 1;
    }

    if (device->GetVolatileConfigID() != MY_ID) {
      std::cout << "Arduino has different ID after USB reset" << std::endl;
      return 1;
    }
  }

  std::cout << "Feeding..." << std::endl;

  Report report {.buttons = 1};
  while (true) {
    Sleep(250);
    report.buttons <<= 1;
    if (!report.buttons) {
      report.buttons = 1;
    }
    const auto start = std::chrono::steady_clock::now();
    const auto result = device->WriteReport(REPORT_ID, &report, sizeof(report));
    const auto end = std::chrono::steady_clock::now();
    if (!result.IsOK()) {
      __debugbreak();
    }

    std::cout << "Submitting report took " << (end - start) << std::endl;
  }

  return 0;
}