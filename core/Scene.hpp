#pragma once
#include "Cursor.hpp"
#include "Ground.hpp"

class Scene {
public:
    Scene(bool quat);
    void update(float dt);
    void render(std::unique_ptr<Renderer>& renderer);
    void renderMenu();
    void renderSamples(std::unique_ptr<Renderer>& renderer, int intermediateFrames);
    inline void setStartPosition(const math137::Vector3f& pos) { m_startPos = pos; }
    inline void setStartQuaternion(const math137::Quaternion& rot) { m_startQuat = rot; }
    inline void setEndEuler(const math137::Vector3f& rot) { m_endEuler = rot; }
    inline void setEndPosition(const math137::Vector3f& pos) { m_endPos = pos; }
    inline void setEndQuaternion(const math137::Quaternion& rot) { m_endQuat = rot; }
    inline void setStartEuler(const math137::Vector3f& rot) { m_startEuler = rot; }
    inline void setT(float t) { m_t = t; }
    void start();
    inline void setUseSphericalInterpolation(bool v) { m_useSpherical = v; }

private:
    Cursor m_cursor;
    Ground m_ground;

    // interpolation helpers
    void interpolateLinear(float alpha);
    void interpolateSpherical(float alpha);
    void interpolateEuler(float alpha);

    math137::Vector3f m_startPos{0.0f, 0.0f, 0.0f};
    math137::Vector3f m_startEuler{0.0f, 0.0f, 0.0f};
    math137::Quaternion m_startQuat{1.0f, 0.0f, 0.0f, 0.0f};
    math137::Vector3f m_endPos{0.0f, 0.0f, 0.0f};
    math137::Vector3f m_endEuler{0.0f, 0.0f, 0.0f};
    math137::Quaternion m_endQuat{1.0f, 0.0f, 0.0f, 0.0f};
    float m_t{0.0f};
    float m_elapsedTime{0.0f};
    bool m_useSpherical{false};
    bool m_useQuat{true};
};