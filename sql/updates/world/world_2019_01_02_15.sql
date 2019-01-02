DELETE ca, wd FROM creature_addon ca LEFT JOIN waypoint_data wd ON wd.id = ca.path_id WHERE spawnID IN (326, 13230, 11198, 265, 14153, 37647, 37669, 37671, 37676, 37677, 37682, 37691, 37692, 37693, 37694, 37695, 37696, 37697, 37698, 37699, 37700, 38347, 39540, 51475, 52482, 56624, 68953, 90444, 135445, 135805, 1215227);

-- Importing creature spawn with guid 121 with import type IMPORT_WP
INSERT INTO waypoint_data (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `action_chance`, `wpguid`) VALUES ('12949729', '10', '-4982.62', '-898.722', '501.598', '0', '0', '0', NULL, '100', '0');
INSERT INTO waypoint_data (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `action_chance`, `wpguid`) VALUES ('12949729', '9', '-5011.28', '-923.054', '501.659', '0', '0', '0', NULL, '100', '0');
INSERT INTO waypoint_data (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `action_chance`, `wpguid`) VALUES ('12949729', '8', '-5011.69', '-939.201', '501.659', '0', '0', '0', NULL, '100', '0');
INSERT INTO waypoint_data (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `action_chance`, `wpguid`) VALUES ('12949729', '7', '-4997.78', '-956.213', '501.661', '0', '0', '0', NULL, '100', '0');
INSERT INTO waypoint_data (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `action_chance`, `wpguid`) VALUES ('12949729', '6', '-4986.6', '-963.481', '501.659', '0', '20000', '0', NULL, '100', '0');
INSERT INTO waypoint_data (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `action_chance`, `wpguid`) VALUES ('12949729', '5', '-5000.37', '-953.101', '501.661', '0', '0', '0', NULL, '100', '0');
INSERT INTO waypoint_data (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `action_chance`, `wpguid`) VALUES ('12949729', '4', '-5013.24', '-937.798', '501.659', '0', '0', '0', NULL, '100', '0');
INSERT INTO waypoint_data (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `action_chance`, `wpguid`) VALUES ('12949729', '3', '-5007.11', '-919.845', '501.659', '0', '0', '0', NULL, '100', '0');
INSERT INTO waypoint_data (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `action_chance`, `wpguid`) VALUES ('12949729', '2', '-4982.62', '-898.722', '501.598', '0', '0', '0', NULL, '100', '0');
INSERT INTO waypoint_scripts (`id`, `delay`, `command`, `datalong`, `datalong2`, `dataint`, `x`, `y`, `z`, `o`) VALUES ('8319703', '1', '30', '0', '0', '0', '0', '0', '0', '2.30383');
INSERT INTO waypoint_data (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `action_chance`, `wpguid`) VALUES ('12949729', '1', '-4979.97', '-889.479', '501.627', '0', '20000', '0', '8319703', '100', '0');
UPDATE creature SET MovementType = 2 WHERE spawnID = 121;
UPDATE creature_addon SET path_id = 12949729 WHERE spawnID = 121;

