// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#pragma once

#include <bit>
#include <cinttypes>
#include <concepts>
#include <limits>
#include <string_view>
#include <type_traits>

namespace FAVHID::Descriptors {

template <size_t N>
class Entry {
 public:
  static constexpr size_t Size = N;

  constexpr const uint8_t* data() const {
    return mSerialized;
  }

  constexpr size_t size() const {
    return mUsedBytes;
  }

 protected:
  Entry() = default;
  constexpr Entry(size_t usedBytes, const uint8_t (&value)[N])
    : mUsedBytes(usedBytes) {
    std::copy_n(value, N, mSerialized);
  }

  template <class... Bytes>
  constexpr Entry(size_t usedBytes, Bytes... bytes) : mUsedBytes(usedBytes) {
    size_t i = 0;
    ([&]() { mSerialized[i++] = bytes; }(), ...);
  }

  uint8_t mSerialized[N] {};
  size_t mUsedBytes {0};
};

namespace UsagePage {

template <class... Vs>
class UsagePage final : public Entry<sizeof...(Vs) + 1> {
 public:
  constexpr UsagePage(Vs... vs)
    : Entry<sizeof...(vs) + 1>(sizeof...(vs) + 1, 0x05, vs...) {
  }
};

constexpr UsagePage GenericDesktop {0x01};
constexpr UsagePage SimulationControls {0x02};
constexpr UsagePage VRControls {0x03};
constexpr UsagePage SportControls {0x04};
constexpr UsagePage GameControls {0x05};
constexpr UsagePage GenericDeviceControls {0x06};
constexpr UsagePage KeyboardKeypad {0x07};
constexpr UsagePage LED {0x08};
constexpr UsagePage Button {0x09};
constexpr UsagePage Ordinal {0x0A};
constexpr UsagePage TelephonyDevice {0x0B};
constexpr UsagePage Consumer {0x0C};
constexpr UsagePage Digitizers {0x0D};
constexpr UsagePage Haptics {0x0E};
constexpr UsagePage PhysicalInputDevice {0x0F};
constexpr UsagePage Unicode {0x10};
constexpr UsagePage SoC {0x11};
constexpr UsagePage EyeAndHeadTrackers {0x12};
constexpr UsagePage AuxilliaryDisplay {0x15};
constexpr UsagePage Sensors {0x20};
constexpr UsagePage MedicalInstrument {0x40};
constexpr UsagePage BrailleDisplay {0x41};
constexpr UsagePage LightingAndIllumination {0x59};
constexpr UsagePage Monitor {0x80};
constexpr UsagePage MonitorEnumerated {0x81};
constexpr UsagePage VESAVirtualControls {0x82};
constexpr UsagePage Power {0x84};
constexpr UsagePage BatterySystem {0x85};
constexpr UsagePage BarcodeScanner {0x8C};
constexpr UsagePage Scales {0x8D};
constexpr UsagePage MagneticStripeReader {0x8E};
constexpr UsagePage CameraControl {0x90};
constexpr UsagePage Arcade {0x91};
constexpr UsagePage GamingDevice {0x92};

// This class doesn't currently support multi-byte values
// static constexpr UsagePage FIDOAlliance {0xF1D0};
}// namespace UsagePage

namespace Usage {

template <class... Vs>
class Usage final : public Entry<sizeof...(Vs) + 1> {
 public:
  constexpr Usage(Vs... vs)
    : Entry<sizeof...(vs) + 1>(sizeof...(vs) + 1, 0x09, vs...) {
  }
};

///// Generic Desktop /////

constexpr Usage Pointer {0x01};
constexpr Usage Mouse {0x02};
constexpr Usage Joystick {0x04};
constexpr Usage Gamepad {0x05};
constexpr Usage Keyboard {0x06};
constexpr Usage Keypad {0x07};
constexpr Usage MultiAxisController {0x08};
constexpr Usage TabletPCSystemControls {0x08};
constexpr Usage WaterCoolingDevice {0x0A};
constexpr Usage ComputerChassisDevice {0x0B};
constexpr Usage WirelessRadioControls {0x0C};
constexpr Usage PortableDeviceControl {0x0D};
constexpr Usage SystemMultiAxisController {0x0E};
constexpr Usage SpacialController {0x0F};
constexpr Usage AssistiveControl {0x10};
constexpr Usage DeviceDock {0x11};
constexpr Usage DockableDevice {0x12};
constexpr Usage CallStateManagementControl {0x13};
constexpr Usage X {0x30};
constexpr Usage Y {0x31};
constexpr Usage Z {0x32};
constexpr Usage Rx {0x33};
constexpr Usage Ry {0x34};
constexpr Usage Rz {0x35};
constexpr Usage Slider {0x36};
constexpr Usage Dial {0x37};
constexpr Usage Wheel {0x38};
constexpr Usage HatSwitch {0x39};
constexpr Usage CountedBuffer {0x3A};
constexpr Usage ByteCount {0x3B};
constexpr Usage MotionWakeup {0x3C};
constexpr Usage Start {0x3D};
constexpr Usage Select {0x3E};
constexpr Usage Vx {0x40};
constexpr Usage Vy {0x41};
constexpr Usage Vz {0x42};
constexpr Usage Vbrx {0x43};
constexpr Usage Vbry {0x44};
constexpr Usage Vbrz {0x45};
constexpr Usage Vno {0x46};
constexpr Usage FeatureNotification {0x47};
constexpr Usage ResolutionMultiplier {0x48};
constexpr Usage Qx {0x49};
constexpr Usage Qy {0x4A};
constexpr Usage Qz {0x4B};
constexpr Usage Qw {0x4C};

}// namespace Usage

template <class... Vs>
class UsageMinimum final : public Entry<sizeof...(Vs) + 1> {
 public:
  constexpr UsageMinimum(Vs... vs)
    : Entry<sizeof...(Vs) + 1>(sizeof...(Vs) + 1, 0x19, vs...) {
  }
};

template <class... Vs>
class UsageMaximum final : public Entry<sizeof...(Vs) + 1> {
 public:
  constexpr UsageMaximum(Vs... vs)
    : Entry<sizeof...(Vs) + 1>(sizeof...(Vs) + 1, 0x29, vs...) {
  }
};

namespace Collection {

template <class... Entries>
class Collection : public Entry<3 + (... + Entries::Size)> {
 private:
  using Base = Entry<3 + (... + Entries::Size)>;

 public:
  constexpr Collection(uint8_t id, const Entries&... entries) : Base({}) {
    Base::mSerialized[0] = 0xa1;
    Base::mSerialized[1] = id;
    size_t i = 2;
    (
      [&]() {
        const auto size = entries.size();
        std::copy_n(entries.data(), size, Base::mSerialized + i);
        i += size;
      }(),
      ...);
    Base::mSerialized[i] = '\xc0';
    Base::mUsedBytes = i + 1;
  }
};

template <class... Entries>
class Physical final : public Collection<Entries...> {
 public:
  constexpr Physical(const Entries&... entries)
    : Collection<Entries...>(0x00, entries...) {
  }
};

template <class... Entries>
class Application final : public Collection<Entries...> {
 public:
  constexpr Application(const Entries&... entries)
    : Collection<Entries...>(0x01, entries...) {
  }
};
}// namespace Collection

class ReportID final : public Entry<2> {
 public:
  constexpr ReportID(uint8_t id) : Entry<2>(2, 0x85, id) {
  }
};

class ReportSize final : public Entry<2> {
 public:
  constexpr ReportSize(uint8_t value) : Entry<2>(2, 0x75, value) {
  }
};

class ReportCount final : public Entry<2> {
 public:
  constexpr ReportCount(uint8_t value) : Entry<2>(2, 0x95, value) {
  }
};

// + 1 byte for id
// *sometimes* +1 byte to fit values at std::numeric_limits<V>
template <uint8_t Tag, class V>
class IntegerEntry : public Entry<sizeof(V) + 2> {
 private:
  using Base = Entry<sizeof(V) + 2>;

 protected:
  constexpr IntegerEntry(V value) : Base() {
    // Strip size
    const uint8_t id = Tag & ~0x03;
    Base::mSerialized[0] = id;

    if (static_cast<int8_t>(value) == value) {
      this->Fill<int8_t>(value);
      return;
    }

    if (static_cast<int16_t>(value) == value) {
      this->Fill<int16_t>(value);
      return;
    }

    if (static_cast<int32_t>(value) == value) {
      this->Fill<int32_t>(value);
      return;
    }

    // If we reach here, the size bits and the data bytes will be 0
  }

  template <class FillT>
  constexpr void Fill(FillT value) {
    const auto unsigned_v = std::bit_cast<std::make_unsigned_t<FillT>>(value);

    auto& id = Base::mSerialized[0];
    switch (sizeof(value)) {
      case 1:
        id |= 1;
        break;
      case 2:
        id |= 2;
        break;
      case 4:
        id |= 3;// careful now
        break;
    }

    for (int i = 0; i < sizeof(value); ++i) {
      Base::mSerialized[i + 1] = static_cast<uint8_t>(unsigned_v >> (i * 8));
    }
    Base::mUsedBytes = sizeof(value)+ 1;
  }
};

template <class V>
class LogicalMinimum final : public IntegerEntry<0x15, V> {
 public:
  constexpr LogicalMinimum(V value) : IntegerEntry<0x15, V>(value) {
  }

  constexpr LogicalMinimum() : LogicalMinimum(std::numeric_limits<V>::min()) {
  }
};

template <class V>
class LogicalMaximum final : public IntegerEntry<0x25, V> {
 public:
  constexpr LogicalMaximum(V value) : IntegerEntry<0x25, V>(value) {
  }
  constexpr LogicalMaximum() : LogicalMaximum(std::numeric_limits<V>::max()) {
  }
};

namespace Input {
class Input final : public Entry<2> {
 public:
  constexpr Input(uint8_t flags) : Entry(2, 0x81, flags) {
  }
};

namespace Flags {
using Flag = uint8_t;

// constexpr Flag Data = 0;
constexpr Flag Constant = 1;

// constexpr Flag Array = 0;
constexpr Flag Variable = 1 << 1;

// constexpr Flag Absolute = 0;
constexpr Flag Relative = 1 << 2;

// constexpr Flag NoWrap = 0;
constexpr Flag Wrap = 1 << 3;

// constexpr Flag Linear = 0;
constexpr Flag NonLinear = 1 << 4;

// constexpr Flag PreferredState = 0;
constexpr Flag NoPreferredState = 1 << 5;

// constexpr Flag NoNullPosition = 0;
constexpr Flag NullState = 1 << 6;

// constexpr Flag NonVolatile = 0;
constexpr Flag Volatile = 1 << 7;
};// namespace Flags

constexpr Input DataVariableAbsolute {Flags::Variable};
constexpr Input DataVariableAbsoluteNullState {
  Flags::Variable | Flags::NullState};
constexpr Input Padding {Flags::Constant};
}// namespace Input

template <class... Entries>
class Descriptor final : public Entry<(... + Entries::Size)> {
 private:
  using Base = Entry<(... + Entries::Size)>;

 public:
  Descriptor() = delete;

  constexpr Descriptor(const Entries&... entries) : Base({}) {
    size_t i = 0;
    (
      [&]() {
        const auto size = entries.size();
        std::copy_n(entries.data(), size, Base::mSerialized + i);
        i += size;
      }(),
      ...);
    Base::mUsedBytes = i;
  }
};

}// namespace FAVHID::Descriptors