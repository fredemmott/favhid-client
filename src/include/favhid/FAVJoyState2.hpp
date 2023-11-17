// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#pragma once

#include "Arduino.hpp"

#include <dinput.h>

namespace FAVHID {

// Wrapper around 'Arduino' that provides a quick-start interface for those
// familiar with DIJOYSTATE2
class FAVJoyState2 final {
 public:
  FAVJoyState2() = delete;

  static constexpr uint8_t MAX_DEVICES = 4;
    
  static std::optional<FAVJoyState2> Open(uint8_t deviceCount = 1);
  static std::optional<FAVJoyState2> Open(const OpaqueID& serial, uint8_t deviceCount = 1);
  inline static std::optional<FAVJoyState2> Open(const GUID& serial, uint8_t deviceCount = 1) {
    return Open(reinterpret_cast<const OpaqueID&>(serial), deviceCount);
  }

  void WriteReport(const DIJOYSTATE2&, uint8_t deviceIndex = 0);

  private:
    FAVJoyState2(uint8_t deviceCount, Arduino&&);

    Arduino mDevice;
    uint8_t mCount {};
    OpaqueID mConfigID;
};

}// namespace FAVHID
