# Meristem

A C++23 framework for generating and visualizing Functional-Structural Plant Models (FSPM).  
Primary algorithm: L-systems (Lindenmayer systems). More algorithms to follow.

[Русский](README.ru.md)

## Features

- **D0L-systems** — deterministic context-free L-systems
- **2D Turtle graphics** — string-to-geometry interpretation
- **Interactive viewer** — OpenGL + ImGui, live control of branching angle, zoom, and pan
- **Extensible architecture** — `IPlantAlgorithm`, `IGeometryBuilder`, `IRenderer` interfaces

## Architecture

```
IPlantAlgorithm → PlantStructure → IGeometryBuilder → Mesh → IRenderer
```

`PlantStructure = std::variant<StringStructure, TreeGraph>` — dispatched via `std::visit`.  
No virtual methods on structure types, eliminating circular header dependencies.

```
libs/
  core/         — Vec2/3/4, Mat4, Quat, Symbol, Word
  environment/  — IEnvironment, EnvironmentSample
  grammar/      — Rule, LSystemGrammar
  structure/    — StringStructure, TreeGraph, PlantStructure
  algorithm/    — IPlantAlgorithm, D0LSystemAlgorithm
  geometry/     — Mesh, IGeometryBuilder, TurtleBuilder2D
  renderer/     — DrawCall, IRenderer, OpenGLRenderer

apps/
  viewer/       — GLFW + ImGui main loop
```

## Requirements

| Tool | Version |
|---|---|
| CMake | 3.30+ |
| MSVC | 19.40+ (VS 2022 17.10+) |
| GCC | 14+ |
| Clang | 18+ |
| Python | 3.x (required by glad2 at configure time) |

## Build

```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

All dependencies are fetched automatically via [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) v0.42.3.

## Dependencies

| Library | Version | Purpose |
|---|---|---|
| [GLFW](https://github.com/glfw/glfw) | 3.4 | window and input |
| [GLM](https://github.com/g-truc/glm) | 1.0.1 | math (Vec, Mat, Quat) |
| [glad2](https://github.com/Dav1dde/glad) | v2.0.6 | OpenGL loader |
| [spdlog](https://github.com/gabime/spdlog) | v1.14.1 | logging |
| [Dear ImGui](https://github.com/ocornut/imgui) | v1.91.6 | GUI |

All licenses are MIT or zlib — no copyleft restrictions.

## License

MIT — Copyright (c) 2026-present Ikhtierzhon Rakhimov
