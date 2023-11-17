// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#include "favhid/protocol.hpp"

#include <format>
#include <iostream>

using namespace FAVHID;

int main() {
  for (int i = 0; i < 10; ++i) {
    const auto id = OpaqueID::Random();
    std::cout << "// " << id.HumanReadable() << std::endl;
    std::cout << std::format(
      "constexpr OpaqueID MY_ID {{\n"
      "  0x{:08x},\n"
      "  0x{:04x},\n"
      "  0x{:04x},\n"
      "  {{"
      "0x{:02x}, 0x{:02x}, 0x{:02x}, 0x{:02x}, "
      "0x{:02x}, 0x{:02x}, 0x{:02x}, 0x{:02x}}},\n"
      "}};",
      id.Data1,
      id.Data2,
      id.Data3,
      id.Data4[0],
      id.Data4[1],
      id.Data4[2],
      id.Data4[3],
      id.Data4[4],
      id.Data4[5],
      id.Data4[6],
      id.Data4[7])
              << std::endl;
  }
  return 0;
}