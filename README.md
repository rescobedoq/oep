# Routing App

Este proyecto es una aplicación de escritorio para carga de grafos viales, cálculo de rutas óptimas y resolución de TSP (Traveling Salesman Problem) con una interfaz gráfica basada en Qt6.

## Visión general
- Cargar grafos viales procesados (formato binario derivado de OSM) de forma eficiente.
- Calcular rutas óptimas entre puntos usando Dijkstra, A* o ALT.
- Resolver instancias TSP (10–20 waypoints) mediante metaheurísticas (IG, IGSA, ILS).
- Visualizar mapas y resultados en una UI responsiva.
- Proporcionar métricas de rendimiento y exportación de resultados si es necesario.

## Características principales
- Carga rápida de grafos (miles a decenas de miles de nodos) en un hilo de fondo.
- Búsqueda de rutas (shortest-path) con implementación modular de algoritmos.
- Módulo TSP que precomputará una matriz de distancias (N² caminos) y ejecutará heurísticas para obtener tours.
- UI con controles para seleccionar algoritmo, perfil de vehículo y waypoints; mapa interactivo con selección por clic.
- Soporte para operaciones asíncronas para evitar bloqueo de la interfaz.

## Arquitectura
- Presentación: Qt (MainWindow, MapWidget, ControlPanel, ResultsPanel).
- Aplicación: Controllers (slots) que delegan a servicios.
- Lógica de negocio: Servicios (PathfindingService, TspService), factories y algoritmos.
- Dominio: Entidades core: Graph, Node, Edge, Route, VehicleProfile.
- Infraestructura: BinaryGraphLoader / GraphRepository para lectura de grafos en disco.

