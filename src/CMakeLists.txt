add_definitions(
    -DUNICODE
)

add_subdirectory(lib)

add_executable(randomize-serial-number randomize-serial-number.cpp)
target_link_libraries(randomize-serial-number PRIVATE favhid)

add_executable(generate-random-ids generate-random-ids.cpp)
target_link_libraries(generate-random-ids PRIVATE favhid)

add_executable(test-favjoystate2-dinput-style test-favjoystate2-dinput-style.cpp)
target_link_libraries(test-favjoystate2-dinput-style PRIVATE favhid)

add_executable(test-favjoystate2-report test-favjoystate2-report.cpp)
target_link_libraries(test-favjoystate2-report PRIVATE favhid)

add_executable(test-raw test-raw.cpp)
target_link_libraries(test-raw PRIVATE favhid)

add_executable(test-dynamic-descriptor test-dynamic-descriptor.cpp)
target_link_libraries(test-dynamic-descriptor PRIVATE favhid-headers)