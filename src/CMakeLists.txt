add_definitions(
    -DUNICODE
)

add_library(
    favhid
    Arduino.cpp
    OpaqueiD.cpp
)
target_compile_definitions(
    favhid
    PUBLIC
    -DNOMINMAX=1
    -DFAVHID_CLIENT=1
)
target_link_libraries(
    favhid
    PRIVATE
    OneCore # OpenCommPort
)
target_include_directories(
    favhid
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_executable(easytest easytest.cpp)
target_link_libraries(easytest PRIVATE favhid)

add_executable(randomize-serial-number randomize-serial-number.cpp)
target_link_libraries(randomize-serial-number PRIVATE favhid)

add_executable(generate-random-ids generate-random-ids.cpp)
target_link_libraries(generate-random-ids PRIVATE favhid)