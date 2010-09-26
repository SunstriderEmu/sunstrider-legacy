UPDATE creature_template SET scriptname = "npc_harrison_jones", npcflag = 1 WHERE entry = 24358;
UPDATE creature_template SET scriptname = "npc_amanishi_berserker", mechanic_immune_mask = 536872960 WHERE entry = 24374;
UPDATE creature_template SET mechanic_immune_mask = 617299807 WHERE entry = 23574;

UPDATE gameobject_template SET flags = 2, scriptname = "go_za_gong" WHERE entry = 187359;
UPDATE gameobject_template SET flags = 36 WHERE entry = 186728;
UPDATE gameobject_template SET flags = 36 WHERE entry = 186303;
UPDATE creature_template SET scriptname = "totem_halazzi" WHERE entry = 24224;
UPDATE creature_template SET mechanic_immune_mask = 617299807 WHERE entry = 24239;
UPDATE creature_template SET flags_extra = 3 WHERE entry BETWEEN 23877 AND 23880;

-- Ensure that hostages aren't spawned by default
UPDATE creature SET spawnmask = 0 WHERE id IN (23790, 23999, 24001, 24024);

-- Respawn Malacrass && Zul'jin && Akil'Zon
UPDATE creature SET spawnmask = 1 WHERE id IN (24239, 23863, 23574);

DELETE FROM gameobject WHERE id IN (186859, 186306);
INSERT INTO gameobject (id, map, spawnmask, position_x, position_y, position_z, orientation, rotation0, rotation1, rotation2, rotation3, spawntimesecs, animprogress, state) VALUES
(186859, 568, 1, 120.344, 732.22, 45.1125, 1.54132, 0, 0, 0.696608, 0.717452, 25, 0, 1),
(186306, 568, 1, 123.007, 916.043, 34.135, 4.75931, 0, 0, 0.690326, -0.723499, 25, 0, 1);
UPDATE gameobject_template SET flags = 36 WHERE entry = 186859;
UPDATE gameobject_template SET flags = 38 WHERE entry = 186306;

-- Massive Door at the beginning (opened by the gong)
DELETE FROM gameobject WHERE id = 186728 AND `map` = 568;
INSERT INTO gameobject (id, map, spawnmask, position_x, position_y, position_z, orientation, rotation2, rotation3, spawntimesecs, animprogress, state) VALUES (186728, 568, 1, 120.073, 1605.38, 62.7175, 3.12571, 0.999968, 0.00794148, 180, 100, 1);

UPDATE creature_template SET faction_A = 1890, faction_H = 1890, scriptname = "npc_amanishi_lookout" WHERE entry = 24175;
UPDATE creature_template SET faction_A = 1890, faction_H = 1890, scriptname = "npc_amanishi_tempest" WHERE entry = 24549;
UPDATE creature_template SET faction_A = 1890, faction_H = 1890 WHERE entry IN (24180, 24159, 24225);
UPDATE creature_template SET minlevel = 70, maxlevel = 70, minhealth = 4800, maxhealth = 4900, mindmg = 25, maxdmg = 225, attackpower = 875 WHERE entry = 24159;

-- Akil'Zon gauntlet mobs
DELETE FROM creature WHERE id IN (24175, 24179, 24180, 24549);
INSERT INTO creature (id, `map`, spawnmask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, movementtype) VALUES
(24175, 568, 1, 0, 0, 233.611, 1476.31, 25.9168, 3.36178, 1800, 0),
(24179, 568, 1, 0, 0, 111.692, 1417.37, -2.47524, 3.02313, 7200, 0),
(24179, 568, 1, 0, 0, 115.513, 1404.04, -7.85024, 1.6843, 7200, 0),
(24179, 568, 1, 22262, 2179, 251.828, 1365.61, 49.3204, 2.63251, 1800, 0),
(24179, 568, 1, 0, 2179, 226.402, 1390.56, 41.466, 1.61725, 1800, 0),
(24179, 568, 1, 0, 2179, 226.331, 1423, 30.4674, 1.67747, 1800, 0),
(24179, 568, 1, 22262, 2179, 281.218, 1370.77, 49.3209, 3.42031, 1800, 0),
(24180, 568, 1, 0, 0, 230.605, 1423.17, 30.5989, 1.61224, 1800, 0),
(24180, 568, 1, 0, 0, 231.121, 1390.78, 41.5298, 1.56708, 1800, 0),
(24180, 568, 1, 0, 0, 253.586, 1378.85, 49.3204, 3.19952, 1800, 0),
(24180, 568, 1, 0, 0, 277.734, 1385.05, 49.3204, 3.38017, 1800, 0),
(24549, 568, 1, 0, 0, 306.063, 1386.39, 57.1376, 3.44248, 1800, 0);

-- Path for spawned mobs during Akil'Zon Gauntlet event
DELETE FROM waypoint_data WHERE id IN (24159, 24225);
INSERT INTO waypoint_data (id, point, position_x, position_y, position_z) VALUES
(24159, 0, 336.942902, 1395.780396, 74.461060),
(24159, 1, 313.977417, 1388.563965, 57.498966),
(24159, 2, 299.847748, 1384.816284, 57.845371),
(24159, 3, 285.212646, 1380.696289, 49.321800),
(24159, 4, 234.612244, 1371.709839, 48.124924),
(24159, 5, 228.249954, 1389.996460, 41.749157),
(24159, 6, 228.579117, 1441.494141, 26.042671),
(24159, 7, 221.667770, 1477.149658, 25.917961),
(24225, 0, 227.380508, 1435.022095, 26.795401),
(24225, 1, 229.785645, 1374.889648, 46.950874),
(24225, 2, 238.955399, 1372.417847, 48.658478),
(24225, 3, 284.589935, 1379.860229, 49.320435),
(24225, 4, 300.546082, 1384.496582, 57.776867),
(24225, 5, 313.660736, 1388.640747, 57.446144),
(24225, 6, 335.192871, 1395.329590, 74.626633);

DELETE FROM `spell_disabled` WHERE entry = 44132;
INSERT INTO `spell_disabled` VALUES ('44132', '1', 'Disable spell Drain Power (Malacrass) for players');

-- Update bosses armor
UPDATE creature_template SET armor = 7700 WHERE entry IN (23574, 23576, 23578, 23577, 23863);
UPDATE creature_template SET armor = 6200 WHERE entry = 24239;

-- Some missing immunes
UPDATE creature_template SET mechanic_immune_mask = 617299807 WHERE entry = 23576;
UPDATE creature_template SET mechanic_immune_mask = 617299807 WHERE entry = 23578;
UPDATE creature_template SET mechanic_immune_mask = 617299807 WHERE entry = 23863;

-- This stucks Jan'Alai in combat
UPDATE creature_template SET unit_flags = 33554434 WHERE entry = 24444;
