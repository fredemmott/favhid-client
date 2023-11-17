// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#include "favhid/FAVJoyState2.hpp"

#include <chrono>
#include <thread>

int main() {
  constexpr uint8_t VIRTUAL_DEVICE_COUNT = 2;
  auto favhid = FAVHID::FAVJoyState2::Open(VIRTUAL_DEVICE_COUNT);
  if (!favhid) {
    return 1;
  }

  uint64_t frameCount = 0;
  while (true) {
    for (int i = 0; i < VIRTUAL_DEVICE_COUNT; ++i) {
      DIJOYSTATE2 state {};
      state.rgbButtons[i] = 0xff;
      state.rgbButtons
        [FAVHID::FAVJoyState2::MAX_DEVICES
         + frameCount % (128 - FAVHID::FAVJoyState2::MAX_DEVICES)]
        = 0xff;

      for (int pov = 0; pov < 4; ++pov) {
        // 8 directions + center = 9;
        const auto pos = (frameCount + pov) % 9;
        state.rgdwPOV[pov] = pos ? ((pos - 1) * 4500) : -1;
      }

      favhid->WriteReport(state, i);
    }
    frameCount++;
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
  }

  return 0;
}