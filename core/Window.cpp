#include "Window.hpp"
#include "GLFW/glfw3.h"
#include "ImGuiFileDialog.h"
#include "MatrixUtils.hpp"
#include "Quaternion.hpp"
#include "Renderer.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <sstream>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

#define DEBUG 1

GLenum glCheckError_(const char *file, int line)
{
  GLenum errorCode;
  while ((errorCode = glGetError()) != GL_NO_ERROR)
  {
    std::string error;
    switch (errorCode)
    {
    case GL_INVALID_ENUM:
      error = "INVALID_ENUM";
      break;
    case GL_INVALID_VALUE:
      error = "INVALID_VALUE";
      break;
    case GL_INVALID_OPERATION:
      error = "INVALID_OPERATION";
      break;
    case GL_STACK_OVERFLOW:
      error = "STACK_OVERFLOW";
      break;
    case GL_STACK_UNDERFLOW:
      error = "STACK_UNDERFLOW";
      break;
    case GL_OUT_OF_MEMORY:
      error = "OUT_OF_MEMORY";
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      error = "INVALID_FRAMEBUFFER_OPERATION";
      break;
    }
    std::cout << error << " | " << file << " (" << line << ")" << std::endl;
  }
  return errorCode;
}

void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id,
                            GLenum severity, GLsizei length,
                            const char *message, const void *userParam)
{
  // ignore non-significant error/warning codes
  if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
    return;

  std::cout << "---------------" << std::endl;
  std::cout << "Debug message (" << id << "): " << message << std::endl;

  switch (source)
  {
  case GL_DEBUG_SOURCE_API:
    std::cout << "Source: API";
    break;
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
    std::cout << "Source: Window System";
    break;
  case GL_DEBUG_SOURCE_SHADER_COMPILER:
    std::cout << "Source: Shader Compiler";
    break;
  case GL_DEBUG_SOURCE_THIRD_PARTY:
    std::cout << "Source: Third Party";
    break;
  case GL_DEBUG_SOURCE_APPLICATION:
    std::cout << "Source: Application";
    break;
  case GL_DEBUG_SOURCE_OTHER:
    std::cout << "Source: Other";
    break;
  }
  std::cout << std::endl;

  switch (type)
  {
  case GL_DEBUG_TYPE_ERROR:
    std::cout << "Type: Error";
    break;
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    std::cout << "Type: Deprecated Behaviour";
    break;
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    std::cout << "Type: Undefined Behaviour";
    break;
  case GL_DEBUG_TYPE_PORTABILITY:
    std::cout << "Type: Portability";
    break;
  case GL_DEBUG_TYPE_PERFORMANCE:
    std::cout << "Type: Performance";
    break;
  case GL_DEBUG_TYPE_MARKER:
    std::cout << "Type: Marker";
    break;
  case GL_DEBUG_TYPE_PUSH_GROUP:
    std::cout << "Type: Push Group";
    break;
  case GL_DEBUG_TYPE_POP_GROUP:
    std::cout << "Type: Pop Group";
    break;
  case GL_DEBUG_TYPE_OTHER:
    std::cout << "Type: Other";
    break;
  }
  std::cout << std::endl;

  switch (severity)
  {
  case GL_DEBUG_SEVERITY_HIGH:
    std::cout << "Severity: high";
    break;
  case GL_DEBUG_SEVERITY_MEDIUM:
    std::cout << "Severity: medium";
    break;
  case GL_DEBUG_SEVERITY_LOW:
    std::cout << "Severity: low";
    break;
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    std::cout << "Severity: notification";
    break;
  }
  std::cout << std::endl;
  std::cout << std::endl;
}

#define glCheckError() glCheckError_(__FILE__, __LINE__)

Window::Window(uint16_t width, uint16_t height, std::string title)
    : m_camera(1.f, {0.0f, 0.0f, 0.0f}), m_t(0.f), m_height(height),
      m_width(width), m_clicked(false)
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  m_window = std::unique_ptr<GLFWwindow, GLFWwindowDeleter>(
      glfwCreateWindow(width, height, title.c_str(), NULL, NULL));
  if (m_window.get() == NULL)
  {
    throw std::runtime_error("Failed to create GLFW window");
  }
  glfwMakeContextCurrent(m_window.get());
  glewInit();

  m_renderer = std::make_unique<Renderer>();
  m_sceneQuat = std::make_unique<Scene>(true);
  m_sceneEuler = std::make_unique<Scene>(false);
  // initial projection will be set per-viewport in draw()
  m_renderer->setProjection(math137::MatrixUtils::Projection(
      M_PI_4, (float)m_width / m_height, 0.1f, 100.f));

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui_ImplGlfw_InitForOpenGL(m_window.get(), true);
  ImGui_ImplOpenGL3_Init();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_STENCIL_TEST);
  glfwSetWindowUserPointer(m_window.get(), reinterpret_cast<void *>(this));
  glfwSetScrollCallback(m_window.get(), scrollInputCallback);
  glfwSetKeyCallback(m_window.get(), keyInputCallback);
  glfwSetMouseButtonCallback(m_window.get(), mouseButtonCallback);
  glfwSetCursorPosCallback(m_window.get(), cursorPositionCallback);
  glfwSetFramebufferSizeCallback(m_window.get(), resizeWindowCallback);
#if DEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(glDebugOutput, nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr,
                        GL_TRUE);

#endif // 0
}

Window::~Window()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();
}

void Window::update(bool &running)
{
  running = !glfwWindowShouldClose(m_window.get());
  glfwPollEvents();
}

void Window::draw()
{
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  float t = glfwGetTime();
  float dt = (m_t == 0.0f) ? 0.0f : (t - m_t);

  // advance scene state
  if (m_sceneQuat) m_sceneQuat->update(dt);
  if (m_sceneEuler) m_sceneEuler->update(dt);

  int halfW = m_width / 2;
  m_renderer->setView(m_camera.getView());

  glViewport(0, 0, halfW, m_height);
  m_renderer->setProjection(math137::MatrixUtils::Projection(
      M_PI_4, (float)halfW / (float)m_height, 0.1f, 100.f));
  if (m_showAllFrames) m_sceneQuat->renderSamples(m_renderer, m_intermediateFrames);
  m_sceneQuat->render(m_renderer);

  glViewport(halfW, 0, m_width - halfW, m_height);
  m_renderer->setProjection(math137::MatrixUtils::Projection(
      M_PI_4, (float)(m_width - halfW) / (float)m_height, 0.1f, 100.f));
  if (m_showAllFrames) m_sceneEuler->renderSamples(m_renderer, m_intermediateFrames);
  m_sceneEuler->render(m_renderer);

  glViewport(0, 0, m_width, m_height);
  renderImgui(t - m_t);
  glfwSwapBuffers(m_window.get());
  m_t = t;
}

void Window::renderImgui(float dt)
{
  auto eulerToQuaternion = [](float roll, float pitch, float yaw) {
    float cr = std::cos(roll * 0.5f);
    float sr = std::sin(roll * 0.5f);
    float cp = std::cos(pitch * 0.5f);
    float sp = std::sin(pitch * 0.5f);
    float cy = std::cos(yaw * 0.5f);
    float sy = std::sin(yaw * 0.5f);
    math137::Quaternion q;
    q.a = cr * cp * cy + sr * sp * sy;
    q.b = sr * cp * cy - cr * sp * sy;
    q.c = cr * sp * cy + sr * cp * sy;
    q.d = cr * cp * sy - sr * sp * cy;
    return q;
  };
  // ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
  // ImGui::SetNextWindowSize(ImVec2(100, m_height), ImGuiCond_Always);
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::Begin("Info");
  static float startPos[3] = {0.0f, 0.0f, 0.0f};
  ImGui::InputFloat3("Cursor Start Position", startPos, "%.3f");
  static float startEuler[3] = {0.0f, 0.0f, 0.0f};
  static float startQuat[4] = {1.0f, 0.0f, 0.0f, 0.0f};
  bool startEulerChanged = ImGui::InputFloat3("Cursor Start Rotation (Euler)", startEuler, "%.3f");
  if (startEulerChanged) {
    math137::Quaternion q = eulerToQuaternion(startEuler[0], startEuler[1], startEuler[2]);
    q.normalize();
    startQuat[0] = q.a;
    startQuat[1] = q.b;
    startQuat[2] = q.c;
    startQuat[3] = q.d;
  }
  bool startQuatChanged = ImGui::InputFloat4("Cursor Start Rotation (Quat)", startQuat, "%.3f");
  if (startQuatChanged) {
    math137::Quaternion q{startQuat[0], startQuat[1], startQuat[2], startQuat[3]};
    q.normalize();
    // quaternion -> euler (X=roll, Y=pitch, Z=yaw) using Tait-Bryan angles
    float w = q.a, x = q.b, y = q.c, z = q.d;
    // roll (x-axis)
    float sinr_cosp = 2.0f * (w * x + y * z);
    float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
    float roll = atan2f(sinr_cosp, cosr_cosp);
    // pitch (y-axis)
    float sinp = 2.0f * (w * y - z * x);
    float pitch;
    if (fabsf(sinp) >= 1.0f)
      pitch = copysignf(M_PI_2, sinp);
    else
      pitch = asinf(sinp);
    // yaw (z-axis)
    float siny_cosp = 2.0f * (w * z + x * y);
    float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
    float yaw = atan2f(siny_cosp, cosy_cosp);
    startEuler[0] = roll;
    startEuler[1] = pitch;
    startEuler[2] = yaw;
  }
  ImGui::Separator();
  static float endPos[3] = {0.0f, 0.0f, 0.0f};
  ImGui::InputFloat3("Cursor End Position", endPos, "%.3f");
  static float endEuler[3] = {0.0f, 0.0f, 0.0f};
  static float endQuat[4] = {1.0f, 0.0f, 0.0f, 0.0f};
  bool endEulerChanged = ImGui::InputFloat3("Cursor End Rotation (Euler)", endEuler, "%.3f");
  if (endEulerChanged) {
    math137::Quaternion q = eulerToQuaternion(endEuler[0], endEuler[1], endEuler[2]);
    q.normalize();
    endQuat[0] = q.a;
    endQuat[1] = q.b;
    endQuat[2] = q.c;
    endQuat[3] = q.d;
  }
  bool endQuatChanged = ImGui::InputFloat4("Cursor End Rotation (Quat)", endQuat, "%.3f");
  if (endQuatChanged) {
    math137::Quaternion q{endQuat[0], endQuat[1], endQuat[2], endQuat[3]};
    q.normalize();
    float w = q.a, x = q.b, y = q.c, z = q.d;
    float sinr_cosp = 2.0f * (w * x + y * z);
    float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
    float roll = atan2f(sinr_cosp, cosr_cosp);
    float sinp = 2.0f * (w * y - z * x);
    float pitch;
    if (fabsf(sinp) >= 1.0f)
      pitch = copysignf(M_PI_2, sinp);
    else
      pitch = asinf(sinp);
    float siny_cosp = 2.0f * (w * z + x * y);
    float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
    float yaw = atan2f(siny_cosp, cosy_cosp);
    endEuler[0] = roll;
    endEuler[1] = pitch;
    endEuler[2] = yaw;
  }
  ImGui::Separator();
  static float duration = 5.0f;
  ImGui::InputFloat("Interpolation Duration (s)", &duration, 0.1f, 10.0f, "%.3f");
  static bool useSpherical = false;
  ImGui::Checkbox("Use Spherical Interpolation", &useSpherical);
  ImGui::Checkbox("Show All Frames", &m_showAllFrames);
  ImGui::InputInt("Intermediate Frames", &m_intermediateFrames);

  if (ImGui::Button("Start"))
  {
    // set Euler angles on the Euler scene
    // normalize Euler angles into [-pi, pi]
    math137::Quaternion startQ{startQuat[0], startQuat[1], startQuat[2], startQuat[3]};
    startQ.normalize();
    math137::Quaternion endQ{endQuat[0], endQuat[1], endQuat[2], endQuat[3]};
    endQ.normalize();
    startQuat[0] = startQ.a;
    startQuat[1] = startQ.b;
    startQuat[2] = startQ.c;
    startQuat[3] = startQ.d;
    endQuat[0] = endQ.a;
    endQuat[1] = endQ.b;
    endQuat[2] = endQ.c;
    endQuat[3] = endQ.d;
    m_sceneQuat->setStartPosition({startPos[0], startPos[1], startPos[2]});
    m_sceneEuler->setStartPosition({startPos[0], startPos[1], startPos[2]});
    m_sceneQuat->setEndPosition({endPos[0], endPos[1], endPos[2]});
    m_sceneEuler->setEndPosition({endPos[0], endPos[1], endPos[2]});

    m_sceneEuler->setStartEuler({startEuler[0], startEuler[1], startEuler[2]});
    m_sceneEuler->setEndEuler({endEuler[0], endEuler[1], endEuler[2]});

    m_sceneQuat->setStartQuaternion({startQuat[0], startQuat[1], startQuat[2], startQuat[3]});
    m_sceneQuat->setEndQuaternion({endQuat[0], endQuat[1], endQuat[2], endQuat[3]});

    m_sceneQuat->setUseSphericalInterpolation(useSpherical);

    m_sceneEuler->setT(duration);
    m_sceneQuat->setT(duration);

    m_sceneQuat->start();
    m_sceneEuler->start();
  }
  ImGui::End();
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Window::keyInputCallback(GLFWwindow *window, int key, int scancode,
                              int action, int mods)
{
  ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
  Window *w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
}

void Window::cursorPositionCallback(GLFWwindow *window, double xpos,
                                    double ypos)
{
  ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
  Window *w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  static double x, y;
  if (w->m_clicked)
    w->m_camera.rotateCamera(x - xpos, y - ypos);
  x = xpos;
  y = ypos;
}

void Window::mouseButtonCallback(GLFWwindow *window, int button, int action,
                                 int mods)
{
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
  if (ImGui::GetIO().WantCaptureMouse)
    return;
  Window *w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  w->m_clicked = (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS);
}

void Window::scrollInputCallback(GLFWwindow *window, double xOffset,
                                 double yOffset)
{
  ImGui_ImplGlfw_ScrollCallback(window, xOffset, yOffset);
  if (ImGui::GetIO().WantCaptureMouse)
    return;
  Window *w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  w->m_camera.changeDistance(0.8f * yOffset);
}
void Window::resizeWindowCallback(GLFWwindow *window, int width, int height) {}
