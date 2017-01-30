
#ifndef _FOLLOWERREFERENCE_H
#define _FOLLOWERREFERENCE_H

#include "RefManager.h"

class TargetedMovementGeneratorBase;
class Unit;

class TC_GAME_API FollowerReference : public Reference<Unit, TargetedMovementGeneratorBase>
{
    protected:
        void targetObjectBuildLink() override;
        void targetObjectDestroyLink() override;
        void sourceObjectDestroyLink() override;
};
#endif
