#ifndef GAMELIB_GAMEOBJECT_HPP
#define GAMELIB_GAMEOBJECT_HPP

#include "shape.hpp"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Scene/Component.h>

namespace GameLib
{

class GameObject : public Urho3D::Component
{
    URHO3D_OBJECT(GameObject, Urho3D::Component);

public:

    GameObject(Urho3D::Context* context);
    virtual ~GameObject();

    virtual void finishCreation(bool enable_physics = true);

    // Return false if GameObject should be destroyed
    virtual bool runServerSide(float deltatime, Urho3D::Controls const* controls);

    // Return false if GameObject should be destroyed
    virtual bool runClientSide(float deltatime);

    virtual void handleAddedToClient();

    virtual bool handleHitscan(Urho3D::Vector3 const& pos, Urho3D::Vector3 const& dir);

    virtual void handleExplosion(Urho3D::Vector3 const& pos);

    virtual bool receiveDecals() const;

    virtual Urho3D::Matrix3x4 getCameraTransform() const;

    virtual Shape getPlacementShape() const;

    bool hitscan(Urho3D::Vector3& result_hitpos, Urho3D::Ray const& ray);

    void explosion(Urho3D::Vector3 const& pos);

private:

};

}

#endif
