#pragma once

#include <functional>
#include <cstdint>

struct GLFWwindow;

namespace D {

class Window {
 public:
  Window(int w, int h, const char* title,
         std::function<void(uint32_t, uint32_t)> onResize = {});
  ~Window();

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  // Runs the main loop. frame() is called each iteration between
  // ImGui::NewFrame() and ImGui::Render().
  void run(std::function<void()> frame);
  void setOnResize(std::function<void(uint32_t, uint32_t)> cb) { m_onResize = std::move(cb); }

  int width()  const { return m_w; }
  int height() const { return m_h; }

 private:
  static void resizeCallback(GLFWwindow* win, int w, int h);

  GLFWwindow* m_window = nullptr;
  int m_w, m_h;
  std::function<void(uint32_t, uint32_t)> m_onResize;
};

}  // namespace D
