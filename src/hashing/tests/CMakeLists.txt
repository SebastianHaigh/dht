add_executable(HashingTests HashingTests.cpp)
target_link_libraries(HashingTests PRIVATE Catch2::Catch2WithMain Hashing)
add_test(NAME HashingTests
         COMMAND HashingTests)
