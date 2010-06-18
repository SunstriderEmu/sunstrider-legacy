INSERT INTO eventai_scripts VALUES(1730102, 17301, 6, 0, 100, 5, 0, 0, 0, 0, 34, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Blood Guard Porung - Set Instance Data on Death");
INSERT INTO eventai_scripts VALUES(2092302, 20923, 6, 0, 100, 5, 0, 0, 0, 0, 34, 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Shattered Hand Executioner - Set Instance Data on Death");

INSERT INTO areatrigger_scripts VALUES(4524, 'at_shattered_halls');

UPDATE gameobject_template SET flags = 4 WHERE entry IN (182539, 182540);
UPDATE creature_template SET flags_extra = 0 WHERE entry IN (17301, 20585);

INSERT INTO gameobject (id, map, spawnMask, position_x, position_y, position_z, orientation, rotation0, rotation1, rotation2, rotation3, spawntimesecs, animprogress, state) VALUES
(182539, 540, 3, 141.861, 266.316, -11.5469, 3.14673, 0, 0, 0.999997, -0.00257091, 25, 0, 1),
(182540, 540, 3, 214.405, 266.172, -11.4492, 0.0105082, 0, 0, 0.00525407, 0.999986, 25, 0, 1);

UPDATE creature_template SET unit_flags = 2 WHERE entry IN (17289, 17290, 17292, 17295, 17296, 17297);

DELETE FROM creature WHERE id IN (17301, 17296, 17295, 17297, 17290, 17292, 17289);
INSERT INTO creature (id, map, spawnmask, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, movementtype) VALUES
(17301, 540, 2, 150.672, -84.0326, 1.93665, 6.26466, 43200, 5, 0);
-- (17296, 540, 2, 147.752808, -79.643730, 1.917701, 5.537074, 43200, 5, 0),
-- (17295, 540, 2, 142.168777, -84.358223, 1.908038, 6.264657, 43200, 5, 0),
-- (17297, 540, 2, 145.993637, -89.312386, 1.915693, 0.714077, 43200, 5, 0),
-- (17290, 540, 2, 147.752808, -79.643730, 1.917701, 5.537074, 43200, 5, 0),
-- (17292, 540, 2, 142.168777, -84.358223, 1.908038, 6.264657, 43200, 5, 0),
-- (17289, 540, 2, 145.993637, -89.312386, 1.915693, 0.714077, 43200, 5, 0);

INSERT INTO creature_loot_template VALUES(20585, 31716, -100, 0, 1, 1, 9, 10884, 0);
