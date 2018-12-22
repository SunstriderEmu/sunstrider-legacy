-- wrong dead mines npcs
DELETE c, ce FROM creature c JOIN creature_entry ce ON c.spawnId = ce.spawnId WHERE c.spawnId IN (1098416, 1098417);
UPDATE creature_template SET minlevel = 14, maxlevel = 16 WHERE entry = 1729;
UPDATE creature_template SET minlevel = 16, maxlevel = 17 WHERE entry = 1725;
UPDATE creature_template SET minlevel = 19, maxlevel = 19 WHERE entry = 3586;
UPDATE creature_template SET minlevel = 17, maxlevel = 19 WHERE entry = 4417;
