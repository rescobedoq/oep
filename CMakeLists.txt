cmake_minimum_required(VERSION 3.16)

project(oep VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        src/main.cpp
        src/ui/mainwindow.cpp
        src/ui/mainwindow.h
        src/ui/mainwindow.ui

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

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(oep
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        src/ui/MapWidget.h
        src/ui/MapWidget.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET oep APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(oep SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(oep
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(oep PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.oep)
endif()
set_target_properties(oep PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS oep
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(oep)
endif()
