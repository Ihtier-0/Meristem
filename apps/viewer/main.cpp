#include <cstdlib>
#include <stdexcept>

#include <glm/glm.hpp>

#include "renderer/OpenGLRenderer.h"
#include "Window.h"
#include "ViewerUI.h"

int main() try {
  constexpr int kW = 1280, kH = 720;
  D::Window window(kW, kH, "Meristem v0.1");  // gladLoadGL called here
  D::OpenGLRenderer renderer(kW, kH);
  window.setOnResize([&](uint32_t w, uint32_t h) { renderer.resize(w, h); });

  D::ViewerUI ui(renderer);

  window.run([&] {
    ui.draw();
    renderer.beginFrame();
    renderer.submit({&ui.mesh(),       glm::mat4(1.f), ui.lineColor()});
    renderer.submit({&ui.flowerMesh(), glm::mat4(1.f), ui.flowerColor()});
    renderer.endFrame();
  });

  return EXIT_SUCCESS;
} catch (const std::exception&) {
  return EXIT_FAILURE;
}
