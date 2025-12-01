#pragma once

#include <cstdint>
#include <memory>
#include "Renderer.hpp" 
class Ground {
public:
  Ground();
  void render(const std::unique_ptr<Renderer> &renderer);

protected:

private:
  static constexpr float c_gridSize = 10.f;
  static constexpr float c_gapSize = 1.f;
  static constexpr uint16_t c_gridCount = 24 * (c_gridSize / c_gapSize);

  std::vector<float> getGrid();
  uint32_t m_vao;
  uint32_t m_vbo;
  uint32_t m_ebo;
};
