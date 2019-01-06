-- Fix Guards path in Zabra'jin
REPLACE INTO waypoint_data (id, point, position_x, position_y, position_z, orientation, delay, move_type, action, action_chance, wpguid) VALUES
(1469,	1,	199.476,	7866.16,	44.8099,	0,	0,	0,	NULL,	100,	0),
(1469,	2,	203.437,	7854.5,	37.6973,	0,	0,	0,	NULL,	100,	0),
(1469,	3,	210.282,	7832.36,	33.6637,	0,	10000,	0,	NULL,	100,	0),
(1469,	4,	216.356,	7813.49,	36.5599,	0,	0,	0,	NULL,	100,	0),
(1469,	5,	221.816,	7797.35,	45.1384,	0,	0,	0,	NULL,	100,	0);
REPLACE INTO waypoint_info (id, pathType, pathDirection) VALUES
(1469,	2,	0);
UPDATE creature_addon SET path_id = 1469 WHERE spawnID = 67888;
UPDATE creature SET position_x=199.476, position_y=7866.16, position_z=44.8099 WHERE spawnID = 67888;

REPLACE INTO waypoint_data (id, point, position_x, position_y, position_z, orientation, delay, move_type, action, action_chance, wpguid) VALUES
(1472,	1,	175.692,	7904.98,	54.7183,	0,	0,	0,	NULL,	100,	0),
(1472,	2,	182.137,	7912.84,	47.7924,	0,	0,	0,	NULL,	100,	0),
(1472,	3,	193.201,	7926.27,	43.3382,	0,	10000,	0,	NULL,	100,	0),
(1472,	4,	207.161,	7943.22,	44.3686,	0,	0,	0,	NULL,	100,	0),
(1472,	5,	215.772,	7953.67,	48.8326,	0,	0,	0,	NULL,	100,	0),
(1472,	6,	221.567,	7960.71,	54.6767,	0,	0,	0,	NULL,	100,	0);
REPLACE INTO waypoint_info (id, pathType, pathDirection) VALUES
(1472,	2,	0);
UPDATE creature_addon SET path_id = 1472 WHERE spawnID = 67893;
UPDATE creature SET position_x=175.692, position_y=7904.98, position_z=54.7183 WHERE spawnID = 67893;
