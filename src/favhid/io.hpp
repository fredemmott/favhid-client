// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#pragma once

#include "protocol.hpp"

#include <Windows.h>

#include <winrt/base.h>

#include <array>
#include <optional>

namespace FAVHID {

struct Response {
  MessageType type;
  std::string data;

  constexpr bool IsOK() const {
    return type == MessageType::Response_OK;
  }
};

class Arduino final {
 public:
  Arduino() = delete;

  static std::optional<Arduino> Open();

  // The Arduino will drop the connection after this, close the handle
  // immediately.
  Response PushDescriptor(const void* descriptor, size_t descriptorSize);
  Response WriteReport(uint8_t reportID, const void* report, size_t size);

  /* Retrieves the serial number from EEPROM.
   *
   * The serial number is generated via a past call to
   * `RandomizeSerialNumber()`.
   */
  std::array<char, SERIAL_SIZE> GetSerialNumber();
  // Convenient for windows users
  static_assert(SERIAL_SIZE == sizeof(GUID));
  
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
};

}// namespace FAVHID