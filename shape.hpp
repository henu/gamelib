#ifndef GAMELIB_SHAPE_HPP
#define GAMELIB_SHAPE_HPP

#include <Urho3D/Container/Vector.h>
#include <Urho3D/Math/Vector3.h>

namespace GameLib
{

class Shape
{

public:

    enum Type {
        NOTHING,
        DOT,
        CYLINDER,
        BOX
    };

    inline Shape() :
        type(NOTHING)
    {
    }

    inline Type getType() const
    {
        return type;
    }

    inline void setDot()
    {
        type = DOT;
        data.Clear();
    }

    inline void setCylinder(float height, float diameter)
    {
        type = CYLINDER;
        data.Clear();
        data.Push(height);
        data.Push(diameter);
    }

    inline void setBox(Urho3D::Vector3 const& size)
    {
        type = BOX;
        data.Clear();
        data.Push(size.x_);
        data.Push(size.y_);
        data.Push(size.z_);
    }

    inline Urho3D::Vector3 positionAtNormal(Urho3D::Vector3 dir) const
    {
        dir.Normalize();

        if (type == CYLINDER) {
            if (dir.y_ > Urho3D::Sin(89.0)) {
                return Urho3D::Vector3::UP * data[0] / 2;
            }
            if (dir.y_ > Urho3D::Sin(1.0)) {
                Urho3D::Vector2 dir_xz(dir.x_, dir.z_);
                dir_xz.Normalize();
                return Urho3D::Vector3(dir_xz.x_ * data[1] / 2, data[0] / 2, dir_xz.y_ * data[1] / 2);
            }
            if (dir.y_ < Urho3D::Sin(-1.0)) {
                dir.y_ = 0;
                dir.Normalize();
                return dir * data[1] / 2;
            }
            if (dir.y_ > Urho3D::Sin(-89.0)) {
                Urho3D::Vector2 dir_xz(dir.x_, dir.z_);
                dir_xz.Normalize();
                return Urho3D::Vector3(dir_xz.x_ * data[1] / 2, data[0] / 2, dir_xz.y_ * data[1] / 2);
            }
            return Urho3D::Vector3::UP * -data[0] / 2;
        }

        if (type == BOX) {
            // Easy cases
            if (dir.x_ > Urho3D::Sin(89.0)) return Urho3D::Vector3::RIGHT * data[0] / 2;
            if (dir.x_ < Urho3D::Sin(-89.0)) return Urho3D::Vector3::RIGHT * -data[0] / 2;
            if (dir.y_ > Urho3D::Sin(89.0)) return Urho3D::Vector3::UP * data[1] / 2;
            if (dir.y_ < Urho3D::Sin(-89.0)) return Urho3D::Vector3::UP * -data[1] / 2;
            if (dir.z_ > Urho3D::Sin(89.0)) return Urho3D::Vector3::FORWARD * data[2] / 2;
            if (dir.z_ < Urho3D::Sin(-89.0)) return Urho3D::Vector3::FORWARD * -data[2] / 2;
// TODO: Code edge and corner cases as well!
        }

        return Urho3D::Vector3::ZERO;
    }

private:

    Type type;

    Urho3D::Vector<float> data;
};

}

#endif
