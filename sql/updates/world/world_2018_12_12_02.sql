-- Importing creature with guid 81 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1000079 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1000079 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1000079 AND spawnType = 0 WHERE ce.spawnID = 1000079;
INSERT INTO creature_entry (spawnID, entry) VALUES (81, 19915);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (81, 0, 1, 0, 0, -5044.19, -1277.27,  510.325, 0.958504, 300, 0, 0, 6986, 0, 0, 0, 0, 10);

-- Importing creature with guid 108 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1000106 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1000106 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1000106 AND spawnType = 0 WHERE ce.spawnID = 1000106;
DELETE FROM waypoint_data WHERE id = 3;
INSERT INTO creature_entry (spawnID, entry) VALUES (108, 6175);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (108, 0, 1, 0, 0, -4895.61, -898.098,  501.743, 2.28638, 310, 0, 1, 156, 0, 2, 0, 0, 10);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (2403, 0, 'Give to the charities who seek to help the victims of these hard times! Please.', NULL, 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (10800, 0, 0, 0, 0, 2403, 0, 0, 0, 0);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (2401, 0, 'Don\'t forget the orphans of Stormwind!', NULL, 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (10801, 0, 0, 0, 0, 2401, 0, 0, 0, 0);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (2404, 0, 'Give of your hearts and your purses! Give to the children of Stormwind who have lost their parents.', NULL, 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (10802, 0, 0, 0, 0, 2404, 0, 0, 0, 0);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (2402, 0, 'Help the children of Stormwind... victims of the war and plague!', NULL, 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (10803, 0, 0, 0, 0, 2402, 0, 0, 0, 0);
INSERT INTO waypoint_data VALUES 
(12949702, 1, -4895.61, -898.098, 501.659, 2.28638, 4000, 0, 10800, 100, 0),
(12949702, 2, -4859.86, -887.922, 501.659, 0, 0, 0, NULL, 100, 0),
(12949702, 3, -4842.48, -885.565, 501.659, 0, 0, 0, NULL, 100, 0),
(12949702, 4, -4842.48, -885.565, 501.659, 1.88496, 0, 0, 10801, 100, 0),
(12949702, 5, -4875.52, -889.064, 501.659, 0, 0, 0, NULL, 100, 0),
(12949702, 6, -4895.69, -897.822, 501.659, 0, 0, 0, NULL, 100, 0),
(12949702, 7, -4895.69, -897.822, 501.659, 2.25148, 4000, 0, 10802, 100, 0),
(12949702, 8, -4927.13, -915.428, 501.659, 0, 0, 0, NULL, 100, 0),
(12949702, 9, -4951.84, -934.979, 501.659, 0, 0, 0, NULL, 100, 0),
(12949702, 10, -4981.38, -966.579, 501.659, 0, 0, 0, NULL, 100, 0),
(12949702, 11, -4983.74, -969.77, 501.659, 0, 0, 0, NULL, 100, 0),
(12949702, 12, -4983.74, -969.77, 501.659, 2.44346, 4000, 0, 10803, 100, 0),
(12949702, 13, -5002.57, -1008.67, 501.659, 0, 0, 0, NULL, 100, 0),
(12949702, 14, -5007.88, -1022.32, 501.655, 0, 0, 0, NULL, 100, 0),
(12949702, 15, -5012.98, -1052.93, 501.71, 0, 0, 0, NULL, 100, 0),
(12949702, 16, -5012.98, -1052.93, 501.71, 2.96706, 4000, 0, 10800, 100, 0),
(12949702, 17, -5006.93, -1021.7, 501.655, 0, 0, 0, NULL, 100, 0),
(12949702, 18, -4992.73, -985.894, 501.659, 0, 0, 0, NULL, 100, 0),
(12949702, 19, -4981.07, -962.529, 501.659, 0, 0, 0, NULL, 100, 0),
(12949702, 20, -4944.82, -930.416, 501.659, 0, 0, 0, NULL, 100, 0),
(12949702, 21, -4906.59, -901.662, 501.659, 0, 0, 0, NULL, 100, 0),
(12949702, 22, -4895.61, -898.098, 501.659, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (108, 12949702, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 113 with import type IMPORT
INSERT INTO creature_entry (spawnID, entry) VALUES (113, 13843);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (113, 0, 1, 0, 1, -4903.01, -968.237,  501.533, 2.47837, 490, 0, 0, 3052, 0, 0, 0, 0, 10);

-- Importing creature with guid 122 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1000119 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1000119 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1000119 AND spawnType = 0 WHERE ce.spawnID = 1000119;
DELETE FROM waypoint_data WHERE id = 5;
INSERT INTO creature_entry (spawnID, entry) VALUES (122, 5109);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (122, 0, 1, 0, 1, -4898.43, -902.386,  501.659, 4.88, 310, 0, 1, 1003, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(12949703, 1, -4891.91, -916.892, 501.746, 0, 0, 0, NULL, 100, 0),
(12949703, 2, -4902.03, -933.63, 501.531, 0, 0, 0, NULL, 100, 0),
(12949703, 3, -4925.54, -947.789, 501.629, 0, 0, 0, NULL, 100, 0),
(12949703, 4, -4940.07, -966.556, 501.593, 0, 0, 0, NULL, 100, 0),
(12949703, 5, -4956.59, -978.85, 501.641, 0, 0, 0, NULL, 100, 0),
(12949703, 6, -4966.61, -974.932, 502.674, 0, 0, 0, NULL, 100, 0),
(12949703, 7, -4977.69, -967.015, 501.659, 0, 0, 0, NULL, 100, 0),
(12949703, 8, -4978.87, -957.095, 501.659, 0, 0, 0, NULL, 100, 0),
(12949703, 9, -4966.74, -946.94, 501.659, 0, 0, 0, NULL, 100, 0),
(12949703, 10, -4946.52, -929.99, 501.659, 0, 0, 0, NULL, 100, 0),
(12949703, 11, -4924.84, -913.595, 501.659, 0, 0, 0, NULL, 100, 0),
(12949703, 12, -4904.37, -898.434, 501.659, 0, 0, 0, NULL, 100, 0),
(12949703, 13, -4898.43, -902.386, 501.659, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (122, 12949703, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 219 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1000214 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1000214 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1000214 AND spawnType = 0 WHERE ce.spawnID = 1000214;
DELETE FROM waypoint_data WHERE id = 10;
INSERT INTO creature_entry (spawnID, entry) VALUES (219, 1692);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (219, 0, 1, 0, 0, -5633.44, -496.477,  396.875, 2.5392, 180, 0, 0, 198, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(2190, 1, -5636.06, -495.833, 396.778, 0, 0, 0, NULL, 100, 0),
(2190, 2, -5641.4, -498.896, 396.778, 0, 0, 0, NULL, 100, 0),
(2190, 3, -5640.83, -501.392, 396.794, 4.93611, 20000, 0, NULL, 100, 0),
(2190, 4, -5641.4, -498.896, 396.778, 0, 0, 0, NULL, 100, 0),
(2190, 5, -5640.07, -496.738, 396.778, 0, 0, 0, NULL, 100, 0),
(2190, 6, -5636.06, -495.833, 396.778, 0, 0, 0, NULL, 100, 0),
(2190, 7, -5633.44, -496.477, 396.778, 0, 0, 0, NULL, 100, 0),
(2190, 8, -5633.44, -496.477, 396.778, 2.19912, 30000, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (219, 2190, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 266 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (266, 10803);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (266, 0, 1, 0, 1, -6008.53, -199.145,  406.687, 4.37218, 180, 0, 0, 156, 0, 0, 0, 5, 10);

-- Importing creature with guid 267 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (267, 10611);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (267, 0, 1, 0, 1, -6007.46, -202.158,  407.448, 2.47837, 900, 0, 0, 198, 0, 0, 0, 5, 10);

-- Importing creature with guid 268 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (268, 10804);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (268, 0, 1, 0, 1, -6010.67, -199.434,  406.792, 5.53269, 180, 0, 0, 156, 0, 0, 0, 5, 10);

-- Importing creature with guid 269 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (269, 10610);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (269, 0, 1, 0, 1, -6009.25, -202.65,  406.983, 1.58825, 180, 0, 0, 222, 0, 0, 0, 5, 10);

-- Importing creature with guid 272 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (272, 10805);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (272, 0, 1, 0, 0, -6007.3, -200.348,  406.777, 3.28222, 180, 0, 0, 120, 0, 0, 0, 5, 10);

-- Importing creature with guid 348 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (348, 11941);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (348, 0, 1, 0, 1, -5592.95, -529.919,  399.653, 1.6284, 180, 0, 0, 102, 0, 0, 0, 5, 10);

-- Importing creature with guid 351 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 351 WHERE entry = 658;
UPDATE conditions SET ConditionValue3 = 351 WHERE ConditionValue3 = 1000346 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -351 WHERE SourceEntry = -1000346 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -351 WHERE entryorguid = -1000346 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 351 WHERE target_param1 = 1000346 AND target_type = 10;
UPDATE spawn_group SET spawnID = 351 WHERE spawnID = 1000346 AND spawnType = 0;

-- Importing creature with guid 6155 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 6155 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -6155 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 6155 AND spawnType = 0 WHERE ce.spawnID = 6155;
INSERT INTO creature_entry (spawnID, entry) VALUES (6155, 847);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (6155, 0, 1, 338, 0, -9260.23, -2204.76,  63.9338, 4.72216, 300, 0, 0, 102, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (6155, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 6154 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1006127 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1006127 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1006127 AND spawnType = 0 WHERE ce.spawnID = 1006127;
DELETE FROM waypoint_data WHERE id = 96;
INSERT INTO creature_entry (spawnID, entry) VALUES (6154, 849);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (6154, 0, 1, 252, 0, -9262.29, -2204.6,  63.934, 4.13816, 300, 0, 0, 71, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(61540, 1, -9262.29, -2204.6, 63.934, 0, 0, 1, NULL, 100, 0),
(61540, 2, -9282.55, -2212.77, 63.4702, 0, 0, 1, NULL, 100, 0),
(61540, 3, -9316.21, -2213.16, 61.8981, 0, 0, 1, NULL, 100, 0),
(61540, 4, -9342.98, -2208.78, 61.8981, 0, 0, 1, NULL, 100, 0),
(61540, 5, -9344.81, -2183.14, 61.8981, 0, 0, 1, NULL, 100, 0),
(61540, 6, -9333.46, -2184.53, 61.8981, 0, 0, 1, NULL, 100, 0),
(61540, 7, -9342.04, -2206.52, 61.8981, 0, 0, 1, NULL, 100, 0),
(61540, 8, -9326.1, -2210.98, 61.8981, 0, 0, 1, NULL, 100, 0),
(61540, 9, -9293.37, -2209.15, 61.8213, 0, 0, 1, NULL, 100, 0),
(61540, 10, -9286.83, -2203.5, 62.899, 0, 0, 1, NULL, 100, 0),
(61540, 11, -9280.69, -2175.14, 60.2934, 0, 0, 1, NULL, 100, 0),
(61540, 12, -9270.13, -2152.55, 61.1369, 0, 0, 1, NULL, 100, 0),
(61540, 13, -9264.35, -2133.3, 64.3245, 0, 0, 1, NULL, 100, 0),
(61540, 14, -9249.2, -2126.65, 64.3418, 0, 0, 1, NULL, 100, 0),
(61540, 15, -9209.35, -2136.65, 63.9347, 0, 0, 1, NULL, 100, 0),
(61540, 16, -9190.63, -2136.43, 63.942, 0, 0, 1, NULL, 100, 0),
(61540, 17, -9184.8, -2144.66, 63.9756, 0, 0, 1, NULL, 100, 0),
(61540, 18, -9178.52, -2182.85, 64.0438, 0, 0, 1, NULL, 100, 0),
(61540, 19, -9192.09, -2199.42, 63.9338, 0, 0, 1, NULL, 100, 0),
(61540, 20, -9198.02, -2179.83, 63.9338, 0, 0, 1, NULL, 100, 0),
(61540, 21, -9207.62, -2174.96, 63.9338, 0, 0, 1, NULL, 100, 0),
(61540, 22, -9239.17, -2175.39, 63.9338, 0, 0, 1, NULL, 100, 0),
(61540, 23, -9247.98, -2186.89, 64.0099, 0, 0, 1, NULL, 100, 0),
(61540, 24, -9262.78, -2190.39, 64.0897, 0, 0, 1, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (6154, 61540, 0, 0, 0, 0, 0, NULL);

-- Importing creature with guid 6150 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1006123 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1006123 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1006123 AND spawnType = 0 WHERE ce.spawnID = 1006123;
DELETE FROM waypoint_data WHERE id = 95;
INSERT INTO creature_entry (spawnID, entry) VALUES (6150, 848);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (6150, 0, 1, 257, 0, -9327.6, -2202.14,  61.898, 4.40372, 300, 0, 0, 102, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(61500, 1, -9327.6, -2202.14, 61.898, 1.46783, 30000, 0, NULL, 100, 0),
(61500, 2, -9315.13, -2208.34, 61.8977, 0, 0, 0, NULL, 100, 0),
(61500, 3, -9293.39, -2209.66, 61.821, 0, 0, 0, NULL, 100, 0),
(61500, 4, -9287.74, -2206.22, 62.7899, 0, 0, 0, NULL, 100, 0),
(61500, 5, -9281.29, -2171.65, 59.6998, 0, 0, 0, NULL, 100, 0),
(61500, 6, -9267.21, -2137.46, 62.1799, 0, 0, 0, NULL, 100, 0),
(61500, 7, -9262.62, -2131.42, 65.3403, 0, 0, 0, NULL, 100, 0),
(61500, 8, -9248.75, -2116.83, 66.1153, 0, 0, 0, NULL, 100, 0),
(61500, 9, -9245.36, -2104.48, 70.2651, 0, 0, 0, NULL, 100, 0),
(61500, 10, -9243.86, -2085.17, 75.3524, 0, 0, 0, NULL, 100, 0),
(61500, 11, -9239.41, -2057.41, 76.8398, 0, 0, 0, NULL, 100, 0),
(61500, 12, -9248.06, -2044.15, 77.0034, 0, 0, 0, NULL, 100, 0),
(61500, 13, -9248.36, -2023.24, 77.0013, 0, 60000, 0, NULL, 100, 0),
(61500, 14, -9248.64, -2045.46, 77.0022, 0, 0, 0, NULL, 100, 0),
(61500, 15, -9239.96, -2068.09, 75.8833, 0, 0, 0, NULL, 100, 0),
(61500, 16, -9240.93, -2088.38, 75.2944, 0, 0, 0, NULL, 100, 0),
(61500, 17, -9245.53, -2099.99, 71.8304, 0, 0, 0, NULL, 100, 0),
(61500, 18, -9247.71, -2120.82, 65.1585, 0, 0, 0, NULL, 100, 0),
(61500, 19, -9254.52, -2145.7, 64.0393, 0, 0, 0, NULL, 100, 0),
(61500, 20, -9255.4, -2185.46, 64.0101, 0, 0, 0, NULL, 100, 0),
(61500, 21, -9270.92, -2193.77, 64.0897, 0, 0, 0, NULL, 100, 0),
(61500, 22, -9276.26, -2211.56, 64.0588, 0, 0, 0, NULL, 100, 0),
(61500, 23, -9293.97, -2213.25, 61.6685, 0, 0, 0, NULL, 100, 0),
(61500, 24, -9316.85, -2209.62, 61.8983, 0, 0, 0, NULL, 100, 0),
(61500, 25, -9327.6, -2202.14, 61.898, 1.46783, 30000, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (6150, 61500, 0, 0, 0, 0, 0, NULL);

-- Importing creature with guid 5086 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1005061 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1005061 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1005061 AND spawnType = 0 WHERE ce.spawnID = 1005061;
INSERT INTO creature_entry (spawnID, entry) VALUES (5086, 522);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (5086, 0, 1, 21691, 1, -10363.4, 359.383,  53.171, 3.21659, 600, 0, 0, 2865, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(50860, 1, -10363.4, 359.383, 53.171, 0, 0, 0, NULL, 100, 0),
(50860, 2, -10371.7, 338.914, 48.4701, 0, 0, 0, NULL, 100, 0),
(50860, 3, -10361.1, 322.492, 48.2917, 0, 0, 0, NULL, 100, 0),
(50860, 4, -10358.1, 289.67, 39.8141, 0, 0, 0, NULL, 100, 0),
(50860, 5, -10378.5, 277.855, 37.5104, 0, 0, 0, NULL, 100, 0),
(50860, 6, -10407.3, 284.497, 40.8748, 0, 0, 0, NULL, 100, 0),
(50860, 7, -10450.9, 248.557, 32.4436, 0, 0, 0, NULL, 100, 0),
(50860, 8, -10465.8, 265.495, 32.4443, 0, 0, 0, NULL, 100, 0),
(50860, 9, -10449.3, 288.402, 36.5922, 0, 0, 0, NULL, 100, 0),
(50860, 10, -10474.8, 298.544, 34.4282, 0, 0, 0, NULL, 100, 0),
(50860, 11, -10482.9, 321.214, 33.1095, 0, 0, 0, NULL, 100, 0),
(50860, 12, -10466.8, 331.526, 35.6161, 0, 0, 0, NULL, 100, 0),
(50860, 13, -10447.1, 331.333, 40.5168, 0, 0, 0, NULL, 100, 0),
(50860, 14, -10428.2, 321.36, 40.9299, 0, 0, 0, NULL, 100, 0),
(50860, 15, -10393, 317.033, 41.0087, 0, 0, 0, NULL, 100, 0),
(50860, 16, -10391.8, 331.272, 42.8239, 0, 0, 0, NULL, 100, 0),
(50860, 17, -10395.2, 338.253, 43.6321, 0, 0, 0, NULL, 100, 0),
(50860, 18, -10420.1, 333.926, 42.7166, 0, 0, 0, NULL, 100, 0),
(50860, 19, -10445, 352.519, 40.6271, 0, 0, 0, NULL, 100, 0),
(50860, 20, -10422, 387.013, 45.0018, 0, 0, 0, NULL, 100, 0),
(50860, 21, -10388.9, 383.009, 46.9237, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (5086, 50860, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 5086);

-- Importing creature with guid 4444 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1004420 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1004420 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1004420 AND spawnType = 0 WHERE ce.spawnID = 1004420;
INSERT INTO creature_entry (spawnID, entry) VALUES (4444, 511);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (4444, 0, 1, 828, 1, -11012, -1350.91,  53.658, 5.70029, 300, 0, 0, 787, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(44440, 1, -11012, -1350.91, 53.658, 0, 0, 0, NULL, 100, 0),
(44440, 2, -11008.1, -1349.59, 53.658, 0, 0, 0, NULL, 100, 0),
(44440, 3, -11004.2, -1348.28, 53.658, 0, 0, 0, NULL, 100, 0),
(44440, 4, -11002, -1347.52, 53.658, 0, 0, 0, NULL, 100, 0),
(44440, 5, -11002, -1347.52, 53.658, 0, 0, 0, NULL, 100, 0),
(44440, 6, -11007.7, -1347.67, 53.908, 0, 0, 0, NULL, 100, 0),
(44440, 7, -11003.4, -1346.67, 53.908, 0, 0, 0, NULL, 100, 0),
(44440, 8, -11005.2, -1349.33, 53.908, 0, 0, 0, NULL, 100, 0),
(44440, 9, -11004.7, -1350.54, 53.658, 0, 0, 0, NULL, 100, 0),
(44440, 10, -11000.4, -1348.08, 53.908, 0, 0, 0, NULL, 100, 0),
(44440, 11, -11000.5, -1348.3, 53.908, 0, 0, 0, NULL, 100, 0),
(44440, 12, -11003, -1351.5, 53.908, 0, 0, 0, NULL, 100, 0),
(44440, 13, -11001.4, -1349.49, 53.908, 0, 0, 0, NULL, 100, 0),
(44440, 14, -11000.5, -1348.37, 53.908, 0, 0, 0, NULL, 100, 0),
(44440, 15, -10998.5, -1350.32, 53.908, 0, 0, 0, NULL, 100, 0),
(44440, 16, -11003.1, -1351.9, 53.908, 0, 0, 0, NULL, 100, 0),
(44440, 17, -11003.5, -1347.61, 53.908, 0, 0, 0, NULL, 100, 0),
(44440, 18, -11003, -1348.9, 53.658, 0, 0, 0, NULL, 100, 0),
(44440, 19, -11005, -1349.43, 53.908, 0, 0, 0, NULL, 100, 0),
(44440, 20, -11005.3, -1349.96, 53.908, 0, 0, 0, NULL, 100, 0),
(44440, 21, -11008.6, -1352.75, 53.908, 0, 0, 0, NULL, 100, 0),
(44440, 22, -11011.2, -1349.45, 53.658, 0, 0, 0, NULL, 100, 0),
(44440, 23, -11008.1, -1347.37, 53.908, 0, 0, 0, NULL, 100, 0),
(44440, 24, -10999.8, -1347.31, 53.908, 0, 0, 0, NULL, 100, 0),
(44440, 25, -11000.1, -1347.87, 53.908, 0, 0, 0, NULL, 100, 0),
(44440, 26, -11001, -1348.77, 53.908, 0, 0, 0, NULL, 100, 0),
(44440, 27, -11002.3, -1350.48, 53.658, 0, 0, 0, NULL, 100, 0),
(44440, 28, -11007.4, -1355.68, 53.9021, 0, 0, 0, NULL, 100, 0),
(44440, 29, -11008.2, -1349.79, 53.908, 0, 0, 0, NULL, 100, 0),
(44440, 30, -10999.2, -1339.94, 53.659, 0, 0, 0, NULL, 100, 0),
(44440, 31, -10991, -1325.43, 51.8941, 0, 0, 0, NULL, 100, 0),
(44440, 32, -10974.7, -1319.76, 52.2399, 0, 0, 0, NULL, 100, 0),
(44440, 33, -10967.6, -1304.02, 52.8301, 0, 0, 0, NULL, 100, 0),
(44440, 34, -10951.6, -1294.9, 53.0798, 0, 0, 0, NULL, 100, 0),
(44440, 35, -10975.7, -1278.12, 52.9166, 0, 0, 0, NULL, 100, 0),
(44440, 36, -10995.9, -1278.58, 52.5571, 0, 0, 0, NULL, 100, 0),
(44440, 37, -11003.2, -1291.67, 53.3562, 0, 0, 0, NULL, 100, 0),
(44440, 38, -11011.8, -1287.97, 52.9221, 0, 0, 0, NULL, 100, 0),
(44440, 39, -11028.2, -1302.74, 52.3512, 0, 0, 0, NULL, 100, 0),
(44440, 40, -11031.2, -1319.99, 53.39, 0, 0, 0, NULL, 100, 0),
(44440, 41, -11023.3, -1327.44, 53.1966, 0, 0, 0, NULL, 100, 0),
(44440, 42, -11012.7, -1317.23, 53.104, 0, 0, 0, NULL, 100, 0),
(44440, 43, -10992.5, -1326.81, 51.8687, 0, 0, 0, NULL, 100, 0),
(44440, 44, -10999.9, -1342.19, 53.6589, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (4444, 44440, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 4444);

-- Importing creature with guid 1816 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1001801 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1001801 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1001801 AND spawnType = 0 WHERE ce.spawnID = 1001801;
INSERT INTO creature_entry (spawnID, entry) VALUES (1816, 9099);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (1816, 0, 1, 0, 0, -4790.14, -1168.66,  502.204, 4.84256, 310, 0, 0, 1003, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(18160, 1, -4815.05, -1167.4, 502.205, 0, 0, 0, NULL, 100, 0),
(18160, 2, -4836.73, -1156.9, 502.167, 0, 0, 0, NULL, 100, 0),
(18160, 3, -4853.94, -1143.64, 502.175, 0, 0, 0, NULL, 100, 0),
(18160, 4, -4867.31, -1122.64, 502.182, 0, 0, 0, NULL, 100, 0),
(18160, 5, -4873.94, -1090.73, 502.214, 0, 0, 0, NULL, 100, 0),
(18160, 6, -4868.06, -1085.28, 502.203, 0, 0, 0, NULL, 100, 0),
(18160, 7, -4829.62, -1085.28, 502.208, 0, 0, 0, NULL, 100, 0),
(18160, 8, -4812.88, -1065.88, 502.197, 0, 0, 0, NULL, 100, 0),
(18160, 9, -4810.18, -1032.05, 502.208, 0, 0, 0, NULL, 100, 0),
(18160, 10, -4797.53, -1025.65, 502.202, 0, 0, 0, NULL, 100, 0),
(18160, 11, -4764.07, -1041.11, 502.211, 0, 0, 0, NULL, 100, 0),
(18160, 12, -4737.23, -1073.92, 502.14, 0, 0, 0, NULL, 100, 0),
(18160, 13, -4732.99, -1106.31, 502.221, 0, 0, 0, NULL, 100, 0),
(18160, 14, -4741.4, -1139.08, 502.197, 0, 0, 0, NULL, 100, 0),
(18160, 15, -4756.99, -1154.73, 502.22, 0, 0, 0, NULL, 100, 0),
(18160, 16, -4774.4, -1165.63, 502.187, 0, 0, 0, NULL, 100, 0),
(18160, 17, -4790.14, -1168.66, 502.204, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (1816, 18160, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 1888 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1001872 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1001872 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1001872 AND spawnType = 0 WHERE ce.spawnID = 1001872;
INSERT INTO creature_entry (spawnID, entry) VALUES (1888, 5637);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (1888, 0, 1, 0, 1, -4681.39, -1266.61,  503.465, 1.96398, 540, 0, 0, 1753, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(18880, 1, -4686.15, -1255.14, 501.993, 0, 0, 0, NULL, 100, 0),
(18880, 2, -4675.92, -1244.38, 501.993, 0, 0, 0, NULL, 100, 0),
(18880, 3, -4667.5, -1245.78, 501.993, 0, 0, 0, NULL, 100, 0),
(18880, 4, -4661.6, -1250.52, 503.382, 0, 0, 0, NULL, 100, 0),
(18880, 5, -4661.6, -1250.52, 503.382, 0, 60000, 0, NULL, 100, 0),
(18880, 6, -4670.11, -1243.08, 501.993, 0, 0, 0, NULL, 100, 0),
(18880, 7, -4681.15, -1244.8, 501.993, 0, 0, 0, NULL, 100, 0),
(18880, 8, -4686.53, -1252.01, 501.993, 0, 0, 0, NULL, 100, 0),
(18880, 9, -4685.42, -1262.68, 501.993, 0, 0, 0, NULL, 100, 0),
(18880, 10, -4681.39, -1266.61, 503.382, 0, 0, 0, NULL, 100, 0),
(18880, 11, -4681.39, -1266.61, 503.382, 0, 60000, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (1888, 18880, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 4211 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1004187 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1004187 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1004187 AND spawnType = 0 WHERE ce.spawnID = 1004187;
INSERT INTO creature_entry (spawnID, entry) VALUES (4211, 576);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (4211, 0, 1, 2375, 1, -10577.5, -1165.16,  28.1497, 2.25222, 300, 0, 0, 896, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(42110, 1, -10581.6, -1172.89, 28.4936, 0, 0, 0, NULL, 100, 0),
(42110, 2, -10584, -1180.59, 27.9936, 0, 0, 0, NULL, 100, 0),
(42110, 3, -10573.6, -1185.8, 27.9936, 0, 0, 0, NULL, 100, 0),
(42110, 4, -10570, -1191.73, 27.8686, 0, 0, 0, NULL, 100, 0),
(42110, 5, -10569.2, -1199.18, 27.7436, 0, 0, 0, NULL, 100, 0),
(42110, 6, -10551.3, -1205.73, 27.5662, 0, 0, 0, NULL, 100, 0),
(42110, 7, -10549.3, -1192.89, 27.6623, 0, 0, 0, NULL, 100, 0),
(42110, 8, -10535.2, -1192.91, 27.6623, 0, 0, 0, NULL, 100, 0),
(42110, 9, -10546.2, -1179.32, 28.1623, 0, 0, 0, NULL, 100, 0),
(42110, 10, -10547.9, -1167.96, 27.7873, 0, 0, 0, NULL, 100, 0),
(42110, 11, -10554.2, -1153.21, 28.0765, 0, 0, 0, NULL, 100, 0),
(42110, 12, -10565.9, -1154.21, 27.5765, 0, 0, 0, NULL, 100, 0),
(42110, 13, -10577.5, -1165.16, 28.1497, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (4211, 42110, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 2499 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1096603 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1096603 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1096603 AND spawnType = 0 WHERE ce.spawnID = 1096603;
INSERT INTO creature_entry (spawnID, entry) VALUES (2499, 6909);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (2499, 1, 1, 0, 0, 11053, 1921.79,  1332.84, 3.7522, 60, 0, 0, 247, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (2499, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 2368 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1002352 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1002352 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1002352 AND spawnType = 0 WHERE ce.spawnID = 1002352;
INSERT INTO creature_entry (spawnID, entry) VALUES (2368, 14912);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (2368, 0, 1, 11221, 0, -11838.2, 1256.01,  2.08196, 5.77616, 1800, 0, 0, 9156, 0, 0, 0, 0, 10);

-- Importing creature with guid 14772 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1014665 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1014665 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1014665 AND spawnType = 0 WHERE ce.spawnID = 1014665;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1016282 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1016282 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1016282 AND spawnType = 0 WHERE ce.spawnID = 1016282;
INSERT INTO creature_entry (spawnID, entry) VALUES (14772, 2714);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14772, 0, 1, 4041, 1, -1552.43, -3029.31,  13.6448, 5.00963, 60, 0, 1, 1342, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(147720, 1, -1552.43, -3029.31, 13.6448, 5.00963, 60000, 0, NULL, 100, 0),
(147720, 2, -1551.28, -3042.15, 12.9831, 0, 0, 0, NULL, 100, 0),
(147720, 3, -1551.47, -3055.87, 13.0074, 0, 0, 0, NULL, 100, 0),
(147720, 4, -1557.96, -3065.89, 13.2017, 0, 0, 0, NULL, 100, 0),
(147720, 5, -1568.38, -3066.59, 13.1757, 0, 0, 0, NULL, 100, 0),
(147720, 6, -1584.38, -3059.72, 12.8479, 0, 0, 0, NULL, 100, 0),
(147720, 7, -1604.79, -3043.72, 14.9377, 0, 0, 0, NULL, 100, 0),
(147720, 8, -1633.49, -3013.09, 19.2308, 0, 0, 0, NULL, 100, 0),
(147720, 9, -1640.02, -3004.89, 20.9252, 0, 0, 0, NULL, 100, 0),
(147720, 10, -1645.71, -2989.32, 23.6772, 0, 0, 0, NULL, 100, 0),
(147720, 11, -1645.99, -2971.86, 26.3366, 0, 0, 0, NULL, 100, 0),
(147720, 12, -1643.19, -2959.2, 27.7, 0, 0, 0, NULL, 100, 0),
(147720, 13, -1639.09, -2945.82, 28.5385, 0, 0, 0, NULL, 100, 0),
(147720, 14, -1637.73, -2933.3, 28.1553, 0, 0, 0, NULL, 100, 0),
(147720, 15, -1641.84, -2916.34, 26.7999, 0, 0, 0, NULL, 100, 0),
(147720, 16, -1644.03, -2901.64, 26.299, 0, 0, 0, NULL, 100, 0),
(147720, 17, -1639.03, -2885.94, 27.6014, 0, 0, 0, NULL, 100, 0),
(147720, 18, -1631.71, -2866.27, 30.2701, 0, 0, 0, NULL, 100, 0),
(147720, 19, -1625.44, -2842.61, 32.2295, 0, 0, 0, NULL, 100, 0),
(147720, 20, -1621.03, -2822.09, 33.6745, 0, 0, 0, NULL, 100, 0),
(147720, 21, -1617.7, -2804.91, 34.6386, 0, 0, 0, NULL, 100, 0),
(147720, 22, -1612.97, -2777.32, 36.1324, 0, 0, 0, NULL, 100, 0),
(147720, 23, -1610.07, -2756.52, 37.2584, 0, 0, 0, NULL, 100, 0),
(147720, 24, -1606.39, -2739.47, 37.6036, 0, 0, 0, NULL, 100, 0),
(147720, 25, -1598.18, -2724.09, 37.1732, 0, 0, 0, NULL, 100, 0),
(147720, 26, -1586.96, -2710.69, 36.1226, 0, 0, 0, NULL, 100, 0),
(147720, 27, -1572.97, -2695.7, 34.6664, 0, 0, 0, NULL, 100, 0),
(147720, 28, -1564.09, -2685.41, 35.4253, 0, 0, 0, NULL, 100, 0),
(147720, 29, -1557.93, -2669.06, 36.761, 0, 0, 0, NULL, 100, 0),
(147720, 30, -1555.19, -2641.25, 41.3576, 0, 0, 0, NULL, 100, 0),
(147720, 31, -1555.73, -2620.27, 45.0843, 0, 0, 0, NULL, 100, 0),
(147720, 32, -1557.22, -2604.89, 47.8118, 0, 0, 0, NULL, 100, 0),
(147720, 33, -1562.1, -2591.78, 48.5331, 0, 0, 0, NULL, 100, 0),
(147720, 34, -1567.06, -2571.43, 49.3074, 0, 0, 0, NULL, 100, 0),
(147720, 35, -1567.62, -2550.48, 50.7276, 0, 0, 0, NULL, 100, 0),
(147720, 36, -1561.84, -2530.37, 52.0972, 0, 0, 0, NULL, 100, 0),
(147720, 37, -1555.6, -2517.85, 53.143, 0, 0, 0, NULL, 100, 0),
(147720, 38, -1550.4, -2506.83, 53.9067, 0, 0, 0, NULL, 100, 0),
(147720, 39, -1544.74, -2495.89, 54.115, 0, 0, 0, NULL, 100, 0),
(147720, 40, -1537.25, -2488.54, 53.6039, 0, 0, 0, NULL, 100, 0),
(147720, 41, -1521.28, -2474.91, 53.2915, 0, 0, 0, NULL, 100, 0),
(147720, 42, -1505.08, -2461.55, 52.9902, 0, 0, 0, NULL, 100, 0),
(147720, 43, -1491.57, -2450.42, 53.6645, 0, 0, 0, NULL, 100, 0),
(147720, 44, -1473.67, -2435, 56.438, 0, 0, 0, NULL, 100, 0),
(147720, 45, -1458.8, -2420.24, 58.7543, 0, 0, 0, NULL, 100, 0),
(147720, 46, -1451.61, -2406.21, 60.4971, 0, 0, 0, NULL, 100, 0),
(147720, 47, -1450.24, -2390.07, 61.2988, 0, 0, 0, NULL, 100, 0),
(147720, 48, -1449.98, -2369.08, 61.4719, 0, 0, 0, NULL, 100, 0),
(147720, 49, -1450.01, -2348.08, 61.5948, 0, 0, 0, NULL, 100, 0),
(147720, 50, -1450.28, -2327.09, 61.7184, 0, 0, 0, NULL, 100, 0),
(147720, 51, -1449.22, -2302.62, 61.8278, 0, 0, 0, NULL, 100, 0),
(147720, 52, -1445.48, -2285.57, 62.1128, 0, 0, 0, NULL, 100, 0),
(147720, 53, -1438.68, -2273.44, 63.0625, 0, 0, 0, NULL, 100, 0),
(147720, 54, -1427.75, -2259.8, 63.363, 0, 0, 0, NULL, 100, 0),
(147720, 55, -1417.94, -2245.39, 63.7294, 0, 0, 0, NULL, 100, 0),
(147720, 56, -1411.22, -2229.24, 63.7811, 0, 0, 0, NULL, 100, 0),
(147720, 57, -1406.21, -2212.48, 63.8356, 0, 0, 0, NULL, 100, 0),
(147720, 58, -1399.67, -2185.27, 63.9419, 0, 0, 0, NULL, 100, 0),
(147720, 59, -1395.07, -2161.21, 64.1007, 0, 0, 0, NULL, 100, 0),
(147720, 60, -1388.98, -2130.31, 64.2996, 0, 0, 0, NULL, 100, 0),
(147720, 61, -1385.75, -2095.48, 63.6847, 0, 0, 0, NULL, 100, 0),
(147720, 62, -1383.99, -2067.53, 62.3167, 0, 0, 0, NULL, 100, 0),
(147720, 63, -1381.89, -2032.6, 60.8356, 0, 0, 0, NULL, 100, 0),
(147720, 64, -1379.07, -2001.23, 59.3176, 0, 0, 0, NULL, 100, 0),
(147720, 65, -1376.39, -1980.42, 58.542, 0, 0, 0, NULL, 100, 0),
(147720, 66, -1373.23, -1970.43, 58.49, 0, 0, 0, NULL, 100, 0),
(147720, 67, -1362.48, -1944.58, 58.4418, 0, 0, 0, NULL, 100, 0),
(147720, 68, -1350.37, -1915.5, 58.3884, 0, 0, 0, NULL, 100, 0),
(147720, 69, -1338.43, -1882.6, 60.3171, 0, 0, 0, NULL, 100, 0),
(147720, 70, -1329.51, -1856.07, 62.228, 0, 0, 0, NULL, 100, 0),
(147720, 71, -1325.86, -1842.56, 63.0927, 0, 0, 0, NULL, 100, 0),
(147720, 72, -1316, -1825.72, 64.4425, 0, 0, 0, NULL, 100, 0),
(147720, 73, -1306.65, -1815.35, 65.3555, 0, 0, 0, NULL, 100, 0),
(147720, 74, -1289.41, -1803.39, 66.6877, 0, 0, 0, NULL, 100, 0),
(147720, 75, -1261.87, -1788.14, 66.6785, 0, 0, 0, NULL, 100, 0),
(147720, 76, -1249.45, -1783.07, 65.8931, 0, 0, 0, NULL, 100, 0),
(147720, 77, -1238.49, -1775.65, 64.62, 0, 0, 0, NULL, 100, 0),
(147720, 78, -1215.06, -1754.6, 60.4524, 0, 0, 0, NULL, 100, 0),
(147720, 79, -1194.54, -1735.55, 57.2315, 0, 0, 0, NULL, 100, 0),
(147720, 80, -1172.04, -1719.07, 54.128, 0, 0, 0, NULL, 100, 0),
(147720, 81, -1159.31, -1713.28, 52.2178, 0, 0, 0, NULL, 100, 0),
(147720, 82, -1139.35, -1706.8, 48.8841, 0, 0, 0, NULL, 100, 0),
(147720, 83, -1115.81, -1700, 45.5234, 0, 0, 0, NULL, 100, 0),
(147720, 84, -1092.46, -1692.58, 41.2765, 0, 0, 0, NULL, 100, 0),
(147720, 85, -1068.59, -1687.37, 38.652, 0, 0, 0, NULL, 100, 0),
(147720, 86, -1037.27, -1684.41, 36.8739, 0, 0, 0, NULL, 100, 0),
(147720, 87, -1008.56, -1684.45, 37.151, 0, 0, 0, NULL, 100, 0),
(147720, 88, -980.565, -1684.34, 37.7322, 0, 0, 0, NULL, 100, 0),
(147720, 89, -953.792, -1680.65, 39.8905, 0, 0, 0, NULL, 100, 0),
(147720, 90, -937.298, -1674.86, 41.9623, 0, 0, 0, NULL, 100, 0),
(147720, 91, -922.117, -1666.19, 44.3338, 0, 0, 0, NULL, 100, 0),
(147720, 92, -902.794, -1651.14, 47.3033, 0, 0, 0, NULL, 100, 0),
(147720, 93, -886.795, -1637.57, 49.7302, 0, 0, 0, NULL, 100, 0),
(147720, 94, -874.432, -1625.18, 51.403, 0, 0, 0, NULL, 100, 0),
(147720, 95, -861.587, -1613.31, 52.861, 0, 0, 0, NULL, 100, 0),
(147720, 96, -851.445, -1603.67, 53.5052, 0, 0, 0, NULL, 100, 0),
(147720, 97, -835.526, -1585.06, 54.2578, 0, 0, 0, NULL, 100, 0),
(147720, 98, -822.556, -1568.54, 54.1683, 0, 0, 0, NULL, 100, 0),
(147720, 99, -804.126, -1543, 54.2734, 0, 0, 0, NULL, 100, 0),
(147720, 100, -788.23, -1519.96, 55.9301, 0, 0, 0, NULL, 100, 0),
(147720, 101, -779.762, -1504.66, 57.6297, 0, 0, 0, NULL, 100, 0),
(147720, 102, -771.293, -1481.79, 60.9203, 0, 0, 0, NULL, 100, 0),
(147720, 103, -764.958, -1461.8, 63.8378, 0, 0, 0, NULL, 100, 0),
(147720, 104, -751.162, -1433.5, 66.2523, 0, 0, 0, NULL, 100, 0),
(147720, 105, -737.279, -1406.06, 68.0082, 0, 0, 0, NULL, 100, 0),
(147720, 106, -719.827, -1384.18, 68.2183, 0, 0, 0, NULL, 100, 0),
(147720, 107, -701.185, -1358.79, 67.6137, 0, 0, 0, NULL, 100, 0),
(147720, 108, -676.3, -1329.47, 67.0156, 0, 0, 0, NULL, 100, 0),
(147720, 109, -661.743, -1308.94, 66.136, 0, 0, 0, NULL, 100, 0),
(147720, 110, -646.567, -1281.38, 66.0949, 0, 0, 0, NULL, 100, 0),
(147720, 111, -626.252, -1248.74, 66.0719, 0, 0, 0, NULL, 100, 0),
(147720, 112, -612.39, -1216.62, 65.9958, 0, 0, 0, NULL, 100, 0),
(147720, 113, -600.363, -1191.33, 66.0663, 0, 0, 0, NULL, 100, 0),
(147720, 114, -587.601, -1158.84, 66.2411, 0, 0, 0, NULL, 100, 0),
(147720, 115, -580.715, -1135.33, 65.9731, 0, 0, 0, NULL, 100, 0),
(147720, 116, -569.879, -1109.61, 63.4267, 0, 0, 0, NULL, 100, 0),
(147720, 117, -559.217, -1094.17, 60.3274, 0, 0, 0, NULL, 100, 0),
(147720, 118, -539.599, -1074.21, 56.4081, 0, 0, 0, NULL, 100, 0),
(147720, 119, -523.011, -1056.35, 52.3473, 0, 0, 0, NULL, 100, 0),
(147720, 120, -510.191, -1040.07, 48.6025, 0, 0, 0, NULL, 100, 0),
(147720, 121, -501.715, -1022.04, 44.2363, 0, 0, 0, NULL, 100, 0),
(147720, 122, -496.385, -998.154, 40.2856, 0, 0, 0, NULL, 100, 0),
(147720, 123, -494.702, -980.738, 37.5433, 0, 0, 0, NULL, 100, 0),
(147720, 124, -491.376, -960.06, 33.8258, 0, 0, 0, NULL, 100, 0),
(147720, 125, -487.913, -946.496, 34.5604, 0, 0, 0, NULL, 100, 0),
(147720, 126, -483.667, -929.517, 33.981, 0, 0, 0, NULL, 100, 0),
(147720, 127, -479.713, -916.112, 34.5368, 0, 0, 0, NULL, 100, 0),
(147720, 128, -468.995, -898.097, 37.5774, 0, 0, 0, NULL, 100, 0),
(147720, 129, -452.089, -875.795, 43.0344, 0, 0, 0, NULL, 100, 0),
(147720, 130, -436.879, -856.588, 47.9147, 0, 0, 0, NULL, 100, 0),
(147720, 131, -421.433, -837.576, 52.3119, 0, 0, 0, NULL, 100, 0),
(147720, 132, -408.891, -820.775, 53.7396, 0, 0, 0, NULL, 100, 0),
(147720, 133, -396.945, -802.835, 54.5859, 0, 0, 0, NULL, 100, 0),
(147720, 134, -383.2, -783.4, 54.5997, 0, 0, 0, NULL, 100, 0),
(147720, 135, -375.514, -777.305, 54.4699, 0, 0, 0, NULL, 100, 0),
(147720, 136, -357.159, -768.896, 54.1852, 0, 0, 0, NULL, 100, 0),
(147720, 137, -331.407, -757.909, 53.8287, 0, 0, 0, NULL, 100, 0),
(147720, 138, -301.818, -747.177, 55.4595, 0, 0, 0, NULL, 100, 0),
(147720, 139, -275.645, -739.098, 56.6931, 0, 0, 0, NULL, 100, 0),
(147720, 140, -248.942, -730.377, 58.3106, 0, 0, 0, NULL, 100, 0),
(147720, 141, -217.832, -723.297, 61.0932, 0, 0, 0, NULL, 100, 0),
(147720, 142, -196.602, -718.715, 62.8802, 0, 0, 0, NULL, 100, 0),
(147720, 143, -172.911, -715.793, 63.6936, 0, 0, 0, NULL, 100, 0),
(147720, 144, -137.922, -715.202, 64.7113, 0, 0, 0, NULL, 100, 0),
(147720, 145, -105.48, -713.642, 65.4473, 0, 0, 0, NULL, 100, 0),
(147720, 146, -83.076, -715.929, 67.8031, 0, 0, 0, NULL, 100, 0),
(147720, 147, -62.1287, -717.276, 68.264, 0, 0, 0, NULL, 100, 0),
(147720, 148, -34.829, -715.287, 69.076, 0, 0, 0, NULL, 100, 0),
(147720, 149, -20.5953, -718.476, 69.3706, 0, 0, 0, NULL, 100, 0),
(147720, 150, -17.5473, -725.856, 68.4891, 0, 0, 0, NULL, 100, 0),
(147720, 151, -18.4696, -738.195, 66.1096, 0, 0, 0, NULL, 100, 0),
(147720, 152, -22.5461, -758.719, 62.5768, 0, 0, 0, NULL, 100, 0),
(147720, 153, -16.6739, -786.375, 59.6202, 0, 0, 0, NULL, 100, 0),
(147720, 154, -17.4867, -799.341, 58.967, 0, 0, 0, NULL, 100, 0),
(147720, 155, -28.1695, -814.096, 57.6642, 0, 0, 0, NULL, 100, 0),
(147720, 156, -44.6354, -826.165, 57.0593, 0, 0, 0, NULL, 100, 0),
(147720, 157, -52.8964, -844.203, 56.3922, 0, 0, 0, NULL, 100, 0),
(147720, 158, -50.6344, -863.401, 55.686, 0, 0, 0, NULL, 100, 0),
(147720, 159, -41.1772, -876.152, 55.8108, 0, 0, 0, NULL, 100, 0),
(147720, 160, -28.9852, -888.752, 56.1343, 0, 0, 0, NULL, 100, 0),
(147720, 161, -26.666, -897.974, 56.026, 0, 0, 0, NULL, 100, 0),
(147720, 162, -29.7521, -903.466, 55.8896, 0, 0, 0, NULL, 100, 0),
(147720, 163, -33.5225, -909.703, 55.5687, 0, 0, 0, NULL, 100, 0),
(147720, 164, -40.2458, -919.112, 54.8082, 0, 0, 0, NULL, 100, 0),
(147720, 165, -37.9023, -916.188, 55.1725, 0, 0, 0, NULL, 100, 0),
(147720, 166, -30.3595, -904.044, 55.8805, 0, 0, 0, NULL, 100, 0),
(147720, 167, -27.4496, -898.895, 56.0401, 0, 0, 0, NULL, 100, 0),
(147720, 168, -28.8969, -890.134, 56.0851, 0, 0, 0, NULL, 100, 0),
(147720, 169, -39.6026, -878.277, 55.9009, 0, 0, 0, NULL, 100, 0),
(147720, 170, -49.9883, -864.653, 55.6548, 0, 0, 0, NULL, 100, 0),
(147720, 171, -53.5191, -847.529, 56.3265, 0, 0, 0, NULL, 100, 0),
(147720, 172, -45.6486, -827.843, 57.0118, 0, 0, 0, NULL, 100, 0),
(147720, 173, -30.2519, -815.667, 57.6413, 0, 0, 0, NULL, 100, 0),
(147720, 174, -18.748, -801.928, 58.75, 0, 0, 0, NULL, 100, 0),
(147720, 175, -16.2843, -788.799, 59.5083, 0, 0, 0, NULL, 100, 0),
(147720, 176, -22.8308, -761.081, 62.172, 0, 0, 0, NULL, 100, 0),
(147720, 177, -18.7154, -740.543, 65.6946, 0, 0, 0, NULL, 100, 0),
(147720, 178, -17.5494, -726.441, 68.377, 0, 0, 0, NULL, 100, 0),
(147720, 179, -21.3868, -718.702, 69.3223, 0, 0, 0, NULL, 100, 0),
(147720, 180, -34.1316, -715.917, 69.0751, 0, 0, 0, NULL, 100, 0),
(147720, 181, -60.9359, -717.195, 68.3029, 0, 0, 0, NULL, 100, 0),
(147720, 182, -80.9546, -715.894, 67.9737, 0, 0, 0, NULL, 100, 0),
(147720, 183, -106.424, -713.731, 65.3761, 0, 0, 0, NULL, 100, 0),
(147720, 184, -137.902, -714.769, 64.7226, 0, 0, 0, NULL, 100, 0),
(147720, 185, -172.878, -715.923, 63.6948, 0, 0, 0, NULL, 100, 0),
(147720, 186, -193.71, -718.413, 63.0221, 0, 0, 0, NULL, 100, 0),
(147720, 187, -216.695, -722.939, 61.1133, 0, 0, 0, NULL, 100, 0),
(147720, 188, -247.633, -730.432, 58.4478, 0, 0, 0, NULL, 100, 0),
(147720, 189, -273.719, -738.461, 56.7533, 0, 0, 0, NULL, 100, 0),
(147720, 190, -299.708, -746.781, 55.6276, 0, 0, 0, NULL, 100, 0),
(147720, 191, -329.496, -757.193, 53.9239, 0, 0, 0, NULL, 100, 0),
(147720, 192, -354.54, -768.04, 54.2285, 0, 0, 0, NULL, 100, 0),
(147720, 193, -375.293, -777.319, 54.4701, 0, 0, 0, NULL, 100, 0),
(147720, 194, -383.495, -784.036, 54.6087, 0, 0, 0, NULL, 100, 0),
(147720, 195, -396.589, -802.639, 54.5903, 0, 0, 0, NULL, 100, 0),
(147720, 196, -409.141, -821.598, 53.6989, 0, 0, 0, NULL, 100, 0),
(147720, 197, -420.379, -835.882, 52.5394, 0, 0, 0, NULL, 100, 0),
(147720, 198, -434.7, -853.557, 48.7153, 0, 0, 0, NULL, 100, 0),
(147720, 199, -451.476, -875.092, 43.2386, 0, 0, 0, NULL, 100, 0),
(147720, 200, -468.216, -896.654, 37.8402, 0, 0, 0, NULL, 100, 0),
(147720, 201, -479.431, -916.404, 34.5483, 0, 0, 0, NULL, 100, 0),
(147720, 202, -483.477, -929.436, 34.0016, 0, 0, 0, NULL, 100, 0),
(147720, 203, -486.836, -942.665, 34.6166, 0, 0, 0, NULL, 100, 0),
(147720, 204, -490.311, -955.864, 33.8972, 0, 0, 0, NULL, 100, 0),
(147720, 205, -494.413, -978.225, 37.0571, 0, 0, 0, NULL, 100, 0),
(147720, 206, -495.922, -996.353, 39.9845, 0, 0, 0, NULL, 100, 0),
(147720, 207, -501.894, -1021.3, 44.1397, 0, 0, 0, NULL, 100, 0),
(147720, 208, -508.274, -1036.2, 47.7283, 0, 0, 0, NULL, 100, 0),
(147720, 209, -521.649, -1054.57, 52.0204, 0, 0, 0, NULL, 100, 0),
(147720, 210, -536.872, -1071.44, 56.0008, 0, 0, 0, NULL, 100, 0),
(147720, 211, -556.428, -1090.44, 59.4959, 0, 0, 0, NULL, 100, 0),
(147720, 212, -569.52, -1109.03, 63.3078, 0, 0, 0, NULL, 100, 0),
(147720, 213, -580.307, -1134, 65.9386, 0, 0, 0, NULL, 100, 0),
(147720, 214, -587.943, -1159.8, 66.2384, 0, 0, 0, NULL, 100, 0),
(147720, 215, -599.68, -1189.38, 66.071, 0, 0, 0, NULL, 100, 0),
(147720, 216, -611.207, -1214.11, 66.0131, 0, 0, 0, NULL, 100, 0),
(147720, 217, -625.737, -1247.48, 66.0764, 0, 0, 0, NULL, 100, 0),
(147720, 218, -644.589, -1278.51, 66.0956, 0, 0, 0, NULL, 100, 0),
(147720, 219, -660.145, -1306.27, 66.1604, 0, 0, 0, NULL, 100, 0),
(147720, 220, -675.672, -1328.66, 66.9782, 0, 0, 0, NULL, 100, 0),
(147720, 221, -699.123, -1356.5, 67.6145, 0, 0, 0, NULL, 100, 0),
(147720, 222, -718.071, -1382.09, 68.1733, 0, 0, 0, NULL, 100, 0),
(147720, 223, -737.753, -1407.11, 67.967, 0, 0, 0, NULL, 100, 0),
(147720, 224, -750.189, -1431.38, 66.3932, 0, 0, 0, NULL, 100, 0),
(147720, 225, -764.174, -1459.99, 64.0482, 0, 0, 0, NULL, 100, 0),
(147720, 226, -771.283, -1481.56, 60.9477, 0, 0, 0, NULL, 100, 0),
(147720, 227, -778.669, -1502.06, 58.0151, 0, 0, 0, NULL, 100, 0),
(147720, 228, -786.991, -1518, 56.1296, 0, 0, 0, NULL, 100, 0),
(147720, 229, -802.538, -1540.44, 54.3658, 0, 0, 0, NULL, 100, 0),
(147720, 230, -821.021, -1566.36, 54.1669, 0, 0, 0, NULL, 100, 0),
(147720, 231, -834.994, -1584.29, 54.2546, 0, 0, 0, NULL, 100, 0),
(147720, 232, -851.463, -1603.03, 53.5575, 0, 0, 0, NULL, 100, 0),
(147720, 233, -861.063, -1612.73, 52.8747, 0, 0, 0, NULL, 100, 0),
(147720, 234, -871.01, -1622.07, 52.0091, 0, 0, 0, NULL, 100, 0),
(147720, 235, -884.087, -1634.72, 49.907, 0, 0, 0, NULL, 100, 0),
(147720, 236, -901.22, -1649.65, 47.6492, 0, 0, 0, NULL, 100, 0),
(147720, 237, -919.01, -1663.83, 44.7569, 0, 0, 0, NULL, 100, 0),
(147720, 238, -936.362, -1674.18, 42.1281, 0, 0, 0, NULL, 100, 0),
(147720, 239, -951.672, -1680.22, 40.0861, 0, 0, 0, NULL, 100, 0),
(147720, 240, -978.641, -1684.29, 37.8759, 0, 0, 0, NULL, 100, 0),
(147720, 241, -1004.88, -1684.59, 37.0962, 0, 0, 0, NULL, 100, 0),
(147720, 242, -1036.73, -1684.53, 36.8569, 0, 0, 0, NULL, 100, 0),
(147720, 243, -1063.92, -1686.7, 38.2989, 0, 0, 0, NULL, 100, 0),
(147720, 244, -1090.65, -1692.18, 40.9512, 0, 0, 0, NULL, 100, 0),
(147720, 245, -1112.34, -1698.96, 45.1239, 0, 0, 0, NULL, 100, 0),
(147720, 246, -1138.5, -1706.74, 48.6789, 0, 0, 0, NULL, 100, 0),
(147720, 247, -1155.85, -1712.21, 51.8194, 0, 0, 0, NULL, 100, 0),
(147720, 248, -1171.71, -1719.06, 54.0856, 0, 0, 0, NULL, 100, 0),
(147720, 249, -1193.8, -1735, 57.0706, 0, 0, 0, NULL, 100, 0),
(147720, 250, -1213.93, -1753.41, 60.187, 0, 0, 0, NULL, 100, 0),
(147720, 251, -1237.63, -1774.69, 64.4626, 0, 0, 0, NULL, 100, 0),
(147720, 252, -1248.78, -1782.43, 65.819, 0, 0, 0, NULL, 100, 0),
(147720, 253, -1258.66, -1786.65, 66.482, 0, 0, 0, NULL, 100, 0),
(147720, 254, -1286.65, -1801.77, 66.8709, 0, 0, 0, NULL, 100, 0),
(147720, 255, -1305.07, -1814.08, 65.4825, 0, 0, 0, NULL, 100, 0),
(147720, 256, -1315.82, -1825.3, 64.4726, 0, 0, 0, NULL, 100, 0),
(147720, 257, -1325.03, -1840.48, 63.2467, 0, 0, 0, NULL, 100, 0),
(147720, 258, -1328.78, -1852.54, 62.4244, 0, 0, 0, NULL, 100, 0),
(147720, 259, -1338.59, -1882.78, 60.2984, 0, 0, 0, NULL, 100, 0),
(147720, 260, -1349.41, -1912.74, 58.414, 0, 0, 0, NULL, 100, 0),
(147720, 261, -1361.58, -1942.17, 58.429, 0, 0, 0, NULL, 100, 0),
(147720, 262, -1372.17, -1967.33, 58.476, 0, 0, 0, NULL, 100, 0),
(147720, 263, -1376.43, -1980.29, 58.5393, 0, 0, 0, NULL, 100, 0),
(147720, 264, -1378.99, -1998.28, 59.1872, 0, 0, 0, NULL, 100, 0),
(147720, 265, -1381.75, -2030.01, 60.7155, 0, 0, 0, NULL, 100, 0),
(147720, 266, -1383.96, -2066.34, 62.2794, 0, 0, 0, NULL, 100, 0),
(147720, 267, -1385.64, -2093.57, 63.5406, 0, 0, 0, NULL, 100, 0),
(147720, 268, -1389.12, -2129.8, 64.3013, 0, 0, 0, NULL, 100, 0),
(147720, 269, -1395.05, -2161.59, 64.1007, 0, 0, 0, NULL, 100, 0),
(147720, 270, -1399.36, -2183.92, 63.9526, 0, 0, 0, NULL, 100, 0),
(147720, 271, -1405.66, -2210.48, 63.8462, 0, 0, 0, NULL, 100, 0),
(147720, 272, -1410.75, -2227.95, 63.7957, 0, 0, 0, NULL, 100, 0),
(147720, 273, -1417.65, -2244.76, 63.7518, 0, 0, 0, NULL, 100, 0),
(147720, 274, -1427.39, -2259.19, 63.3891, 0, 0, 0, NULL, 100, 0),
(147720, 275, -1438.66, -2273.46, 63.0608, 0, 0, 0, NULL, 100, 0),
(147720, 276, -1444.57, -2283.39, 62.2072, 0, 0, 0, NULL, 100, 0),
(147720, 277, -1449.32, -2302.21, 61.8304, 0, 0, 0, NULL, 100, 0),
(147720, 278, -1450.22, -2324.94, 61.7334, 0, 0, 0, NULL, 100, 0),
(147720, 279, -1450.08, -2347.68, 61.5991, 0, 0, 0, NULL, 100, 0),
(147720, 280, -1449.96, -2365.88, 61.4915, 0, 0, 0, NULL, 100, 0),
(147720, 281, -1450.25, -2388.62, 61.3176, 0, 0, 0, NULL, 100, 0),
(147720, 282, -1450.85, -2402.24, 60.7831, 0, 0, 0, NULL, 100, 0),
(147720, 283, -1457.88, -2418.95, 58.9511, 0, 0, 0, NULL, 100, 0),
(147720, 284, -1473.83, -2435.1, 56.4162, 0, 0, 0, NULL, 100, 0),
(147720, 285, -1491.02, -2449.99, 53.7364, 0, 0, 0, NULL, 100, 0),
(147720, 286, -1505.01, -2461.63, 52.9892, 0, 0, 0, NULL, 100, 0),
(147720, 287, -1519.04, -2473.22, 53.2747, 0, 0, 0, NULL, 100, 0),
(147720, 288, -1536.46, -2487.83, 53.5903, 0, 0, 0, NULL, 100, 0),
(147720, 289, -1545.19, -2496.39, 54.1743, 0, 0, 0, NULL, 100, 0),
(147720, 290, -1550.28, -2506.8, 53.9085, 0, 0, 0, NULL, 100, 0),
(147720, 291, -1554.16, -2515.03, 53.3954, 0, 0, 0, NULL, 100, 0),
(147720, 292, -1560.37, -2527.18, 52.3382, 0, 0, 0, NULL, 100, 0),
(147720, 293, -1567.58, -2549.4, 50.7789, 0, 0, 0, NULL, 100, 0),
(147720, 294, -1567.29, -2570.35, 49.3619, 0, 0, 0, NULL, 100, 0),
(147720, 295, -1562.37, -2591.16, 48.5309, 0, 0, 0, NULL, 100, 0),
(147720, 296, -1557.69, -2603.97, 47.8564, 0, 0, 0, NULL, 100, 0),
(147720, 297, -1555.93, -2619.39, 45.2651, 0, 0, 0, NULL, 100, 0),
(147720, 298, -1555.27, -2642.13, 41.156, 0, 0, 0, NULL, 100, 0),
(147720, 299, -1557.99, -2669.24, 36.7433, 0, 0, 0, NULL, 100, 0),
(147720, 300, -1564.24, -2685.93, 35.3788, 0, 0, 0, NULL, 100, 0),
(147720, 301, -1570.22, -2693.06, 34.7322, 0, 0, 0, NULL, 100, 0),
(147720, 302, -1585.9, -2709.52, 35.9957, 0, 0, 0, NULL, 100, 0),
(147720, 303, -1597.38, -2723.02, 37.1346, 0, 0, 0, NULL, 100, 0),
(147720, 304, -1606.15, -2738.58, 37.5876, 0, 0, 0, NULL, 100, 0),
(147720, 305, -1610.13, -2756.28, 37.2692, 0, 0, 0, NULL, 100, 0),
(147720, 306, -1612.47, -2774.32, 36.3171, 0, 0, 0, NULL, 100, 0),
(147720, 307, -1617.11, -2801.22, 34.8703, 0, 0, 0, NULL, 100, 0),
(147720, 308, -1620.57, -2819.08, 33.8744, 0, 0, 0, NULL, 100, 0),
(147720, 309, -1625.11, -2841.37, 32.3162, 0, 0, 0, NULL, 100, 0),
(147720, 310, -1632.1, -2867.75, 30.009, 0, 0, 0, NULL, 100, 0),
(147720, 311, -1638.53, -2884.76, 27.7024, 0, 0, 0, NULL, 100, 0),
(147720, 312, -1644, -2901.43, 26.3001, 0, 0, 0, NULL, 100, 0),
(147720, 313, -1641.88, -2915.77, 26.7946, 0, 0, 0, NULL, 100, 0),
(147720, 314, -1637.81, -2933.46, 28.1655, 0, 0, 0, NULL, 100, 0),
(147720, 315, -1638.86, -2944.22, 28.5141, 0, 0, 0, NULL, 100, 0),
(147720, 316, -1643.12, -2959.2, 27.6933, 0, 0, 0, NULL, 100, 0),
(147720, 317, -1645.97, -2971.19, 26.3978, 0, 0, 0, NULL, 100, 0),
(147720, 318, -1645.94, -2988.94, 23.7789, 0, 0, 0, NULL, 100, 0),
(147720, 319, -1640.24, -3005.1, 20.8809, 0, 0, 0, NULL, 100, 0),
(147720, 320, -1634.33, -3012.31, 19.4302, 0, 0, 0, NULL, 100, 0),
(147720, 321, -1604.85, -3044.09, 14.9102, 0, 0, 0, NULL, 100, 0),
(147720, 322, -1584.7, -3059.58, 12.848, 0, 0, 0, NULL, 100, 0),
(147720, 323, -1569.47, -3066.36, 13.156, 0, 0, 0, NULL, 100, 0),
(147720, 324, -1559.23, -3066.54, 13.1732, 0, 0, 0, NULL, 100, 0),
(147720, 325, -1551.49, -3056.18, 13.011, 0, 0, 0, NULL, 100, 0),
(147720, 326, -1551.22, -3043.06, 13.002, 0, 0, 0, NULL, 100, 0),
(147720, 327, -1552.52, -3029.32, 13.6458, 5.00963, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (14772, 147720, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 14772);

-- Importing creature with guid 14773 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1014666 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1014666 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1014666 AND spawnType = 0 WHERE ce.spawnID = 1014666;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1014667 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1014667 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1014667 AND spawnType = 0 WHERE ce.spawnID = 1014667;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1014668 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1014668 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1014668 AND spawnType = 0 WHERE ce.spawnID = 1014668;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1014669 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1014669 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1014669 AND spawnType = 0 WHERE ce.spawnID = 1014669;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1016283 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1016283 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1016283 AND spawnType = 0 WHERE ce.spawnID = 1016283;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1016284 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1016284 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1016284 AND spawnType = 0 WHERE ce.spawnID = 1016284;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1016285 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1016285 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1016285 AND spawnType = 0 WHERE ce.spawnID = 1016285;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1016287 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1016287 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1016287 AND spawnType = 0 WHERE ce.spawnID = 1016287;
INSERT INTO creature_entry (spawnID, entry) VALUES (14773, 2721);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14773, 0, 1, 4037, 1, -1551.74, -3031.64,  13.6448, 5.00963, 60, 0, 0, 610, 0, 0, 0, 0, 10);

-- Importing creature with guid 14774 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (14774, 2721);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14774, 0, 1, 4037, 1, -1554.4, -3029.45,  13.6448, 5.00963, 60, 0, 0, 610, 0, 0, 0, 0, 10);

-- Importing creature with guid 14775 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (14775, 2721);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14775, 0, 1, 4038, 1, -1553.33, -3026.49,  13.6448, 5.00963, 60, 0, 0, 610, 0, 0, 0, 0, 10);

-- Importing creature with guid 14572 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1014480 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1014480 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1014480 AND spawnType = 0 WHERE ce.spawnID = 1014480;
INSERT INTO creature_entry (spawnID, entry) VALUES (14572, 2612);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14572, 0, 1, 4141, 1, -1528.31, -1843.3,  67.7893, 1.53044, 400, 0, 1, 1135, 1779, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(145720, 1, -1528.31, -1843.3, 67.79, 1.53044, 60000, 0, NULL, 100, 0),
(145720, 2, -1528.23, -1824.05, 68.34, 0, 0, 0, NULL, 100, 0),
(145720, 3, -1528.32, -1820.46, 68.9602, 0, 0, 0, NULL, 100, 0),
(145720, 4, -1528.37, -1818.28, 69.5115, 0, 0, 0, NULL, 100, 0),
(145720, 5, -1528.64, -1808.76, 71.3902, 0, 0, 0, NULL, 100, 0),
(145720, 6, -1528.77, -1800.37, 71.2294, 0, 0, 0, NULL, 100, 0),
(145720, 7, -1528.82, -1795.04, 70.1311, 0, 0, 0, NULL, 100, 0),
(145720, 8, -1528.84, -1793.37, 69.5099, 0, 0, 0, NULL, 100, 0),
(145720, 9, -1528.87, -1790.61, 68.9584, 0, 0, 0, NULL, 100, 0),
(145720, 10, -1528.82, -1785.05, 68.3404, 0, 0, 0, NULL, 100, 0),
(145720, 11, -1527.51, -1762.21, 67.0403, 0, 0, 0, NULL, 100, 0),
(145720, 12, -1529.35, -1746.33, 66.8509, 0, 0, 0, NULL, 100, 0),
(145720, 13, -1529.5, -1732.3, 66.2708, 0, 0, 0, NULL, 100, 0),
(145720, 14, -1539.93, -1714.01, 66.363, 0, 0, 0, NULL, 100, 0),
(145720, 15, -1555.47, -1702.87, 66.1763, 0, 0, 0, NULL, 100, 0),
(145720, 16, -1570.19, -1696.83, 66.2239, 0, 0, 0, NULL, 100, 0),
(145720, 17, -1576.23, -1695.24, 65.9776, 0, 0, 0, NULL, 100, 0),
(145720, 18, -1581.84, -1696.44, 66.1376, 0, 0, 0, NULL, 100, 0),
(145720, 19, -1584.78, -1700.91, 66.5301, 0, 0, 0, NULL, 100, 0),
(145720, 20, -1586.19, -1710.78, 66.9607, 0, 0, 0, NULL, 100, 0),
(145720, 21, -1586.08, -1728.19, 67.4567, 0, 0, 0, NULL, 100, 0),
(145720, 22, -1582.99, -1743.81, 66.8545, 0, 0, 0, NULL, 100, 0),
(145720, 23, -1577.8, -1763.7, 67.0782, 0, 0, 0, NULL, 100, 0),
(145720, 24, -1576.49, -1777.64, 67.2815, 0, 0, 0, NULL, 100, 0),
(145720, 25, -1574.01, -1787.45, 67.1901, 0, 0, 0, NULL, 100, 0),
(145720, 26, -1565.38, -1798.76, 67.5678, 0, 0, 0, NULL, 100, 0),
(145720, 27, -1557.03, -1803.77, 67.5158, 0, 0, 0, NULL, 100, 0),
(145720, 28, -1540, -1805.78, 66.432, 0, 0, 0, NULL, 100, 0),
(145720, 29, -1513.43, -1805.73, 66.1469, 0, 0, 0, NULL, 100, 0),
(145720, 30, -1490.95, -1805.66, 68.0658, 0, 0, 0, NULL, 100, 0),
(145720, 31, -1466.56, -1803.87, 67.7702, 0, 0, 0, NULL, 100, 0),
(145720, 32, -1452.87, -1800.94, 65.4412, 0, 0, 0, NULL, 100, 0),
(145720, 33, -1438.86, -1799.91, 61.945, 0, 0, 0, NULL, 100, 0),
(145720, 34, -1425.38, -1803.61, 60.9587, 0, 0, 0, NULL, 100, 0),
(145720, 35, -1395.89, -1814.67, 59.1394, 0, 0, 0, NULL, 100, 0),
(145720, 36, -1363.65, -1828.28, 60.0694, 0, 0, 0, NULL, 100, 0),
(145720, 37, -1337.27, -1839.82, 62.1984, 0, 0, 0, NULL, 100, 0),
(145720, 38, -1330.7, -1845.84, 62.5435, 0, 0, 0, NULL, 100, 0),
(145720, 39, -1328.27, -1851.3, 62.5249, 0, 0, 0, NULL, 100, 0),
(145720, 40, -1329.52, -1861.33, 62.0214, 0, 0, 0, NULL, 100, 0),
(145720, 41, -1342.83, -1896.34, 59.2707, 0, 0, 0, NULL, 100, 0),
(145720, 42, -1358.08, -1935.47, 58.4276, 0, 0, 0, NULL, 100, 0),
(145720, 43, -1375.19, -1977.63, 58.5289, 0, 0, 0, NULL, 100, 0),
(145720, 44, -1381.06, -2026.16, 60.5442, 0, 0, 0, NULL, 100, 0),
(145720, 45, -1383.41, -2071.59, 62.5124, 0, 0, 0, NULL, 100, 0),
(145720, 46, -1386.02, -2110, 64.2494, 0, 0, 0, NULL, 100, 0),
(145720, 47, -1393.31, -2158.42, 64.1111, 0, 0, 0, NULL, 100, 0),
(145720, 48, -1404.24, -2206.13, 63.8535, 0, 0, 0, NULL, 100, 0),
(145720, 49, -1417.1, -2243.68, 63.7514, 0, 0, 0, NULL, 100, 0),
(145720, 50, -1420.49, -2249.98, 63.6904, 0, 0, 0, NULL, 100, 0),
(145720, 51, -1430.49, -2263.3, 63.329, 0, 0, 0, NULL, 100, 0),
(145720, 52, -1441.13, -2277.2, 62.7836, 0, 0, 0, NULL, 100, 0),
(145720, 53, -1446.84, -2287.53, 61.9325, 0, 0, 0, NULL, 100, 0),
(145720, 54, -1449.19, -2299.55, 61.8796, 0, 0, 0, NULL, 100, 0),
(145720, 55, -1449.73, -2327.53, 61.7191, 0, 0, 0, NULL, 100, 0),
(145720, 56, -1450.03, -2355.53, 61.5528, 0, 0, 0, NULL, 100, 0),
(145720, 57, -1450.15, -2380.03, 61.4055, 0, 0, 0, NULL, 100, 0),
(145720, 58, -1450.49, -2397.53, 61.0555, 0, 0, 0, NULL, 100, 0),
(145720, 59, -1453.92, -2410.52, 60.03, 0, 0, 0, NULL, 100, 0),
(145720, 60, -1464.35, -2426.89, 57.8205, 0, 0, 0, NULL, 100, 0),
(145720, 61, -1462.44, -2436.38, 58.0342, 0, 0, 0, NULL, 100, 0),
(145720, 62, -1453.73, -2438.5, 58.342, 0, 0, 0, NULL, 100, 0),
(145720, 63, -1440.72, -2440.2, 58.9664, 0, 0, 0, NULL, 100, 0),
(145720, 64, -1424.83, -2453.5, 59.0382, 0, 0, 0, NULL, 100, 0),
(145720, 65, -1412.62, -2462.65, 57.2066, 0, 0, 0, NULL, 100, 0),
(145720, 66, -1396.91, -2462.16, 57.0177, 0, 0, 0, NULL, 100, 0),
(145720, 67, -1380.78, -2455.37, 55.0267, 0, 0, 0, NULL, 100, 0),
(145720, 68, -1364.89, -2450.51, 52.0455, 0, 0, 0, NULL, 100, 0),
(145720, 69, -1351.11, -2448.04, 49.9138, 0, 0, 0, NULL, 100, 0),
(145720, 70, -1336.2, -2448.68, 46.4684, 0, 0, 0, NULL, 100, 0),
(145720, 71, -1319.9, -2455.01, 43.1772, 0, 0, 0, NULL, 100, 0),
(145720, 72, -1304.21, -2462.75, 39.1337, 0, 0, 0, NULL, 100, 0),
(145720, 73, -1289.01, -2471.38, 33.4891, 0, 0, 0, NULL, 100, 0),
(145720, 74, -1275.46, -2482.79, 26.7595, 0, 0, 0, NULL, 100, 0),
(145720, 75, -1267.01, -2493.63, 22.4559, 0, 0, 0, NULL, 100, 0),
(145720, 76, -1254.69, -2512.44, 20.5413, 0, 0, 0, NULL, 100, 0),
(145720, 77, -1250.21, -2521.37, 20.4269, 0, 0, 0, NULL, 100, 0),
(145720, 78, -1246.19, -2529.78, 20.6058, 0, 0, 0, NULL, 100, 0),
(145720, 79, -1242.05, -2535.15, 21.0179, 0, 0, 0, NULL, 100, 0),
(145720, 80, -1242.99, -2526.84, 20.9146, 0, 0, 0, NULL, 100, 0),
(145720, 81, -1250.83, -2520.96, 20.426, 0, 60000, 0, NULL, 100, 0),
(145720, 82, -1254.85, -2513.19, 20.5112, 0, 0, 0, NULL, 100, 0),
(145720, 83, -1267.24, -2493.93, 22.3988, 0, 0, 0, NULL, 100, 0),
(145720, 84, -1273.85, -2485.78, 25.5707, 0, 0, 0, NULL, 100, 0),
(145720, 85, -1289.24, -2471.59, 33.4718, 0, 0, 0, NULL, 100, 0),
(145720, 86, -1303.98, -2462.99, 39.0339, 0, 0, 0, NULL, 100, 0),
(145720, 87, -1316.64, -2457, 42.604, 0, 0, 0, NULL, 100, 0),
(145720, 88, -1335.67, -2449.4, 46.2431, 0, 0, 0, NULL, 100, 0),
(145720, 89, -1349.51, -2448.47, 49.503, 0, 0, 0, NULL, 100, 0),
(145720, 90, -1363.26, -2451.05, 51.8257, 0, 0, 0, NULL, 100, 0),
(145720, 91, -1379.15, -2455.53, 54.7104, 0, 0, 0, NULL, 100, 0),
(145720, 92, -1395.58, -2461.5, 57.1067, 0, 0, 0, NULL, 100, 0),
(145720, 93, -1411.2, -2463.61, 56.9774, 0, 0, 0, NULL, 100, 0),
(145720, 94, -1424.18, -2454.31, 59.0152, 0, 0, 0, NULL, 100, 0),
(145720, 95, -1440.89, -2442.08, 58.7537, 0, 0, 0, NULL, 100, 0),
(145720, 96, -1452.92, -2439.49, 58.3243, 0, 0, 0, NULL, 100, 0),
(145720, 97, -1461.72, -2436.45, 58.0823, 0, 0, 0, NULL, 100, 0),
(145720, 98, -1463.37, -2427.9, 57.8549, 0, 0, 0, NULL, 100, 0),
(145720, 99, -1454.66, -2411.75, 59.8753, 0, 0, 0, NULL, 100, 0),
(145720, 100, -1451.1, -2399.07, 60.9648, 0, 0, 0, NULL, 100, 0),
(145720, 101, -1450.14, -2381.61, 61.3911, 0, 0, 0, NULL, 100, 0),
(145720, 102, -1450.12, -2357.11, 61.5426, 0, 0, 0, NULL, 100, 0),
(145720, 103, -1449.95, -2329.11, 61.7066, 0, 0, 0, NULL, 100, 0),
(145720, 104, -1449.43, -2301.13, 61.84, 0, 0, 0, NULL, 100, 0),
(145720, 105, -1446.58, -2287.66, 61.9711, 0, 0, 0, NULL, 100, 0),
(145720, 106, -1442.44, -2279.13, 62.5159, 0, 0, 0, NULL, 100, 0),
(145720, 107, -1431.93, -2265.15, 63.3205, 0, 0, 0, NULL, 100, 0),
(145720, 108, -1421.36, -2251.22, 63.6684, 0, 0, 0, NULL, 100, 0),
(145720, 109, -1418.07, -2245.04, 63.7351, 0, 0, 0, NULL, 100, 0),
(145720, 110, -1405.35, -2208.74, 63.8461, 0, 0, 0, NULL, 100, 0),
(145720, 111, -1394.01, -2161.12, 64.1057, 0, 0, 0, NULL, 100, 0),
(145720, 112, -1386.65, -2112.7, 64.4159, 0, 0, 0, NULL, 100, 0),
(145720, 113, -1383.82, -2074.31, 62.6099, 0, 0, 0, NULL, 100, 0),
(145720, 114, -1381.54, -2028.88, 60.6584, 0, 0, 0, NULL, 100, 0),
(145720, 115, -1375.91, -1979.41, 58.5366, 0, 0, 0, NULL, 100, 0),
(145720, 116, -1358.95, -1937.19, 58.4249, 0, 0, 0, NULL, 100, 0),
(145720, 117, -1343.72, -1898.05, 59.126, 0, 0, 0, NULL, 100, 0),
(145720, 118, -1329.89, -1863.08, 61.9069, 0, 0, 0, NULL, 100, 0),
(145720, 119, -1328.35, -1850.86, 62.5391, 0, 0, 0, NULL, 100, 0),
(145720, 120, -1330.65, -1846.29, 62.5257, 0, 0, 0, NULL, 100, 0),
(145720, 121, -1335.96, -1841.73, 62.249, 0, 0, 0, NULL, 100, 0),
(145720, 122, -1361.59, -1829.16, 60.2512, 0, 0, 0, NULL, 100, 0),
(145720, 123, -1393.84, -1815.58, 59.0036, 0, 0, 0, NULL, 100, 0),
(145720, 124, -1423.47, -1804.92, 60.7771, 0, 0, 0, NULL, 100, 0),
(145720, 125, -1438.32, -1800.12, 61.8788, 0, 0, 0, NULL, 100, 0),
(145720, 126, -1450.61, -1800.69, 64.7219, 0, 0, 0, NULL, 100, 0),
(145720, 127, -1464.33, -1803.5, 67.6348, 0, 0, 0, NULL, 100, 0),
(145720, 128, -1488.18, -1805.87, 68.074, 0, 0, 0, NULL, 100, 0),
(145720, 129, -1512.68, -1805.73, 66.2104, 0, 0, 0, NULL, 100, 0),
(145720, 130, -1537.17, -1805.95, 66.1776, 0, 0, 0, NULL, 100, 0),
(145720, 131, -1557.65, -1804.11, 67.4839, 0, 0, 0, NULL, 100, 0),
(145720, 132, -1565.61, -1798.65, 67.5617, 0, 0, 0, NULL, 100, 0),
(145720, 133, -1574.15, -1787.07, 67.1787, 0, 0, 0, NULL, 100, 0),
(145720, 134, -1576.5, -1778.67, 67.2749, 0, 0, 0, NULL, 100, 0),
(145720, 135, -1578.05, -1765.2, 67.1097, 0, 0, 0, NULL, 100, 0),
(145720, 136, -1582.3, -1746.61, 66.6746, 0, 0, 0, NULL, 100, 0),
(145720, 137, -1585.78, -1729.49, 67.4793, 0, 0, 0, NULL, 100, 0),
(145720, 138, -1586.16, -1712.03, 67.0329, 0, 0, 0, NULL, 100, 0),
(145720, 139, -1584.72, -1701.41, 66.5389, 0, 0, 0, NULL, 100, 0),
(145720, 140, -1581.69, -1696.92, 66.1604, 0, 0, 0, NULL, 100, 0),
(145720, 141, -1576.71, -1695.36, 65.9892, 0, 0, 0, NULL, 100, 0),
(145720, 142, -1570.6, -1696.58, 66.1985, 0, 0, 0, NULL, 100, 0),
(145720, 143, -1555.01, -1703.09, 66.1586, 0, 0, 0, NULL, 100, 0),
(145720, 144, -1540.2, -1714.01, 66.3447, 0, 0, 0, NULL, 100, 0),
(145720, 145, -1529.35, -1731.15, 66.2182, 0, 0, 0, NULL, 100, 0),
(145720, 146, -1529.12, -1745.12, 66.8033, 0, 0, 0, NULL, 100, 0),
(145720, 147, -1527.21, -1763.24, 67.0823, 0, 0, 0, NULL, 100, 0),
(145720, 148, -1528.8, -1786.19, 68.3406, 0, 0, 0, NULL, 100, 0),
(145720, 149, -1528.78, -1791.07, 68.9602, 0, 0, 0, NULL, 100, 0),
(145720, 150, -1528.77, -1793.2, 69.5115, 0, 0, 0, NULL, 100, 0),
(145720, 151, -1528.76, -1795.29, 70.1611, 0, 0, 0, NULL, 100, 0),
(145720, 152, -1528.74, -1800.29, 71.2228, 0, 0, 0, NULL, 100, 0),
(145720, 153, -1528.67, -1807.79, 71.4654, 0, 0, 0, NULL, 100, 0),
(145720, 154, -1528.38, -1819.6, 68.9567, 0, 0, 0, NULL, 100, 0),
(145720, 155, -1528.32, -1822.06, 68.3404, 0, 0, 0, NULL, 100, 0),
(145720, 156, -1528.25, -1824.82, 68.3404, 0, 0, 0, NULL, 100, 0),
(145720, 157, -1528.2, -1828.96, 68.0658, 0, 0, 0, NULL, 100, 0),
(145720, 158, -1533.15, -1841.45, 68.09, 0, 0, 0, NULL, 100, 0),
(145720, 159, -1528.19, -1853.83, 67.62, 0, 0, 0, NULL, 100, 0),
(145720, 160, -1528.31, -1843.3, 67.79, 1.53044, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (14572, 145720, 229, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 14572);

-- Importing creature with guid 14574 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1014482 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1014482 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1014482 AND spawnType = 0 WHERE ce.spawnID = 1014482;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1014483 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1014483 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1014483 AND spawnType = 0 WHERE ce.spawnID = 1014483;
INSERT INTO creature_entry (spawnID, entry) VALUES (14574, 2738);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14574, 0, 1, 4145, 1, -1525.9, -1845.88,  67.7647, 1.53044, 400, 0, 0, 958, 0, 0, 0, 0, 10);

-- Importing creature with guid 14517 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1014442 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1014442 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1014442 AND spawnType = 0 WHERE ce.spawnID = 1014442;
INSERT INTO creature_entry (spawnID, entry) VALUES (14517, 4063);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14517, 0, 1, 373, 1, -799.488, -3187.84,  81.9862, 4.96308, 400, 0, 1, 156, 3075, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(145140, 1, -832.334, -3205.47, 79.6893, 0, 0, 0, NULL, 100, 0),
(145140, 2, -860.918, -3218.26, 84.3665, 0, 0, 0, NULL, 100, 0),
(145140, 3, -888.679, -3243.16, 84.0953, 0, 0, 0, NULL, 100, 0),
(145140, 4, -914.439, -3273.13, 79.9203, 0, 0, 0, NULL, 100, 0),
(145140, 5, -934.979, -3286.74, 75.7897, 0, 0, 0, NULL, 100, 0),
(145140, 6, -969.229, -3298.05, 70.3963, 0, 0, 0, NULL, 100, 0),
(145140, 7, -999.526, -3300.1, 59.4386, 0, 0, 0, NULL, 100, 0),
(145140, 8, -1024.02, -3302.78, 52.5727, 0, 0, 0, NULL, 100, 0),
(145140, 9, -1042.51, -3304.22, 53.9611, 0, 0, 0, NULL, 100, 0),
(145140, 10, -1071.52, -3303.01, 53.7243, 0, 0, 0, NULL, 100, 0),
(145140, 11, -1094.37, -3294.41, 51.2826, 0, 0, 0, NULL, 100, 0),
(145140, 12, -1116.25, -3283.38, 48.4626, 0, 0, 0, NULL, 100, 0),
(145140, 13, -1134.6, -3275.85, 45.9398, 0, 0, 0, NULL, 100, 0),
(145140, 14, -1138.17, -3253.6, 37.3472, 0, 0, 0, NULL, 100, 0),
(145140, 15, -1149.55, -3219.16, 32.9213, 0, 0, 0, NULL, 100, 0),
(145140, 16, -1154.65, -3191.67, 39.828, 0, 0, 0, NULL, 100, 0),
(145140, 17, -1152.23, -3172.71, 44.9208, 0, 0, 0, NULL, 100, 0),
(145140, 18, -1155.26, -3148.51, 43.8782, 0, 0, 0, NULL, 100, 0),
(145140, 19, -1156.29, -3118.22, 46.0751, 0, 0, 0, NULL, 100, 0),
(145140, 20, -1158.26, -3089.04, 42.3297, 0, 0, 0, NULL, 100, 0),
(145140, 21, -1166.92, -3051.64, 39.4443, 0, 0, 0, NULL, 100, 0),
(145140, 22, -1173.63, -3018.5, 41.9557, 0, 0, 0, NULL, 100, 0),
(145140, 23, -1177.64, -2989.66, 47.3676, 0, 0, 0, NULL, 100, 0),
(145140, 24, -1184.16, -2956.07, 51.063, 0, 0, 0, NULL, 100, 0),
(145140, 25, -1199.91, -2932.84, 47.8162, 0, 0, 0, NULL, 100, 0),
(145140, 26, -1214.66, -2909.04, 42.4859, 0, 0, 0, NULL, 100, 0),
(145140, 27, -1227.1, -2890.79, 43.511, 0, 0, 0, NULL, 100, 0),
(145140, 28, -1238.98, -2873.56, 44.5421, 0, 0, 0, NULL, 100, 0),
(145140, 29, -1250.41, -2857.38, 45.4169, 0, 0, 0, NULL, 100, 0),
(145140, 30, -1247.51, -2836.02, 46.4793, 0, 0, 0, NULL, 100, 0),
(145140, 31, -1248.64, -2794.01, 51.1922, 0, 0, 0, NULL, 100, 0),
(145140, 32, -1255.58, -2758.49, 49.4708, 0, 0, 0, NULL, 100, 0),
(145140, 33, -1267.21, -2729.37, 48.3409, 0, 0, 0, NULL, 100, 0),
(145140, 34, -1284.02, -2714.97, 48.514, 0, 0, 0, NULL, 100, 0),
(145140, 35, -1301.58, -2701.52, 54.3053, 0, 0, 0, NULL, 100, 0),
(145140, 36, -1318.13, -2685.28, 56.5726, 0, 0, 0, NULL, 100, 0),
(145140, 37, -1337.34, -2665.01, 66.1773, 0, 0, 0, NULL, 100, 0),
(145140, 38, -1351.96, -2651.64, 73.0326, 0, 0, 0, NULL, 100, 0),
(145140, 39, -1368.98, -2639.34, 71.8841, 0, 0, 0, NULL, 100, 0),
(145140, 40, -1386.25, -2625.41, 70.9177, 0, 0, 0, NULL, 100, 0),
(145140, 41, -1404.07, -2608.62, 71.75, 0, 0, 0, NULL, 100, 0),
(145140, 42, -1414.76, -2591.95, 70.5762, 0, 0, 0, NULL, 100, 0),
(145140, 43, -1421.56, -2568.58, 71.2049, 0, 0, 0, NULL, 100, 0),
(145140, 44, -1424.29, -2551.3, 72.8098, 0, 0, 0, NULL, 100, 0),
(145140, 45, -1425.38, -2525.75, 72.7697, 0, 0, 0, NULL, 100, 0),
(145140, 46, -1427.62, -2500.16, 66.6168, 0, 0, 0, NULL, 100, 0),
(145140, 47, -1428.91, -2478.01, 60.9777, 0, 0, 0, NULL, 100, 0),
(145140, 48, -1430.68, -2450.07, 58.7054, 0, 0, 0, NULL, 100, 0),
(145140, 49, -1432.06, -2430.23, 60.3132, 0, 0, 0, NULL, 100, 0),
(145140, 50, -1434.05, -2407.01, 61.2698, 0, 0, 0, NULL, 100, 0),
(145140, 51, -1441.38, -2382.46, 62.3195, 0, 0, 0, NULL, 100, 0),
(145140, 52, -1455.99, -2351.99, 61.8926, 0, 0, 0, NULL, 100, 0),
(145140, 53, -1476.1, -2327.67, 59.2805, 0, 0, 0, NULL, 100, 0),
(145140, 54, -1488.91, -2303.79, 51.6574, 0, 0, 0, NULL, 100, 0),
(145140, 55, -1480.21, -2280.16, 49.2976, 0, 0, 0, NULL, 100, 0),
(145140, 56, -1481.89, -2261.89, 42.7769, 0, 0, 0, NULL, 100, 0),
(145140, 57, -1488.77, -2238.35, 29.7217, 0, 0, 0, NULL, 100, 0),
(145140, 58, -1478.72, -2221.38, 24.4657, 0, 0, 0, NULL, 100, 0),
(145140, 59, -1470.29, -2202.23, 21.3799, 0, 0, 0, NULL, 100, 0),
(145140, 60, -1463.89, -2183.48, 19.2583, 0, 0, 0, NULL, 100, 0),
(145140, 61, -1462.31, -2166.16, 19.5127, 0, 0, 0, NULL, 100, 0),
(145140, 62, -1462.64, -2139.28, 17.9699, 0, 0, 0, NULL, 100, 0),
(145140, 63, -1461.74, -2117.14, 18.4184, 0, 0, 0, NULL, 100, 0),
(145140, 64, -1462.72, -2096.16, 20.9981, 0, 0, 0, NULL, 100, 0),
(145140, 65, -1466.7, -2073.19, 21.766, 0, 0, 0, NULL, 100, 0),
(145140, 66, -1469.74, -2051.22, 22.2819, 0, 0, 0, NULL, 100, 0),
(145140, 67, -1470.71, -2034.95, 23.924, 0, 0, 0, NULL, 100, 0),
(145140, 68, -1467.49, -2011.87, 27.7072, 0, 0, 0, NULL, 100, 0),
(145140, 69, -1461.61, -1991.8, 32.1391, 0, 0, 0, NULL, 100, 0),
(145140, 70, -1453.87, -1976.24, 41.1415, 0, 0, 0, NULL, 100, 0),
(145140, 71, -1440.52, -1963.26, 44.3697, 0, 0, 0, NULL, 100, 0),
(145140, 72, -1427.88, -1962.64, 45.0695, 0, 0, 0, NULL, 100, 0),
(145140, 73, -1403.54, -1960.48, 48.9475, 0, 0, 0, NULL, 100, 0),
(145140, 74, -1384.39, -1955.42, 53.9059, 0, 0, 0, NULL, 100, 0),
(145140, 75, -1369.68, -1945.94, 57.9685, 0, 0, 0, NULL, 100, 0),
(145140, 76, -1350.61, -1921.86, 58.3896, 0, 0, 0, NULL, 100, 0),
(145140, 77, -1340.82, -1894.51, 59.4838, 0, 0, 0, NULL, 100, 0),
(145140, 78, -1331.47, -1868.11, 61.5472, 0, 0, 0, NULL, 100, 0),
(145140, 79, -1324.62, -1843.43, 63.162, 0, 0, 0, NULL, 100, 0),
(145140, 80, -1314.4, -1825.21, 64.5831, 0, 0, 0, NULL, 100, 0),
(145140, 81, -1298.01, -1805.44, 66.0018, 0, 0, 0, NULL, 100, 0),
(145140, 82, -1277.38, -1794.45, 66.9827, 0, 0, 0, NULL, 100, 0),
(145140, 83, -1262.52, -1781.46, 66.2952, 0, 0, 0, NULL, 100, 0),
(145140, 84, -1257.61, -1764.68, 61.1709, 0, 0, 0, NULL, 100, 0),
(145140, 85, -1254.76, -1740.42, 57.4263, 0, 0, 0, NULL, 100, 0),
(145140, 86, -1257.2, -1714.84, 55.3413, 0, 0, 0, NULL, 100, 0),
(145140, 87, -1261.22, -1690.68, 58.784, 0, 0, 0, NULL, 100, 0),
(145140, 88, -1270.5, -1675.87, 58.1884, 0, 0, 0, NULL, 100, 0),
(145140, 89, -1286.64, -1660.74, 56.2642, 0, 0, 0, NULL, 100, 0),
(145140, 90, -1300.36, -1645.05, 56.5116, 0, 0, 0, NULL, 100, 0),
(145140, 91, -1308.54, -1627.06, 54.0608, 0, 0, 0, NULL, 100, 0),
(145140, 92, -1311.49, -1607.6, 54.0495, 0, 0, 0, NULL, 100, 0),
(145140, 93, -1304.89, -1587.72, 55.8418, 0, 0, 0, NULL, 100, 0),
(145140, 94, -1294.45, -1572.35, 57.0907, 0, 0, 0, NULL, 100, 0),
(145140, 95, -1276.31, -1559.62, 56.8721, 0, 0, 0, NULL, 100, 0),
(145140, 96, -1258.6, -1553.95, 55.8263, 0, 0, 0, NULL, 100, 0),
(145140, 97, -1247.02, -1555.56, 54.0814, 0, 0, 0, NULL, 100, 0),
(145140, 98, -1223.99, -1559.11, 53.7523, 0, 0, 0, NULL, 100, 0),
(145140, 99, -1204.17, -1566.04, 57.6988, 0, 0, 0, NULL, 100, 0),
(145140, 100, -1181.22, -1574.42, 57.9026, 0, 0, 0, NULL, 100, 0),
(145140, 101, -1171.8, -1584.78, 56.5109, 0, 0, 0, NULL, 100, 0),
(145140, 102, -1153.56, -1592.68, 51.6124, 0, 0, 0, NULL, 100, 0),
(145140, 103, -1135.42, -1600.81, 44.9674, 0, 0, 0, NULL, 100, 0),
(145140, 104, -1118.64, -1609.03, 43.5657, 0, 0, 0, NULL, 100, 0),
(145140, 105, -1099.77, -1620.57, 44.5041, 0, 0, 0, NULL, 100, 0),
(145140, 106, -1085.97, -1629.27, 42.5487, 0, 0, 0, NULL, 100, 0),
(145140, 107, -1062.28, -1644.19, 38.6492, 0, 0, 0, NULL, 100, 0),
(145140, 108, -1042.5, -1656.66, 34.3277, 0, 0, 0, NULL, 100, 0),
(145140, 109, -1023.84, -1668.41, 35.5141, 0, 0, 0, NULL, 100, 0),
(145140, 110, -1010.46, -1679.57, 37.1343, 0, 0, 0, NULL, 100, 0),
(145140, 111, -994.238, -1696.31, 37.0359, 0, 0, 0, NULL, 100, 0),
(145140, 112, -981.29, -1709.69, 39.2546, 0, 0, 0, NULL, 100, 0),
(145140, 113, -965.091, -1726.44, 45.733, 0, 0, 0, NULL, 100, 0),
(145140, 114, -948.947, -1743.25, 50.5271, 0, 0, 0, NULL, 100, 0),
(145140, 115, -942.321, -1755.45, 50.7807, 0, 0, 0, NULL, 100, 0),
(145140, 116, -931.257, -1775.99, 49.8805, 0, 0, 0, NULL, 100, 0),
(145140, 117, -923.319, -1791.59, 48.1244, 0, 0, 0, NULL, 100, 0),
(145140, 118, -914.847, -1808.24, 49.0443, 0, 0, 0, NULL, 100, 0),
(145140, 119, -916.232, -1825.69, 52.9843, 0, 0, 0, NULL, 100, 0),
(145140, 120, -917.954, -1844.26, 60.7503, 0, 0, 0, NULL, 100, 0),
(145140, 121, -926.844, -1853.56, 64.754, 0, 0, 0, NULL, 100, 0),
(145140, 122, -939.206, -1860.13, 66.1327, 0, 0, 0, NULL, 100, 0),
(145140, 123, -953.152, -1868.59, 66.2573, 0, 0, 0, NULL, 100, 0),
(145140, 124, -958.63, -1878.91, 66.3809, 0, 0, 0, NULL, 100, 0),
(145140, 125, -970.897, -1898.71, 65.0833, 0, 0, 0, NULL, 100, 0),
(145140, 126, -987.406, -1916.73, 65.4069, 0, 0, 0, NULL, 100, 0),
(145140, 127, -1010.65, -1934.49, 63.3908, 0, 0, 0, NULL, 100, 0),
(145140, 128, -1031.82, -1949.48, 62.4668, 0, 0, 0, NULL, 100, 0),
(145140, 129, -1035.49, -1975.89, 62.9083, 0, 0, 0, NULL, 100, 0),
(145140, 130, -1036.42, -1995.75, 64.7552, 0, 0, 0, NULL, 100, 0),
(145140, 131, -1038.65, -2015.43, 67.0361, 0, 0, 0, NULL, 100, 0),
(145140, 132, -1040.25, -2036.3, 66.3034, 0, 0, 0, NULL, 100, 0),
(145140, 133, -1043.27, -2057.06, 61.4778, 0, 0, 0, NULL, 100, 0),
(145140, 134, -1044.61, -2074.51, 55.0994, 0, 0, 0, NULL, 100, 0),
(145140, 135, -1044.43, -2088.51, 52.0924, 0, 0, 0, NULL, 100, 0),
(145140, 136, -1044.65, -2108.3, 53.0293, 0, 0, 0, NULL, 100, 0),
(145140, 137, -1045.71, -2125.84, 57.3443, 0, 0, 0, NULL, 100, 0),
(145140, 138, -1046.98, -2143.3, 60.2948, 0, 0, 0, NULL, 100, 0),
(145140, 139, -1053.53, -2160.68, 61.5841, 0, 0, 0, NULL, 100, 0),
(145140, 140, -1062.26, -2175.84, 62.2059, 0, 0, 0, NULL, 100, 0),
(145140, 141, -1076.35, -2186.12, 61.73, 0, 0, 0, NULL, 100, 0),
(145140, 142, -1094.85, -2193.41, 60.9164, 0, 0, 0, NULL, 100, 0),
(145140, 143, -1106.99, -2197.72, 60.4278, 0, 0, 0, NULL, 100, 0),
(145140, 144, -1120.03, -2202.6, 60.3309, 0, 0, 0, NULL, 100, 0),
(145140, 145, -1134.11, -2208.31, 60.0477, 0, 0, 0, NULL, 100, 0),
(145140, 146, -1147.8, -2215.04, 59.1772, 0, 0, 0, NULL, 100, 0),
(145140, 147, -1160.36, -2221.22, 56.0416, 0, 0, 0, NULL, 100, 0),
(145140, 148, -1171.9, -2226.95, 52.365, 0, 0, 0, NULL, 100, 0),
(145140, 149, -1189.24, -2236.52, 50.8587, 0, 0, 0, NULL, 100, 0),
(145140, 150, -1207.63, -2239.85, 53.6069, 0, 0, 0, NULL, 100, 0),
(145140, 151, -1229.5, -2243.61, 54.4704, 0, 0, 0, NULL, 100, 0),
(145140, 152, -1244.47, -2246.19, 55.4119, 0, 0, 0, NULL, 100, 0),
(145140, 153, -1260.55, -2248.96, 56.6579, 0, 0, 0, NULL, 100, 0),
(145140, 154, -1288.03, -2258.48, 63.4844, 0, 0, 0, NULL, 100, 0),
(145140, 155, -1312.67, -2265.47, 63.8305, 0, 0, 0, NULL, 100, 0),
(145140, 156, -1329.86, -2267.82, 60.9728, 0, 0, 0, NULL, 100, 0),
(145140, 157, -1340.94, -2282.78, 60.76, 0, 0, 0, NULL, 100, 0),
(145140, 158, -1346.83, -2306.51, 63.2913, 0, 0, 0, NULL, 100, 0),
(145140, 159, -1356.51, -2330.38, 62.0932, 0, 0, 0, NULL, 100, 0),
(145140, 160, -1364.83, -2350.88, 63.2731, 0, 0, 0, NULL, 100, 0),
(145140, 161, -1376.38, -2370.98, 64.3251, 0, 0, 0, NULL, 100, 0),
(145140, 162, -1390, -2386.96, 64.4464, 0, 0, 0, NULL, 100, 0),
(145140, 163, -1407.31, -2405.95, 61.7889, 0, 0, 0, NULL, 100, 0),
(145140, 164, -1420.56, -2420.49, 60.0932, 0, 0, 0, NULL, 100, 0),
(145140, 165, -1425.02, -2436.25, 60.0972, 0, 0, 0, NULL, 100, 0),
(145140, 166, -1429.39, -2451.96, 58.95, 0, 0, 0, NULL, 100, 0),
(145140, 167, -1434.99, -2478.18, 61.3683, 0, 0, 0, NULL, 100, 0),
(145140, 168, -1440.61, -2499.64, 65.6851, 0, 0, 0, NULL, 100, 0),
(145140, 169, -1444.77, -2517.79, 67.8829, 0, 0, 0, NULL, 100, 0),
(145140, 170, -1445.7, -2534.07, 69.1921, 0, 0, 0, NULL, 100, 0),
(145140, 171, -1441.83, -2548.73, 71.5545, 0, 0, 0, NULL, 100, 0),
(145140, 172, -1434.36, -2565.78, 69.5201, 0, 0, 0, NULL, 100, 0),
(145140, 173, -1424.97, -2590.84, 66.9125, 0, 0, 0, NULL, 100, 0),
(145140, 174, -1425.67, -2614.12, 63.2577, 0, 0, 0, NULL, 100, 0),
(145140, 175, -1431.74, -2624.11, 59.1063, 0, 0, 0, NULL, 100, 0),
(145140, 176, -1442.61, -2636.35, 53.5248, 0, 0, 0, NULL, 100, 0),
(145140, 177, -1455.66, -2648.02, 49.1358, 0, 0, 0, NULL, 100, 0),
(145140, 178, -1466.47, -2658.47, 45.5302, 0, 0, 0, NULL, 100, 0),
(145140, 179, -1465.66, -2675.95, 41.9098, 0, 0, 0, NULL, 100, 0),
(145140, 180, -1460.71, -2696.35, 37.1821, 0, 0, 0, NULL, 100, 0),
(145140, 181, -1452.89, -2710.66, 35.1992, 0, 0, 0, NULL, 100, 0),
(145140, 182, -1439.53, -2733.91, 36.7902, 0, 0, 0, NULL, 100, 0),
(145140, 183, -1426.57, -2754.7, 36.5802, 0, 0, 0, NULL, 100, 0),
(145140, 184, -1414.25, -2774.49, 40.4362, 0, 0, 0, NULL, 100, 0),
(145140, 185, -1409.16, -2793.71, 40.1381, 0, 0, 0, NULL, 100, 0),
(145140, 186, -1403.95, -2814.05, 38.1709, 0, 0, 0, NULL, 100, 0),
(145140, 187, -1400.76, -2834.88, 37.2834, 0, 0, 0, NULL, 100, 0),
(145140, 188, -1392.98, -2846.44, 36.5229, 0, 0, 0, NULL, 100, 0),
(145140, 189, -1382.8, -2860.67, 34.9138, 0, 0, 0, NULL, 100, 0),
(145140, 190, -1374.26, -2877.22, 32.6507, 0, 0, 0, NULL, 100, 0),
(145140, 191, -1366.55, -2891.67, 32.4265, 0, 0, 0, NULL, 100, 0),
(145140, 192, -1354.48, -2905.83, 39.1326, 0, 0, 0, NULL, 100, 0),
(145140, 193, -1337.49, -2913.61, 41.9231, 0, 0, 0, NULL, 100, 0),
(145140, 194, -1322.41, -2942.04, 38.8526, 0, 0, 0, NULL, 100, 0),
(145140, 195, -1311.79, -2949.18, 37.9754, 0, 0, 0, NULL, 100, 0),
(145140, 196, -1293.35, -2959.24, 36.0428, 0, 0, 0, NULL, 100, 0),
(145140, 197, -1286.24, -2968.5, 35.3916, 0, 0, 0, NULL, 100, 0),
(145140, 198, -1278.27, -2985.29, 35.5228, 0, 0, 0, NULL, 100, 0),
(145140, 199, -1278.45, -3001.67, 36.3541, 0, 0, 0, NULL, 100, 0),
(145140, 200, -1276.65, -3021.39, 36.0018, 0, 0, 0, NULL, 100, 0),
(145140, 201, -1277.91, -3042.34, 35.9597, 0, 0, 0, NULL, 100, 0),
(145140, 202, -1283.24, -3057.74, 35.2043, 0, 0, 0, NULL, 100, 0),
(145140, 203, -1285.87, -3073.83, 34.6964, 0, 0, 0, NULL, 100, 0),
(145140, 204, -1285.93, -3087.83, 34.1618, 0, 0, 0, NULL, 100, 0),
(145140, 205, -1287.67, -3106.38, 33.6795, 0, 0, 0, NULL, 100, 0),
(145140, 206, -1291.36, -3123.48, 34.1784, 0, 0, 0, NULL, 100, 0),
(145140, 207, -1294.93, -3139.4, 35.2509, 0, 0, 0, NULL, 100, 0),
(145140, 208, -1299.37, -3164.64, 35.1117, 0, 0, 0, NULL, 100, 0),
(145140, 209, -1290.96, -3179.93, 35.7953, 0, 0, 0, NULL, 100, 0),
(145140, 210, -1278.23, -3195.11, 35.0469, 0, 0, 0, NULL, 100, 0),
(145140, 211, -1265.49, -3211.8, 34.8543, 0, 0, 0, NULL, 100, 0),
(145140, 212, -1254.87, -3225.8, 34.6723, 0, 0, 0, NULL, 100, 0),
(145140, 213, -1242.02, -3239.18, 34.4204, 0, 0, 0, NULL, 100, 0),
(145140, 214, -1226.99, -3250.29, 32.2023, 0, 0, 0, NULL, 100, 0),
(145140, 215, -1209.01, -3258.55, 29.5988, 0, 0, 0, NULL, 100, 0),
(145140, 216, -1190.41, -3265.4, 32.8749, 0, 0, 0, NULL, 100, 0),
(145140, 217, -1166.79, -3271.9, 39.4746, 0, 0, 0, NULL, 100, 0),
(145140, 218, -1142.09, -3278.94, 45.5479, 0, 0, 0, NULL, 100, 0),
(145140, 219, -1119.98, -3286.24, 48.9263, 0, 0, 0, NULL, 100, 0),
(145140, 220, -1095.75, -3294.77, 51.3525, 0, 0, 0, NULL, 100, 0),
(145140, 221, -1074.82, -3302.14, 53.405, 0, 0, 0, NULL, 100, 0),
(145140, 222, -1055.15, -3305.01, 54.2724, 0, 0, 0, NULL, 100, 0),
(145140, 223, -1035.34, -3304.85, 53.2756, 0, 0, 0, NULL, 100, 0),
(145140, 224, -1014.5, -3304.04, 54.2019, 0, 0, 0, NULL, 100, 0),
(145140, 225, -990.851, -3298.04, 63.2881, 0, 0, 0, NULL, 100, 0),
(145140, 226, -970.008, -3295.53, 70.0985, 0, 0, 0, NULL, 100, 0),
(145140, 227, -944.635, -3292, 74.3861, 0, 0, 0, NULL, 100, 0),
(145140, 228, -932.371, -3285.32, 76.2887, 0, 0, 0, NULL, 100, 0),
(145140, 229, -913.366, -3271.83, 80.1374, 0, 0, 0, NULL, 100, 0),
(145140, 230, -897.063, -3255.07, 83.1267, 0, 0, 0, NULL, 100, 0),
(145140, 231, -881.291, -3241.24, 84.5982, 0, 0, 0, NULL, 100, 0),
(145140, 232, -867.33, -3227.2, 84.7689, 0, 0, 0, NULL, 100, 0),
(145140, 233, -850.828, -3216.25, 83.228, 0, 0, 0, NULL, 100, 0),
(145140, 234, -833.957, -3205.94, 79.7054, 0, 0, 0, NULL, 100, 0),
(145140, 235, -816.334, -3196.89, 81.5434, 3.6, 60000, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (14517, 145140, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 14516 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1014441 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1014441 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1014441 AND spawnType = 0 WHERE ce.spawnID = 1014441;
INSERT INTO creature_entry (spawnID, entry) VALUES (14516, 2765);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14516, 0, 1, 373, 1, -799.488, -3187.84,  81.9862, 4.98765, 400, 0, 1, 219, 0, 2, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (14516, 145140, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 14515 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1014440 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1014440 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1014440 AND spawnType = 0 WHERE ce.spawnID = 1014440;
INSERT INTO creature_entry (spawnID, entry) VALUES (14515, 2764);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14515, 0, 1, 2299, 1, -799.488, -3187.84,  81.9862, 4.97949, 400, 0, 1, 184, 1283, 2, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (14515, 145140, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 14514 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1014439 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1014439 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1014439 AND spawnType = 0 WHERE ce.spawnID = 1014439;
INSERT INTO creature_entry (spawnID, entry) VALUES (14514, 2611);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14514, 0, 1, 1918, 0, -799.488, -3187.84,  81.9862, 4.96305, 400, 0, 1, 3820, 0, 2, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (14514, 145140, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 14514);

-- Importing creature with guid 12013 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 12013 WHERE entry = 2607;
UPDATE conditions SET ConditionValue3 = 12013 WHERE ConditionValue3 = 1011969 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -12013 WHERE SourceEntry = -1011969 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -12013 WHERE entryorguid = -1011969 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 12013 WHERE target_param1 = 1011969 AND target_type = 10;
UPDATE spawn_group SET spawnID = 12013 WHERE spawnID = 1011969 AND spawnType = 0;

-- Importing creature with guid 1846 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (1846, 27704);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (1846, 1, 1, 0, 1, -3738.56, -4442.27,  56.2398, 0.41887, 300, 0, 0, 1, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(18460, 1, -3740.01, -4437.76, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 2, -3742.82, -4433.89, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 3, -3746.76, -4431.05, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 4, -3751.26, -4429.58, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 5, -3756.03, -4429.63, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 6, -3760.62, -4431.11, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 7, -3764.42, -4433.89, 56.2261, 0, 0, 0, NULL, 100, 0),
(18460, 8, -3767.25, -4437.72, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 9, -3768.72, -4442.31, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 10, -3768.66, -4447.08, 56.2247, 0, 0, 0, NULL, 100, 0),
(18460, 11, -3767.22, -4451.63, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 12, -3764.42, -4455.48, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 13, -3767.22, -4451.63, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 14, -3768.66, -4447.08, 56.2247, 0, 0, 0, NULL, 100, 0),
(18460, 15, -3768.72, -4442.31, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 16, -3767.25, -4437.72, 56.2018, 0, 0, 0, NULL, 100, 0),
(18460, 17, -3764.42, -4433.89, 56.2261, 0, 0, 0, NULL, 100, 0),
(18460, 18, -3760.62, -4431.11, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 19, -3756.03, -4429.63, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 20, -3751.26, -4429.58, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 21, -3746.76, -4431.05, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 22, -3742.82, -4433.89, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 23, -3740.01, -4437.76, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 24, -3738.56, -4442.27, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 25, -3738.58, -4447.06, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 26, -3740.03, -4451.58, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 27, -3742.87, -4455.45, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 28, -3746.7, -4458.25, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 29, -3751.22, -4459.75, 56.2626, 0, 0, 0, NULL, 100, 0),
(18460, 30, -3756.02, -4459.73, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 31, -3751.22, -4459.75, 56.2626, 0, 0, 0, NULL, 100, 0),
(18460, 32, -3746.7, -4458.25, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 33, -3742.87, -4455.45, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 34, -3740.03, -4451.58, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 35, -3738.58, -4447.06, 56.2398, 0, 0, 0, NULL, 100, 0),
(18460, 36, -3738.56, -4442.27, 56.2398, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (1846, 18460, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 7450 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1007418 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1007418 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1007418 AND spawnType = 0 WHERE ce.spawnID = 1007418;
INSERT INTO creature_entry (spawnID, entry) VALUES (7450, 1212);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (7450, 0, 1, 0, 0, -8529, 855.316,  106.518, 3.886, 540, 0, 0, 1753, 0, 0, 0, 0, 10);

-- Importing creature with guid 17040 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1016924 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1016924 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1016924 AND spawnType = 0 WHERE ce.spawnID = 1016924;
INSERT INTO creature_entry (spawnID, entry) VALUES (17040, 2417);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (17040, 0, 1, 11563, 1, 483.455, -211.943,  145.366, 1.44298, 600, 0, 0, 3540, 1236, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(170400, 1, 483.455, -211.943, 145.366, 0, 30000, 0, NULL, 100, 0),
(170400, 2, 484.363, -204.875, 145.366, 0, 0, 0, NULL, 100, 0),
(170400, 3, 489.105, -202.629, 145.729, 0, 0, 0, NULL, 100, 0),
(170400, 4, 490.724, -205.655, 146.934, 0, 0, 0, NULL, 100, 0),
(170400, 5, 492.269, -208.544, 148.86, 0, 0, 0, NULL, 100, 0),
(170400, 6, 494.028, -211.834, 151.054, 0, 0, 0, NULL, 100, 0),
(170400, 7, 495.24, -214.1, 152.522, 0, 0, 0, NULL, 100, 0),
(170400, 8, 497.445, -218.223, 152.522, 0, 0, 0, NULL, 100, 0),
(170400, 9, 487.539, -223.48, 152.522, 0, 0, 0, NULL, 100, 0),
(170400, 10, 472.323, -231.556, 152.522, 0, 0, 0, NULL, 100, 0),
(170400, 11, 473.665, -237.36, 152.522, 0, 0, 0, NULL, 100, 0),
(170400, 12, 476.727, -243.128, 152.522, 0, 0, 0, NULL, 100, 0),
(170400, 13, 487.11, -237.534, 152.522, 0, 0, 0, NULL, 100, 0),
(170400, 14, 494.296, -233.662, 151.023, 0, 0, 0, NULL, 100, 0),
(170400, 15, 499.73, -240.012, 151.006, 0, 0, 0, NULL, 100, 0),
(170400, 16, 511.527, -260.497, 151.372, 0, 0, 0, NULL, 100, 0),
(170400, 17, 521.315, -277.494, 150.669, 0, 0, 0, NULL, 100, 0),
(170400, 18, 525.75, -293.445, 151.252, 0, 0, 0, NULL, 100, 0),
(170400, 19, 538.297, -303.477, 152.147, 0, 0, 0, NULL, 100, 0),
(170400, 20, 543.011, -314.067, 151.662, 0, 0, 0, NULL, 100, 0),
(170400, 21, 555.934, -319.452, 150.67, 0, 0, 0, NULL, 100, 0),
(170400, 22, 575.609, -327.652, 150.67, 0, 0, 0, NULL, 100, 0),
(170400, 23, 588.855, -336.454, 150.735, 0, 0, 0, NULL, 100, 0),
(170400, 24, 608.37, -340.924, 151.677, 0, 0, 0, NULL, 100, 0),
(170400, 25, 620.733, -345.274, 151.892, 0, 0, 0, NULL, 100, 0),
(170400, 26, 628.55, -352.284, 151.323, 0, 0, 0, NULL, 100, 0),
(170400, 27, 630.055, -358.358, 151.91, 0, 0, 0, NULL, 100, 0),
(170400, 28, 636.973, -359.514, 152.407, 0, 10000, 0, NULL, 100, 0),
(170400, 29, 634.513, -350.666, 150.912, 0, 0, 0, NULL, 100, 0),
(170400, 30, 618.574, -345.559, 152.033, 0, 0, 0, NULL, 100, 0),
(170400, 31, 600.921, -338.654, 150.949, 0, 0, 0, NULL, 100, 0),
(170400, 32, 583.413, -331.806, 150.692, 0, 0, 0, NULL, 100, 0),
(170400, 33, 567.701, -322.93, 150.67, 0, 0, 0, NULL, 100, 0),
(170400, 34, 557.249, -317.025, 150.67, 0, 0, 0, NULL, 100, 0),
(170400, 35, 544.084, -311.732, 151.633, 0, 0, 0, NULL, 100, 0),
(170400, 36, 539.037, -306.881, 152.326, 0, 0, 0, NULL, 100, 0),
(170400, 37, 530.512, -296.477, 151.526, 0, 0, 0, NULL, 100, 0),
(170400, 38, 521.22, -285.05, 151.121, 0, 0, 0, NULL, 100, 0),
(170400, 39, 516.919, -272.902, 150.948, 0, 0, 0, NULL, 100, 0),
(170400, 40, 513.425, -263.035, 151.491, 0, 0, 0, NULL, 100, 0),
(170400, 41, 507.308, -250.38, 151.283, 0, 0, 0, NULL, 100, 0),
(170400, 42, 497.071, -232.076, 151.022, 0, 0, 0, NULL, 100, 0),
(170400, 43, 502.656, -229.569, 151.648, 0, 0, 0, NULL, 100, 0),
(170400, 44, 507.337, -227.467, 152.522, 0, 0, 0, NULL, 100, 0),
(170400, 45, 516.603, -223.307, 152.522, 0, 0, 0, NULL, 100, 0),
(170400, 46, 512.04, -211.965, 152.522, 0, 0, 0, NULL, 100, 0),
(170400, 47, 500.043, -216.841, 152.522, 0, 0, 0, NULL, 100, 0),
(170400, 48, 497.124, -218.512, 152.522, 0, 0, 0, NULL, 100, 0),
(170400, 49, 494.878, -214.149, 152.491, 0, 0, 0, NULL, 100, 0),
(170400, 50, 492.796, -210.103, 149.816, 0, 0, 0, NULL, 100, 0),
(170400, 51, 490.925, -206.469, 147.412, 0, 0, 0, NULL, 100, 0),
(170400, 52, 488.842, -202.423, 145.729, 0, 0, 0, NULL, 100, 0),
(170400, 53, 484.401, -204.745, 145.367, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (17040, 170400, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 17040);

-- Importing creature with guid 14776 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (14776, 2721);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14776, 0, 1, 4037, 1, -1550.34, -3028.26,  13.6448, 5.00963, 60, 0, 0, 610, 0, 0, 0, 0, 10);

-- Importing creature with guid 10500 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1010466 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1010466 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1010466 AND spawnType = 0 WHERE ce.spawnID = 1010466;
INSERT INTO creature_entry (spawnID, entry) VALUES (10500, 1752);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (10500, 0, 1, 0, 1, -8534, 367.126,  108.569, 2.286, 490, 0, 0, 4120, 0, 0, 0, 0, 10);

-- Importing creature with guid 16047 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1015935 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1015935 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1015935 AND spawnType = 0 WHERE ce.spawnID = 1015935;
INSERT INTO creature_entry (spawnID, entry) VALUES (16047, 14277);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (16047, 0, 1, 4978, 1, -1276.08, -976,  -0.561754, 0.553209, 43200, 0, 0, 918, 2457, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(160470, 1, -1276.08, -976, -0.561754, 0, 0, 0, NULL, 100, 0),
(160470, 2, -1258.03, -950.293, 1.1057, 0, 0, 0, NULL, 100, 0),
(160470, 3, -1245.88, -925.964, 0.299947, 0, 0, 0, NULL, 100, 0),
(160470, 4, -1241.09, -902.986, 0.176827, 0, 0, 0, NULL, 100, 0),
(160470, 5, -1216.54, -879.045, -0.012203, 0, 0, 0, NULL, 100, 0),
(160470, 6, -1204.1, -862.604, -0.022923, 0, 0, 0, NULL, 100, 0),
(160470, 7, -1177.1, -836.439, 0.405907, 0, 0, 0, NULL, 100, 0),
(160470, 8, -1155.64, -807.945, -0.769681, 0, 0, 0, NULL, 100, 0),
(160470, 9, -1134.61, -785.771, -0.684184, 0, 0, 0, NULL, 100, 0),
(160470, 10, -1111.14, -792.039, -0.047394, 0, 0, 0, NULL, 100, 0),
(160470, 11, -1120.56, -803.641, 2.45509, 0, 0, 0, NULL, 100, 0),
(160470, 12, -1149.99, -814.86, 1.4159, 0, 0, 0, NULL, 100, 0),
(160470, 13, -1170.68, -831.741, 0.905636, 0, 0, 0, NULL, 100, 0),
(160470, 14, -1186.74, -842.143, -0.424433, 0, 0, 0, NULL, 100, 0),
(160470, 15, -1205.23, -862.842, -0.181762, 0, 0, 0, NULL, 100, 0),
(160470, 16, -1221.71, -885.192, -0.091555, 0, 0, 0, NULL, 100, 0),
(160470, 17, -1244.15, -916.463, 0.404305, 0, 0, 0, NULL, 100, 0),
(160470, 18, -1261.29, -949.49, 0.631858, 0, 0, 0, NULL, 100, 0),
(160470, 19, -1277.74, -975.288, -0.7961, 0, 0, 0, NULL, 100, 0),
(160470, 20, -1290.77, -995.08, -0.895832, 0, 0, 0, NULL, 100, 0),
(160470, 21, -1313.06, -1004.87, -1.52388, 0, 0, 0, NULL, 100, 0),
(160470, 22, -1344.6, -1026.51, 0.392506, 0, 0, 0, NULL, 100, 0),
(160470, 23, -1366.78, -1036.92, 2.69835, 0, 0, 0, NULL, 100, 0),
(160470, 24, -1392.12, -1048.81, 3.73084, 0, 0, 0, NULL, 100, 0),
(160470, 25, -1414.89, -1059.5, 4.62124, 0, 0, 0, NULL, 100, 0),
(160470, 26, -1433.06, -1083.33, 8.08795, 0, 0, 0, NULL, 100, 0),
(160470, 27, -1441.77, -1091.69, 9.71514, 0, 0, 0, NULL, 100, 0),
(160470, 28, -1457.46, -1095.82, 3.53264, 0, 0, 0, NULL, 100, 0),
(160470, 29, -1468.13, -1097.07, -1.56115, 0, 0, 0, NULL, 100, 0),
(160470, 30, -1472.68, -1080.05, -1.56115, 0, 0, 0, NULL, 100, 0),
(160470, 31, -1458.45, -1062.96, -1.56115, 0, 0, 0, NULL, 100, 0),
(160470, 32, -1451.28, -1056.44, -1.56115, 0, 0, 0, NULL, 100, 0),
(160470, 33, -1432.58, -1051.24, -0.703612, 0, 0, 0, NULL, 100, 0),
(160470, 34, -1409.8, -1040.14, -0.755762, 0, 0, 0, NULL, 100, 0),
(160470, 35, -1385.61, -1038.06, 0.275001, 0, 0, 0, NULL, 100, 0),
(160470, 36, -1357.3, -1024.25, -0.809856, 0, 0, 0, NULL, 100, 0),
(160470, 37, -1326.17, -1014.01, -1.29519, 0, 0, 0, NULL, 100, 0),
(160470, 38, -1303.49, -997.97, -1.23017, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (16047, 160470, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 15970 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1015858 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1015858 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1015858 AND spawnType = 0 WHERE ce.spawnID = 1015858;
INSERT INTO creature_entry (spawnID, entry) VALUES (15970, 14276);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (15970, 0, 1, 540, 0, -1172.77, 123.663,  0.157823, 4.57671, 43200, 0, 0, 1003, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(159700, 1, -1172.77, 123.663, 0.157823, 0, 0, 0, NULL, 100, 0),
(159700, 2, -1172.87, 118.873, 0.322706, 0, 0, 0, NULL, 100, 0),
(159700, 3, -1164.9, 111.492, 3.19687, 0, 0, 0, NULL, 100, 0),
(159700, 4, -1159.58, 90.6803, 1.9422, 0, 0, 0, NULL, 100, 0),
(159700, 5, -1152.25, 56.6911, 0.424014, 0, 0, 0, NULL, 100, 0),
(159700, 6, -1151.73, 33.0373, -0.172392, 0, 0, 0, NULL, 100, 0),
(159700, 7, -1151.24, 14.382, -0.125738, 0, 0, 0, NULL, 100, 0),
(159700, 8, -1144.4, 0.226702, -0.029557, 0, 0, 0, NULL, 100, 0),
(159700, 9, -1133.86, -10.8596, -0.189161, 0, 0, 0, NULL, 100, 0),
(159700, 10, -1104.63, -13.0749, 0.554654, 0, 0, 0, NULL, 100, 0),
(159700, 11, -1081.96, -16.5233, 1.19604, 0, 0, 0, NULL, 100, 0),
(159700, 12, -1073.98, -31.1868, 0.77585, 0, 0, 0, NULL, 100, 0),
(159700, 13, -1089.35, -31.3077, -1.07762, 0, 0, 0, NULL, 100, 0),
(159700, 14, -1115.1, -22.2406, -1.09916, 0, 0, 0, NULL, 100, 0),
(159700, 15, -1138.02, -7.81281, -0.156224, 0, 0, 0, NULL, 100, 0),
(159700, 16, -1150.51, 10.8034, -0.153825, 0, 0, 0, NULL, 100, 0),
(159700, 17, -1154.11, 26.389, -0.769621, 0, 0, 0, NULL, 100, 0),
(159700, 18, -1154.74, 64.912, 0.486252, 0, 0, 0, NULL, 100, 0),
(159700, 19, -1150.34, 95.8842, 3.87744, 0, 0, 0, NULL, 100, 0),
(159700, 20, -1143.77, 109.315, 7.45656, 0, 0, 0, NULL, 100, 0),
(159700, 21, -1146.89, 127.139, 5.36332, 0, 0, 0, NULL, 100, 0),
(159700, 22, -1163.18, 154.262, 0.071613, 0, 0, 0, NULL, 100, 0),
(159700, 23, -1151.92, 184.312, 0.078821, 0, 0, 0, NULL, 100, 0),
(159700, 24, -1138.85, 210.151, 0.64674, 0, 0, 0, NULL, 100, 0),
(159700, 25, -1120.27, 230.815, 2.71415, 0, 0, 0, NULL, 100, 0),
(159700, 26, -1103.25, 257.063, 3.6444, 0, 0, 0, NULL, 100, 0),
(159700, 27, -1088.66, 273.3, 2.29494, 0, 0, 0, NULL, 100, 0),
(159700, 28, -1067.18, 270.992, 2.63842, 0, 0, 0, NULL, 100, 0),
(159700, 29, -1033.01, 278.171, 0.676006, 0, 0, 0, NULL, 100, 0),
(159700, 30, -1007.7, 280.798, 1.6107, 0, 0, 0, NULL, 100, 0),
(159700, 31, -989.369, 301.059, 0.846009, 0, 0, 0, NULL, 100, 0),
(159700, 32, -981.966, 323.582, -1.07191, 0, 0, 0, NULL, 100, 0),
(159700, 33, -975.038, 334.123, -1.3281, 0, 0, 0, NULL, 100, 0),
(159700, 34, -963.251, 334.046, 2.5684, 0, 0, 0, NULL, 100, 0),
(159700, 35, -960.338, 322.978, 6.63507, 0, 0, 0, NULL, 100, 0),
(159700, 36, -967.984, 309.869, 5.90922, 0, 0, 0, NULL, 100, 0),
(159700, 37, -992.889, 300.466, 0.063592, 0, 0, 0, NULL, 100, 0),
(159700, 38, -1009.43, 291.063, -1.3482, 0, 0, 0, NULL, 100, 0),
(159700, 39, -1032.87, 284.953, -1.52593, 0, 0, 0, NULL, 100, 0),
(159700, 40, -1049.37, 280.662, -0.629822, 0, 0, 0, NULL, 100, 0),
(159700, 41, -1078.16, 282.367, -0.724659, 0, 0, 0, NULL, 100, 0),
(159700, 42, -1095.26, 287.094, -1.10337, 0, 0, 0, NULL, 100, 0),
(159700, 43, -1105.95, 283.475, -0.136028, 0, 0, 0, NULL, 100, 0),
(159700, 44, -1117.02, 277.427, -0.169369, 0, 0, 0, NULL, 100, 0),
(159700, 45, -1121.42, 258.795, 0.527346, 0, 0, 0, NULL, 100, 0),
(159700, 46, -1130.18, 237.408, -0.735995, 0, 0, 0, NULL, 100, 0),
(159700, 47, -1142.05, 208.455, -0.004859, 0, 0, 0, NULL, 100, 0),
(159700, 48, -1155.78, 174.963, -0.17143, 0, 0, 0, NULL, 100, 0),
(159700, 49, -1163.85, 152.053, 0.127761, 0, 0, 0, NULL, 100, 0),
(159700, 50, -1169.17, 136.967, 0.156267, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (15970, 159700, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 9525 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1009492 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1009492 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1009492 AND spawnType = 0 WHERE ce.spawnID = 1009492;
INSERT INTO creature_entry (spawnID, entry) VALUES (9525, 1451);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (9525, 0, 1, 3462, 1, -3776.6, -797.833,  8.69004, 2.882, 300, 0, 0, 713, 0, 2, 0, 0, 10);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (486, 0, 1, 29, 0, 0, 0, 0, 0, 0);
INSERT INTO waypoint_data VALUES 
(95250, 1, -3776.6, -797.833, 8.69004, 0, 29000, 0, NULL, 100, 0),
(95250, 2, -3778.79, -800.852, 8.57007, 0, 0, 0, NULL, 100, 0),
(95250, 3, -3778.43, -807.106, 8.86214, 0, 0, 0, NULL, 100, 0),
(95250, 4, -3775.92, -808.031, 9.76186, 0, 0, 0, NULL, 100, 0),
(95250, 5, -3775.25, -806.261, 9.89661, 0, 12000, 0, NULL, 100, 0),
(95250, 6, -3772.94, -807.634, 10.4204, 0, 0, 0, NULL, 100, 0),
(95250, 7, -3770.64, -805.001, 10.3747, 0, 0, 0, NULL, 100, 0),
(95250, 8, -3768.57, -799.271, 9.746, 0, 0, 0, NULL, 100, 0),
(95250, 9, -3772.09, -796.733, 9.22766, 0, 13000, 0, 486, 100, 0),
(95250, 10, -3770.23, -794.62, 9.36567, 0, 0, 0, NULL, 100, 0),
(95250, 11, -3772.34, -792.51, 8.97663, 0, 0, 0, NULL, 100, 0),
(95250, 12, -3774.92, -791.916, 8.61633, 0, 0, 0, NULL, 100, 0),
(95250, 13, -3776.59, -793.099, 8.51261, 0, 0, 0, NULL, 100, 0),
(95250, 14, -3775.36, -798.295, 8.85547, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (9525, 95250, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 8877 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1008844 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1008844 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1008844 AND spawnType = 0 WHERE ce.spawnID = 1008844;
INSERT INTO creature_entry (spawnID, entry) VALUES (8877, 1225);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (8877, 0, 1, 706, 0, -5716.18, -3110.81,  316.687, 3.06542, 1200, 0, 0, 484, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(88770, 1, -5716.18, -3110.81, 316.687, 0, 0, 0, NULL, 100, 0),
(88770, 2, -5716.19, -3093.08, 325.601, 0, 0, 0, NULL, 100, 0),
(88770, 3, -5712.21, -3090.3, 327.739, 0, 0, 0, NULL, 100, 0),
(88770, 4, -5705.48, -3092.52, 329.362, 0, 0, 0, NULL, 100, 0),
(88770, 5, -5681.83, -3110.57, 338.122, 0, 0, 0, NULL, 100, 0),
(88770, 6, -5659.5, -3122.22, 344.336, 0, 0, 0, NULL, 100, 0),
(88770, 7, -5639.59, -3124.54, 348.405, 0, 0, 0, NULL, 100, 0),
(88770, 8, -5618.11, -3110.91, 360.618, 0, 0, 0, NULL, 100, 0),
(88770, 9, -5621.49, -3096.32, 368.248, 0, 0, 0, NULL, 100, 0),
(88770, 10, -5632.21, -3078.61, 374.991, 0, 0, 0, NULL, 100, 0),
(88770, 11, -5629.79, -3056.12, 384.466, 0, 0, 0, NULL, 100, 0),
(88770, 12, -5642.28, -3036.87, 385.472, 0, 0, 0, NULL, 100, 0),
(88770, 13, -5609.37, -3006.88, 386.288, 0, 0, 0, NULL, 100, 0),
(88770, 14, -5643.63, -3036.39, 385.532, 0, 0, 0, NULL, 100, 0),
(88770, 15, -5630.17, -3057.02, 384.386, 0, 0, 0, NULL, 100, 0),
(88770, 16, -5629.84, -3065.5, 381.13, 0, 0, 0, NULL, 100, 0),
(88770, 17, -5634.87, -3078.45, 374.489, 0, 0, 0, NULL, 100, 0),
(88770, 18, -5620.42, -3101.08, 364.82, 0, 0, 0, NULL, 100, 0),
(88770, 19, -5624.63, -3117.04, 354.494, 0, 0, 0, NULL, 100, 0),
(88770, 20, -5644.95, -3125.08, 347.271, 0, 0, 0, NULL, 100, 0),
(88770, 21, -5660.74, -3121.58, 343.976, 0, 0, 0, NULL, 100, 0),
(88770, 22, -5676.21, -3111.59, 340.021, 0, 0, 0, NULL, 100, 0),
(88770, 23, -5691.9, -3102.99, 333.647, 0, 0, 0, NULL, 100, 0),
(88770, 24, -5711.66, -3088.43, 328.762, 0, 0, 0, NULL, 100, 0),
(88770, 25, -5717.66, -3099.03, 321.687, 0, 0, 0, NULL, 100, 0),
(88770, 26, -5705.21, -3132.32, 315.838, 0, 0, 0, NULL, 100, 0),
(88770, 27, -5679.01, -3185.05, 319.508, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (8877, 88770, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 8877);

-- Importing creature with guid 8855 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1008822 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1008822 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1008822 AND spawnType = 0 WHERE ce.spawnID = 1008822;
DELETE FROM waypoint_data WHERE id = 155;
INSERT INTO creature_entry (spawnID, entry) VALUES (8855, 1282);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (8855, 0, 1, 1809, 1, -5733.6, -2581.92,  335.213, 2.88288, 300, 0, 0, 1910, 0, 2, 0, 0, 10);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1207, 1, 30, 0, 0, 0, 0, 0, 0, 0.331613);
INSERT INTO waypoint_data VALUES 
(88550, 1, -5721.12, -2584.82, 338.329, 0, 0, 0, NULL, 100, 0),
(88550, 2, -5710.5, -2587.33, 340.329, 0, 0, 0, NULL, 100, 0),
(88550, 3, -5704.55, -2586.63, 341.829, 0, 0, 0, NULL, 100, 0),
(88550, 4, -5693.83, -2583.28, 345.206, 0, 0, 0, NULL, 100, 0),
(88550, 5, -5678.78, -2577.44, 351.456, 0, 0, 0, NULL, 100, 0),
(88550, 6, -5673.2, -2573.21, 353.956, 0, 0, 0, NULL, 100, 0),
(88550, 7, -5666.21, -2565.07, 358.562, 0, 0, 0, NULL, 100, 0),
(88550, 8, -5655.03, -2553.07, 364.812, 0, 0, 0, NULL, 100, 0),
(88550, 9, -5646.83, -2544.12, 371.437, 0, 0, 0, NULL, 100, 0),
(88550, 10, -5640.7, -2541.29, 374.187, 0, 150000, 0, 1207, 100, 0),
(88550, 11, -5645.91, -2543.41, 371.812, 0, 0, 0, NULL, 100, 0),
(88550, 12, -5654.59, -2552.66, 364.812, 0, 0, 0, NULL, 100, 0),
(88550, 13, -5660.7, -2559.52, 361.062, 0, 0, 0, NULL, 100, 0),
(88550, 14, -5668.46, -2568.08, 357.081, 0, 0, 0, NULL, 100, 0),
(88550, 15, -5675.02, -2575.66, 352.706, 0, 0, 0, NULL, 100, 0),
(88550, 16, -5682.13, -2579.42, 350.081, 0, 0, 0, NULL, 100, 0),
(88550, 17, -5690.3, -2582.82, 346.581, 0, 0, 0, NULL, 100, 0),
(88550, 18, -5699.89, -2586.38, 343.206, 0, 0, 0, NULL, 100, 0),
(88550, 19, -5710.8, -2587.57, 340.204, 0, 0, 0, NULL, 100, 0),
(88550, 20, -5719.78, -2585.79, 338.704, 0, 0, 0, NULL, 100, 0),
(88550, 21, -5730.87, -2582.85, 335.829, 0, 0, 0, NULL, 100, 0),
(88550, 22, -5741.06, -2582.22, 332.713, 0, 0, 0, NULL, 100, 0),
(88550, 23, -5749.34, -2585, 329.588, 0, 0, 0, NULL, 100, 0),
(88550, 24, -5755.76, -2591.94, 327.088, 0, 0, 0, NULL, 100, 0),
(88550, 25, -5764.8, -2601.28, 324.604, 0, 0, 0, NULL, 100, 0),
(88550, 26, -5777.56, -2611.75, 321.358, 0, 150000, 0, NULL, 100, 0),
(88550, 27, -5759.69, -2596.19, 325.838, 0, 0, 0, NULL, 100, 0),
(88550, 28, -5755.2, -2590.93, 327.338, 0, 0, 0, NULL, 100, 0),
(88550, 29, -5751.05, -2585.88, 328.838, 0, 0, 0, NULL, 100, 0),
(88550, 30, -5746.82, -2584.3, 330.588, 0, 0, 0, NULL, 100, 0),
(88550, 31, -5740, -2581.3, 333.088, 0, 0, 0, NULL, 100, 0),
(88550, 32, -5733.6, -2581.92, 335.213, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (8855, 88550, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 8840 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1008807 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1008807 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1008807 AND spawnType = 0 WHERE ce.spawnID = 1008807;
INSERT INTO creature_entry (spawnID, entry) VALUES (8840, 1280);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (8840, 0, 1, 1781, 1, -5433.62, -2884.02,  345.496, 3.9968, 300, 0, 0, 1910, 0, 2, 0, 0, 10);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1206, 1, 30, 0, 0, 0, 0, 0, 0, 3.9968);
INSERT INTO waypoint_data VALUES 
(88400, 1, -5456.04, -2879.35, 348.621, 0, 0, 0, NULL, 100, 0),
(88400, 2, -5467.57, -2876.02, 350.449, 0, 0, 0, NULL, 100, 0),
(88400, 3, -5476.57, -2874.38, 351.824, 0, 0, 0, NULL, 100, 0),
(88400, 4, -5487.51, -2872.97, 353.699, 0, 0, 0, NULL, 100, 0),
(88400, 5, -5499, -2870.62, 355.574, 0, 0, 0, NULL, 100, 0),
(88400, 6, -5509.7, -2867.91, 357.002, 0, 0, 0, NULL, 100, 0),
(88400, 7, -5520.05, -2865.88, 358.615, 0, 0, 0, NULL, 100, 0),
(88400, 8, -5527.45, -2863.46, 359.74, 0, 0, 0, NULL, 100, 0),
(88400, 9, -5533.81, -2861.29, 360.441, 0, 0, 0, NULL, 100, 0),
(88400, 10, -5541.13, -2854.75, 361.441, 0, 0, 0, NULL, 100, 0),
(88400, 11, -5546.06, -2848.45, 362.191, 0, 0, 0, NULL, 100, 0),
(88400, 12, -5551.6, -2842.09, 362.941, 0, 0, 0, NULL, 100, 0),
(88400, 13, -5560.15, -2832.46, 364.697, 0, 0, 0, NULL, 100, 0),
(88400, 14, -5568.65, -2823.52, 365.497, 0, 0, 0, NULL, 100, 0),
(88400, 15, -5574.22, -2818.61, 366.122, 0, 0, 0, NULL, 100, 0),
(88400, 16, -5580.25, -2814.35, 366.372, 0, 0, 0, NULL, 100, 0),
(88400, 17, -5591.22, -2808.31, 366.997, 0, 150000, 0, NULL, 100, 0),
(88400, 18, -5570.91, -2821.55, 365.622, 0, 0, 0, NULL, 100, 0),
(88400, 19, -5558.25, -2835.02, 364.316, 0, 0, 0, NULL, 100, 0),
(88400, 20, -5545.79, -2850.86, 361.941, 0, 0, 0, NULL, 100, 0),
(88400, 21, -5536.26, -2860.83, 360.691, 0, 0, 0, NULL, 100, 0),
(88400, 22, -5529.58, -2863.82, 359.865, 0, 0, 0, NULL, 100, 0),
(88400, 23, -5499.83, -2870.05, 355.574, 0, 0, 0, NULL, 100, 0),
(88400, 24, -5480.54, -2873.26, 352.574, 0, 0, 0, NULL, 100, 0),
(88400, 25, -5457.91, -2878.8, 348.996, 0, 0, 0, NULL, 100, 0),
(88400, 26, -5433.62, -2884.02, 345.496, 0, 0, 0, NULL, 100, 0),
(88400, 27, -5418.01, -2884.54, 343.776, 0, 0, 0, NULL, 100, 0),
(88400, 28, -5413.86, -2886.71, 343.526, 0, 0, 0, NULL, 100, 0),
(88400, 29, -5412.14, -2889.73, 343.151, 0, 150000, 0, 1206, 100, 0),
(88400, 30, -5413.86, -2886.71, 343.526, 0, 0, 0, NULL, 100, 0),
(88400, 31, -5418.01, -2884.54, 343.776, 0, 0, 0, NULL, 100, 0),
(88400, 32, -5433.62, -2884.02, 345.496, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (8840, 88400, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 8837 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1008804 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1008804 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1008804 AND spawnType = 0 WHERE ce.spawnID = 1008804;
INSERT INTO creature_entry (spawnID, entry) VALUES (8837, 3291);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (8837, 0, 1, 1837, 0, -5355.96, -2941.26,  324.009, 3.82227, 300, 0, 0, 664, 0, 2, 0, 0, 10);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1209, 1, 30, 0, 0, 0, 0, 0, 0, 3.82227);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1210, 1, 30, 0, 0, 0, 0, 0, 0, 3.9968);
INSERT INTO waypoint_data VALUES 
(88370, 1, -5359.74, -2939.84, 325.634, 0, 0, 0, NULL, 100, 0),
(88370, 2, -5366.35, -2936.03, 327.634, 0, 0, 0, NULL, 100, 0),
(88370, 3, -5372.43, -2932.24, 329.656, 0, 0, 0, NULL, 100, 0),
(88370, 4, -5378.13, -2927.26, 331.531, 0, 0, 0, NULL, 100, 0),
(88370, 5, -5386.19, -2918.44, 334.781, 0, 0, 0, NULL, 100, 0),
(88370, 6, -5393.9, -2909.54, 338.031, 0, 0, 0, NULL, 100, 0),
(88370, 7, -5401.52, -2899.87, 340.901, 0, 0, 0, NULL, 100, 0),
(88370, 8, -5408.32, -2889.98, 342.776, 0, 0, 0, NULL, 100, 0),
(88370, 9, -5415.96, -2884.63, 343.651, 0, 0, 0, NULL, 100, 0),
(88370, 10, -5430.75, -2884.07, 345.026, 0, 0, 0, NULL, 100, 0),
(88370, 11, -5441.15, -2881.79, 346.496, 0, 0, 0, NULL, 100, 0),
(88370, 12, -5451.92, -2880.05, 347.996, 0, 0, 0, NULL, 100, 0),
(88370, 13, -5464.37, -2876.68, 349.871, 0, 0, 0, NULL, 100, 0),
(88370, 14, -5475.52, -2874.67, 351.699, 0, 0, 0, NULL, 100, 0),
(88370, 15, -5487.63, -2872.45, 353.699, 0, 0, 0, NULL, 100, 0),
(88370, 16, -5510.59, -2868.1, 357.127, 0, 0, 0, NULL, 100, 0),
(88370, 17, -5525.44, -2864.27, 359.49, 0, 0, 0, NULL, 100, 0),
(88370, 18, -5534.25, -2861, 360.441, 0, 0, 0, NULL, 100, 0),
(88370, 19, -5542.37, -2854.51, 361.566, 0, 0, 0, NULL, 100, 0),
(88370, 20, -5546.35, -2849.53, 362.066, 0, 0, 0, NULL, 100, 0),
(88370, 21, -5554.38, -2839.59, 363.441, 0, 0, 0, NULL, 100, 0),
(88370, 22, -5565.08, -2828.53, 365.322, 0, 0, 0, NULL, 100, 0),
(88370, 23, -5571, -2820.95, 365.622, 0, 0, 0, NULL, 100, 0),
(88370, 24, -5586.27, -2811.42, 366.622, 0, 0, 0, NULL, 100, 0),
(88370, 25, -5594.97, -2806.89, 366.997, 0, 0, 0, NULL, 100, 0),
(88370, 26, -5609.28, -2799.13, 366.195, 0, 0, 0, NULL, 100, 0),
(88370, 27, -5623.16, -2792.06, 365.445, 0, 0, 0, NULL, 100, 0),
(88370, 28, -5633, -2788.02, 365.445, 0, 0, 0, NULL, 100, 0),
(88370, 29, -5644.65, -2783.89, 364.507, 0, 0, 0, NULL, 100, 0),
(88370, 30, -5653.82, -2779.97, 364.132, 0, 0, 0, NULL, 100, 0),
(88370, 31, -5666.23, -2776.11, 363.382, 0, 0, 0, NULL, 100, 0),
(88370, 32, -5679.12, -2771.49, 363.052, 0, 0, 0, NULL, 100, 0),
(88370, 33, -5688.91, -2767.23, 362.427, 0, 0, 0, NULL, 100, 0),
(88370, 34, -5696.22, -2760.99, 361.728, 0, 0, 0, NULL, 100, 0),
(88370, 35, -5700.72, -2752.9, 360.744, 0, 0, 0, NULL, 100, 0),
(88370, 36, -5703.73, -2741.32, 358.369, 0, 0, 0, NULL, 100, 0),
(88370, 37, -5708.06, -2731.15, 355.72, 0, 0, 0, NULL, 100, 0),
(88370, 38, -5719.77, -2711.89, 350.595, 0, 0, 0, NULL, 100, 0),
(88370, 39, -5724.94, -2704.64, 348.47, 0, 0, 0, NULL, 100, 0),
(88370, 40, -5729.34, -2695.43, 345.398, 0, 0, 0, NULL, 100, 0),
(88370, 41, -5734.72, -2684.96, 342.172, 0, 0, 0, NULL, 100, 0),
(88370, 42, -5741.89, -2675.92, 339.297, 0, 0, 0, NULL, 100, 0),
(88370, 43, -5753.89, -2663.79, 335.208, 0, 0, 0, NULL, 100, 0),
(88370, 44, -5765.92, -2652.88, 330.458, 0, 0, 0, NULL, 100, 0),
(88370, 45, -5780.38, -2638.05, 324.76, 0, 0, 0, NULL, 100, 0),
(88370, 46, -5789.38, -2623.71, 320.733, 0, 480000, 0, NULL, 100, 0),
(88370, 47, -5788.48, -2624.68, 320.983, 0, 0, 0, NULL, 100, 0),
(88370, 48, -5782.33, -2636.13, 324.01, 0, 0, 0, NULL, 100, 0),
(88370, 49, -5769.25, -2650.27, 329.51, 0, 0, 0, NULL, 100, 0),
(88370, 50, -5756.28, -2662.46, 334.208, 0, 0, 0, NULL, 100, 0),
(88370, 51, -5744.69, -2672.09, 338.422, 0, 0, 0, NULL, 100, 0),
(88370, 52, -5734.94, -2684.67, 342.172, 0, 0, 0, NULL, 100, 0),
(88370, 53, -5731.8, -2691, 344.023, 0, 0, 0, NULL, 100, 0),
(88370, 54, -5728.58, -2700.88, 346.845, 0, 0, 0, NULL, 100, 0),
(88370, 55, -5715, -2718.66, 352.345, 0, 0, 0, NULL, 100, 0),
(88370, 56, -5708.42, -2732.71, 356.095, 0, 0, 0, NULL, 100, 0),
(88370, 57, -5702.85, -2742.93, 358.994, 0, 0, 0, NULL, 100, 0),
(88370, 58, -5699.33, -2755.45, 361.103, 0, 0, 0, NULL, 100, 0),
(88370, 59, -5694.58, -2765.91, 362.228, 0, 0, 0, NULL, 100, 0),
(88370, 60, -5685.3, -2770.85, 362.802, 0, 0, 0, NULL, 100, 0),
(88370, 61, -5665.48, -2776.23, 363.382, 0, 0, 0, NULL, 100, 0),
(88370, 62, -5631.77, -2788.57, 365.445, 0, 0, 0, NULL, 100, 0),
(88370, 63, -5620.69, -2793.96, 365.445, 0, 0, 0, NULL, 100, 0),
(88370, 64, -5608.81, -2800.25, 366.274, 0, 0, 0, NULL, 100, 0),
(88370, 65, -5588.05, -2809.67, 366.747, 0, 0, 0, NULL, 100, 0),
(88370, 66, -5573.91, -2819.02, 365.997, 0, 0, 0, NULL, 100, 0),
(88370, 67, -5561.19, -2832.71, 364.822, 0, 0, 0, NULL, 100, 0),
(88370, 68, -5547.26, -2848.41, 362.191, 0, 0, 0, NULL, 100, 0),
(88370, 69, -5540.5, -2856.76, 361.316, 0, 0, 0, NULL, 100, 0),
(88370, 70, -5532.64, -2863.15, 360.24, 0, 0, 0, NULL, 100, 0),
(88370, 71, -5512.56, -2868.11, 357.377, 0, 0, 0, NULL, 100, 0),
(88370, 72, -5487.21, -2872.07, 353.574, 0, 0, 0, NULL, 100, 0),
(88370, 73, -5461.11, -2877.83, 349.371, 0, 0, 0, NULL, 100, 0),
(88370, 74, -5438.5, -2882.87, 345.996, 0, 0, 0, NULL, 100, 0),
(88370, 75, -5418.17, -2884.11, 343.776, 0, 0, 0, NULL, 100, 0),
(88370, 76, -5401.08, -2882.8, 342.526, 0, 0, 0, NULL, 100, 0),
(88370, 77, -5387.69, -2876.06, 341.294, 0, 0, 0, NULL, 100, 0),
(88370, 78, -5372.36, -2865.12, 340.589, 0, 0, 0, NULL, 100, 0),
(88370, 79, -5361.94, -2860.15, 340.111, 0, 0, 0, NULL, 100, 0),
(88370, 80, -5348.32, -2858.7, 340.111, 0, 0, 0, NULL, 100, 0),
(88370, 81, -5333.68, -2859.74, 339.861, 0, 0, 0, NULL, 100, 0),
(88370, 82, -5323.2, -2861.15, 339.682, 0, 0, 0, NULL, 100, 0),
(88370, 83, -5303.08, -2867.47, 339.39, 0, 0, 0, NULL, 100, 0),
(88370, 84, -5289.73, -2875.12, 338.845, 0, 0, 0, NULL, 100, 0),
(88370, 85, -5281.11, -2880.25, 338.72, 0, 0, 0, NULL, 100, 0),
(88370, 86, -5272.28, -2882.19, 338.845, 0, 0, 0, NULL, 100, 0),
(88370, 87, -5261.87, -2883.87, 339.224, 0, 0, 0, NULL, 100, 0),
(88370, 88, -5250.37, -2883.83, 339.224, 0, 0, 0, NULL, 100, 0),
(88370, 89, -5239.1, -2879.95, 338.849, 0, 0, 0, NULL, 100, 0),
(88370, 90, -5228.33, -2871.78, 338.201, 0, 0, 0, NULL, 100, 0),
(88370, 91, -5219.27, -2864.04, 337.326, 0, 0, 0, NULL, 100, 0),
(88370, 92, -5206.53, -2856.11, 336.451, 0, 0, 0, NULL, 100, 0),
(88370, 93, -5194.13, -2850.41, 335.914, 0, 0, 0, NULL, 100, 0),
(88370, 94, -5178.38, -2847.05, 335.164, 0, 0, 0, NULL, 100, 0),
(88370, 95, -5161.85, -2843.03, 334.019, 0, 0, 0, NULL, 100, 0),
(88370, 96, -5141.43, -2839.25, 331.769, 0, 0, 0, NULL, 100, 0),
(88370, 97, -5128.13, -2835.52, 330.348, 0, 0, 0, NULL, 100, 0),
(88370, 98, -5108.58, -2832.45, 328.104, 0, 0, 0, NULL, 100, 0),
(88370, 99, -5094.14, -2830.39, 327.362, 0, 0, 0, NULL, 100, 0),
(88370, 100, -5077.85, -2825.76, 327.362, 0, 0, 0, NULL, 100, 0),
(88370, 101, -5065.58, -2820.55, 327.467, 0, 0, 0, NULL, 100, 0),
(88370, 102, -5050.61, -2813.99, 327.467, 0, 0, 0, NULL, 100, 0),
(88370, 103, -5033.33, -2804.15, 326.967, 0, 0, 0, NULL, 100, 0),
(88370, 104, -5017.87, -2795.1, 326.588, 0, 0, 0, NULL, 100, 0),
(88370, 105, -4997.96, -2786.48, 326.083, 0, 0, 0, NULL, 100, 0),
(88370, 106, -4971.96, -2774.73, 325.708, 0, 0, 0, NULL, 100, 0),
(88370, 107, -4952.18, -2763.71, 325.972, 0, 0, 0, NULL, 100, 0),
(88370, 108, -4942.4, -2755.38, 326.722, 0, 0, 0, NULL, 100, 0),
(88370, 109, -4930.26, -2745.37, 327.549, 0, 0, 0, NULL, 100, 0),
(88370, 110, -4930.26, -2745.37, 327.549, 0, 0, 0, NULL, 100, 0),
(88370, 111, -4923.54, -2739.28, 328.174, 0, 0, 0, NULL, 100, 0),
(88370, 112, -4916.87, -2733.83, 328.549, 0, 0, 0, NULL, 100, 0),
(88370, 113, -4902.89, -2728.64, 329.004, 0, 0, 0, NULL, 100, 0),
(88370, 114, -4893.55, -2725.46, 329.134, 0, 0, 0, NULL, 100, 0),
(88370, 115, -4878.61, -2721.18, 329.009, 0, 0, 0, NULL, 100, 0),
(88370, 116, -4865.14, -2716.86, 329.099, 0, 0, 0, NULL, 100, 0),
(88370, 117, -4852.49, -2715.27, 329.224, 0, 0, 0, NULL, 100, 0),
(88370, 118, -4838.67, -2712.17, 328.849, 0, 0, 0, NULL, 100, 0),
(88370, 119, -4827.89, -2712.34, 328.95, 0, 0, 0, NULL, 100, 0),
(88370, 120, -4816.42, -2711.93, 328.075, 0, 0, 0, NULL, 100, 0),
(88370, 121, -4800.38, -2711.82, 327.075, 0, 0, 0, NULL, 100, 0),
(88370, 122, -4787.2, -2711.78, 326.372, 0, 0, 0, NULL, 100, 0),
(88370, 123, -4771.74, -2710.35, 326.497, 0, 0, 0, NULL, 100, 0),
(88370, 124, -4762.36, -2709.65, 326.31, 0, 0, 0, NULL, 100, 0),
(88370, 125, -4749.9, -2704.78, 325.06, 0, 0, 0, NULL, 100, 0),
(88370, 126, -4742.16, -2702.72, 324.31, 0, 0, 0, NULL, 100, 0),
(88370, 127, -4724.57, -2699.77, 321.732, 0, 0, 0, NULL, 100, 0),
(88370, 128, -4713.88, -2695.41, 319.732, 0, 0, 0, NULL, 100, 0),
(88370, 129, -4704.73, -2686.48, 318.982, 0, 480000, 0, 1209, 100, 0),
(88370, 130, -4717.67, -2696.74, 320.482, 0, 0, 0, NULL, 100, 0),
(88370, 131, -4731.33, -2700.64, 322.843, 0, 0, 0, NULL, 100, 0),
(88370, 132, -4738.51, -2702.03, 323.685, 0, 0, 0, NULL, 100, 0),
(88370, 133, -4745.31, -2704.01, 324.685, 0, 0, 0, NULL, 100, 0),
(88370, 134, -4752.6, -2705.99, 325.435, 0, 0, 0, NULL, 100, 0),
(88370, 135, -4762.18, -2708.76, 326.31, 0, 0, 0, NULL, 100, 0),
(88370, 136, -4770.29, -2710.79, 326.497, 0, 0, 0, NULL, 100, 0),
(88370, 137, -4784.5, -2711.67, 326.372, 0, 0, 0, NULL, 100, 0),
(88370, 138, -4797.29, -2711.24, 326.997, 0, 0, 0, NULL, 100, 0),
(88370, 139, -4808.83, -2710.9, 327.575, 0, 0, 0, NULL, 100, 0),
(88370, 140, -4819.8, -2712.04, 328.45, 0, 0, 0, NULL, 100, 0),
(88370, 141, -4831.77, -2711.63, 329.075, 0, 0, 0, NULL, 100, 0),
(88370, 142, -4841.65, -2712.95, 328.849, 0, 0, 0, NULL, 100, 0),
(88370, 143, -4849.67, -2713.96, 329.099, 0, 0, 0, NULL, 100, 0),
(88370, 144, -4859.83, -2716.55, 329.224, 0, 0, 0, NULL, 100, 0),
(88370, 145, -4870.26, -2717.95, 329.134, 0, 0, 0, NULL, 100, 0),
(88370, 146, -4886.87, -2724.08, 329.009, 0, 0, 0, NULL, 100, 0),
(88370, 147, -4895.86, -2727.09, 328.884, 0, 0, 0, NULL, 100, 0),
(88370, 148, -4901.36, -2726.47, 329.254, 0, 0, 0, NULL, 100, 0),
(88370, 149, -4903.67, -2725.18, 329.504, 0, 480000, 0, NULL, 100, 0),
(88370, 150, -4910.04, -2728.99, 329.004, 0, 0, 0, NULL, 100, 0),
(88370, 151, -4918.72, -2734.68, 328.549, 0, 0, 0, NULL, 100, 0),
(88370, 152, -4927.25, -2742.53, 327.924, 0, 0, 0, NULL, 100, 0),
(88370, 153, -4939.73, -2754.26, 326.847, 0, 0, 0, NULL, 100, 0),
(88370, 154, -4950.77, -2763.37, 326.097, 0, 0, 0, NULL, 100, 0),
(88370, 155, -4961.1, -2769.41, 325.681, 0, 0, 0, NULL, 100, 0),
(88370, 156, -4976.89, -2777.53, 325.708, 0, 0, 0, NULL, 100, 0),
(88370, 157, -4987.04, -2781.37, 325.708, 0, 0, 0, NULL, 100, 0),
(88370, 158, -4996.45, -2785.28, 325.958, 0, 0, 0, NULL, 100, 0),
(88370, 159, -5004.28, -2789.16, 326.213, 0, 0, 0, NULL, 100, 0),
(88370, 160, -5018.58, -2794.61, 326.463, 0, 0, 0, NULL, 100, 0),
(88370, 161, -5028.46, -2800.17, 326.828, 0, 0, 0, NULL, 100, 0),
(88370, 162, -5035.88, -2805.19, 327.092, 0, 0, 0, NULL, 100, 0),
(88370, 163, -5043.92, -2809.96, 327.342, 0, 0, 0, NULL, 100, 0),
(88370, 164, -5053.04, -2815.22, 327.467, 0, 0, 0, NULL, 100, 0),
(88370, 165, -5064.96, -2819.95, 327.467, 0, 0, 0, NULL, 100, 0),
(88370, 166, -5076.34, -2824.69, 327.362, 0, 0, 0, NULL, 100, 0),
(88370, 167, -5094.38, -2829.96, 327.237, 0, 0, 0, NULL, 100, 0),
(88370, 168, -5110.39, -2833.33, 328.229, 0, 0, 0, NULL, 100, 0),
(88370, 169, -5123.83, -2834.54, 329.848, 0, 0, 0, NULL, 100, 0),
(88370, 170, -5135.4, -2837.19, 331.019, 0, 0, 0, NULL, 100, 0),
(88370, 171, -5157.86, -2842.52, 333.394, 0, 0, 0, NULL, 100, 0),
(88370, 172, -5174.73, -2846.02, 335.039, 0, 0, 0, NULL, 100, 0),
(88370, 173, -5185.71, -2848.4, 335.539, 0, 0, 0, NULL, 100, 0),
(88370, 174, -5196.61, -2851.75, 335.914, 0, 0, 0, NULL, 100, 0),
(88370, 175, -5207.44, -2856.91, 336.451, 0, 0, 0, NULL, 100, 0),
(88370, 176, -5215.62, -2862.42, 336.951, 0, 0, 0, NULL, 100, 0),
(88370, 177, -5223.25, -2867.82, 337.701, 0, 0, 0, NULL, 100, 0),
(88370, 178, -5231.61, -2874.67, 338.451, 0, 0, 0, NULL, 100, 0),
(88370, 179, -5239.23, -2879.84, 338.849, 0, 0, 0, NULL, 100, 0),
(88370, 180, -5245.9, -2883.9, 339.224, 0, 0, 0, NULL, 100, 0),
(88370, 181, -5258.93, -2894.81, 338.349, 0, 0, 0, NULL, 100, 0),
(88370, 182, -5270.1, -2901.57, 337.396, 0, 0, 0, NULL, 100, 0),
(88370, 183, -5284.19, -2904.96, 338.021, 0, 0, 0, NULL, 100, 0),
(88370, 184, -5295.71, -2907.05, 338.146, 0, 0, 0, NULL, 100, 0),
(88370, 185, -5304.23, -2914.32, 338.737, 0, 0, 0, NULL, 100, 0),
(88370, 186, -5310.98, -2921.97, 339.987, 0, 0, 0, NULL, 100, 0),
(88370, 187, -5316.07, -2929.81, 338.612, 0, 0, 0, NULL, 100, 0),
(88370, 188, -5318.16, -2933.97, 336.485, 0, 0, 0, NULL, 100, 0),
(88370, 189, -5318.11, -2939.98, 333.985, 0, 0, 0, NULL, 100, 0),
(88370, 190, -5316.84, -2945.46, 333.61, 0, 0, 0, NULL, 100, 0),
(88370, 191, -5318.44, -2951.34, 332.985, 0, 0, 0, NULL, 100, 0),
(88370, 192, -5322.65, -2954.56, 331.36, 0, 0, 0, NULL, 100, 0),
(88370, 193, -5329.89, -2956.08, 328.11, 0, 0, 0, NULL, 100, 0),
(88370, 194, -5337.45, -2957.14, 325.384, 0, 0, 0, NULL, 100, 0),
(88370, 195, -5345.12, -2957.3, 323.759, 0, 0, 0, NULL, 100, 0),
(88370, 196, -5351, -2950.18, 323.634, 0, 0, 0, NULL, 100, 0),
(88370, 197, -5355.96, -2941.26, 324.009, 0, 480000, 0, 1210, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (8837, 88370, 0, 0, 0, 257, 0, NULL);

-- Importing creature with guid 8836 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1008803 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1008803 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1008803 AND spawnType = 0 WHERE ce.spawnID = 1008803;
DELETE FROM waypoint_data WHERE id = 154;
INSERT INTO creature_entry (spawnID, entry) VALUES (8836, 1279);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (8836, 0, 1, 1783, 1, -5062.33, -2819.33,  327.467, 0.556077, 300, 0, 0, 1910, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(88360, 1, -4942.93, -2757.03, 326.597, 0, 0, 0, NULL, 100, 0),
(88360, 2, -4953.73, -2765.19, 325.847, 0, 0, 0, NULL, 100, 0),
(88360, 3, -4965.82, -2771.82, 325.681, 0, 0, 0, NULL, 100, 0),
(88360, 4, -4989.56, -2782.11, 325.708, 0, 0, 0, NULL, 100, 0),
(88360, 5, -5000.13, -2787.29, 326.088, 0, 0, 0, NULL, 100, 0),
(88360, 6, -5011.12, -2791.75, 326.338, 0, 0, 0, NULL, 100, 0),
(88360, 7, -5022.85, -2797.01, 326.588, 0, 0, 0, NULL, 100, 0),
(88360, 8, -5037.75, -2806.09, 327.092, 0, 0, 0, NULL, 100, 0),
(88360, 9, -5049.75, -2813.56, 327.467, 0, 0, 0, NULL, 100, 0),
(88360, 10, -5062.33, -2819.33, 327.467, 0, 0, 0, NULL, 100, 0),
(88360, 11, -5079.38, -2825.85, 327.362, 0, 0, 0, NULL, 100, 0),
(88360, 12, -5097.02, -2830.69, 327.237, 0, 0, 0, NULL, 100, 0),
(88360, 13, -5108.48, -2833.38, 328.098, 0, 0, 0, NULL, 100, 0),
(88360, 14, -5121.28, -2834.46, 329.723, 0, 0, 0, NULL, 100, 0),
(88360, 15, -5139.66, -2838.25, 331.769, 0, 0, 0, NULL, 100, 0),
(88360, 16, -5152.31, -2841.23, 332.894, 0, 0, 0, NULL, 100, 0),
(88360, 17, -5163.88, -2843.81, 334.269, 0, 0, 0, NULL, 100, 0),
(88360, 18, -5152.31, -2841.23, 332.894, 0, 0, 0, NULL, 100, 0),
(88360, 19, -5139.66, -2838.25, 331.769, 0, 0, 0, NULL, 100, 0),
(88360, 20, -5121.28, -2834.46, 329.723, 0, 0, 0, NULL, 100, 0),
(88360, 21, -5108.48, -2833.38, 328.098, 0, 0, 0, NULL, 100, 0),
(88360, 22, -5097.02, -2830.69, 327.237, 0, 0, 0, NULL, 100, 0),
(88360, 23, -5079.38, -2825.85, 327.362, 0, 0, 0, NULL, 100, 0),
(88360, 24, -5062.33, -2819.33, 327.467, 0, 0, 0, NULL, 100, 0),
(88360, 25, -5049.75, -2813.56, 327.467, 0, 0, 0, NULL, 100, 0),
(88360, 26, -5037.75, -2806.09, 327.092, 0, 0, 0, NULL, 100, 0),
(88360, 27, -5022.85, -2797.01, 326.588, 0, 0, 0, NULL, 100, 0),
(88360, 28, -5011.12, -2791.75, 326.338, 0, 0, 0, NULL, 100, 0),
(88360, 29, -5000.13, -2787.29, 326.088, 0, 0, 0, NULL, 100, 0),
(88360, 30, -4989.56, -2782.11, 325.708, 0, 0, 0, NULL, 100, 0),
(88360, 31, -4965.82, -2771.82, 325.681, 0, 0, 0, NULL, 100, 0),
(88360, 32, -4953.73, -2765.19, 325.847, 0, 0, 0, NULL, 100, 0),
(88360, 33, -4942.93, -2757.03, 326.597, 0, 0, 0, NULL, 100, 0),
(88360, 34, -4924.17, -2740.74, 328.049, 0, 0, 0, NULL, 100, 0),
(88360, 35, -4917.93, -2735.47, 328.549, 0, 0, 0, NULL, 100, 0),
(88360, 36, -4924.17, -2740.74, 328.049, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (8836, 88360, 2784, 0, 0, 257, 0, NULL);

-- Importing creature with guid 8376 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1008343 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1008343 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1008343 AND spawnType = 0 WHERE ce.spawnID = 1008343;
INSERT INTO creature_entry (spawnID, entry) VALUES (8376, 1278);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (8376, 0, 1, 1804, 1, -5400.07, -2890.89,  342.151, 3.8275, 300, 0, 0, 1910, 0, 2, 0, 0, 10);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1205, 1, 30, 0, 0, 0, 0, 0, 0, 1.5708);
INSERT INTO waypoint_data VALUES 
(83760, 1, -5400.81, -2890.18, 342.151, 0, 0, 0, NULL, 100, 0),
(83760, 2, -5402.46, -2887.74, 342.526, 0, 0, 0, NULL, 100, 0),
(83760, 3, -5402.48, -2884.26, 342.651, 0, 0, 0, NULL, 100, 0),
(83760, 4, -5396.96, -2881.71, 342.044, 0, 0, 0, NULL, 100, 0),
(83760, 5, -5385.52, -2874.89, 341.044, 0, 0, 0, NULL, 100, 0),
(83760, 6, -5376.24, -2867.62, 340.669, 0, 0, 0, NULL, 100, 0),
(83760, 7, -5367.39, -2861.61, 340.464, 0, 0, 0, NULL, 100, 0),
(83760, 8, -5358.19, -2859.16, 339.986, 0, 0, 0, NULL, 100, 0),
(83760, 9, -5346.85, -2858.04, 340.236, 0, 0, 0, NULL, 100, 0),
(83760, 10, -5327.97, -2859.75, 339.807, 0, 0, 0, NULL, 100, 0),
(83760, 11, -5298.9, -2869.29, 339.22, 0, 150000, 0, NULL, 100, 0),
(83760, 12, -5327.97, -2859.75, 339.807, 0, 0, 0, NULL, 100, 0),
(83760, 13, -5345.94, -2858.23, 340.236, 0, 0, 0, NULL, 100, 0),
(83760, 14, -5360.1, -2859.91, 339.986, 0, 0, 0, NULL, 100, 0),
(83760, 15, -5366.3, -2860.87, 340.486, 0, 0, 0, NULL, 100, 0),
(83760, 16, -5372.33, -2864.81, 340.589, 0, 0, 0, NULL, 100, 0),
(83760, 17, -5385.4, -2874.93, 341.044, 0, 0, 0, NULL, 100, 0),
(83760, 18, -5393.84, -2884.03, 342.044, 0, 0, 0, NULL, 100, 0),
(83760, 19, -5396.55, -2889.1, 341.919, 0, 0, 0, NULL, 100, 0),
(83760, 20, -5400.07, -2890.89, 342.151, 0, 150000, 0, 1205, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (8376, 83760, 0, 0, 0, 257, 0, NULL);

-- Importing creature with guid 8267 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1008234 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1008234 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1008234 AND spawnType = 0 WHERE ce.spawnID = 1008234;
DELETE FROM waypoint_data WHERE id = 146;
INSERT INTO creature_entry (spawnID, entry) VALUES (8267, 1277);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (8267, 0, 1, 1784, 1, -4913.53, -2728.87,  328.879, 1.03567, 300, 0, 0, 1910, 0, 2, 0, 0, 10);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1204, 1, 30, 0, 0, 0, 0, 0, 0, 1.39626);
INSERT INTO waypoint_data VALUES 
(82670, 1, -4898.83, -2712.74, 330.759, 0, 0, 0, NULL, 100, 0),
(82670, 2, -4885.94, -2697.47, 333.408, 0, 0, 0, NULL, 100, 0),
(82670, 3, -4870.99, -2677.91, 337.283, 0, 0, 0, NULL, 100, 0),
(82670, 4, -4860.33, -2660.15, 341.36, 0, 0, 0, NULL, 100, 0),
(82670, 5, -4847.55, -2636.46, 345.985, 0, 0, 0, NULL, 100, 0),
(82670, 6, -4830.76, -2608.48, 351.367, 0, 0, 0, NULL, 100, 0),
(82670, 7, -4816.64, -2580.04, 355.197, 0, 0, 0, NULL, 100, 0),
(82670, 8, -4805.33, -2558.51, 355.431, 0, 0, 0, NULL, 100, 0),
(82670, 9, -4802.09, -2550.53, 355.556, 0, 0, 0, NULL, 100, 0),
(82670, 10, -4801, -2539.98, 354.431, 0, 0, 0, NULL, 100, 0),
(82670, 11, -4794.47, -2525.01, 353.426, 0, 150000, 0, 1204, 100, 0),
(82670, 12, -4800.21, -2530.92, 353.649, 0, 0, 0, NULL, 100, 0),
(82670, 13, -4801.5, -2541.83, 354.806, 0, 0, 0, NULL, 100, 0),
(82670, 14, -4803.78, -2555.07, 355.431, 0, 0, 0, NULL, 100, 0),
(82670, 15, -4808.03, -2563.27, 355.306, 0, 0, 0, NULL, 100, 0),
(82670, 16, -4812.43, -2572.1, 355.447, 0, 0, 0, NULL, 100, 0),
(82670, 17, -4820.55, -2588.57, 354.072, 0, 0, 0, NULL, 100, 0),
(82670, 18, -4829.76, -2605.47, 351.867, 0, 0, 0, NULL, 100, 0),
(82670, 19, -4843.01, -2629.36, 347.459, 0, 0, 0, NULL, 100, 0),
(82670, 20, -4858.24, -2658.04, 341.86, 0, 0, 0, NULL, 100, 0),
(82670, 21, -4874.24, -2682.2, 336.283, 0, 0, 0, NULL, 100, 0),
(82670, 22, -4889.43, -2701.91, 332.759, 0, 0, 0, NULL, 100, 0),
(82670, 23, -4902.95, -2717.82, 330.004, 0, 0, 0, NULL, 100, 0),
(82670, 24, -4913.53, -2728.87, 328.879, 0, 150000, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (8267, 82670, 0, 0, 0, 257, 0, NULL);

-- Importing creature with guid 8266 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1008233 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1008233 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1008233 AND spawnType = 0 WHERE ce.spawnID = 1008233;
INSERT INTO creature_entry (spawnID, entry) VALUES (8266, 1283);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (8266, 0, 1, 1793, 1, -5835.77, -2581.63,  311.248, 2.33874, 300, 0, 0, 1910, 0, 2, 0, 0, 10);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1208, 1, 30, 0, 0, 0, 0, 0, 0, 2.33874);
INSERT INTO waypoint_data VALUES 
(82660, 1, -5826.23, -2589.55, 312.487, 0, 0, 0, NULL, 100, 0),
(82660, 2, -5815.71, -2599.46, 314.112, 0, 0, 0, NULL, 100, 0),
(82660, 3, -5803.19, -2612.9, 317.146, 0, 150000, 0, NULL, 100, 0),
(82660, 4, -5818.31, -2596.83, 313.737, 0, 0, 0, NULL, 100, 0),
(82660, 5, -5832.67, -2584.36, 311.737, 0, 0, 0, NULL, 100, 0),
(82660, 6, -5840.37, -2577.5, 310.623, 0, 0, 0, NULL, 100, 0),
(82660, 7, -5860.45, -2565.25, 308.309, 0, 0, 0, NULL, 100, 0),
(82660, 8, -5870.4, -2558.15, 307.326, 0, 0, 0, NULL, 100, 0),
(82660, 9, -5880.91, -2549.04, 307.451, 0, 0, 0, NULL, 100, 0),
(82660, 10, -5890.77, -2542.17, 307.326, 0, 0, 0, NULL, 100, 0),
(82660, 11, -5898.68, -2537.93, 307.201, 0, 0, 0, NULL, 100, 0),
(82660, 12, -5905.8, -2535.37, 307.122, 0, 0, 0, NULL, 100, 0),
(82660, 13, -5918.47, -2532.91, 307.389, 0, 0, 0, NULL, 100, 0),
(82660, 14, -5931.82, -2532.19, 307.639, 0, 0, 0, NULL, 100, 0),
(82660, 15, -5943.23, -2530.59, 307.884, 0, 0, 0, NULL, 100, 0),
(82660, 16, -5958.02, -2527.77, 308.134, 0, 0, 0, NULL, 100, 0),
(82660, 17, -5971.92, -2524.21, 307.852, 0, 0, 0, NULL, 100, 0),
(82660, 18, -5985.28, -2520.41, 307.977, 0, 0, 0, NULL, 100, 0),
(82660, 19, -5999.92, -2514.3, 307.977, 0, 0, 0, NULL, 100, 0),
(82660, 20, -6010.38, -2507.41, 308.054, 0, 0, 0, NULL, 100, 0),
(82660, 21, -6018.31, -2501.65, 308.804, 0, 0, 0, NULL, 100, 0),
(82660, 22, -6022.93, -2491.44, 309.861, 0, 210000, 0, 1208, 100, 0),
(82660, 23, -6020.1, -2500.61, 309.054, 0, 0, 0, NULL, 100, 0),
(82660, 24, -6006.42, -2510.47, 308.054, 0, 0, 0, NULL, 100, 0),
(82660, 25, -5996.96, -2515.4, 308.102, 0, 0, 0, NULL, 100, 0),
(82660, 26, -5986.25, -2519.77, 307.977, 0, 0, 0, NULL, 100, 0),
(82660, 27, -5969.3, -2524.48, 307.977, 0, 0, 0, NULL, 100, 0),
(82660, 28, -5948.3, -2529.43, 308.009, 0, 0, 0, NULL, 100, 0),
(82660, 29, -5933.23, -2532.08, 307.639, 0, 0, 0, NULL, 100, 0),
(82660, 30, -5915.6, -2532.92, 307.389, 0, 0, 0, NULL, 100, 0),
(82660, 31, -5905.06, -2535.47, 307.122, 0, 0, 0, NULL, 100, 0),
(82660, 32, -5896.94, -2538.66, 307.201, 0, 0, 0, NULL, 100, 0),
(82660, 33, -5889.17, -2542.61, 307.326, 0, 0, 0, NULL, 100, 0),
(82660, 34, -5870.55, -2558.16, 307.326, 0, 0, 0, NULL, 100, 0),
(82660, 35, -5859.49, -2566.11, 308.434, 0, 0, 0, NULL, 100, 0),
(82660, 36, -5846.71, -2573.42, 309.873, 0, 0, 0, NULL, 100, 0),
(82660, 37, -5835.77, -2581.63, 311.248, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (8266, 82660, 0, 0, 0, 257, 0, NULL);

-- Importing creature with guid 8245 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1008212 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1008212 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1008212 AND spawnType = 0 WHERE ce.spawnID = 1008212;
DELETE FROM waypoint_data WHERE id = 145;
INSERT INTO creature_entry (spawnID, entry) VALUES (8245, 2525);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (8245, 0, 1, 1775, 1, -4746.26, -3261.94,  310.257, 0.777376, 300, 0, 0, 1910, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(82450, 1, -4726.79, -3227.81, 310.242, 0, 0, 0, NULL, 100, 0),
(82450, 2, -4715.29, -3211.72, 310.216, 0, 0, 0, NULL, 100, 0),
(82450, 3, -4683.43, -3180.37, 310.175, 0, 0, 0, NULL, 100, 0),
(82450, 4, -4715.29, -3211.72, 310.216, 0, 0, 0, NULL, 100, 0),
(82450, 5, -4726.79, -3227.81, 310.242, 0, 0, 0, NULL, 100, 0),
(82450, 6, -4746.26, -3261.94, 310.257, 0, 0, 0, NULL, 100, 0),
(82450, 7, -4751.9, -3284.64, 310.257, 0, 0, 0, NULL, 100, 0),
(82450, 8, -4753.05, -3302.44, 310.257, 0, 0, 0, NULL, 100, 0),
(82450, 9, -4752.91, -3323.97, 310.257, 0, 0, 0, NULL, 100, 0),
(82450, 10, -4748.53, -3348.1, 310.257, 0, 0, 0, NULL, 100, 0),
(82450, 11, -4739.19, -3378.95, 310.257, 0, 0, 0, NULL, 100, 0),
(82450, 12, -4727.73, -3400.77, 310.257, 0, 0, 0, NULL, 100, 0),
(82450, 13, -4712.54, -3423.35, 310.216, 0, 0, 0, NULL, 100, 0),
(82450, 14, -4688.15, -3453.38, 310.134, 0, 0, 0, NULL, 100, 0),
(82450, 15, -4712.54, -3423.35, 310.216, 0, 0, 0, NULL, 100, 0),
(82450, 16, -4727.7, -3400.83, 310.249, 0, 0, 0, NULL, 100, 0),
(82450, 17, -4739.19, -3378.95, 310.257, 0, 0, 0, NULL, 100, 0),
(82450, 18, -4748.53, -3348.1, 310.257, 0, 0, 0, NULL, 100, 0),
(82450, 19, -4752.91, -3323.97, 310.257, 0, 0, 0, NULL, 100, 0),
(82450, 20, -4753.05, -3302.44, 310.257, 0, 0, 0, NULL, 100, 0),
(82450, 21, -4751.9, -3284.64, 310.257, 0, 0, 0, NULL, 100, 0),
(82450, 22, -4746.26, -3261.94, 310.257, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (8245, 82450, 0, 0, 0, 257, 0, NULL);

-- Importing creature with guid 33919 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1033779 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1033779 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1033779 AND spawnType = 0 WHERE ce.spawnID = 1033779;
DELETE FROM waypoint_data WHERE id = 850;
INSERT INTO creature_entry (spawnID, entry) VALUES (33919, 4982);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (33919, 0, 1, 0, 0, -8511, 860.464,  109.839, 3.799, 180, 0, 0, 42, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(339190, 1, -8512.6, 857.178, 109.835, 0, 0, 0, NULL, 100, 0),
(339190, 2, -8516.57, 857.887, 109.842, 0, 0, 0, NULL, 100, 0),
(339190, 3, -8524.35, 851.796, 106.519, 0, 0, 0, NULL, 100, 0),
(339190, 4, -8564.14, 820.632, 106.519, 0, 0, 0, NULL, 100, 0),
(339190, 5, -8562.94, 810.543, 106.519, 0, 0, 0, NULL, 100, 0),
(339190, 6, -8575.4, 802.051, 106.519, 0, 0, 0, NULL, 100, 0),
(339190, 7, -8585.01, 815.154, 106.519, 0, 0, 0, NULL, 100, 0),
(339190, 8, -8572.4, 825.437, 106.519, 0, 0, 0, NULL, 100, 0),
(339190, 9, -8565.34, 821.039, 106.519, 0, 0, 0, NULL, 100, 0),
(339190, 10, -8516.42, 857.777, 109.843, 0, 0, 0, NULL, 100, 0),
(339190, 11, -8512.67, 857.747, 109.836, 0, 0, 0, NULL, 100, 0),
(339190, 12, -8510.97, 860.365, 109.838, 0, 0, 0, NULL, 100, 0),
(339190, 13, -8511.45, 859.994, 109.838, 0, 4, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (33919, 339190, 0, 0, 0, 0, 0, NULL);

-- Importing creature with guid 15424 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1015313 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1015313 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1015313 AND spawnType = 0 WHERE ce.spawnID = 1015313;
INSERT INTO creature_entry (spawnID, entry) VALUES (15424, 3537);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (15424, 0, 1, 7133, 1, -36.31, -916.366,  55.075, 1.062, 300, 0, 0, 1163, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(154240, 1, -36.31, -916.366, 55.075, 1.062, 180000, 0, NULL, 100, 0),
(154240, 2, -28.5326, -902.441, 55.9086, 0, 0, 0, NULL, 100, 0),
(154240, 3, -26.3387, -896.902, 56.0395, 0, 0, 0, NULL, 100, 0),
(154240, 4, -51.5009, -836.735, 56.5249, 0, 0, 0, NULL, 100, 0),
(154240, 5, -17.1652, -802.633, 58.7518, 0, 0, 0, NULL, 100, 0),
(154240, 6, -20.9832, -719.064, 69.291, 0, 0, 0, NULL, 100, 0),
(154240, 7, -149.228, -712.801, 64.4451, 0, 0, 0, NULL, 100, 0),
(154240, 8, -226.017, -722.711, 60.8613, 0, 0, 0, NULL, 100, 0),
(154240, 9, -325.539, -754.267, 54.0897, 0, 0, 0, NULL, 100, 0),
(154240, 10, -377.87, -776.472, 54.473, 0, 0, 0, NULL, 100, 0),
(154240, 11, -403.164, -680.893, 54.499, 0, 0, 0, NULL, 100, 0),
(154240, 12, -331.618, -671.147, 54.9188, 0, 0, 0, NULL, 100, 0),
(154240, 13, -341.248, -713.172, 57.733, 0, 0, 0, NULL, 100, 0),
(154240, 14, -340.615, -711.776, 57.733, 0, 120000, 0, NULL, 100, 0),
(154240, 15, -325.248, -677.674, 54.5963, 0, 0, 0, NULL, 100, 0),
(154240, 16, -339.87, -673.188, 55.0129, 0, 0, 0, NULL, 100, 0),
(154240, 17, -385.749, -685.196, 54.3873, 0, 0, 0, NULL, 100, 0),
(154240, 18, -413.708, -656.888, 54.489, 0, 0, 0, NULL, 100, 0),
(154240, 19, -440.758, -585.658, 53.4242, 0, 0, 0, NULL, 100, 0),
(154240, 20, -520.923, -558.852, 39.921, 0, 0, 0, NULL, 100, 0),
(154240, 21, -569.16, -567.547, 32.8094, 0, 0, 0, NULL, 100, 0),
(154240, 22, -594.512, -575.941, 31.9821, 0, 0, 0, NULL, 100, 0),
(154240, 23, -650.699, -560.432, 26.121, 0, 0, 0, NULL, 100, 0),
(154240, 24, -691.625, -568.642, 24.5409, 0, 0, 0, NULL, 100, 0),
(154240, 25, -707.11, -562.611, 22.8095, 0, 0, 0, NULL, 100, 0),
(154240, 26, -724.787, -549.384, 20.2918, 0, 0, 0, NULL, 100, 0),
(154240, 27, -811.11, -542.175, 15.772, 0, 0, 0, NULL, 100, 0),
(154240, 28, -817.55, -533.02, 15.1606, 0, 180000, 0, NULL, 100, 0),
(154240, 29, -811.11, -542.175, 15.772, 0, 0, 0, NULL, 100, 0),
(154240, 30, -724.787, -549.384, 20.2918, 0, 0, 0, NULL, 100, 0),
(154240, 31, -707.11, -562.611, 22.8095, 0, 0, 0, NULL, 100, 0),
(154240, 32, -691.625, -568.642, 24.5409, 0, 0, 0, NULL, 100, 0),
(154240, 33, -650.699, -560.432, 26.121, 0, 0, 0, NULL, 100, 0),
(154240, 34, -594.512, -575.941, 31.9821, 0, 0, 0, NULL, 100, 0),
(154240, 35, -569.16, -567.547, 32.8094, 0, 0, 0, NULL, 100, 0),
(154240, 36, -520.923, -558.852, 39.921, 0, 0, 0, NULL, 100, 0),
(154240, 37, -440.758, -585.658, 53.4242, 0, 0, 0, NULL, 100, 0),
(154240, 38, -413.708, -656.888, 54.489, 0, 0, 0, NULL, 100, 0),
(154240, 39, -385.749, -685.196, 54.3873, 0, 0, 0, NULL, 100, 0),
(154240, 40, -339.87, -673.188, 55.0129, 0, 0, 0, NULL, 100, 0),
(154240, 41, -325.248, -677.674, 54.5963, 0, 0, 0, NULL, 100, 0),
(154240, 42, -340.615, -711.776, 57.733, 0, 120000, 0, NULL, 100, 0),
(154240, 43, -341.248, -713.172, 57.733, 0, 0, 0, NULL, 100, 0),
(154240, 44, -331.618, -671.147, 54.9188, 0, 0, 0, NULL, 100, 0),
(154240, 45, -403.164, -680.893, 54.499, 0, 0, 0, NULL, 100, 0),
(154240, 46, -377.87, -776.472, 54.473, 0, 0, 0, NULL, 100, 0),
(154240, 47, -325.539, -754.267, 54.0897, 0, 0, 0, NULL, 100, 0),
(154240, 48, -226.017, -722.711, 60.8613, 0, 0, 0, NULL, 100, 0),
(154240, 49, -149.228, -712.801, 64.4451, 0, 0, 0, NULL, 100, 0),
(154240, 50, -20.9832, -719.064, 69.291, 0, 0, 0, NULL, 100, 0),
(154240, 51, -17.1652, -802.633, 58.7518, 0, 0, 0, NULL, 100, 0),
(154240, 52, -51.5009, -836.735, 56.5249, 0, 0, 0, NULL, 100, 0),
(154240, 53, -26.3387, -896.902, 56.0395, 0, 0, 0, NULL, 100, 0),
(154240, 54, -28.5326, -902.441, 55.9086, 0, 0, 0, NULL, 100, 0),
(154240, 55, -38.9737, -917.01, 55.0652, 1.062, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (15424, 154240, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 15423 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1015312 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1015312 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1015312 AND spawnType = 0 WHERE ce.spawnID = 1015312;
INSERT INTO creature_entry (spawnID, entry) VALUES (15423, 3538);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (15423, 0, 1, 5299, 0, -37.248, -914.605,  55.343, 0.826, 300, 0, 0, 1163, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (15423, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 769 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (769, 723);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (769, 0, 1, 0, 1, -12492.5, -942.217,  42.1543, 0.814825, 300, 10, 0, 2050, 0, 1, 0, 0, 10);

-- Importing creature with guid 636 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1000628 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1000628 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1000628 AND spawnType = 0 WHERE ce.spawnID = 1000628;
INSERT INTO creature_entry (spawnID, entry) VALUES (636, 14508);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (636, 0, 1, 14561, 1, -13184.5, 332.671,  40.3307, 4.41568, 300, 0, 0, 2488, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (636, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 5813 with import type REPLACE_ALL
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 5804 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -5804 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 5804 AND spawnType = 0 WHERE ce.spawnID = 5804;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 5813 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -5813 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 5813 AND spawnType = 0 WHERE ce.spawnID = 5813;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1005779 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1005779 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1005779 AND spawnType = 0 WHERE ce.spawnID = 1005779;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1005780 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1005780 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1005780 AND spawnType = 0 WHERE ce.spawnID = 1005780;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1005782 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1005782 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1005782 AND spawnType = 0 WHERE ce.spawnID = 1005782;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1005784 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1005784 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1005784 AND spawnType = 0 WHERE ce.spawnID = 1005784;
INSERT INTO creature_entry (spawnID, entry) VALUES (5813, 8402);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (5813, 0, 1, 7756, 1, -6432.58, -1215.31,  180.799, 0.929468, 500, 0, 0, 2399, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (5813, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO creature_entry (spawnID, entry) VALUES (5811, 8402);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (5811, 0, 1, 7753, 1, -6454.82, -1196.93,  180.932, 1.44846, 500, 0, 0, 2399, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (5811, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO creature_entry (spawnID, entry) VALUES (5809, 8402);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (5809, 0, 1, 7751, 1, -6455.95, -1247.35,  180.442, 4.55847, 500, 0, 0, 2399, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (5809, NULL, 0, 0, 0, 4097, 69, NULL);
INSERT INTO creature_entry (spawnID, entry) VALUES (5807, 8402);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (5807, 0, 1, 7756, 1, -6452.48, -1258.55,  180.688, 2.74524, 500, 0, 0, 2399, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (5807, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO creature_entry (spawnID, entry) VALUES (5806, 8402);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (5806, 0, 1, 7754, 1, -6463.98, -1249.29,  180.455, 5.08271, 500, 0, 0, 2399, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (5806, NULL, 0, 0, 0, 4097, 69, NULL);
INSERT INTO creature_entry (spawnID, entry) VALUES (5804, 8402);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (5804, 0, 1, 7756, 1, -6499.06, -1212.11,  180.352, 2.28919, 500, 0, 0, 2399, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (5804, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 5811 with import type REPLACE_ALL

-- Importing creature with guid 5809 with import type REPLACE_ALL

-- Importing creature with guid 5807 with import type REPLACE_ALL

-- Importing creature with guid 5806 with import type REPLACE_ALL

-- Importing creature with guid 5804 with import type REPLACE_ALL

-- Importing creature with guid 5799 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1005772 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1005772 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1005772 AND spawnType = 0 WHERE ce.spawnID = 1005772;
INSERT INTO creature_entry (spawnID, entry) VALUES (5799, 8400);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (5799, 0, 1, 13929, 0, -6461.91, -1254.6,  180.578, 3.54302, 500, 0, 0, 2878, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (5799, NULL, 0, 0, 7, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 5799);

-- Importing creature with guid 45501 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (45501, 8666);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (45501, 0, 1, 0, 0, -8634.51, 918.961,  99.355, 3.83299, 5400, 0, 0, 102, 0, 2, 0, 5, 10);
INSERT INTO waypoint_data VALUES 
(455010, 1, -8641.4, 912.342, 99.1397, 0, 0, 0, NULL, 100, 0),
(455010, 2, -8661.71, 894.74, 97.6239, 0, 0, 0, NULL, 100, 0),
(455010, 3, -8679.15, 880.967, 97.0168, 0, 0, 0, NULL, 100, 0),
(455010, 4, -8681.12, 877.654, 97.0168, 0, 0, 0, NULL, 100, 0),
(455010, 5, -8679.29, 873.082, 97.0168, 0, 0, 0, NULL, 100, 0),
(455010, 6, -8659.98, 849.329, 97.0168, 0, 0, 0, NULL, 100, 0),
(455010, 7, -8639.96, 825.073, 96.6251, 0, 0, 0, NULL, 100, 0),
(455010, 8, -8636.74, 813.025, 96.6486, 0, 0, 0, NULL, 100, 0),
(455010, 9, -8634, 793.001, 96.6508, 0, 0, 0, NULL, 100, 0),
(455010, 10, -8635.94, 785.58, 96.6515, 0, 0, 0, NULL, 100, 0),
(455010, 11, -8651.43, 775.162, 96.6714, 0, 0, 0, NULL, 100, 0),
(455010, 12, -8661.39, 764.974, 96.6998, 0, 0, 0, NULL, 100, 0),
(455010, 13, -8662.58, 758.134, 96.6947, 0, 0, 0, NULL, 100, 0),
(455010, 14, -8647.73, 738.576, 96.6965, 0, 0, 0, NULL, 100, 0),
(455010, 15, -8630.74, 726.606, 96.7377, 0, 0, 0, NULL, 100, 0),
(455010, 16, -8618.88, 711.997, 96.7248, 0, 0, 0, NULL, 100, 0),
(455010, 17, -8614.67, 709.545, 96.7549, 0, 0, 0, NULL, 100, 0),
(455010, 18, -8606.13, 711.345, 96.7382, 0, 0, 0, NULL, 100, 0),
(455010, 19, -8598.07, 712.945, 96.6746, 0, 0, 0, NULL, 100, 0),
(455010, 20, -8588.25, 706.887, 97.0168, 0, 0, 0, NULL, 100, 0),
(455010, 21, -8566.09, 678.512, 97.0168, 0, 0, 0, NULL, 100, 0),
(455010, 22, -8561.86, 674.735, 97.0168, 0, 0, 0, NULL, 100, 0),
(455010, 23, -8556.46, 676.784, 97.0168, 0, 0, 0, NULL, 100, 0),
(455010, 24, -8542.79, 686.774, 97.6239, 0, 0, 0, NULL, 100, 0),
(455010, 25, -8536.45, 686.854, 97.6775, 0, 0, 0, NULL, 100, 0),
(455010, 26, -8531.64, 683.194, 98.4422, 0, 0, 0, NULL, 100, 0),
(455010, 27, -8524.58, 673.178, 102.5, 0, 0, 0, NULL, 100, 0),
(455010, 28, -8519.8, 666.4, 102.615, 0, 0, 0, NULL, 100, 0),
(455010, 29, -8512.94, 656.648, 100.901, 0, 0, 0, NULL, 100, 0),
(455010, 30, -8513.15, 648.714, 100.292, 0, 0, 0, NULL, 100, 0),
(455010, 31, -8518.18, 642.361, 100.092, 0, 0, 0, NULL, 100, 0),
(455010, 32, -8538.04, 630.723, 100.404, 0, 0, 0, NULL, 100, 0),
(455010, 33, -8554.03, 617.81, 102.053, 0, 0, 0, NULL, 100, 0),
(455010, 34, -8564.5, 613.48, 102.435, 0, 0, 0, NULL, 100, 0),
(455010, 35, -8576.12, 601.799, 103.26, 0, 0, 0, NULL, 100, 0),
(455010, 36, -8582.44, 589.572, 103.691, 0, 0, 0, NULL, 100, 0),
(455010, 37, -8586.68, 575.605, 102.985, 0, 0, 0, NULL, 100, 0),
(455010, 38, -8585.96, 565.941, 102.26, 0, 0, 0, NULL, 100, 0),
(455010, 39, -8578.9, 545.988, 101.779, 0, 0, 0, NULL, 100, 0),
(455010, 40, -8581.73, 541.012, 102.09, 0, 0, 0, NULL, 100, 0),
(455010, 41, -8590.09, 533.912, 104.76, 0, 0, 0, NULL, 100, 0),
(455010, 42, -8598.32, 527.164, 106.399, 0, 0, 0, NULL, 100, 0),
(455010, 43, -8605.67, 520.882, 105.748, 0, 0, 0, NULL, 100, 0),
(455010, 44, -8610.26, 515.735, 103.79, 0, 0, 0, NULL, 100, 0),
(455010, 45, -8613.43, 514.684, 103.401, 0, 0, 0, NULL, 100, 0),
(455010, 46, -8618.8, 518.794, 103.068, 0, 0, 0, NULL, 100, 0),
(455010, 47, -8635.17, 535.152, 99.9833, 0, 0, 0, NULL, 100, 0),
(455010, 48, -8647.39, 546.721, 97.8568, 0, 0, 0, NULL, 100, 0),
(455010, 49, -8655.78, 552.938, 96.9435, 0, 0, 0, NULL, 100, 0),
(455010, 50, -8671.86, 552.874, 97.2037, 0, 0, 0, NULL, 100, 0),
(455010, 51, -8679.66, 549.654, 97.5031, 0, 0, 0, NULL, 100, 0),
(455010, 52, -8689.63, 540.268, 97.828, 0, 0, 0, NULL, 100, 0),
(455010, 53, -8698.98, 530.295, 97.7173, 0, 0, 0, NULL, 100, 0),
(455010, 54, -8712.64, 520.242, 97.2398, 0, 0, 0, NULL, 100, 0),
(455010, 55, -8715.24, 521.571, 97.4039, 0, 0, 0, NULL, 100, 0),
(455010, 56, -8720.77, 528.729, 99.1496, 0, 0, 0, NULL, 100, 0),
(455010, 57, -8729.84, 539.87, 101.105, 0, 0, 0, NULL, 100, 0),
(455010, 58, -8735.95, 547.101, 100.845, 0, 0, 0, NULL, 100, 0),
(455010, 59, -8745.79, 557.737, 97.7107, 0, 0, 0, NULL, 100, 0),
(455010, 60, -8746.01, 564.915, 97.4001, 0, 0, 0, NULL, 100, 0),
(455010, 61, -8729.92, 581.294, 97.6775, 0, 0, 0, NULL, 100, 0),
(455010, 62, -8719.58, 591.033, 98.4713, 0, 0, 0, NULL, 100, 0),
(455010, 63, -8712.04, 594.001, 98.6079, 0, 0, 0, NULL, 100, 0),
(455010, 64, -8707.26, 600.676, 98.9982, 0, 0, 0, NULL, 100, 0),
(455010, 65, -8704.46, 616.407, 100.215, 0, 0, 0, NULL, 100, 0),
(455010, 66, -8705.6, 629.078, 100.477, 0, 0, 0, NULL, 100, 0),
(455010, 67, -8708.67, 645.787, 99.9994, 0, 0, 0, NULL, 100, 0),
(455010, 68, -8716.46, 666.585, 98.8681, 0, 0, 0, NULL, 100, 0),
(455010, 69, -8724.09, 676.482, 98.6317, 0, 0, 0, NULL, 100, 0),
(455010, 70, -8728.54, 684.167, 98.7324, 0, 0, 0, NULL, 100, 0),
(455010, 71, -8733.47, 695.151, 98.723, 0, 0, 0, NULL, 100, 0),
(455010, 72, -8743.6, 709.876, 98.2678, 0, 0, 0, NULL, 100, 0),
(455010, 73, -8741.08, 714.561, 98.9815, 0, 0, 0, NULL, 100, 0),
(455010, 74, -8734.46, 720.119, 101.647, 0, 0, 0, NULL, 100, 0),
(455010, 75, -8726.79, 726.231, 100.924, 0, 0, 0, NULL, 100, 0),
(455010, 76, -8718.09, 733.687, 97.9511, 0, 0, 0, NULL, 100, 0),
(455010, 77, -8716.42, 737.269, 97.7782, 0, 0, 0, NULL, 100, 0),
(455010, 78, -8721.01, 746.752, 97.9693, 0, 0, 0, NULL, 100, 0),
(455010, 79, -8730.96, 759.107, 98.1572, 0, 0, 0, NULL, 100, 0),
(455010, 80, -8731.99, 769.385, 98.0161, 0, 0, 0, NULL, 100, 0),
(455010, 81, -8724.64, 778.108, 98.0604, 0, 0, 0, NULL, 100, 0),
(455010, 82, -8717.55, 792.762, 97.1197, 0, 0, 0, NULL, 100, 0),
(455010, 83, -8717.68, 798.804, 97.1792, 0, 0, 0, NULL, 100, 0),
(455010, 84, -8728.3, 817.744, 96.9777, 0, 0, 0, NULL, 100, 0),
(455010, 85, -8726.79, 830.504, 96.3102, 0, 0, 0, NULL, 100, 0),
(455010, 86, -8723.42, 841.35, 96.0764, 0, 0, 0, NULL, 100, 0),
(455010, 87, -8709.57, 857.779, 96.978, 0, 0, 0, NULL, 100, 0),
(455010, 88, -8692.38, 870.557, 97.0284, 0, 0, 0, NULL, 100, 0),
(455010, 89, -8679.35, 880.974, 97.0167, 0, 0, 0, NULL, 100, 0),
(455010, 90, -8661.22, 896.239, 97.5968, 0, 0, 0, NULL, 100, 0),
(455010, 91, -8643.7, 912.233, 98.9288, 0, 0, 0, NULL, 100, 0),
(455010, 92, -8634.58, 918.926, 99.3551, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (45501, 455010, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 579 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (579, 7389);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (579, 0, 1, 6190, 0, -14336, 427.171,  6.64303, 5.72989, 300, 0, 0, 42, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (579, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 384 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1000379 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1000379 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1000379 AND spawnType = 0 WHERE ce.spawnID = 1000379;
INSERT INTO creature_entry (spawnID, entry) VALUES (384, 7802);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (384, 0, 1, 6846, 1, -12033.4, -1003.86,  49.7795, 3.91088, 300, 0, 0, 4120, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (384, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 15312 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1015201 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1015201 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1015201 AND spawnType = 0 WHERE ce.spawnID = 1015201;
INSERT INTO creature_entry (spawnID, entry) VALUES (15312, 3542);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (15312, 0, 1, 3699, 1, -859.126, -559.593,  18.6293, 4.34587, 300, 0, 0, 1753, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (15312, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 314 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1000309 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1000309 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1000309 AND spawnType = 0 WHERE ce.spawnID = 1000309;
INSERT INTO creature_entry (spawnID, entry) VALUES (314, 14911);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (314, 0, 1, 15004, 1, -11834.7, 1257.75,  2.15351, 5.56803, 1800, 0, 0, 7326, 2434, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (314, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 73429 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (73429, 815);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (73429, 0, 1, 0, 1, -11662.9, -637.447,  30.0416, 0.000360489, 300, 0, 0, 993, 2680, 0, 0, 0, 10);

-- Importing creature with guid 23427 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (23427, 7386);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (23427, 0, 1, 0, 0, -8632.05, 921.279,  99.3828, 3.8978, 180, 0, 0, 42, 0, 0, 0, 5, 10);

-- Importing creature with guid 34036 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (34036, 15561);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (34036, 0, 1, 0, 0, 514.754, 1560.97,  130.145, 4.39823, 25, 0, 0, 2563, 5751, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (34036, 7);

-- Importing creature with guid 34035 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (34035, 15567);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (34035, 0, 1, 0, 0, -7274.65, -801.765,  296.871, 6.14681, 25, 0, 0, 2563, 5751, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (34035, 7);

-- Importing creature with guid 134515 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (134515, 8976);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (134515, 0, 1, 0, 0, -8203.09, -922.076,  133.28, 0.964897, 151200, 5, 0, 12210, 4868, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(1345150, 1, -8198.23, -917.961, 132.864, 0, 0, 0, NULL, 100, 0),
(1345150, 2, -8189.78, -894.28, 132.904, 0, 0, 0, NULL, 100, 0),
(1345150, 3, -8188.53, -850.906, 131.776, 0, 0, 0, NULL, 100, 0),
(1345150, 4, -8187.55, -816.536, 129.697, 0, 0, 0, NULL, 100, 0),
(1345150, 5, -8166.84, -790.365, 129.848, 0, 0, 0, NULL, 100, 0),
(1345150, 6, -8131.38, -768.808, 130.859, 0, 0, 0, NULL, 100, 0),
(1345150, 7, -8102.34, -775.695, 131.46, 0, 0, 0, NULL, 100, 0),
(1345150, 8, -8022.56, -800.541, 131.608, 0, 0, 0, NULL, 100, 0),
(1345150, 9, -7986.66, -829.885, 130.545, 0, 0, 0, NULL, 100, 0),
(1345150, 10, -7978.98, -852.116, 131.678, 0, 0, 0, NULL, 100, 0),
(1345150, 11, -7968.91, -881.28, 128.998, 0, 0, 0, NULL, 100, 0),
(1345150, 12, -7984.89, -904.946, 129.484, 0, 0, 0, NULL, 100, 0),
(1345150, 13, -7993.79, -934.841, 129.321, 0, 0, 0, NULL, 100, 0),
(1345150, 14, -8039.18, -1086.67, 131.087, 0, 0, 0, NULL, 100, 0),
(1345150, 15, -8053.88, -1148.67, 131.087, 0, 0, 0, NULL, 100, 0),
(1345150, 16, -8076.81, -1189.43, 131.642, 0, 0, 0, NULL, 100, 0),
(1345150, 17, -8094.72, -1230.07, 132.802, 0, 0, 0, NULL, 100, 0),
(1345150, 18, -8117.75, -1237.46, 133.074, 0, 0, 0, NULL, 100, 0),
(1345150, 19, -8143.38, -1219.68, 133.111, 0, 0, 0, NULL, 100, 0),
(1345150, 20, -8157.7, -1206.47, 134.896, 0, 0, 0, NULL, 100, 0),
(1345150, 21, -8159.24, -1193.96, 136.698, 0, 0, 0, NULL, 100, 0),
(1345150, 22, -8152.86, -1167.79, 135.585, 0, 0, 0, NULL, 100, 0),
(1345150, 23, -8150.15, -1140.2, 135.762, 0, 0, 0, NULL, 100, 0),
(1345150, 24, -8145.04, -1088.04, 132.858, 0, 0, 0, NULL, 100, 0),
(1345150, 25, -8142.54, -1062.51, 132.316, 0, 0, 0, NULL, 100, 0),
(1345150, 26, -8146.18, -1022.53, 133.425, 0, 0, 0, NULL, 100, 0),
(1345150, 27, -8169.36, -998.282, 135.425, 0, 0, 0, NULL, 100, 0),
(1345150, 28, -8194.4, -964.901, 133.777, 0, 0, 0, NULL, 100, 0),
(1345150, 29, -8197.23, -939.184, 133.549, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (134515, 1345150, 0, 0, 0, 0, 0, NULL);

-- Importing creature with guid 79668 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1078735 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1078735 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1078735 AND spawnType = 0 WHERE ce.spawnID = 1078735;
INSERT INTO creature_entry (spawnID, entry) VALUES (79668, 482);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (79668, 0, 1, 0, 1, -8854.12, 575.652,  101.15, 4.11898, 310, 0, 0, 1003, 0, 0, 0, 0, 10);

-- Importing creature with guid 79672 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1078737 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1078737 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1078737 AND spawnType = 0 WHERE ce.spawnID = 1078737;
INSERT INTO creature_entry (spawnID, entry) VALUES (79672, 1286);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (79672, 0, 1, 0, 0, -8821.22, 598.249,  95.6618, 2.37365, 310, 0, 0, 1003, 0, 0, 0, 0, 10);

-- Importing creature with guid 79746 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1078808 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1078808 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1078808 AND spawnType = 0 WHERE ce.spawnID = 1078808;
INSERT INTO creature_entry (spawnID, entry) VALUES (79746, 3626);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (79746, 0, 1, 0, 0, -8692.72, 397.438,  101.37, 5.51888, 370, 0, 0, 102, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(797460, 1, -8703.23, 406.059, 99.8461, 0, 0, 0, NULL, 100, 0),
(797460, 2, -8712.55, 405.839, 98.2215, 0, 0, 0, NULL, 100, 0),
(797460, 3, -8724.25, 407.106, 97.652, 0, 0, 0, NULL, 100, 0),
(797460, 4, -8726.03, 415.403, 97.527, 0, 0, 0, NULL, 100, 0),
(797460, 5, -8723.28, 424.437, 97.8376, 0, 0, 0, NULL, 100, 0),
(797460, 6, -8721.03, 435.953, 97.4688, 0, 0, 0, NULL, 100, 0),
(797460, 7, -8716.93, 446.793, 97.0539, 0, 0, 0, NULL, 100, 0),
(797460, 8, -8713.91, 455.698, 97.0791, 0, 0, 0, NULL, 100, 0),
(797460, 9, -8705.39, 463.781, 96.0151, 0, 0, 0, NULL, 100, 0),
(797460, 10, -8682.33, 483.703, 96.2082, 0, 0, 0, NULL, 100, 0),
(797460, 11, -8678.09, 490.699, 97.9041, 0, 0, 0, NULL, 100, 0),
(797460, 12, -8666.33, 496.996, 100.347, 0, 0, 0, NULL, 100, 0),
(797460, 13, -8659.76, 494.769, 100.87, 0, 0, 0, NULL, 100, 0),
(797460, 14, -8652, 485.892, 102.315, 0, 0, 0, NULL, 100, 0),
(797460, 15, -8637.63, 467.013, 102.253, 0, 0, 0, NULL, 100, 0),
(797460, 16, -8632.63, 459.051, 102.244, 0, 0, 0, NULL, 100, 0),
(797460, 17, -8632.17, 446.828, 102.308, 0, 0, 0, NULL, 100, 0),
(797460, 18, -8639.18, 438.867, 101.756, 0, 0, 0, NULL, 100, 0),
(797460, 19, -8645.07, 427.67, 101.423, 0, 0, 0, NULL, 100, 0),
(797460, 20, -8651.61, 418.216, 101.98, 0, 0, 0, NULL, 100, 0),
(797460, 21, -8665.2, 409.606, 102.914, 0, 0, 0, NULL, 100, 0),
(797460, 22, -8680.51, 399.868, 102.766, 0, 0, 0, NULL, 100, 0),
(797460, 23, -8692.72, 397.438, 101.37, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (79746, 797460, 0, 0, 0, 257, 0, NULL);

-- Importing creature with guid 79770 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1078832 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1078832 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1078832 AND spawnType = 0 WHERE ce.spawnID = 1078832;
INSERT INTO creature_entry (spawnID, entry) VALUES (79770, 1721);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (79770, 0, 1, 0, 0, -8696.33, 391.153,  102.258, 1.64061, 180, 0, 0, 102, 0, 2, 0, 0, 10);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1182, 1, 30, 0, 0, 0, 0, 0, 0, 2.86234);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1183, 1, 30, 0, 0, 0, 0, 0, 0, 1.64061);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1184, 1, 30, 0, 0, 0, 0, 0, 0, 3.50811);
INSERT INTO waypoint_data VALUES 
(797700, 1, -8698.37, 401.569, 101.036, 0, 0, 0, NULL, 100, 0),
(797700, 2, -8717.3, 406.85, 97.8757, 0, 0, 0, NULL, 100, 0),
(797700, 3, -8724.67, 406.652, 97.6569, 0, 0, 0, NULL, 100, 0),
(797700, 4, -8733.15, 394.315, 98.2072, 0, 60000, 0, 1182, 100, 0),
(797700, 5, -8732.05, 394.275, 98.2072, 0, 0, 0, NULL, 100, 0),
(797700, 6, -8726.09, 405.419, 97.759, 0, 0, 0, NULL, 100, 0),
(797700, 7, -8724.3, 422.966, 97.8733, 0, 0, 0, NULL, 100, 0),
(797700, 8, -8715.03, 455.983, 97.1363, 0, 0, 0, NULL, 100, 0),
(797700, 9, -8681.31, 485.258, 96.6978, 0, 0, 0, NULL, 100, 0),
(797700, 10, -8666.83, 497.894, 100.43, 0, 0, 0, NULL, 100, 0),
(797700, 11, -8652.38, 486.102, 102.261, 0, 0, 0, NULL, 100, 0),
(797700, 12, -8636.97, 465.872, 102.235, 0, 0, 0, NULL, 100, 0),
(797700, 13, -8632.35, 456.593, 102.26, 0, 0, 0, NULL, 100, 0),
(797700, 14, -8638.79, 450.024, 102.389, 0, 60000, 0, 1183, 100, 0),
(797700, 15, -8639.3, 451.104, 102.395, 0, 0, 0, NULL, 100, 0),
(797700, 16, -8633.5, 442.8, 102.209, 0, 0, 0, NULL, 100, 0),
(797700, 17, -8640.33, 437.125, 101.617, 0, 0, 0, NULL, 100, 0),
(797700, 18, -8646.86, 427.444, 101.366, 0, 0, 0, NULL, 100, 0),
(797700, 19, -8655.47, 420.304, 102.237, 0, 0, 0, NULL, 100, 0),
(797700, 20, -8665.62, 411.493, 103.009, 0, 0, 0, NULL, 100, 0),
(797700, 21, -8670.61, 403.653, 103.321, 0, 0, 0, NULL, 100, 0),
(797700, 22, -8680.94, 400.982, 102.882, 0, 0, 0, NULL, 100, 0),
(797700, 23, -8687.24, 395.868, 101.947, 0, 0, 0, NULL, 100, 0),
(797700, 24, -8693.1, 396.712, 101.469, 0, 0, 0, NULL, 100, 0),
(797700, 25, -8696.33, 391.153, 102.258, 0, 60000, 0, 1184, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (79770, 797700, 0, 0, 0, 257, 0, NULL);

-- Importing creature with guid 79815 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (79815, 1368);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (79815, 0, 1, 0, 1, -8582.03, 633.634,  96.338, 4.98933, 180, 0, 0, 42, 0, 2, 0, 5, 10);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (151, 7, 'They say he can turn into a raven sometimes.', 'They say he can turn into a raven sometimes.', 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (487, 0, 0, 0, 0, 151, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (488, 0, 0, 0, 0, 151, 0, 0, 0, 0);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (155, 7, 'And then the rabbit just bit his head off... I swear.', 'And then the rabbit just bit his head off... I swear.', 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (489, 0, 0, 0, 0, 155, 0, 0, 0, 0);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (154, 7, 'I swear, people have actually seen them. Pandaren really do exist!', 'I swear, people have actually seen them. Pandaren really do exist!', 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (490, 0, 0, 0, 0, 154, 0, 0, 0, 0);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (152, 7, 'You know why orc eyes glow red? It\'s because they drink blood!', 'You know why orc eyes glow red? It\'s because they drink blood!', 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (491, 0, 0, 0, 0, 152, 0, 0, 0, 0);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (177, 7, 'I don\'t think there\'s any fish in these canals.', 'I don\'t think there\'s any fish in these canals.', 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (492, 0, 0, 0, 0, 177, 0, 0, 0, 0);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (149, 7, 'And that\'s how Lothar killed thirty six orcs with his bare hands!', 'And that\'s how Lothar killed thirty six orcs with his bare hands!', 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (493, 0, 0, 0, 0, 149, 0, 0, 0, 0);
INSERT INTO waypoint_data VALUES 
(798150, 1, -8582.03, 633.634, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798150, 2, -8582.03, 633.634, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798150, 3, -8582.03, 633.634, 96.3386, 0, 10000, 0, 487, 100, 0),
(798150, 4, -8582.03, 633.634, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798150, 5, -8582.03, 633.634, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798150, 6, -8582.03, 633.634, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798150, 7, -8582.03, 633.634, 96.3386, 0, 10000, 0, 488, 100, 0),
(798150, 8, -8582.03, 633.634, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798150, 9, -8582.03, 633.634, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798150, 10, -8582.03, 633.634, 96.3386, 0, 10000, 0, 489, 100, 0),
(798150, 11, -8582.03, 633.634, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798150, 12, -8582.03, 633.634, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798150, 13, -8582.03, 633.634, 96.3386, 0, 10000, 0, 490, 100, 0),
(798150, 14, -8582.03, 633.634, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798150, 15, -8582.03, 633.634, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798150, 16, -8582.03, 633.634, 96.3386, 0, 10000, 0, 491, 100, 0),
(798150, 17, -8582.03, 633.634, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798150, 18, -8582.03, 633.634, 96.3386, 0, 10000, 0, 492, 100, 0),
(798150, 19, -8582.03, 633.634, 96.3386, 0, 10000, 0, 493, 100, 0),
(798150, 20, -8582.03, 633.634, 96.3386, 0, 10000, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (79815, 798150, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 79816 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (79816, 1371);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (79816, 0, 1, 0, 1, -8583.88, 633.127,  96.338, 4.98933, 180, 0, 0, 42, 0, 2, 0, 5, 10);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (173, 7, 'Eww... that\'s not a fish!', 'Eww... that\'s not a fish!', 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (494, 0, 0, 0, 0, 173, 0, 0, 0, 0);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (174, 7, 'I got worm guts on my shoes.', 'I got worm guts on my shoes.', 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (495, 0, 0, 0, 0, 174, 0, 0, 0, 0);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (172, 7, 'I think I see something.', 'I think I see something.', 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (496, 0, 0, 0, 0, 172, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (497, 0, 0, 0, 0, 173, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (498, 0, 0, 0, 0, 172, 0, 0, 0, 0);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (176, 7, 'I hope that was a fish!', 'I hope that was a fish!', 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (499, 0, 0, 0, 0, 176, 0, 0, 0, 0);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (178, 7, 'Worm goes on the hook, hook goes in the water. Fish is in the water, our fish.', 'Worm goes on the hook, hook goes in the water. Fish is in the water, our fish.', 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (500, 0, 0, 0, 0, 178, 0, 0, 0, 0);
INSERT INTO waypoint_data VALUES 
(798160, 1, -8583.88, 633.127, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798160, 2, -8583.88, 633.127, 96.3386, 0, 10000, 0, 494, 100, 0),
(798160, 3, -8583.88, 633.127, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798160, 4, -8583.88, 633.127, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798160, 5, -8583.88, 633.127, 96.3386, 0, 10000, 0, 495, 100, 0),
(798160, 6, -8583.88, 633.127, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798160, 7, -8583.88, 633.127, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798160, 8, -8583.88, 633.127, 96.3386, 0, 10000, 0, 496, 100, 0),
(798160, 9, -8583.88, 633.127, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798160, 10, -8583.88, 633.127, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798160, 11, -8583.88, 633.127, 96.3386, 0, 10000, 0, 497, 100, 0),
(798160, 12, -8583.88, 633.127, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798160, 13, -8583.88, 633.127, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798160, 14, -8583.88, 633.127, 96.3386, 0, 10000, 0, 498, 100, 0),
(798160, 15, -8583.88, 633.127, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798160, 16, -8583.88, 633.127, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798160, 17, -8583.88, 633.127, 96.3386, 0, 10000, 0, 499, 100, 0),
(798160, 18, -8583.88, 633.127, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798160, 19, -8583.88, 633.127, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798160, 20, -8583.88, 633.127, 96.3386, 0, 10000, 0, 500, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (79816, 798160, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 79817 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (79817, 1370);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (79817, 0, 1, 0, 1, -8580.51, 635.108,  96.338, 4.98933, 180, 0, 0, 42, 0, 2, 0, 5, 10);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (169, 7, 'Can you imagine?', 'Can you imagine?', 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (501, 0, 0, 0, 0, 169, 0, 0, 0, 0);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (147, 7, 'Really?', 'Really?', 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (502, 0, 0, 0, 0, 147, 0, 0, 0, 0);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (157, 7, 'Wow.', 'Wow.', 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (503, 0, 0, 0, 0, 157, 0, 0, 0, 0);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (168, 7, 'My father says that\'s just a story.', 'My father says that\'s just a story.', 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (504, 0, 0, 0, 0, 168, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (505, 0, 0, 0, 0, 147, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (506, 0, 0, 0, 0, 157, 0, 0, 0, 0);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (167, 7, 'That\'s neat.', 'That\'s neat.', 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (507, 0, 0, 0, 0, 167, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (508, 0, 0, 0, 0, 157, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (509, 0, 0, 0, 0, 147, 0, 0, 0, 0);
INSERT INTO waypoint_data VALUES 
(798170, 1, -8580.51, 635.108, 96.3386, 0, 10000, 0, 501, 100, 0),
(798170, 2, -8580.51, 635.108, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798170, 3, -8580.51, 635.108, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798170, 4, -8580.51, 635.108, 96.3386, 0, 10000, 0, 502, 100, 0),
(798170, 5, -8580.51, 635.108, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798170, 6, -8580.51, 635.108, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798170, 7, -8580.51, 635.108, 96.3386, 0, 10000, 0, 503, 100, 0),
(798170, 8, -8580.51, 635.108, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798170, 9, -8580.51, 635.108, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798170, 10, -8580.51, 635.108, 96.3386, 0, 10000, 0, 504, 100, 0),
(798170, 11, -8580.51, 635.108, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798170, 12, -8580.51, 635.108, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798170, 13, -8580.51, 635.108, 96.3386, 0, 10000, 0, 505, 100, 0),
(798170, 14, -8580.51, 635.108, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798170, 15, -8580.51, 635.108, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798170, 16, -8580.51, 635.108, 96.3386, 0, 10000, 0, 506, 100, 0),
(798170, 17, -8580.51, 635.108, 96.3386, 0, 10000, 0, 507, 100, 0),
(798170, 18, -8580.51, 635.108, 96.3386, 0, 10000, 0, NULL, 100, 0),
(798170, 19, -8580.51, 635.108, 96.3386, 0, 10000, 0, 508, 100, 0),
(798170, 20, -8580.51, 635.108, 96.3386, 0, 10000, 0, 509, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (79817, 798170, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 79942 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 79942 WHERE entry = 823;
UPDATE conditions SET ConditionValue3 = 79942 WHERE ConditionValue3 = 1079000 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -79942 WHERE SourceEntry = -1079000 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -79942 WHERE entryorguid = -1079000 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 79942 WHERE target_param1 = 1079000 AND target_type = 10;
UPDATE spawn_group SET spawnID = 79942 WHERE spawnID = 1079000 AND spawnType = 0;

-- Importing creature with guid 79949 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (79949, 11940);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (79949, 0, 1, 0, 0, -9478.9, 49.9652,  57.313, 6.1665, 180, 0, 0, 102, 0, 0, 0, 5, 10);

-- Importing creature with guid 80848 with import type REPLACE_ALL
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1079899 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1079899 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1079899 AND spawnType = 0 WHERE ce.spawnID = 1079899;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1079901 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1079901 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1079901 AND spawnType = 0 WHERE ce.spawnID = 1079901;
INSERT INTO creature_entry (spawnID, entry) VALUES (80848, 390);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (80848, 0, 1, 0, 0, -9781.22, -881.556,  39.5677, 3.7601, 180, 0, 0, 137, 0, 0, 0, 0, 10);
INSERT INTO creature_entry (spawnID, entry) VALUES (80850, 390);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (80850, 0, 1, 0, 0, -9785.44, -883.454,  39.6473, 3.76008, 180, 0, 0, 137, 0, 0, 0, 0, 10);

-- Importing creature with guid 80849 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1079900 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1079900 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1079900 AND spawnType = 0 WHERE ce.spawnID = 1079900;
INSERT INTO creature_entry (spawnID, entry) VALUES (80849, 330);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (80849, 0, 1, 0, 0, -9785.24, -880.61,  39.6131, 3.75995, 180, 0, 0, 176, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(808490, 1, -9785.24, -880.61, 39.6131, 0, 0, 0, NULL, 100, 0),
(808490, 2, -9792.77, -872.876, 39.5751, 0, 0, 0, NULL, 100, 0),
(808490, 3, -9793.2, -867.623, 39.4678, 0, 0, 0, NULL, 100, 0),
(808490, 4, -9774.36, -847.313, 39.7919, 0, 0, 0, NULL, 100, 0),
(808490, 5, -9767.83, -847.814, 39.6107, 0, 0, 0, NULL, 100, 0),
(808490, 6, -9752.2, -863.369, 39.4772, 0, 0, 0, NULL, 100, 0),
(808490, 7, -9753.51, -871.917, 39.5511, 0, 0, 0, NULL, 100, 0),
(808490, 8, -9769.58, -886.229, 39.4867, 0, 0, 0, NULL, 100, 0),
(808490, 9, -9776.63, -887.336, 39.5384, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (80849, 808490, 0, 0, 0, 0, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 80849);

-- Importing creature with guid 80850 with import type REPLACE_ALL

-- Importing creature with guid 86156 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1084918 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1084918 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1084918 AND spawnType = 0 WHERE ce.spawnID = 1084918;
INSERT INTO creature_entry (spawnID, entry) VALUES (86156, 802);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86156, 0, 1, 0, 0, -9507.03, -1275.85,  44.1562, 5.16513, 939, 8, 0, 42, 0, 1, 0, 0, 10);

-- Importing creature with guid 86191 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (86191, 12047);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86191, 0, 1, 0, 1, -4492, -1582,  509.005, 3.935, 1290, 7, 0, 2614, 0, 1, 0, 0, 10);

-- Importing creature with guid 86354 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1085103 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1085103 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1085103 AND spawnType = 0 WHERE ce.spawnID = 1085103;
INSERT INTO creature_entry (spawnID, entry) VALUES (86354, 801);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86354, 0, 1, 0, 0, -9511.99, -1272.53,  43.669, 5.56961, 318, 8, 0, 42, 0, 1, 0, 0, 10);

-- Importing creature with guid 48204 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1047981 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1047981 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1047981 AND spawnType = 0 WHERE ce.spawnID = 1047981;
INSERT INTO creature_entry (spawnID, entry) VALUES (48204, 14447);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (48204, 0, 1, 0, 1, -10583.5, -4377.98,  3.36329, 5.89153, 100800, 15, 0, 2059, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(482040, 1, -10567.2, -4384.11, 2.47185, 0, 0, 0, NULL, 100, 0),
(482040, 2, -10523.7, -4397.76, 0.7824, 0, 0, 0, NULL, 100, 0),
(482040, 3, -10487.1, -4403.39, 1.83189, 0, 0, 0, NULL, 100, 0),
(482040, 4, -10474.4, -4409.14, 0.148833, 0, 0, 0, NULL, 100, 0),
(482040, 5, -10441, -4402.84, 1.78278, 0, 0, 0, NULL, 100, 0),
(482040, 6, -10408.1, -4412.25, 1.53543, 0, 0, 0, NULL, 100, 0),
(482040, 7, -10354.7, -4410.85, 0.640205, 0, 0, 0, NULL, 100, 0),
(482040, 8, -10278.6, -4393.26, 0.269319, 0, 0, 0, NULL, 100, 0),
(482040, 9, -10246.8, -4376.13, 1.62744, 0, 0, 0, NULL, 100, 0),
(482040, 10, -10213.6, -4366.42, 2.41287, 0, 0, 0, NULL, 100, 0),
(482040, 11, -10167.5, -4353.14, 0.781765, 0, 0, 0, NULL, 100, 0),
(482040, 12, -10096.4, -4320.95, 0.610327, 0, 0, 0, NULL, 100, 0),
(482040, 13, -10078.5, -4326.13, -0.652936, 0, 0, 0, NULL, 100, 0),
(482040, 14, -10030, -4304.91, 1.89858, 0, 0, 0, NULL, 100, 0),
(482040, 15, -9972.14, -4295.12, -0.531087, 0, 0, 0, NULL, 100, 0),
(482040, 16, -9948.26, -4278.76, 0.351839, 0, 0, 0, NULL, 100, 0),
(482040, 17, -9920.98, -4246.13, 1.18211, 0, 0, 0, NULL, 100, 0),
(482040, 18, -9885.57, -4218.42, 0.320862, 0, 0, 0, NULL, 100, 0),
(482040, 19, -9815.96, -4147.78, -1.36226, 0, 0, 0, NULL, 100, 0),
(482040, 20, -9828.67, -4108.46, 10.4992, 0, 0, 0, NULL, 100, 0),
(482040, 21, -9815.18, -4097.89, 9.4921, 0, 0, 0, NULL, 100, 0),
(482040, 22, -9799.58, -4085.66, 4.98271, 0, 0, 0, NULL, 100, 0),
(482040, 23, -9776.8, -4084.98, 1.94095, 0, 0, 0, NULL, 100, 0),
(482040, 24, -9740.16, -4048.67, -0.472428, 0, 0, 0, NULL, 100, 0),
(482040, 25, -9692.74, -3984.84, 0.17136, 0, 0, 0, NULL, 100, 0),
(482040, 26, -9691.69, -3945.27, 0.453949, 0, 0, 0, NULL, 100, 0),
(482040, 27, -9680.98, -3921.22, -0.278533, 0, 0, 0, NULL, 100, 0),
(482040, 28, -9680.98, -3921.22, -0.278533, 0, 0, 0, NULL, 100, 0),
(482040, 29, -9704.96, -3936.47, 4.25237, 0, 0, 0, NULL, 100, 0),
(482040, 30, -9699.35, -3940.94, 2.63714, 0, 0, 0, NULL, 100, 0),
(482040, 31, -9698.84, -3955.15, 2.025, 0, 0, 0, NULL, 100, 0),
(482040, 32, -9702.2, -3992.69, 2.44361, 0, 0, 0, NULL, 100, 0),
(482040, 33, -9706.03, -4005.98, 0.783066, 0, 0, 0, NULL, 100, 0),
(482040, 34, -9720.89, -4004.98, 2.87266, 0, 0, 0, NULL, 100, 0),
(482040, 35, -9746.07, -4043.45, 2.34027, 0, 0, 0, NULL, 100, 0),
(482040, 36, -9755.29, -4070.84, -1.53693, 0, 0, 0, NULL, 100, 0),
(482040, 37, -9784.94, -4085.1, 2.74481, 0, 0, 0, NULL, 100, 0),
(482040, 38, -9797.77, -4092.44, 3.27877, 0, 0, 0, NULL, 100, 0),
(482040, 39, -9813.65, -4095.45, 9.83706, 0, 0, 0, NULL, 100, 0),
(482040, 40, -9830.61, -4110.08, 10.2745, 0, 0, 0, NULL, 100, 0),
(482040, 41, -9838.29, -4123.31, 4.00854, 0, 0, 0, NULL, 100, 0),
(482040, 42, -9848.13, -4140.26, 3.21548, 0, 0, 0, NULL, 100, 0),
(482040, 43, -9869.95, -4162.86, 4.18876, 0, 0, 0, NULL, 100, 0),
(482040, 44, -9866.48, -4198.87, -0.400182, 0, 0, 0, NULL, 100, 0),
(482040, 45, -9912.63, -4210.96, 4.55649, 0, 0, 0, NULL, 100, 0),
(482040, 46, -9945.64, -4236.01, 3.92758, 0, 0, 0, NULL, 100, 0),
(482040, 47, -9979.29, -4250.89, 6.65143, 0, 0, 0, NULL, 100, 0),
(482040, 48, -9989.05, -4260.38, 4.31283, 0, 0, 0, NULL, 100, 0),
(482040, 49, -10013.9, -4271.83, 4.61698, 0, 0, 0, NULL, 100, 0),
(482040, 50, -10046.1, -4271.59, 5.91962, 0, 0, 0, NULL, 100, 0),
(482040, 51, -10065.4, -4300.91, 3.61137, 0, 0, 0, NULL, 100, 0),
(482040, 52, -10105.8, -4308.16, 2.96357, 0, 0, 0, NULL, 100, 0),
(482040, 53, -10149.6, -4316.03, 4.63064, 0, 0, 0, NULL, 100, 0),
(482040, 54, -10184, -4337.94, 4.29623, 0, 0, 0, NULL, 100, 0),
(482040, 55, -10222.4, -4348.53, 5.70684, 0, 0, 0, NULL, 100, 0),
(482040, 56, -10242.3, -4367.82, 2.61615, 0, 0, 0, NULL, 100, 0),
(482040, 57, -10270.6, -4360.97, 4.43074, 0, 0, 0, NULL, 100, 0),
(482040, 58, -10312.5, -4384.61, 3.91312, 0, 0, 0, NULL, 100, 0),
(482040, 59, -10370.6, -4377.75, 4.95789, 0, 0, 0, NULL, 100, 0),
(482040, 60, -10407.1, -4376.55, 5.97529, 0, 0, 0, NULL, 100, 0),
(482040, 61, -10434.2, -4370.46, 6.32136, 0, 0, 0, NULL, 100, 0),
(482040, 62, -10494.8, -4373.62, 5.90146, 0, 0, 0, NULL, 100, 0),
(482040, 63, -10532.7, -4362.45, 6.90338, 0, 0, 0, NULL, 100, 0),
(482040, 64, -10576.8, -4349.41, 4.64166, 0, 0, 0, NULL, 100, 0),
(482040, 65, -10612.4, -4347.68, 6.89719, 0, 0, 0, NULL, 100, 0),
(482040, 66, -10646.2, -4326.05, 6.54085, 0, 0, 0, NULL, 100, 0),
(482040, 67, -10676, -4320.51, 4.12031, 0, 0, 0, NULL, 100, 0),
(482040, 68, -10711.5, -4307.04, 1.46877, 0, 0, 0, NULL, 100, 0),
(482040, 69, -10729.4, -4303.37, -0.23472, 0, 0, 0, NULL, 100, 0),
(482040, 70, -10757.2, -4279.12, 2.95269, 0, 0, 0, NULL, 100, 0),
(482040, 71, -10780.7, -4249.31, 4.90815, 0, 0, 0, NULL, 100, 0),
(482040, 72, -10805.3, -4225.93, 6.52459, 0, 0, 0, NULL, 100, 0),
(482040, 73, -10839.9, -4192.93, 5.18837, 0, 0, 0, NULL, 100, 0),
(482040, 74, -10875, -4174.39, 5.26251, 0, 0, 0, NULL, 100, 0),
(482040, 75, -10905.4, -4149.85, 7.21389, 0, 0, 0, NULL, 100, 0),
(482040, 76, -10942.5, -4129.09, 7.89548, 0, 0, 0, NULL, 100, 0),
(482040, 77, -10978.1, -4104.8, 3.69468, 0, 0, 0, NULL, 100, 0),
(482040, 78, -11006.6, -4071.34, 4.19032, 0, 0, 0, NULL, 100, 0),
(482040, 79, -11048.1, -4068.54, 0.468027, 0, 0, 0, NULL, 100, 0),
(482040, 80, -11073.8, -4044.83, 0.542318, 0, 0, 0, NULL, 100, 0),
(482040, 81, -11083.1, -4013.82, 0.33779, 0, 0, 0, NULL, 100, 0),
(482040, 82, -11083.5, -3980.84, 0.468939, 0, 0, 0, NULL, 100, 0),
(482040, 83, -11078.7, -3963.82, 2.33889, 0, 0, 0, NULL, 100, 0),
(482040, 84, -11089.7, -3939.84, 0.184559, 0, 0, 0, NULL, 100, 0),
(482040, 85, -11094.1, -3930.21, 0.182695, 0, 0, 0, NULL, 100, 0),
(482040, 86, -11081.6, -3910.09, 1.63978, 0, 0, 0, NULL, 100, 0),
(482040, 87, -11085.5, -3895.02, 1.72853, 0, 0, 0, NULL, 100, 0),
(482040, 88, -11094.2, -3896.14, -0.019768, 0, 0, 0, NULL, 100, 0),
(482040, 89, -11099.1, -3889.36, -0.034751, 0, 0, 0, NULL, 100, 0),
(482040, 90, -11100.3, -3877.04, 1.54966, 0, 0, 0, NULL, 100, 0),
(482040, 91, -11103.1, -3871.73, 0.933718, 0, 0, 0, NULL, 100, 0),
(482040, 92, -11101.5, -3862.52, -1.62778, 0, 0, 0, NULL, 100, 0),
(482040, 93, -11093, -3857.37, 0.067524, 0, 0, 0, NULL, 100, 0),
(482040, 94, -11100, -3842.71, 0.572051, 0, 0, 0, NULL, 100, 0),
(482040, 95, -11100, -3829.91, 4.04081, 0, 0, 0, NULL, 100, 0),
(482040, 96, -11103.7, -3820.58, 6.14741, 0, 0, 0, NULL, 100, 0),
(482040, 97, -11110.5, -3815.49, 7.09592, 0, 0, 0, NULL, 100, 0),
(482040, 98, -11121.8, -3819.78, 7.12925, 0, 0, 0, NULL, 100, 0),
(482040, 99, -11112.8, -3825.41, 4.69881, 0, 0, 0, NULL, 100, 0),
(482040, 100, -11109.4, -3833.83, 2.06878, 0, 0, 0, NULL, 100, 0),
(482040, 101, -11105.8, -3849.03, -1.55209, 0, 0, 0, NULL, 100, 0),
(482040, 102, -11108.1, -3866.38, -1.55209, 0, 0, 0, NULL, 100, 0),
(482040, 103, -11105.7, -3879.48, -1.55209, 0, 0, 0, NULL, 100, 0),
(482040, 104, -11096.6, -3899.48, -1.52235, 0, 0, 0, NULL, 100, 0),
(482040, 105, -11083.5, -3908.41, 0.827534, 0, 0, 0, NULL, 100, 0),
(482040, 106, -11092.6, -3924.1, -0.077631, 0, 0, 0, NULL, 100, 0),
(482040, 107, -11096.1, -3930.15, -0.275966, 0, 0, 0, NULL, 100, 0),
(482040, 108, -11085.1, -3955.61, -0.107875, 0, 0, 0, NULL, 100, 0),
(482040, 109, -11085.9, -3984.66, -0.115067, 0, 0, 0, NULL, 100, 0),
(482040, 110, -11078.5, -4029.82, 1.11663, 0, 0, 0, NULL, 100, 0),
(482040, 111, -11068.9, -4050.07, 0.694047, 0, 0, 0, NULL, 100, 0),
(482040, 112, -11044.4, -4075.32, 0.624391, 0, 0, 0, NULL, 100, 0),
(482040, 113, -11047.4, -4108.33, 0.83627, 0, 0, 0, NULL, 100, 0),
(482040, 114, -11034.3, -4155.69, 0.391246, 0, 0, 0, NULL, 100, 0),
(482040, 115, -10997.3, -4175.31, -0.310164, 0, 0, 0, NULL, 100, 0),
(482040, 116, -10973.6, -4195.75, -0.020198, 0, 0, 0, NULL, 100, 0),
(482040, 117, -10944, -4202.99, 0.736565, 0, 0, 0, NULL, 100, 0),
(482040, 118, -10905.9, -4216.24, -0.696765, 0, 0, 0, NULL, 100, 0),
(482040, 119, -10875.4, -4218.58, 1.64649, 0, 0, 0, NULL, 100, 0),
(482040, 120, -10850.2, -4233.52, -0.663568, 0, 0, 0, NULL, 100, 0),
(482040, 121, -10809.7, -4250.52, 0.401708, 0, 0, 0, NULL, 100, 0),
(482040, 122, -10772.9, -4265.97, 0.213619, 0, 0, 0, NULL, 100, 0),
(482040, 123, -10759.2, -4285.28, 0.304047, 0, 0, 0, NULL, 100, 0),
(482040, 124, -10746.9, -4295.43, 0.033441, 0, 0, 0, NULL, 100, 0),
(482040, 125, -10723.4, -4306.58, -0.419961, 0, 0, 0, NULL, 100, 0),
(482040, 126, -10692.6, -4321.13, 0.014835, 0, 0, 0, NULL, 100, 0),
(482040, 127, -10673.3, -4334.28, 0.160222, 0, 0, 0, NULL, 100, 0),
(482040, 128, -10647.4, -4355.38, 0.798208, 0, 0, 0, NULL, 100, 0),
(482040, 129, -10617.3, -4368.1, 4.43837, 0, 0, 0, NULL, 100, 0),
(482040, 130, -10595.9, -4385.32, 0.489108, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (48204, 482040, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 66 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (66, 7390);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (66, 0, 1, 6191, 0, -14332.5, 425.102,  6.63462, 2.30658, 300, 0, 0, 42, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (66, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 89424 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1095733 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1095733 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1095733 AND spawnType = 0 WHERE ce.spawnID = 1095733;
INSERT INTO creature_entry (spawnID, entry) VALUES (89424, 13602);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (89424, 0, 1, 13644, 0, 235.035, -359.131,  161.38, 4.45075, 180, 0, 0, 3831, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (89424, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO game_event_creature (guid, event) VALUES (89424, 2);
INSERT INTO spawn_group VALUES (2, 0, 89424);

-- Importing creature with guid 10105 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1010072 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1010072 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1010072 AND spawnType = 0 WHERE ce.spawnID = 1010072;
INSERT INTO creature_entry (spawnID, entry) VALUES (10105, 345);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (10105, 0, 1, 703, 0, -9286.79, -1926.36,  72.0246, 2.34692, 300, 10, 0, 1139, 0, 1, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (10105, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 10105);

-- Importing creature with guid 45525 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1045317 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1045317 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1045317 AND spawnType = 0 WHERE ce.spawnID = 1045317;
INSERT INTO creature_entry (spawnID, entry) VALUES (45525, 550);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (45525, 0, 1, 2312, 1, -11025.1, 1459.74,  43.0861, 5.28786, 300, 0, 0, 300, 0, 2, 0, 0, 10);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (37, 7, 'The boss wants all hands on high alert.  The rusty anchor sinks tonight.', NULL, 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (455250, 0, 0, 1, 0, 37, 0, 0, 0, 0);
INSERT INTO broadcast_text (ID, Language, MaleText, FemaleText, EmoteID0, EmoteID1, EmoteID2, EmoteDelay0, EmoteDelay1, EmoteDelay2, SoundId, Unk1, Unk2, VerifiedBuild) VALUES (36, 7, 'Stonemasons. . .errr. . Defias be warned:  The rusty anchor sinks tonight.', NULL, 0, 0, 0, 0, 0, 0, 0, 0, 1, 18019);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1189, 0, 0, 1, 0, 36, 0, 0, 0, 0);
INSERT INTO waypoint_data VALUES 
(455250, 1, -11024.3, 1458.48, 42.9863, 0, 0, 0, NULL, 100, 0),
(455250, 2, -11018.3, 1434.54, 43.1113, 0, 0, 0, NULL, 100, 0),
(455250, 3, -10999.9, 1416.52, 43.1031, 0, 0, 0, NULL, 100, 0),
(455250, 4, -10983.1, 1410.91, 42.9781, 0, 0, 0, NULL, 100, 0),
(455250, 5, -10965.8, 1418.47, 43.1677, 0, 0, 0, NULL, 100, 0),
(455250, 6, -10933.5, 1423.93, 43.0427, 0, 0, 0, NULL, 100, 0),
(455250, 7, -10915.6, 1419.7, 42.8789, 0, 0, 0, NULL, 100, 0),
(455250, 8, -10910.1, 1432.67, 42.5049, 0, 0, 0, NULL, 100, 0),
(455250, 9, -10900, 1445.11, 41.7178, 0, 0, 0, NULL, 100, 0),
(455250, 10, -10885.3, 1457.35, 40.7271, 0, 0, 0, NULL, 100, 0),
(455250, 11, -10866.9, 1466.69, 41.7989, 0, 0, 0, NULL, 100, 0),
(455250, 12, -10833.4, 1477.93, 44.4321, 0, 0, 0, NULL, 100, 0),
(455250, 13, -10799.9, 1489.47, 46.8256, 0, 0, 0, NULL, 100, 0),
(455250, 14, -10766.5, 1498.05, 47.711, 0, 0, 0, NULL, 100, 0),
(455250, 15, -10733, 1502.48, 48.0632, 0, 0, 0, NULL, 100, 0),
(455250, 16, -10700.1, 1502.66, 47.1882, 0, 0, 0, NULL, 100, 0),
(455250, 17, -10666.7, 1507.09, 46.5445, 0, 0, 0, NULL, 100, 0),
(455250, 18, -10633.3, 1539.86, 47.3145, 0, 0, 0, NULL, 100, 0),
(455250, 19, -10600.2, 1555.27, 47.4395, 0, 0, 0, NULL, 100, 0),
(455250, 20, -10566.9, 1562.48, 46.8828, 0, 0, 0, NULL, 100, 0),
(455250, 21, -10533.6, 1587.42, 44.4937, 0, 0, 0, NULL, 100, 0),
(455250, 22, -10508.9, 1614.24, 41.9046, 0, 0, 0, NULL, 100, 0),
(455250, 23, -10481.2, 1648.12, 37.4687, 0, 0, 0, NULL, 100, 0),
(455250, 24, -10461.7, 1666.65, 35.5912, 0, 0, 0, NULL, 100, 0),
(455250, 25, -10462.7, 1691.47, 35.5957, 0, 3000, 0, NULL, 100, 0),
(455250, 26, -10460.4, 1693.59, 35.5957, 0, 0, 1, NULL, 100, 0),
(455250, 27, -10449, 1708.79, 35.2494, 0, 0, 1, NULL, 100, 0),
(455250, 28, -10426.5, 1743.92, 33.7402, 0, 0, 1, NULL, 100, 0),
(455250, 29, -10416.6, 1777.68, 28.7875, 0, 0, 1, NULL, 100, 0),
(455250, 30, -10412.4, 1810.25, 24.3714, 0, 0, 1, NULL, 100, 0),
(455250, 31, -10404, 1849.8, 18.6227, 0, 0, 1, NULL, 100, 0),
(455250, 32, -10403.2, 1873.64, 14.9501, 0, 0, 1, NULL, 100, 0),
(455250, 33, -10414, 1899.89, 9.32512, 0, 0, 1, NULL, 100, 0),
(455250, 34, -10439, 1920.27, 10.5826, 0, 0, 1, NULL, 100, 0),
(455250, 35, -10458.1, 1931.1, 9.11068, 0, 0, 1, NULL, 100, 0),
(455250, 36, -10472.5, 1938.23, 7.89757, 0, 0, 1, NULL, 100, 0),
(455250, 37, -10464, 1955.49, 8.93939, 0, 0, 1, NULL, 100, 0),
(455250, 38, -10477.6, 1976.32, 9.24443, 0, 0, 1, NULL, 100, 0),
(455250, 39, -10501.4, 1972.82, 10.7568, 0, 0, 1, NULL, 100, 0),
(455250, 40, -10511.7, 1961.35, 7.41663, 0, 0, 1, NULL, 100, 0),
(455250, 41, -10523.6, 1943.46, 4.23074, 0, 0, 1, NULL, 100, 0),
(455250, 42, -10548.2, 1951.47, -0.460234, 0, 0, 1, NULL, 100, 0),
(455250, 43, -10568.6, 1978.86, -5.38337, 0, 10000, 1, 455250, 100, 0),
(455250, 44, -10555.8, 1958.8, -3.3736, 0, 0, 1, NULL, 100, 0),
(455250, 45, -10542.3, 1947.6, 1.62642, 0, 0, 1, NULL, 100, 0),
(455250, 46, -10524.9, 1942.46, 4.34905, 0, 0, 1, NULL, 100, 0),
(455250, 47, -10516.4, 1948.99, 4.69907, 0, 0, 1, NULL, 100, 0),
(455250, 48, -10503.2, 1973.02, 10.3947, 0, 0, 1, NULL, 100, 0),
(455250, 49, -10475.6, 1974.96, 9.28033, 0, 0, 1, NULL, 100, 0),
(455250, 50, -10464, 1957.86, 8.6916, 0, 0, 1, NULL, 100, 0),
(455250, 51, -10471.7, 1938.27, 7.97253, 0, 0, 1, NULL, 100, 0),
(455250, 52, -10457.9, 1932.24, 9.2374, 0, 0, 1, NULL, 100, 0),
(455250, 53, -10436.6, 1916.33, 9.53566, 0, 0, 1, NULL, 100, 0),
(455250, 54, -10418.4, 1904.54, 7.83281, 0, 0, 1, NULL, 100, 0),
(455250, 55, -10409.8, 1887.79, 11.506, 0, 0, 1, NULL, 100, 0),
(455250, 56, -10406.8, 1877.75, 14.0414, 0, 0, 1, NULL, 100, 0),
(455250, 57, -10400.5, 1863.82, 16.2073, 0, 0, 1, NULL, 100, 0),
(455250, 58, -10404.8, 1832.07, 21.0701, 0, 0, 1, NULL, 100, 0),
(455250, 59, -10413.2, 1806.55, 24.8249, 0, 0, 1, NULL, 100, 0),
(455250, 60, -10424, 1772.13, 29.7769, 0, 0, 1, NULL, 100, 0),
(455250, 61, -10425.3, 1747.43, 33.4757, 0, 0, 1, NULL, 100, 0),
(455250, 62, -10411.2, 1735.17, 37.0929, 0, 0, 1, NULL, 100, 0),
(455250, 63, -10387.9, 1725.82, 38.0202, 0, 0, 1, NULL, 100, 0),
(455250, 64, -10352.2, 1724.66, 36.0416, 0, 0, 0, NULL, 100, 0),
(455250, 65, -10323.8, 1718.77, 34.8388, 0, 0, 0, NULL, 100, 0),
(455250, 66, -10299.7, 1706.26, 37.1308, 0, 0, 0, NULL, 100, 0),
(455250, 67, -10266, 1692.6, 35.8463, 0, 0, 0, NULL, 100, 0),
(455250, 68, -10247, 1675.18, 34.4902, 0, 0, 0, NULL, 100, 0),
(455250, 69, -10223.5, 1642.29, 38.3598, 0, 0, 0, NULL, 100, 0),
(455250, 70, -10205.1, 1605.19, 46.0555, 0, 0, 0, NULL, 100, 0),
(455250, 71, -10185.6, 1586.09, 45.8647, 0, 0, 0, NULL, 100, 0),
(455250, 72, -10164.2, 1572.13, 44.8373, 0, 0, 0, NULL, 100, 0),
(455250, 73, -10123.5, 1555.31, 42.3237, 0, 0, 0, NULL, 100, 0),
(455250, 74, -10108.4, 1541.78, 42.2112, 0, 0, 0, NULL, 100, 0),
(455250, 75, -10085.1, 1516.22, 42.4478, 0, 0, 0, NULL, 100, 0),
(455250, 76, -10067.8, 1486.12, 46.4501, 0, 0, 0, NULL, 100, 0),
(455250, 77, -10052.9, 1467.47, 45.4537, 0, 0, 1, NULL, 100, 0),
(455250, 78, -10014.5, 1459.66, 41.2095, 0, 0, 1, NULL, 100, 0),
(455250, 79, -9991.49, 1458.64, 41.4995, 0, 0, 1, NULL, 100, 0),
(455250, 80, -9976.18, 1457.13, 45.0206, 0, 0, 1, NULL, 100, 0),
(455250, 81, -9948.04, 1454.6, 40.4763, 0, 0, 1, NULL, 100, 0),
(455250, 82, -9933.26, 1441.73, 39.1431, 0, 0, 1, NULL, 100, 0),
(455250, 83, -9923.84, 1443.33, 39.3621, 0, 0, 1, NULL, 100, 0),
(455250, 84, -9900.19, 1457.97, 40.8349, 0, 0, 1, NULL, 100, 0),
(455250, 85, -9881.88, 1450.04, 43.709, 0, 0, 1, NULL, 100, 0),
(455250, 86, -9875.23, 1450.28, 42.8109, 0, 0, 1, NULL, 100, 0),
(455250, 87, -9859.63, 1457.97, 41.3427, 0, 0, 1, NULL, 100, 0),
(455250, 88, -9849.83, 1453.91, 40.5755, 0, 0, 1, NULL, 100, 0),
(455250, 89, -9843.56, 1445.74, 39.0059, 0, 0, 1, NULL, 100, 0),
(455250, 90, -9823.62, 1423.47, 36.7852, 0, 0, 1, NULL, 100, 0),
(455250, 91, -9821.87, 1412.5, 36.499, 0, 0, 1, NULL, 100, 0),
(455250, 92, -9826.46, 1406.79, 36.9153, 0, 0, 1, NULL, 100, 0),
(455250, 93, -9835.87, 1401.99, 38.0764, 0, 0, 1, NULL, 100, 0),
(455250, 94, -9842.13, 1396.29, 37.3674, 0, 10000, 1, 1189, 100, 0),
(455250, 95, -9823.06, 1411.54, 36.3635, 0, 0, 1, NULL, 100, 0),
(455250, 96, -9822.35, 1420.64, 36.5567, 0, 0, 1, NULL, 100, 0),
(455250, 97, -9834.79, 1435.69, 37.6628, 0, 0, 1, NULL, 100, 0),
(455250, 98, -9853.21, 1456.05, 40.9653, 0, 0, 1, NULL, 100, 0),
(455250, 99, -9870.27, 1455.45, 42.3767, 0, 0, 1, NULL, 100, 0),
(455250, 100, -9882.26, 1448.69, 43.7431, 0, 0, 1, NULL, 100, 0),
(455250, 101, -9898.55, 1456.87, 41.1526, 0, 0, 1, NULL, 100, 0),
(455250, 102, -9906.68, 1454.73, 40.4808, 0, 0, 1, NULL, 100, 0),
(455250, 103, -9932.6, 1441.31, 39.124, 0, 0, 1, NULL, 100, 0),
(455250, 104, -9950.89, 1456.18, 40.5866, 0, 0, 1, NULL, 100, 0),
(455250, 105, -9967.03, 1455.99, 44.5031, 0, 0, 1, NULL, 100, 0),
(455250, 106, -9991.21, 1458.52, 41.5305, 0, 0, 1, NULL, 100, 0),
(455250, 107, -10012.2, 1466.57, 40.9185, 0, 0, 1, NULL, 100, 0),
(455250, 108, -10050.7, 1470.6, 45.6082, 0, 0, 1, NULL, 100, 0),
(455250, 109, -10070, 1490.06, 46.4594, 0, 0, 1, NULL, 100, 0),
(455250, 110, -10089.4, 1518.78, 41.9798, 0, 0, 0, NULL, 100, 0),
(455250, 111, -10121.4, 1553.17, 42.3523, 0, 0, 0, NULL, 100, 0),
(455250, 112, -10158.6, 1569.76, 44.1323, 0, 0, 0, NULL, 100, 0),
(455250, 113, -10185.4, 1584.59, 45.8592, 0, 0, 0, NULL, 100, 0),
(455250, 114, -10204, 1605.52, 46.0855, 0, 0, 0, NULL, 100, 0),
(455250, 115, -10218.8, 1633.28, 40.5342, 0, 0, 0, NULL, 100, 0),
(455250, 116, -10242.3, 1669.62, 34.6274, 0, 0, 0, NULL, 100, 0),
(455250, 117, -10262.1, 1689.54, 35.5146, 0, 0, 0, NULL, 100, 0),
(455250, 118, -10282.5, 1700.46, 36.9913, 0, 0, 0, NULL, 100, 0),
(455250, 119, -10311.1, 1711.42, 36.4032, 0, 0, 0, NULL, 100, 0),
(455250, 120, -10335.8, 1721.85, 34.6113, 0, 0, 0, NULL, 100, 0),
(455250, 121, -10381.6, 1726.92, 38.463, 0, 0, 0, NULL, 100, 0),
(455250, 122, -10409, 1723.05, 35.455, 0, 0, 0, NULL, 100, 0),
(455250, 123, -10425.2, 1708.05, 34.4076, 0, 0, 0, NULL, 100, 0),
(455250, 124, -10440.8, 1688.63, 34.9281, 0, 0, 0, NULL, 100, 0),
(455250, 125, -10462.9, 1663.45, 35.5745, 0, 0, 0, NULL, 100, 0),
(455250, 126, -10482.5, 1648.56, 37.4599, 0, 0, 0, NULL, 100, 0),
(455250, 127, -10506.4, 1618.82, 41.2972, 0, 0, 0, NULL, 100, 0),
(455250, 128, -10525.8, 1593.32, 43.8772, 0, 0, 0, NULL, 100, 0),
(455250, 129, -10543.4, 1581.86, 45.0814, 0, 0, 0, NULL, 100, 0),
(455250, 130, -10554.1, 1574.88, 45.7229, 0, 0, 0, NULL, 100, 0),
(455250, 131, -10576.7, 1557.12, 47.438, 0, 0, 0, NULL, 100, 0),
(455250, 132, -10601.5, 1555.4, 47.4711, 0, 0, 0, NULL, 100, 0),
(455250, 133, -10625.3, 1545.97, 47.1961, 0, 0, 0, NULL, 100, 0),
(455250, 134, -10642.9, 1533.3, 47.3155, 0, 0, 0, NULL, 100, 0),
(455250, 135, -10663.4, 1510.29, 46.5769, 0, 0, 0, NULL, 100, 0),
(455250, 136, -10676.3, 1504.88, 46.0457, 0, 0, 0, NULL, 100, 0),
(455250, 137, -10706.1, 1503.8, 47.5113, 0, 0, 0, NULL, 100, 0),
(455250, 138, -10719.3, 1503.75, 48.4023, 0, 0, 0, NULL, 100, 0),
(455250, 139, -10741.6, 1501.3, 47.8971, 0, 0, 0, NULL, 100, 0),
(455250, 140, -10779, 1495.48, 47.7248, 0, 0, 0, NULL, 100, 0),
(455250, 141, -10814.5, 1484.85, 45.5877, 0, 0, 0, NULL, 100, 0),
(455250, 142, -10847.3, 1473.48, 43.427, 0, 0, 0, NULL, 100, 0),
(455250, 143, -10875.3, 1462.61, 41.51, 0, 0, 0, NULL, 100, 0),
(455250, 144, -10891.5, 1454.56, 41.0618, 0, 0, 0, NULL, 100, 0),
(455250, 145, -10904.6, 1442.63, 41.8719, 0, 0, 0, NULL, 100, 0),
(455250, 146, -10912.3, 1429.66, 42.6348, 0, 0, 0, NULL, 100, 0),
(455250, 147, -10916.2, 1420.05, 42.7921, 0, 0, 0, NULL, 100, 0),
(455250, 148, -10942.5, 1424.95, 43.1856, 0, 0, 0, NULL, 100, 0),
(455250, 149, -10957.7, 1422.2, 43.1459, 0, 0, 0, NULL, 100, 0),
(455250, 150, -10979.7, 1411.7, 42.7758, 0, 0, 0, NULL, 100, 0),
(455250, 151, -10992.3, 1412.79, 42.9071, 0, 0, 0, NULL, 100, 0),
(455250, 152, -11011.4, 1426.53, 43.0596, 0, 0, 0, NULL, 100, 0),
(455250, 153, -11021.4, 1442.32, 43.0378, 0, 0, 0, NULL, 100, 0),
(455250, 154, -11024.3, 1459.81, 42.9215, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (45525, 455250, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 45525);

-- Importing creature with guid 66996 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1066315 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1066315 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1066315 AND spawnType = 0 WHERE ce.spawnID = 1066315;
INSERT INTO creature_entry (spawnID, entry) VALUES (66996, 7053);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (66996, 0, 1, 5805, 1, -11124.1, 545.44,  70.4323, 1.72551, 300, 0, 0, 605, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (66996, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 89532 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1088187 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1088187 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1088187 AND spawnType = 0 WHERE ce.spawnID = 1088187;
INSERT INTO creature_entry (spawnID, entry) VALUES (89532, 869);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (89532, 0, 1, 2366, 1, -10657.1, 1010.88,  32.5521, 0.38323, 300, 0, 0, 1910, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(895320, 1, -10642.8, 1016.66, 31.5651, 0, 0, 0, NULL, 100, 0),
(895320, 2, -10620.2, 1000.85, 32.9944, 0, 0, 0, NULL, 100, 0),
(895320, 3, -10594, 987.937, 36.8771, 0, 0, 0, NULL, 100, 0),
(895320, 4, -10564.6, 975.733, 40.6344, 0, 0, 0, NULL, 100, 0),
(895320, 5, -10532.9, 961.984, 40.8852, 0, 0, 0, NULL, 100, 0),
(895320, 6, -10500.2, 952.64, 39.4598, 0, 0, 0, NULL, 100, 0),
(895320, 7, -10466.9, 953.813, 35.8982, 0, 0, 0, NULL, 100, 0),
(895320, 8, -10433.2, 981.964, 34.7575, 0, 0, 0, NULL, 100, 0),
(895320, 9, -10410.5, 994.716, 32.1643, 0, 0, 0, NULL, 100, 0),
(895320, 10, -10400.4, 992.543, 31.5791, 0, 0, 0, NULL, 100, 0),
(895320, 11, -10386.8, 984.074, 31.5979, 0, 0, 0, NULL, 100, 0),
(895320, 12, -10367.4, 980.618, 31.4729, 0, 0, 0, NULL, 100, 0),
(895320, 13, -10333.5, 978.783, 31.2549, 0, 0, 0, NULL, 100, 0),
(895320, 14, -10300.2, 965.637, 31.253, 0, 0, 0, NULL, 100, 0),
(895320, 15, -10285.6, 964.09, 31.1856, 0, 0, 0, NULL, 100, 0),
(895320, 16, -10273.9, 970.693, 31.1856, 0, 0, 0, NULL, 100, 0),
(895320, 17, -10267.6, 983.352, 31.3106, 0, 0, 0, NULL, 100, 0),
(895320, 18, -10260.9, 989.903, 31.3775, 0, 0, 0, NULL, 100, 0),
(895320, 19, -10234, 996.264, 31.4918, 0, 0, 0, NULL, 100, 0),
(895320, 20, -10215.9, 994.502, 32.5031, 0, 0, 0, NULL, 100, 0),
(895320, 21, -10200.1, 989.352, 32.9333, 0, 0, 0, NULL, 100, 0),
(895320, 22, -10190, 984.9, 33.7804, 0, 0, 0, NULL, 100, 0),
(895320, 23, -10166.8, 986.717, 34.0541, 0, 0, 0, NULL, 100, 0),
(895320, 24, -10133.5, 990.649, 36.0486, 0, 0, 0, NULL, 100, 0),
(895320, 25, -10116.7, 990.127, 38.2197, 0, 0, 0, NULL, 100, 0),
(895320, 26, -10100.2, 999.191, 37.4609, 0, 0, 0, NULL, 100, 0),
(895320, 27, -10116.7, 990.127, 38.2197, 0, 0, 0, NULL, 100, 0),
(895320, 28, -10133.5, 990.649, 36.0486, 0, 0, 0, NULL, 100, 0),
(895320, 29, -10166.8, 986.717, 34.0541, 0, 0, 0, NULL, 100, 0),
(895320, 30, -10190, 984.9, 33.7804, 0, 0, 0, NULL, 100, 0),
(895320, 31, -10200.1, 989.352, 32.9333, 0, 0, 0, NULL, 100, 0),
(895320, 32, -10215.9, 994.502, 32.5031, 0, 0, 0, NULL, 100, 0),
(895320, 33, -10234, 996.264, 31.4918, 0, 0, 0, NULL, 100, 0),
(895320, 34, -10260.9, 989.903, 31.3775, 0, 0, 0, NULL, 100, 0),
(895320, 35, -10267.6, 983.352, 31.3106, 0, 0, 0, NULL, 100, 0),
(895320, 36, -10273.9, 970.693, 31.1856, 0, 0, 0, NULL, 100, 0),
(895320, 37, -10285.6, 964.09, 31.1856, 0, 0, 0, NULL, 100, 0),
(895320, 38, -10300.2, 965.637, 31.253, 0, 0, 0, NULL, 100, 0),
(895320, 39, -10333.5, 978.783, 31.2549, 0, 0, 0, NULL, 100, 0),
(895320, 40, -10367.4, 980.618, 31.4729, 0, 0, 0, NULL, 100, 0),
(895320, 41, -10386.8, 984.074, 31.5979, 0, 0, 0, NULL, 100, 0),
(895320, 42, -10400.4, 992.543, 31.5791, 0, 0, 0, NULL, 100, 0),
(895320, 43, -10410.5, 994.716, 32.1643, 0, 0, 0, NULL, 100, 0),
(895320, 44, -10433.2, 981.964, 34.7575, 0, 0, 0, NULL, 100, 0),
(895320, 45, -10466.9, 953.813, 35.8982, 0, 0, 0, NULL, 100, 0),
(895320, 46, -10500.2, 952.64, 39.4598, 0, 0, 0, NULL, 100, 0),
(895320, 47, -10532.9, 961.984, 40.8852, 0, 0, 0, NULL, 100, 0),
(895320, 48, -10564.6, 975.733, 40.6344, 0, 0, 0, NULL, 100, 0),
(895320, 49, -10594, 987.937, 36.8771, 0, 0, 0, NULL, 100, 0),
(895320, 50, -10620.2, 1000.85, 32.9944, 0, 0, 0, NULL, 100, 0),
(895320, 51, -10642.8, 1016.66, 31.5651, 0, 0, 0, NULL, 100, 0),
(895320, 52, -10647.5, 1012.37, 31.7765, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (89532, 895320, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 90476 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1089094 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1089094 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1089094 AND spawnType = 0 WHERE ce.spawnID = 1089094;
INSERT INTO creature_entry (spawnID, entry) VALUES (90476, 6267);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (90476, 0, 1, 0, 1, -8962.71, 1043.58,  53.548, 3.93008, 190, 0, 0, 433, 490, 0, 0, 0, 10);

-- Importing creature with guid 49222 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1048962 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1048962 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1048962 AND spawnType = 0 WHERE ce.spawnID = 1048962;
INSERT INTO creature_entry (spawnID, entry) VALUES (49222, 1533);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (49222, 0, 1, 0, 0, 2869.45, 1046.28,  114.79, 0.199966, 72000, 0, 0, 187, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(492220, 1, 2882.23, 1037.63, 111.597, 0, 0, 0, NULL, 100, 0),
(492220, 2, 2891.75, 1028.41, 107.154, 0, 0, 0, NULL, 100, 0),
(492220, 3, 2897.44, 1009.81, 108.33, 0, 0, 0, NULL, 100, 0),
(492220, 4, 2898.23, 975.086, 114.218, 0, 0, 0, NULL, 100, 0),
(492220, 5, 2898.81, 957.889, 115.079, 0, 0, 0, NULL, 100, 0),
(492220, 6, 2900.9, 948.054, 115.272, 0, 0, 0, NULL, 100, 0),
(492220, 7, 2908.08, 932.171, 114.899, 0, 0, 0, NULL, 100, 0),
(492220, 8, 2908.56, 927.233, 114.845, 0, 0, 0, NULL, 100, 0),
(492220, 9, 2904.41, 921.068, 114.925, 0, 0, 0, NULL, 100, 0),
(492220, 10, 2897.89, 915.506, 114.76, 0, 0, 0, NULL, 100, 0),
(492220, 11, 2891.89, 912.366, 114.579, 0, 0, 0, NULL, 100, 0),
(492220, 12, 2864.54, 888.054, 112.785, 0, 0, 0, NULL, 100, 0),
(492220, 13, 2846.08, 873.334, 112.199, 0, 0, 0, NULL, 100, 0),
(492220, 14, 2822.72, 864.899, 111.841, 0, 0, 0, NULL, 100, 0),
(492220, 15, 2805.19, 861.231, 111.841, 0, 0, 0, NULL, 100, 0),
(492220, 16, 2792.86, 863.311, 111.756, 0, 0, 0, NULL, 100, 0),
(492220, 17, 2785.89, 866.45, 111.558, 0, 0, 0, NULL, 100, 0),
(492220, 18, 2763.86, 884.346, 111.468, 0, 0, 0, NULL, 100, 0),
(492220, 19, 2752.15, 900.057, 111.775, 0, 0, 0, NULL, 100, 0),
(492220, 20, 2745.14, 912.963, 110.819, 0, 0, 0, NULL, 100, 0),
(492220, 21, 2735.78, 942.471, 109.498, 0, 0, 0, NULL, 100, 0),
(492220, 22, 2733.88, 952.497, 109.278, 0, 0, 0, NULL, 100, 0),
(492220, 23, 2733.97, 964.451, 109.262, 0, 0, 0, NULL, 100, 0),
(492220, 24, 2735.98, 972.289, 109.316, 0, 0, 0, NULL, 100, 0),
(492220, 25, 2748.28, 980.282, 109.316, 0, 0, 0, NULL, 100, 0),
(492220, 26, 2759.92, 988.789, 109.336, 0, 0, 0, NULL, 100, 0),
(492220, 27, 2767.27, 997.694, 109.877, 0, 0, 0, NULL, 100, 0),
(492220, 28, 2770.28, 1005.85, 109.477, 0, 0, 0, NULL, 100, 0),
(492220, 29, 2772.9, 1017.25, 108.61, 0, 0, 0, NULL, 100, 0),
(492220, 30, 2773.85, 1027.46, 108.478, 0, 0, 0, NULL, 100, 0),
(492220, 31, 2790.31, 1042.25, 110.684, 0, 0, 0, NULL, 100, 0),
(492220, 32, 2810.03, 1038.34, 111.127, 0, 0, 0, NULL, 100, 0),
(492220, 33, 2824.45, 1037.32, 112.118, 0, 0, 0, NULL, 100, 0),
(492220, 34, 2839.4, 1037.63, 113.927, 0, 0, 0, NULL, 100, 0),
(492220, 35, 2853.29, 1041.9, 115.65, 0, 0, 0, NULL, 100, 0),
(492220, 36, 2861.56, 1043.2, 115.985, 0, 0, 0, NULL, 100, 0),
(492220, 37, 2882.23, 1037.63, 111.597, 0, 20000, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (49222, 492220, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 29803 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1029668 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1029668 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1029668 AND spawnType = 0 WHERE ce.spawnID = 1029668;
INSERT INTO creature_entry (spawnID, entry) VALUES (29803, 1568);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (29803, 0, 1, 1582, 1, 1671.89, 1678.29,  120.802, 0.0349066, 300, 0, 0, 102, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (29803, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 31872 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1031733 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1031733 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1031733 AND spawnType = 0 WHERE ce.spawnID = 1031733;
INSERT INTO creature_entry (spawnID, entry) VALUES (31872, 5732);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (31872, 0, 1, 4109, 1, 1535.91, 275.839,  -62.1765, 0.022364, 300, 0, 0, 484, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(318720, 1, 1535.91, 275.839, -62.1765, 0, 0, 0, NULL, 100, 0),
(318720, 2, 1542.76, 286.413, -62.182, 0, 0, 0, NULL, 100, 0),
(318720, 3, 1549.17, 295.035, -62.1812, 0, 0, 0, NULL, 100, 0),
(318720, 4, 1565.81, 303.835, -62.1815, 0, 0, 0, NULL, 100, 0),
(318720, 5, 1577.01, 309.116, -62.1801, 0, 0, 0, NULL, 100, 0),
(318720, 6, 1588.85, 311.303, -62.1778, 0, 0, 0, NULL, 100, 0),
(318720, 7, 1594.25, 319.317, -62.1777, 0, 0, 0, NULL, 100, 0),
(318720, 8, 1595.19, 347.093, -62.1777, 0, 0, 0, NULL, 100, 0),
(318720, 9, 1594.17, 364.99, -62.1777, 0, 0, 0, NULL, 100, 0),
(318720, 10, 1586.59, 373.629, -62.1777, 0, 0, 0, NULL, 100, 0),
(318720, 11, 1570.1, 372.695, -62.1777, 0, 0, 0, NULL, 100, 0),
(318720, 12, 1550.57, 370.057, -62.1792, 0, 0, 0, NULL, 100, 0),
(318720, 13, 1543.08, 363.322, -62.1868, 0, 0, 0, NULL, 100, 0),
(318720, 14, 1519.26, 348.814, -60.7859, 0, 0, 0, NULL, 100, 0),
(318720, 15, 1501.66, 336.185, -60.0916, 0, 0, 0, NULL, 100, 0),
(318720, 16, 1485.87, 314.104, -61.4839, 0, 0, 0, NULL, 100, 0),
(318720, 17, 1474.47, 295.632, -62.1456, 0, 0, 0, NULL, 100, 0),
(318720, 18, 1466.42, 280.133, -61.831, 0, 0, 0, NULL, 100, 0),
(318720, 19, 1462.71, 264.028, -62.1777, 0, 0, 0, NULL, 100, 0),
(318720, 20, 1465.77, 247.546, -62.1777, 0, 0, 0, NULL, 100, 0),
(318720, 21, 1478.26, 242.965, -62.1777, 0, 0, 0, NULL, 100, 0),
(318720, 22, 1501.93, 242.955, -62.1777, 0, 0, 0, NULL, 100, 0),
(318720, 23, 1520.96, 243.381, -62.1781, 0, 0, 0, NULL, 100, 0),
(318720, 24, 1525.29, 251.149, -62.178, 0, 0, 0, NULL, 100, 0),
(318720, 25, 1528.97, 265.893, -62.1777, 0, 0, 0, NULL, 100, 0),
(318720, 26, 1535.91, 275.839, -62.1765, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (31872, 318720, 0, 0, 0, 257, 0, NULL);

-- Importing creature with guid 31873 with import type REPLACE_ALL
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 38094 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -38094 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 38094 AND spawnType = 0 WHERE ce.spawnID = 38094;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1031734 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1031734 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1031734 AND spawnType = 0 WHERE ce.spawnID = 1031734;
DELETE FROM waypoint_data WHERE id = 317640;
INSERT INTO creature_entry (spawnID, entry) VALUES (31873, 5735);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (31873, 0, 1, 3979, 0, 1401.93, 361.817,  -84.868, 0.122173, 300, 0, 0, 102, 0, 0, 0, 0, 10);
INSERT INTO creature_entry (spawnID, entry) VALUES (38094, 5735);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (38094, 0, 1, 3980, 0, 1405.82, 375.381,  -84.868, 4.83456, 300, 0, 0, 102, 0, 0, 0, 0, 10);

-- Importing creature with guid 31923 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1031784 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1031784 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1031784 AND spawnType = 0 WHERE ce.spawnID = 1031784;
INSERT INTO creature_entry (spawnID, entry) VALUES (31923, 11945);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (31923, 0, 1, 11904, 0, 2286.34, 247.798,  41.1148, 3.00922, 300, 0, 0, 102, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (31923, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 32035 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1031896 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1031896 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1031896 AND spawnType = 0 WHERE ce.spawnID = 1031896;
INSERT INTO creature_entry (spawnID, entry) VALUES (32035, 23713);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32035, 0, 1, 8848, 1, 1556.82, 206.279,  -60.7725, 1.4197, 300, 0, 0, 2218, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(320350, 1, 1560.83, 209.632, -60.7421, 0, 0, 0, NULL, 100, 0),
(320350, 2, 1567.42, 215.764, -59.5841, 0, 0, 0, NULL, 100, 0),
(320350, 3, 1573.73, 221.612, -59.6846, 0, 0, 0, NULL, 100, 0),
(320350, 4, 1578.99, 223.877, -61.8354, 0, 0, 0, NULL, 100, 0),
(320350, 5, 1583.84, 224.62, -62.0892, 0, 0, 0, NULL, 100, 0),
(320350, 6, 1595.7, 219.823, -57.1618, 0, 0, 0, NULL, 100, 0),
(320350, 7, 1595.65, 225.848, -55.2846, 0, 0, 0, NULL, 100, 0),
(320350, 8, 1596.25, 231.32, -52.153, 0, 0, 0, NULL, 100, 0),
(320350, 9, 1605.35, 230.99, -52.1497, 0, 0, 0, NULL, 100, 0),
(320350, 10, 1607.17, 235.047, -52.1509, 0, 0, 0, NULL, 100, 0),
(320350, 11, 1607.14, 240.583, -52.1573, 0, 0, 0, NULL, 100, 0),
(320350, 12, 1616.12, 240.593, -47.4926, 0, 0, 0, NULL, 100, 0),
(320350, 13, 1624.23, 239.499, -43.8868, 0, 0, 0, NULL, 100, 0),
(320350, 14, 1631.11, 241.189, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 15, 1624.3, 239.49, -43.8772, 0, 0, 0, NULL, 100, 0),
(320350, 16, 1616.12, 240.593, -47.4926, 0, 0, 0, NULL, 100, 0),
(320350, 17, 1607.14, 240.583, -52.1573, 0, 0, 0, NULL, 100, 0),
(320350, 18, 1607.17, 235.047, -52.1509, 0, 0, 0, NULL, 100, 0),
(320350, 19, 1605.35, 230.99, -52.1497, 0, 0, 0, NULL, 100, 0),
(320350, 20, 1596.25, 231.32, -52.153, 0, 0, 0, NULL, 100, 0),
(320350, 21, 1595.65, 225.848, -55.2846, 0, 0, 0, NULL, 100, 0),
(320350, 22, 1595.7, 219.823, -57.1618, 0, 0, 0, NULL, 100, 0),
(320350, 23, 1583.84, 224.62, -62.0892, 0, 0, 0, NULL, 100, 0),
(320350, 24, 1579.19, 223.908, -61.9083, 0, 0, 0, NULL, 100, 0),
(320350, 25, 1573.73, 221.612, -59.6846, 0, 0, 0, NULL, 100, 0),
(320350, 26, 1567.42, 215.764, -59.5841, 0, 0, 0, NULL, 100, 0),
(320350, 27, 1560.83, 209.632, -60.7421, 0, 0, 0, NULL, 100, 0),
(320350, 28, 1557.04, 206.466, -60.7763, 0, 0, 0, NULL, 100, 0),
(320350, 29, 1550.39, 203.157, -60.7695, 0, 0, 0, NULL, 100, 0),
(320350, 30, 1545.23, 202.719, -62.1832, 0, 0, 0, NULL, 100, 0),
(320350, 31, 1539.65, 204.423, -62.1826, 0, 0, 0, NULL, 100, 0),
(320350, 32, 1534.03, 208.736, -62.1818, 0, 0, 0, NULL, 100, 0),
(320350, 33, 1530.08, 215.568, -62.1809, 0, 0, 0, NULL, 100, 0),
(320350, 34, 1531.97, 227.786, -62.1783, 0, 0, 0, NULL, 100, 0),
(320350, 35, 1532.85, 234.12, -62.1783, 0, 0, 0, NULL, 100, 0),
(320350, 36, 1533.76, 241.554, -62.1782, 0, 0, 0, NULL, 100, 0),
(320350, 37, 1535.32, 247.9, -62.1781, 0, 0, 0, NULL, 100, 0),
(320350, 38, 1536.04, 255.27, -62.1777, 0, 0, 0, NULL, 100, 0),
(320350, 39, 1537.23, 263.136, -62.1777, 0, 0, 0, NULL, 100, 0),
(320350, 40, 1542.86, 271.875, -62.1768, 0, 0, 0, NULL, 100, 0),
(320350, 41, 1546.77, 277.976, -62.1833, 0, 0, 0, NULL, 100, 0),
(320350, 42, 1555.23, 283.841, -60.7743, 0, 0, 0, NULL, 100, 0),
(320350, 43, 1559, 279.925, -60.772, 0, 0, 0, NULL, 100, 0),
(320350, 44, 1562.27, 274.753, -60.7577, 0, 0, 0, NULL, 100, 0),
(320350, 45, 1565.53, 269.078, -60.6939, 0, 0, 0, NULL, 100, 0),
(320350, 46, 1568.66, 265.453, -59.4534, 0, 0, 0, NULL, 100, 0),
(320350, 47, 1573.28, 260.474, -59.3003, 0, 0, 0, NULL, 100, 0),
(320350, 48, 1576.91, 256.333, -61.3601, 0, 0, 0, NULL, 100, 0),
(320350, 49, 1579.96, 255.398, -61.9104, 0, 0, 0, NULL, 100, 0),
(320350, 50, 1583.6, 256.013, -62.0965, 0, 0, 0, NULL, 100, 0),
(320350, 51, 1586.94, 257.516, -61.4295, 0, 0, 0, NULL, 100, 0),
(320350, 52, 1589.17, 258.373, -59.639, 0, 0, 0, NULL, 100, 0),
(320350, 53, 1593.72, 259.792, -57.1617, 0, 0, 0, NULL, 100, 0),
(320350, 54, 1596.38, 257.637, -57.1618, 0, 0, 0, NULL, 100, 0),
(320350, 55, 1595.66, 252.608, -53.6675, 0, 0, 0, NULL, 100, 0),
(320350, 56, 1596.05, 249.318, -52.1528, 0, 0, 0, NULL, 100, 0),
(320350, 57, 1589.19, 248.801, -52.1533, 0, 0, 0, NULL, 100, 0),
(320350, 58, 1585.73, 245.916, -52.1526, 0, 0, 0, NULL, 100, 0),
(320350, 59, 1584.44, 242.4, -52.1516, 0, 0, 0, NULL, 100, 0),
(320350, 60, 1579.16, 241.369, -49.833, 0, 0, 0, NULL, 100, 0),
(320350, 61, 1569.92, 242.132, -44.9335, 0, 0, 0, NULL, 100, 0),
(320350, 62, 1563.14, 243.895, -43.3325, 0, 0, 0, NULL, 100, 0),
(320350, 63, 1559.2, 247.966, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 64, 1561.46, 256.767, -43.1026, 0, 0, 0, NULL, 100, 0),
(320350, 65, 1568.45, 263.594, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 66, 1575.97, 271.122, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 67, 1587.46, 276.32, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 68, 1601.59, 275.179, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 69, 1611.95, 274.535, -43.1028, 0, 0, 0, NULL, 100, 0),
(320350, 70, 1620.38, 266.854, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 71, 1628.55, 255.421, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 72, 1630.92, 248.495, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 73, 1631.49, 231.21, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 74, 1629.06, 224.007, -43.1025, 0, 0, 0, NULL, 100, 0),
(320350, 75, 1621.78, 216.405, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 76, 1612.36, 208.564, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 77, 1599.45, 204.769, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 78, 1589.99, 204.937, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 79, 1579.58, 207.341, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 80, 1572.47, 213.801, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 81, 1563.91, 223.09, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 82, 1559.54, 231.344, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 83, 1558.18, 239.552, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 84, 1559.54, 231.359, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 85, 1563.91, 223.09, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 86, 1572.36, 213.924, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 87, 1579.58, 207.341, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 88, 1589.99, 204.937, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 89, 1599.45, 204.769, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 90, 1612.36, 208.564, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 91, 1621.78, 216.405, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 92, 1629.06, 224.007, -43.1025, 0, 0, 0, NULL, 100, 0),
(320350, 93, 1631.49, 231.21, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 94, 1630.92, 248.495, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 95, 1628.55, 255.421, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 96, 1620.38, 266.854, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 97, 1611.95, 274.535, -43.1028, 0, 0, 0, NULL, 100, 0),
(320350, 98, 1601.59, 275.179, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 99, 1587.46, 276.32, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 100, 1575.97, 271.122, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 101, 1568.45, 263.594, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 102, 1561.46, 256.767, -43.1026, 0, 0, 0, NULL, 100, 0),
(320350, 103, 1559.2, 247.966, -43.1027, 0, 0, 0, NULL, 100, 0),
(320350, 104, 1563.14, 243.895, -43.3325, 0, 0, 0, NULL, 100, 0),
(320350, 105, 1569.92, 242.132, -44.9335, 0, 0, 0, NULL, 100, 0),
(320350, 106, 1579.16, 241.369, -49.833, 0, 0, 0, NULL, 100, 0),
(320350, 107, 1584.44, 242.4, -52.1516, 0, 0, 0, NULL, 100, 0),
(320350, 108, 1585.73, 245.916, -52.1526, 0, 0, 0, NULL, 100, 0),
(320350, 109, 1589.19, 248.801, -52.1533, 0, 0, 0, NULL, 100, 0),
(320350, 110, 1596.05, 249.318, -52.1528, 0, 0, 0, NULL, 100, 0),
(320350, 111, 1595.65, 252.503, -53.5886, 0, 0, 0, NULL, 100, 0),
(320350, 112, 1596.38, 257.637, -57.1618, 0, 0, 0, NULL, 100, 0),
(320350, 113, 1593.8, 259.818, -57.1618, 0, 0, 0, NULL, 100, 0),
(320350, 114, 1589.17, 258.373, -59.639, 0, 0, 0, NULL, 100, 0),
(320350, 115, 1587.2, 257.613, -61.2253, 0, 0, 0, NULL, 100, 0),
(320350, 116, 1583.6, 256.013, -62.0965, 0, 0, 0, NULL, 100, 0),
(320350, 117, 1579.96, 255.398, -61.9104, 0, 0, 0, NULL, 100, 0),
(320350, 118, 1576.91, 256.333, -61.3601, 0, 0, 0, NULL, 100, 0),
(320350, 119, 1573.33, 260.418, -59.3121, 0, 0, 0, NULL, 100, 0),
(320350, 120, 1568.66, 265.453, -59.4534, 0, 0, 0, NULL, 100, 0),
(320350, 121, 1565.53, 269.078, -60.6939, 0, 0, 0, NULL, 100, 0),
(320350, 122, 1562.27, 274.753, -60.7577, 0, 0, 0, NULL, 100, 0),
(320350, 123, 1559, 279.925, -60.772, 0, 0, 0, NULL, 100, 0),
(320350, 124, 1555.23, 283.841, -60.7743, 0, 0, 0, NULL, 100, 0),
(320350, 125, 1546.77, 277.976, -62.1833, 0, 0, 0, NULL, 100, 0),
(320350, 126, 1542.86, 271.875, -62.1768, 0, 0, 0, NULL, 100, 0),
(320350, 127, 1537.23, 263.136, -62.1777, 0, 0, 0, NULL, 100, 0),
(320350, 128, 1536.04, 255.27, -62.1777, 0, 0, 0, NULL, 100, 0),
(320350, 129, 1535.32, 247.9, -62.1781, 0, 0, 0, NULL, 100, 0),
(320350, 130, 1533.76, 241.554, -62.1782, 0, 0, 0, NULL, 100, 0),
(320350, 131, 1532.85, 234.12, -62.1783, 0, 0, 0, NULL, 100, 0),
(320350, 132, 1531.97, 227.786, -62.1783, 0, 0, 0, NULL, 100, 0),
(320350, 133, 1530.08, 215.568, -62.1809, 0, 0, 0, NULL, 100, 0),
(320350, 134, 1534.03, 208.736, -62.1818, 0, 0, 0, NULL, 100, 0),
(320350, 135, 1539.65, 204.423, -62.1826, 0, 0, 0, NULL, 100, 0),
(320350, 136, 1545.23, 202.719, -62.1832, 0, 0, 0, NULL, 100, 0),
(320350, 137, 1550.39, 203.157, -60.7695, 0, 0, 0, NULL, 100, 0),
(320350, 138, 1556.82, 206.279, -60.7725, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (32035, 320350, 0, 0, 0, 257, 0, NULL);

-- Importing creature with guid 32036 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1031897 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1031897 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1031897 AND spawnType = 0 WHERE ce.spawnID = 1031897;
DELETE FROM waypoint_data WHERE id = 319270;
INSERT INTO creature_entry (spawnID, entry) VALUES (32036, 9566);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32036, 0, 1, 8849, 1, 2064.01, 288.57,  97.1146, 4.34587, 300, 0, 0, 2218, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (32036, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 38094 with import type REPLACE_ALL

-- Importing creature with guid 38100 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1037924 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1037924 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1037924 AND spawnType = 0 WHERE ce.spawnID = 1037924;
INSERT INTO creature_entry (spawnID, entry) VALUES (38100, 5747);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (38100, 0, 1, 4123, 1, 1514.81, 238.436,  -62.1777, 1.81138, 300, 0, 0, 405, 1202, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(381000, 1, 1525.65, 233.328, -62.1782, 0, 0, 0, NULL, 100, 0),
(381000, 2, 1526.56, 219.579, -62.1802, 0, 0, 0, NULL, 100, 0),
(381000, 3, 1531.41, 205.202, -62.1819, 0, 0, 0, NULL, 100, 0),
(381000, 4, 1542.79, 191.362, -62.1818, 0, 0, 0, NULL, 100, 0),
(381000, 5, 1559.79, 178.594, -62.1764, 0, 0, 0, NULL, 100, 0),
(381000, 6, 1577.52, 172.16, -62.1777, 0, 0, 0, NULL, 100, 0),
(381000, 7, 1587.42, 169.376, -62.1774, 0, 0, 0, NULL, 100, 0),
(381000, 8, 1594.17, 161.334, -62.1777, 0, 0, 0, NULL, 100, 0),
(381000, 9, 1594.49, 131.577, -62.1777, 0, 0, 0, NULL, 100, 0),
(381000, 10, 1592.6, 113.022, -62.1777, 0, 0, 0, NULL, 100, 0),
(381000, 11, 1582.28, 104.431, -62.1777, 0, 0, 0, NULL, 100, 0),
(381000, 12, 1563.74, 107.65, -62.1777, 0, 0, 0, NULL, 100, 0),
(381000, 13, 1547.06, 113.084, -62.1431, 0, 0, 0, NULL, 100, 0),
(381000, 14, 1537.05, 122.453, -61.5088, 0, 0, 0, NULL, 100, 0),
(381000, 15, 1532.86, 139.527, -62.1717, 0, 0, 0, NULL, 100, 0),
(381000, 16, 1521.77, 154.188, -62.1582, 0, 0, 0, NULL, 100, 0),
(381000, 17, 1509.39, 165.757, -62.157, 0, 0, 0, NULL, 100, 0),
(381000, 18, 1501.14, 174.273, -62.1594, 0, 0, 0, NULL, 100, 0),
(381000, 19, 1488.74, 180.9, -62.1781, 0, 0, 0, NULL, 100, 0),
(381000, 20, 1474.74, 188.001, -62.1923, 0, 0, 0, NULL, 100, 0),
(381000, 21, 1465.01, 201.309, -62.1762, 0, 0, 0, NULL, 100, 0),
(381000, 22, 1462.54, 211.253, -62.1777, 0, 0, 0, NULL, 100, 0),
(381000, 23, 1461.92, 229.935, -62.1777, 0, 0, 0, NULL, 100, 0),
(381000, 24, 1474.28, 240.684, -62.1777, 0, 0, 0, NULL, 100, 0),
(381000, 25, 1514.81, 238.436, -62.1777, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (38100, 381000, 0, 0, 0, 257, 0, NULL);

-- Importing creature with guid 38101 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1037925 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1037925 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1037925 AND spawnType = 0 WHERE ce.spawnID = 1037925;
DELETE FROM waypoint_data WHERE id = 379600;
INSERT INTO creature_entry (spawnID, entry) VALUES (38101, 2802);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (38101, 0, 1, 10563, 1, 1442.49, 36.327,  -62.0263, 5.4319, 300, 0, 0, 3398, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(381010, 1, 1442.49, 36.327, -62.0263, 0, 0, 0, NULL, 100, 0),
(381010, 2, 1435.49, 32.077, -62.0263, 0, 0, 0, NULL, 100, 0),
(381010, 3, 1416.44, 21.467, -62.0259, 0, 0, 0, NULL, 100, 0),
(381010, 4, 1405.55, 23.9147, -62.0261, 0, 0, 0, NULL, 100, 0),
(381010, 5, 1402.3, 26.4147, -62.0261, 0, 0, 0, NULL, 100, 0),
(381010, 6, 1387.13, 36.8857, -62.0265, 0, 0, 0, NULL, 100, 0),
(381010, 7, 1381.63, 49.8857, -62.0265, 0, 0, 0, NULL, 100, 0),
(381010, 8, 1379.01, 56.3228, -62.0267, 0, 0, 0, NULL, 100, 0),
(381010, 9, 1379.76, 60.8228, -62.0267, 0, 0, 0, NULL, 100, 0),
(381010, 10, 1383.4, 83.5168, -62.0329, 0, 0, 0, NULL, 100, 0),
(381010, 11, 1388.4, 87.5168, -62.0329, 0, 0, 0, NULL, 100, 0),
(381010, 12, 1395.9, 94.0168, -62.2829, 0, 0, 0, NULL, 100, 0),
(381010, 13, 1400.16, 97.4923, -62.0575, 0, 0, 0, NULL, 100, 0),
(381010, 14, 1412.66, 127.992, -62.0575, 0, 0, 0, NULL, 100, 0),
(381010, 15, 1413.66, 130.242, -62.0575, 0, 0, 0, NULL, 100, 0),
(381010, 16, 1415.16, 134.242, -62.0575, 0, 0, 0, NULL, 100, 0),
(381010, 17, 1418.15, 141.239, -62.0407, 0, 0, 0, NULL, 100, 0),
(381010, 18, 1423.4, 143.739, -62.0407, 0, 0, 0, NULL, 100, 0),
(381010, 19, 1434.17, 149.965, -62.0399, 0, 0, 0, NULL, 100, 0),
(381010, 20, 1435.67, 148.465, -62.0399, 0, 0, 0, NULL, 100, 0),
(381010, 21, 1445.42, 139.465, -62.0399, 0, 0, 0, NULL, 100, 0),
(381010, 22, 1447.84, 137.207, -61.7518, 0, 0, 0, NULL, 100, 0),
(381010, 23, 1450.84, 133.707, -61.7518, 0, 0, 0, NULL, 100, 0),
(381010, 24, 1452.09, 132.457, -61.7518, 0, 0, 0, NULL, 100, 0),
(381010, 25, 1453.84, 130.457, -60.0018, 0, 0, 0, NULL, 100, 0),
(381010, 26, 1456.34, 127.457, -60.0018, 0, 0, 0, NULL, 100, 0),
(381010, 27, 1462.01, 120.872, -59.9412, 0, 0, 0, NULL, 100, 0),
(381010, 28, 1465.15, 111.251, -59.9415, 0, 0, 0, NULL, 100, 0),
(381010, 29, 1471.4, 105.751, -59.9415, 0, 0, 0, NULL, 100, 0),
(381010, 30, 1479.85, 98.1165, -59.9706, 0, 0, 0, NULL, 100, 0),
(381010, 31, 1480.85, 97.3665, -59.9706, 0, 0, 0, NULL, 100, 0),
(381010, 32, 1482.85, 96.1165, -61.4706, 0, 0, 0, NULL, 100, 0),
(381010, 33, 1484.35, 95.1165, -61.7206, 0, 0, 0, NULL, 100, 0),
(381010, 34, 1483.16, 95.7806, -61.6284, 0, 0, 0, NULL, 100, 0),
(381010, 35, 1484.66, 94.7806, -61.8784, 0, 0, 0, NULL, 100, 0),
(381010, 36, 1485.16, 94.5306, -61.8784, 0, 0, 0, NULL, 100, 0),
(381010, 37, 1489.66, 90.7806, -61.8784, 0, 0, 0, NULL, 100, 0),
(381010, 38, 1499.66, 82.2806, -61.8784, 0, 0, 0, NULL, 100, 0),
(381010, 39, 1499.69, 82.068, -62.047, 0, 0, 0, NULL, 100, 0),
(381010, 40, 1500.44, 81.318, -62.047, 0, 0, 0, NULL, 100, 0),
(381010, 41, 1494.94, 71.318, -62.047, 0, 0, 0, NULL, 100, 0),
(381010, 42, 1481.83, 47.6647, -62.0359, 0, 0, 0, NULL, 100, 0),
(381010, 43, 1469.58, 44.1647, -62.0359, 0, 0, 0, NULL, 100, 0),
(381010, 44, 1466.58, 43.4147, -62.0359, 0, 0, 0, NULL, 100, 0),
(381010, 45, 1464.83, 42.9147, -62.0359, 0, 0, 0, NULL, 100, 0),
(381010, 46, 1452.33, 39.1647, -62.0359, 0, 0, 0, NULL, 100, 0),
(381010, 47, 1442.51, 36.291, -62.0262, 0, 0, 0, NULL, 100, 0),
(381010, 48, 1435.51, 32.291, -62.0262, 0, 0, 0, NULL, 100, 0),
(381010, 49, 1416.5, 21.4253, -62.0259, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (38101, 381010, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 38421 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1038245 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1038245 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1038245 AND spawnType = 0 WHERE ce.spawnID = 1038245;
INSERT INTO creature_entry (spawnID, entry) VALUES (38421, 4571);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (38421, 0, 1, 2640, 1, 1764.38, 120.86,  -62.2913, 0.773067, 300, 0, 0, 1003, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(384210, 1, 1759.66, 131.193, -62.297, 0, 0, 0, NULL, 100, 0),
(384210, 2, 1748.01, 137.403, -62.2873, 0, 0, 0, NULL, 100, 0),
(384210, 3, 1740.99, 133.782, -62.1428, 0, 0, 0, NULL, 100, 0),
(384210, 4, 1734, 128.82, -60.1267, 0, 0, 0, NULL, 100, 0),
(384210, 5, 1728.17, 124.137, -60.3366, 0, 0, 0, NULL, 100, 0),
(384210, 6, 1721.19, 117.524, -55.2157, 0, 0, 0, NULL, 100, 0),
(384210, 7, 1717.06, 115.939, -55.2152, 0, 0, 0, NULL, 100, 0),
(384210, 8, 1706.28, 127.667, -48.9426, 0, 0, 0, NULL, 100, 0),
(384210, 9, 1694.08, 139.229, -55.2145, 0, 0, 0, NULL, 100, 0),
(384210, 10, 1695.94, 143.22, -55.2145, 0, 0, 0, NULL, 100, 0),
(384210, 11, 1701.63, 150.057, -60.4713, 0, 0, 0, NULL, 100, 0),
(384210, 12, 1700.79, 156.415, -60.4486, 0, 0, 0, NULL, 100, 0),
(384210, 13, 1706.71, 173.641, -60.7377, 0, 0, 0, NULL, 100, 0),
(384210, 14, 1701.23, 179.241, -62.1717, 0, 0, 0, NULL, 100, 0),
(384210, 15, 1689.14, 177.471, -62.1717, 0, 0, 0, NULL, 100, 0),
(384210, 16, 1682.12, 173.618, -62.1501, 0, 0, 0, NULL, 100, 0),
(384210, 17, 1679.01, 170.419, -62.1451, 0, 0, 0, NULL, 100, 0),
(384210, 18, 1664.06, 155.13, -62.1506, 0, 0, 0, NULL, 100, 0),
(384210, 19, 1660.32, 151.225, -62.159, 0, 0, 0, NULL, 100, 0),
(384210, 20, 1656.65, 142.33, -62.1464, 0, 0, 0, NULL, 100, 0),
(384210, 21, 1661.99, 130.196, -61.4884, 0, 0, 0, NULL, 100, 0),
(384210, 22, 1676.1, 127.901, -60.3797, 0, 0, 0, NULL, 100, 0),
(384210, 23, 1683.65, 131.558, -60.4389, 0, 0, 0, NULL, 100, 0),
(384210, 24, 1691.37, 139.089, -55.2145, 0, 0, 0, NULL, 100, 0),
(384210, 25, 1694.58, 138.998, -55.2145, 0, 0, 0, NULL, 100, 0),
(384210, 26, 1705.71, 127.613, -48.9109, 0, 0, 0, NULL, 100, 0),
(384210, 27, 1716.68, 116.095, -55.2151, 0, 0, 0, NULL, 100, 0),
(384210, 28, 1715.67, 112.278, -55.2156, 0, 0, 0, NULL, 100, 0),
(384210, 29, 1709.76, 105.679, -60.2417, 0, 0, 0, NULL, 100, 0),
(384210, 30, 1697.48, 93.3407, -62.1885, 0, 0, 0, NULL, 100, 0),
(384210, 31, 1694.8, 80.5407, -62.2903, 0, 0, 0, NULL, 100, 0),
(384210, 32, 1700.17, 73.6353, -62.2899, 0, 0, 0, NULL, 100, 0),
(384210, 33, 1713.85, 66.6249, -62.2861, 0, 0, 0, NULL, 100, 0),
(384210, 34, 1733.04, 77.4807, -62.2728, 0, 0, 0, NULL, 100, 0),
(384210, 35, 1744.22, 91.9121, -62.2754, 0, 0, 0, NULL, 100, 0),
(384210, 36, 1757.18, 102.997, -62.277, 0, 0, 0, NULL, 100, 0),
(384210, 37, 1764.38, 120.86, -62.2913, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (38421, 384210, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 38422 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1038246 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1038246 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1038246 AND spawnType = 0 WHERE ce.spawnID = 1038246;
DELETE FROM waypoint_data WHERE id = 382810;
INSERT INTO creature_entry (spawnID, entry) VALUES (38422, 4568);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (38422, 0, 1, 2657, 1, 1813.74, 56.4138,  -47.3038, 3.05433, 300, 0, 0, 1753, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (38422, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 38429 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1038253 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1038253 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1038253 AND spawnType = 0 WHERE ce.spawnID = 1038253;
INSERT INTO creature_entry (spawnID, entry) VALUES (38429, 5668);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (38429, 0, 1, 4004, 1, 1717.82, 303.114,  -61.4799, 1.54469, 300, 0, 0, 484, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(384290, 1, 1717.82, 303.114, -61.4799, 0, 0, 0, NULL, 100, 0),
(384290, 2, 1724.06, 285.519, -62.1839, 0, 0, 0, NULL, 100, 0),
(384290, 3, 1725.81, 279.24, -62.1822, 0, 0, 0, NULL, 100, 0),
(384290, 4, 1728.31, 268.62, -62.1777, 0, 0, 0, NULL, 100, 0),
(384290, 5, 1730.3, 250.8, -62.1777, 0, 0, 0, NULL, 100, 0),
(384290, 6, 1716.66, 240.302, -62.1777, 0, 0, 0, NULL, 100, 0),
(384290, 7, 1692.6, 241.784, -62.1777, 0, 0, 0, NULL, 100, 0),
(384290, 8, 1672.5, 242.941, -62.1777, 0, 0, 0, NULL, 100, 0),
(384290, 9, 1665.62, 249.509, -62.1778, 0, 0, 0, NULL, 100, 0),
(384290, 10, 1663.85, 264.295, -62.1782, 0, 0, 0, NULL, 100, 0),
(384290, 11, 1659.71, 273.889, -62.1794, 0, 0, 0, NULL, 100, 0),
(384290, 12, 1652.45, 286.605, -62.1813, 0, 0, 0, NULL, 100, 0),
(384290, 13, 1637.7, 299.425, -62.1758, 0, 0, 0, NULL, 100, 0),
(384290, 14, 1621.36, 308.351, -62.1777, 0, 0, 0, NULL, 100, 0),
(384290, 15, 1605.39, 311.208, -62.1777, 0, 0, 0, NULL, 100, 0),
(384290, 16, 1598.25, 318.912, -62.1777, 0, 0, 0, NULL, 100, 0),
(384290, 17, 1598.48, 334.921, -62.1777, 0, 0, 0, NULL, 100, 0),
(384290, 18, 1599.09, 358.207, -62.1777, 0, 0, 0, NULL, 100, 0),
(384290, 19, 1598.42, 367.154, -62.2217, 0, 0, 0, NULL, 100, 0),
(384290, 20, 1610.37, 374.763, -62.1777, 0, 0, 0, NULL, 100, 0),
(384290, 21, 1627.03, 371.827, -62.1777, 0, 0, 0, NULL, 100, 0),
(384290, 22, 1637.79, 368.53, -62.1685, 0, 0, 0, NULL, 100, 0),
(384290, 23, 1655.58, 358.96, -60.7415, 0, 0, 0, NULL, 100, 0),
(384290, 24, 1657.84, 352.998, -60.7289, 0, 0, 0, NULL, 100, 0),
(384290, 25, 1654.79, 342.431, -62.1717, 0, 0, 0, NULL, 100, 0),
(384290, 26, 1668.16, 325.349, -62.1557, 0, 0, 0, NULL, 100, 0),
(384290, 27, 1680.75, 314.66, -62.1566, 0, 0, 0, NULL, 100, 0),
(384290, 28, 1694.48, 302.044, -62.1636, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (38429, 384290, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 43909 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1043709 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1043709 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1043709 AND spawnType = 0 WHERE ce.spawnID = 1043709;
INSERT INTO creature_entry (spawnID, entry) VALUES (43909, 1910);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (43909, 0, 1, 2597, 1, 2560.56, 1384.3,  4.44448, 1.98845, 3600, 0, 0, 186, 286, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(439090, 1, 2560.87, 1383.37, 4.50395, 0, 0, 0, NULL, 100, 0),
(439090, 2, 2570.03, 1402.94, 1.54059, 0, 0, 0, NULL, 100, 0),
(439090, 3, 2579.45, 1429.17, -4.3321, 0, 0, 0, NULL, 100, 0),
(439090, 4, 2593.99, 1450.15, -7.89234, 0, 0, 0, NULL, 100, 0),
(439090, 5, 2601.62, 1446.89, -7.27309, 0, 0, 0, NULL, 100, 0),
(439090, 6, 2613.47, 1438.87, -6.89807, 0, 0, 0, NULL, 100, 0),
(439090, 7, 2624.67, 1438.41, -9.39612, 0, 0, 0, NULL, 100, 0),
(439090, 8, 2625.67, 1463.9, -10.579, 0, 0, 0, NULL, 100, 0),
(439090, 9, 2618.77, 1481.38, -13.1043, 0, 0, 0, NULL, 100, 0),
(439090, 10, 2602.46, 1474.33, -11.9274, 0, 0, 0, NULL, 100, 0),
(439090, 11, 2602.56, 1463.68, -10.5436, 0, 60000, 0, NULL, 100, 0),
(439090, 12, 2590.15, 1474.77, -10.7952, 0, 0, 0, NULL, 100, 0),
(439090, 13, 2564.63, 1506.97, -8.94545, 0, 0, 0, NULL, 100, 0),
(439090, 14, 2541.87, 1516.33, -4.3325, 0, 0, 0, NULL, 100, 0),
(439090, 15, 2524.31, 1512.05, -0.096567, 0, 0, 0, NULL, 100, 0),
(439090, 16, 2503.73, 1481.51, 0.811042, 0, 0, 0, NULL, 100, 0),
(439090, 17, 2502.97, 1468.14, 2.37526, 0, 0, 0, NULL, 100, 0),
(439090, 18, 2492.1, 1466.8, 4.67212, 0, 0, 0, NULL, 100, 0),
(439090, 19, 2488.31, 1453.82, 6.30721, 0, 0, 0, NULL, 100, 0),
(439090, 20, 2494.6, 1429.94, 6.60753, 0, 0, 0, NULL, 100, 0),
(439090, 21, 2497.23, 1393.23, 5.80512, 0, 0, 0, NULL, 100, 0),
(439090, 22, 2526.28, 1419, 1.64461, 0, 0, 0, NULL, 100, 0),
(439090, 23, 2534.23, 1414.35, 3.52403, 0, 0, 0, NULL, 100, 0),
(439090, 24, 2538.32, 1407.85, 5.59288, 0, 30000, 0, NULL, 100, 0),
(439090, 25, 2543.13, 1404.27, 5.74442, 0, 0, 0, NULL, 100, 0),
(439090, 26, 2539.93, 1402.86, 5.42545, 0, 0, 0, NULL, 100, 0),
(439090, 27, 2540.81, 1406.7, 5.88053, 0, 0, 0, NULL, 100, 0),
(439090, 28, 2546.03, 1402.95, 5.06959, 0, 0, 0, NULL, 100, 0),
(439090, 29, 2547.8, 1396.79, 4.25417, 0, 0, 0, NULL, 100, 0),
(439090, 30, 2555.93, 1388.16, 4.95441, 0, 0, 0, NULL, 100, 0),
(439090, 31, 2564.59, 1387.98, 3.86626, 0, 0, 0, NULL, 100, 0),
(439090, 32, 2552.19, 1391.92, 4.88426, 0, 0, 0, NULL, 100, 0),
(439090, 33, 2549.76, 1385.2, 5.93925, 0, 0, 0, NULL, 100, 0),
(439090, 34, 2559.06, 1382.17, 4.9502, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (43909, 439090, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 92848 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1091460 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1091460 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1091460 AND spawnType = 0 WHERE ce.spawnID = 1091460;
INSERT INTO creature_entry (spawnID, entry) VALUES (92848, 5634);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (92848, 0, 1, 3874, 1, 219.598, -2612.21,  160.449, 1.15192, 350, 0, 0, 2138, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (92848, NULL, 0, 0, 0, 4097, 93, NULL);

-- Importing creature with guid 92909 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1091520 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1091520 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1091520 AND spawnType = 0 WHERE ce.spawnID = 1091520;
INSERT INTO creature_entry (spawnID, entry) VALUES (92909, 14737);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (92909, 0, 1, 14769, 1, -589.954, -4548.46,  9.16149, 3.22886, 350, 0, 0, 1990, 0, 2, 0, 0, 10);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (929090, 1, 30, 0, 0, 0, 0, 0, 0, 3.22886);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (929090, 2, 2, 74, 8, 0, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (929090, 5, 2, 74, 0, 0, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (929091, 1, 30, 0, 0, 0, 0, 0, 0, 0.610865);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (929091, 2, 15, 23488, 1, 0, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (929091, 5, 2, 83, 233, 0, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (929091, 24, 2, 83, 69, 0, 0, 0, 0, 0);
INSERT INTO waypoint_data VALUES 
(929090, 1, -591.677, -4554.79, 9.16149, 0, 0, 0, NULL, 100, 0),
(929090, 2, -595.75, -4559.67, 9.16149, 0, 6000, 0, 929090, 100, 0),
(929090, 3, -589.954, -4548.46, 9.16149, 0, 29000, 0, 929091, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (92909, 929090, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 61634 with import type REPLACE_ALL
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1061029 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1061029 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1061029 AND spawnType = 0 WHERE ce.spawnID = 1061029;
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1098238 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1098238 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1098238 AND spawnType = 0 WHERE ce.spawnID = 1098238;
INSERT INTO creature_entry (spawnID, entry) VALUES (61634, 1885);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (61634, 0, 1, 0, 1, 2990.86, -1458.58,  146.273, 3.02789, 72000, 0, 0, 8613, 0, 2, 0, 0, 10);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (616340, 18, 1, 0, 1, 0, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (616340, 1, 1, 69, 1, 0, 0, 0, 0, 0);
INSERT INTO waypoint_data VALUES 
(616340, 1, 2990.86, -1458.58, 146.273, 0, 0, 0, NULL, 100, 0),
(616340, 2, 2978.03, -1450.18, 145.989, 0, 0, 0, NULL, 100, 0),
(616340, 3, 2962.46, -1452.65, 145.716, 0, 0, 0, NULL, 100, 0),
(616340, 4, 2963.32, -1464.52, 145.592, 0, 0, 0, NULL, 100, 0),
(616340, 5, 2973.34, -1468.99, 145.754, 0, 0, 0, NULL, 100, 0),
(616340, 6, 2974.3, -1475.57, 145.509, 0, 0, 0, NULL, 100, 0),
(616340, 7, 2974.67, -1484.54, 146.999, 0, 0, 0, NULL, 100, 0),
(616340, 8, 2975.15, -1490.23, 147.022, 0, 0, 0, NULL, 100, 0),
(616340, 9, 2984.02, -1491.19, 147.042, 6.18384, 20000, 0, 616340, 100, 0),
(616340, 10, 2974.94, -1498.83, 147.042, 3.80251, 20000, 0, 616340, 100, 0),
(616340, 11, 2978.95, -1500.69, 146.999, 0, 0, 0, NULL, 100, 0),
(616340, 12, 2979.92, -1509.63, 145.319, 0, 0, 0, NULL, 100, 0),
(616340, 13, 2996.73, -1508.11, 146.204, 0, 0, 0, NULL, 100, 0),
(616340, 14, 3004.9, -1514.71, 145.967, 0, 0, 0, NULL, 100, 0),
(616340, 15, 3005.29, -1522.01, 146.069, 0, 0, 0, NULL, 100, 0),
(616340, 16, 2992.68, -1516.82, 146.041, 0, 0, 0, NULL, 100, 0),
(616340, 17, 2996.49, -1507.15, 146.152, 0, 0, 0, NULL, 100, 0),
(616340, 18, 3004.37, -1496.56, 146.397, 0, 0, 0, NULL, 100, 0),
(616340, 19, 3005.32, -1481.19, 145.908, 0, 0, 0, NULL, 100, 0),
(616340, 20, 3008.27, -1470.62, 146.531, 0, 0, 0, NULL, 100, 0),
(616340, 21, 3002.19, -1461.06, 146.273, 0, 0, 0, NULL, 100, 0),
(616340, 22, 2998.02, -1460.04, 145.81, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (61634, 616340, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 63913 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1063260 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1063260 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1063260 AND spawnType = 0 WHERE ce.spawnID = 1063260;
INSERT INTO creature_entry (spawnID, entry) VALUES (63913, 2258);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (63913, 0, 1, 0, 0, 664.315, -1027.59,  160.39, 5.21757, 72000, 0, 0, 1536, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(639130, 1, 664.315, -1027.59, 160.39, 0, 0, 0, NULL, 100, 0),
(639130, 2, 654.02, -1032.34, 163.127, 0, 0, 0, NULL, 100, 0),
(639130, 3, 634.638, -1041.36, 163.816, 0, 0, 0, NULL, 100, 0),
(639130, 4, 622.304, -1055.48, 163.186, 0, 0, 0, NULL, 100, 0),
(639130, 5, 620.168, -1114.91, 160.186, 0, 0, 0, NULL, 100, 0),
(639130, 6, 627.218, -1131.61, 159.535, 0, 0, 0, NULL, 100, 0),
(639130, 7, 631.939, -1149.35, 155.421, 0, 0, 0, NULL, 100, 0),
(639130, 8, 629.934, -1192.34, 145.181, 0, 0, 0, NULL, 100, 0),
(639130, 9, 633.981, -1229.12, 136.877, 0, 0, 0, NULL, 100, 0),
(639130, 10, 634.73, -1271.43, 121.429, 0, 0, 0, NULL, 100, 0),
(639130, 11, 627.306, -1321.81, 106.197, 0, 0, 0, NULL, 100, 0),
(639130, 12, 629.422, -1375.57, 93.9589, 0, 0, 0, NULL, 100, 0),
(639130, 13, 643.925, -1408.88, 87.1831, 0, 0, 0, NULL, 100, 0),
(639130, 14, 668.443, -1437.6, 81.8552, 0, 0, 0, NULL, 100, 0),
(639130, 15, 708.689, -1455.94, 81.4619, 0, 0, 0, NULL, 100, 0),
(639130, 16, 745.689, -1459.7, 80.3421, 0, 0, 0, NULL, 100, 0),
(639130, 17, 708.769, -1455.86, 81.4599, 0, 0, 0, NULL, 100, 0),
(639130, 18, 667.903, -1437.37, 81.9153, 0, 0, 0, NULL, 100, 0),
(639130, 19, 643.733, -1408.63, 87.2146, 0, 0, 0, NULL, 100, 0),
(639130, 20, 629.19, -1375.19, 94.0446, 0, 0, 0, NULL, 100, 0),
(639130, 21, 627.374, -1321.59, 106.257, 0, 0, 0, NULL, 100, 0),
(639130, 22, 634.783, -1271.27, 121.478, 0, 0, 0, NULL, 100, 0),
(639130, 23, 633.98, -1228.57, 137.007, 0, 0, 0, NULL, 100, 0),
(639130, 24, 629.947, -1191.76, 145.328, 0, 0, 0, NULL, 100, 0),
(639130, 25, 631.939, -1148.87, 155.504, 0, 0, 0, NULL, 100, 0),
(639130, 26, 627.116, -1131.07, 159.662, 0, 0, 0, NULL, 100, 0),
(639130, 27, 620.092, -1114.7, 160.189, 0, 0, 0, NULL, 100, 0),
(639130, 28, 622.218, -1054.98, 163.22, 0, 0, 0, NULL, 100, 0),
(639130, 29, 634.789, -1041.15, 163.826, 0, 0, 0, NULL, 100, 0),
(639130, 30, 653.802, -1032.34, 163.225, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (63913, 639130, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 90511 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1089129 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1089129 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1089129 AND spawnType = 0 WHERE ce.spawnID = 1089129;
INSERT INTO creature_entry (spawnID, entry) VALUES (90511, 14223);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (90511, 0, 1, 0, 0, 1153.34, -262.649,  33.1925, 3.13032, 72000, 0, 0, 1163, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(905110, 1, 1129.87, -260.801, 33.1925, 0, 0, 0, NULL, 100, 0),
(905110, 2, 1099.4, -246.603, 33.2646, 0, 0, 0, NULL, 100, 0),
(905110, 3, 1043.01, -218.998, 32.6469, 0, 0, 0, NULL, 100, 0),
(905110, 4, 987.828, -183.095, 32.6947, 0, 0, 0, NULL, 100, 0),
(905110, 5, 944.295, -133.382, 32.326, 0, 0, 0, NULL, 100, 0),
(905110, 6, 896.154, -88.7433, 32.8252, 0, 0, 0, NULL, 100, 0),
(905110, 7, 822.507, -69.9838, 32.5853, 0, 0, 0, NULL, 100, 0),
(905110, 8, 777.692, -43.3707, 33.055, 0, 0, 0, NULL, 100, 0),
(905110, 9, 724.597, 15.5293, 32.9236, 0, 0, 0, NULL, 100, 0),
(905110, 10, 681.48, 86.9326, 32.6039, 0, 0, 0, NULL, 100, 0),
(905110, 11, 614.492, 189.908, 33.3183, 0, 0, 0, NULL, 100, 0),
(905110, 12, 555.051, 231.309, 33.459, 0, 0, 0, NULL, 100, 0),
(905110, 13, 517.042, 274.988, 33.0772, 0, 0, 0, NULL, 100, 0),
(905110, 14, 525.949, 251.626, 33.3941, 0, 0, 0, NULL, 100, 0),
(905110, 15, 569.713, 221.093, 31.6346, 0, 0, 0, NULL, 100, 0),
(905110, 16, 632.957, 163.248, 32.4543, 0, 0, 0, NULL, 100, 0),
(905110, 17, 667.423, 98.5867, 33.6662, 0, 0, 0, NULL, 100, 0),
(905110, 18, 711.496, 41.1962, 31.6577, 0, 0, 0, NULL, 100, 0),
(905110, 19, 745.464, -14.5894, 33.0865, 0, 0, 0, NULL, 100, 0),
(905110, 20, 801.702, -56.7232, 31.9294, 0, 0, 0, NULL, 100, 0),
(905110, 21, 869.771, -83.0213, 32.9922, 0, 0, 0, NULL, 100, 0),
(905110, 22, 933.646, -117.308, 31.9558, 0, 0, 0, NULL, 100, 0),
(905110, 23, 962.605, -156.086, 33.3342, 0, 0, 0, NULL, 100, 0),
(905110, 24, 1031.77, -216.432, 33.2716, 0, 0, 0, NULL, 100, 0),
(905110, 25, 1118.48, -252.114, 31.7163, 0, 0, 0, NULL, 100, 0),
(905110, 26, 1186.55, -265.553, 33.9631, 0, 0, 0, NULL, 100, 0),
(905110, 27, 1153.34, -262.649, 33.1925, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (90511, 905110, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 90783 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1089401 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1089401 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1089401 AND spawnType = 0 WHERE ce.spawnID = 1089401;
INSERT INTO creature_entry (spawnID, entry) VALUES (90783, 14275);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (90783, 0, 1, 0, 1, -1316.82, -1212.59,  49.9032, 0.681901, 72000, 0, 0, 2196, 756, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(907830, 1, -1316.82, -1212.59, 49.9032, 0, 0, 0, NULL, 100, 0),
(907830, 2, -1353.08, -1242.71, 49.9048, 0, 0, 0, NULL, 100, 0),
(907830, 3, -1357.06, -1248.49, 49.9033, 0, 0, 0, NULL, 100, 0),
(907830, 4, -1329.89, -1278.96, 49.9017, 0, 0, 0, NULL, 100, 0),
(907830, 5, -1289.34, -1243.49, 49.9032, 0, 0, 0, NULL, 100, 0),
(907830, 6, -1331.07, -1278.83, 49.9017, 0, 0, 0, NULL, 100, 0),
(907830, 7, -1357.24, -1246.72, 49.9025, 0, 0, 0, NULL, 100, 0),
(907830, 8, -1334.4, -1227.01, 49.9047, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (90783, 907830, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 68938 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1068178 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1068178 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1068178 AND spawnType = 0 WHERE ce.spawnID = 1068178;
INSERT INTO creature_entry (spawnID, entry) VALUES (68938, 19911);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (68938, 0, 1, 0, 0, -8401.95, 264.45,  120.886, 0.932609, 250, 0, 0, 6986, 0, 0, 0, 0, 10);

-- Importing creature with guid 212043 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (212043, 25909);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (212043, 0, 1, 16339, 0, 1008.57, -1452.78,  61.661, 5.27254, 300, 0, 0, 42, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (212043, 1);

-- Importing creature with guid 211790 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (211790, 25892);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (211790, 0, 1, 0, 0, -8254.28, -2625.62,  133.159, 1.29179, 300, 0, 0, 1524, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (211790, 1);

-- Importing creature with guid 120784 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (120784, 29093);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (120784, 0, 1, 25901, 0, -8807.04, 638.359,  94.312, 3.56047, 180, 0, 0, 1220, 0, 0, 0, 0, 10);

-- Importing creature with guid 120801 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (120801, 25058);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (120801, 0, 1, 5585, 0, -8864.59, 670.196,  106.072, 1.32645, 180, 0, 0, 4, 0, 0, 0, 0, 10);

-- Importing creature with guid 12776 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (12776, 14842);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (12776, 0, 1, 0, 0, -4982.69, -963.739,  501.659, 0.802197, 300, 0, 0, 955, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(127760, 1, -4982.68, -963.738, 501.659, 0.802197, 0, 0, NULL, 100, 0),
(127760, 2, -4966.25, -948.009, 501.659, 0.852462, 0, 0, NULL, 100, 0),
(127760, 3, -4947.42, -931.671, 501.659, 0.694597, 0, 0, NULL, 100, 0),
(127760, 4, -4922.59, -910.984, 501.659, 0.694597, 0, 0, NULL, 100, 0),
(127760, 5, -4900.64, -898.418, 501.659, 0.539874, 0, 0, NULL, 100, 0),
(127760, 6, -4875.92, -890.53, 501.659, 0.228856, 0, 0, NULL, 100, 0),
(127760, 7, -4855.47, -885.766, 501.659, 0.228856, 0, 0, NULL, 100, 0),
(127760, 8, -4835.2, -884.038, 501.659, 0.042716, 0, 0, NULL, 100, 0),
(127760, 9, -4820.52, -883.41, 501.659, 0.042716, 0, 0, NULL, 100, 0),
(127760, 10, -4800.39, -884.07, 501.663, 6.2505, 0, 0, NULL, 100, 0),
(127760, 11, -4784.24, -885.432, 501.659, 6.17275, 0, 0, NULL, 100, 0),
(127760, 12, -4763.38, -889.904, 501.659, 6.05494, 0, 0, NULL, 100, 0),
(127760, 13, -4748.81, -894.673, 501.659, 5.85938, 0, 0, NULL, 100, 0),
(127760, 14, -4749.41, -905.275, 501.659, 4.65615, 0, 0, NULL, 100, 0),
(127760, 15, -4753.94, -909.319, 501.659, 3.87075, 0, 0, NULL, 100, 0),
(127760, 16, -4769.22, -906.058, 501.663, 2.93141, 0, 0, NULL, 100, 0),
(127760, 17, -4775.83, -907.238, 501.661, 3.24243, 0, 0, NULL, 100, 0),
(127760, 18, -4784.78, -907.646, 497.954, 3.08456, 0, 0, NULL, 100, 0),
(127760, 19, -4804.25, -904.848, 497.954, 2.96675, 0, 0, NULL, 100, 0),
(127760, 20, -4821.49, -901.804, 501, 2.96675, 0, 0, NULL, 100, 0),
(127760, 21, -4844.94, -907.025, 501.66, 3.36102, 0, 0, NULL, 100, 0),
(127760, 22, -4865.21, -911.545, 501.66, 3.36102, 0, 0, NULL, 100, 0),
(127760, 23, -4875.69, -913.882, 501.66, 3.36102, 0, 0, NULL, 100, 0),
(127760, 24, -4890.25, -921.527, 501.623, 3.62492, 0, 0, NULL, 100, 0),
(127760, 25, -4913.38, -941.166, 501.528, 3.8464, 0, 0, NULL, 100, 0),
(127760, 26, -4932.3, -957.266, 501.553, 3.8464, 0, 0, NULL, 100, 0),
(127760, 27, -4945.19, -968.223, 501.568, 3.8464, 0, 0, NULL, 100, 0),
(127760, 28, -4956.21, -977.597, 501.609, 3.8464, 0, 0, NULL, 100, 0),
(127760, 29, -4966.41, -987.795, 501.657, 4.00191, 0, 0, NULL, 100, 0),
(127760, 30, -4975.4, -998.238, 501.657, 4.00191, 0, 0, NULL, 100, 0),
(127760, 31, -4981.95, -1012.55, 501.655, 4.31528, 0, 0, NULL, 100, 0),
(127760, 32, -4987.98, -1032.34, 501.654, 4.4378, 0, 0, NULL, 100, 0),
(127760, 33, -4991.24, -1043.91, 500.679, 4.4378, 0, 0, NULL, 100, 0),
(127760, 34, -4991.98, -1052.34, 497.937, 4.55639, 0, 0, NULL, 100, 0),
(127760, 35, -4993.67, -1063.06, 497.963, 4.55639, 0, 0, NULL, 100, 0),
(127760, 36, -4994.55, -1080.96, 497.972, 4.60587, 0, 0, NULL, 100, 0),
(127760, 37, -4995.38, -1088.74, 501.638, 4.60587, 0, 0, NULL, 100, 0),
(127760, 38, -4996.99, -1103.82, 501.659, 4.60587, 0, 0, NULL, 100, 0),
(127760, 39, -4997.76, -1111, 501.663, 4.60587, 0, 0, NULL, 100, 0),
(127760, 40, -5010.9, -1113.08, 501.668, 3.29897, 0, 0, NULL, 100, 0),
(127760, 41, -5014.93, -1102.52, 501.671, 1.93552, 0, 0, NULL, 100, 0),
(127760, 42, -5015.86, -1081.45, 501.68, 1.60801, 0, 0, NULL, 100, 0),
(127760, 43, -5014.07, -1055.77, 501.724, 1.49177, 0, 0, NULL, 100, 0),
(127760, 44, -5011.75, -1042.1, 501.712, 1.37396, 0, 0, NULL, 100, 0),
(127760, 45, -5003.69, -1018.41, 501.656, 1.20824, 0, 0, NULL, 100, 0),
(127760, 46, -4998.19, -1003.91, 501.658, 1.20824, 0, 0, NULL, 100, 0),
(127760, 47, -4990.7, -984.164, 501.659, 1.20824, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (12776, 127760, 0, 0, 0, 0, 0, NULL);
INSERT INTO game_event_creature (guid, event) VALUES (12776, 4);

-- Importing creature with guid 12776 with import type MOVE_UNIQUE_IMPORT_WP

-- Importing creature with guid 207170 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (207170, 24364);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (207170, 0, 1, 0, 1, -5609.83, -459.056,  404.633, 5.3058, 120, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (207170, 26);

-- Importing creature with guid 134380 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (134380, 7846);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (134380, 0, 1, 0, 0, -11685.9, -2955.42,  7.736, 1.48164, 72000, 0, 0, 91560, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(1343800, 1, -11685.9, -2955.42, 7.736, 0, 0, 0, NULL, 100, 0),
(1343800, 2, -11704.9, -2918.86, 6.45326, 0, 0, 0, NULL, 100, 0),
(1343800, 3, -11721.9, -2867.16, 3.8946, 0, 0, 0, NULL, 100, 0),
(1343800, 4, -11692.8, -2852.44, 4.11769, 0, 0, 0, NULL, 100, 0),
(1343800, 5, -11654.9, -2802.53, 4.68803, 0, 0, 0, NULL, 100, 0),
(1343800, 6, -11594.6, -2801.7, 3.20341, 0, 0, 0, NULL, 100, 0),
(1343800, 7, -11578.9, -2802.17, 4.60269, 0, 0, 0, NULL, 100, 0),
(1343800, 8, -11561.7, -2802.68, 6.52035, 0, 0, 0, NULL, 100, 0),
(1343800, 9, -11518.3, -2803.98, 0.488808, 0, 0, 0, NULL, 100, 0),
(1343800, 10, -11468.7, -2803.11, -1.62479, 0, 0, 0, NULL, 100, 0),
(1343800, 11, -11441.5, -2832.68, -0.937041, 0, 0, 0, NULL, 100, 0),
(1343800, 12, -11421.9, -2859.63, 1.48494, 0, 0, 0, NULL, 100, 0),
(1343800, 13, -11414.2, -2880.72, 2.28805, 0, 0, 0, NULL, 100, 0),
(1343800, 14, -11404.8, -2929.37, 2.55446, 0, 0, 0, NULL, 100, 0),
(1343800, 15, -11392.8, -2992.83, -0.732148, 0, 0, 0, NULL, 100, 0),
(1343800, 16, -11386.5, -3007.38, -1.37165, 0, 0, 0, NULL, 100, 0),
(1343800, 17, -11382.6, -3016.46, -4.43486, 0, 0, 0, NULL, 100, 0),
(1343800, 18, -11373.4, -3037.97, -4.56898, 0, 0, 0, NULL, 100, 0),
(1343800, 19, -11362.3, -3063.66, -4.41712, 0, 0, 0, NULL, 100, 0),
(1343800, 20, -11357.1, -3075.77, -0.351967, 0, 0, 0, NULL, 100, 0),
(1343800, 21, -11328.8, -3083.39, 1.96921, 0, 0, 0, NULL, 100, 0),
(1343800, 22, -11316.1, -3085.65, 3.2105, 0, 0, 0, NULL, 100, 0),
(1343800, 23, -11287.7, -3120.03, 3.83791, 0, 0, 0, NULL, 100, 0),
(1343800, 24, -11251.2, -3120.18, 2.41064, 0, 0, 0, NULL, 100, 0),
(1343800, 25, -11234.2, -3110.42, 4.85904, 0, 0, 0, NULL, 100, 0),
(1343800, 26, -11200.2, -3089.59, 8.21009, 0, 0, 0, NULL, 100, 0),
(1343800, 27, -11182.9, -3058.21, 9.40781, 0, 0, 0, NULL, 100, 0),
(1343800, 28, -11183.2, -3018.53, 7.36377, 0, 0, 0, NULL, 100, 0),
(1343800, 29, -11148.3, -2985.85, 8.42881, 0, 0, 0, NULL, 100, 0),
(1343800, 30, -11120.2, -2949.3, 8.01267, 0, 0, 0, NULL, 100, 0),
(1343800, 31, -11101.2, -2939.86, 9.51159, 0, 0, 0, NULL, 100, 0),
(1343800, 32, -11095.8, -2924.6, 11.0533, 0, 0, 0, NULL, 100, 0),
(1343800, 33, -11083.4, -2891.39, 9.60061, 0, 0, 0, NULL, 100, 0),
(1343800, 34, -11073.1, -2866.13, 10.1398, 0, 0, 0, NULL, 100, 0),
(1343800, 35, -11064.5, -2849.33, 11.4183, 0, 0, 0, NULL, 100, 0),
(1343800, 36, -11049.5, -2837.42, 11.8948, 0, 0, 0, NULL, 100, 0),
(1343800, 37, -11043.8, -2832.82, 13.3134, 0, 0, 0, NULL, 100, 0),
(1343800, 38, -11037.4, -2827.78, 14.5429, 0, 0, 0, NULL, 100, 0),
(1343800, 39, -11030.5, -2822.24, 11.5598, 0, 0, 0, NULL, 100, 0),
(1343800, 40, -11021.7, -2815.25, 6.9271, 0, 0, 0, NULL, 100, 0),
(1343800, 41, -11004, -2801.13, 5.79904, 0, 0, 0, NULL, 100, 0),
(1343800, 42, -11013.1, -2775.63, 4.03772, 0, 0, 0, NULL, 100, 0),
(1343800, 43, -11025.7, -2775.42, 3.99298, 0, 0, 0, NULL, 100, 0),
(1343800, 44, -11055.4, -2789.81, 7.18522, 0, 0, 0, NULL, 100, 0),
(1343800, 45, -11067.3, -2780.71, 8.11705, 0, 0, 0, NULL, 100, 0),
(1343800, 46, -11084.6, -2767.57, 11.7554, 0, 0, 0, NULL, 100, 0),
(1343800, 47, -11109.8, -2755.54, 16.4986, 0, 0, 0, NULL, 100, 0),
(1343800, 48, -11138.8, -2739.77, 14.0536, 0, 0, 0, NULL, 100, 0),
(1343800, 49, -11177.6, -2739.13, 15.8042, 0, 0, 0, NULL, 100, 0),
(1343800, 50, -11214.6, -2738.52, 15.6535, 0, 0, 0, NULL, 100, 0),
(1343800, 51, -11237.7, -2727.9, 14.0062, 0, 0, 0, NULL, 100, 0),
(1343800, 52, -11253.9, -2728.6, 13.6018, 0, 0, 0, NULL, 100, 0),
(1343800, 53, -11273.1, -2729.42, 11.1961, 0, 0, 0, NULL, 100, 0),
(1343800, 54, -11313.8, -2740.82, 9.15918, 0, 0, 0, NULL, 100, 0),
(1343800, 55, -11338.3, -2762.38, 8.9837, 0, 0, 0, NULL, 100, 0),
(1343800, 56, -11350.4, -2800.78, 6.74236, 0, 0, 0, NULL, 100, 0),
(1343800, 57, -11365.3, -2847.95, 5.40127, 0, 0, 0, NULL, 100, 0),
(1343800, 58, -11396.5, -2872.61, 3.28905, 0, 0, 0, NULL, 100, 0),
(1343800, 59, -11416.6, -2888.47, 2.56652, 0, 0, 0, NULL, 100, 0),
(1343800, 60, -11422, -2896.06, 4.96626, 0, 0, 0, NULL, 100, 0),
(1343800, 61, -11425.9, -2927.06, 3.56432, 0, 0, 0, NULL, 100, 0),
(1343800, 62, -11430.5, -2964.42, 3.03477, 0, 0, 0, NULL, 100, 0),
(1343800, 63, -11462.2, -3005.25, 4.60708, 0, 0, 0, NULL, 100, 0),
(1343800, 64, -11465, -3031.89, 3.43886, 0, 0, 0, NULL, 100, 0),
(1343800, 65, -11465.2, -3043.88, 1.67074, 0, 0, 0, NULL, 100, 0),
(1343800, 66, -11473.9, -3055, 2.22994, 0, 0, 0, NULL, 100, 0),
(1343800, 67, -11481.6, -3064.77, 1.61777, 0, 0, 0, NULL, 100, 0),
(1343800, 68, -11475.1, -3084.53, 2.58828, 0, 0, 0, NULL, 100, 0),
(1343800, 69, -11468.7, -3104.08, 3.9277, 0, 0, 0, NULL, 100, 0),
(1343800, 70, -11483.1, -3127.29, 7.02215, 0, 0, 0, NULL, 100, 0),
(1343800, 71, -11500.7, -3130.29, 6.24468, 0, 0, 0, NULL, 100, 0),
(1343800, 72, -11519.2, -3138.46, 6.11397, 0, 0, 0, NULL, 100, 0),
(1343800, 73, -11526.6, -3151.09, 3.83939, 0, 0, 0, NULL, 100, 0),
(1343800, 74, -11540.6, -3174.95, 5.03185, 0, 0, 0, NULL, 100, 0),
(1343800, 75, -11556.5, -3188.08, 7.6258, 0, 0, 0, NULL, 100, 0),
(1343800, 76, -11569.7, -3188.44, 11.5304, 0, 0, 0, NULL, 100, 0),
(1343800, 77, -11571.2, -3181.64, 10.0935, 0, 0, 0, NULL, 100, 0),
(1343800, 78, -11576.3, -3170.74, 4.83658, 0, 0, 0, NULL, 100, 0),
(1343800, 79, -11581.3, -3160.02, 5.57895, 0, 0, 0, NULL, 100, 0),
(1343800, 80, -11587, -3147.63, 4.36948, 0, 0, 0, NULL, 100, 0),
(1343800, 81, -11591.9, -3137.06, 5.80185, 0, 0, 0, NULL, 100, 0),
(1343800, 82, -11612.2, -3089.67, 8.39702, 0, 0, 0, NULL, 100, 0),
(1343800, 83, -11633.3, -3067.21, 10.5188, 0, 0, 0, NULL, 100, 0),
(1343800, 84, -11648.6, -3050.89, 10.2593, 0, 0, 0, NULL, 100, 0),
(1343800, 85, -11661.6, -3027.83, 8.65773, 0, 0, 0, NULL, 100, 0),
(1343800, 86, -11683.1, -2999.68, 7.70751, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (134380, 1343800, 0, 0, 0, 0, 0, NULL);

-- Importing creature with guid 49591 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 49591 WHERE entry = 24729;
UPDATE conditions SET ConditionValue3 = 49591 WHERE ConditionValue3 = 1098243 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -49591 WHERE SourceEntry = -1098243 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -49591 WHERE entryorguid = -1098243 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 49591 WHERE target_param1 = 1098243 AND target_type = 10;
UPDATE spawn_group SET spawnID = 49591 WHERE spawnID = 1098243 AND spawnType = 0;

-- Importing creature with guid 42880 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1095904 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1095904 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1095904 AND spawnType = 0 WHERE ce.spawnID = 1095904;
DELETE FROM waypoint_data WHERE id = 4478012;
INSERT INTO creature_entry (spawnID, entry) VALUES (42880, 12900);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (42880, 0, 1, 0, 0, -10666, -3923,  18.438, 6.21831, 300, 0, 0, 38844, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(428800, 1, -10666, -3923, 18.438, 0, 0, 0, NULL, 100, 0),
(428800, 2, -10674.5, -3909.81, 17.3732, 0, 0, 0, NULL, 100, 0),
(428800, 3, -10703.7, -3916.17, 17.3732, 0, 0, 0, NULL, 100, 0),
(428800, 4, -10770, -4007.98, 17.9099, 0, 0, 0, NULL, 100, 0),
(428800, 5, -10722.1, -4038.75, 17.6482, 0, 0, 0, NULL, 100, 0),
(428800, 6, -10654.9, -4061.03, 17.3742, 0, 0, 0, NULL, 100, 0),
(428800, 7, -10591.6, -4027.09, 17.3742, 0, 0, 0, NULL, 100, 0),
(428800, 8, -10568.6, -4002.12, 17.3853, 0, 0, 0, NULL, 100, 0),
(428800, 9, -10575.8, -3978.89, 17.4333, 0, 0, 0, NULL, 100, 0),
(428800, 10, -10624.2, -3924.85, 17.5842, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (42880, 428800, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 42880);

-- Importing creature with guid 88299 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (88299, 29095);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (88299, 0, 1, 25902, 0, 1581.83, 196.686,  -43.1022, 2.25663, 180, 0, 0, 1220, 0, 0, 0, 0, 10);

-- Importing creature with guid 77865 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (77865, 1849);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (77865, 0, 1, 0, 0, 1752.82, -2333.55,  59.8002, 2.51818, 72000, 0, 0, 0, 0, 0, 0, 0, 10);

-- Importing creature with guid 77864 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (77864, 10818);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (77864, 0, 1, 0, 1, 3022.93, -4942.99,  100.735, 3.02004, 72000, 8, 0, 0, 0, 1, 0, 0, 10);

-- Importing creature with guid 77815 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (77815, 10819);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (77815, 0, 1, 0, 1, 3122.37, -3725.47,  133.101, 3.1512, 72000, 0, 0, 0, 0, 0, 0, 0, 10);

-- Importing creature with guid 202758 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (202758, 25946);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (202758, 0, 1, 0, 0, 2278.37, 447.68,  34.0328, 3.14884, 120, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (202758, 1);

-- Importing creature with guid 203521 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1096010 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1096010 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1096010 AND spawnType = 0 WHERE ce.spawnID = 1096010;
INSERT INTO creature_entry (spawnID, entry) VALUES (203521, 15077);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (203521, 0, 1, 0, 1, -14430.4, 455.763,  3.69061, 4.41001, 300, 0, 0, 2215, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (203521, 82);

-- Importing creature with guid 209028 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (209028, 15119);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (209028, 0, 1, 0, 1, -4964.69, -936.287,  501.743, 5.44543, 300, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (209028, 14);

-- Importing creature with guid 74033 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (74033, 10820);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (74033, 0, 1, 0, 1, 3285.29, -3379.7,  142.093, 3.24509, 72000, 0, 0, 0, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(740330, 1, 3285.29, -3379.7, 142.093, 0, 0, 0, NULL, 100, 0),
(740330, 2, 3313.75, -3380.01, 145.006, 0, 0, 0, NULL, 100, 0),
(740330, 3, 3346.58, -3379.95, 144.845, 0, 0, 0, NULL, 100, 0),
(740330, 4, 3359.66, -3382.52, 144.781, 0, 0, 0, NULL, 100, 0),
(740330, 5, 3362.25, -3418.58, 142.269, 0, 0, 0, NULL, 100, 0),
(740330, 6, 3387.64, -3420.77, 142.272, 0, 0, 0, NULL, 100, 0),
(740330, 7, 3393.32, -3403.63, 142.249, 0, 0, 0, NULL, 100, 0),
(740330, 8, 3392.78, -3354.85, 142.259, 0, 0, 0, NULL, 100, 0),
(740330, 9, 3388.8, -3338.3, 142.272, 0, 0, 0, NULL, 100, 0),
(740330, 10, 3371.81, -3338.53, 142.272, 0, 0, 0, NULL, 100, 0),
(740330, 11, 3361.79, -3350.17, 142.25, 0, 0, 0, NULL, 100, 0),
(740330, 12, 3361.2, -3365.57, 144.639, 0, 0, 0, NULL, 100, 0),
(740330, 13, 3354.42, -3379.49, 144.807, 0, 0, 0, NULL, 100, 0),
(740330, 14, 3339.82, -3379.75, 144.878, 0, 0, 0, NULL, 100, 0),
(740330, 15, 3322.65, -3379.82, 144.962, 0, 0, 0, NULL, 100, 0),
(740330, 16, 3306.77, -3379.84, 144.837, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (74033, 740330, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 77866 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (77866, 5348);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (77866, 0, 1, 0, 0, -10712.4, -2533.74,  27.7624, 6.15004, 72000, 0, 0, 0, 0, 0, 0, 0, 10);

-- Importing creature with guid 42575 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1095339 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1095339 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1095339 AND spawnType = 0 WHERE ce.spawnID = 1095339;
INSERT INTO creature_entry (spawnID, entry) VALUES (42575, 12339);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (42575, 0, 1, 0, 0, 1567.16, -5611,  114.19, 1.084, 900, 0, 1, 26412, 35250, 2, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 42575);

-- Importing creature with guid 85635 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1096637 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1096637 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1096637 AND spawnType = 0 WHERE ce.spawnID = 1096637;
INSERT INTO creature_entry (spawnID, entry) VALUES (85635, 15197);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (85635, 0, 1, 0, 1, 1734.6, 508.803,  41.2847, 1.6173, 300, 0, 0, 15260, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (85635, 12);

-- Importing creature with guid 16397 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (16397, 16123);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (16397, 0, 1, 0, 0, -14457.2, 448.085,  20.407, 6.11562, 600, 0, 0, 0, 0, 0, 0, 0, 10);

-- Importing creature with guid 127336 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (127336, 27478);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (127336, 0, 1, 22396, 0, -4849.41, -862.255,  501.997, 4.85202, 300, 0, 0, 2215, 0, 0, 0, 0, 10);

-- Importing creature with guid 14575 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (14575, 2738);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14575, 0, 1, 4145, 1, -1531.2, -1845.85,  67.8759, 1.53044, 400, 0, 0, 958, 0, 0, 0, 0, 10);

-- Importing creature with guid 84742 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1098124 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1098124 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1098124 AND spawnType = 0 WHERE ce.spawnID = 1098124;
INSERT INTO creature_entry (spawnID, entry) VALUES (84742, 16226);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (84742, 0, 1, 0, 0, 2305.29, -5286.12,  82.0207, 4.83456, 120, 0, 0, 0, 0, 0, 0, 0, 10);

-- Importing creature with guid 11004 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (11004, 2435);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (11004, 0, 1, 0, 1, -740.538, -525.298,  21.9343, 3.9523, 3600, 0, 0, 1163, 0, 0, 0, 0, 10);

-- Importing creature with guid 131335 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (131335, 11282);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (131335, 0, 1, 0, 0, 1113.59, -2555.59,  59.2532, 3.50681, 0, 0, 0, 42, 0, 0, 0, 0, 10);

-- Importing creature with guid 131336 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (131336, 11283);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (131336, 0, 1, 0, 0, 1111.49, -2556.4,  59.2532, 0.365222, 0, 0, 0, 42, 0, 0, 0, 0, 10);

-- Importing creature with guid 131337 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (131337, 11316);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (131337, 0, 1, 0, 0, 1223.27, -2506.41,  72.9261, 4.10152, 0, 0, 0, 3500, 0, 0, 0, 0, 10);

-- Importing creature with guid 131345 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (131345, 11287);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (131345, 0, 1, 0, 0, 1081.91, -2574,  59.957, 0.301, 360, 0, 0, 1536, 0, 0, 0, 0, 10);

-- Importing creature with guid 79379 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (79379, 1419);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (79379, 0, 1, 0, 0, -8994.19, 947.069,  118.348, 4.29351, 180, 0, 0, 8, 0, 0, 0, 0, 10);

-- Importing creature with guid 43465 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (43465, 2604);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (43465, 0, 1, 0, 1, -2056.97, -2785.61,  68.571, 5.47225, 54000, 0, 0, 1678, 0, 0, 0, 0, 10);

-- Importing creature with guid 84200 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (84200, 2601);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (84200, 0, 1, 0, 1, -1777.47, -1568.15,  53.0089, 3.82291, 172800, 0, 0, 6605, 1381, 0, 0, 0, 10);

-- Importing creature with guid 84227 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (84227, 2602);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (84227, 0, 1, 0, 1, -1797.35, -1507.83,  99.3938, 6.12074, 180000, 0, 0, 3540, 3708, 0, 0, 0, 10);

-- Importing creature with guid 86846 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (86846, 1001);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86846, 0, 1, 0, 0, -10912, -388,  40.632, 0.812, 300, 0, 0, 0, 0, 0, 0, 0, 10);

-- Importing creature with guid 86847 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (86847, 1098);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86847, 0, 1, 0, 0, -10533, -1128,  26.287, 4.084, 300, 0, 0, 0, 0, 0, 0, 0, 10);

-- Importing creature with guid 86851 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (86851, 1099);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86851, 0, 1, 0, 0, -10531, -1129,  26.093, 3.882, 300, 0, 0, 0, 0, 0, 0, 0, 10);

-- Importing creature with guid 86852 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (86852, 1100);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86852, 0, 1, 0, 0, -10526, -1128,  26.277, 3.621, 300, 0, 0, 0, 0, 0, 0, 0, 10);

-- Importing creature with guid 86853 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (86853, 1101);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86853, 0, 1, 0, 0, -10533, -1124,  26.347, 4.318, 300, 0, 0, 0, 0, 0, 0, 0, 10);

-- Importing creature with guid 86854 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (86854, 1203);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86854, 0, 1, 0, 0, -10552.4, -1316.49,  43.5212, 0.063056, 300, 0, 0, 0, 0, 0, 0, 0, 10);

-- Importing creature with guid 86855 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (86855, 1204);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86855, 0, 1, 0, 0, -10584, -1173,  28.568, 6.069, 300, 0, 0, 0, 0, 0, 0, 0, 10);

-- Importing creature with guid 86857 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (86857, 1436);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86857, 0, 1, 0, 0, -10929, -386,  40.186, 0.816, 300, 0, 0, 0, 0, 0, 0, 0, 10);

-- Importing creature with guid 86858 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (86858, 1129);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86858, 0, 1, 0, 0, -4300.36, -1420.53,  391.496, 5.16617, 300, 0, 0, 0, 0, 0, 0, 0, 10);

-- Importing creature with guid 86868 with import type IMPORT
INSERT INTO creature_entry (spawnID, entry) VALUES (86868, 1128);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86868, 0, 1, 0, 0, -4298.23, -1420.11,  391.32, 4.43579, 300, 0, 0, 0, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (86868, NULL, 0, 0, 1, 0, 0, NULL);

-- Importing creature with guid 86870 with import type IMPORT
INSERT INTO creature_entry (spawnID, entry) VALUES (86870, 1128);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86870, 0, 1, 0, 0, -4301.81, -1422.12,  391.556, 5.79545, 300, 0, 0, 0, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (86870, NULL, 0, 0, 1, 0, 0, NULL);

-- Importing creature with guid 87022 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (87022, 2683);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (87022, 0, 1, 0, 1, -4923.42, 725.513,  253.102, 0, 300, 0, 0, 1, 0, 0, 0, 0, 10);

-- Importing creature with guid 87023 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (87023, 3509);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (87023, 0, 1, 0, 0, -8838.52, 670.28,  98.0987, 0.553153, 300, 0, 0, 1, 0, 0, 0, 0, 10);

-- Importing creature with guid 90430 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (90430, 5569);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (90430, 0, 1, 0, 0, -4850.7, -1295.24,  501.951, 1.39626, 180, 0, 0, 1, 0, 0, 0, 0, 10);

-- Importing creature with guid 90451 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (90451, 5981);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (90451, 0, 1, 0, 0, -11351.9, -2754.5,  7.95043, 2.103, 430, 0, 0, 1, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(904510, 1, -11351.9, -2754.5, 7.95043, 0, 0, 0, NULL, 100, 0),
(904510, 2, -11358.7, -2797.01, 5.45784, 0, 0, 0, NULL, 100, 0),
(904510, 3, -11370.9, -2855.73, 4.67282, 0, 0, 0, NULL, 100, 0),
(904510, 4, -11396.7, -2926.03, 3.05246, 0, 0, 0, NULL, 100, 0),
(904510, 5, -11330.8, -2986.01, 3.24732, 0, 0, 0, NULL, 100, 0),
(904510, 6, -11380.8, -3013.05, -4.29948, 0, 0, 0, NULL, 100, 0),
(904510, 7, -11444, -3037.98, 1.11936, 0, 0, 0, NULL, 100, 0),
(904510, 8, -11500.1, -3041.02, -0.783425, 0, 0, 0, NULL, 100, 0),
(904510, 9, -11444.1, -3037.88, 1.11443, 0, 0, 0, NULL, 100, 0),
(904510, 10, -11380.8, -3012.74, -4.27558, 0, 0, 0, NULL, 100, 0),
(904510, 11, -11331, -2986.1, 3.21401, 0, 0, 0, NULL, 100, 0),
(904510, 12, -11396.9, -2926.05, 3.04129, 0, 0, 0, NULL, 100, 0),
(904510, 13, -11370.7, -2855.8, 4.71995, 0, 0, 0, NULL, 100, 0),
(904510, 14, -11358.6, -2796.98, 5.47431, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (90451, 904510, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 91799 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM world.creature_entry ce LEFT JOIN world.conditions c1 ON c1.ConditionValue3 = 1098406 AND c1.ConditionTypeOrReference = 31 LEFT JOIN world.conditions c2 ON c1.SourceEntry = -1098406 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN world.spawn_group sg ON sg.spawnID = 1098406 AND spawnType = 0 WHERE ce.spawnID = 1098406;
INSERT INTO creature_entry (spawnID, entry) VALUES (91799, 15353);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (91799, 0, 1, 0, 0, -4918.64, -983.141,  501.538, 2.43867, 120, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (91799, 12);

-- Formations...
INSERT INTO creature_formations VALUES 
(6154, 6155, 3, 2.6179938779915, 2, 1, 0),
(NULL, 6154, 0, 0, 2, 1, 0),
(NULL, 14772, 0, 0, 2, 1, 0),
(14772, 14773, 3, 3.1415926535898, 2, 1, 0),
(14772, 14774, 3, 1.5707963267949, 2, 1, 0),
(14772, 14775, 3, 6.2831853071796, 2, 1, 0),
(NULL, 14572, 0, 0, 2, 1, 0),
(14572, 14574, 3, 0.78539816339745, 2, 1, 0),
(14514, 14517, 5, 0.78539816339745, 2, 1, 0),
(14514, 14516, 5, 0, 2, 1, 0),
(14514, 14515, 5, 5.4977871437821, 2, 1, 0),
(NULL, 14514, 0, 0, 2, 1, 0),
(14772, 14776, 3, 4.7123889803847, 2, 1, 0),
(NULL, 15424, 0, 0, 2, 1, 0),
(15424, 15423, 3, 0, 2, 1, 0),
(NULL, 45501, 0, 0, 2, 1, 0),
(45501, 23427, 2, 0, 2, 1, 0),
(80849, 80848, 3, 1.0471975511966, 2, 1, 0),
(80849, 80850, 3, 4.1887902047864, 2, 1, 0),
(NULL, 80849, 0, 0, 2, 1, 0),
(NULL, 38429, 0, 0, 2, 1, 0),
(14572, 14575, 3, 5.4977871437821, 2, 1, 0),
(84028, 87023, 3, 2.0943951023932, 2, 1, 0);
