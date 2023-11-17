// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#pragma once

#include "protocol.hpp"

#include <Windows.h>

#include <winrt/base.h>

#include <optional>

namespace FAVHID {

struct Response {
  MessageType type;
  std::string data;

  constexpr bool IsOK() const {
    return type == MessageType::Response_OK;
  }
};

/** The primary C++ API for FAVHID
 *
 * 
 * One-off Preparation
 * ===================
 * 
 * Generate a GUID or OpaqueID for your configuration; this is used to
 * detect when the Arduino needs to be rebooted to get a fresh start.
 * 
 * You can use the Visual Studio GUID generator for this, or the
 * `generate-random-ids` tool from this repository.
 * 
 * This ID should be the same between runs, unless you use different
 * HID descriptors in different runs.
 * 
 * Usage
 * =====
 * 
 * 1. Open() an Arduino. Specifying a serial number is recommended, but
 *    optional. Serial numbers are unique to FAVHID.
 * 2. Check if `GetVolatileConfigID()` matches the `OpaqueID` you created
 *    earlier; if so, jump to step 3; otherwise, you need to initialize the
 *    device:
 *    2.1) Call `HardReset()` to reset the device and clear RAM
 *    2.2) Call `PushDescriptor()` for each of your descriptors
 *    2.3) Call `SetVolatileConfigID()`, passing in the `OpaqueID` you
 *      created earlier.
 *    2.4) Call `ResetUSB()` so that the OS picks up the new descriptors
 *    2.5) Call `GetVolatileConfigID()` now matches the `OpaqueID` you
 *      provided
 * 3. Feed: Call `WriteReport()` to push new data.
 */
class Arduino final {
 public:
  Arduino() = delete;

  static std::optional<Arduino> Open();
  static std::optional<Arduino> Open(const OpaqueID& serial);
  inline static std::optional<Arduino> Open(const GUID& serial) {
    return Open(reinterpret_cast<const OpaqueID&>(serial));
  }

  // The Arduino will drop the connection after this, close the handle
  // immediately.
  Response PushDescriptor(const void* descriptor, size_t descriptorSize);
  Response WriteReport(uint8_t reportID, const void* report, size_t size);

  OpaqueID GetVolatileConfigID();
  void SetVolatileConfigID(const OpaqueID&);
  inline void SetVolatileConfigID(const GUID& id) {
    return SetVolatileConfigID(reinterpret_cast<const OpaqueID&>(id));
  }

  [[nodiscard]] bool ResetUSB();
  [[nodiscard]] bool HardReset();

  /* Retrieves the serial number from EEPROM.
   *
   * The serial number is generated via a past call to
   * `RandomizeSerialNumber()`.
   */
  OpaqueID GetSerialNumber();
  // Convenient for windows users
  static_assert(sizeof(OpaqueID) == sizeof(GUID));
  
  /* Write a random number to EEPROM.
   *
   * This should be called extremely rarely, ideally only when setting
   * up a device for the very first time.
   */
  void RandomizeSerialNumber();
  
 private:
  using THandle = winrt::file_handle;

  THandle mHandle;

  Arduino(THandle&&);
  void Write(const void* data, size_t size);
  Response ReadResponse();

  static THandle OpenHandle(const std::optional<OpaqueID>& serial = {});
};

}// namespace FAVHID