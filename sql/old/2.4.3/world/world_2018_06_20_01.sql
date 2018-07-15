REPLACE INTO spell_template_override (entry, effect2, Comment) VALUES (768, 0, "Removed effect 2 triggering spell 0 to silence errors");
-- duplicate spell affect
DELETE FROM spell_affect WHERE entry = 27790;