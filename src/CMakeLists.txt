add_definitions(
    -DUNICODE
    -DNOMINMAX=1
    -DFAVHID_CLIENT=1
)

add_executable(
    easytest
    easytest.cpp
    Arduino.cpp
)

target_link_libraries(
    easytest
    PRIVATE
    OneCore # OpenCommPort
)

add_executable(
    randomize-serial-number
    randomize-serial-number.cpp
    Arduino.cpp
)

target_link_libraries(
    randomize-serial-number
    PRIVATE
    OneCore # OpenCommPort
)