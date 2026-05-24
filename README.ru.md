# Meristem

Фреймворк для генерации и визуализации функционально-структурных моделей растений (FSPM) на C++23.  
Основной алгоритм — L-системы (Lindenmayer systems). Планируется поддержка алгоритма колонизации пространства и других.

## Возможности

- **D0L-системы** — детерминированные контекстно-свободные L-системы
- **Черепашья графика 2D** — интерпретация строки символов как геометрии
- **Интерактивный просмотрщик** — OpenGL + ImGui, управление углом ветвления, масштабом и смещением
- **Расширяемая архитектура** — интерфейсы `IPlantAlgorithm`, `IGeometryBuilder`, `IRenderer`

## Архитектура

```
IPlantAlgorithm -> PlantStructure -> IGeometryBuilder -> Mesh -> IRenderer
```

Структура результата — `std::variant<StringStructure, TreeGraph>`, диспетчеризация через `std::visit`.  
Нет виртуальных методов на структурных типах — исключены циклические зависимости заголовков.

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
  viewer/       — GLFW + ImGui, главный цикл
```

## Требования

| Инструмент | Версия |
|---|---|
| CMake | 3.30+ |
| MSVC | 19.40+ (VS 2022 17.10+) |
| GCC | 14+ |
| Clang | 18+ |
| Python | 3.x (для генерации OpenGL-загрузчика glad2) |

## Сборка

```bash
cmake -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Release
```

Все зависимости загружаются автоматически через [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) v0.42.3:
`glfw`, `glm`, `glad2`, `spdlog`, `imgui`.

## Зависимости

| Библиотека | Версия | Назначение |
|---|---|---|
| [GLFW](https://github.com/glfw/glfw) | 3.4 | окно и ввод |
| [GLM](https://github.com/g-truc/glm) | 1.0.1 | математика (Vec, Mat, Quat) |
| [glad2](https://github.com/Dav1dde/glad) | v2.0.6 | загрузчик OpenGL |
| [spdlog](https://github.com/gabime/spdlog) | v1.14.1 | логирование |
| [Dear ImGui](https://github.com/ocornut/imgui) | v1.91.6 | GUI |

Все лицензии — MIT или zlib, без копилефта.

## Лицензия

MIT — Copyright (c) 2026-present Ikhtierzhon Rakhimov
