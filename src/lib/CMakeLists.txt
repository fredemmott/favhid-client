add_library(favhid-headers INTERFACE)
target_compile_definitions(
    favhid-headers
    INTERFACE
    -DNOMINMAX=1
    -DFAVHID_CLIENT=1
)
target_include_directories(
    favhid-headers
    INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

add_library(
    favhid
    Arduino.cpp
    FAVJoyState2.cpp
    OpaqueID.cpp
)
target_link_libraries(
    favhid
    PUBLIC
    favhid-headers
    PRIVATE
    OneCore # OpenCommPort
)