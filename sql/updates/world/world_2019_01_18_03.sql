UPDATE spell_proc SET SpellTypeMask = 0x1 WHERE SpellId = 37657;
DELETE FROM spell_script_names WHERE spell_id = 18350;
UPDATE spell_template_override SET attributesEx3 = 0x20000000, Comment = "Added SPELL_ATTR3_NO_DONE_BONUS + MaxLevel set to 0 (from 6)" WHERE entry = 37661;