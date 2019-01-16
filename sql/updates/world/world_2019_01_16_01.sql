-- Add Tree of Life to Natural Shapeshifter talent
UPDATE spell_affect SET SpellFamilyMask = SpellFamilyMask | 0x1000000000000 WHERE entry IN (16833, 16834, 16835);