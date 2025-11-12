include_directories(
    ..                  
    ../core/entities    
    ../core/value_objects
)

set(CORE_SOURCES
    ../core/entities/Node.cpp
    ../core/entities/Edge.cpp
    ../core/entities/Graph.cpp
)

set(TEST_SOURCES
    GraphTest.cpp
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