// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#pragma once

#include "Arduino.hpp"

#include <stdexcept>

#include <dinput.h>

namespace FAVHID {

// Wrapper around 'Arduino' that provides a quick-start interface for those
// familiar with DIJOYSTATE2, or who just want a quick-and-easy full-featured
// virtual joystick
class FAVJoyState2 final {
 public:
  FAVJoyState2() = delete;

  static constexpr uint8_t MAX_DEVICES = 8;

  static std::string_view GetDescriptor(uint8_t device);

  // Open the the first Arduino Micro running compatible firmware, and create
  // the specified number of virtual joysticks
  static std::optional<FAVJoyState2> Open(uint8_t deviceCount = 1);
  // Open the the Arduino Micro running compatible firmware with the specified
  // serial number, and create the specified number of virtual joysticks
  static std::optional<FAVJoyState2> Open(
    const OpaqueID& serial,
    uint8_t deviceCount = 1);

  /* Create and write a HID report based on the provided DIJOYSTATE2.
   *
   * Directly calling `WriteReport(const Report&, uint8_t deviceIndex)` is
   * more efficient; this function exists just for familiarity.
   */
  void WriteReport(const DIJOYSTATE2&, uint8_t deviceIndex = 0);

#pragma pack(push, 1)
  // The raw HID report actually used
  struct Report final {
    // Standard axes from INT16_MIN to INT16_MAX
    int16_t x, y, z, rx, ry, rz;
    // 2 additional axes from INT16_MIN to INT16_MAX
    int16_t slider[2];
    /* 4x 8-way+center POV hats, with 4 bits each.
     *
     * - 0b1111: center
     * - 0b0000-0b0111: degrees from north / 45, i.e.:
     *   - 0b0000: N
     *   - 0b0001: NE
     *   - 0b0010: E
     *   - 0b0011: SE
     *   - 0b0100: S
     *   - 0b0101: SW
     *   - 0b0110: W
     *   - 0b0111: NW
     *
     * IMPORTANT: these are 4 sequential 4-byte fields - so,
     * despite the values being little-endian, the fields are
     * in the opposite order to what you'd expect
     */
    uint8_t povs[2] { 0xff, 0xff };
    // 128 buttons, 1 bit per button
    uint8_t buttons[128 / 8];

    // The following fields from DIJOYSTATE2 are not currently supported:
    //
    // int16_t vx, vy, vz;
    // int16_t rvx, rvy, rvz;
    // int16_t vslider[2];
    // int16_t ax, ay, az;
    // int16_t arx, ary, arz;
    // int16_t aslider[2];
    // int16_t fx, fy, fz;
    // int16_t frx, fry, frz;
    // int16_t fslider[2];

    inline void SetButton(uint8_t buttonIndex, bool on = true) {
      if (buttonIndex >= (sizeof(buttons) * 8)) {
        throw std::logic_error("button index out of range");
      }

      const uint8_t bitOffset = buttonIndex % 8;
      const uint8_t byteOffset = (buttonIndex - bitOffset) / 8;

      auto& byte = this->buttons[byteOffset];
      if (on) {
        byte |= (1 << bitOffset);
      } else {
        byte &= ~static_cast<uint8_t>(1 << bitOffset);
      }
    }

    // See documentation of 'povs' field for information
    // on values
    inline void SetPOV(uint8_t hatIndex, uint8_t value) {
      if (hatIndex >= (sizeof(povs) * 2)) {
        throw std::logic_error("hat index out of range");
      }

      const auto bitOffset = 4 - (4 * (hatIndex % 2));
      const auto byteOffset = 1 - (hatIndex / 2);
      auto& byte = this->povs[byteOffset];

      byte &= ~(0b1111 << bitOffset);
      byte |= (value << bitOffset);
    }
  };
#pragma pack(pop)
  // Write the specified raw HID report.
  void WriteReport(const Report&, uint8_t deviceIndex);

 private:
  FAVJoyState2(uint8_t deviceCount, Arduino&&);

  Arduino mDevice;
  uint8_t mCount {};
  OpaqueID mConfigID;
};

}// namespace FAVHID
