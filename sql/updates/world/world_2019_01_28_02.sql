-- NPC shouldn't have this path
DELETE FROM creature_addon WHERE spawnID = 135306;

-- Malchezaar's Shadow Nova uninterruptible
INSERT IGNORE INTO spell_template_override (entry, interruptFlags) VALUES (30852, 0);

-- Brute Lair now immune to stuns and snares
UPDATE creature_template SET mechanic_immune_mask = mechanic_immune_mask | 0xC01 WHERE entry = 19389;