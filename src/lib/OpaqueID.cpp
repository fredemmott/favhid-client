// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#include "favhid/protocol.hpp"

#include <Windows.h>
#include <stdexcept>

#include <winrt/base.h>

namespace FAVHID {

void OpaqueID::Randomize() {
  static_assert(sizeof(UUID) == sizeof(OpaqueID));
  if (UuidCreate(reinterpret_cast<UUID*>(this)) != RPC_S_OK) {
    throw new std::runtime_error("Failed to create a UUID");
  }
}

OpaqueID OpaqueID::Random() {
    OpaqueID ret;
    ret.Randomize();
    return ret;
}

std::string OpaqueID::HumanReadable() const {
    static_assert(sizeof(OpaqueID) == sizeof(winrt::guid));
    winrt::guid guid(*reinterpret_cast<const GUID *>(this));
    return winrt::to_string(winrt::to_hstring(guid));
}

}// namespace FAVHID