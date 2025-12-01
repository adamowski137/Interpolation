#pragma once
#include "Window.hpp"

class App {
public:
  App();

  void run();

private:
  bool m_isRunning;
  Window m_window;
};
