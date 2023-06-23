#include "spectatorghost.hpp"

#include "network.hpp"

#include <Urho3D/Scene/Node.h>

namespace GameLib
{

float const MOVEMENT_SPEED = 20;

SpectatorGhost::SpectatorGhost(Urho3D::Context* context) :
    GameObject(context)
{
}

bool SpectatorGhost::runServerSide(float deltatime, Urho3D::Controls const* controls)
{
    Urho3D::Node* node = GetNode();

    if (controls) {
        node->SetRotation(Urho3D::Quaternion(controls->yaw_, Urho3D::Vector3::UP) * Urho3D::Quaternion(controls->pitch_, Urho3D::Vector3::RIGHT));

        // Find out what direction the player would like to move
        Urho3D::Vector3 movement = Urho3D::Vector3::ZERO;
        if (controls->IsDown(CTRL_FORWARD) && !controls->IsDown(CTRL_BACKWARD)) {
            movement.x_ += Urho3D::Sin(controls->yaw_) * Urho3D::Cos(controls->pitch_);
            movement.y_ += -Urho3D::Sin(controls->pitch_);
            movement.z_ += Urho3D::Cos(controls->yaw_) * Urho3D::Cos(controls->pitch_);
        } else if (controls->IsDown(CTRL_BACKWARD) && !controls->IsDown(CTRL_FORWARD)) {
            movement.x_ += -Urho3D::Sin(controls->yaw_) * Urho3D::Cos(controls->pitch_);
            movement.y_ += Urho3D::Sin(controls->pitch_);
            movement.z_ += -Urho3D::Cos(controls->yaw_) * Urho3D::Cos(controls->pitch_);
        }
        if (controls->IsDown(CTRL_RIGHT) && !controls->IsDown(CTRL_LEFT)) {
            movement.x_ += Urho3D::Cos(controls->yaw_);
            movement.z_ += -Urho3D::Sin(controls->yaw_);
        } else if (controls->IsDown(CTRL_LEFT) && !controls->IsDown(CTRL_RIGHT)) {
            movement.x_ += -Urho3D::Cos(controls->yaw_);
            movement.z_ += Urho3D::Sin(controls->yaw_);
        }
        if (controls->IsDown(CTRL_JUMP) && !controls->IsDown(CTRL_CROUCH)) {
            movement.y_ += 1;
        } else if (controls->IsDown(CTRL_CROUCH) && !controls->IsDown(CTRL_JUMP)) {
            movement.y_ -= 1;
        }
        float movement_length = movement.Length();
        if (movement_length > 0) {
            movement /= movement_length;
            movement *= MOVEMENT_SPEED * deltatime;
            node->SetPosition(node->GetPosition() + movement);
        }
    }

    return true;
}

void SpectatorGhost::modifyControls(Urho3D::Controls* controls) const
{
    controls->pitch_ = Urho3D::Clamp(controls->pitch_, -90.0f, 90.0f);
}

Urho3D::Matrix3x4 SpectatorGhost::getCameraTransform(Urho3D::Controls const* controls) const
{
    return Urho3D::Matrix3x4(
        GetNode()->GetPosition(),
        Urho3D::Quaternion(controls->yaw_, Urho3D::Vector3::UP) * Urho3D::Quaternion(controls->pitch_, Urho3D::Vector3::RIGHT),
        1
    );
}

void SpectatorGhost::registerObject(Urho3D::Context* context)
{
    context->RegisterFactory<SpectatorGhost>();
}

}
