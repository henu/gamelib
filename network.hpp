#ifndef GAMELIB_NETWORK_HPP
#define GAMELIB_NETWORK_HPP

#include <Urho3D/Math/StringHash.h>

namespace GameLib
{

extern const Urho3D::StringHash E_TO_CLIENT_SET_CONTROLLED_NODE;

extern const Urho3D::StringHash P_ID;

extern const unsigned CTRL_FORWARD;
extern const unsigned CTRL_BACKWARD;
extern const unsigned CTRL_LEFT;
extern const unsigned CTRL_RIGHT;
extern const unsigned CTRL_JUMP;
extern const unsigned CTRL_CROUCH;
extern const unsigned CTRL_FIRE;
extern const unsigned CTRL_ANY_MOVEMENT_KEY;

}

#endif
