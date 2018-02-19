#ifndef _GROUPREFMANAGER
#define _GROUPREFMANAGER

#include "RefManager.h"

class Group;
class Player;
class GroupReference;

class TC_GAME_API GroupRefManager : public RefManager<Group, Player>
{
    public:
        GroupReference* getFirst() { return ((GroupReference* ) RefManager<Group, Player>::getFirst()); }
        GroupReference const* getFirst() const { return ((GroupReference const*)RefManager<Group, Player>::getFirst()); }
};
#endif

