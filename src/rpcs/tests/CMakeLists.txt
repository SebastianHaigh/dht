add_executable(MessageTests MessageTests.cpp)
target_link_libraries(MessageTests PRIVATE Catch2::Catch2WithMain RPCs)
add_test(NAME MessageTests
         COMMAND MessageTests)
