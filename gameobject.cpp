#include "gameobject.hpp"

#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Scene/Scene.h>

namespace GameLib
{

GameObject::GameObject(Urho3D::Context* context) :
    Urho3D::Component(context)
{
}

GameObject::~GameObject()
{
}

void GameObject::finishCreation(App* app, bool enable_physics, Urho3D::VariantMap* data)
{
    this->app = app;
    handleCreated(enable_physics, data);
}

bool GameObject::runServerSide(float deltatime, Urho3D::Controls const* controls)
{
    (void)deltatime;
    (void)controls;
    return true;
}

bool GameObject::runClientSide(float deltatime)
{
    (void)deltatime;
    return true;
}

void GameObject::handleCreated(bool enable_physics, Urho3D::VariantMap* data)
{
    (void)enable_physics;
    (void)data;
}

void GameObject::handleAddedToClient()
{
}

bool GameObject::handleHitscan(Urho3D::Vector3 const& pos, Urho3D::Vector3 const& dir)
{
    (void)pos;
    (void)dir;
    return false;
}

void GameObject::handleExplosion(Urho3D::Vector3 const& pos)
{
    (void)pos;
}

bool GameObject::receiveDecals() const
{
    return false;
}

Urho3D::Matrix3x4 GameObject::getCameraTransform() const
{
    return Urho3D::Matrix3x4::IDENTITY;
}

Shape GameObject::getPlacementShape() const
{
    return Shape();
}

bool GameObject::hitscan(Urho3D::Vector3& result_hitpos, Urho3D::Ray const& ray)
{
    // Do ray query
    Urho3D::PODVector<Urho3D::RayQueryResult> ray_hits;
    Urho3D::RayOctreeQuery query(ray_hits, ray, Urho3D::RAY_TRIANGLE, Urho3D::M_INFINITY, Urho3D::DRAWABLE_GEOMETRY);
    Urho3D::Octree* octree = GetScene()->GetComponent<Urho3D::Octree>();
	octree->Raycast(query);

    // Iterate hit objects
    for (unsigned i = 0; i < ray_hits.Size(); ++ i) {
        Urho3D::RayQueryResult const& ray_hit = ray_hits[i];
        Urho3D::Node* node = ray_hit.drawable_->GetNode();
        while (node != GetScene()) {
            for (unsigned j = 0; j < node->GetNumComponents(); ++ j) {
                Urho3D::Component* component = node->GetComponents()[j];
                GameLib::GameObject* gameobj = dynamic_cast<GameLib::GameObject*>(component);
                if (gameobj && gameobj != this && gameobj->handleHitscan(ray_hit.position_, ray.direction_)) {
                    result_hitpos = ray_hit.position_;
                    return true;
                }
            }
            node = node->GetParent();
        }
    }

    return false;
}

void GameObject::explosion(Urho3D::Vector3 const& pos)
{
    // Iterate all objects in Scene
    Urho3D::PODVector<Urho3D::Node*> nodes = GetScene()->GetChildren(false);
    for (unsigned i = 0; i < nodes.Size(); ++ i) {
        Urho3D::Node* node = nodes[i];
        for (unsigned j = 0; j < node->GetNumComponents(); ++ j) {
            Urho3D::Component* component = node->GetComponents()[j];
            GameObject* gameobj = dynamic_cast<GameObject*>(component);
            if (gameobj) {
                gameobj->handleExplosion(pos);
            }
        }
    }
}

App* GameObject::getApp() const
{
    return app;
}

}
