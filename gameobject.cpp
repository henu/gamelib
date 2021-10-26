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

void GameObject::finishCreation()
{
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

void GameObject::handleAddedToClient()
{
}

bool GameObject::handleHitscan(Urho3D::Vector3 const& pos, Urho3D::Vector3 const& dir)
{
    (void)pos;
    (void)dir;
    return false;
}

Urho3D::Matrix3x4 GameObject::getCameraTransform() const
{
    return Urho3D::Matrix3x4::IDENTITY;
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
        for (unsigned j = 0; j < node->GetNumComponents(); ++ j) {
            Urho3D::Component* component = node->GetComponents()[j];
            GameLib::GameObject* gameobj = dynamic_cast<GameLib::GameObject*>(component);
            if (gameobj && gameobj != this && gameobj->handleHitscan(ray_hit.position_, ray.direction_)) {
                result_hitpos = ray_hit.position_;
                return true;
            }
        }
    }

    return false;
}

}
