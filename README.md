\# ROUTING APP C++ + QT6: SISTEMA DE RUTEOS RÁPIDOS



\## 1. Visión General



Este proyecto es una aplicación de escritorio de \*\*alto rendimiento\*\* diseñada para resolver problemas complejos de optimización en grafos viales masivos. Migramos de Java a \*\*C++ / Qt6\*\* para lograr velocidad y bajo consumo de memoria.



\* \*\*Propósito:\*\* Encontrar rutas óptimas y resolver el Problema del Viajante (TSP) en tiempo real.

\* \*\*Enfoque:\*\* Rendimiento algorítmico y capacidad para manejar mapas grandes ($30k+$ nodos) sin congelar la interfaz.







\## 2. Stack Tecnológico



Elegimos un \*stack\* robusto y de bajo nivel para garantizar la máxima velocidad.



| Categoría | Tecnología | Rol Principal |

| :--- | :--- | :--- |

| \*\*Lenguaje Base\*\* | \*\*C++17\*\* | Núcleo de alto rendimiento para algoritmos. |

| \*\*Interfaz Gráfica (UI)\*\* | \*\*Qt 6.6+ Widgets\*\* | Desarrollo de UI nativa y multiplataforma. |

| \*\*Build System\*\* | \*\*CMake\*\* | Compilación y gestión de dependencias simple. |

| \*\*Concurrencia\*\* | \*\*std::thread / Qt Concurrent\*\* | Ejecutar cálculos pesados en segundo plano (OBLIGATORIO). |







\## 3. Funcionalidades y Algoritmos



El sistema soporta dos modos de operación críticos:



\### A. Búsqueda de Ruta Corta (Shortest Path)



\* \*\*Algoritmos:\*\*

&nbsp;   \* \*\*Dijkstra:\*\* El estándar de oro.

&nbsp;   \* \*\*A\\\*\*\*: Más rápido en la práctica (usa heurística de distancia).

&nbsp;   \* \*\*Meta de Rendimiento:\*\* Cálculo en \*\*menos de 100ms\*\*.



\### B. Problema del Viajante (TSP)



\* \*\*Proceso:\*\* Primero, se crea una Matriz de Distancias (paso lento, ejecutado en hilo), y luego se resuelve la matriz.

\* \*\*Algoritmos:\*\* Metaheurísticas avanzadas como \*\*Iterated Greedy (IG)\*\* para encontrar soluciones casi-óptimas rápidamente.



\### C. UI Responsiva (Clave)



\* La carga de mapas y los cálculos de TSP se realizan en \*\*hilos de fondo\*\* (`Async`) para que la interfaz (zoom, pan, botones) \*\*nunca se congele\*\*.







\## 4.Integrantes (Ejemplo)



| Nombre | Rol Clave |

| :--- | :--- |

| \[Nombre 1] | \*\*Líder de Algoritmos\*\* (Dijkstra, A\\\*, TSP Core) |

| \[Nombre 2] | \*\*Ingeniería de Grafo\*\* (Binary Loader, Estructuras C++) |

| \[Nombre 3] | \*\*Desarrollo Qt6 UI/UX\*\* (MapWidget, Señales y Slots) |







\## 5.Cronograma de Implementación (16 Días)



| Fase | Tarea Principal (Entregable) | Duración |

| :--- | :--- | :--- |

| \*\*Fase 1: Setup\*\* | \*\*Carga del Grafo\*\* y Estructuras Core. | 2 Días |

| \*\*Fase 2: Pathfinding\*\* | \*\*Dijkstra\*\* y \*\*Threading para Carga\*\*. | 3 Días |

| \*\*Fase 3: UI\*\* | Interfaz Gráfica (Map/Control) y Visualización. | 3 Días |

| \*\*Fase 4: A\\\*\*\* | Implementación de \*\*A\\\*\*\* (Mejora de rendimiento SP). | 2 Días |

| \*\*Fase 5: TSP Core\*\* | \*\*TspMatrix\*\* e Implementación de \*\*IGAlgorithm\*\*. | 4 Días |

| \*\*Fase 6: Final\*\* | Pulido, Métricas y Tests Unitarios. | 2 Días |

