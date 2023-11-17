// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#include "favhid/Arduino.hpp"

#include <winrt/base.h>

#include <iostream>

using namespace FAVHID;

int main()
{
    auto maybeArduino = Arduino::Open();
    if (!maybeArduino)
    {
        std::cout << "Failed to find a compatible arduino :'(" << std::endl;
        return 1;
    }
    
    auto& a = *maybeArduino;

    const auto oldSerial = a.GetSerialNumber();
    a.RandomizeSerialNumber();
    const auto newSerial = a.GetSerialNumber();

    if (oldSerial == newSerial)
    {
        std::cout << "Serial number unchanged :'(" << std::endl;
        return 2;
    }

    std::cout << std::format("Old serial: {}\nNew serial: {}", oldSerial.HumanReadable(), newSerial.HumanReadable())
              << std::endl;

    return 0;
}