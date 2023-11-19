// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#include "favhid/FAVJoyState2.hpp"

#include "favhid/descriptors.hpp"

#include <stdexcept>

namespace FAVHID {

namespace {

template <uint8_t reportID>
constexpr auto MakeDescriptor() {
  using namespace FAVHID::Descriptors;
  using namespace FAVHID::Descriptors::IntegerSuffixes;

  return Descriptor {
    UsagePage::GenericDesktop,
    Usage::Joystick,
    Collection::Application {
      Collection::Physical {
        ReportID {reportID},
        UsagePage::GenericDesktop,
        Usage::X,
        Usage::Y,
        Usage::Z,
        Usage::Rx,
        Usage::Ry,
        Usage::Rz,
        Usage::Slider,
        Usage::Slider,
        ReportSize {16},
        ReportCount {8},
        LogicalMinimum<int16_t> {},
        LogicalMaximum<int16_t> {},
        Input::DataVariableAbsolute,
        Usage::HatSwitch,
        Usage::HatSwitch,
        Usage::HatSwitch,
        Usage::HatSwitch,
        LogicalMinimum {0_u8},
        LogicalMaximum {7_u8},
        ReportSize {4_u8},
        ReportCount {4_u8},
        Input::DataVariableAbsoluteNullState,
        UsagePage::Button,
        UsageMinimum {1_u8},
        UsageMaximum {128_u8},
        LogicalMinimum {0_u8},
        LogicalMaximum {1_u8},
        ReportSize {1_u8},
        ReportCount {128_u8},
        Input::DataVariableAbsolute,
        // int16_t vx, vy, vz;
        // int16_t rvx, rvy, rvz;
        // int16_t vslider[2];
        // int16_t ax, ay, az;
        // int16_t arx, ary, arz;
        // int16_t aslider[2];
        // int16_t fx, fy, fz;
        // int16_t frx, fry, frz;
        // int16_t fslider[2];
      },
    },
  };
}

constexpr uint8_t REPORT_IDS[FAVJoyState2::MAX_DEVICES] {
  FIRST_AVAILABLE_REPORT_ID,
  FIRST_AVAILABLE_REPORT_ID + 1,
  FIRST_AVAILABLE_REPORT_ID + 2,
  FIRST_AVAILABLE_REPORT_ID + 3,
  FIRST_AVAILABLE_REPORT_ID + 4,
  FIRST_AVAILABLE_REPORT_ID + 5,
  FIRST_AVAILABLE_REPORT_ID + 6,
  FIRST_AVAILABLE_REPORT_ID + 7,
};

using Descriptor = decltype(MakeDescriptor<REPORT_IDS[0]>());
constexpr Descriptor DESCRIPTORS[FAVJoyState2::MAX_DEVICES] {
  MakeDescriptor<REPORT_IDS[0]>(),
  MakeDescriptor<REPORT_IDS[1]>(),
  MakeDescriptor<REPORT_IDS[2]>(),
  MakeDescriptor<REPORT_IDS[3]>(),
  MakeDescriptor<REPORT_IDS[4]>(),
  MakeDescriptor<REPORT_IDS[5]>(),
  MakeDescriptor<REPORT_IDS[6]>(),
  MakeDescriptor<REPORT_IDS[7]>(),
};

// Varies depending on how many devices we're attaching
constexpr OpaqueID CONFIG_IDS[FAVJoyState2::MAX_DEVICES] {
  // {8310f304-ac31-4747-81d8-2d035c7ee5a4}
  OpaqueID {
    0x8310f304,
    0xac31,
    0x4747,
    {0x81, 0xd8, 0x2d, 0x03, 0x5c, 0x7e, 0xe5, 0xa4},
  },
  // {8ebeb019-6e27-42d0-808d-b4b08d78ac2a}
  OpaqueID {
    0x8ebeb019,
    0x6e27,
    0x42d0,
    {0x80, 0x8d, 0xb4, 0xb0, 0x8d, 0x78, 0xac, 0x2a},
  },
  // {a46ce528-412e-4a7b-8bda-279e7a48bd56}
  OpaqueID {
    0xa46ce528,
    0x412e,
    0x4a7b,
    {0x8b, 0xda, 0x27, 0x9e, 0x7a, 0x48, 0xbd, 0x56},
  },
  // {528704d6-1799-41a2-b839-84d999dadb38}
  OpaqueID {
    0x528704d6,
    0x1799,
    0x41a2,
    {0xb8, 0x39, 0x84, 0xd9, 0x99, 0xda, 0xdb, 0x38},
  },// {84458de2-bc0a-43b0-9346-57c46dd9f11f}
  OpaqueID {
    0x84458de2,
    0xbc0a,
    0x43b0,
    {0x93, 0x46, 0x57, 0xc4, 0x6d, 0xd9, 0xf1, 0x1f},
  },
  // {342fa25d-3626-4ee2-998e-ded7801959f4}
  OpaqueID {
    0x342fa25d,
    0x3626,
    0x4ee2,
    {0x99, 0x8e, 0xde, 0xd7, 0x80, 0x19, 0x59, 0xf4},
  },
  // {635c14ff-cdf9-4fa8-9f83-43d30c429ba0}
  OpaqueID {
    0x635c14ff,
    0xcdf9,
    0x4fa8,
    {0x9f, 0x83, 0x43, 0xd3, 0x0c, 0x42, 0x9b, 0xa0},
  },
  // {827a4ec3-b892-4b74-9b04-a278f5bdd6d3}
  OpaqueID {
    0x827a4ec3,
    0xb892,
    0x4b74,
    {0x9b, 0x04, 0xa2, 0x78, 0xf5, 0xbd, 0xd6, 0xd3},
  },
};

}// namespace

FAVJoyState2::FAVJoyState2(uint8_t deviceCount, Arduino&& a)
  : mDevice(std::move(a)),
    mCount(deviceCount),
    mConfigID(CONFIG_IDS[deviceCount - 1]) {
  const auto oldID = mDevice.GetVolatileConfigID();
  if (oldID == mConfigID) {
    return;
  }

  if (!oldID.IsZero()) {
    if (!mDevice.HardReset()) {
      throw std::runtime_error("Arduino did not come back after hard reset");
    }
  }

  for (int i = 0; i < deviceCount; ++i) {
    mDevice.PushDescriptor(&DESCRIPTORS[i], sizeof(Descriptor));
  }
  mDevice.SetVolatileConfigID(mConfigID);

  if (!mDevice.ResetUSB()) {
    throw std::runtime_error("Arduino did not come back after USB reset");
  }

  if (mDevice.GetVolatileConfigID() != mConfigID) {
    throw std::runtime_error(
      "Arduino came back with a different volatile config ID");
  }
}

void FAVJoyState2::WriteReport(const DIJOYSTATE2& di, uint8_t deviceIndex) {
  if (deviceIndex >= mCount) {
    throw std::logic_error("Device index is >= device count");
  }

  Report report {
    .x = static_cast<int16_t>(di.lX),
    .y = static_cast<int16_t>(di.lY),
    .z = static_cast<int16_t>(di.lZ),
    .rx = static_cast<int16_t>(di.lRx),
    .ry = static_cast<int16_t>(di.lRy),
    .rz = static_cast<int16_t>(di.lRz),
    .slider = {
      static_cast<int16_t>(di.rglSlider[0]),
      static_cast<int16_t>(di.rglSlider[1]),
    },
  };

  // Convert POVs from centidegrees
  for (off_t i = 0; i < 4; ++i) {
    const auto diValue = di.rgdwPOV[i];
    const auto centered = (LOWORD(diValue) == 0xFFFF);
    const uint8_t value = centered ? 0b1111 : diValue / 4500;
    const auto bitOffset = 4 * (i % 2);
    const auto byteOffset = ((4 * i) - bitOffset) / 8;

    auto& byte = reinterpret_cast<uint8_t*>(&report.povs)[byteOffset];
    byte |= (value << bitOffset);
  }

  // Convert buttons from byte with high-bit to just a bit mask
  constexpr uint8_t BUTTON_ON_BIT = (1 << 7);
  for (off_t i = 0; i < 128; ++i) {
    if (!(di.rgbButtons[i] & BUTTON_ON_BIT)) {
      continue;
    }
    const auto bitOffset = i % 8;
    const auto byteOffset = (i - bitOffset) / 8;

    auto& byte = report.buttons[byteOffset];
    byte |= (1 << bitOffset);
  }

  this->WriteReport(report, deviceIndex);
}

void FAVJoyState2::WriteReport(const Report& report, uint8_t deviceIndex) {
  if (deviceIndex >= mCount) {
    throw std::logic_error("Device index is >= device count");
  }
  mDevice.WriteReport(REPORT_IDS[deviceIndex], &report, sizeof(report));
}

std::optional<FAVJoyState2> FAVJoyState2::Open(uint8_t deviceCount) {
  auto a = Arduino::Open();
  if (!a) {
    return {};
  }
  return FAVJoyState2 {deviceCount, std::move(*a)};
}

std::optional<FAVJoyState2> FAVJoyState2::Open(
  const OpaqueID& serial,
  uint8_t deviceCount) {
  auto a = Arduino::Open(serial);
  if (!a) {
    return {};
  }
  return FAVJoyState2 {deviceCount, std::move(*a)};
}

}// namespace FAVHID