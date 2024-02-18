// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#include "favhid/Arduino.hpp"

#include "favhid/protocol.hpp"

#include <iostream>

#include <SetupAPI.h>

namespace FAVHID {

constexpr std::string_view MSG_HELLO {"FAVHID" FAVHID_PROTO_VERSION};
constexpr std::string_view MSG_HELLO_ACK {"ACKVER" FAVHID_PROTO_VERSION};

static void
WriteArduino(const winrt::file_handle& handle, const void* data, size_t size) {
  auto h = handle.get();
  winrt::check_bool(WriteFile(h, data, static_cast<DWORD>(size), nullptr, nullptr));
  winrt::check_bool(FlushFileBuffers(h));
}

static winrt::file_handle OpenArduino(std::wstring_view port) {
  winrt::file_handle f {
    CreateFileW(std::format(L"\\\\.\\{}", port).c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, NULL) };
  COMMCONFIG config {sizeof(COMMCONFIG)};
  DWORD configSize = sizeof(config);
  winrt::check_bool(GetCommConfig(f.get(), &config, &configSize));
  auto& dcb = config.dcb;
  dcb.BaudRate = 115200;
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;
  dcb.fDtrControl = DTR_CONTROL_ENABLE;
  dcb.fRtsControl = RTS_CONTROL_DISABLE;
  winrt::check_bool(SetCommConfig(f.get(), &config, sizeof(config)));

  WriteArduino(f, MSG_HELLO.data(), MSG_HELLO.size());

  char buf[MSG_HELLO_ACK.size()];
  DWORD bytesRead {};

  winrt::check_bool(ReadFile(f.get(), buf, sizeof(buf), &bytesRead, nullptr));

  if (bytesRead != MSG_HELLO_ACK.size()) {
    return {};
  }

  const std::string_view response {buf, bytesRead};
  if (response != MSG_HELLO_ACK) {
    return {};
  }

  return f;
}

winrt::file_handle Arduino::OpenHandle(const std::optional<OpaqueID>& serial) {
  std::vector<std::wstring> ports;
  ULONG count = 255;
  auto infoSet = SetupDiGetClassDevsW(&GUID_DEVINTERFACE_COMPORT, nullptr, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT | DIGCF_PROFILE);

  SP_DEVINFO_DATA devInfo  { sizeof(SP_DEVINFO_DATA) };
  DWORD deviceIndex = 0;
  while (SetupDiEnumDeviceInfo(infoSet , deviceIndex++, &devInfo)) {
    DWORD size = 0;

    SetupDiGetDeviceRegistryPropertyW(infoSet, &devInfo, SPDRP_DEVICEDESC, 0, nullptr, 0, &size);
    std::wstring description(size / sizeof(wchar_t), '\0');
    SetupDiGetDeviceRegistryPropertyW(infoSet, &devInfo, SPDRP_DEVICEDESC, 0, reinterpret_cast<PBYTE>(description.data()), static_cast<DWORD>(description.size() * sizeof(wchar_t)), &size);
    description.resize(description.size() - 1);
    if (description != L"Arduino Micro") {
      continue;
    }

    SetupDiGetCustomDevicePropertyW(infoSet, &devInfo, L"PortName", 0, 0, nullptr, 0, &size);
    std::wstring portName(size / sizeof(wchar_t), '\0');
    SetupDiGetCustomDevicePropertyW(infoSet, &devInfo, L"PortName", 0, 0, reinterpret_cast<PBYTE>(portName.data()), static_cast<DWORD>(portName.size() * sizeof(wchar_t)), &size);
    portName.resize(portName.size() - 1);

    ports.push_back(portName);
  }

  for (const auto port: ports) {
    try {
      auto f = OpenArduino(port);

      if (!f) {
        continue;
      }
      if (!serial) {
        return f;
      }

      Arduino a(std::move(f));
      if (a.GetSerialNumber() == *serial) {
        return std::move(a.mHandle);
      }
    } catch (...) {
      continue;
    }
  }
  return {};
}

std::optional<Arduino> Arduino::Open() {
  auto f = OpenHandle();
  if (!f) {
    return {};
  }
  return Arduino {std::move(f)};
}

std::optional<Arduino> Arduino::Open(const OpaqueID& serial) {
  auto f = OpenHandle(serial);
  if (!f) {
    return {};
  }
  return Arduino {std::move(f)};
}

Arduino::Arduino(THandle&& h) : mHandle(std::move(h)) {
}

void Arduino::Write(const void* data, size_t size) {
  WriteArduino(mHandle, data, size);
}

void Arduino::RandomizeSerialNumber() {
  static_assert(sizeof(OpaqueID) == SERIAL_SIZE);

  char buf[sizeof(ShortMessageHeader) + sizeof(OpaqueID)];
  *reinterpret_cast<ShortMessageHeader*>(buf) = {
    .type = MessageType::SetSerialNumber,
    .dataLength = sizeof(OpaqueID),
  };
  reinterpret_cast<OpaqueID*>(buf + sizeof(ShortMessageHeader))->Randomize();
  Write(buf, sizeof(buf));

  const auto response = ReadResponse();
  if (!response.IsOK()) {
    throw new std::runtime_error("Failed to set serial number");
  }
}

OpaqueID Arduino::GetSerialNumber() {
  ShortMessageHeader header {MessageType::GetSerialNumber, 0};
  Write(&header, sizeof(header));

  const auto response = ReadResponse();
  if (response.type != MessageType::Response_OK) {
    throw std::exception("Failed to get serial number");
  }
  if (response.data.size() != sizeof(OpaqueID)) {
    throw std::exception("Serial number is the wrong size");
  }

  return *reinterpret_cast<const OpaqueID*>(response.data.data());
}

Response Arduino::ReadResponse() {
  auto handle = mHandle.get();
  ShortMessageHeader header;
  winrt::check_bool(
    ReadFile(handle, &header, sizeof(header), nullptr, nullptr));

  if (header.dataLength == 0) {
    return {header.type};
  }

  std::string buf(header.dataLength, '\0');
  winrt::check_bool(ReadFile(
    handle, buf.data(), static_cast<DWORD>(buf.size()), nullptr, nullptr));

  return {header.type, std::move(buf)};
}

Response Arduino::PushDescriptor(
  const void* descriptor,
  size_t descriptorSize) {
  const auto dataSize = descriptorSize;
  const bool isLongMessage = dataSize > 0xff;
  const auto headerSize
    = isLongMessage ? sizeof(LongMessageHeader) : sizeof(ShortMessageHeader);
  const auto messageSize = headerSize + dataSize;

  std::string buf(messageSize, '\0');
  auto it = buf.data();
  if (isLongMessage) {
    *reinterpret_cast<LongMessageHeader*>(it) = {
      .type = MessageType::PushDescriptor,
      .dataLength = static_cast<uint16_t>(dataSize),
    };
    it += sizeof(LongMessageHeader);
  } else {
    *reinterpret_cast<ShortMessageHeader*>(it) = {
      .type = MessageType::PushDescriptor,
      .dataLength = static_cast<uint8_t>(dataSize),
    };
    it += sizeof(ShortMessageHeader);
  }

  memcpy(it, descriptor, descriptorSize);
  it += descriptorSize;

  // If you need to re-enable this, you have my sympathy.
  if constexpr (false) {
    for (int i = 0; i < descriptorSize; ++i) {
      std::cout << std::format(
        "{:#04x}", static_cast<const uint8_t*>(descriptor)[i])
                << std::endl;
    }
  }

  Write(buf.data(), buf.size());

  return ReadResponse();
}

Response
Arduino::WriteReport(uint8_t reportID, const void* report, size_t size) {
  const auto dataSize = size + 1;
  const bool isLongMessage = dataSize > 0xff;
  const auto headerSize
    = isLongMessage ? sizeof(LongMessageHeader) : sizeof(ShortMessageHeader);
  const auto messageSize = headerSize + dataSize;

  std::string buf(messageSize, '\0');
  auto it = buf.data();
  if (isLongMessage) {
    *reinterpret_cast<LongMessageHeader*>(it) = {
      .type = MessageType::Report,
      .dataLength = static_cast<uint16_t>(dataSize),
    };
    it += sizeof(LongMessageHeader);
  } else {
    *reinterpret_cast<ShortMessageHeader*>(it) = {
      .type = MessageType::Report,
      .dataLength = static_cast<uint8_t>(dataSize),
    };
    it += sizeof(ShortMessageHeader);
  }

  memcpy(it, &reportID, 1);
  it++;

  memcpy(it, report, size);

  Write(buf.data(), buf.size());
  return ReadResponse();
}

bool Arduino::ResetUSB() {
  const auto serial = GetSerialNumber();

  ShortMessageHeader header {MessageType::ResetUSB, 0};
  Write(&header, sizeof(header));
  mHandle.close();

  Sleep(1000);

  for (int i = 0; i < 25; ++i) {
    mHandle = OpenHandle(serial);
    if (mHandle) {
      break;
    }
    Sleep(250);
  }

  return !!mHandle;
}

bool Arduino::HardReset() {
  const auto serial = GetSerialNumber();

  ShortMessageHeader header {MessageType::HardReset, 0};
  Write(&header, sizeof(header));
  mHandle.close();

  Sleep(2000);

  for (int i = 0; i < 5; ++i) {
    mHandle = OpenHandle(serial);
    if (mHandle) {
      break;
    }

    Sleep(1000);
  }

  return !!mHandle;
}

OpaqueID Arduino::GetVolatileConfigID() {
  ShortMessageHeader header {MessageType::GetVolatileConfigID, 0};
  Write(&header, sizeof(header));

  const auto response = ReadResponse();
  if (response.type != MessageType::Response_OK) {
    throw std::exception("Failed to get config ID");
  }

  if (response.data.size() != sizeof(OpaqueID)) {
    throw std::exception("Config ID is the wrong size");
  }

  return *reinterpret_cast<const OpaqueID*>(response.data.data());
}

void Arduino::SetVolatileConfigID(const OpaqueID& id) {
  char buf[sizeof(ShortMessageHeader) + sizeof(OpaqueID)];
  *reinterpret_cast<ShortMessageHeader*>(buf) = {
    .type = MessageType::SetVolatileConfigID,
    .dataLength = sizeof(OpaqueID),
  };
  memcpy(buf + sizeof(ShortMessageHeader), &id, sizeof(OpaqueID));
  Write(buf, sizeof(buf));

  const auto response = ReadResponse();
  if (!response.IsOK()) {
    throw new std::runtime_error("Failed to set serial number");
  }
}

}// namespace FAVHID