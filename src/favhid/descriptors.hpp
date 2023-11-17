// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#pragma once

#include <bit>
#include <cinttypes>
#include <concepts>
#include <limits>
#include <string_view>

namespace FAVHID::Descriptors {

class Entry {
 public:
  std::basic_string_view<uint8_t> GetBytes() const {
    return mSerialized;
  }

 protected:
  Entry() = delete;
  Entry(std::basic_string_view<uint8_t> value) {
    mSerialized = std::basic_string<uint8_t> {value};
  }

  Entry(uint8_t value) : Entry({&value, 1}) {
  }

  Entry(uint16_t value) : Entry({reinterpret_cast<uint8_t*>(&value), 2}) {
  }

  Entry(uint8_t a, uint8_t b) : Entry(static_cast<uint16_t>((b << 8) | a)) {
  }

  std::basic_string<uint8_t> mSerialized;
};

class UsagePage final : public Entry {
 public:
  UsagePage(uint8_t value) : Entry(0x05, value) {
  }

  static UsagePage GenericDesktop() {
    return {0x01};
  }

  static UsagePage SimulationControls() {
    return {0x02};
  }

  static UsagePage VRControls() {
    return {0x03};
  }

  static UsagePage SportControls() {
    return {0x04};
  }

  static UsagePage GameControls() {
    return {0x05};
  }

  static UsagePage GenericDeviceControls() {
    return {0x06};
  }

  static UsagePage KeyboardKeypad() {
    return {0x07};
  }

  static UsagePage LED() {
    return {0x08};
  }
  
  static UsagePage Button() {
    return {0x09};
  }

  static UsagePage Ordinal() {
    return {0x0A};
  }

  static UsagePage TelephonyDevice() {
    return {0x0B};
  }

  static UsagePage Consumer() {
    return {0x0C};
  }

  static UsagePage Digitizers() {
    return {0x0D};
  }

  static UsagePage Haptics() {
    return {0x0E};
  }

  static UsagePage PhysicalInputDevice() {
    return {0x0F};
  }

  static UsagePage Unicode() {
    return {0x10};
  }

  static UsagePage SoC() {
    return {0x11};
  }

  static UsagePage EyeAndHeadTrackers() {
    return {0x12};
  }

  static UsagePage AuxilliaryDisplay() {
    return {0x15};
  }

  static UsagePage Sensors() {
    return {0x20};
  }

  static UsagePage MedicalInstrument() {
    return {0x40};
  }

  static UsagePage BrailleDisplay() {
    return {0x41};
  }

  static UsagePage LightingAndIllumination() {
    return {0x59};
  }

  static UsagePage Monitor() {
    return {0x80};
  }

  static UsagePage MonitorEnumerated() {
    return {0x81};
  }

  static UsagePage VESAVirtualControls() {
    return {0x82};
  }

  static UsagePage Power() {
    return {0x84};
  }

  static UsagePage BatterySystem() {
    return {0x85};
  }

  static UsagePage BarcodeScanner() {
    return {0x8C};
  }

  static UsagePage Scales() {
    return {0x8D};
  }

  static UsagePage MagneticStripeReader() {
    return {0x8E};
  }

  static UsagePage CameraControl() {
    return {0x90};
  }

  static UsagePage Arcade() {
    return {0x91};
  }

  static UsagePage GamingDevice() {
    return {0x92};
  }

/* This class doesn't currently support multi-byte values  
  static UsagePage FIDOAlliance() {
    return {0xF1D0};
  }
*/
};

class Usage final : public Entry {
 public:
  Usage(uint8_t value) : Entry(0x09, value) {
  }

  static Usage Pointer() {
    return {0x01};
  }

  static Usage Mouse() {
    return {0x02};
  }

  static Usage Joystick() {
    return {0x04};
  }

  static Usage Gamepad() {
    return {0x05};
  }

  static Usage Keyboard() {
    return {0x06};
  }

  static Usage Keypad() {
    return {0x07};
  }

  static Usage MultiAxisController() {
    return {0x08};
  }

  static Usage TabletPCSystemControls() {
    return {0x08};
  }

  static Usage WaterCoolingDevice() {
    return {0x0A};
  }

  static Usage ComputerChassisDevice() {
    return {0x0B};
  }

  static Usage WirelessRadioControls() {
    return {0x0C};
  }

  static Usage PortableDeviceControl() {
    return {0x0D};
  }

  static Usage SystemMultiAxisController() {
    return {0x0E};
  }

  static Usage SpacialController() {
    return {0x0F};
  }

  static Usage AssistiveControl() {
    return {0x10};
  }

  static Usage DeviceDock() {
    return {0x11};
  }

  static Usage DockableDevice() {
    return {0x12};
  }

  static Usage CallStateManagementControl() {
    return {0x13};
  }

  static Usage X() {
    return {0x30};
  }

  static Usage Y() {
    return {0x31};
  }

  static Usage Z() {
    return {0x32};
  }

  static Usage Rx() {
    return {0x33};
  }
  
  static Usage Ry() {
    return {0x34};
  }

  static Usage Rz() {
    return {0x35};
  }

  static Usage Slider() {
    return {0x36};
  }

  static Usage Dial() {
    return {0x37};
  }

  static Usage Wheel() {
    return {0x38};
  }

  static Usage HatSwitch() {
    return {0x39};
  }

  static Usage CountedBuffer() {
    return {0x3A};
  }

  static Usage ByteCount() {
    return {0x3B};
  }

  static Usage MotionWakeup() {
    return {0x3C};
  }

  static Usage Start() {
    return {0x3D};
  }

  static Usage Select() {
    return {0x3E};
  }

  static Usage Vx() {
    return {0x40};
  }

  static Usage Vy() {
    return {0x41};
  }

  static Usage Vz() {
    return {0x42};
  }
};

class UsageMinimum final : public Entry {
 public:
  UsageMinimum(auto value) : Entry(0x19, value) {
  }
};

class UsageMaximum final : public Entry {
 public:
  UsageMaximum(auto value) : Entry(0x29, value) {
  }
};

class Collection final : public Entry {
 public:
  template <std::derived_from<Entry>... Ts>
  Collection(uint8_t id, const Ts&... ts) : Entry(0xa1, id) {
    ([&]() { mSerialized += ts.GetBytes(); }(), ...);
    mSerialized += '\xc0';
  }

  template <std::convertible_to<Entry>... Ts>
  static Collection Physical(const Ts&... ts) {
    return {0x00, ts...};
  }

  template <std::convertible_to<Entry>... Ts>
  static Collection Application(const Ts&... ts) {
    return {0x01, ts...};
  }
};

class ReportID final : public Entry {
 public:
  ReportID(uint8_t value) : Entry(0x85, value) {
  }
};

// TODO: figure out multi-byte stuff
class IntValueEntry : public Entry {
 public:
  IntValueEntry(uint8_t id, auto value) : Entry(id) {
    if (value < 0) {
      mSerialized += std::bit_cast<char>(static_cast<int8_t>(value));
    } else {
      mSerialized += static_cast<char>(value);
    }
  }
};

class ReportSize final : public IntValueEntry {
 public:
  ReportSize(auto value) : IntValueEntry(0x75, value) {
  }
};

class ReportCount final : public IntValueEntry {
 public:
  ReportCount(auto value) : IntValueEntry(0x95, value) {
  }
};

class LogicalMinimum final : public IntValueEntry {
 public:
  LogicalMinimum(auto value) : IntValueEntry(0x15, value) {
  }

  template <class T>
  static LogicalMinimum ForType() {
    return {std::numeric_limits<T>::min()};
  }
};

class LogicalMaximum final : public IntValueEntry {
 public:
  LogicalMaximum(auto value) : IntValueEntry(0x25, value) {
  }

  template <class T>
  static LogicalMaximum ForType() {
    return {std::numeric_limits<T>::max()};
  }
};

// TODO: add support for multibyte flags
class Input final : public IntValueEntry {
 public:
  Input(auto flags) : IntValueEntry(0x81, flags) {
  }

  using Flag = uint8_t;

  // static constexpr Flag Data = 0;
  static constexpr Flag Constant = 1;

  // static constexpr Flag Array = 0;
  static constexpr Flag Variable = 1 << 1;

  // static constexpr Flag Absolute = 0;
  static constexpr Flag Relative = 1 << 2;

  // static constexpr Flag NoWrap = 0;
  static constexpr Flag Wrap = 1 << 3;

  // static constexpr Flag Linear = 0;
  static constexpr Flag NonLinear = 1 << 4;

  // static constexpr Flag PreferredState = 0;
  static constexpr Flag NoPreferredState = 1 << 5;

  // static constexpr Flag NoNullPosition = 0;
  static constexpr Flag NullState = 1 << 6;

  // static constexpr Flag NonVolatile = 0;
  static constexpr Flag Volatile = 1 << 7;

  static Input DataVariableAbsolute() {
    return {Variable};
  }
  static Input Padding() {
    return {Constant};
  }
};

class Descriptor final {
 public:
  Descriptor() = delete;

  template <std::derived_from<Entry>... Ts>
  Descriptor(const Ts&... entries) {
    ([&]() { mSerialized += entries.GetBytes(); }(), ...);
  }

  const void* data() const {
    return mSerialized.data();
  }

  size_t size() const {
    return mSerialized.size();
  }

 private:
  std::basic_string<uint8_t> mSerialized;
};

}// namespace FAVHID::Descriptors