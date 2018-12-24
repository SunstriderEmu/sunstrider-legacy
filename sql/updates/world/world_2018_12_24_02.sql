-- Delete higher ranks of Hunter Pet Frenzy
DELETE FROM spell_affect WHERE entry IN (19625, 19624, 19623, 19622);
UPDATE spell_affect SET SpellFamilyMask = 0x80000000 WHERE entry = 19621;