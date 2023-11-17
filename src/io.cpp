// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#include "favhid/io.hpp"

#include "favhid/protocol.hpp"

constexpr std::string_view MSG_HELLO {"FAVHID" FAVHID_PROTO_VERSION};
constexpr std::string_view MSG_HELLO_ACK {"ACKVER" FAVHID_PROTO_VERSION};

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

  Write(f, MSG_HELLO.data(), MSG_HELLO.size());

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

winrt::file_handle OpenArduino() {
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

void Write(const THandle& handle, const void* data, size_t size) {
  winrt::check_bool(WriteFile(handle.get(), data, size, nullptr, nullptr));
  winrt::check_bool(FlushFileBuffers(handle.get()));
}

void RandomizeSerialNumber(const THandle& handle) {
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
  Write(handle, buf, sizeof(buf));

  const auto response = ReadResponse(handle);
  if (response.type != MessageType::Response_OK) {
    throw new std::runtime_error("Failed to set serial number");
  }
}

std::array<char, SERIAL_SIZE> GetSerialNumber(const THandle& handle) {
  ShortMessageHeader header {MessageType::GetSerialNumber, 0};
  Write(handle, &header, sizeof(header));

  auto response = ReadResponse(handle);
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

Response ReadResponse(const THandle& handle) {
  ShortMessageHeader header;
  winrt::check_bool(
    ReadFile(handle.get(), &header, sizeof(header), nullptr, nullptr));

  if (header.dataLength == 0) {
    return {header.type};
  }

  char buf[header.dataLength];
  winrt::check_bool(
    ReadFile(handle.get(), buf, header.dataLength, nullptr, nullptr));

  return {header.type, std::string {buf, header.dataLength}};
}

Response WriteDescriptor(
  const THandle& handle,
  uint8_t reportID,
  const void* descriptor,
  size_t descriptorSize,
  const void* initialReport,
  size_t reportSize) {
  const auto dataSize = static_cast<uint16_t>(
    sizeof(PlugDataHeader) + descriptorSize + reportSize);
  const bool isLongMessage = dataSize > 0xff;
  const auto headerSize
    = isLongMessage ? sizeof(LongMessageHeader) : sizeof(ShortMessageHeader);
  const auto messageSize = headerSize + dataSize;

  uint8_t buf[messageSize];
  auto it = buf;
  if (isLongMessage) {
    *reinterpret_cast<LongMessageHeader*>(it) = {
      .type = MessageType::Plug,
      .dataLength = dataSize,
    };
    it += sizeof(LongMessageHeader);
  } else {
    *reinterpret_cast<ShortMessageHeader*>(it) = {
      .type = MessageType::Plug,
      .dataLength = static_cast<uint8_t>(dataSize),
    };
    it += sizeof(ShortMessageHeader);
  }

  *reinterpret_cast<PlugDataHeader*>(it) = {
    .id = reportID,
    .descriptorLength = static_cast<uint16_t>(descriptorSize),
    .reportLength = static_cast<uint16_t>(reportSize),
  };
  it += sizeof(PlugDataHeader);

  memcpy(it, descriptor, descriptorSize);
  it += descriptorSize;
  memcpy(it, initialReport, reportSize);

  Write(handle, buf, messageSize);

  return ReadResponse(handle);
}

Response WriteReport(
  const THandle& handle,
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

  Write(handle, buf, messageSize);
  return ReadResponse(handle);
}