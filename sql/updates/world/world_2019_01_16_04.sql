REPLACE INTO spell_script_names VALUES (-23881, "spell_warr_bloodthirst");

-- proc spell ranks
REPLACE INTO spell_ranks VALUES (23885, 23885, 1);
REPLACE INTO spell_ranks VALUES (23885, 23886, 2);
REPLACE INTO spell_ranks VALUES (23885, 23887, 3);
REPLACE INTO spell_ranks VALUES (23885, 23888, 4);
REPLACE INTO spell_ranks VALUES (23885, 25252, 5);
REPLACE INTO spell_ranks VALUES (23885, 30339, 6);

-- Add bloodthirst to Impale talent
UPDATE spell_affect SET SpellFamilyMask = SpellFamilyMask | 0x40000000000 WHERE entry IN (16493);
DELETE FROM spell_affect WHERE entry = 16494;