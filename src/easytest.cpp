// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#include <Windows.h>

#include <winrt/base.h>

#include <iostream>
#include <format>
#include <wbemidl.h>

#include <cstdint>

#include "favhid/protocol.hpp"
#include "favhid/io.hpp"

constexpr uint8_t REPORT_ID = 0x03;

constexpr uint8_t HID_DESCRIPTOR[] = {
// clang-format off
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,                    // USAGE (Game Pad)
    0xa1, 0x01,                    // COLLECTION (Application)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    // ReportID - 8 bits
    0x85, REPORT_ID,                    //     REPORT_ID (REPORT_ID)
    // X & Y - 2x8 = 16 bits
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x02,                    //     REPORT_COUNT (2)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    // Buttons - 8 bits
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x08,                    //     USAGE_MAXIMUM (Button 8)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x95, 0x08,                    //     REPORT_COUNT (8)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0xc0,                          //     END_COLLECTION
    0xc0                           // END_COLLECTION
// clang-format on
};

#pragma pack(push, 1)
struct Report {
    uint8_t x {};
    uint8_t y {};
    uint8_t buttons {};
};
#pragma pack(pop)


int main() {
    std::cout << "Opening port..." << std::endl;
    auto f = OpenArduino();

    if (!f) {
        std::cout << "Failed to find arduino." << std::endl;
        return 1;
    }

    {
        constexpr Report report {
            .x = 128,
            .y = 128,
            .buttons = 0xff,
        };

        auto response = WriteDescriptor(f, REPORT_ID, HID_DESCRIPTOR, sizeof(HID_DESCRIPTOR), &report, sizeof(report));
        if (!response.IsOK()) {
            std::cout << "Failed to put device" << std::endl;
        }
    }

    f.close();
    std::cout << "Waiting to re-open" << std::endl;
    Sleep(500);
    f = OpenArduino();

    if (!f) {
        return 1;
    }

    Report report { .buttons = 1 };
    while(true) {
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