#ifndef GAMELIB_SPECTATORGHOST_HPP
#define GAMELIB_SPECTATORGHOST_HPP

#include "gameobject.hpp"

namespace GameLib
{

class SpectatorGhost : public GameObject
{
    URHO3D_OBJECT(SpectatorGhost, GameLib::GameObject);

public:

    SpectatorGhost(Urho3D::Context* context);

    bool runServerSide(float deltatime, Urho3D::Controls const* controls) override;

    void modifyControls(Urho3D::Controls* controls) const override;

    Urho3D::Matrix3x4 getCameraTransform(Urho3D::Controls const* controls) const override;

    static void registerObject(Urho3D::Context* context);

private:

};

}

#endif
