-- Fixed shadow reach talent not affecting SWD
UPDATE spell_affect SET SpellFamilyMask = SpellFamilyMask | 0x200000000 WHERE entry = 17323 AND effectId = 0;