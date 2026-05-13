#pragma once

#include <cstdint>

#include <glm/vec4.hpp>

#include "renderer/IRenderer.h"

namespace D {

class OpenGLRenderer : public IRenderer {
 public:
  OpenGLRenderer(uint32_t width, uint32_t height);
  ~OpenGLRenderer();

  void beginFrame() override;
  void submit(const DrawCall& dc) override;
  void endFrame() override;
  void resize(uint32_t w, uint32_t h) override;

  void setZoom(float zoom)              { m_zoom = zoom; }
  void setPan(float x, float y)         { m_panX = x; m_panY = y; }
  void setClearColor(const glm::vec4& c){ m_clearColor = c; }

  float      zoom()       const { return m_zoom; }
  float      panX()       const { return m_panX; }
  float      panY()       const { return m_panY; }
  glm::vec4  clearColor() const { return m_clearColor; }

 private:
  void compileShaders();
  void updateProjection();

  uint32_t m_shader = 0;
  uint32_t m_vao = 0;
  uint32_t m_vbo = 0;
  uint32_t m_ebo = 0;
  uint32_t m_width = 0;
  uint32_t m_height = 0;
  int m_locProj = -1;
  int m_locColor = -1;

  float     m_zoom = 1.f;
  float     m_panX = 0.f;
  float     m_panY = 0.f;
  glm::vec4 m_clearColor = {0.08f, 0.08f, 0.08f, 1.f};
};

}  // namespace D
