DELETE FROM spell_template_override WHERE entry = 37657;
REPLACE INTO spell_script_names VALUES (18350, "spell_item_lightning_capacitor");
REPLACE INTO spell_script_names VALUES (37657, "spell_item_lightning_capacitor_aura");
UPDATE spell_proc SET Cooldown = 2500 WHERE SpellId = 37657