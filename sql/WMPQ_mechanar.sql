DELETE FROM gameobject WHERE id IN (184632, 184322);
INSERT INTO gameobject(id, map, spawnMask, position_x, position_y, position_z, orientation, rotation0, rotation1, rotation2, rotation3, spawntimesecs, animprogress, state) VALUES
(184322, 554, 3, 242.876, 52.3013, 1.61459, 3.14856, 0, 0, 0.999994, -0.00348393, 25, 0, 1),
(184632, 554, 3, 236.556, 52.3209, 1.58961, 6.27947, 0, 0, 0.00185934, -0.999998, 25, 0, 1);
UPDATE gameobject_template SET flags = 4 WHERE entry IN (184322, 184632);

DELETE FROM eventai_scripts WHERE id IN (1921809, 1921908);
INSERT INTO eventai_scripts VALUES(1921809, 19218, 6, 0, 100, 6, 0, 0, 0, 0, 34, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Gatewatcher Gyro-kill (Both) - Set Instance Data on Death');
INSERT INTO eventai_scripts VALUES(1921908, 19219, 6, 0, 100, 6, 0, 0, 0, 0, 34, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Mecano-Lord Capacitus (Both) - Set Instance Data on Death');

UPDATE creature SET position_x = 88.3413, position_y = 19.2654, position_z = 14.9232, orientation = 1.0441, movementtype = 0 WHERE id = 19218;
UPDATE creature SET position_x = 145.7956, position_y = -76.5193, position_z = 7.6248, orientation = 3.0914, movementtype = 0 WHERE id = 19710;

UPDATE creature_template SET MovementType = 0 WHERE entry IN (19218, 19710);

-- SET DROP OF LEGION CACHE ON PATHALEON
-- Normal
DELETE FROM creature_loot_template WHERE entry = 19220 AND groupid = 2;
INSERT INTO creature_loot_template VALUES
(19220, 28248, 0, 2, 1, 1, 0, 0, 0),
(19220, 28249, 0, 2, 1, 1, 0, 0, 0),
(19220, 28250, 0, 2, 1, 1, 0, 0, 0),
(19220, 28251, 0, 2, 1, 1, 0, 0, 0),
(19220, 28252, 0, 2, 1, 1, 0, 0, 0);

-- Heroic
DELETE FROM creature_loot_template WHERE entry = 21537 AND groupid = 2;
INSERT INTO creature_loot_template VALUES
(21537, 28248, 0, 2, 1, 1, 0, 0, 0),
(21537, 28249, 0, 2, 1, 1, 0, 0, 0),
(21537, 28250, 0, 2, 1, 1, 0, 0, 0),
(21537, 28251, 0, 2, 1, 1, 0, 0, 0),
(21537, 28252, 0, 2, 1, 1, 0, 0, 0);
UPDATE creature_loot_template SET mincountOrRef = 2, maxcount = 2 WHERE entry = 21537 AND item = 29434;

-- RESTORE NORMAL DROPS
-- DELETE FROM creature_loot_template WHERE entry = 19220 AND groupid = 2;
-- DELETE FROM creature_loot_template WHERE entry = 21537 AND groupid = 2;
-- UPDATE creature_template SET mincountOrRef = 1, maxcount = 1 WHERE entry = 21537 AND item = 29434);

-- Despawn Cache
UPDATE gameobject SET spawnMask = 0 WHERE id IN (184465, 184849);

-- Capacitus sentences
INSERT INTO script_texts (entry, content_default, content_loc2, sound, type, language, emote, comment) VALUES
(-1554028, "You should split while you can.", "Degagez, tant que vous le pouvez.", 11162, 1, 0, 0, "Mechano-lord Capacitus - Aggro"),
(-1554029, "Damn, I'm good!", "Aaah qu'est-ce que j'suis bon !", 11164, 1, 0, 0, "Mechano-lord Capacitus - Player Kill 1"),
(-1554030, "Can't say I didn't warn you!", "Ne dites pas que je ne vous avais pas prevenus...", 11163, 1, 0, 0, "Mechano-lord Capacitus - Player Kill 2"),
(-1554031, "Bully!", "Brutes !", 11167, 1, 0, 0, "Mechano-lord Capacitus - Death"),
(-1554032, "Go ahead, gimme your best shot. I can take it!", "Allez-y, donnez-moi ce que vous avez de mieux ! J'peux encaisser !", 11166, 1, 0, 0, "Mechano-lord Capacitus - First Shield"),
(-1554033, "Think you can hurt me, huh? Think I'm afraid of you?!", "Vous croyez que vous pouvez me faire mal, hein ? Vous croyez que j'ai peur de vous ?!", 11165, 1, 0, 0, "Mechano-lord Capacitus - Second Shield");

UPDATE creature_template SET scriptname = "boss_mechanolord_capacitus" WHERE entry = 19219;

UPDATE creature_template SET faction_A = 14, faction_H = 14 WHERE entry = 20405;

-- AreaTrigger at the end of the instance
INSERT INTO areatrigger_scripts VALUES(4614, 'at_mechanar');
