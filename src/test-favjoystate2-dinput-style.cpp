// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#include "favhid/FAVJoyState2.hpp"

#include <chrono>
#include <numbers>
#include <thread>

int main() {
  constexpr uint8_t VIRTUAL_DEVICE_COUNT = FAVHID::FAVJoyState2::MAX_DEVICES;
  auto favhid = FAVHID::FAVJoyState2::Open(VIRTUAL_DEVICE_COUNT);
  if (!favhid) {
    return 1;
  }

  uint64_t frameCount = 0;
  while (true) {
    for (int i = 0; i < VIRTUAL_DEVICE_COUNT; ++i) {
      // Don't change POV and button states *too* quickly

      const auto slowChangeCount = frameCount / 2;
      DIJOYSTATE2 state {};
      
      // Flash the button with ID matching the device ID
      state.rgbButtons[i] = (slowChangeCount % 2) ? 0 : 0xff;
      // Rotate through all the rest
      state.rgbButtons
        [FAVHID::FAVJoyState2::MAX_DEVICES
         + slowChangeCount % (128 - FAVHID::FAVJoyState2::MAX_DEVICES)]
        = 0xff;

      // Rotate POVs through all positions
      for (int pov = 0; pov < 4; ++pov) {
        // 8 directions + center = 9 possibilities;
        const auto pos = (slowChangeCount + pov) % 9;
        state.rgdwPOV[pov] = pos ? ((pos - 1) * 4500) : -1;        
      }

      for (int axis = 0; axis < 8; ++axis) {
        constexpr auto accelerate = 8.0f;
        auto* value = axis + &state.lX;
        const auto pi = std::numbers::pi_v<float>;
        *value = std::numeric_limits<int16_t>::max() * sin((accelerate * frameCount * pi / 180.0f) + (std::numbers::pi * axis / 2.0f));
      }

      favhid->WriteReport(state, i);
    }
    frameCount++;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return 0;
}