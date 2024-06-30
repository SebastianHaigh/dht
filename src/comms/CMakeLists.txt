add_library(Comms STATIC Comms.cpp EncodedMessage.cpp)

target_link_libraries(Comms PRIVATE Hashing)

target_include_directories(Comms PUBLIC .)

enable_testing()

add_subdirectory(tests)
