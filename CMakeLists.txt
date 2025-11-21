cmake_minimum_required(VERSION 3.16)

project(RoutingApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON) # For signals and slots
set(CMAKE_AUTORCC ON) # For resources (icons, images)
set(CMAKE_AUTOUIC ON) # For .ui files (graphical interfaces)

# Find the Qt libraries installed on your PC
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Declare your source files (THIS IS WHERE YOU ADD YOUR CLASSES)
set(PROJECT_SOURCES
    # Core entities
    src/core/entities/Node.h
    src/core/entities/Node.cpp
    src/core/entities/Edge.h
    src/core/entities/Edge.cpp
    src/core/entities/Graph.h
    src/core/entities/Graph.cpp
    
    # Value objects
    src/core/value_objects/Coordinate.h
    src/core/value_objects/Distance.h
    src/core/value_objects/RouteSegment.h
    
    # Algorithms - VehicleProfile
    src/algorithms/VehicleProfile.h
    src/algorithms/VehicleProfile.cpp
    
    # Factories
    src/algorithms/factories/VehicleProfileFactory.h
    src/algorithms/factories/VehicleProfileFactory.cpp
)

# Create the executable
add_executable(RoutingApp ${PROJECT_SOURCES})

# Define the project root path as a macro to use in the code
target_compile_definitions(RoutingApp PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}")

# Link the Qt libraries to your executable
target_link_libraries(RoutingApp PRIVATE Qt6::Widgets)