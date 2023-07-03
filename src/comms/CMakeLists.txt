add_library(Comms STATIC Comms.cpp)
target_link_libraries(Comms PRIVATE Hashing)

enable_testing()

add_subdirectory(tests)
