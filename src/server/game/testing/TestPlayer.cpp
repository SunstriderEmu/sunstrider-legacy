#include "TestPlayer.h"

bool TestPlayer::Create(uint32 guidlow, CharacterCreateInfo* createInfo)
{
    return Player::Create(guidlow, createInfo->Name, createInfo->Race, createInfo->Class, createInfo->Gender, createInfo->Skin, createInfo->Face, createInfo->HairStyle, createInfo->HairColor, createInfo->FacialHair, createInfo->OutfitId, false);
}
