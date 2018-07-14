-- remove some wrong server spells
DELETE FROM spell_template_override WHERE entry IN (30023, 40200);