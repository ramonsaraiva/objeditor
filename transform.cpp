#include "transform.h"

Transform::Transform(float* trans, float* rot, float* sca)
{
    translate = trans;
    rotate = rot;
    scale = sca;
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
