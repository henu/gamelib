#include "network.hpp"

namespace GameLib
{

const Urho3D::StringHash E_TO_CLIENT_SET_CONTROLLED_NODE("set_controlled_node");

const Urho3D::StringHash P_ID("id");

const unsigned CTRL_FORWARD = 1;
const unsigned CTRL_BACKWARD = 2;
const unsigned CTRL_LEFT = 4;
const unsigned CTRL_RIGHT = 8;
const unsigned CTRL_FIRE = 16;

}
