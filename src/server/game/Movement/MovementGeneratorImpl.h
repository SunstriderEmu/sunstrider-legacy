
#ifndef TRINITY_MOVEMENTGENERATOR_IMPL_H
#define TRINITY_MOVEMENTGENERATOR_IMPL_H

class MovementGenerator;

template<class Movement>
inline MovementGenerator* MovementGeneratorFactory<Movement>::Create(void * /*data*/) const
{
    return (new Movement());
}
#endif
