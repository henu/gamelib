#include "network.hpp"

namespace GameLib
{

const Urho3D::StringHash E_TO_CLIENT_SET_CONTROLLED_NODE("set_controlled_node");

const Urho3D::StringHash P_ID("id");

const unsigned CTRL_FORWARD = 0x01;
const unsigned CTRL_BACKWARD = 0x02;
const unsigned CTRL_LEFT = 0x04;
const unsigned CTRL_RIGHT = 0x08;
const unsigned CTRL_JUMP = 0x10;
const unsigned CTRL_CROUCH = 0x20;
const unsigned CTRL_FIRE = 0x40;
const unsigned CTRL_ANY_MOVEMENT_KEY = 0x3f;

}
