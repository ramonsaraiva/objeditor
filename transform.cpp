#include "transform.h"

Transform::Transform(float* trans, float* rot, float* sca, float ai)
{
    translate = new float[3];
    rotate = new float[3];
    scale = new float[3];

    translate[0] = trans[0];
    translate[1] = trans[1];
    translate[2] = trans[2];

    rotate[0] = rot[0];
    rotate[1] = rot[1];
    rotate[2] = rot[2];

    scale[0] = sca[0];
    scale[1] = sca[1];
    scale[2] = sca[2];

    angle_inc = ai;
}

void Transform::set_translate(float* trans)
{
    for (int i = 0; i < 3; i++)
        translate[i] = trans[i];
}

void Transform::set_rotate(float* rot)
{
    for (int i = 0; i < 3; i++)
        rotate[i] = rot[i];
}

void Transform::set_scale(float* sca)
{
    for (int i = 0; i < 3; i++)
        scale[i] = sca[i];
}

void Transform::set_angle_inc(float ai)
{
    angle_inc = ai;
}

float* Transform::get_translate()
{
    return translate;
}

float* Transform::get_rotate()
{
    return rotate;
}

float* Transform::get_scale()
{
    return scale;
}

void Transform::toggle_rotation()
{
    rotation = !rotation;
}

bool Transform::is_rotating()
{
    return rotation == true;
}

float Transform::get_rotation_angle()
{
    return rotation_angle;
}

void Transform::inc_rotation_angle()
{
    rotation_angle += angle_inc;
}
