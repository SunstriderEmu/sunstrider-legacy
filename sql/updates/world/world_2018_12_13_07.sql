-- Importing creature with guid 40729 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 40729 WHERE entry = 23166;
UPDATE conditions SET ConditionValue3 = 40729 WHERE ConditionValue3 = 1040543 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -40729 WHERE SourceEntry = -1040543 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -40729 WHERE entryorguid = -1040543 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 40729 WHERE target_param1 = 1040543 AND target_type = 10;
UPDATE spawn_group SET spawnID = 40729 WHERE spawnID = 1040543 AND spawnType = 0;

-- Importing creature with guid 2196 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (2196, 23272);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (2196, 530, 1, 0, 0, -1728.36, 5145.19,  -36.867, 1.8544, 25, 0, 0, 5589, 3155, 0, 0, 5, 10);

-- Importing creature with guid 3413 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (3413, 17076);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (3413, 530, 1, 0, 0, -1883.62, 5420.01,  -12.4279, 0.519128, 25, 0, 0, 1117800, 31550, 0, 0, 5, 10);

-- Importing creature with guid 3248 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1096500 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1096500 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1096500 AND spawnType = 0 WHERE ce.spawnID = 1096500;
INSERT INTO creature_entry (spawnID, entry) VALUES (3248, 22823);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (3248, 530, 1, 0, 0, 203.588, 8550.11,  22.3256, 4.47675, 25, 0, 0, 41, 60, 0, 0, 0, 10);

-- Importing creature with guid 207610 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (207610, 20269);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (207610, 530, 1, 0, 1, -1964.78, 5259.32,  -38.7629, 4.43314, 120, 0, 0, 1, 0, 0, 0, 5, 10);
INSERT INTO game_event_creature (guid, event) VALUES (207610, 19);

-- Importing creature with guid 54984 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 54984 WHERE entry = 15278;
UPDATE conditions SET ConditionValue3 = 54984 WHERE ConditionValue3 = 1054413 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -54984 WHERE SourceEntry = -1054413 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -54984 WHERE entryorguid = -1054413 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 54984 WHERE target_param1 = 1054413 AND target_type = 10;
UPDATE spawn_group SET spawnID = 54984 WHERE spawnID = 1054413 AND spawnType = 0;

-- Importing creature with guid 55358 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1054786 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1054786 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1054786 AND spawnType = 0 WHERE ce.spawnID = 1054786;
INSERT INTO creature_entry (spawnID, entry) VALUES (55358, 15402);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (55358, 530, 1, 0, 1, 8711.23, -7160.89,  43.6012, 1.02974, 300, 0, 0, 328, 350, 0, 0, 0, 10);

-- Importing creature with guid 55415 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (55415, 15493);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (55415, 530, 1, 0, 0, 9507.19, -6801.12,  16.4921, 6.0033, 300, 0, 0, 102, 0, 0, 0, 5, 10);

-- Importing creature with guid 56972 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1056389 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1056389 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1056389 AND spawnType = 0 WHERE ce.spawnID = 1056389;
INSERT INTO creature_entry (spawnID, entry) VALUES (56972, 16261);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (56972, 530, 1, 0, 0, 8692.21, -6639.57,  72.8276, 0.226893, 300, 0, 0, 178, 382, 0, 0, 0, 10);

-- Importing creature with guid 57173 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 57173 WHERE entry = 16475;
UPDATE conditions SET ConditionValue3 = 57173 WHERE ConditionValue3 = 1056589 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -57173 WHERE SourceEntry = -1056589 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -57173 WHERE entryorguid = -1056589 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 57173 WHERE target_param1 = 1056589 AND target_type = 10;
UPDATE spawn_group SET spawnID = 57173 WHERE spawnID = 1056589 AND spawnType = 0;

-- Importing creature with guid 57174 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1056590 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1056590 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1056590 AND spawnType = 0 WHERE ce.spawnID = 1056590;
INSERT INTO creature_entry (spawnID, entry) VALUES (57174, 16476);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (57174, 530, 1, 0, 0, -4173.15, -12499.2,  45.3233, 0.396613, 300, 0, 0, 71, 0, 0, 0, 0, 10);

-- Importing creature with guid 57626 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1057041 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1057041 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1057041 AND spawnType = 0 WHERE ce.spawnID = 1057041;
INSERT INTO creature_entry (spawnID, entry) VALUES (57626, 16632);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (57626, 530, 1, 0, 0, -4051.54, -11552.8,  -138.392, 5.21853, 120, 0, 0, 737, 2175, 0, 0, 0, 10);

-- Importing creature with guid 57746 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1057161 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1057161 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1057161 AND spawnType = 0 WHERE ce.spawnID = 1057161;
INSERT INTO creature_entry (spawnID, entry) VALUES (57746, 16755);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (57746, 530, 1, 0, 0, -4043.18, -11556.4,  -138.307, 5.41052, 120, 0, 0, 737, 2175, 0, 0, 0, 10);

-- Importing creature with guid 57747 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1057162 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1057162 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1057162 AND spawnType = 0 WHERE ce.spawnID = 1057162;
INSERT INTO creature_entry (spawnID, entry) VALUES (57747, 16757);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (57747, 530, 1, 0, 0, -3697.55, -11479.4,  -119.491, 1.77213, 300, 0, 0, 737, 2175, 0, 0, 0, 10);

-- Importing creature with guid 57879 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 57879 WHERE entry = 16821;
UPDATE conditions SET ConditionValue3 = 57879 WHERE ConditionValue3 = 1057294 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -57879 WHERE SourceEntry = -1057294 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -57879 WHERE entryorguid = -1057294 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 57879 WHERE target_param1 = 1057294 AND target_type = 10;
UPDATE spawn_group SET spawnID = 57879 WHERE spawnID = 1057294 AND spawnType = 0;

-- Importing creature with guid 58021 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 58021 WHERE entry = 16856;
UPDATE conditions SET ConditionValue3 = 58021 WHERE ConditionValue3 = 1057436 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -58021 WHERE SourceEntry = -1057436 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -58021 WHERE entryorguid = -1057436 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 58021 WHERE target_param1 = 1057436 AND target_type = 10;
UPDATE spawn_group SET spawnID = 58021 WHERE spawnID = 1057436 AND spawnType = 0;

-- Importing creature with guid 58048 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1057463 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1057463 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1057463 AND spawnType = 0 WHERE ce.spawnID = 1057463;
INSERT INTO creature_entry (spawnID, entry) VALUES (58048, 16860);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (58048, 530, 1, 0, 1, 8683.78, -6694.29,  73.2179, 0.191986, 300, 0, 0, 160, 350, 0, 0, 0, 10);

-- Importing creature with guid 59416 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 59416 WHERE entry = 17015;
UPDATE conditions SET ConditionValue3 = 59416 WHERE ConditionValue3 = 1058828 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -59416 WHERE SourceEntry = -1058828 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -59416 WHERE entryorguid = -1058828 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 59416 WHERE target_param1 = 1058828 AND target_type = 10;
UPDATE spawn_group SET spawnID = 59416 WHERE spawnID = 1058828 AND spawnType = 0;

-- Importing creature with guid 63167 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1062521 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1062521 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1062521 AND spawnType = 0 WHERE ce.spawnID = 1062521;
INSERT INTO creature_entry (spawnID, entry) VALUES (63167, 17584);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (63167, 530, 1, 0, 0, -3972.18, -11927.5,  -0.670018, 3.66519, 300, 0, 0, 300, 0, 0, 0, 0, 10);

-- Importing creature with guid 64949 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1064296 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1064296 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1064296 AND spawnType = 0 WHERE ce.spawnID = 1064296;
INSERT INTO creature_entry (spawnID, entry) VALUES (64949, 18146);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (64949, 530, 1, 0, 1, 9834.04, -7480.85,  14.9347, 6.15987, 300, 0, 0, 2442, 2434, 0, 0, 0, 10);

-- Importing creature with guid 65505 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1064841 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1064841 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1064841 AND spawnType = 0 WHERE ce.spawnID = 1064841;
DELETE FROM waypoint_data WHERE id = 1427;
INSERT INTO creature_entry (spawnID, entry) VALUES (65505, 18237);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (65505, 530, 1, 0, 0, 9886.45, -7179.34,  31.0351, 2.35346, 120, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (65505, NULL, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 65502 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1064838 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1064838 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1064838 AND spawnType = 0 WHERE ce.spawnID = 1064838;
INSERT INTO creature_entry (spawnID, entry) VALUES (65502, 18231);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (65502, 530, 1, 0, 0, 9714.11, -7313.33,  24.8485, 1.13446, 300, 0, 0, 494, 1443, 0, 0, 0, 10);

-- Importing creature with guid 65503 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1064839 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1064839 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1064839 AND spawnType = 0 WHERE ce.spawnID = 1064839;
INSERT INTO creature_entry (spawnID, entry) VALUES (65503, 18232);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (65503, 530, 1, 0, 0, 9715.04, -7311.68,  24.8428, 4.86947, 300, 0, 0, 494, 1443, 0, 0, 0, 10);

-- Importing creature with guid 66481 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 66481 WHERE entry = 18475;
UPDATE conditions SET ConditionValue3 = 66481 WHERE ConditionValue3 = 1065811 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -66481 WHERE SourceEntry = -1065811 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -66481 WHERE entryorguid = -1065811 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 66481 WHERE target_param1 = 1065811 AND target_type = 10;
UPDATE spawn_group SET spawnID = 66481 WHERE spawnID = 1065811 AND spawnType = 0;

-- Importing creature with guid 67250 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 67250 WHERE entry = 18841;
UPDATE conditions SET ConditionValue3 = 67250 WHERE ConditionValue3 = 1066567 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -67250 WHERE SourceEntry = -1066567 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -67250 WHERE entryorguid = -1066567 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 67250 WHERE target_param1 = 1066567 AND target_type = 10;
UPDATE spawn_group SET spawnID = 67250 WHERE spawnID = 1066567 AND spawnType = 0;

-- Importing creature with guid 68635 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 68635 WHERE entry = 19186;
UPDATE conditions SET ConditionValue3 = 68635 WHERE ConditionValue3 = 1067876 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -68635 WHERE SourceEntry = -1067876 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -68635 WHERE entryorguid = -1067876 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 68635 WHERE target_param1 = 1067876 AND target_type = 10;
UPDATE spawn_group SET spawnID = 68635 WHERE spawnID = 1067876 AND spawnType = 0;

-- Importing creature with guid 70166 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1069382 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1069382 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1069382 AND spawnType = 0 WHERE ce.spawnID = 1069382;
INSERT INTO creature_entry (spawnID, entry) VALUES (70166, 19667);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (70166, 530, 1, 0, 0, -2057.45, 8528.71,  24.6556, 2.05449, 120, 0, 0, 14193, 8370, 0, 0, 0, 10);

-- Importing creature with guid 70668 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1069873 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1069873 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1069873 AND spawnType = 0 WHERE ce.spawnID = 1069873;
INSERT INTO creature_entry (spawnID, entry) VALUES (70668, 19747);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (70668, 530, 1, 0, 1, 2223.49, 5426.15,  144.35, 3.36923, 300, 0, 0, 26165, 8973, 0, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 70668);

-- Importing creature with guid 71940 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1071134 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1071134 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1071134 AND spawnType = 0 WHERE ce.spawnID = 1071134;
INSERT INTO creature_entry (spawnID, entry) VALUES (71940, 20244);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (71940, 530, 1, 0, 0, 10512.1, -6499.65,  3.6119, 0.855017, 300, 8, 0, 42, 0, 1, 0, 0, 10);

-- Importing creature with guid 71941 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1071135 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1071135 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1071135 AND spawnType = 0 WHERE ce.spawnID = 1071135;
INSERT INTO creature_entry (spawnID, entry) VALUES (71941, 20246);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (71941, 530, 1, 0, 0, 10512.1, -6499.65,  3.6119, 0.855017, 300, 8, 0, 42, 0, 1, 0, 0, 10);

-- Importing creature with guid 72243 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 72243 WHERE entry = 20333;
UPDATE conditions SET ConditionValue3 = 72243 WHERE ConditionValue3 = 1071434 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -72243 WHERE SourceEntry = -1071434 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -72243 WHERE entryorguid = -1071434 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 72243 WHERE target_param1 = 1071434 AND target_type = 10;
UPDATE spawn_group SET spawnID = 72243 WHERE spawnID = 1071434 AND spawnType = 0;

-- Importing creature with guid 72249 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 72249 WHERE entry = 20336;
UPDATE conditions SET ConditionValue3 = 72249 WHERE ConditionValue3 = 1071440 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -72249 WHERE SourceEntry = -1071440 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -72249 WHERE entryorguid = -1071440 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 72249 WHERE target_param1 = 1071440 AND target_type = 10;
UPDATE spawn_group SET spawnID = 72249 WHERE spawnID = 1071440 AND spawnType = 0;

-- Importing creature with guid 72250 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 72250 WHERE entry = 20337;
UPDATE conditions SET ConditionValue3 = 72250 WHERE ConditionValue3 = 1071441 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -72250 WHERE SourceEntry = -1071441 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -72250 WHERE entryorguid = -1071441 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 72250 WHERE target_param1 = 1071441 AND target_type = 10;
UPDATE spawn_group SET spawnID = 72250 WHERE spawnID = 1071441 AND spawnType = 0;

-- Importing creature with guid 72251 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 72251 WHERE entry = 20338;
UPDATE conditions SET ConditionValue3 = 72251 WHERE ConditionValue3 = 1071442 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -72251 WHERE SourceEntry = -1071442 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -72251 WHERE entryorguid = -1071442 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 72251 WHERE target_param1 = 1071442 AND target_type = 10;
UPDATE spawn_group SET spawnID = 72251 WHERE spawnID = 1071442 AND spawnType = 0;

-- Importing creature with guid 72438 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 72438 WHERE entry = 20450;
UPDATE conditions SET ConditionValue3 = 72438 WHERE ConditionValue3 = 1071622 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -72438 WHERE SourceEntry = -1071622 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -72438 WHERE entryorguid = -1071622 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 72438 WHERE target_param1 = 1071622 AND target_type = 10;
UPDATE spawn_group SET spawnID = 72438 WHERE spawnID = 1071622 AND spawnType = 0;

-- Importing creature with guid 72462 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 72462 WHERE entry = 20454;
UPDATE conditions SET ConditionValue3 = 72462 WHERE ConditionValue3 = 1071646 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -72462 WHERE SourceEntry = -1071646 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -72462 WHERE entryorguid = -1071646 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 72462 WHERE target_param1 = 1071646 AND target_type = 10;
UPDATE spawn_group SET spawnID = 72462 WHERE spawnID = 1071646 AND spawnType = 0;

-- Importing creature with guid 73096 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1072276 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1072276 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1072276 AND spawnType = 0 WHERE ce.spawnID = 1072276;
INSERT INTO creature_entry (spawnID, entry) VALUES (73096, 20724);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (73096, 530, 1, 0, 0, 9631.35, -7119.37,  14.4079, 2.63545, 300, 0, 0, 2215, 0, 0, 0, 0, 10);

-- Importing creature with guid 73835 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (73835, 21025);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (73835, 530, 1, 0, 0, 2809.79, 5250.41,  268.553, 3.26377, 300, 0, 0, 42, 0, 0, 0, 0, 10);

-- Importing creature with guid 73961 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1073133 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1073133 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1073133 AND spawnType = 0 WHERE ce.spawnID = 1073133;
INSERT INTO creature_entry (spawnID, entry) VALUES (73961, 21055);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (73961, 530, 1, 0, 0, 8682.23, -6700.56,  73.1184, 5.49269, 300, 0, 0, 42, 0, 0, 0, 0, 10);

-- Importing creature with guid 74035 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1073207 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1073207 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1073207 AND spawnType = 0 WHERE ce.spawnID = 1073207;
INSERT INTO creature_entry (spawnID, entry) VALUES (74035, 21063);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (74035, 530, 1, 0, 0, 8688.39, -6694.29,  73.1161, 5.46913, 300, 0, 0, 42, 0, 0, 0, 0, 10);

-- Importing creature with guid 74036 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1073208 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1073208 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1073208 AND spawnType = 0 WHERE ce.spawnID = 1073208;
INSERT INTO creature_entry (spawnID, entry) VALUES (74036, 21064);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (74036, 530, 1, 0, 0, 8679.07, -6690.34,  73.1234, 0.429256, 300, 0, 0, 42, 0, 0, 0, 0, 10);

-- Importing creature with guid 74299 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1073464 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1073464 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1073464 AND spawnType = 0 WHERE ce.spawnID = 1073464;
INSERT INTO creature_entry (spawnID, entry) VALUES (74299, 21182);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (74299, 530, 1, 0, 0, -1157.05, 2104.98,  82.9548, 1.18682, 300, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 74299);

-- Importing creature with guid 74397 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 74397 WHERE entry = 21209;
UPDATE conditions SET ConditionValue3 = 74397 WHERE ConditionValue3 = 1073562 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -74397 WHERE SourceEntry = -1073562 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -74397 WHERE entryorguid = -1073562 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 74397 WHERE target_param1 = 1073562 AND target_type = 10;
UPDATE spawn_group SET spawnID = 74397 WHERE spawnID = 1073562 AND spawnType = 0;

-- Importing creature with guid 74638 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 74638 WHERE entry = 21315;
UPDATE conditions SET ConditionValue3 = 74638 WHERE ConditionValue3 = 1310722 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -74638 WHERE SourceEntry = -1310722 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -74638 WHERE entryorguid = -1310722 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 74638 WHERE target_param1 = 1310722 AND target_type = 10;
UPDATE spawn_group SET spawnID = 74638 WHERE spawnID = 1310722 AND spawnType = 0;

-- Importing creature with guid 75654 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1074811 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1074811 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1074811 AND spawnType = 0 WHERE ce.spawnID = 1074811;
INSERT INTO creature_entry (spawnID, entry) VALUES (75654, 21657);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (75654, 530, 1, 0, 0, -4147.19, 852.921,  112.449, 4.27906, 300, 0, 0, 110700, 0, 2, 0, 0, 10);

-- Importing creature with guid 76723 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 76723 WHERE entry = 21937;
UPDATE conditions SET ConditionValue3 = 76723 WHERE ConditionValue3 = 1075866 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -76723 WHERE SourceEntry = -1075866 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -76723 WHERE entryorguid = -1075866 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 76723 WHERE target_param1 = 1075866 AND target_type = 10;
UPDATE spawn_group SET spawnID = 76723 WHERE spawnID = 1075866 AND spawnType = 0;

-- Importing creature with guid 78735 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1077853 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1077853 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1077853 AND spawnType = 0 WHERE ce.spawnID = 1077853;
INSERT INTO creature_entry (spawnID, entry) VALUES (78735, 22401);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (78735, 530, 1, 0, 0, -825.249, 2026.35,  46.5842, 1.18682, 300, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 78735);

-- Importing creature with guid 78736 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1077854 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1077854 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1077854 AND spawnType = 0 WHERE ce.spawnID = 1077854;
INSERT INTO creature_entry (spawnID, entry) VALUES (78736, 22402);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (78736, 530, 1, 0, 0, -894.47, 1919.87,  93.7102, 1.18682, 300, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 78736);

-- Importing creature with guid 78737 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1077855 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1077855 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1077855 AND spawnType = 0 WHERE ce.spawnID = 1077855;
INSERT INTO creature_entry (spawnID, entry) VALUES (78737, 22403);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (78737, 530, 1, 0, 0, -979.369, 1876.67,  121.587, 1.18682, 300, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 78737);

-- Importing creature with guid 78787 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1077905 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1077905 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1077905 AND spawnType = 0 WHERE ce.spawnID = 1077905;
INSERT INTO creature_entry (spawnID, entry) VALUES (78787, 22433);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (78787, 530, 1, 0, 1, -5144.4, 600.909,  82.7549, 6.02139, 300, 0, 0, 7266, 3155, 0, 0, 0, 10);

-- Importing creature with guid 78818 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1077936 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1077936 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1077936 AND spawnType = 0 WHERE ce.spawnID = 1077936;
INSERT INTO creature_entry (spawnID, entry) VALUES (78818, 22458);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (78818, 530, 1, 0, 1, -3741.11, 5403.61,  -3.33493, 1.9117, 300, 0, 0, 5914, 0, 0, 0, 0, 10);

-- Importing creature with guid 39681 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1039502 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1039502 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1039502 AND spawnType = 0 WHERE ce.spawnID = 1039502;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1096587 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1096587 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1096587 AND spawnType = 0 WHERE ce.spawnID = 1096587;
INSERT INTO creature_entry (spawnID, entry) VALUES (39681, 17832);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (39681, 530, 1, 0, 0, 6465.84, -6433.7,  50.33, 1.7237, 300, 0, 0, 605, 0, 0, 0, 0, 10);

-- Importing creature with guid 84082 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 84082 WHERE entry = 22026;
UPDATE conditions SET ConditionValue3 = 84082 WHERE ConditionValue3 = 1083121 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -84082 WHERE SourceEntry = -1083121 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -84082 WHERE entryorguid = -1083121 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 84082 WHERE target_param1 = 1083121 AND target_type = 10;
UPDATE spawn_group SET spawnID = 84082 WHERE spawnID = 1083121 AND spawnType = 0;

-- Importing creature with guid 84399 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1083421 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1083421 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1083421 AND spawnType = 0 WHERE ce.spawnID = 1083421;
INSERT INTO creature_entry (spawnID, entry) VALUES (84399, 17528);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (84399, 530, 1, 0, 0, -2542.23, -11328.8,  22.8675, 0.662011, 120, 0, 0, 360, 0, 0, 0, 0, 10);

-- Importing creature with guid 90922 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 90922 WHERE entry = 25196;
UPDATE conditions SET ConditionValue3 = 90922 WHERE ConditionValue3 = 1089540 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -90922 WHERE SourceEntry = -1089540 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -90922 WHERE entryorguid = -1089540 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 90922 WHERE target_param1 = 1089540 AND target_type = 10;
UPDATE spawn_group SET spawnID = 90922 WHERE spawnID = 1089540 AND spawnType = 0;

-- Importing creature with guid 84940 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 84940 WHERE entry = 22094;
UPDATE conditions SET ConditionValue3 = 84940 WHERE ConditionValue3 = 1083897 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -84940 WHERE SourceEntry = -1083897 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -84940 WHERE entryorguid = -1083897 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 84940 WHERE target_param1 = 1083897 AND target_type = 10;
UPDATE spawn_group SET spawnID = 84940 WHERE spawnID = 1083897 AND spawnType = 0;

-- Importing creature with guid 85526 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1084421 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1084421 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1084421 AND spawnType = 0 WHERE ce.spawnID = 1084421;
INSERT INTO creature_entry (spawnID, entry) VALUES (85526, 19879);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (85526, 530, 1, 0, 1, -3376.35, 5216.41,  -101.048, 5.66421, 450, 0, 0, 5341, 0, 0, 0, 0, 10);

-- Importing creature with guid 86485 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 86485 WHERE entry = 21710;
UPDATE conditions SET ConditionValue3 = 86485 WHERE ConditionValue3 = 1085204 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -86485 WHERE SourceEntry = -1085204 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -86485 WHERE entryorguid = -1085204 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 86485 WHERE target_param1 = 1085204 AND target_type = 10;
UPDATE spawn_group SET spawnID = 86485 WHERE spawnID = 1085204 AND spawnType = 0;

-- Importing creature with guid 87039 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1095728 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1095728 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1095728 AND spawnType = 0 WHERE ce.spawnID = 1095728;
INSERT INTO creature_entry (spawnID, entry) VALUES (87039, 20682);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (87039, 530, 1, 0, 0, -3945.56, 4152.5,  3.57, 1.5144, 1250, 0, 0, 23656, 0, 0, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 87039);

-- Importing creature with guid 51876 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1051571 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1051571 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1051571 AND spawnType = 0 WHERE ce.spawnID = 1051571;
INSERT INTO creature_entry (spawnID, entry) VALUES (51876, 23140);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (51876, 530, 1, 0, 1, -5114.44, 588.484,  85.8724, 3.03687, 25, 0, 0, 151600, 0, 0, 0, 0, 10);

-- Importing creature with guid 18230 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18097 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18097 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18097 AND spawnType = 0 WHERE ce.spawnID = 18097;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18222 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18222 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18222 AND spawnType = 0 WHERE ce.spawnID = 18222;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18227 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18227 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18227 AND spawnType = 0 WHERE ce.spawnID = 18227;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18228 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18228 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18228 AND spawnType = 0 WHERE ce.spawnID = 18228;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18230 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18230 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18230 AND spawnType = 0 WHERE ce.spawnID = 18230;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18231 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18231 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18231 AND spawnType = 0 WHERE ce.spawnID = 18231;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18330 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18330 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18330 AND spawnType = 0 WHERE ce.spawnID = 18330;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18331 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18331 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18331 AND spawnType = 0 WHERE ce.spawnID = 18331;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18338 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18338 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18338 AND spawnType = 0 WHERE ce.spawnID = 18338;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18339 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18339 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18339 AND spawnType = 0 WHERE ce.spawnID = 18339;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18344 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18344 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18344 AND spawnType = 0 WHERE ce.spawnID = 18344;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18345 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18345 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18345 AND spawnType = 0 WHERE ce.spawnID = 18345;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18346 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18346 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18346 AND spawnType = 0 WHERE ce.spawnID = 18346;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18417 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18417 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18417 AND spawnType = 0 WHERE ce.spawnID = 18417;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18418 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18418 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18418 AND spawnType = 0 WHERE ce.spawnID = 18418;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18419 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18419 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18419 AND spawnType = 0 WHERE ce.spawnID = 18419;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18420 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18420 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18420 AND spawnType = 0 WHERE ce.spawnID = 18420;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18421 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18421 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18421 AND spawnType = 0 WHERE ce.spawnID = 18421;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18422 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18422 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18422 AND spawnType = 0 WHERE ce.spawnID = 18422;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18423 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18423 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18423 AND spawnType = 0 WHERE ce.spawnID = 18423;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18425 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18425 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18425 AND spawnType = 0 WHERE ce.spawnID = 18425;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 18472 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -18472 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 18472 AND spawnType = 0 WHERE ce.spawnID = 18472;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 19186 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -19186 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 19186 AND spawnType = 0 WHERE ce.spawnID = 19186;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 24618 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -24618 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 24618 AND spawnType = 0 WHERE ce.spawnID = 24618;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 24619 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -24619 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 24619 AND spawnType = 0 WHERE ce.spawnID = 24619;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 24713 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -24713 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 24713 AND spawnType = 0 WHERE ce.spawnID = 24713;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 25728 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -25728 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 25728 AND spawnType = 0 WHERE ce.spawnID = 25728;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 25729 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -25729 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 25729 AND spawnType = 0 WHERE ce.spawnID = 25729;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 26024 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -26024 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 26024 AND spawnType = 0 WHERE ce.spawnID = 26024;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 26025 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -26025 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 26025 AND spawnType = 0 WHERE ce.spawnID = 26025;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 26026 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -26026 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 26026 AND spawnType = 0 WHERE ce.spawnID = 26026;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 26027 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -26027 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 26027 AND spawnType = 0 WHERE ce.spawnID = 26027;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 26029 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -26029 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 26029 AND spawnType = 0 WHERE ce.spawnID = 26029;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 26040 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -26040 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 26040 AND spawnType = 0 WHERE ce.spawnID = 26040;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 26041 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -26041 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 26041 AND spawnType = 0 WHERE ce.spawnID = 26041;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 26042 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -26042 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 26042 AND spawnType = 0 WHERE ce.spawnID = 26042;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 26043 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -26043 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 26043 AND spawnType = 0 WHERE ce.spawnID = 26043;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 26044 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -26044 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 26044 AND spawnType = 0 WHERE ce.spawnID = 26044;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 26046 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -26046 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 26046 AND spawnType = 0 WHERE ce.spawnID = 26046;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 26047 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -26047 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 26047 AND spawnType = 0 WHERE ce.spawnID = 26047;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 26048 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -26048 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 26048 AND spawnType = 0 WHERE ce.spawnID = 26048;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 26049 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -26049 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 26049 AND spawnType = 0 WHERE ce.spawnID = 26049;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 26050 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -26050 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 26050 AND spawnType = 0 WHERE ce.spawnID = 26050;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 26123 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -26123 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 26123 AND spawnType = 0 WHERE ce.spawnID = 26123;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 27799 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -27799 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 27799 AND spawnType = 0 WHERE ce.spawnID = 27799;
INSERT INTO creature_entry (spawnID, entry) VALUES (18230, 1797);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (18230, 0, 1, 820, 0, 1184.34, 1961.32,  9.59904, 5.55792, 275, 10, 0, 296, 0, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 18230);

-- Importing creature with guid 23428 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (23428, 23144);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (23428, 530, 1, 0, 0, -5119.8, 624.766,  86.8275, 1.25664, 300, 0, 0, 3858, 0, 0, 0, 0, 10);

-- Importing creature with guid 52336 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (52336, 23345);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (52336, 530, 1, 0, 0, -5066.31, 640.214,  86.4967, 1.55334, 120, 0, 0, 1, 0, 0, 0, 0, 10);

-- Importing creature with guid 52337 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (52337, 23346);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (52337, 530, 1, 0, 0, -5060.33, 640.436,  86.6616, 1.50098, 120, 0, 0, 1, 0, 0, 0, 0, 10);

-- Importing creature with guid 52338 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (52338, 23348);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (52338, 530, 1, 0, 0, -5074.88, 625.449,  85.9117, 1.5708, 120, 0, 0, 1, 0, 0, 0, 0, 10);

-- Importing creature with guid 52340 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1040517 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1040517 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1040517 AND spawnType = 0 WHERE ce.spawnID = 1040517;
INSERT INTO creature_entry (spawnID, entry) VALUES (52340, 23340);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (52340, 530, 1, 0, 0, -5088.56, 640.836,  86.6143, 1.48353, 120, 0, 0, 1, 0, 0, 0, 0, 10);

-- Importing creature with guid 52341 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1040530 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1040530 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1040530 AND spawnType = 0 WHERE ce.spawnID = 1040530;
INSERT INTO creature_entry (spawnID, entry) VALUES (52341, 23342);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (52341, 530, 1, 0, 0, -5081.62, 640.932,  86.6143, 1.5708, 120, 0, 0, 1, 0, 0, 0, 0, 10);

-- Importing creature with guid 52342 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1040539 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1040539 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1040539 AND spawnType = 0 WHERE ce.spawnID = 1040539;
INSERT INTO creature_entry (spawnID, entry) VALUES (52342, 23344);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (52342, 530, 1, 0, 0, -5072.99, 640.034,  86.4893, 1.60313, 120, 0, 0, 1, 0, 0, 0, 0, 10);

-- Importing creature with guid 212295 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (212295, 25891);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (212295, 530, 1, 0, 0, -2238.78, -11890.3,  28.0834, 4.91003, 300, 0, 0, 1524, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (212295, 1);

-- Importing creature with guid 86642 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1085359 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1085359 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1085359 AND spawnType = 0 WHERE ce.spawnID = 1085359;
INSERT INTO creature_entry (spawnID, entry) VALUES (86642, 25947);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86642, 530, 1, 16340, 0, 205.463, 7693.15,  23.3862, 2.04204, 300, 0, 0, 3524, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (86642, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO game_event_creature (guid, event) VALUES (86642, 1);

-- Importing creature with guid 213109 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (213109, 25903);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (213109, 530, 1, 0, 0, -2515.71, 7550.71,  -1.47649, 4.94805, 300, 0, 0, 3052, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (213109, 1);

-- Importing creature with guid 212852 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (212852, 25913);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (212852, 530, 1, 0, 0, 3127.12, 3744.35,  141.695, 5.94888, 300, 0, 0, 42, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (212852, 1);

-- Importing creature with guid 212603 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (212603, 25888);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (212603, 530, 1, 0, 0, -4217.18, -12309.5,  2.14482, 5.98868, 300, 0, 0, 1524, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (212603, 1);

-- Importing creature with guid 207611 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (207611, 20272);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (207611, 530, 1, 0, 1, -1766.13, 5257.02,  -38.764, 5.51524, 120, 0, 0, 1, 0, 0, 0, 5, 10);
INSERT INTO game_event_creature (guid, event) VALUES (207611, 19);

-- Importing creature with guid 12401 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1096494 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1096494 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1096494 AND spawnType = 0 WHERE ce.spawnID = 1096494;
INSERT INTO creature_entry (spawnID, entry) VALUES (12401, 22819);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (12401, 530, 1, 0, 0, -1891.93, 5152.62,  -40.2091, 0.817548, 25, 0, 0, 5341, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (12401, 10);

-- Importing creature with guid 45834 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (45834, 21321);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (45834, 530, 1, 0, 0, 1316.63, 6689.88,  -18.6724, 0.160565, 0, 0, 0, 1, 0, 0, 0, 0, 10);

-- Importing creature with guid 151903 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1095320 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1095320 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1095320 AND spawnType = 0 WHERE ce.spawnID = 1095320;
INSERT INTO creature_entry (spawnID, entry) VALUES (151903, 18689);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (151903, 530, 1, 0, 0, -3005.68, 5037.76,  -23.7032, 4.12334, 28800, 30, 0, 11828, 0, 1, 0, 0, 10);

-- Importing creature with guid 202740 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (202740, 25931);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (202740, 530, 1, 0, 0, 9380.33, -6772.76,  14.1056, 0, 120, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (202740, 1);

-- Importing creature with guid 202757 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (202757, 25933);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (202757, 530, 1, 0, 0, 7687.18, -6831.15,  78.56, 0, 120, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (202757, 1);

-- Importing creature with guid 151928 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (151928, 18683);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (151928, 530, 1, 0, 0, -2422.61, 8206.15,  -38.3408, 0.177212, 28800, 0, 0, 10466, 11964, 0, 0, 0, 10);

-- Importing creature with guid 41876 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (41876, 23101);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (41876, 530, 1, 0, 1, -480.499, 7499.03,  181.289, 3.56047, 120, 0, 0, 1, 0, 0, 0, 0, 10);

-- Importing creature with guid 85187 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (85187, 22838);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (85187, 530, 1, 0, 0, -2075.76, 8559.34,  23.027, 4.85702, 120, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (85187, 10);

-- Importing creature with guid 85188 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (85188, 22866);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (85188, 530, 1, 0, 0, 9506.09, -7329.31,  14.3973, 0, 120, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (85188, 10);

-- Importing creature with guid 85189 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1096496 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1096496 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1096496 AND spawnType = 0 WHERE ce.spawnID = 1096496;
INSERT INTO creature_entry (spawnID, entry) VALUES (85189, 22831);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (85189, 530, 1, 0, 0, -3320.86, 4925.1,  -101.1, 0, 120, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (85189, 10);

-- Importing creature with guid 85190 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (85190, 22829);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (85190, 530, 1, 0, 0, 203.587, 8550.11,  22.3256, 0, 120, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (85190, 10);

-- Importing creature with guid 85191 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (85191, 22839);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (85191, 530, 1, 0, 0, -781.294, 6943.52,  33.3344, 0, 120, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (85191, 10);

-- Importing creature with guid 40471 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (40471, 23143);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (40471, 530, 1, 0, 0, -5142.57, 581.328,  84.1357, 0.506145, 120, 0, 0, 1, 0, 0, 0, 0, 10);

-- Importing creature with guid 76310 with import type IMPORT
INSERT INTO creature_entry (spawnID, entry) VALUES (76310, 23081);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (76310, 530, 1, 0, 0, 3279.73, 4640.09,  216.526, 4.08274, 120, 0, 0, 1, 0, 0, 0, 0, 10);

-- Importing creature with guid 94710 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (94710, 25938);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (94710, 530, 1, 0, 0, -3069.81, 2382.73,  61.8417, 3.90071, 300, 0, 0, 42, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (94710, 1);

-- Importing creature with guid 209925 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (209925, 25918);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (209925, 530, 1, 0, 0, 2927.71, 3702.01,  143.705, 2.80239, 300, 0, 0, 42, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (209925, 1);

-- Importing creature with guid 72776 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (72776, 23108);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (72776, 530, 1, 0, 0, -493.09, 7448.85,  178.608, 4.83718, 120, 5, 0, 1, 0, 1, 0, 0, 10);

-- Importing creature with guid 72777 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (72777, 23107);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (72777, 530, 1, 0, 0, -475.565, 7463.81,  181.969, 1.4965, 120, 5, 0, 1, 0, 1, 0, 0, 10);

-- Importing creature with guid 72778 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (72778, 23106);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (72778, 530, 1, 0, 0, -511.772, 7500.89,  183.117, 5.09636, 120, 0, 0, 1, 0, 0, 0, 0, 10);

-- Importing creature with guid 52391 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 52391 WHERE entry = 23383;
UPDATE conditions SET ConditionValue3 = 52391 WHERE ConditionValue3 = 1096824 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -52391 WHERE SourceEntry = -1096824 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -52391 WHERE entryorguid = -1096824 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 52391 WHERE target_param1 = 1096824 AND target_type = 10;
UPDATE spawn_group SET spawnID = 52391 WHERE spawnID = 1096824 AND spawnType = 0;

-- Importing creature with guid 74960 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (74960, 24545);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (74960, 530, 1, 0, 0, -1882.76, 5575.42,  -12.3448, 4.67748, 120, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (74960, 26);

-- Importing creature with guid 12481 with import type IMPORT
INSERT INTO creature_entry (spawnID, entry) VALUES (12481, 23037);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (12481, 530, 1, 0, 0, 3535.15, 5590.77,  0.177, 3.89, 180, 0, 0, 42, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (12481, NULL, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 209675 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (209675, 25926);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (209675, 530, 1, 0, 0, 2298.33, 6134.3,  135.674, 1.15799, 300, 0, 0, 42, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (209675, 1);

-- Importing creature with guid 213118 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (213118, 25905);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (213118, 530, 1, 0, 0, -3953.29, 2043.81,  95.0653, 4.37634, 300, 0, 0, 42, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (213118, 1);

-- Importing creature with guid 85197 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (85197, 25912);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (85197, 530, 1, 0, 0, 195.944, 6003.78,  23.5325, 5.86299, 300, 0, 0, 42, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (85197, 1);

-- Importing creature with guid 213101 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (213101, 25889);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (213101, 530, 1, 0, 0, 2024.17, 6601.49,  136.211, 6.05482, 300, 0, 0, 1524, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (213101, 1);

-- Importing creature with guid 78652 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (78652, 25907);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (78652, 530, 1, 16339, 0, -3000.43, 4160.95,  4.82476, 6.12213, 300, 0, 0, 42, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (78652, 1);

-- Formations...

-- Additional static deletes...
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1097168 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1097168 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1097168 AND spawnType = 0 WHERE ce.spawnID = 1097168;
