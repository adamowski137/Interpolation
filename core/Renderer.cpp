#include "Renderer.hpp"
#include "Matrix.hpp"
#include "Shader.hpp"
#include <cstdint>

Renderer::Renderer()
    : m_objectShader("shaders/base.vs", "shaders/base.fs"){
  m_selectedShader = &m_objectShader;
  m_type = ShaderType::OBJECT;
  m_selectedShader->use();
}

void Renderer::setProjection(const math137::Matrix4f &projection) {
  m_objectShader.use();
  m_objectShader.setMat4("projection", projection);
  m_selectedShader->use();
}

void Renderer::setView(const math137::Matrix4f &view) {
  m_objectShader.use();
  m_objectShader.setMat4("view", view);
  m_selectedShader->use();
}

void Renderer::setShader(const ShaderType type) {
  // if (m_type == type)
  //   return;
  m_type = type;
  switch (type) {
  case ShaderType::OBJECT:
    m_selectedShader = &m_objectShader;
    break;
  }
  m_selectedShader->use();
}

void Renderer::setDegree(uint8_t degree) {
  m_selectedShader->setInt("count", degree);
}

void Renderer::setColor(const math137::Vector4f &color) {
  m_selectedShader->setVec4("color", color);
}

void Renderer::setModel(const math137::Matrix4f &model) {
  m_selectedShader->setMat4("model", model);
}

void Renderer::setCamerPos(const math137::Vector3f &pos) {
  m_objectShader.use();
  m_objectShader.setVec3("cameraPos", pos);
  m_selectedShader->use();
}
