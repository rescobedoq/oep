<<<<<<< HEAD
# ROUTING APP C++ + Qt6

**Proyecto:** Sistema de rutas optimizadas y TSP con interfaz gráfica interactiva

## Integrantes:
- Coaquira Suyo Gabriela Dayana
- Nina Calizaya Rafael Diego
- Quispe Saavedra Dennis Javier
- Venero Guevara Christian Henry
- Villafuerte Quispe Alexander

---

## TABLA DE CONTENIDO

1. [Visión General](#1-visión-general)
2. [Arquitectura del Sistema](#2-arquitectura-del-sistema)
3. [Diagramas](#3-diagramas)
4. [Flujos Principales](#4-flujos-principales)
5. [Stack Tecnológico](#5-stack-tecnológico)
6. [Requisitos y Cumplimiento](#6-requisitos-y-cumplimiento)
7. [Estructura de Carpetas](#7-estructura-de-carpetas)
---

## 1. VISIÓN GENERAL

### 1.1 ¿Qué es esta aplicación?

Una **aplicación de escritorio multiplataforma** (Windows, macOS, Linux) que:

- **Carga grafos viales** desde archivos binarios (formato OSM procesado)       // Para carga rapida
- **Calcula rutas óptimas** entre dos puntos usando Dijkstra, A* o ALT
- **Resuelve TSP** (Traveling Salesman Problem) para múltiples waypoints con metaheurísticas (IG, IGSA, ILS)
- **Visualiza resultados** en un mapa interactivo renderizado con Qt Graphics View
- **Compara algoritmos** mostrando métricas de rendimiento en tiempo real
- **Exporta resultados** a JSON, CSV o imágenes PNG         // Opcional

### 1.2 Objetivos del proyecto

**Funcionales:**
- Cargar grafos de hasta 50k+ nodos con bajo consumo de memoria      // En nuestro caso me parece que son 30k nodos y 60k aristas
- Calcular rutas en < 100ms para grafos urbanos típicos (50k nodos)
- Resolver TSP de 10-20 waypoints en < 5 segundos       // Opcional - talves llegue a demorar mas tiempo segun nodos
- UI responsiva (60 FPS) con zoom/pan suave             // Opcional - depende de que tan pesados sea el mapa generado
- Exportar reportes de métricas y rutas                 // Opcional - solo si hay tiempo

**No funcionales:**
- **Memoria:** < 500 MB RAM para grafo de 50k nodos
- **Portabilidad:** Compilar en Windows/macOS/Linux sin cambios
- **Mantenibilidad:** Arquitectura limpia con < 15% duplicación de código
- **Testeo:** > 80% cobertura en lógica crítica (algoritmos, servicios)

---

## 2. ARQUITECTURA DEL SISTEMA

### 2.1 Arquitectura de Capas (Clean Architecture)

```
┌──────────────────────────────────────────────────────────────┐
│                  PRESENTATION LAYER (Qt)                     │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐        │
│  │ MainWindow   │  │ MapWidget    │  │ ControlPanel │        │
│  │ (Qt Widgets) │  │ (QGraphics)  │  │ (Forms)      │        │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘        │
│         │                  │                  │              │
│         └──────────────────┼──────────────────┘              │
│                            │                                 │
├────────────────────────────┼─────────────────────────────────┤
│                    APPLICATION LAYER                         │
│         ┌──────────────────▼──────────────────┐              │
│         │  Controllers (Qt Slots)             │              │
│         │  • RouteController                  │              │
│         │  • TspController                    │              │
│         │  • MapController                    │              │
│         └──────────────┬──────────────────────┘              │
│                        │                                     │
├────────────────────────┼─────────────────────────────────────┤
│                   BUSINESS LOGIC LAYER                       │
│         ┌──────────────▼──────────────────┐                  │
│         │  Services                       │                  │
│         │  • PathfindingService           │                  │
│         │  • TspService                   │                  │
│         │  • GraphService                 │                  │
│         └──────────────┬──────────────────┘                  │
│                        │                                     │
│         ┌──────────────▼──────────────────┐                  │
│         │  Algorithms                     │                  │
│         │  • Dijkstra, A*, ALT            │                  │
│         │  • IG, IGSA, ILS (TSP)          │                  │
│         │  • TspMatrix                    │                  │
│         └──────────────┬──────────────────┘                  │
│                        │                                     │
├────────────────────────┼─────────────────────────────────────┤
│                      DOMAIN LAYER                            │
│         ┌──────────────▼──────────────────┐                  │
│         │  Core Entities                  │                  │
│         │  • Graph, Node, Edge            │                  │
│         │  • Route, RouteSegment          │                  │
│         │  • VehicleProfile               │                  │
│         └──────────────┬──────────────────┘                  │
│                        │                                     │
├────────────────────────┼─────────────────────────────────────┤
│                  INFRASTRUCTURE LAYER                        │
│         ┌──────────────▼──────────────────┐                  │
│         │  Persistence                    │                  │
│         │  • BinaryGraphLoader            │                  │
│         │  • GraphRepository              │                  │
│         └─────────────────────────────────┘                  │
│         ┌───────────────────────────────┐                    │
│         │  Utils                        │                    │
│         │  • Configuration, Logger(No)  │                    │
│         │  • ThreadPool, Async(No)      │                    │
│         └───────────────────────────────┘                    │
└──────────────────────────────────────────────────────────────┘
```

### 2.2 Patrón MVC Detallado

```
┌────────────────────────────────────────────────────────────┐
│                         VIEW (Qt)                          │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ MainWindow.ui (Qt Designer)                          │  │
│  │  • MenuBar (File, Edit, Algorithms, View, Help)      │  │  // Integrar en caso alcance el tiempo
│  │  • Toolbar (Quick actions)                           │  │  // Inegrar en caso alcance el tiempo
│  │  • Central Widget:                                   │  │
│  │    ┌───────────────┬──────────────────────────────┐  │  │
│  │    │               │                              │  │  │
│  │    │  MapWidget    │  ControlPanel                │  │  │
│  │    │ (QGraphics    │  • Algorithm selector        │  │  │  // Como: Dijkstra, A*, ALT
│  │    │  Scene)       │  • Heuristic selector (TSP)  │  │  │  // Como: IG, IGSA, ILS
│  │    │               │  • Vehicle profile           │  │  │  // Como: Automovil, Peaton
│  │    │  [Mapa con    │  • Start node                │  │  │
│  │    │   zoom/pan]   │  • Destiny node/s            │  │  │
│  │    │               │  • [Calculate] button        │  │  │
│  │    │               │  • Results panel             │  │  │
│  │    │               │  • Back to start (checkbox)  │  │  │  // Volver al inicio o no volver al inicio al generar la ruta TSP
│  │    └───────────────┴──────────────────────────────┘  │  │
│  │  • StatusBar (progress, stats)                       │  │
│  └──────────────────────────────────────────────────────┘  │
│                          ▲                                 │
│                          │ Qt Signals                      │
│                          │                                 │
├──────────────────────────┼─────────────────────────────────┤
│                     CONTROLLER                             │
│  ┌───────────────────────▼──────────────────────────────┐  │
│  │ MainWindowController (Qt Slots)                      │  │
│  │                                                      │  │
│  │  void onCalculateRoute()                             │  │  // Shortest path
│  │  void onSolveTsp()                                   │  │  // TSP
│  │  void onMapClicked(QPointF pos)                      │  │  // Click nodes in the map
│  │  void onAlgorithmChanged(QString algo)               │  │  
│  │                                                      │  │
│  │  // Delegación a servicios:                          │  │
│  │  PathfindingService* pathfindingService_;            │  │
│  │  TspService* tspService_;                            │  │
│  └───────────────────────┬──────────────────────────────┘  │
│                          │                                 │
│                          │ Llama a                         │
│                          │                                 │
├──────────────────────────┼─────────────────────────────────┤
│                       MODEL                                │
│  ┌───────────────────────▼──────────────────────────────┐  │
│  │ Services (Business Logic)                            │  │
│  │                                                      │  │
│  │  PathfindingService::findPath(origin, dest, algo)    │  │  // Resolver Shortest Path
│  │    └─> AlgorithmFactory::create(algo)                │  │
│  │        └─> Dijkstra/AStar/ALT::findPath()            │  │
│  │                                                      │  │
│  │  TspService::solve(waypoints, algo)                  │  │  // Resolver TSP
│  │    └─> TspMatrix::precompute() ***                   │  │
│  │        └─> TspAlgorithm::solve()                     │  │
│  │                                                      │  │
│  │  GraphService::loadGraph(path)                       │  │  // Carga del mapa
│  │    └─> GraphRepository::load()                       │  │
│  │        └─> BinaryGraphLoader::load()                 │  │
│  └───────────────────────┬──────────────────────────────┘  │
│                          │                                 │
│                          │ Modifica                        │
│                          ▼                                 │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ Core Entities (Domain Model)                         │  │
│  │  • Graph (nodes_, edges_, adjacencyList_)            │  │
│  │  • Route (segments_, totalDistance_)                 │  │
│  │  • VehicleProfile (restrictions, speeds)             │  │
│  └──────────────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────────┘
```

---

## 3. DIAGRAMAS

### 3.1 Diagrama de Clases (Core + Algoritmos)

```
┌────────────────────────────────────────────────────────────────┐
│                      <<interface>>                             │
│                  IPathfindingAlgorithm                         │
│  ──────────────────────────────────────────────────────────    │
│  + findPath(graph, start, goal): vector<int64_t>               │
│  + getName(): string                                           │
│  + getNodesExplored(): size_t                                  │
└────────────────┬───────────────────────────────────────────────┘
                 │
       ┌─────────┼─────────┬─────────────────┐
       │         │         │                 │
       ▼         ▼         ▼                 ▼
┌─────────┐ ┌─────────┐ ┌─────────┐    ┌──────────────┐
│Dijkstra │ │ AStar   │ │  ALT    │    │ <<interface>>│
│Algorithm│ │Algorithm│ │Algorithm│    │ITspAlgorithm │
└─────────┘ └─────────┘ └─────────┘    └─────┬────────┘
                                             │
                                  ┌──────────┼──────────┐
                                  │          │          │
                                  ▼          ▼          ▼
                            ┌─────────┐ ┌─────────┐ ┌─────────┐
                            │   IG    │ │  IGSA   │ │  ILS    │        // Y otros si en caso hubiese
                            │Algorithm│ │Algorithm│ │Algorithm│
                            └─────────┘ └─────────┘ └─────────┘

┌────────────────────────────────────────────────────────────────┐
│                          Graph                                 │
│  ────────────────────────────────────────────────────────────  │
│  - nodes_: unordered_map<int64_t, Node>                        │
│  - edges_: unordered_map<int64_t, Edge>                        │
│  - adjacencyList_: unordered_map<int64_t, vector<int64_t>>     │
│  ────────────────────────────────────────────────────────────  │
│  + addNode(node: Node)                                         │
│  + addEdge(edge: Edge)                                         │
│  + getNode(id): optional<Node>                                 │
│  + getOutgoingEdges(nodeId): vector<int64_t>                   │
│  + buildAdjacencyList()                                        │ 
└──┬───────────────────────────────────────────────────┬─────────┘
   │ contains                                          │ contains
   │                                                   │
   ▼                                                   ▼
┌──────────────┐                              ┌──────────────┐
│     Node     │                              │     Edge     │
│──────────────│                              │──────────────│
│- id_         │                              │- id_         │
│- coordinate_ │                              │- fromNodeId_ │
│- tags_       │                              │- toNodeId_   │
│──────────────│                              │- distance_   │
│+ getId()     │                              │- isOneway_   │
│+ getCoord()  │                              │- tags_       │
└──────────────┘                              │──────────────│
                                              │+ getWeight() │
                                              │+ isAccessible│
                                              └──────────────┘

┌────────────────────────────────────────────────────────────────┐
│                        TspMatrix ***                           │
│  ────────────────────────────────────────────────────────────  │
│  - size_: size_t                                               │
│  - nodeIds_: vector<int64_t>                                   │
│  - matrix_: vector<vector<double>>                             │
│  ────────────────────────────────────────────────────────────  │
│  + precompute(graph, algorithm)          // O(N² PathCost)     │ 
│  + getDistance(i, j): double                                   │
│  + calculateTourCost(tour): double                             │
└────────────────────────────────────────────────────────────────┘
```

### 3.2 Diagrama de Secuencia: Calcular Ruta Corta

```
Usuario   MainWindow  Controller  Service   AlgoFactory  Algorithm   Graph
  │           │         │           │           │            │         │
  │ Click     │         │           │           │            │         │
  │"Calculate"│         │           │           │            │         │
  ├──────────>│         │           │           │            │         │
  │           │ Slot    │           │           │            │         │
  │           │onCalculate()        │           │            │         │
  │           ├────────>│           │           │            │         │
  │           │         │ findPath()│           │            │         │
  │           │         ├──────────>│           │            │         │
  │           │         │           │create()   │            │         │
  │           │         │           ├──────────>│            │         │
  │           │         │           │           │new Dijkstra│         │
  │           │         │           │           ├───────────>│         │
  │           │         │           │           │            │         │
  │           │         │           │<──────────┤            │         │
  │           │         │           │ algo*     │            │         │
  │           │         │           │           │            │         │
  │           │         │           │ findPath(start,goal)   │         │
  │           │         │           ├───────────────────────>│         │
  │           │         │           │           │            │getNode()│
  │           │         │           │           │            ├────────>│
  │           │         │           │           │            │<────────┤
  │           │         │           │           │            │getEdges │
  │           │         │           │           │            ├────────>│
  │           │         │           │           │            │<────────┤
  │           │         │           │           │            │ Loop    │
  │           │         │           │           │            │ (BFS)   │
  │           │         │           │<───────────────────────┤         │
  │           │         │           │ path: vector<int64_t>  │         │
  │           │         │           │           │            │         │
  │           │         │<──────────┤           │            │         │
  │           │         │ PathResult│           │            │         │
  │           │<────────┤           │           │            │         │
  │           │ Update  │           │           │            │         │
  │           │ UI      │           │           │            │         │
  │<──────────┤         │           │           │            │         │
  │  Display  │         │           │           │            │         │
  │  Route    │         │           │           │            │         │
```

### 3.3 Diagrama de Secuencia: Resolver TSP

```
Usuario   MainWindow  TspController  TspService  TspMatrix  PathService  TspAlgo
  │           │           │             │           │           │          │
  │ Click     │           │             │           │           │          │
  │"Solve TSP"│           │             │           │           │          │
  ├──────────>│           │             │           │           │          │
  │           │ onSolveTsp()            │           │           │          │
  │           ├───────────>│            │           │           │          │
  │           │            │ solve()    │           │           │          │
  │           │            ├───────────>│           │           │          │
  │           │            │            │new Matrix │           │          │
  │           │            │            ├──────────>│           │          │
  │           │            │            │           │           │          │
  │           │            │            │precompute()           │          │
  │           │            │            ├──────────────────────>│          │
  │           │            │            │           │           │          │
  │           │            │            │           │ findPath()│          │
  │           │            │            │           │ (N² veces)│          │
  │           │            │            │           ├──────────>│          │
  │           │            │            │<──────────────────────┤          │
  │           │            │            │ matrix completa       │          │
  │           │            │            │           │           │          │
  │           │            │            │createAlgo()           │          │
  │           │            │            ├─────────────────────────────────>│
  │           │            │            │           │           │  IG/IGSA/│
  │           │            │            │           │           │    ILS   │
  │           │            │            │           │           │          │
  │           │            │            │solve(matrix)          │          │
  │           │            │            ├─────────────────────────────────>│
  │           │            │            │           │           │  tour    │
  │           │            │            │<─────────────────────────────────┤
  │           │            │            │           │           │          │
  │           │            │<───────────┤           │           │          │
  │           │            │ TspResult  │           │           │          │
  │           │<───────────┤            │           │           │          │
  │           │ Update UI  │            │           │           │          │
  │<──────────┤            │            │           │           │          │
  │  Display  │            │            │           │           │          │
  │   Tour    │            │            │           │           │          │
```

### 3.4 Diagrama de Componentes Qt

```
┌────────────────────────────────────────────────────────────────┐
│                       QApplication                             │
│                       (main.cpp)                               │
└────────────────┬───────────────────────────────────────────────┘
                 │
                 │ creates
                 ▼
┌────────────────────────────────────────────────────────────────┐
│                      MainWindow                                │
│  ────────────────────────────────────────────────────────────  │
│  [QMainWindow]                                                 │
│  • QMenuBar                                                    │   // Opcional
│  • QToolBar                                                    │   // Opcional
│  • QStatusBar                                                  │
│  • Central Widget: QSplitter                                   │
│    ├─ MapWidget (left)                                         │
│    └─ ControlPanel (right)                                     │
│  ────────────────────────────────────────────────────────────  │
│  Signals:                                                      │
│    signal routeCalculated(Route route)                         │
│    signal tspSolved(TspResult result)                          │
│  ────────────────────────────────────────────────────────────  │
│  Slots:                                                        │
│    slot onCalculateRoute()                                     │
│    slot onSolveTsp()                                           │ 
│    slot onLoadGraph(QString path)                              │
└──┬────────────────────────────┬───────────────────────────┬────┘
   │ contains                   │ contains                  │
   │                            │                           │
   ▼                            ▼                           ▼
┌─────────────────┐  ┌──────────────────┐  ┌────────────────────┐
│   MapWidget     │  │  ControlPanel    │  │  ProgressDialog    │
│ [QGraphicsView] │  │  [QWidget]       │  │  [QProgressDialog] │
│─────────────────│  │──────────────────│  │────────────────────│
│- scene_:        │  │- algoCombo_      │  │- taskName_         │
│  QGraphics      │  │- heuriCombo_     │  │- progress_         │
│  Scene*         │  │- vehicleCombo_   │  │────────────────────│
│─────────────────│  │- startNode_      │  │+ setProgress(int)  │
│+ drawRoute()    │  │- destNode_       │  │+ setStatus(str)    │
│+ drawMarker()   │  │- calculateBtn_   │  └────────────────────┘
│+ clearMap()     │  │- resultsPanel_   │
│+ zoomIn()       │  │- backCheck_      │
│+ zoomOut()      │  │──────────────────│
│+ pan(delta)     │  │ Signals:         │
└─────────────────┘  │  calculateClicked│
                     │  waypointsChanged│
                     └──────────────────┘

Conexiones Qt (Signals/Slots):
───────────────────────────────
ControlPanel::calculateClicked() 
    → MainWindow::onCalculateRoute()
    → PathfindingService::findPathAsync()
    → AsyncTaskExecutor::submit() [C++ thread]
    → [Computation in background...]
    → PathfindingService::pathFound(Route) [Qt signal]
    → MainWindow::onRouteReady(Route)
    → MapWidget::drawRoute(Route)
```

---

## 4. FLUJOS PRINCIPALES

### 4.1 Flujo: Cargar Grafo

```
START
  │
  ├─> Carga y lectura del OSM       // Si se llegara a trabajar el menu: Usuario selecciona "File > Open Graph..." 
  │
  ├─> MainWindow::onLoadGraph(path)
  │     │
  │     ├─> Mostrar ProgressDialog ("Loading graph...")
  │     │
  │     ├─> GraphService::loadGraphAsync(path)
  │     │     │
  │     │     ├─> AsyncTaskExecutor::submit([path]() {
  │     │     │     BinaryGraphLoader loader;
  │     │     │     Graph* graph = loader.load(path);  // Big-endian read
  │     │     │     return graph;
  │     │     │   })
  │     │     │
  │     │     └─> [Background thread ejecuta load()]
  │     │           │
  │     │           ├─> Leer header OSMGRAPH (128 bytes)
  │     │           ├─> Leer string table
  │     │           ├─> Leer nodes (lat, lon, id)
  │     │           ├─> Leer edges (from, to, distance, tags)
  │     │           └─> buildAdjacencyList()
  │     │
  │     └─> Signal: graphLoaded(Graph*)
  │           │
  │           └─> MainWindow::onGraphReady(Graph*)
  │                 │
  │                 ├─> Ocultar ProgressDialog
  │                 ├─> MapWidget::setGraph(graph)
  │                 ├─> ControlPanel::enableControls()
  │                 └─> StatusBar: "Loaded 50,000 nodes, 120,000 edges"
  │
END
```

### 4.2 Flujo: Calcular Ruta Más Corta

```
START
  │
  ├─> Usuario ingresa origin ID: 12345
  ├─> Usuario ingresa destination ID: 67890
  ├─> Usuario selecciona algoritmo: "Dijkstra"
  ├─> Usuario selecciona perfil: "Automovil"
  ├─> Click "Calculate Shortest Path"
  │
  ├─> MainWindow::onCalculateRoute()
  │     │
  │     ├─> Validar inputs (InputValidator::validate())
  │     │     │
  │     │     ├─> ¿Origin existe en graph? → NO → Mostrar error
  │     │     └─> ¿Dest existe en graph?   → NO → Mostrar error
  │     │
  │     ├─> Mostrar ProgressDialog ("Calculating route...")
  │     │
  │     ├─> PathfindingService::findPathAsync(
  │     │       origin=12345,
  │     │       dest=67890,
  │     │       algo="dijkstra",
  │     │       profile="car"
  │     │     )
  │     │       │
  │     │       ├─> AlgorithmFactory::create("dijkstra")
  │     │       │     → new DijkstraAlgorithm(VehicleProfile::car())
  │     │       │
  │     │       ├─> AsyncTaskExecutor::submit([=]() {
  │     │       │     auto path = dijkstra->findPath(graph, 12345, 67890);
  │     │       │     Route route = buildRoute(path, graph);
  │     │       │     return route;
  │     │       │   })
  │     │       │
  │     │       └─> [Thread ejecuta Dijkstra]
  │     │             │
  │     │             ├─> Priority queue (min-heap)
  │     │             ├─> Explorar vecinos (expandir nodos)
  │     │             ├─> Actualizar distancias
  │     │             └─> Reconstruir path
  │     │                   │
  │     │                   └─> [path: {12345, 20000, 35000, ..., 67890}]
  │     │
  │     └─> Signal: pathFound(Route route)
  │           │
  │           └─> MainWindow::onRouteReady(Route route)
  │                 │
  │                 ├─> Ocultar ProgressDialog
  │                 ├─> MapWidget::drawRoute(route)
  │                 │     │
  │                 │     ├─> Dibujar línea (QGraphicsPathItem)
  │                 │     ├─> Dibujar markers (inicio/fin)
  │                 │     └─> Auto-zoom a bounding box
  │                 │
  │                 ├─> ResultsPanel::displayMetrics()
  │                 │     │
  │                 │     ├─> "Distance: 15.3 km"
  │                 │     ├─> "Time: 18 min"
  │                 │     ├─> "Nodes explored: 2,430"
  │                 │     └─> "Computation Time: 45 ms"
  │                 │
  │                 └─> StatusBar: "Route calculated (15.3 km, 18 min)"
  │
END
```

### 4.3 Flujo: Resolver TSP

```
START
  │
  ├─> Usuario añade waypoints:
  │     • Click derecho en mapa → "Add waypoint"
  │     • Lista en ControlPanel: [12345, 20000, 30000, 40000, 50000]
  │
  ├─> Usuario selecciona algoritmo TSP: "IG" (Iterated Greedy)
  │
  ├─> Click "Solve TSP"
  │
  ├─> MainWindow::onSolveTsp()
  │     │
  │     ├─> Validar: ¿waypoints >= 3? → NO → Error
  │     │
  │     ├─> Mostrar ProgressDialog ("Solving TSP...")
  │     │
  │     ├─> TspService::solveAsync(waypoints, algo="IG")
  │     │     │
  │     │     ├─> 1. Crear TspMatrix
  │     │     │     │
  │     │     │     └─> TspMatrix matrix(5, waypoints)
  │     │     │
  │     │     ├─> 2. Precomputar distancias (N² pathfinding)
  │     │     │     │
  │     │     │     ├─> Para i=0..4, j=0..4 (i≠j):
  │     │     │     │     │
  │     │     │     │     ├─> Dijkstra->findPath(waypoints[i], waypoints[j])
  │     │     │     │     │     → path: {12345, ..., 20000}
  │     │     │     │     │
  │     │     │     │     ├─> Calcular distancia total del path
  │     │     │     │     │     → sumando edge.getDistance()
  │     │     │     │     │
  │     │     │     │     └─> matrix[i][j] = totalDistance
  │     │     │     │           matrix[j][i] = totalDistance
  │     │     │     │
  │     │     │     └─> Signal: precomputeProgress(int percent)
  │     │     │           → Actualizar ProgressDialog
  │     │     │
  │     │     ├─> 3. Ejecutar algoritmo TSP
  │     │     │     │
  │     │     │     ├─> TspAlgorithmFactory::create("IG")
  │     │     │     │     → new IGAlgorithm()
  │     │     │     │
  │     │     │     ├─> AsyncTaskExecutor::submit([=]() {
  │     │     │     │     tour = igAlgo->solve(matrix);
  │     │     │     │     // tour: {0, 2, 4, 1, 3} (índices)
  │     │     │     │     return tour;
  │     │     │     │   })
  │     │     │     │
  │     │     │     └─> [Thread ejecuta IG]
  │     │     │           │
  │     │     │           ├─> Construcción greedy inicial
  │     │     │           ├─> Iteraciones de destrucción/reconstrucción
  │     │     │           ├─> Búsqueda local (2-opt, 3-opt)
  │     │     │           └─> Retornar mejor tour
  │     │     │
  │     │     └─> 4. Construir ruta completa
  │     │           │
  │     │           ├─> Para cada par consecutivo en tour:
  │     │           │     • tour[0]=0 → waypoint 12345
  │     │           │     • tour[1]=2 → waypoint 30000
  │     │           │     │
  │     │           │     └─> Recuperar path almacenado en TspMatrix
  │     │           │           o re-ejecutar findPath()
  │     │           │
  │     │           └─> Concatenar todos los paths → Route completa
  │     │
  │     └─> Signal: tspSolved(TspResult result)
  │           │
  │           └─> MainWindow::onTspReady(TspResult result)
  │                 │
  │                 ├─> Ocultar ProgressDialog
  │                 ├─> MapWidget::drawTspRoute(result.route)
  │                 │     │
  │                 │     ├─> Dibujar tour completo (líneas)
  │                 │     ├─> Numerar waypoints (1, 2, 3, ...)
  │                 │     └─> Resaltar punto inicial
  │                 │
  │                 ├─> ResultsPanel::displayTspMetrics()
  │                 │     │
  │                 │     ├─> "Tour: 1 → 3 → 5 → 2 → 4 → 1"
  │                 │     ├─> "Total distance: 42.7 km"
  │                 │     ├─> "Total time: 51 min"
  │                 │     └─> "Computation: 3.2 s"
  │                 │
  │                 └─> StatusBar: "TSP solved (42.7 km, 5 waypoints)"
  │
END
```

---

## 5. STACK TECNOLÓGICO

### 5.1 Herramientas y Librerías

| Categoría | Tecnología | Versión | Propósito |
|-----------|-----------|---------|-----------|
| **Lenguaje** | C++ | 17 | Código base |
| **GUI Framework** | Qt | 6.6+ | Interfaz gráfica |
| **Build System** | CMake | 3.20+ | Compilación multiplataforma |
| **Threading** | std::thread + Qt | - | Concurrencia |

### 5.2 Compiladores Soportados

- **Windows:** MSVC 2022, MinGW-w64
- **macOS:** Clang 14+ (Xcode)
- **Linux:** GCC 11+, Clang 14+

### 5.3 Qt Modules Utilizados

```cmake
find_package(Qt6 REQUIRED COMPONENTS
    Core        # Clases fundamentales (QObject, signals/slots)
    Widgets     # UI (QMainWindow, QPushButton, etc.)
    Gui         # Renderizado (QPixmap, QPainter)
    Concurrent  # Futuro/async Qt
    Test        # Framework de testing
)
```

---

## 6. REQUISITOS Y CUMPLIMIENTO

### 6.1 Testing (LIGERO - Solo Esencial)

**Tests Unitarios Básicos (GoogleTest):**
- `GraphTest`: Operaciones básicas de grafo (add, get, adjacency)
- `DijkstraTest`: Pathfinding simple en grafo pequeño
- `TspMatrixTest`: Construcción de matriz básica
- `BinaryGraphLoaderTest`: Lectura de archivo .bin

**NOTA:** Recomendado 4-5 archivos de tests (pueden ser mas si se da el caso), enfoque en validar funcionalidad crítica.  
**Objetivo:** Validar que compile y funcione, no cobertura >80%.

### 6.2 Patrones de Diseño

**Patrón Creacional: Factory Method**
- `AlgorithmFactory`: Crea Dijkstra/A*/ALT según string
- `TspAlgorithmFactory`: Crea IG/IGSA/ILS

**Patrón Estructural: Strategy**
- `IPathfindingAlgorithm`, `ITspAlgorithm`, `IHeuristic` (intercambiables)
- Permite cambiar algoritmos en runtime sin modificar servicios

**Bonus (integrados en Qt/C++):**
- **Observer:** Qt Signals/Slots (MainWindow escucha a Services)
- **MVC:** Separación clara View/Controller/Model

### 6.3 Multithreading Básico

#### 6.3.1 Análisis de Operaciones Pesadas

**Operaciones que pueden congelar la UI (>100ms):**

| Operación | Tiempo Estimado | Impacto en UI | Prioridad Thread |
|-----------|----------------|---------------|------------------|
| `BinaryGraphLoader::load()` | 1-30 segundos | Congela completamente | **ALTA** |
| `TspMatrix::precompute()` | 1-60 segundos | Congela completamente | **ALTA** |
| `TspAlgorithm::solve()` | 1-10 segundos | Congela completamente | **ALTA** |
| `PathfindingService::findPath()` | 100-500ms | Puede congelar | **MEDIA** |
| `MapWidget::drawGraph()` | 200-500ms | Puede congelar | **BAJA** (opcional) |

#### 6.3.5 Casos de Uso de Hilos

**OBLIGATORIOS (evitan congelamiento total):**
1. **GraphService::loadGraphAsync()** → Hilo para `BinaryGraphLoader::load()`
2. **TspService::solveAsync()** → Hilo para `TspMatrix::precompute()` + `TspAlgorithm::solve()`

**RECOMENDADOS (mejoran UX):**
3. **PathfindingService::findPathAsync()** → Hilo si pathfinding demora >200ms
4. **Progress signals** → Feedback visual durante operaciones largas (TspMatrix)

**OPCIONALES (solo si hay lag visible):**
5. **MapWidget::drawGraphAsync()** → Hilo si renderizar 30k nodos causa lag

### 6.4 Interfaz en Qt

**Widgets Principales:**
- `MainWindow`: Ventana principal con status bar
- `MapWidget`: QGraphicsView para renderizar mapa
- `ControlPanel`: **OBLIGATORIO** - Formularios de entrada (algoritmo, heurística, vehículo, nodos)
- `ResultsPanel`: Métricas y estadísticas (tiempo, distancia, nodos visitados)

**Características UI:**
- Zoom/pan en mapa
- Click para seleccionar nodos
- Dibujar rutas calculadas
- **MenuBar/Toolbar:** OPCIONAL

---

## 7. ESTRUCTURA DE CARPETAS

```
routing-app-cpp/
├── CMakeLists.txt              # Build raíz
├── README.md
│
├── src/
│   ├── main.cpp                # Entry point Qt
│   │
│   ├── core/                   # DOMAIN LAYER
│   │   ├── entities/
│   │   │   ├── Node.h / .cpp
│   │   │   ├── Edge.h / .cpp
│   │   │   └── Graph.h / .cpp
│   │   ├── value_objects/
│   │   │   ├── Coordinate.h
│   │   │   ├── Distance.h
│   │   │   └── RouteSegment.h
│   │   └── interfaces/
│   │       ├── IPathfindingAlgorithm.h
│   │       ├── ITspAlgorithm.h
│   │       └── IHeuristic.h
│   │
│   ├── algorithms/             # BUSINESS LOGIC
│   │   ├── VehicleProfile.h / .cpp
│   │   ├── pathfinding/
│   │   │   ├── DijkstraAlgorithm.h / .cpp
│   │   │   ├── AStarAlgorithm.h / .cpp
│   │   │   └── ALTAlgorithm.h / .cpp      // Opcional si alcanza tiempo
│   │   ├── tsp/
│   │   │   ├── TspMatrix.h / .cpp         *** Usar std::thread aquí
│   │   │   ├── IGAlgorithm.h / .cpp
│   │   │   ├── IGSAAlgorithm.h / .cpp     // Opcional
│   │   │   └── ILSAlgorithm.h / .cpp      // Opcional
│   │   └── factories/
│   │       ├── AlgorithmFactory.h / .cpp
│   │       └── TspAlgorithmFactory.h / .cpp
│   │
│   ├── services/               # APPLICATION LAYER
│   │   ├── PathfindingService.h / .cpp    *** Usar std::thread para async
│   │   ├── TspService.h / .cpp            *** Usar std::thread para async
│   │   └── GraphService.h / .cpp
│   │
│   ├── infrastructure/         # INFRASTRUCTURE LAYER
│   │   └── persistence/
│   │       └── BinaryGraphLoader.h / .cpp
│   │
│   ├── ui/                     # PRESENTATION LAYER (Qt)
│   │   ├── MainWindow.h / .cpp / .ui
│   │   ├── widgets/
│   │   │   ├── MapWidget.h / .cpp
│   │   │   ├── ControlPanel.h / .cpp / .ui   # OBLIGATORIO
│   │   │   └── ResultsPanel.h / .cpp / .ui   # OBLIGATORIO
│   │   └── resources/
│   │       ├── resources.qrc   # Qt Resource Collection (opcional)
│   │       └── icons/          # Opcional
│   │           └── marker.svg
│   │
│   └── utils/                  # UTILITIES
│       └── exceptions/
│           ├── BaseException.h
│           └── GraphException.h
│
├── tests/                      # TESTING LIGERO
│   ├── CMakeLists.txt
│   │
│   └── unit/                   # Solo GoogleTest básico
│       ├── GraphTest.cpp
│       ├── DijkstraTest.cpp
│       ├── TspMatrixTest.cpp
│       └── BinaryGraphLoaderTest.cpp
│
└── data/                       # Runtime data
    └── graphs/
        └── arequipa.bin
```

---
=======
# ROUTING APP C++ + Qt6

**Proyecto:** Sistema de rutas optimizadas y TSP con interfaz gráfica interactiva

## Integrantes:
- Coaquira Suyo Gabriela Dayana
- Nina Calizaya Rafael Diego
- Quispe Saavedra Dennis Javier
- Venero Guevara Christian Henry
- Villafuerte Quispe Alexander

---

## TABLA DE CONTENIDO

1. [Visión General](#1-visión-general)
2. [Arquitectura del Sistema](#2-arquitectura-del-sistema)
3. [Diagramas](#3-diagramas)
4. [Flujos Principales](#4-flujos-principales)
5. [Stack Tecnológico](#5-stack-tecnológico)
6. [Requisitos y Cumplimiento](#6-requisitos-y-cumplimiento)
7. [Estructura de Carpetas](#7-estructura-de-carpetas)
---

## 1. VISIÓN GENERAL

### 1.1 ¿Qué es esta aplicación?

Una **aplicación de escritorio multiplataforma** (Windows, macOS, Linux) que:

- **Carga grafos viales** desde archivos binarios (formato OSM procesado)       // Para carga rapida
- **Calcula rutas óptimas** entre dos puntos usando Dijkstra, A* o ALT
- **Resuelve TSP** (Traveling Salesman Problem) para múltiples waypoints con metaheurísticas (IG, IGSA, ILS)
- **Visualiza resultados** en un mapa interactivo renderizado con Qt Graphics View
- **Compara algoritmos** mostrando métricas de rendimiento en tiempo real
- **Exporta resultados** a JSON, CSV o imágenes PNG         // Opcional

### 1.2 Objetivos del proyecto

**Funcionales:**
- Cargar grafos de hasta 50k+ nodos con bajo consumo de memoria      // En nuestro caso me parece que son 30k nodos y 60k aristas
- Calcular rutas en < 100ms para grafos urbanos típicos (50k nodos)
- Resolver TSP de 10-20 waypoints en < 5 segundos       // Opcional - talves llegue a demorar mas tiempo segun nodos
- UI responsiva (60 FPS) con zoom/pan suave             // Opcional - depende de que tan pesados sea el mapa generado
- Exportar reportes de métricas y rutas                 // Opcional - solo si hay tiempo

**No funcionales:**
- **Memoria:** < 500 MB RAM para grafo de 50k nodos
- **Portabilidad:** Compilar en Windows/macOS/Linux sin cambios
- **Mantenibilidad:** Arquitectura limpia con < 15% duplicación de código
- **Testeo:** > 80% cobertura en lógica crítica (algoritmos, servicios)

---

## 2. ARQUITECTURA DEL SISTEMA

### 2.1 Arquitectura de Capas (Clean Architecture)

```
┌──────────────────────────────────────────────────────────────┐
│                  PRESENTATION LAYER (Qt)                     │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐        │
│  │ MainWindow   │  │ MapWidget    │  │ ControlPanel │        │
│  │ (Qt Widgets) │  │ (QGraphics)  │  │ (Forms)      │        │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘        │
│         │                  │                  │              │
│         └──────────────────┼──────────────────┘              │
│                            │                                 │
├────────────────────────────┼─────────────────────────────────┤
│                    APPLICATION LAYER                         │
│         ┌──────────────────▼──────────────────┐              │
│         │  Controllers (Qt Slots)             │              │
│         │  • RouteController                  │              │
│         │  • TspController                    │              │
│         │  • MapController                    │              │
│         └──────────────┬──────────────────────┘              │
│                        │                                     │
├────────────────────────┼─────────────────────────────────────┤
│                   BUSINESS LOGIC LAYER                       │
│         ┌──────────────▼──────────────────┐                  │
│         │  Services                       │                  │
│         │  • PathfindingService           │                  │
│         │  • TspService                   │                  │
│         │  • GraphService                 │                  │
│         └──────────────┬──────────────────┘                  │
│                        │                                     │
│         ┌──────────────▼──────────────────┐                  │
│         │  Algorithms                     │                  │
│         │  • Dijkstra, A*, ALT            │                  │
│         │  • IG, IGSA, ILS (TSP)          │                  │
│         │  • TspMatrix                    │                  │
│         └──────────────┬──────────────────┘                  │
│                        │                                     │
├────────────────────────┼─────────────────────────────────────┤
│                      DOMAIN LAYER                            │
│         ┌──────────────▼──────────────────┐                  │
│         │  Core Entities                  │                  │
│         │  • Graph, Node, Edge            │                  │
│         │  • Route, RouteSegment          │                  │
│         │  • VehicleProfile               │                  │
│         └──────────────┬──────────────────┘                  │
│                        │                                     │
├────────────────────────┼─────────────────────────────────────┤
│                  INFRASTRUCTURE LAYER                        │
│         ┌──────────────▼──────────────────┐                  │
│         │  Persistence                    │                  │
│         │  • BinaryGraphLoader            │                  │
│         │  • GraphRepository              │                  │
│         └─────────────────────────────────┘                  │
│         ┌───────────────────────────────┐                    │
│         │  Utils                        │                    │
│         │  • Configuration, Logger(No)  │                    │
│         │  • ThreadPool, Async(No)      │                    │
│         └───────────────────────────────┘                    │
└──────────────────────────────────────────────────────────────┘
```

### 2.2 Patrón MVC Detallado

```
┌────────────────────────────────────────────────────────────┐
│                         VIEW (Qt)                          │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ MainWindow.ui (Qt Designer)                          │  │
│  │  • MenuBar (File, Edit, Algorithms, View, Help)      │  │  // Integrar en caso alcance el tiempo
│  │  • Toolbar (Quick actions)                           │  │  // Inegrar en caso alcance el tiempo
│  │  • Central Widget:                                   │  │
│  │    ┌───────────────┬──────────────────────────────┐  │  │
│  │    │               │                              │  │  │
│  │    │  MapWidget    │  ControlPanel                │  │  │
│  │    │ (QGraphics    │  • Algorithm selector        │  │  │  // Como: Dijkstra, A*, ALT
│  │    │  Scene)       │  • Heuristic selector (TSP)  │  │  │  // Como: IG, IGSA, ILS
│  │    │               │  • Vehicle profile           │  │  │  // Como: Automovil, Peaton
│  │    │  [Mapa con    │  • Start node                │  │  │
│  │    │   zoom/pan]   │  • Destiny node/s            │  │  │
│  │    │               │  • [Calculate] button        │  │  │
│  │    │               │  • Results panel             │  │  │
│  │    │               │  • Back to start (checkbox)  │  │  │  // Volver al inicio o no volver al inicio al generar la ruta TSP
│  │    └───────────────┴──────────────────────────────┘  │  │
│  │  • StatusBar (progress, stats)                       │  │
│  └──────────────────────────────────────────────────────┘  │
│                          ▲                                 │
│                          │ Qt Signals                      │
│                          │                                 │
├──────────────────────────┼─────────────────────────────────┤
│                     CONTROLLER                             │
│  ┌───────────────────────▼──────────────────────────────┐  │
│  │ MainWindowController (Qt Slots)                      │  │
│  │                                                      │  │
│  │  void onCalculateRoute()                             │  │  // Shortest path
│  │  void onSolveTsp()                                   │  │  // TSP
│  │  void onMapClicked(QPointF pos)                      │  │  // Click nodes in the map
│  │  void onAlgorithmChanged(QString algo)               │  │  
│  │                                                      │  │
│  │  // Delegación a servicios:                          │  │
│  │  PathfindingService* pathfindingService_;            │  │
│  │  TspService* tspService_;                            │  │
│  └───────────────────────┬──────────────────────────────┘  │
│                          │                                 │
│                          │ Llama a                         │
│                          │                                 │
├──────────────────────────┼─────────────────────────────────┤
│                       MODEL                                │
│  ┌───────────────────────▼──────────────────────────────┐  │
│  │ Services (Business Logic)                            │  │
│  │                                                      │  │
│  │  PathfindingService::findPath(origin, dest, algo)    │  │  // Resolver Shortest Path
│  │    └─> AlgorithmFactory::create(algo)                │  │
│  │        └─> Dijkstra/AStar/ALT::findPath()            │  │
│  │                                                      │  │
│  │  TspService::solve(waypoints, algo)                  │  │  // Resolver TSP
│  │    └─> TspMatrix::precompute() ***                   │  │
│  │        └─> TspAlgorithm::solve()                     │  │
│  │                                                      │  │
│  │  GraphService::loadGraph(path)                       │  │  // Carga del mapa
│  │    └─> GraphRepository::load()                       │  │
│  │        └─> BinaryGraphLoader::load()                 │  │
│  └───────────────────────┬──────────────────────────────┘  │
│                          │                                 │
│                          │ Modifica                        │
│                          ▼                                 │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ Core Entities (Domain Model)                         │  │
│  │  • Graph (nodes_, edges_, adjacencyList_)            │  │
│  │  • Route (segments_, totalDistance_)                 │  │
│  │  • VehicleProfile (restrictions, speeds)             │  │
│  └──────────────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────────┘
```

---

## 3. DIAGRAMAS

### 3.1 Diagrama de Clases (Core + Algoritmos)

```
┌────────────────────────────────────────────────────────────────┐
│                      <<interface>>                             │
│                  IPathfindingAlgorithm                         │
│  ──────────────────────────────────────────────────────────    │
│  + findPath(graph, start, goal): vector<int64_t>               │
│  + getName(): string                                           │
│  + getNodesExplored(): size_t                                  │
└────────────────┬───────────────────────────────────────────────┘
                 │
       ┌─────────┼─────────┬─────────────────┐
       │         │         │                 │
       ▼         ▼         ▼                 ▼
┌─────────┐ ┌─────────┐ ┌─────────┐    ┌──────────────┐
│Dijkstra │ │ AStar   │ │  ALT    │    │ <<interface>>│
│Algorithm│ │Algorithm│ │Algorithm│    │ITspAlgorithm │
└─────────┘ └─────────┘ └─────────┘    └─────┬────────┘
                                             │
                                  ┌──────────┼──────────┐
                                  │          │          │
                                  ▼          ▼          ▼
                            ┌─────────┐ ┌─────────┐ ┌─────────┐
                            │   IG    │ │  IGSA   │ │  ILS    │        // Y otros si en caso hubiese
                            │Algorithm│ │Algorithm│ │Algorithm│
                            └─────────┘ └─────────┘ └─────────┘

┌────────────────────────────────────────────────────────────────┐
│                          Graph                                 │
│  ────────────────────────────────────────────────────────────  │
│  - nodes_: unordered_map<int64_t, Node>                        │
│  - edges_: unordered_map<int64_t, Edge>                        │
│  - adjacencyList_: unordered_map<int64_t, vector<int64_t>>     │
│  ────────────────────────────────────────────────────────────  │
│  + addNode(node: Node)                                         │
│  + addEdge(edge: Edge)                                         │
│  + getNode(id): optional<Node>                                 │
│  + getOutgoingEdges(nodeId): vector<int64_t>                   │
│  + buildAdjacencyList()                                        │ 
└──┬───────────────────────────────────────────────────┬─────────┘
   │ contains                                          │ contains
   │                                                   │
   ▼                                                   ▼
┌──────────────┐                              ┌──────────────┐
│     Node     │                              │     Edge     │
│──────────────│                              │──────────────│
│- id_         │                              │- id_         │
│- coordinate_ │                              │- fromNodeId_ │
│- tags_       │                              │- toNodeId_   │
│──────────────│                              │- distance_   │
│+ getId()     │                              │- isOneway_   │
│+ getCoord()  │                              │- tags_       │
└──────────────┘                              │──────────────│
                                              │+ getWeight() │
                                              │+ isAccessible│
                                              └──────────────┘

┌────────────────────────────────────────────────────────────────┐
│                        TspMatrix ***                           │
│  ────────────────────────────────────────────────────────────  │
│  - size_: size_t                                               │
│  - nodeIds_: vector<int64_t>                                   │
│  - matrix_: vector<vector<double>>                             │
│  ────────────────────────────────────────────────────────────  │
│  + precompute(graph, algorithm)          // O(N² PathCost)     │ 
│  + getDistance(i, j): double                                   │
│  + calculateTourCost(tour): double                             │
└────────────────────────────────────────────────────────────────┘
```

### 3.2 Diagrama de Secuencia: Calcular Ruta Corta

```
Usuario   MainWindow  Controller  Service   AlgoFactory  Algorithm   Graph
  │           │         │           │           │            │         │
  │ Click     │         │           │           │            │         │
  │"Calculate"│         │           │           │            │         │
  ├──────────>│         │           │           │            │         │
  │           │ Slot    │           │           │            │         │
  │           │onCalculate()        │           │            │         │
  │           ├────────>│           │           │            │         │
  │           │         │ findPath()│           │            │         │
  │           │         ├──────────>│           │            │         │
  │           │         │           │create()   │            │         │
  │           │         │           ├──────────>│            │         │
  │           │         │           │           │new Dijkstra│         │
  │           │         │           │           ├───────────>│         │
  │           │         │           │           │            │         │
  │           │         │           │<──────────┤            │         │
  │           │         │           │ algo*     │            │         │
  │           │         │           │           │            │         │
  │           │         │           │ findPath(start,goal)   │         │
  │           │         │           ├───────────────────────>│         │
  │           │         │           │           │            │getNode()│
  │           │         │           │           │            ├────────>│
  │           │         │           │           │            │<────────┤
  │           │         │           │           │            │getEdges │
  │           │         │           │           │            ├────────>│
  │           │         │           │           │            │<────────┤
  │           │         │           │           │            │ Loop    │
  │           │         │           │           │            │ (BFS)   │
  │           │         │           │<───────────────────────┤         │
  │           │         │           │ path: vector<int64_t>  │         │
  │           │         │           │           │            │         │
  │           │         │<──────────┤           │            │         │
  │           │         │ PathResult│           │            │         │
  │           │<────────┤           │           │            │         │
  │           │ Update  │           │           │            │         │
  │           │ UI      │           │           │            │         │
  │<──────────┤         │           │           │            │         │
  │  Display  │         │           │           │            │         │
  │  Route    │         │           │           │            │         │
```

### 3.3 Diagrama de Secuencia: Resolver TSP

```
Usuario   MainWindow  TspController  TspService  TspMatrix  PathService  TspAlgo
  │           │           │             │           │           │          │
  │ Click     │           │             │           │           │          │
  │"Solve TSP"│           │             │           │           │          │
  ├──────────>│           │             │           │           │          │
  │           │ onSolveTsp()            │           │           │          │
  │           ├───────────>│            │           │           │          │
  │           │            │ solve()    │           │           │          │
  │           │            ├───────────>│           │           │          │
  │           │            │            │new Matrix │           │          │
  │           │            │            ├──────────>│           │          │
  │           │            │            │           │           │          │
  │           │            │            │precompute()           │          │
  │           │            │            ├──────────────────────>│          │
  │           │            │            │           │           │          │
  │           │            │            │           │ findPath()│          │
  │           │            │            │           │ (N² veces)│          │
  │           │            │            │           ├──────────>│          │
  │           │            │            │<──────────────────────┤          │
  │           │            │            │ matrix completa       │          │
  │           │            │            │           │           │          │
  │           │            │            │createAlgo()           │          │
  │           │            │            ├─────────────────────────────────>│
  │           │            │            │           │           │  IG/IGSA/│
  │           │            │            │           │           │    ILS   │
  │           │            │            │           │           │          │
  │           │            │            │solve(matrix)          │          │
  │           │            │            ├─────────────────────────────────>│
  │           │            │            │           │           │  tour    │
  │           │            │            │<─────────────────────────────────┤
  │           │            │            │           │           │          │
  │           │            │<───────────┤           │           │          │
  │           │            │ TspResult  │           │           │          │
  │           │<───────────┤            │           │           │          │
  │           │ Update UI  │            │           │           │          │
  │<──────────┤            │            │           │           │          │
  │  Display  │            │            │           │           │          │
  │   Tour    │            │            │           │           │          │
```

### 3.4 Diagrama de Componentes Qt

```
┌────────────────────────────────────────────────────────────────┐
│                       QApplication                             │
│                       (main.cpp)                               │
└────────────────┬───────────────────────────────────────────────┘
                 │
                 │ creates
                 ▼
┌────────────────────────────────────────────────────────────────┐
│                      MainWindow                                │
│  ────────────────────────────────────────────────────────────  │
│  [QMainWindow]                                                 │
│  • QMenuBar                                                    │   // Opcional
│  • QToolBar                                                    │   // Opcional
│  • QStatusBar                                                  │
│  • Central Widget: QSplitter                                   │
│    ├─ MapWidget (left)                                         │
│    └─ ControlPanel (right)                                     │
│  ────────────────────────────────────────────────────────────  │
│  Signals:                                                      │
│    signal routeCalculated(Route route)                         │
│    signal tspSolved(TspResult result)                          │
│  ────────────────────────────────────────────────────────────  │
│  Slots:                                                        │
│    slot onCalculateRoute()                                     │
│    slot onSolveTsp()                                           │ 
│    slot onLoadGraph(QString path)                              │
└──┬────────────────────────────┬───────────────────────────┬────┘
   │ contains                   │ contains                  │
   │                            │                           │
   ▼                            ▼                           ▼
┌─────────────────┐  ┌──────────────────┐  ┌────────────────────┐
│   MapWidget     │  │  ControlPanel    │  │  ProgressDialog    │
│ [QGraphicsView] │  │  [QWidget]       │  │  [QProgressDialog] │
│─────────────────│  │──────────────────│  │────────────────────│
│- scene_:        │  │- algoCombo_      │  │- taskName_         │
│  QGraphics      │  │- heuriCombo_     │  │- progress_         │
│  Scene*         │  │- vehicleCombo_   │  │────────────────────│
│─────────────────│  │- startNode_      │  │+ setProgress(int)  │
│+ drawRoute()    │  │- destNode_       │  │+ setStatus(str)    │
│+ drawMarker()   │  │- calculateBtn_   │  └────────────────────┘
│+ clearMap()     │  │- resultsPanel_   │
│+ zoomIn()       │  │- backCheck_      │
│+ zoomOut()      │  │──────────────────│
│+ pan(delta)     │  │ Signals:         │
└─────────────────┘  │  calculateClicked│
                     │  waypointsChanged│
                     └──────────────────┘

Conexiones Qt (Signals/Slots):
───────────────────────────────
ControlPanel::calculateClicked() 
    → MainWindow::onCalculateRoute()
    → PathfindingService::findPathAsync()
    → AsyncTaskExecutor::submit() [C++ thread]
    → [Computation in background...]
    → PathfindingService::pathFound(Route) [Qt signal]
    → MainWindow::onRouteReady(Route)
    → MapWidget::drawRoute(Route)
```

---

## 4. FLUJOS PRINCIPALES

### 4.1 Flujo: Cargar Grafo

```
START
  │
  ├─> Carga y lectura del OSM       // Si se llegara a trabajar el menu: Usuario selecciona "File > Open Graph..." 
  │
  ├─> MainWindow::onLoadGraph(path)
  │     │
  │     ├─> Mostrar ProgressDialog ("Loading graph...")
  │     │
  │     ├─> GraphService::loadGraphAsync(path)
  │     │     │
  │     │     ├─> AsyncTaskExecutor::submit([path]() {
  │     │     │     BinaryGraphLoader loader;
  │     │     │     Graph* graph = loader.load(path);  // Big-endian read
  │     │     │     return graph;
  │     │     │   })
  │     │     │
  │     │     └─> [Background thread ejecuta load()]
  │     │           │
  │     │           ├─> Leer header OSMGRAPH (128 bytes)
  │     │           ├─> Leer string table
  │     │           ├─> Leer nodes (lat, lon, id)
  │     │           ├─> Leer edges (from, to, distance, tags)
  │     │           └─> buildAdjacencyList()
  │     │
  │     └─> Signal: graphLoaded(Graph*)
  │           │
  │           └─> MainWindow::onGraphReady(Graph*)
  │                 │
  │                 ├─> Ocultar ProgressDialog
  │                 ├─> MapWidget::setGraph(graph)
  │                 ├─> ControlPanel::enableControls()
  │                 └─> StatusBar: "Loaded 50,000 nodes, 120,000 edges"
  │
END
```

### 4.2 Flujo: Calcular Ruta Más Corta

```
START
  │
  ├─> Usuario ingresa origin ID: 12345
  ├─> Usuario ingresa destination ID: 67890
  ├─> Usuario selecciona algoritmo: "Dijkstra"
  ├─> Usuario selecciona perfil: "Automovil"
  ├─> Click "Calculate Shortest Path"
  │
  ├─> MainWindow::onCalculateRoute()
  │     │
  │     ├─> Validar inputs (InputValidator::validate())
  │     │     │
  │     │     ├─> ¿Origin existe en graph? → NO → Mostrar error
  │     │     └─> ¿Dest existe en graph?   → NO → Mostrar error
  │     │
  │     ├─> Mostrar ProgressDialog ("Calculating route...")
  │     │
  │     ├─> PathfindingService::findPathAsync(
  │     │       origin=12345,
  │     │       dest=67890,
  │     │       algo="dijkstra",
  │     │       profile="car"
  │     │     )
  │     │       │
  │     │       ├─> AlgorithmFactory::create("dijkstra")
  │     │       │     → new DijkstraAlgorithm(VehicleProfile::car())
  │     │       │
  │     │       ├─> AsyncTaskExecutor::submit([=]() {
  │     │       │     auto path = dijkstra->findPath(graph, 12345, 67890);
  │     │       │     Route route = buildRoute(path, graph);
  │     │       │     return route;
  │     │       │   })
  │     │       │
  │     │       └─> [Thread ejecuta Dijkstra]
  │     │             │
  │     │             ├─> Priority queue (min-heap)
  │     │             ├─> Explorar vecinos (expandir nodos)
  │     │             ├─> Actualizar distancias
  │     │             └─> Reconstruir path
  │     │                   │
  │     │                   └─> [path: {12345, 20000, 35000, ..., 67890}]
  │     │
  │     └─> Signal: pathFound(Route route)
  │           │
  │           └─> MainWindow::onRouteReady(Route route)
  │                 │
  │                 ├─> Ocultar ProgressDialog
  │                 ├─> MapWidget::drawRoute(route)
  │                 │     │
  │                 │     ├─> Dibujar línea (QGraphicsPathItem)
  │                 │     ├─> Dibujar markers (inicio/fin)
  │                 │     └─> Auto-zoom a bounding box
  │                 │
  │                 ├─> ResultsPanel::displayMetrics()
  │                 │     │
  │                 │     ├─> "Distance: 15.3 km"
  │                 │     ├─> "Time: 18 min"
  │                 │     ├─> "Nodes explored: 2,430"
  │                 │     └─> "Computation Time: 45 ms"
  │                 │
  │                 └─> StatusBar: "Route calculated (15.3 km, 18 min)"
  │
END
```

### 4.3 Flujo: Resolver TSP

```
START
  │
  ├─> Usuario añade waypoints:
  │     • Click derecho en mapa → "Add waypoint"
  │     • Lista en ControlPanel: [12345, 20000, 30000, 40000, 50000]
  │
  ├─> Usuario selecciona algoritmo TSP: "IG" (Iterated Greedy)
  │
  ├─> Click "Solve TSP"
  │
  ├─> MainWindow::onSolveTsp()
  │     │
  │     ├─> Validar: ¿waypoints >= 3? → NO → Error
  │     │
  │     ├─> Mostrar ProgressDialog ("Solving TSP...")
  │     │
  │     ├─> TspService::solveAsync(waypoints, algo="IG")
  │     │     │
  │     │     ├─> 1. Crear TspMatrix
  │     │     │     │
  │     │     │     └─> TspMatrix matrix(5, waypoints)
  │     │     │
  │     │     ├─> 2. Precomputar distancias (N² pathfinding)
  │     │     │     │
  │     │     │     ├─> Para i=0..4, j=0..4 (i≠j):
  │     │     │     │     │
  │     │     │     │     ├─> Dijkstra->findPath(waypoints[i], waypoints[j])
  │     │     │     │     │     → path: {12345, ..., 20000}
  │     │     │     │     │
  │     │     │     │     ├─> Calcular distancia total del path
  │     │     │     │     │     → sumando edge.getDistance()
  │     │     │     │     │
  │     │     │     │     └─> matrix[i][j] = totalDistance
  │     │     │     │           matrix[j][i] = totalDistance
  │     │     │     │
  │     │     │     └─> Signal: precomputeProgress(int percent)
  │     │     │           → Actualizar ProgressDialog
  │     │     │
  │     │     ├─> 3. Ejecutar algoritmo TSP
  │     │     │     │
  │     │     │     ├─> TspAlgorithmFactory::create("IG")
  │     │     │     │     → new IGAlgorithm()
  │     │     │     │
  │     │     │     ├─> AsyncTaskExecutor::submit([=]() {
  │     │     │     │     tour = igAlgo->solve(matrix);
  │     │     │     │     // tour: {0, 2, 4, 1, 3} (índices)
  │     │     │     │     return tour;
  │     │     │     │   })
  │     │     │     │
  │     │     │     └─> [Thread ejecuta IG]
  │     │     │           │
  │     │     │           ├─> Construcción greedy inicial
  │     │     │           ├─> Iteraciones de destrucción/reconstrucción
  │     │     │           ├─> Búsqueda local (2-opt, 3-opt)
  │     │     │           └─> Retornar mejor tour
  │     │     │
  │     │     └─> 4. Construir ruta completa
  │     │           │
  │     │           ├─> Para cada par consecutivo en tour:
  │     │           │     • tour[0]=0 → waypoint 12345
  │     │           │     • tour[1]=2 → waypoint 30000
  │     │           │     │
  │     │           │     └─> Recuperar path almacenado en TspMatrix
  │     │           │           o re-ejecutar findPath()
  │     │           │
  │     │           └─> Concatenar todos los paths → Route completa
  │     │
  │     └─> Signal: tspSolved(TspResult result)
  │           │
  │           └─> MainWindow::onTspReady(TspResult result)
  │                 │
  │                 ├─> Ocultar ProgressDialog
  │                 ├─> MapWidget::drawTspRoute(result.route)
  │                 │     │
  │                 │     ├─> Dibujar tour completo (líneas)
  │                 │     ├─> Numerar waypoints (1, 2, 3, ...)
  │                 │     └─> Resaltar punto inicial
  │                 │
  │                 ├─> ResultsPanel::displayTspMetrics()
  │                 │     │
  │                 │     ├─> "Tour: 1 → 3 → 5 → 2 → 4 → 1"
  │                 │     ├─> "Total distance: 42.7 km"
  │                 │     ├─> "Total time: 51 min"
  │                 │     └─> "Computation: 3.2 s"
  │                 │
  │                 └─> StatusBar: "TSP solved (42.7 km, 5 waypoints)"
  │
END
```

---

## 5. STACK TECNOLÓGICO

### 5.1 Herramientas y Librerías

| Categoría | Tecnología | Versión | Propósito |
|-----------|-----------|---------|-----------|
| **Lenguaje** | C++ | 17 | Código base |
| **GUI Framework** | Qt | 6.6+ | Interfaz gráfica |
| **Build System** | CMake | 3.20+ | Compilación multiplataforma |
| **Threading** | std::thread + Qt | - | Concurrencia |

### 5.2 Compiladores Soportados

- **Windows:** MSVC 2022, MinGW-w64
- **macOS:** Clang 14+ (Xcode)
- **Linux:** GCC 11+, Clang 14+

### 5.3 Qt Modules Utilizados

```cmake
find_package(Qt6 REQUIRED COMPONENTS
    Core        # Clases fundamentales (QObject, signals/slots)
    Widgets     # UI (QMainWindow, QPushButton, etc.)
    Gui         # Renderizado (QPixmap, QPainter)
    Concurrent  # Futuro/async Qt
    Test        # Framework de testing
)
```

---

## 6. REQUISITOS Y CUMPLIMIENTO

### 6.1 Testing (LIGERO - Solo Esencial)

**Tests Unitarios Básicos (GoogleTest):**
- `GraphTest`: Operaciones básicas de grafo (add, get, adjacency)
- `DijkstraTest`: Pathfinding simple en grafo pequeño
- `TspMatrixTest`: Construcción de matriz básica
- `BinaryGraphLoaderTest`: Lectura de archivo .bin

**NOTA:** Recomendado 4-5 archivos de tests (pueden ser mas si se da el caso), enfoque en validar funcionalidad crítica.  
**Objetivo:** Validar que compile y funcione, no cobertura >80%.

### 6.2 Patrones de Diseño

**Patrón Creacional: Factory Method**
- `AlgorithmFactory`: Crea Dijkstra/A*/ALT según string
- `TspAlgorithmFactory`: Crea IG/IGSA/ILS

**Patrón Estructural: Strategy**
- `IPathfindingAlgorithm`, `ITspAlgorithm`, `IHeuristic` (intercambiables)
- Permite cambiar algoritmos en runtime sin modificar servicios

**Bonus (integrados en Qt/C++):**
- **Observer:** Qt Signals/Slots (MainWindow escucha a Services)
- **MVC:** Separación clara View/Controller/Model

### 6.3 Multithreading Básico

#### 6.3.1 Análisis de Operaciones Pesadas

**Operaciones que pueden congelar la UI (>100ms):**

| Operación | Tiempo Estimado | Impacto en UI | Prioridad Thread |
|-----------|----------------|---------------|------------------|
| `BinaryGraphLoader::load()` | 1-30 segundos | Congela completamente | **ALTA** |
| `TspMatrix::precompute()` | 1-60 segundos | Congela completamente | **ALTA** |
| `TspAlgorithm::solve()` | 1-10 segundos | Congela completamente | **ALTA** |
| `PathfindingService::findPath()` | 100-500ms | Puede congelar | **MEDIA** |
| `MapWidget::drawGraph()` | 200-500ms | Puede congelar | **BAJA** (opcional) |

#### 6.3.5 Casos de Uso de Hilos

**OBLIGATORIOS (evitan congelamiento total):**
1. **GraphService::loadGraphAsync()** → Hilo para `BinaryGraphLoader::load()`
2. **TspService::solveAsync()** → Hilo para `TspMatrix::precompute()` + `TspAlgorithm::solve()`

**RECOMENDADOS (mejoran UX):**
3. **PathfindingService::findPathAsync()** → Hilo si pathfinding demora >200ms
4. **Progress signals** → Feedback visual durante operaciones largas (TspMatrix)

**OPCIONALES (solo si hay lag visible):**
5. **MapWidget::drawGraphAsync()** → Hilo si renderizar 30k nodos causa lag

### 6.4 Interfaz en Qt

**Widgets Principales:**
- `MainWindow`: Ventana principal con status bar
- `MapWidget`: QGraphicsView para renderizar mapa
- `ControlPanel`: **OBLIGATORIO** - Formularios de entrada (algoritmo, heurística, vehículo, nodos)
- `ResultsPanel`: Métricas y estadísticas (tiempo, distancia, nodos visitados)

**Características UI:**
- Zoom/pan en mapa
- Click para seleccionar nodos
- Dibujar rutas calculadas
- **MenuBar/Toolbar:** OPCIONAL

---

## 7. ESTRUCTURA DE CARPETAS

```
routing-app-cpp/
├── CMakeLists.txt              # Build raíz
├── README.md
│
├── src/
│   ├── main.cpp                # Entry point Qt
│   │
│   ├── core/                   # DOMAIN LAYER
│   │   ├── entities/
│   │   │   ├── Node.h / .cpp
│   │   │   ├── Edge.h / .cpp
│   │   │   └── Graph.h / .cpp
│   │   ├── value_objects/
│   │   │   ├── Coordinate.h
│   │   │   ├── Distance.h
│   │   │   └── RouteSegment.h
│   │   └── interfaces/
│   │       ├── IPathfindingAlgorithm.h
│   │       ├── ITspAlgorithm.h
│   │       └── IHeuristic.h
│   │
│   ├── algorithms/             # BUSINESS LOGIC
│   │   ├── VehicleProfile.h / .cpp
│   │   ├── pathfinding/
│   │   │   ├── DijkstraAlgorithm.h / .cpp
│   │   │   ├── AStarAlgorithm.h / .cpp
│   │   │   └── ALTAlgorithm.h / .cpp      // Opcional si alcanza tiempo
│   │   ├── tsp/
│   │   │   ├── TspMatrix.h / .cpp         *** Usar std::thread aquí
│   │   │   ├── IGAlgorithm.h / .cpp
│   │   │   ├── IGSAAlgorithm.h / .cpp     // Opcional
│   │   │   └── ILSAlgorithm.h / .cpp      // Opcional
│   │   └── factories/
│   │       ├── AlgorithmFactory.h / .cpp
│   │       └── TspAlgorithmFactory.h / .cpp
│   │
│   ├── services/               # APPLICATION LAYER
│   │   ├── PathfindingService.h / .cpp    *** Usar std::thread para async
│   │   ├── TspService.h / .cpp            *** Usar std::thread para async
│   │   └── GraphService.h / .cpp
│   │
│   ├── infrastructure/         # INFRASTRUCTURE LAYER
│   │   └── persistence/
│   │       └── BinaryGraphLoader.h / .cpp
│   │
│   ├── ui/                     # PRESENTATION LAYER (Qt)
│   │   ├── MainWindow.h / .cpp / .ui
│   │   ├── widgets/
│   │   │   ├── MapWidget.h / .cpp
│   │   │   ├── ControlPanel.h / .cpp / .ui   # OBLIGATORIO
│   │   │   └── ResultsPanel.h / .cpp / .ui   # OBLIGATORIO
│   │   └── resources/
│   │       ├── resources.qrc   # Qt Resource Collection (opcional)
│   │       └── icons/          # Opcional
│   │           └── marker.svg
│   │
│   └── utils/                  # UTILITIES
│       └── exceptions/
│           ├── BaseException.h
│           └── GraphException.h
│
├── tests/                      # TESTING LIGERO
│   ├── CMakeLists.txt
│   │
│   └── unit/                   # Solo GoogleTest básico
│       ├── GraphTest.cpp
│       ├── DijkstraTest.cpp
│       ├── TspMatrixTest.cpp
│       └── BinaryGraphLoaderTest.cpp
│
└── data/                       # Runtime data
    └── graphs/
        └── arequipa.bin
```

---
>>>>>>> main
