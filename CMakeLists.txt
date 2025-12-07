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
        # UI
	      src/main.cpp
        src/ui/mainwindow.cpp
        src/ui/mainwindow.h
        src/ui/mainwindow.ui
	      src/ui/ControlPanel.h
    	  src/ui/ControlPanel.cpp
    	  src/ui/ResultsPanel.h
   	    src/ui/ResultsPanel.cpp

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
        src/algorithms/pathfinding/AStarAlgorithm.h
        src/algorithms/pathfinding/AStarAlgorithm.cpp
        
        # Algorithms - TSP
        src/algorithms/tsp/TspMatrix.h
        src/algorithms/tsp/TspMatrix.cpp
        src/algorithms/tsp/IGAlgorithm.h
        src/algorithms/tsp/IGAlgorithm.cpp
        src/algorithms/tsp/IGNAlgorithm.h
        src/algorithms/tsp/IGNAlgorithm.cpp
        src/algorithms/tsp/ILSBAlgorithm.h
        src/algorithms/tsp/ILSBAlgorithm.cpp
        
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
target_include_directories(oep PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Copiar carpeta data al directorio de build
add_custom_command(TARGET oep POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:oep>/data
    COMMENT "Copiando carpeta data al directorio de ejecución"
)

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

set(TEST_RUNNER_SOURCES
    tests/unit/TestRunner.cpp
    
    src/core/entities/Node.cpp
    src/core/entities/Edge.cpp
    src/core/entities/Graph.cpp
    src/infraestructure/loaders/BinaryGraphLoader.cpp
    src/services/PathfindingService.cpp
    src/services/TspService.cpp
    src/algorithms/VehicleProfile.cpp
    src/algorithms/pathfinding/DijkstraAlgorithm.cpp
    src/algorithms/tsp/IGAlgorithm.cpp
    src/algorithms/tsp/TspMatrix.cpp
    src/algorithms/factories/VehicleProfileFactory.cpp
    src/algorithms/factories/AlgorithmFactory.cpp
    src/algorithms/factories/TspAlgorithmFactory.cpp
 )

 if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(test_runner
        MANUAL_FINALIZATION
        ${TEST_RUNNER_SOURCES}
    )
 else()
    add_executable(test_runner ${TEST_RUNNER_SOURCES})
 endif()

 target_include_directories(test_runner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
 target_include_directories(test_runner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/core/entities)
 target_link_libraries(test_runner PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

 if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(test_runner)
 endif()

 add_custom_command(TARGET test_runner POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:test_runner>/data
    COMMENT "Copiando carpeta data para test_runner"
 )

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(oep)
endif()
