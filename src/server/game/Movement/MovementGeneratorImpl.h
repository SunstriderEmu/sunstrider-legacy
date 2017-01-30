
#ifndef TRINITY_MOVEMENTGENERATOR_IMPL_H
#define TRINITY_MOVEMENTGENERATOR_IMPL_H

class MovementGenerator;

template<class MOVEMENT_GEN>
inline MovementGenerator*
MovementGeneratorFactory<MOVEMENT_GEN>::Create(void * /*data*/) const
{
    return (new MOVEMENT_GEN());
}
#endif
