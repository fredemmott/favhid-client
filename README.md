# FAVHID-Client

Fred's Arduino Virtual HID (FAVHID) is a sketch (program) for the Arduino Micro that accepts USB HID descriptors and reports over the USB-serial interface, and reflects them as actual HID devices and reports.

This is a C++ client library for FAVHID ; you can find the arduino sketch (firmware) in the [FAVHID repository](https://github.com/fredemmott/favhid).

## Examples

- Simple mode: [src/test-favjoystate2-report.cpp](src/test-favjoystate2-report.cpp)
- DIJOYSTATE2 mode: [src/test-favjoystate2-dinput-style.cpp](src/test-favjoystate2-dinput-style.cpp)
- Low-level with custom HID descriptor: [src/test-raw.cpp](src/test-raw.cpp)

## Contents

- `descriptors.hpp` is a small library for creating HID descriptors; they can be dynamic or compile-time evaluated.
- `Arduino.hpp` is the main client library; this allows full control over FAVHID, including using your own HID reports and descriptors.
- `FAVJoyState2.hpp` is a convenience library, allowing you to create up to 8 virtual joysticks that implement most of the familiar `DIJOYSTATE2` struct. You can either provide a raw report, or a `DIJOYSTATE2` structure.

Two utilities are also included:

- `generate-random-ids`: creates unique IDs to represent a specific configuration of the HID device; this is not needed when using `FAVJoyState2`
- `randomize-serial-number`: assigns a new, random UniqueID to the Arduino as a FAVHID serial number. This allows distinguishing between multiple Arduino that are running FAVHID, allowing more virtual devices. It is not neceessary, but is supported both with and without `FAVJoyState2`

## License

Copyright (c) 2023 Fred Emmott.

Permission to use, copy, modify, and/or distribute this software for any purpose
with or without fee is hereby granted, provided that the above copyright notice
and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED “AS IS” AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.