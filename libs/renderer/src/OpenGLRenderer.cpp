#include "renderer/OpenGLRenderer.h"

#include <stdexcept>
#include <string>

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace D {

static constexpr const char* kVert = R"(
#version 460 core
layout(location = 0) in vec3 aPos;
uniform mat4 uProj;
void main() { gl_Position = uProj * vec4(aPos, 1.0); }
)";

static constexpr const char* kFrag = R"(
#version 460 core
out vec4 FragColor;
uniform vec4 uColor;
void main() { FragColor = uColor; }
)";

static uint32_t compileShader(GLenum type, const char* src) {
  uint32_t id = glCreateShader(type);
  glShaderSource(id, 1, &src, nullptr);
  glCompileShader(id);

  int ok = 0;
  glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    char log[512];
    glGetShaderInfoLog(id, 512, nullptr, log);
    glDeleteShader(id);
    throw std::runtime_error(std::string("Shader compile error: ") + log);
  }
  return id;
}

OpenGLRenderer::OpenGLRenderer(uint32_t width, uint32_t height)
    : m_width(width), m_height(height) {
  compileShaders();

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glGenBuffers(1, &m_ebo);

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);

  updateProjection();
}

OpenGLRenderer::~OpenGLRenderer() {
  glDeleteBuffers(1, &m_ebo);
  glDeleteBuffers(1, &m_vbo);
  glDeleteVertexArrays(1, &m_vao);
  glDeleteProgram(m_shader);
}

void OpenGLRenderer::compileShaders() {
  uint32_t vert = compileShader(GL_VERTEX_SHADER, kVert);
  uint32_t frag = compileShader(GL_FRAGMENT_SHADER, kFrag);

  m_shader = glCreateProgram();
  glAttachShader(m_shader, vert);
  glAttachShader(m_shader, frag);
  glLinkProgram(m_shader);
  glDeleteShader(vert);
  glDeleteShader(frag);

  m_locProj  = glGetUniformLocation(m_shader, "uProj");
  m_locColor = glGetUniformLocation(m_shader, "uColor");
}

void OpenGLRenderer::updateProjection() {
  float aspect = static_cast<float>(m_width) / static_cast<float>(m_height);
  float halfH = 10.f / m_zoom;
  float halfW = halfH * aspect;
  glm::mat4 proj = glm::ortho(-halfW + m_panX,  halfW + m_panX,
                               -halfH + m_panY,  halfH + m_panY,
                               -1.f, 1.f);
  glUseProgram(m_shader);
  glUniformMatrix4fv(m_locProj, 1, GL_FALSE, glm::value_ptr(proj));
}

void OpenGLRenderer::beginFrame() {
  glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
  glClear(GL_COLOR_BUFFER_BIT);
  updateProjection();
}

void OpenGLRenderer::submit(const DrawCall& dc) {
  if (!dc.mesh || dc.mesh->empty()) return;

  glBindVertexArray(m_vao);

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(dc.mesh->positions.size() * sizeof(glm::vec3)),
               dc.mesh->positions.data(), GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(dc.mesh->indices.size() * sizeof(uint32_t)),
               dc.mesh->indices.data(), GL_DYNAMIC_DRAW);

  glUseProgram(m_shader);
  glUniform4fv(m_locColor, 1, glm::value_ptr(dc.color));

  GLenum mode = (dc.mesh->mode == PrimitiveMode::Lines) ? GL_LINES : GL_TRIANGLES;
  glDrawElements(mode, static_cast<GLsizei>(dc.mesh->indices.size()),
                 GL_UNSIGNED_INT, nullptr);

  glBindVertexArray(0);
}

void OpenGLRenderer::endFrame() {}

void OpenGLRenderer::resize(uint32_t w, uint32_t h) {
  m_width  = w;
  m_height = h;
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
}

}  // namespace D
