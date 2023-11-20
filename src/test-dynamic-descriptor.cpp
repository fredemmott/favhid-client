// Copyright 2023 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: ISC

#include "favhid/descriptors.hpp"

#include <iostream>

int main() {
  using namespace FAVHID::Descriptors;
  constexpr auto REPORT_ID = 3;

  constexpr Descriptor constantDescriptor {
    UsagePage::GenericDesktop,
    Usage::Joystick,
    Collection::Application {
      Collection::Physical {
        ReportID {REPORT_ID},
        UsagePage::GenericDesktop,
        Usage::X,
        Usage::Y,
        LogicalMinimum<int8_t> {},
        LogicalMaximum<int8_t> {},
        ReportSize {8},
        ReportCount {2},
        Input::DataVariableAbsolute,
      },
    },
  };

  auto force_runtime_eval = [](auto&& x) {
    // Pointer stuff is banned in constant evaluation
    return (&x + 123)[-123];
  };

  Dynamic::Descriptor dynamicDescriptor {
    UsagePage::GenericDesktop,
    Usage::Joystick,
  };

  {
    auto physColl = Dynamic::Collection::Physical {
      ReportID {force_runtime_eval(REPORT_ID)},
      UsagePage::UsagePage(
        force_runtime_eval(UsagePage::GenericDesktop.data()[1])),
    };
    physColl.append(Usage::Usage(force_runtime_eval(Usage::X.data()[1])), Usage::Usage(force_runtime_eval(Usage::Y.data()[1])));
    physColl.append(
      LogicalMinimum(force_runtime_eval(std::numeric_limits<int8_t>::min())),
      LogicalMaximum(force_runtime_eval(std::numeric_limits<int8_t>::max())),
      ReportSize { force_runtime_eval(8) },
      ReportCount { force_runtime_eval(2) },
      Input::DataVariableAbsolute
    );
    dynamicDescriptor.append(Dynamic::Collection::Application(physColl));
  }

  assert(constantDescriptor.size() == dynamicDescriptor.size());
  assert(
    memcmp(
      constantDescriptor.data(),
      dynamicDescriptor.data(),
      constantDescriptor.size())
    == 0);
  return 0;
}