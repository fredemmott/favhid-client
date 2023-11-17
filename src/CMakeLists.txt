add_definitions(
    -DUNICODE
)

add_subdirectory(lib)

add_executable(randomize-serial-number randomize-serial-number.cpp)
target_link_libraries(randomize-serial-number PRIVATE favhid)

add_executable(generate-random-ids generate-random-ids.cpp)
target_link_libraries(generate-random-ids PRIVATE favhid)

add_executable(test-favjoystate2 test-favjoystate2.cpp)
target_link_libraries(test-favjoystate2 PRIVATE favhid)

add_executable(test-raw test-raw.cpp)
target_link_libraries(test-raw PRIVATE favhid)
