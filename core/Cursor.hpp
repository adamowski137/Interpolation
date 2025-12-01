#pragma once

#include <Vector.hpp>
#include <Quaternion.hpp>
#include "Renderer.hpp"

class Cursor {
public:
    Cursor();
    ~Cursor();
    inline void setPosition(const math137::Vector3f& pos) { m_position = pos; }
    inline void setRotation(const math137::Matrix4f& rot) { m_rotation = rot; }
    inline math137::Vector3f getPosition() const { return m_position; }
    inline math137::Matrix4f getRotation() const { return m_rotation; }

    void recalculateModelMatrix();
    void render(std::unique_ptr<Renderer>& renderer);
private:
    std::vector<math137::Vector3f> generateVertices();
    std::vector<uint32_t> generateIndices();
    
   static constexpr float cursorRadius = 0.02f;
   static constexpr float cursorLength = 0.2f;
   static constexpr uint16_t radiusSegments = 16;

    math137::Matrix4f m_model;
    math137::Matrix4f m_rotation;
    math137::Vector3f m_position;
    uint32_t m_vao;
    uint32_t m_vbo;
    uint32_t m_ebo;
    uint32_t m_indexCount;
};