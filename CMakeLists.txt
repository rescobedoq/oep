cmake_minimum_required(VERSION 3.16)

project(RoutingApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON) # For signals and slots
set(CMAKE_AUTORCC ON) # For resources (icons, images)
set(CMAKE_AUTOUIC ON) # For .ui files (graphical interfaces)

# Find the Qt libraries installed on your PC
find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent)

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

    # Services
    src/services/GraphService.h
    src/services/GraphService.cpp
    src/services/PathfindingService.h
    src/services/PathfindingService.cpp
    src/services/TspService.h
    src/services/TspService.cpp
    
    # Infrastructure - Loaders
    src/infraestructure/loaders/OSMGraphLoader.h
    src/infraestructure/loaders/OSMGraphLoader.cpp
    src/infraestructure/loaders/BinaryGraphSerializer.h 
    src/infraestructure/loaders/BinaryGraphSerializer.cpp
    src/infraestructure/loaders/BinaryGraphLoader.h
    src/infraestructure/loaders/BinaryGraphLoader.cpp

    # Algorithms - VehicleProfile
    src/algorithms/VehicleProfile.h
    src/algorithms/VehicleProfile.cpp
    
    # Algorithms - Pathfinding
    src/algorithms/pathfinding/DijkstraAlgorithm.h
    src/algorithms/pathfinding/DijkstraAlgorithm.cpp
    
    # Algorithms - TSP
    src/algorithms/tsp/TspMatrix.h
    src/algorithms/tsp/TspMatrix.cpp
    src/algorithms/tsp/IGAlgorithm.h
    src/algorithms/tsp/IGAlgorithm.cpp
    
    # Factories
    src/algorithms/factories/AlgorithmFactory.h
    src/algorithms/factories/AlgorithmFactory.cpp
    src/algorithms/factories/TspAlgorithmFactory.h
    src/algorithms/factories/TspAlgorithmFactory.cpp
    src/algorithms/factories/VehicleProfileFactory.h
    src/algorithms/factories/VehicleProfileFactory.cpp
)

# Create the executable
add_executable(RoutingApp ${PROJECT_SOURCES})

# Define the project root path as a macro to use in the code
target_compile_definitions(RoutingApp PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}")

# Link the Qt libraries to your executable
target_link_libraries(RoutingApp PRIVATE Qt6::Widgets Qt6::Concurrent)