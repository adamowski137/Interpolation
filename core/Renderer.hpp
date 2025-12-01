#pragma once

#include "Shader.hpp"
#include "Vector.hpp"
#include <cstdint>

class Object;

class Renderer {
public:
  Renderer();
  void setProjection(const math137::Matrix4f &projection);
  void setView(const math137::Matrix4f &view);
  void setModel(const math137::Matrix4f &model);
  void setShader(const ShaderType type);
  void setColor(const math137::Vector4f &color);
  void setDegree(uint8_t degree);
  void setBlockData(float sizeX, float sizeY, float sizeZ);
  void setCamerPos(const math137::Vector3f &pos);

private:
  Shader *m_selectedShader;
  Shader m_objectShader;
  ShaderType m_type;
};
