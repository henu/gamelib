#ifndef GAMELIB_GAMEOBJECT_HPP
#define GAMELIB_GAMEOBJECT_HPP

#include "shape.hpp"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Scene/Component.h>

namespace GameLib
{

class App;

class GameObject : public Urho3D::Component
{
    URHO3D_OBJECT(GameObject, Urho3D::Component);

public:

    GameObject(Urho3D::Context* context);
    virtual ~GameObject();

    // Sets if this GameObject is interested about physics collisions. This only
    // happens on server side and by default GameObjects are not interested.
    void setHandlesPhysicsCollisions(bool handles_physics_collisions);

    bool getHandlesPhysicsCollisions() const;

    // Called on client by GameState
    void setApp(App* app);

    // This is only called on server and in editor
    void finishCreation(App* app, bool enable_physics = true, Urho3D::VariantMap* data = NULL);

    // Return false if GameObject should be destroyed
    virtual bool runServerSide(float deltatime, Urho3D::Controls const* controls);

    // Return false if GameObject should be destroyed
    virtual bool runClientSide(float deltatime);

    // This is only called on server and in editor
    virtual void handleCreated(bool enable_physics, Urho3D::VariantMap* data);

    virtual void handleAddedToClient();

    virtual bool handleHitscan(Urho3D::Vector3 const& pos, Urho3D::Vector3 const& dir);

    virtual void handleExplosion(Urho3D::Vector3 const& pos);

    virtual void handlePhysicsCollision(Urho3D::Vector3 const& pos, Urho3D::Vector3 const& normal, float distance, Urho3D::Node* node, GameObject* obj);

    virtual bool receiveDecals() const;

    virtual void modifyControls(Urho3D::Controls* controls) const;

    virtual Urho3D::Matrix3x4 getCameraTransform(Urho3D::Controls const* controls) const;

    virtual Shape getPlacementShape() const;

    bool hitscan(Urho3D::Vector3& result_hitpos, Urho3D::Ray const& ray);

    void explosion(Urho3D::Vector3 const& pos);

protected:

    App* getApp() const;

private:

    App* app;

    bool handles_physics_collisions;
};

}

#endif
