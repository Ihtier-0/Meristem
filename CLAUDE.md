# Meristem — Project Context for Claude

## Project

**Meristem** — C++23 framework for generating and visualizing Functional-Structural
Plant Models (FSPM). Primary algorithm: L-systems. More algorithms to follow.

> **C++ standard note:** The project targets C++23 (`CMAKE_CXX_STANDARD 23`) because
> MSVC in VS 2026 preview (19.51) has no `/std:c++26` flag yet — only `/std:c++latest`.
> CMake 4.x has no mapping for CXX26 on this compiler. Raise to 26 once MSVC ships
> a stable `/std:c++26`.

**C++ namespace:** `D`  
**License:** MIT — Copyright (c) 2026-present Ikhtierzhon Rakhimov  
**Platform:** Windows-first, cross-platform goal (Linux/macOS later)

---

## Coding conventions

- **Standard:** C++23 (see note above; intent is C++26)
- **Indent:** 2 spaces (Google style base, see `.clang-format`)
- **Headers:** `.h` extension, not `.hpp`
- **Local includes:** `"path/to/file.h"` (quotes)
- **Third-party / stdlib includes:** `<header>` (angle brackets)
- **Return types:** traditional syntax — `void foo()`, not `auto foo() -> void`.
  Trailing return type only when the return type genuinely depends on template
  parameters or is otherwise unwritable before the function name.
- **Naming:** types `PascalCase`, methods/functions `camelCase`, private members `m_fieldName`, constants `kPascalCase`
- **No comments** explaining what code does; only non-obvious why.
- **Warnings-as-errors** on all compilers (`/WX` MSVC, `-Werror` GCC/Clang).

---

## Module structure

```
libs/
  core/         include/core/types.h         — Vec2/3/4, Mat4, Quat, Symbol, Word
  environment/  include/environment/         — IEnvironment, EnvironmentSample
  grammar/      include/grammar/             — Rule, LSystemGrammar
  algorithm/    include/algorithm/           — IPlantAlgorithm
  geometry/     include/geometry/            — Mesh, IGeometryBuilder
  renderer/     include/renderer/            — DrawCall, IRenderer

apps/
  viewer/       — Qt6 QMainWindow, QOpenGLWidget, sidebar
```

**Dependency rule (no cycles):**
```
core ← environment, grammar, structure
structure ← algorithm, geometry
geometry ← renderer ← viewer
```

---

## Pipeline

```
IPlantAlgorithm -> StringStructure -> IGeometryBuilder -> Mesh -> IRenderer
```

---

## Key interfaces

```cpp
// algorithm/IPlantAlgorithm.h
class IPlantAlgorithm {
  virtual void step() = 0;
  virtual void reset() = 0;
  virtual int generation() const = 0;
  virtual const Word& current() const = 0;
};

// geometry/IGeometryBuilder.h
class IGeometryBuilder {
  virtual Mesh build(const Word&) = 0;
};

// renderer/IRenderer.h
class IRenderer {
  virtual void beginFrame() = 0;
  virtual void submit(const DrawCall&) = 0;
  virtual void endFrame() = 0;
  virtual void resize(uint32_t w, uint32_t h) = 0;
};
```

---

## LSystemGrammar — rule priority in derive()

When multiple rules match a symbol, apply in this order:

1. Context-sensitive (2L) with condition
2. Context-free with condition
3. Stochastic (pick by probability weight)
4. Identity production (symbol maps to itself)

`derive()` is a single step. `deriveN(n)` applies it n times from axiom.

---

## Turtle interpreter — symbol table

| Symbol | Action |
|--------|--------|
| `F` | move forward + draw segment |
| `f` | move forward, no draw |
| `+` | turn left by δ |
| `-` | turn right by δ |
| `&` | pitch down by δ |
| `^` | pitch up by δ |
| `\` | roll left by δ |
| `/` | roll right by δ |
| `\|` | turn 180° |
| `[` | push turtle state |
| `]` | pop turtle state |

2D turtle uses only F/f/+/-/[/]. Orientation stored as `Quat` for correct 3D
rotation (replaces H/L/U matrix triplet from Prusinkiewicz & Lindenmayer 1990).

---

## Node editor design decisions

- **Graph is a runtime view** — changes take effect on the running simulation
  immediately (not a static config file).
- **Expressions in nodes** — plain text fields (`"s * 0.7"`, `"s > 0.5"`),
  parsed by a small expression parser (TinyExpr++ or custom). No visual math nodes.
- **On graph change**, the compiler rebuilds `LSystemGrammar` (<1 ms) and applies
  one of three update policies (user-selectable in UI):

```cpp
enum class UpdatePolicy {
  RestartFromAxiom,      // reparse + restart from generation 0
  ApplyFromNextStep,     // keep current word, new rules from step N+1
  KeepCurrentGeneration, // only rebuild geometry (angle/width changed)
};
```

Node types (v1): Axiom, Rule (predecessor/condition/successor/probability),
Environment (gravity/light direction+strength), Preview (small framebuffer).

---

## Renderer backends

```
OpenGLRenderer    : IRenderer   // v1 — interactive viewer
MoonrayGLRenderer : IRenderer   // future — Moonray's interactive GL viewport
```

`MoonrayGLRenderer` wraps **MoonrayGL** (DreamWorks' OpenGL-based interactive
preview renderer, part of the Moonray production ray-tracer). It is *not* a
custom PBR shader — it is a full renderer exposed as another `IRenderer` impl.

Further future: USD Stage -> Hydra delegate -> Moonray/RenderMan/Arnold
(requires `D_ENABLE_USD` CMake option).

---

## Dependencies

| Library       | How        | Version  | Purpose                        | Required |
|---------------|------------|----------|--------------------------------|----------|
| Qt6           | system     | 6.x      | UI (Widgets + OpenGLWidgets)   | yes      |
| glm           | CPM        | 1.0.1    | Vec2/Vec3/Mat4/Quat            | yes      |
| glad2         | CPM        | v2.0.6   | OpenGL loader (needs Python 3) | yes      |
| spdlog        | CPM        | v1.14.1  | logging                        | yes      |
| assimp        | —          | —        | OBJ/GLTF export                | optional |
| pxr (OpenUSD) | —          | —        | OpenUSD integration            | optional |

Qt6 must be installed system-wide (Qt Online Installer, `msvc2022_64` variant).

**No setup required** if Qt is installed in `C:\Qt\` or `D:\Qt\` — CMake auto-detects it.

If Qt is somewhere unusual, override by setting `CMAKE_PREFIX_PATH` (via `CMakeUserPresets.json`,
which is in `.gitignore` and not committed):
```json
{
  "version": 6,
  "include": ["CMakePresets.json"],
  "configurePresets": [
    {
      "name": "x64-Debug",
      "inherits": "x64-Debug",
      "cacheVariables": { "CMAKE_PREFIX_PATH": "E:/MyQt/6.9.0/msvc2022_64" }
    }
  ]
}
```

**Architectural rule:** Qt dependency is confined to `apps/viewer/`.
All libs (`algorithm`, `geometry`, `grammar`, etc.) must stay Qt-free.

---

## Roadmap

```
v0.1  D0L + TurtleBuilder2D + OpenGL viewer ✅
v0.2  Stochastic + parametric L-systems
v0.3  Node editor (imnodes), runtime rule editing
v0.4  Context-sensitive (2L) + tropisms
v0.5  Space Colonization Algorithm
v1.0  OBJ/GLTF export, stable API
v1.x  Table L-systems (T0L), differential L-systems (dL)
v2.x  MoonrayGLRenderer (Moonray interactive GL viewport)
v3.x  OpenUSD + Hydra delegate -> Moonray/RenderMan/Arnold, leaves, LOD
```
