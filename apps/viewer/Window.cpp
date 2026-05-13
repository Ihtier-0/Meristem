#include "Window.h"

#include <stdexcept>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace D {

Window::Window(int w, int h, const char* title,
               std::function<void(uint32_t, uint32_t)> onResize)
    : m_w(w), m_h(h), m_onResize(std::move(onResize)) {
  if (!glfwInit()) throw std::runtime_error("glfwInit failed");

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  m_window = glfwCreateWindow(w, h, title, nullptr, nullptr);
  if (!m_window) { glfwTerminate(); throw std::runtime_error("glfwCreateWindow failed"); }

  glfwSetWindowUserPointer(m_window, this);
  glfwSetFramebufferSizeCallback(m_window, resizeCallback);
  glfwMakeContextCurrent(m_window);
  glfwSwapInterval(1);

  if (!gladLoadGL(glfwGetProcAddress)) throw std::runtime_error("gladLoadGL failed");

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(m_window, true);
  ImGui_ImplOpenGL3_Init("#version 460");
}

Window::~Window() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(m_window);
  glfwTerminate();
}

void Window::run(std::function<void()> frame) {
  while (!glfwWindowShouldClose(m_window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    frame();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(m_window);
  }
}

void Window::resizeCallback(GLFWwindow* win, int w, int h) {
  auto* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
  self->m_w = w;
  self->m_h = h;
  if (self->m_onResize) self->m_onResize(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
}

}  // namespace D
