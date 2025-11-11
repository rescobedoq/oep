# ROUTING APP C++ + Qt6 — Sistema de Ruteos Rápidos

## 1. Visión general

`Routing App` es una aplicación de escritorio de **alto rendimiento** para resolver problemas de optimización en grafos viales masivos. Migramos de Java a **C++ / Qt 6** para mejorar velocidad y uso de memoria.

- **Propósito:** Encontrar rutas óptimas y resolver el Problema del Viajante (TSP) en tiempo real.  
- **Objetivo de escala:** manejar mapas grandes (30k+ nodos) sin bloquear la interfaz de usuario.

---

## 2. Stack tecnológico

| Categoría                      | Tecnología         | Rol principal                                      |
|--------------------------------|--------------------|---------------------------------------------------|
| Lenguaje base                  | C++17              | Núcleo de alto rendimiento para algoritmos.       |
| Interfaz gráfica (UI)          | Qt 6.6+ (Widgets)  | UI nativa y multiplataforma.                      |
| Build system                   | CMake              | Compilación y gestión de dependencias.            |
| Concurrencia                   | `std::thread` / Qt Concurrent | Ejecutar cálculos pesados en segundo plano. |

---

## 3. Funcionalidades y algoritmos

La app soporta dos modos de operación principales:

### A. Búsqueda de ruta corta (Shortest Path)
- **Algoritmos:** `Dijkstra`, `A*` (A-star, heurística Euclidiana/Haversine).
- **Meta de rendimiento:** consultas en < 100 ms (dependiendo del tamaño del grafo y heurística).

### B. Problema del Viajante (TSP)
- **Flujo:** construcción de una Matriz de Distancias (ejecución en hilo de fondo) → resolución con metaheurísticas.
- **Algoritmos sugeridos:** `Iterated Greedy (IG)`, búsquedas locales (2-opt/3-opt) como refinamiento.

### C. UI responsiva (clave)
- Carga de mapas y cómputos pesados en hilos separados para que la UI (zoom, pan, controles) no se congele.

---

## 4. Integrantes (ejemplo)

- Nina Calizaya Rafael Diego
- Coaquira Suyo Gabriela Dayana
- Villafuerte Quispe Alexander
- Quispe Saavedra Dennis Javier
- Venero Guevara Christian Henry 

---

## 5. Cronograma de implementación (16 días)

| Fase         | Tarea principal (entregable)                            | Duración |
|--------------|---------------------------------------------------------|----------|
| Fase 1: Setup| Carga del grafo y estructuras core (binary loader, structs) | 2 días  |
| Fase 2: Pathfinding | Implementación de `Dijkstra` y threading para carga | 3 días  |
| Fase 3: UI   | MapWidget, controles, y visualización                   | 3 días  |
| Fase 4: A*   | Implementación de `A*` y optimización de heurística     | 2 días  |
| Fase 5: TSP Core | `TspMatrix` y `IGAlgorithm` (Iterated Greedy)       | 4 días  |
| Fase 6: Final| Pulido, métricas, tests unitarios y documentación       | 2 días  |

---

## 6. Recomendaciones de diseño técnico (rápido)

1. **Formato de grafo en disco:** usar un formato binario compacto (nodes array + edges contiguous) para lecturas mmap-friendly.
2. **Estructuras de memoria:** vectores contiguos (`std::vector`) y estructuras POD para minimizar overhead.
3. **Heurística A*:** Haversine (geo) o Euclidean (plano) como función admissible; precomputar bounding boxes si es necesario.
4. **Paralelismo:** usar `std::thread` / `QtConcurrent::run` para operaciones de I/O y cálculo (NO bloquear hilo UI).
5. **TSP:** evitar resolver TSP exacto en nodos grandes; usar matriz reducido (solo puntos de interés) y metaheurísticas.

---

## 7. Cómo usar / desarrollo local (rápido)

```bash
# Clona y entra al repo (ejemplo)
git clone <URL_DEL_REPO>
cd <REPO>

# Crea y entra a la rama 'dennis' si aún no existe localmente
git checkout -b dennis origin/dennis || git checkout dennis

# Agrega README.md (o reemplaza su contenido) y sube los cambios:
git add README.md
git commit -m "docs: README inicial del proyecto Routing App (C++ / Qt6)"
git push origin dennis
# Si es la primera vez y te pide upstream:
# git push --set-upstream origin dennis
```