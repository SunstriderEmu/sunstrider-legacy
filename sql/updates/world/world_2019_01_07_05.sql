REPLACE INTO spell_script_names VALUES (-8253, "spell_sha_flametongue_totem_proc");

REPLACE INTO spell_ranks VALUES (8253, 8253, 1);
REPLACE INTO spell_ranks VALUES (8253, 8248, 2);
REPLACE INTO spell_ranks VALUES (8253, 10523, 3);
REPLACE INTO spell_ranks VALUES (8253, 16389, 4);
REPLACE INTO spell_ranks VALUES (8253, 25555, 5);

REPLACE INTO spell_template_override (entry, comment, AttributesEx3) VALUES (16368, "Add SPELL_ATTR3_NO_DONE_BONUS + SPELL_ATTR3_CANT_TRIGGER_PROC to Flametongue attack (totem)", 0x20010000);