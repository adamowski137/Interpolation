#include "Ground.hpp"
#include <GL/glew.h>
#include <MatrixUtils.hpp>

Ground::Ground() {
  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  auto vertices = getGrid();
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glBindVertexArray(0);
}

void Ground::render(const std::unique_ptr<Renderer> &renderer) {
  glBindVertexArray(m_vao);
  renderer->setModel(math137::MatrixUtils::Identity());
  renderer->setColor({1.f, 1.f, 1.f, 1.f});
  glDrawArrays(GL_LINES, 0, (c_gridSize * 4 / c_gapSize + 1) * 4);
  glBindVertexArray(0);
}

std::vector<float> Ground::getGrid() {
  std::vector<float> vertices;
  for (float i = -c_gridSize; i <= c_gridSize; i += c_gapSize) {
    vertices.push_back(i);
    vertices.push_back(0.0f);
    vertices.push_back(-c_gridSize);
    vertices.push_back(i);
    vertices.push_back(0.0f);
    vertices.push_back(c_gridSize);

    vertices.push_back(-c_gridSize);
    vertices.push_back(0.0f);
    vertices.push_back(i);
    vertices.push_back(c_gridSize);
    vertices.push_back(0.0f);
    vertices.push_back(i);
  }

  return vertices;
}
