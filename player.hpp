#ifndef GAME_PLAYER_HPP
#define GAME_PLAYER_HPP

#include <Urho3D/Container/RefCounted.h>
#include <Urho3D/Network/Connection.h>

namespace GameLib
{

struct Player : public Urho3D::RefCounted
{
    unsigned controlled_node_id;
    time_t respawn_at;

    Urho3D::Connection* conn;

    inline Player(Urho3D::Connection* conn) :
        controlled_node_id(0),
        respawn_at(0),
        conn(conn)
    {
    }
};

}

#endif
