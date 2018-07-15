-- Fixed Holy Nova (healing) not affected by Holy Reach talent
UPDATE spell_affect SET SpellFamilyMask = SpellFamilyMask | 0x8000000 WHERE entry = 27789 AND effectId = 1;