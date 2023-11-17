// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#include "favhid/Arduino.hpp"

#include "favhid/protocol.hpp"

namespace FAVHID {

constexpr std::string_view MSG_HELLO {"FAVHID" FAVHID_PROTO_VERSION};
constexpr std::string_view MSG_HELLO_ACK {"ACKVER" FAVHID_PROTO_VERSION};

static void WriteArduino(const winrt::file_handle& handle, const void* data, size_t size) {
  auto h = handle.get();
  winrt::check_bool(WriteFile(h, data, size, nullptr, nullptr));
  winrt::check_bool(FlushFileBuffers(h));
}

static winrt::file_handle OpenArduino(ULONG port) {
  winrt::file_handle f {
    OpenCommPort(port, GENERIC_READ | GENERIC_WRITE, /* flags = */ 0)};
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

static winrt::file_handle OpenArduino() {
  ULONG ports[255];
  ULONG count = 255;
  if (GetCommPorts(ports, 255, &count) != ERROR_SUCCESS) {
    return {};
  }
  for (int i = 0; i < count; ++i) {
    auto f = OpenArduino(ports[i]);
    if (f) {
      return f;
    }
  }
  return {};
}

std::optional<Arduino> Arduino::Open() {
  auto f = OpenArduino();
  if (!f) {
    return {};
  }
  return Arduino { std::move(f) };
}

Arduino::Arduino(THandle&& h) : mHandle(std::move(h)) {}

void Arduino::Write(const void* data, size_t size) {
  WriteArduino(mHandle, data, size);
}

void Arduino::RandomizeSerialNumber() {
  constexpr auto dataSize = sizeof(UUID);
  static_assert(dataSize == SERIAL_SIZE);

  char buf[dataSize + sizeof(ShortMessageHeader)];
  *reinterpret_cast<ShortMessageHeader*>(buf) = {
    .type = MessageType::SetSerialNumber,
    .dataLength = dataSize,
  };
  if (
    UuidCreate(reinterpret_cast<UUID*>(buf + sizeof(ShortMessageHeader)))
    != RPC_S_OK) {
    throw new std::runtime_error("Failed to create a UUID");
  }
  Write(buf, sizeof(buf));

  const auto response = ReadResponse();
  if (response.type != MessageType::Response_OK) {
    throw new std::runtime_error("Failed to set serial number");
  }
}

std::array<char, SERIAL_SIZE> Arduino::GetSerialNumber() {
  ShortMessageHeader header {MessageType::GetSerialNumber, 0};
  Write(&header, sizeof(header));

  auto response = ReadResponse();
  if (response.type != MessageType::Response_OK) {
    throw std::exception("Failed to get serial number");
  }
  if (response.data.size() != SERIAL_SIZE) {
    throw std::exception("Serial number is the wrong size");
  }

  std::array<char, SERIAL_SIZE> ret;
  memcpy(ret.data(), response.data.data(), SERIAL_SIZE);
  return ret;
}

Response Arduino::ReadResponse() {
  auto handle = mHandle.get();
  ShortMessageHeader header;
  winrt::check_bool(
    ReadFile(handle, &header, sizeof(header), nullptr, nullptr));

  if (header.dataLength == 0) {
    return {header.type};
  }

  char buf[header.dataLength];
  winrt::check_bool(
    ReadFile(handle, buf, header.dataLength, nullptr, nullptr));

  return {header.type, std::string {buf, header.dataLength}};
}

Response Arduino::PushDescriptor(
  const void* descriptor,
  size_t descriptorSize) {
  const auto dataSize = descriptorSize;
  const bool isLongMessage = dataSize > 0xff;
  const auto headerSize
    = isLongMessage ? sizeof(LongMessageHeader) : sizeof(ShortMessageHeader);
  const auto messageSize = headerSize + dataSize;

  uint8_t buf[messageSize];
  auto it = buf;
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

  Write(buf, messageSize);

  return ReadResponse();
}

Response Arduino::WriteReport(
  uint8_t reportID,
  const void* report,
  size_t size) {

  const auto dataSize = size + 1;
  const bool isLongMessage = dataSize > 0xff;
  const auto headerSize
    = isLongMessage ? sizeof(LongMessageHeader) : sizeof(ShortMessageHeader);
  const auto messageSize = headerSize + dataSize;

  uint8_t buf[messageSize];
  auto it = buf;
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

  Write(buf, messageSize);
  return ReadResponse();
}

}