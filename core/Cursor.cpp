#include "Cursor.hpp"
#include "MatrixUtils.hpp"
#include <GL/glew.h>

Cursor::Cursor()
    : m_position{0.0f, 0.0f, 0.0f}, m_rotation{math137::MatrixUtils::Identity()} {
        recalculateModelMatrix();
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        auto vertices = generateVertices();
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(math137::Vector3f), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(math137::Vector3f), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        auto indices = generateIndices();
        m_indexCount = static_cast<uint32_t>(indices.size());
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
        glBindVertexArray(0);
    }

Cursor::~Cursor()
{
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
    glDeleteVertexArrays(1, &m_vao);
}

void Cursor::recalculateModelMatrix() {
    m_model = math137::MatrixUtils::Translate(m_position.x(), m_position.y(), m_position.z()) * m_rotation;
}

void Cursor::render(std::unique_ptr<Renderer> &renderer)
{
    recalculateModelMatrix();
    renderer->setModel(m_model);
    renderer->setShader(ShaderType::OBJECT);
    glBindVertexArray(m_vao);
    // Z axis (original) - blue
    renderer->setColor(math137::Vector4f{0.0f, 0.0f, 1.0f, 1.0f});
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);

    // Y axis - green (rotate cylinder so its length points along +Y)
    {
        auto rotY = math137::MatrixUtils::RotateX(-static_cast<float>(M_PI_2));
        renderer->setModel(m_model * rotY);
        renderer->setColor(math137::Vector4f{0.0f, 1.0f, 0.0f, 1.0f});
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    }

    // X axis - red (rotate cylinder so its length points along +X)
    {
        auto rotX = math137::MatrixUtils::RotateY(static_cast<float>(M_PI_2));
        renderer->setModel(m_model * rotX);
        renderer->setColor(math137::Vector4f{1.0f, 0.0f, 0.0f, 1.0f});
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    }
    //glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
}

std::vector<math137::Vector3f> Cursor::generateVertices()
{
    std::vector<math137::Vector3f> vertices;
    vertices.reserve((radiusSegments + 1) * 2);

    for (uint16_t i = 0; i < 2; ++i) {
        float z = (cursorLength * i);
        for (uint16_t j = 0; j <= radiusSegments; ++j) {
            float angle = (2.0f * M_PI * j) / radiusSegments;
            float x = cursorRadius * cosf(angle);
            float y = cursorRadius * sinf(angle);
            vertices.emplace_back(x, y, z);
        }
    }

    return vertices;
}


std::vector<uint32_t> Cursor::generateIndices() {
    std::vector<uint32_t> indices;
    indices.reserve(radiusSegments * 2 * 6);

    for (uint16_t i = 0; i < 2; ++i) {
        for (uint16_t j = 0; j < radiusSegments; ++j) {
            uint32_t first = (i * (radiusSegments + 1)) + j;
            uint32_t second = first + radiusSegments + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    return indices;
}