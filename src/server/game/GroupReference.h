
#ifndef _GROUPREFERENCE_H
#define _GROUPREFERENCE_H

#include "Reference.h"

class Group;
class Player;

class TC_GAME_API GroupReference : public Reference<Group, Player>
{
    protected:
        uint8 iSubGroup;
        void targetObjectBuildLink() override;
        void targetObjectDestroyLink() override;
        void sourceObjectDestroyLink() override;
    public:
        GroupReference() : Reference<Group, Player>(), iSubGroup(0) {}
        ~GroupReference() override { unlink(); }
        GroupReference *next() { return (GroupReference*)Reference<Group, Player>::next(); }
        GroupReference const* next() const { return (GroupReference const*)Reference<Group, Player>::next(); }
        uint8 getSubGroup() const { return iSubGroup; }
        void setSubGroup(uint8 pSubGroup) { iSubGroup = pSubGroup; }
};
#endif

