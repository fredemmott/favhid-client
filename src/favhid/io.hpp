// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#pragma once

#include "protocol.hpp"

#include <Windows.h>

#include <winrt/base.h>

#include <array>

namespace FAVHID {

using THandle = winrt::file_handle;

THandle OpenArduino();

void Write(const THandle&, const void* data, size_t size);
void RandomizeSerialNumber(const THandle&);

std::array<char, SERIAL_SIZE> GetSerialNumber(const THandle&);

struct Response {
  MessageType type;
  std::string data;

  constexpr bool IsOK() const {
    return type == MessageType::Response_OK;
  }
};

Response ReadResponse(const THandle&);

// The Arduino will drop the connection after this, close the handle
// immediately.
Response WriteDescriptor(
  const THandle& handle,
  uint8_t reportID,
  const void* descriptor,
  size_t descriptorSize,
  const void* initialReport,
  size_t initialReportSize);

Response WriteReport(
  const THandle& handle,
  uint8_t reportID,
  const void* report,
  size_t size);

}