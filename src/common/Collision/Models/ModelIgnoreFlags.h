
#ifndef ModelIgnoreFlags_h__
#define ModelIgnoreFlags_h__

#include "Define.h"

namespace VMAP
{
enum class ModelIgnoreFlags : uint32
{
    Nothing = 0x00,
    M2      = 0x01
};

inline ModelIgnoreFlags operator&(ModelIgnoreFlags left, ModelIgnoreFlags right) { return ModelIgnoreFlags(uint32(left) & uint32(right)); }
}

#endif // ModelIgnoreFlags_h__