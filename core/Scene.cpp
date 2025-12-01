#include "Scene.hpp"
#include <imgui.h>
#include <cmath>
#include <MatrixUtils.hpp>
#include <iostream>

Scene::Scene(bool quat)
    : m_cursor(), m_ground(), m_useQuat(quat)
{
}

void Scene::update(float dt)
{
    if (m_t <= 0.0f)
        return; // nothing to do if duration is zero or negative

    // advance elapsed time
    m_elapsedTime += dt;
    float alpha = m_elapsedTime / m_t;
    if (alpha < 0.0f)
        alpha = 0.0f;
    if (alpha > 1.0f)
        alpha = 1.0f;
    // call the selected interpolation method
    if (!m_useQuat)
    {
        interpolateEuler(alpha);
    }
    else if (m_useSpherical)
    {
        interpolateSpherical(alpha);
    }
    else
    {
        interpolateLinear(alpha);
    }

    // clamp elapsed to duration
    if (m_elapsedTime >= m_t)
    {
        m_elapsedTime = m_t;
    }
}

void Scene::interpolateLinear(float alpha)
{
    // linear position interpolation
    math137::Vector3f pos;
    pos.x(m_startPos.x() + (m_endPos.x() - m_startPos.x()) * alpha);
    pos.y(m_startPos.y() + (m_endPos.y() - m_startPos.y()) * alpha);
    pos.z(m_startPos.z() + (m_endPos.z() - m_startPos.z()) * alpha);
    m_cursor.setPosition(pos);
    
    math137::Quaternion q1 = m_startQuat;
    math137::Quaternion q2 = m_endQuat;

    // compute dot product
    float dot = q1.a * q2.a + q1.b * q2.b + q1.c * q2.c + q1.d * q2.d;

    // if dot < 0, the quaternions have opposite handed-ness and slerp won't take
    // the shorter path. Fix by reversing one quaternion.
    if (dot < 0.0f)
    {
        dot = -dot;
        q2.a = -q2.a;
        q2.b = -q2.b;
        q2.c = -q2.c;
        q2.d = -q2.d;
    }
    
    // rotation: normalized linear interpolation (nlerp)
    math137::Quaternion qr = q1 * (1.0f - alpha) + q2 * alpha;
    qr.normalize();
    m_cursor.setRotation(math137::MatrixUtils::FromQuaternion(qr));
}

void Scene::interpolateSpherical(float alpha)
{
    // position still interpolated linearly
    math137::Vector3f pos;
    pos.x(m_startPos.x() + (m_endPos.x() - m_startPos.x()) * alpha);
    pos.y(m_startPos.y() + (m_endPos.y() - m_startPos.y()) * alpha);
    pos.z(m_startPos.z() + (m_endPos.z() - m_startPos.z()) * alpha);
    m_cursor.setPosition(pos);

    // rotation: spherical linear interpolation (slerp)
    math137::Quaternion q1 = m_startQuat;
    math137::Quaternion q2 = m_endQuat;

    // compute dot product
    float dot = q1.a * q2.a + q1.b * q2.b + q1.c * q2.c + q1.d * q2.d;

    // if dot < 0, the quaternions have opposite handed-ness and slerp won't take
    // the shorter path. Fix by reversing one quaternion.
    if (dot < 0.0f)
    {
        dot = -dot;
        q2.a = -q2.a;
        q2.b = -q2.b;
        q2.c = -q2.c;
        q2.d = -q2.d;
    }

    math137::Quaternion qr;
    // standard slerp
    float theta_0 = acosf(dot);    // angle between input quaternions
    float theta = theta_0 * alpha; // angle at t

    // q3 = (q2 - q1 * dot) normalized
    math137::Quaternion q3 = q2 + (q1 * (-dot));
    q3.normalize();

    float s0 = cosf(theta);
    float s1 = sinf(theta);

    qr = q1 * s0 + q3 * s1;
    qr.normalize();

    m_cursor.setRotation(math137::MatrixUtils::FromQuaternion(qr));
}

void Scene::interpolateEuler(float alpha)
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
    // position still interpolated linearly
    math137::Vector3f pos;
    pos.x(m_startPos.x() + (m_endPos.x() - m_startPos.x()) * alpha);
    pos.y(m_startPos.y() + (m_endPos.y() - m_startPos.y()) * alpha);
    pos.z(m_startPos.z() + (m_endPos.z() - m_startPos.z()) * alpha);
    m_cursor.setPosition(pos);

    // interpolate Euler angles component-wise with wrap-around handling
    auto interpAngle = [](float a0, float a1, float t)
    {
        float delta = a1 - a0;
        delta = fmodf(delta + M_PI, 2.0f * static_cast<float>(M_PI));
        if (delta < 0)
            delta += 2.0f * static_cast<float>(M_PI);
        delta -= static_cast<float>(M_PI);

        return a0 + delta * t;
    };

    float ax = interpAngle(m_startEuler.x(), m_endEuler.x(), alpha);
    float ay = interpAngle(m_startEuler.y(), m_endEuler.y(), alpha);
    float az = interpAngle(m_startEuler.z(), m_endEuler.z(), alpha);
    std::cout << "Interpolated Euler angles: (" << ax << ", " << ay << ", " << az << ")\n";

    m_cursor.setRotation(math137::MatrixUtils::RotateZ(az) *
                         math137::MatrixUtils::RotateY(ay) *
                         math137::MatrixUtils::RotateX(ax));
}

void Scene::renderSamples(std::unique_ptr<Renderer> &renderer, int intermediateFrames)
{
    if (intermediateFrames < 0)
        intermediateFrames = 0;
    int totalSamples = intermediateFrames + 2; // include start and end

    // save current cursor transform
    math137::Vector3f savedPos = m_cursor.getPosition();
    math137::Matrix4f savedRot = m_cursor.getRotation();

    for (int i = 0; i < totalSamples; ++i)
    {
        float alpha = 0.0f;
        if (totalSamples > 1)
            alpha = static_cast<float>(i) / static_cast<float>(totalSamples - 1);

        // apply chosen interpolation to set m_cursor
        if (!m_useQuat)
            interpolateEuler(alpha);
        else if (m_useSpherical)
            interpolateSpherical(alpha);
        else
            interpolateLinear(alpha);

        // render the cursor at this sample
        m_cursor.render(renderer);
    }

    // restore saved cursor transform
    m_cursor.setPosition(savedPos);
    m_cursor.setRotation(savedRot);
}

void Scene::render(std::unique_ptr<Renderer> &renderer)
{
    m_cursor.render(renderer);
    m_ground.render(renderer);
}

void Scene::start()
{
    m_elapsedTime = 0.0f;
    m_cursor.setPosition(m_startPos);
    if(m_useQuat)
        m_cursor.setRotation(math137::MatrixUtils::FromQuaternion(m_startQuat));
    else
        m_cursor.setRotation(math137::MatrixUtils::RotateZ(m_startEuler.z()) *
                             math137::MatrixUtils::RotateY(m_startEuler.y()) *
                             math137::MatrixUtils::RotateX(m_startEuler.x()));
}
