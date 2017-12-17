#ifndef CREATUREAIIMPL_H
#define CREATUREAIIMPL_H

#include "Random.h"
#include "Define.h"
#include "InstanceScript.h"

class WorldObject;

#define HEROIC(n,h) (HeroicMode ? h : n)

template<typename First, typename Second, typename... Rest>
inline First const& RAND(First const& first, Second const& second, Rest const&... rest)
{
    std::reference_wrapper<typename std::add_const<First>::type> const pack[] = { first, second, rest... };
    return pack[urand(0, sizeof...(rest)+1)].get();
}

TC_GAME_API bool InstanceHasScript(WorldObject const* obj, char const* scriptName);

template <class AI, class T>
AI* GetInstanceAI(T* obj, char const* scriptName)
{
    if (InstanceScript::InstanceHasScript(obj, scriptName))
        return new AI(obj);

    return nullptr;
}

#endif

