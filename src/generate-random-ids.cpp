// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#include "favhid/protocol.hpp"

#include <iostream>

using namespace FAVHID;

int main() {
    for (int i = 0; i < 10; ++i) {
        std::cout << OpaqueID::Random().HumanReadable() << std::endl;
    }
    return 0;
}