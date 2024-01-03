// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#include "favhid/FAVJoyState2.hpp"

#include <chrono>
#include <numbers>
#include <thread>

#include <iostream>
#include <format>


static void test_hat_math() {
  #define CHECK(x) std::cout << ((x) ? "OK: ": "FAIL: ") << #x << std::endl;

  FAVHID::FAVJoyState2::Report report {};

  report.SetPOV(0, 1);
  CHECK(report.povs[1] == 0b00011111);
  report.SetPOV(1, 1);
  CHECK(report.povs[1] == 0b00010001);

  report.SetPOV(2, 1);
  CHECK(report.povs[0] == 0b00011111);
  report.SetPOV(3, 1);
  CHECK(report.povs[0] == 0b00010001);

  #undef CHECK
}

static void unit_tests() {
  test_hat_math();
}

int main() {
  unit_tests();

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
      FAVHID::FAVJoyState2::Report state {};

      // Flash the button with ID matching the device ID
      state.SetButton(i, (slowChangeCount % 2) == 0);
      // Rotate through all the rest
      state.SetButton(
        FAVHID::FAVJoyState2::MAX_DEVICES
         + slowChangeCount % (128 - FAVHID::FAVJoyState2::MAX_DEVICES));

      // Rotate POVs through all positions
      for (int pov = 0; pov < 4; ++pov) {
        // 8 directions + center = 9 possibilities;
        const auto pos = (slowChangeCount + pov) % 9;
        constexpr uint8_t center = 0b1111;
        state.SetPOV(pov, pos == 8 ? center : pos);
      }

      // Draw sine waves on all aaxes
      for (int axis = 0; axis < 8; ++axis) {
        constexpr auto accelerate = 8.0f;
        auto* value = axis + &state.x;
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