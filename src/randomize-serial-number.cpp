// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#include "favhid/io.hpp"

#include <winrt/base.h>

#include <iostream>

using namespace FAVHID;

static std::string FormatSerial(const std::array<char, SERIAL_SIZE> &serial)
{
    static_assert(SERIAL_SIZE == sizeof(winrt::guid));
    winrt::guid guid(*reinterpret_cast<const GUID *>(serial.data()));
    return winrt::to_string(winrt::to_hstring(guid));
}

int main()
{
    auto f = OpenArduino();
    if (!f)
    {
        std::cout << "Failed to find a compatible arduino :'(" << std::endl;
        return 1;
    }

    const auto oldSerial = GetSerialNumber(f);
    RandomizeSerialNumber(f);
    const auto newSerial = GetSerialNumber(f);

    if (oldSerial == newSerial)
    {
        std::cout << "Serial number unchanged :'(" << std::endl;
        return 2;
    }

    std::cout << std::format("Old serial: {}\nNew serial: {}", FormatSerial(oldSerial), FormatSerial(newSerial))
              << std::endl;

    return 0;
}