#pragma once

#include "Camera.hpp"
#include "Renderer.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include "Scene.hpp"

class GLFWwindowDeleter {
public:
  void operator()(GLFWwindow *ptr) { glfwDestroyWindow(ptr); }
};

class Window {
public:
  Window(uint16_t width, uint16_t height, std::string title);
  ~Window();

  void update(bool &running);
  void draw();

public:
  static void scrollInputCallback(GLFWwindow *window, double xOffset,
                                  double yOffset);
  static void keyInputCallback(GLFWwindow *window, int key, int scancode,
                               int action, int mods);
  static void cursorPositionCallback(GLFWwindow *window, double xpos,
                                     double ypos);
  static void mouseButtonCallback(GLFWwindow *window, int button, int action,
                                  int mods);
  static void resizeWindowCallback(GLFWwindow *window, int width, int height);

private:
  void renderImgui(float dt);

private:
  std::unique_ptr<GLFWwindow, GLFWwindowDeleter> m_window;
  std::unique_ptr<Renderer> m_renderer;
  std::unique_ptr<Scene> m_sceneQuat;
  std::unique_ptr<Scene> m_sceneEuler;
  Camera m_camera;
  float m_t;
  int m_height, m_width;
  bool m_clicked;
  bool m_showAllFrames{false};
  int m_intermediateFrames{5};
};
