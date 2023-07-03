add_executable(CommsTests CommsTests.cpp)
target_link_libraries(CommsTests PRIVATE Catch2::Catch2WithMain Comms)
add_test(NAME CommsTests
         COMMAND CommsTests)
