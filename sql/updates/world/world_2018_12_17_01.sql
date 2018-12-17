-- Remove useless Pyroclasm change
DELETE FROM spell_template_override WHERE entry = 18073;

REPLACE INTO spell_script_names VALUES (-18119, "spell_warl_aftermath");
