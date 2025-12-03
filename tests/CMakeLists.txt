include_directories(
    ..                  
    ../src              # allow #include "algorithms/..." from tests
    ../core/entities    
    ../core/value_objects
)

# Download and make GoogleTest available if not provided by the system
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.14.0
)
FetchContent_MakeAvailable(googletest)

set(CORE_SOURCES
    ../core/entities/Node.cpp
    ../core/entities/Edge.cpp
    ../core/entities/Graph.cpp
)

set(TEST_SOURCES
    unit/GraphTests.cpp
    unit/IGNAlgorithmTest.cpp
)

add_executable(
    run_tests 
    ${TEST_SOURCES}
    ../core/entities/Node.cpp
    ../core/entities/Edge.cpp
    ../core/entities/Graph.cpp
)

target_link_libraries(
    run_tests
    GTest::gtest_main 
)

include(GoogleTest)
gtest_discover_tests(run_tests)