REPLACE INTO spell_script_names VALUES (-8026, "spell_sha_flametongue_proc");

REPLACE INTO spell_ranks VALUES (8026, 8026, 1);
REPLACE INTO spell_ranks VALUES (8026, 8028, 2);
REPLACE INTO spell_ranks VALUES (8026, 8029, 3);
REPLACE INTO spell_ranks VALUES (8026, 10445, 4);
REPLACE INTO spell_ranks VALUES (8026, 16343, 5);
REPLACE INTO spell_ranks VALUES (8026, 16344, 6);
REPLACE INTO spell_ranks VALUES (8026, 25488, 7);

REPLACE INTO spell_template_override (entry, comment, AttributesEx3) VALUES (10444, "Add SPELL_ATTR3_NO_DONE_BONUS + SPELL_ATTR3_CANT_TRIGGER_PROC to Flametongue attack (totem)", 0x20010000);