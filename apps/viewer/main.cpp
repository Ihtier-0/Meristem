#include <cstdlib>
#include <stdexcept>

#include <glm/glm.hpp>

#include "algorithm/D0LSystemAlgorithm.h"
#include "geometry/TurtleBuilder2D.h"
#include "renderer/OpenGLRenderer.h"
#include "Window.h"
#include "ViewerUI.h"
#include "examples.h"

int main() try {
  auto grammar = D::examples::binaryTree();
  D::D0LSystemAlgorithm algo(grammar);
  D::TurtleBuilder2D turtle(grammar.angle);
  D::Mesh mesh = D::buildMesh(turtle, algo.getStructure());

  constexpr int kW = 1280, kH = 720;
  D::Window window(kW, kH, "Meristem v0.1");  // gladLoadGL called here
  D::OpenGLRenderer renderer(kW, kH);
  window.setOnResize([&](uint32_t w, uint32_t h) { renderer.resize(w, h); });

  D::ViewerUI ui(grammar, algo, turtle, mesh, renderer);

  window.run([&] {
    ui.draw();
    renderer.beginFrame();
    renderer.submit({&mesh, glm::mat4(1.f), ui.lineColor()});
    renderer.endFrame();
  });

  return EXIT_SUCCESS;
} catch (const std::exception&) {
  return EXIT_FAILURE;
}
