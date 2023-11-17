add_executable(
    easytest
    easytest.cpp
    io.cpp
)

target_link_libraries(
    easytest
    PRIVATE
    OneCore # OpenCommPort
)

add_executable(
    randomize-serial-number
    randomize-serial-number.cpp
    io.cpp
)

target_link_libraries(
    randomize-serial-number
    PRIVATE
    OneCore # OpenCommPort
)