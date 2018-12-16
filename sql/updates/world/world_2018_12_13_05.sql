-- Importing creature with guid 1007 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1095729 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1095729 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1095729 AND spawnType = 0 WHERE ce.spawnID = 1095729;
INSERT INTO creature_entry (spawnID, entry) VALUES (1007, 18694);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (1007, 530, 1, 0, 0, -3035.06, 424.317,  2.77, 0.89, 86400, 0, 0, 13084, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(10070, 1, -3013.61, 464.581, -1.66238, 0, 0, 0, NULL, 100, 0),
(10070, 2, -3014.17, 498.897, -7.54742, 0, 0, 0, NULL, 100, 0),
(10070, 3, -3003.42, 539.168, 4.48822, 0, 0, 0, NULL, 100, 0),
(10070, 4, -2950.68, 549.48, -5.14097, 0, 0, 0, NULL, 100, 0),
(10070, 5, -2902.85, 551.89, -2.70704, 0, 0, 0, NULL, 100, 0),
(10070, 6, -2856.27, 572.067, -4.73839, 0, 0, 0, NULL, 100, 0),
(10070, 7, -2802.92, 611.373, -6.24457, 0, 0, 0, NULL, 100, 0),
(10070, 8, -2788.24, 654.777, -10.935, 0, 0, 0, NULL, 100, 0),
(10070, 9, -2777.88, 699.178, -6.10801, 0, 0, 0, NULL, 100, 0),
(10070, 10, -2755.72, 741.17, -1.55358, 0, 0, 0, NULL, 100, 0),
(10070, 11, -2732.75, 770.537, -0.223728, 0, 0, 0, NULL, 100, 0),
(10070, 12, -2728.42, 809.036, 7.36859, 0, 0, 0, NULL, 100, 0),
(10070, 13, -2735.92, 862.171, 4.16407, 0, 0, 0, NULL, 100, 0),
(10070, 14, -2751.32, 917.112, 2.24615, 0, 0, 0, NULL, 100, 0),
(10070, 15, -2756.96, 958.185, -2.23026, 0, 0, 0, NULL, 100, 0),
(10070, 16, -2790.47, 1014.25, 1.36017, 0, 0, 0, NULL, 100, 0),
(10070, 17, -2819.15, 1042.74, 5.87698, 0, 0, 0, NULL, 100, 0),
(10070, 18, -2790.04, 1008.86, 0.766533, 0, 0, 0, NULL, 100, 0),
(10070, 19, -2762.36, 963.438, -1.60225, 0, 0, 0, NULL, 100, 0),
(10070, 20, -2750.99, 914.038, 2.43204, 0, 0, 0, NULL, 100, 0),
(10070, 21, -2733.95, 864.899, 5.03877, 0, 0, 0, NULL, 100, 0),
(10070, 22, -2729.98, 818.492, -2.149, 0, 0, 0, NULL, 100, 0),
(10070, 23, -2743.59, 745.799, -4.93666, 0, 0, 0, NULL, 100, 0),
(10070, 24, -2756.81, 669.144, -13.8135, 0, 0, 0, NULL, 100, 0),
(10070, 25, -2772.33, 640.811, -20.9951, 0, 0, 0, NULL, 100, 0),
(10070, 26, -2822.45, 617.9, -8.97523, 0, 0, 0, NULL, 100, 0),
(10070, 27, -2877.12, 580.651, -9.083, 0, 0, 0, NULL, 100, 0),
(10070, 28, -2960.09, 552.926, -7.03473, 0, 0, 0, NULL, 100, 0),
(10070, 29, -3005.1, 496.315, -13.8056, 0, 0, 0, NULL, 100, 0),
(10070, 30, -3012.09, 442.061, -3.58871, 0, 0, 0, NULL, 100, 0),
(10070, 31, -3004.98, 363.255, 2.23716, 0, 0, 0, NULL, 100, 0),
(10070, 32, -2996.85, 290.217, -0.5488, 0, 0, 0, NULL, 100, 0),
(10070, 33, -2996.14, 206.339, 2.47533, 0, 0, 0, NULL, 100, 0),
(10070, 34, -2994.31, 115.131, 2.65022, 0, 0, 0, NULL, 100, 0),
(10070, 35, -2997.15, 190.237, 3.96603, 0, 0, 0, NULL, 100, 0),
(10070, 36, -2996.59, 287.268, -1.48792, 0, 0, 0, NULL, 100, 0),
(10070, 37, -2977.16, 330.412, -7.62497, 0, 0, 0, NULL, 100, 0),
(10070, 38, -3005.61, 362.537, 2.1557, 0, 0, 0, NULL, 100, 0),
(10070, 39, -3035.38, 425.13, -0.692898, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (1007, 10070, 0, 0, 0, 0, 0, NULL);

-- Importing creature with guid 32914 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1032774 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1032774 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1032774 AND spawnType = 0 WHERE ce.spawnID = 1032774;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1051562 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1051562 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1051562 AND spawnType = 0 WHERE ce.spawnID = 1051562;
INSERT INTO creature_entry (spawnID, entry) VALUES (32914, 10647);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32914, 1, 1, 11331, 1, 2808.24, -3003.13,  157.067, 4.11898, 19990, 0, 1, 1332, 1169, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(329140, 1, 2808.24, -3003.13, 157.067, 0, 0, 0, NULL, 100, 0),
(329140, 2, 2811.39, -2991.73, 155.334, 0, 0, 0, NULL, 100, 0),
(329140, 3, 2808.64, -2972.1, 155.101, 0, 0, 0, NULL, 100, 0),
(329140, 4, 2804.84, -2959.18, 152.49, 0, 0, 0, NULL, 100, 0),
(329140, 5, 2805.55, -2938.3, 154.463, 0, 0, 0, NULL, 100, 0),
(329140, 6, 2814.46, -2921.44, 163.474, 0, 0, 0, NULL, 100, 0),
(329140, 7, 2818.51, -2918.65, 166.685, 0, 0, 0, NULL, 100, 0),
(329140, 8, 2840.34, -2914.93, 183.502, 0, 0, 0, NULL, 100, 0),
(329140, 9, 2854.31, -2914.02, 192.162, 0, 0, 0, NULL, 100, 0),
(329140, 10, 2871.51, -2911.2, 198.77, 0, 0, 0, NULL, 100, 0),
(329140, 11, 2882.42, -2903.18, 201.185, 0, 0, 0, NULL, 100, 0),
(329140, 12, 2888.19, -2886.37, 203.285, 0, 0, 0, NULL, 100, 0),
(329140, 13, 2892.45, -2865.81, 203.822, 0, 0, 0, NULL, 100, 0),
(329140, 14, 2898.97, -2852.82, 207.158, 0, 0, 0, NULL, 100, 0),
(329140, 15, 2908.75, -2844.04, 209.319, 0, 0, 0, NULL, 100, 0),
(329140, 16, 2926.23, -2833.18, 211.339, 0, 0, 0, NULL, 100, 0),
(329140, 17, 2943.59, -2830.33, 212.666, 0, 0, 0, NULL, 100, 0),
(329140, 18, 2953.08, -2824.16, 212.614, 0, 0, 0, NULL, 100, 0),
(329140, 19, 2964.85, -2820.93, 211.645, 0, 0, 0, NULL, 100, 0),
(329140, 20, 2978.74, -2819.18, 209.464, 0, 0, 0, NULL, 100, 0),
(329140, 21, 3001.01, -2818.46, 206.083, 0, 0, 0, NULL, 100, 0),
(329140, 22, 3022.53, -2814.14, 203.552, 0, 0, 0, NULL, 100, 0),
(329140, 23, 3035.24, -2811.01, 198.438, 0, 0, 0, NULL, 100, 0),
(329140, 24, 3040, -2799.58, 195.965, 0, 0, 0, NULL, 100, 0),
(329140, 25, 3035.48, -2809.66, 198.041, 0, 0, 0, NULL, 100, 0),
(329140, 26, 3024.45, -2819.71, 202.394, 0, 0, 0, NULL, 100, 0),
(329140, 27, 3006.51, -2818.99, 205.281, 0, 0, 0, NULL, 100, 0),
(329140, 28, 2984.26, -2818.4, 209.399, 0, 0, 0, NULL, 100, 0),
(329140, 29, 2970.59, -2821.38, 210.346, 0, 0, 0, NULL, 100, 0),
(329140, 30, 2952.68, -2822.8, 212.661, 0, 0, 0, NULL, 100, 0),
(329140, 31, 2942.47, -2830.03, 212.666, 0, 0, 0, NULL, 100, 0),
(329140, 32, 2930.23, -2831.12, 211.663, 0, 0, 0, NULL, 100, 0),
(329140, 33, 2915.55, -2836, 209.536, 0, 0, 0, NULL, 100, 0),
(329140, 34, 2904.55, -2845.87, 208.518, 0, 0, 0, NULL, 100, 0),
(329140, 35, 2889.94, -2865.74, 203.493, 0, 0, 0, NULL, 100, 0),
(329140, 36, 2887.78, -2881.86, 203.375, 0, 0, 0, NULL, 100, 0),
(329140, 37, 2884.01, -2899.52, 202.009, 0, 0, 0, NULL, 100, 0),
(329140, 38, 2875.23, -2911.85, 199.338, 0, 0, 0, NULL, 100, 0),
(329140, 39, 2862.77, -2914.38, 196.4, 0, 0, 0, NULL, 100, 0),
(329140, 40, 2850.1, -2914.26, 189.543, 0, 0, 0, NULL, 100, 0),
(329140, 41, 2843.34, -2912.35, 185.238, 0, 0, 0, NULL, 100, 0),
(329140, 42, 2829.4, -2911.12, 175.736, 0, 0, 0, NULL, 100, 0),
(329140, 43, 2818.51, -2910.98, 166.842, 0, 0, 0, NULL, 100, 0),
(329140, 44, 2808.09, -2915.23, 159.118, 0, 0, 0, NULL, 100, 0),
(329140, 45, 2804.95, -2931.01, 155.844, 0, 0, 0, NULL, 100, 0),
(329140, 46, 2806.35, -2945.02, 154.086, 0, 0, 0, NULL, 100, 0),
(329140, 47, 2807.29, -2956.84, 153.169, 0, 0, 0, NULL, 100, 0),
(329140, 48, 2807.59, -2963.08, 153.936, 0, 0, 0, NULL, 100, 0),
(329140, 49, 2808.62, -2969.8, 155.047, 0, 0, 0, NULL, 100, 0),
(329140, 50, 2813.63, -2989.59, 155.539, 0, 0, 0, NULL, 100, 0),
(329140, 51, 2810.64, -2998.34, 156.787, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (32914, 329140, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 32869 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1032729 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1032729 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1032729 AND spawnType = 0 WHERE ce.spawnID = 1032729;
INSERT INTO creature_entry (spawnID, entry) VALUES (32869, 10639);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32869, 1, 1, 6800, 1, 3265.91, -365.72,  153.103, 5.31552, 19990, 0, 1, 713, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(328690, 1, 3265.91, -365.72, 153.103, 0, 0, 0, NULL, 100, 0),
(328690, 2, 3273.11, -371.724, 152.879, 0, 0, 0, NULL, 100, 0),
(328690, 3, 3278.83, -382.906, 152.523, 0, 0, 0, NULL, 100, 0),
(328690, 4, 3290.6, -408.267, 150.345, 0, 0, 0, NULL, 100, 0),
(328690, 5, 3307.69, -430.402, 149.836, 0, 0, 0, NULL, 100, 0),
(328690, 6, 3320.12, -447.278, 150.563, 0, 0, 0, NULL, 100, 0),
(328690, 7, 3336.11, -470.103, 153.995, 0, 0, 0, NULL, 100, 0),
(328690, 8, 3347.99, -480.422, 154.224, 0, 0, 0, NULL, 100, 0),
(328690, 9, 3358.45, -476.939, 154.218, 0, 0, 0, NULL, 100, 0),
(328690, 10, 3365.44, -464.821, 154.221, 0, 0, 0, NULL, 100, 0),
(328690, 11, 3370.22, -452.802, 153.903, 0, 0, 0, NULL, 100, 0),
(328690, 12, 3384.15, -428.401, 149.322, 0, 0, 0, NULL, 100, 0),
(328690, 13, 3392.83, -417.417, 146.747, 0, 0, 0, NULL, 100, 0),
(328690, 14, 3405.05, -403.26, 143.911, 0, 0, 0, NULL, 100, 0),
(328690, 15, 3421.9, -390.825, 139.321, 0, 0, 0, NULL, 100, 0),
(328690, 16, 3434.33, -384.392, 135.794, 0, 0, 0, NULL, 100, 0),
(328690, 17, 3447.36, -378.007, 135.662, 0, 0, 0, NULL, 100, 0),
(328690, 18, 3434.16, -382.637, 135.496, 0, 0, 0, NULL, 100, 0),
(328690, 19, 3421.42, -388.368, 138.45, 0, 0, 0, NULL, 100, 0),
(328690, 20, 3404.06, -400.145, 143.263, 0, 0, 0, NULL, 100, 0),
(328690, 21, 3390.34, -412.986, 145.813, 0, 0, 0, NULL, 100, 0),
(328690, 22, 3377.26, -421.769, 148.56, 0, 0, 0, NULL, 100, 0),
(328690, 23, 3362.35, -430.288, 151.45, 0, 0, 0, NULL, 100, 0),
(328690, 24, 3349.06, -433.942, 151.799, 0, 0, 0, NULL, 100, 0),
(328690, 25, 3333.84, -433.347, 150.422, 0, 0, 0, NULL, 100, 0),
(328690, 26, 3318.93, -428.626, 149.402, 0, 0, 0, NULL, 100, 0),
(328690, 27, 3307.99, -420.836, 149.473, 0, 0, 0, NULL, 100, 0),
(328690, 28, 3293.19, -405.975, 150.442, 0, 0, 0, NULL, 100, 0),
(328690, 29, 3284.6, -394.928, 151.113, 0, 0, 0, NULL, 100, 0),
(328690, 30, 3275.52, -378.113, 152.894, 0, 0, 0, NULL, 100, 0),
(328690, 31, 3269.43, -368.935, 153.063, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (32869, 328690, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 32773 with import type REPLACE_ALL
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32761 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32761 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32761 AND spawnType = 0 WHERE ce.spawnID = 32761;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32763 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32763 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32763 AND spawnType = 0 WHERE ce.spawnID = 32763;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32764 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32764 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32764 AND spawnType = 0 WHERE ce.spawnID = 32764;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32765 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32765 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32765 AND spawnType = 0 WHERE ce.spawnID = 32765;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32766 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32766 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32766 AND spawnType = 0 WHERE ce.spawnID = 32766;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32767 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32767 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32767 AND spawnType = 0 WHERE ce.spawnID = 32767;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32768 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32768 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32768 AND spawnType = 0 WHERE ce.spawnID = 32768;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32769 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32769 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32769 AND spawnType = 0 WHERE ce.spawnID = 32769;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32770 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32770 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32770 AND spawnType = 0 WHERE ce.spawnID = 32770;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32771 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32771 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32771 AND spawnType = 0 WHERE ce.spawnID = 32771;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32772 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32772 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32772 AND spawnType = 0 WHERE ce.spawnID = 32772;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32773 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32773 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32773 AND spawnType = 0 WHERE ce.spawnID = 32773;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1032622 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1032622 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1032622 AND spawnType = 0 WHERE ce.spawnID = 1032622;
INSERT INTO creature_entry (spawnID, entry) VALUES (32773, 3715);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32773, 1, 1, 4981, 1, 3652.6, 1017.44,  -0.965293, 0.02262, 300, 12, 0, 404, 456, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32773);
INSERT INTO creature_entry (spawnID, entry) VALUES (32772, 3715);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32772, 1, 1, 4981, 1, 3653.49, 1048.73,  -3.29143, 0.012661, 300, 12, 0, 404, 456, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32772);
INSERT INTO creature_entry (spawnID, entry) VALUES (32771, 3715);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32771, 1, 1, 4981, 1, 3630.08, 1077.88,  -1.98365, 3.02484, 300, 12, 0, 404, 456, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32771);
INSERT INTO creature_entry (spawnID, entry) VALUES (32770, 3715);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32770, 1, 1, 4981, 1, 3573.89, 1073.63,  -6.83789, 1.16909, 300, 12, 0, 404, 456, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32770);
INSERT INTO creature_entry (spawnID, entry) VALUES (32769, 3715);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32769, 1, 1, 4981, 1, 3448.34, 1017.37,  2.59086, 0.772484, 300, 12, 0, 404, 456, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32769);
INSERT INTO creature_entry (spawnID, entry) VALUES (32768, 3715);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32768, 1, 1, 4981, 1, 3520.83, 1004.17,  2.12923, 5.2248, 300, 12, 0, 404, 456, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32768);
INSERT INTO creature_entry (spawnID, entry) VALUES (32767, 3715);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32767, 1, 1, 4981, 1, 3723.27, 1154.48,  -49.3801, 5.73691, 300, 12, 0, 404, 456, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32767);
INSERT INTO creature_entry (spawnID, entry) VALUES (32766, 3715);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32766, 1, 1, 4981, 1, 3743.5, 1082.2,  -17.3022, 3.76691, 300, 12, 0, 404, 456, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32766);
INSERT INTO creature_entry (spawnID, entry) VALUES (32765, 3715);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32765, 1, 1, 4981, 1, 3685.9, 1185.94,  -45.0512, 5.77649, 300, 12, 0, 404, 456, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32765);
INSERT INTO creature_entry (spawnID, entry) VALUES (32764, 3715);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32764, 1, 1, 4981, 1, 3586.14, 1215.76,  -31.558, 0.128679, 300, 12, 0, 404, 456, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32764);
INSERT INTO creature_entry (spawnID, entry) VALUES (32763, 3715);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32763, 1, 1, 4981, 1, 3517.63, 1153.11,  -10.6651, 2.83547, 300, 12, 0, 404, 456, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32763);
INSERT INTO creature_entry (spawnID, entry) VALUES (32762, 3715);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32762, 1, 1, 4981, 1, 4158.46, 1274.85,  3.35226, 3.46136, 300, 0, 0, 404, 456, 0, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32762);
INSERT INTO creature_entry (spawnID, entry) VALUES (32761, 3715);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32761, 1, 1, 4981, 1, 3685.59, 1151.96,  -20.5701, 4.74171, 300, 12, 0, 404, 456, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32761);

-- Importing creature with guid 32772 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32771 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32770 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32769 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32768 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32767 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32766 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32765 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32764 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32763 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32762 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32761 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32735 with import type REPLACE_ALL
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32722 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32722 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32722 AND spawnType = 0 WHERE ce.spawnID = 32722;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32723 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32723 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32723 AND spawnType = 0 WHERE ce.spawnID = 32723;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32724 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32724 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32724 AND spawnType = 0 WHERE ce.spawnID = 32724;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32725 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32725 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32725 AND spawnType = 0 WHERE ce.spawnID = 32725;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32726 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32726 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32726 AND spawnType = 0 WHERE ce.spawnID = 32726;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32727 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32727 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32727 AND spawnType = 0 WHERE ce.spawnID = 32727;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32729 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32729 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32729 AND spawnType = 0 WHERE ce.spawnID = 32729;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32730 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32730 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32730 AND spawnType = 0 WHERE ce.spawnID = 32730;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32731 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32731 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32731 AND spawnType = 0 WHERE ce.spawnID = 32731;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32732 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32732 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32732 AND spawnType = 0 WHERE ce.spawnID = 32732;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32733 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32733 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32733 AND spawnType = 0 WHERE ce.spawnID = 32733;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32734 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32734 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32734 AND spawnType = 0 WHERE ce.spawnID = 32734;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 32735 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -32735 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 32735 AND spawnType = 0 WHERE ce.spawnID = 32735;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1032588 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1032588 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1032588 AND spawnType = 0 WHERE ce.spawnID = 1032588;
INSERT INTO creature_entry (spawnID, entry) VALUES (32735, 3711);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32735, 1, 1, 6747, 1, 3480.51, 1051.29,  0.15928, 2.85975, 300, 12, 0, 494, 0, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32735);
INSERT INTO creature_entry (spawnID, entry) VALUES (32734, 3711);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32734, 1, 1, 6747, 1, 4055.59, 1248.42,  -27.6482, 4.8829, 300, 12, 0, 494, 0, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32734);
INSERT INTO creature_entry (spawnID, entry) VALUES (32733, 3711);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32733, 1, 1, 6747, 1, 4123.95, 1284.23,  2.36956, 0.675914, 300, 12, 0, 494, 0, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32733);
INSERT INTO creature_entry (spawnID, entry) VALUES (32732, 3711);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32732, 1, 1, 6747, 1, 4089.29, 1219.06,  -22.1998, 6.01098, 300, 12, 0, 494, 0, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32732);
INSERT INTO creature_entry (spawnID, entry) VALUES (32731, 3711);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32731, 1, 1, 6747, 1, 4078.5, 1292.64,  -0.700187, 2.50727, 300, 12, 0, 494, 0, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32731);
INSERT INTO creature_entry (spawnID, entry) VALUES (32730, 3711);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32730, 1, 1, 6747, 1, 4089.36, 1240.29,  -12.3752, 5.13693, 300, 12, 0, 494, 0, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32730);
INSERT INTO creature_entry (spawnID, entry) VALUES (32729, 3711);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32729, 1, 1, 6747, 1, 4184.61, 1349,  8.15056, 0.453909, 300, 12, 0, 494, 0, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32729);
INSERT INTO creature_entry (spawnID, entry) VALUES (32728, 3711);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32728, 1, 1, 6747, 1, 4159.55, 1271.75,  3.00348, 3.76264, 300, 0, 0, 494, 0, 0, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32728);
INSERT INTO creature_entry (spawnID, entry) VALUES (32727, 3711);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32727, 1, 1, 6747, 1, 4144.84, 1286.15,  3.53631, 2.49402, 300, 12, 0, 494, 0, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32727);
INSERT INTO creature_entry (spawnID, entry) VALUES (32726, 3711);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32726, 1, 1, 6747, 1, 4115.92, 1414.73,  -7.53851, 2.0574, 300, 12, 0, 494, 0, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32726);
INSERT INTO creature_entry (spawnID, entry) VALUES (32725, 3711);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32725, 1, 1, 6747, 1, 4217.98, 1327.39,  3.88642, 1.92844, 300, 12, 0, 494, 0, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32725);
INSERT INTO creature_entry (spawnID, entry) VALUES (32724, 3711);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32724, 1, 1, 6747, 1, 4188.38, 1247.62,  -0.476587, 5.50469, 300, 12, 0, 494, 0, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32724);
INSERT INTO creature_entry (spawnID, entry) VALUES (32723, 3711);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32723, 1, 1, 6747, 1, 4249.93, 1352.61,  -3.77582, 0.745698, 300, 12, 0, 494, 0, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32723);
INSERT INTO creature_entry (spawnID, entry) VALUES (32722, 3711);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32722, 1, 1, 6747, 1, 4051.3, 1318.53,  -10.8468, 4.8209, 300, 12, 0, 494, 0, 1, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 32722);

-- Importing creature with guid 32734 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32733 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32732 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32731 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32730 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32729 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32728 with import type MOVE_UNIQUE_IMPORT_WP

-- Importing creature with guid 32727 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32726 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32725 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32724 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32723 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32722 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 32617 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1032477 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1032477 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1032477 AND spawnType = 0 WHERE ce.spawnID = 1032477;
INSERT INTO creature_entry (spawnID, entry) VALUES (32617, 3943);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32617, 1, 1, 5029, 0, 4157.59, 1272.73,  3.22775, 3.14648, 300, 0, 0, 731, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(326170, 1, 4157.59, 1272.73, 3.22775, 0, 0, 0, NULL, 100, 0),
(326170, 2, 4118.6, 1268.22, 1.93513, 0, 0, 0, NULL, 100, 0),
(326170, 3, 4100.35, 1277.8, 1.42555, 0, 0, 0, NULL, 100, 0),
(326170, 4, 4090.54, 1314.88, 2.11756, 0, 0, 0, NULL, 100, 0),
(326170, 5, 4098.61, 1352.27, 2.58844, 0, 0, 0, NULL, 100, 0),
(326170, 6, 4116.9, 1363.47, 1.69482, 0, 0, 0, NULL, 100, 0),
(326170, 7, 4133.67, 1375.35, 2.18856, 0, 0, 0, NULL, 100, 0),
(326170, 8, 4176.57, 1370.93, 2.58461, 0, 0, 0, NULL, 100, 0),
(326170, 9, 4221.99, 1339.24, 2.22232, 0, 0, 0, NULL, 100, 0),
(326170, 10, 4225.52, 1317.01, 1.90514, 0, 0, 0, NULL, 100, 0),
(326170, 11, 4206.5, 1285.38, 1.3025, 0, 0, 0, NULL, 100, 0),
(326170, 12, 4185.92, 1278.95, 1.27956, 0, 0, 0, NULL, 100, 0),
(326170, 13, 4168.32, 1281.27, 3.36765, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (32617, 326170, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 32617);

-- Importing creature with guid 32384 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1032244 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1032244 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1032244 AND spawnType = 0 WHERE ce.spawnID = 1032244;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1051580 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1051580 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1051580 AND spawnType = 0 WHERE ce.spawnID = 1051580;
INSERT INTO creature_entry (spawnID, entry) VALUES (32384, 10559);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32384, 1, 1, 4979, 1, 3722.43, 913.772,  1.00271, 1.84775, 19990, 0, 1, 508, 544, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(323840, 1, 3722.43, 913.772, 1.00271, 0, 0, 0, NULL, 100, 0),
(323840, 2, 3732.03, 904.331, 0.968114, 0, 0, 0, NULL, 100, 0),
(323840, 3, 3739.74, 895.524, 0.758106, 0, 0, 0, NULL, 100, 0),
(323840, 4, 3750.79, 880.085, 0.36612, 0, 0, 0, NULL, 100, 0),
(323840, 5, 3758.79, 867.507, 0.679295, 0, 0, 0, NULL, 100, 0),
(323840, 6, 3767.16, 856.288, 0.976484, 0, 0, 0, NULL, 100, 0),
(323840, 7, 3782.52, 843.849, 0.968118, 0, 0, 0, NULL, 100, 0),
(323840, 8, 3795.71, 840.702, 1.67544, 0, 0, 0, NULL, 100, 0),
(323840, 9, 3808.05, 838.972, 0.956729, 0, 0, 0, NULL, 100, 0),
(323840, 10, 3829.03, 839.984, 0.967158, 0, 0, 0, NULL, 100, 0),
(323840, 11, 3843.01, 840.75, 1.18636, 0, 0, 0, NULL, 100, 0),
(323840, 12, 3868.52, 844.504, 1.14136, 0, 0, 0, NULL, 100, 0),
(323840, 13, 3876.15, 850.338, 1.75528, 0, 0, 0, NULL, 100, 0),
(323840, 14, 3881.14, 864.116, 1.31083, 0, 0, 0, NULL, 100, 0),
(323840, 15, 3888.65, 865.324, 1.37802, 0, 0, 0, NULL, 100, 0),
(323840, 16, 3906.88, 866.121, 1.13415, 0, 0, 0, NULL, 100, 0),
(323840, 17, 3953.09, 864.282, 0.148911, 0, 0, 0, NULL, 100, 0),
(323840, 18, 3983.3, 857.571, 1.04679, 0, 0, 0, NULL, 100, 0),
(323840, 19, 4000.2, 860.726, 1.17044, 0, 0, 0, NULL, 100, 0),
(323840, 20, 4014.44, 868.47, 0.720758, 0, 0, 0, NULL, 100, 0),
(323840, 21, 4027.74, 883.204, 0.801679, 0, 0, 0, NULL, 100, 0),
(323840, 22, 4040.63, 896.32, 0.261184, 0, 0, 0, NULL, 100, 0),
(323840, 23, 4053.57, 909.004, 1.51234, 0, 0, 0, NULL, 100, 0),
(323840, 24, 4065.36, 911.44, 3.64765, 0, 0, 0, NULL, 100, 0),
(323840, 25, 4077.34, 909.21, 5.3324, 0, 0, 0, NULL, 100, 0),
(323840, 26, 4089.09, 903.19, 5.97173, 0, 0, 0, NULL, 100, 0),
(323840, 27, 4097.36, 901.197, 7.08078, 0, 0, 0, NULL, 100, 0),
(323840, 28, 4113.48, 897.702, 9.81231, 0, 0, 0, NULL, 100, 0),
(323840, 29, 4117.06, 886.646, 9.75709, 0, 0, 0, NULL, 100, 0),
(323840, 30, 4117.48, 870.737, 9.81522, 0, 0, 0, NULL, 100, 0),
(323840, 31, 4126.46, 866.359, 9.70748, 0, 0, 0, NULL, 100, 0),
(323840, 32, 4133.69, 864.305, 6.78576, 0, 0, 0, NULL, 100, 0),
(323840, 33, 4146.95, 861.234, 4.07842, 0, 0, 0, NULL, 100, 0),
(323840, 34, 4150.84, 864.035, 4.07842, 0, 0, 0, NULL, 100, 0),
(323840, 35, 4155.39, 871.404, 4.08262, 0, 0, 0, NULL, 100, 0),
(323840, 36, 4157.45, 877.768, 1.50408, 0, 0, 0, NULL, 100, 0),
(323840, 37, 4159.56, 886.388, -1.45882, 0, 0, 0, NULL, 100, 0),
(323840, 38, 4157.62, 893.779, -1.45399, 0, 0, 0, NULL, 100, 0),
(323840, 39, 4152.75, 898.932, -1.45343, 0, 0, 0, NULL, 100, 0),
(323840, 40, 4147.64, 900.964, -2.47628, 0, 0, 0, NULL, 100, 0),
(323840, 41, 4141.22, 902.808, -5.82136, 0, 0, 0, NULL, 100, 0),
(323840, 42, 4135.14, 904.173, -7.00924, 0, 0, 0, NULL, 100, 0),
(323840, 43, 4126.2, 900.406, -7.00887, 0, 0, 0, NULL, 100, 0),
(323840, 44, 4123.51, 895.363, -7.06427, 0, 0, 0, NULL, 100, 0),
(323840, 45, 4121.02, 888.352, -10.0093, 0, 0, 0, NULL, 100, 0),
(323840, 46, 4118.93, 881.739, -12.623, 0, 0, 0, NULL, 100, 0),
(323840, 47, 4118.7, 874.418, -12.5636, 0, 0, 0, NULL, 100, 0),
(323840, 48, 4124.14, 869.048, -12.5636, 0, 0, 0, NULL, 100, 0),
(323840, 49, 4131.27, 865.741, -14.0774, 0, 0, 0, NULL, 100, 0),
(323840, 50, 4127.22, 866.879, -12.5624, 0, 0, 0, NULL, 100, 0),
(323840, 51, 4122.48, 869.655, -12.5624, 0, 0, 0, NULL, 100, 0),
(323840, 52, 4117.99, 876.806, -12.5624, 0, 0, 0, NULL, 100, 0),
(323840, 53, 4119.47, 886.473, -11.127, 0, 0, 0, NULL, 100, 0),
(323840, 54, 4121.21, 893.253, -7.62682, 0, 0, 0, NULL, 100, 0),
(323840, 55, 4124.16, 900.133, -7.00757, 0, 0, 0, NULL, 100, 0),
(323840, 56, 4129.97, 902.295, -7.00757, 0, 0, 0, NULL, 100, 0),
(323840, 57, 4137.78, 903.703, -7.00901, 0, 0, 0, NULL, 100, 0),
(323840, 58, 4143.77, 902.755, -4.58501, 0, 0, 0, NULL, 100, 0),
(323840, 59, 4148.36, 901.232, -2.1679, 0, 0, 0, NULL, 100, 0),
(323840, 60, 4155.44, 896.502, -1.45183, 0, 0, 0, NULL, 100, 0),
(323840, 61, 4159.38, 886.155, -1.45803, 0, 0, 0, NULL, 100, 0),
(323840, 62, 4158.67, 882.022, -0.702407, 0, 0, 0, NULL, 100, 0),
(323840, 63, 4156.95, 875.239, 2.78633, 0, 0, 0, NULL, 100, 0),
(323840, 64, 4153.64, 865.145, 4.07995, 0, 0, 0, NULL, 100, 0),
(323840, 65, 4149.28, 863.342, 4.07995, 0, 0, 0, NULL, 100, 0),
(323840, 66, 4142.28, 863.443, 4.08353, 0, 0, 0, NULL, 100, 0),
(323840, 67, 4135.52, 863.82, 5.8326, 0, 0, 0, NULL, 100, 0),
(323840, 68, 4128.33, 866.176, 9.61656, 0, 0, 0, NULL, 100, 0),
(323840, 69, 4124.06, 869.521, 9.79827, 0, 0, 0, NULL, 100, 0),
(323840, 70, 4117.5, 877.209, 9.77358, 0, 0, 0, NULL, 100, 0),
(323840, 71, 4113.08, 883.249, 9.81171, 0, 0, 0, NULL, 100, 0),
(323840, 72, 4108.95, 885.474, 9.81171, 0, 0, 0, NULL, 100, 0),
(323840, 73, 4101.36, 894.021, 9.50601, 0, 0, 0, NULL, 100, 0),
(323840, 74, 4099.21, 901.252, 7.48032, 0, 0, 0, NULL, 100, 0),
(323840, 75, 4094.04, 900.019, 6.55541, 0, 0, 0, NULL, 100, 0),
(323840, 76, 4080.27, 902.525, 4.84452, 0, 0, 0, NULL, 100, 0),
(323840, 77, 4066.64, 905.211, 2.67609, 0, 0, 0, NULL, 100, 0),
(323840, 78, 4052.04, 900.389, -0.015366, 0, 0, 0, NULL, 100, 0),
(323840, 79, 4036.89, 889.341, 0.380059, 0, 0, 0, NULL, 100, 0),
(323840, 80, 4015.93, 874.015, 0.399262, 0, 0, 0, NULL, 100, 0),
(323840, 81, 3996.32, 866.859, 1.08266, 0, 0, 0, NULL, 100, 0),
(323840, 82, 3960.32, 860.776, 0.167445, 0, 0, 0, NULL, 100, 0),
(323840, 83, 3952.94, 863.004, 0.241122, 0, 0, 0, NULL, 100, 0),
(323840, 84, 3940.34, 865.997, 0.925124, 0, 0, 0, NULL, 100, 0),
(323840, 85, 3919.45, 868.037, 1.20335, 0, 0, 0, NULL, 100, 0),
(323840, 86, 3898.7, 867.626, 1.27737, 0, 0, 0, NULL, 100, 0),
(323840, 87, 3881.74, 862.046, 1.27606, 0, 0, 0, NULL, 100, 0),
(323840, 88, 3878, 857.874, 1.554, 0, 0, 0, NULL, 100, 0),
(323840, 89, 3874.55, 851.784, 1.57469, 0, 0, 0, NULL, 100, 0),
(323840, 90, 3867.74, 841.905, 1.19773, 0, 0, 0, NULL, 100, 0),
(323840, 91, 3851.13, 838.355, 0.993947, 0, 0, 0, NULL, 100, 0),
(323840, 92, 3809.33, 842.446, 0.796721, 0, 0, 0, NULL, 100, 0),
(323840, 93, 3781.46, 845.069, 0.967946, 0, 0, 0, NULL, 100, 0),
(323840, 94, 3770.63, 851.672, 0.967075, 0, 0, 0, NULL, 100, 0),
(323840, 95, 3759.56, 863.324, 0.99252, 0, 0, 0, NULL, 100, 0),
(323840, 96, 3739.67, 892.069, 0.73476, 0, 0, 0, NULL, 100, 0),
(323840, 97, 3729.48, 910.431, 0.96781, 0, 0, 0, NULL, 100, 0),
(323840, 98, 3716.26, 916.782, 1.25324, 0, 0, 0, NULL, 100, 0),
(323840, 99, 3705.52, 923.452, 1.56637, 0, 0, 0, NULL, 100, 0),
(323840, 100, 3693.32, 935.196, 3.14731, 0, 0, 0, NULL, 100, 0),
(323840, 101, 3682.39, 953.076, 1.51719, 0, 0, 0, NULL, 100, 0),
(323840, 102, 3675.43, 967.51, 2.46863, 0, 0, 0, NULL, 100, 0),
(323840, 103, 3677.18, 984.397, 0.705175, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (32384, 323840, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 32296 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1032156 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1032156 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1032156 AND spawnType = 0 WHERE ce.spawnID = 1032156;
INSERT INTO creature_entry (spawnID, entry) VALUES (32296, 4074);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (32296, 1, 1, 1269, 0, 1222.28, 199.114,  5.52599, 5.10178, 300, 0, 1, 617, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(322960, 1, 1222.28, 199.114, 5.52599, 0, 0, 0, NULL, 100, 0),
(322960, 2, 1209.34, 203.459, 5.94868, 0, 0, 0, NULL, 100, 0),
(322960, 3, 1192.07, 209.217, 5.2444, 0, 0, 0, NULL, 100, 0),
(322960, 4, 1179.19, 213.726, 5.67425, 0, 0, 0, NULL, 100, 0),
(322960, 5, 1164.16, 220.786, 4.38275, 0, 0, 0, NULL, 100, 0),
(322960, 6, 1147.99, 229.043, 3.53207, 0, 0, 0, NULL, 100, 0),
(322960, 7, 1132.15, 231.525, 7.35797, 0, 0, 0, NULL, 100, 0),
(322960, 8, 1118.51, 231.104, 9.25522, 0, 0, 0, NULL, 100, 0),
(322960, 9, 1100.39, 229.453, 11.3085, 0, 0, 0, NULL, 100, 0),
(322960, 10, 1087.94, 226.725, 10.9176, 0, 0, 0, NULL, 100, 0),
(322960, 11, 1080.09, 214.417, 10.397, 0, 0, 0, NULL, 100, 0),
(322960, 12, 1080.62, 198.052, 9.54274, 0, 0, 0, NULL, 100, 0),
(322960, 13, 1084.61, 185.012, 8.58309, 0, 0, 0, NULL, 100, 0),
(322960, 14, 1093.25, 171.69, 4.98058, 0, 0, 0, NULL, 100, 0),
(322960, 15, 1092.94, 164.038, 4.32033, 0, 0, 0, NULL, 100, 0),
(322960, 16, 1088.53, 156.011, 4.62988, 0, 0, 0, NULL, 100, 0),
(322960, 17, 1080.08, 140.359, 4.00439, 0, 0, 0, NULL, 100, 0),
(322960, 18, 1085.4, 127.791, 1.79286, 0, 0, 0, NULL, 100, 0),
(322960, 19, 1096.18, 118.597, 0.878709, 0, 0, 0, NULL, 100, 0),
(322960, 20, 1099.51, 109.012, 0.82763, 0, 0, 0, NULL, 100, 0),
(322960, 21, 1098.44, 95.0113, -0.082417, 0, 0, 0, NULL, 100, 0),
(322960, 22, 1098.89, 82.3005, -1.11889, 0, 0, 0, NULL, 100, 0),
(322960, 23, 1102.5, 69.4638, -2.65482, 0, 0, 0, NULL, 100, 0),
(322960, 24, 1096.98, 57.1571, -0.905613, 0, 0, 0, NULL, 100, 0),
(322960, 25, 1091.63, 44.6145, 1.43142, 0, 0, 0, NULL, 100, 0),
(322960, 26, 1086.33, 32.9632, 3.48268, 0, 0, 0, NULL, 100, 0),
(322960, 27, 1085.05, 20.6796, 4.1329, 0, 0, 0, NULL, 100, 0),
(322960, 28, 1092.92, 7.78493, 3.96483, 0, 0, 0, NULL, 100, 0),
(322960, 29, 1103.3, 1.74843, 3.81136, 0, 0, 0, NULL, 100, 0),
(322960, 30, 1108.9, 6.66384, 1.96549, 0, 0, 0, NULL, 100, 0),
(322960, 31, 1119.75, 17.2556, 2.57707, 0, 0, 0, NULL, 100, 0),
(322960, 32, 1126.57, 33.5558, 4.28383, 0, 0, 0, NULL, 100, 0),
(322960, 33, 1125.52, 48.322, 2.38885, 0, 0, 0, NULL, 100, 0),
(322960, 34, 1129.9, 61.2477, 0.658247, 0, 0, 0, NULL, 100, 0),
(322960, 35, 1132.7, 73.0197, -0.850287, 0, 0, 0, NULL, 100, 0),
(322960, 36, 1136.62, 88.9702, -5.98268, 0, 0, 0, NULL, 100, 0),
(322960, 37, 1137.55, 99.0144, -5.72923, 0, 0, 0, NULL, 100, 0),
(322960, 38, 1134.35, 114.687, -1.26875, 0, 0, 0, NULL, 100, 0),
(322960, 39, 1130.8, 132.522, 0.171318, 0, 0, 0, NULL, 100, 0),
(322960, 40, 1128.71, 146.011, 1.77626, 0, 0, 0, NULL, 100, 0),
(322960, 41, 1126.63, 159.501, -0.202374, 0, 0, 0, NULL, 100, 0),
(322960, 42, 1128.08, 177.528, 1.14402, 0, 0, 0, NULL, 100, 0),
(322960, 43, 1132.36, 188.767, 2.93699, 0, 0, 0, NULL, 100, 0),
(322960, 44, 1141.54, 204.471, 2.41599, 0, 0, 0, NULL, 100, 0),
(322960, 45, 1149.88, 215.243, 2.58233, 0, 0, 0, NULL, 100, 0),
(322960, 46, 1161.78, 225.303, 3.65146, 0, 0, 0, NULL, 100, 0),
(322960, 47, 1172.94, 233.153, 5.6798, 0, 0, 0, NULL, 100, 0),
(322960, 48, 1189.03, 229.718, 8.99644, 0, 0, 0, NULL, 100, 0),
(322960, 49, 1196.27, 219.676, 6.68681, 0, 0, 0, NULL, 100, 0),
(322960, 50, 1207.26, 211.585, 6.36744, 0, 0, 0, NULL, 100, 0),
(322960, 51, 1218.4, 203.704, 6.20296, 0, 0, 0, NULL, 100, 0),
(322960, 52, 1229.44, 193.181, 4.62727, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (32296, 322960, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 32296);

-- Importing creature with guid 29247 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1029112 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1029112 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1029112 AND spawnType = 0 WHERE ce.spawnID = 1029112;
INSERT INTO creature_entry (spawnID, entry) VALUES (29247, 11862);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (29247, 1, 1, 5562, 0, 824.854, 933.05,  155.485, 2.16375, 300, 0, 0, 713, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (29247, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 26987 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (26987, 5644);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (26987, 1, 1, 3791, 0, -331.142, 1766.99,  139.6, 3.85718, 300, 0, 0, 1342, 0, 0, 0, 5, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (26987, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (3, 0, 26987);

-- Importing creature with guid 26895 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 26895 WHERE entry = 3057;
UPDATE conditions SET ConditionValue3 = 26895 WHERE ConditionValue3 = 1097110 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -26895 WHERE SourceEntry = -1097110 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -26895 WHERE entryorguid = -1097110 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 26895 WHERE target_param1 = 1097110 AND target_type = 10;
UPDATE spawn_group SET spawnID = 26895 WHERE spawnID = 1097110 AND spawnType = 0;

-- Importing creature with guid 26577 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1026444 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1026444 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1026444 AND spawnType = 0 WHERE ce.spawnID = 1026444;
INSERT INTO creature_entry (spawnID, entry) VALUES (26577, 2988);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (26577, 1, 1, 1624, 1, -2369.16, -876.936,  -9.42208, 4.55148, 250, 0, 0, 198, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(265770, 1, -2369.16, -876.936, -9.42208, 0, 0, 0, NULL, 100, 0),
(265770, 2, -2366.53, -856.691, -9.36085, 0, 0, 0, NULL, 100, 0),
(265770, 3, -2357.15, -811.864, -9.42138, 0, 0, 0, NULL, 100, 0),
(265770, 4, -2345.23, -748.967, -9.42454, 0, 0, 0, NULL, 100, 0),
(265770, 5, -2341.13, -700.582, -9.29519, 0, 0, 0, NULL, 100, 0),
(265770, 6, -2335.57, -673.555, -9.41523, 0, 0, 0, NULL, 100, 0),
(265770, 7, -2323.8, -641.711, -9.34184, 0, 0, 0, NULL, 100, 0),
(265770, 8, -2310.09, -613.217, -9.42431, 0, 0, 0, NULL, 100, 0),
(265770, 9, -2295.38, -586.808, -9.4231, 2.39676, 21600, 0, NULL, 100, 0),
(265770, 10, -2323.09, -595.375, -9.42242, 0, 0, 0, NULL, 100, 0),
(265770, 11, -2312.27, -614.66, -9.42405, 0, 0, 0, NULL, 100, 0),
(265770, 12, -2326.41, -645.536, -9.34993, 0, 0, 0, NULL, 100, 0),
(265770, 13, -2336.51, -676.214, -9.40864, 0, 0, 0, NULL, 100, 0),
(265770, 14, -2344.05, -723.843, -9.42353, 0, 0, 0, NULL, 100, 0),
(265770, 15, -2346.75, -760.367, -9.42353, 0, 0, 0, NULL, 100, 0),
(265770, 16, -2357.56, -811.652, -9.42353, 0, 0, 0, NULL, 100, 0),
(265770, 17, -2366.12, -855.675, -9.32695, 0, 0, 0, NULL, 100, 0),
(265770, 18, -2376.83, -896.428, -9.39643, 6.2165, 21600, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (26577, 265770, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 26568 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (26568, 11944);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (26568, 1, 1, 11902, 0, -2328.49, -382.269,  -7.89994, 1.79769, 250, 0, 0, 102, 0, 0, 0, 5, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (26568, NULL, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 26208 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1026075 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1026075 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1026075 AND spawnType = 0 WHERE ce.spawnID = 1026075;
INSERT INTO creature_entry (spawnID, entry) VALUES (26208, 3229);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (26208, 1, 1, 1254, 0, -3103.75, -1196.7,  85.6689, 5.48033, 250, 0, 0, 102, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (26208, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 26053 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1025921 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1025921 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1025921 AND spawnType = 0 WHERE ce.spawnID = 1025921;
INSERT INTO creature_entry (spawnID, entry) VALUES (26053, 3224);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (26053, 1, 1, 10181, 1, -2258.76, -296.012,  -9.42604, 4.05437, 250, 5, 0, 600, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(260530, 1, -2261.21, -304.058, -9.42604, 0, 0, 0, NULL, 100, 0),
(260530, 2, -2257.54, -312.712, -9.42604, 0, 25000, 0, NULL, 100, 0),
(260530, 3, -2265.13, -312.055, -9.42604, 0, 0, 0, NULL, 100, 0),
(260530, 4, -2272.79, -315.722, -9.42604, 0, 25000, 0, NULL, 100, 0),
(260530, 5, -2270.73, -308.69, -9.42604, 0, 0, 0, NULL, 100, 0),
(260530, 6, -2274.7, -300.973, -9.42604, 0, 25000, 0, NULL, 100, 0),
(260530, 7, -2265.9, -301.458, -9.42604, 0, 0, 0, NULL, 100, 0),
(260530, 8, -2259.11, -296.448, -9.42604, 0, 25000, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (26053, 260530, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 24790 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1024659 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1024659 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1024659 AND spawnType = 0 WHERE ce.spawnID = 1024659;
INSERT INTO creature_entry (spawnID, entry) VALUES (24790, 4721);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (24790, 1, 1, 2738, 1, -1207.75, -57.0394,  158.601, 4.74729, 250, 0, 0, 713, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (24790, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 24709 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (24709, 20499);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (24709, 1, 1, 18290, 0, -1384.87, -69.0243,  159.92, 3.89208, 250, 0, 0, 6986, 0, 0, 0, 5, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (24709, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 23741 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1023613 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1023613 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1023613 AND spawnType = 0 WHERE ce.spawnID = 1023613;
DELETE FROM waypoint_data WHERE id = 2077;
INSERT INTO creature_entry (spawnID, entry) VALUES (23741, 6498);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (23741, 1, 1, 5239, 0, -7223.92, -1582.28,  -269.345, 0.518643, 600, 0, 0, 7599, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(237410, 1, -7246.9, -1564.65, -273.213, 0, 0, 0, NULL, 100, 0),
(237410, 2, -7295.44, -1561.83, -272.039, 0, 0, 0, NULL, 100, 0),
(237410, 3, -7313.57, -1570.71, -271.525, 0, 0, 0, NULL, 100, 0),
(237410, 4, -7340.28, -1583, -272.641, 0, 0, 0, NULL, 100, 0),
(237410, 5, -7369.2, -1587.4, -276.399, 0, 0, 0, NULL, 100, 0),
(237410, 6, -7404.05, -1594.68, -273.52, 0, 0, 0, NULL, 100, 0),
(237410, 7, -7420.93, -1611.74, -275.854, 0, 0, 0, NULL, 100, 0),
(237410, 8, -7448.23, -1611.88, -276.741, 0, 0, 0, NULL, 100, 0),
(237410, 9, -7473.32, -1605.2, -276.227, 0, 0, 0, NULL, 100, 0),
(237410, 10, -7489.71, -1586.38, -274.98, 0, 0, 0, NULL, 100, 0),
(237410, 11, -7518.57, -1584.45, -272.694, 0, 0, 0, NULL, 100, 0),
(237410, 12, -7542.85, -1588.22, -271.853, 0, 0, 0, NULL, 100, 0),
(237410, 13, -7554.35, -1603.8, -271.973, 0, 0, 0, NULL, 100, 0),
(237410, 14, -7582.9, -1603.94, -271.972, 0, 0, 0, NULL, 100, 0),
(237410, 15, -7617.38, -1604.88, -272.097, 0, 0, 0, NULL, 100, 0),
(237410, 16, -7650.82, -1613.32, -271.972, 0, 0, 0, NULL, 100, 0),
(237410, 17, -7667.4, -1636.99, -272.097, 0, 0, 0, NULL, 100, 0),
(237410, 18, -7702.58, -1643.71, -272.097, 0, 0, 0, NULL, 100, 0),
(237410, 19, -7727.59, -1637.04, -272.097, 0, 0, 0, NULL, 100, 0),
(237410, 20, -7736.22, -1602.91, -271.722, 0, 0, 0, NULL, 100, 0),
(237410, 21, -7756.19, -1594.47, -269.77, 0, 0, 0, NULL, 100, 0),
(237410, 22, -7788.7, -1608.79, -271.316, 0, 0, 0, NULL, 100, 0),
(237410, 23, -7823.67, -1614.99, -268.353, 0, 0, 0, NULL, 100, 0),
(237410, 24, -7788.7, -1608.79, -271.316, 0, 0, 0, NULL, 100, 0),
(237410, 25, -7756.19, -1594.47, -269.77, 0, 0, 0, NULL, 100, 0),
(237410, 26, -7736.22, -1602.91, -271.722, 0, 0, 0, NULL, 100, 0),
(237410, 27, -7727.59, -1637.04, -272.097, 0, 0, 0, NULL, 100, 0),
(237410, 28, -7702.58, -1643.71, -272.097, 0, 0, 0, NULL, 100, 0),
(237410, 29, -7667.4, -1636.99, -272.097, 0, 0, 0, NULL, 100, 0),
(237410, 30, -7650.82, -1613.32, -271.972, 0, 0, 0, NULL, 100, 0),
(237410, 31, -7617.38, -1604.88, -272.097, 0, 0, 0, NULL, 100, 0),
(237410, 32, -7582.9, -1603.94, -271.972, 0, 0, 0, NULL, 100, 0),
(237410, 33, -7554.35, -1603.8, -271.973, 0, 0, 0, NULL, 100, 0),
(237410, 34, -7542.85, -1588.22, -271.853, 0, 0, 0, NULL, 100, 0),
(237410, 35, -7518.57, -1584.45, -272.694, 0, 0, 0, NULL, 100, 0),
(237410, 36, -7489.71, -1586.38, -274.98, 0, 0, 0, NULL, 100, 0),
(237410, 37, -7473.32, -1605.2, -276.227, 0, 0, 0, NULL, 100, 0),
(237410, 38, -7448.23, -1611.88, -276.741, 0, 0, 0, NULL, 100, 0),
(237410, 39, -7421.02, -1611.81, -275.854, 0, 0, 0, NULL, 100, 0),
(237410, 40, -7404.05, -1594.68, -273.52, 0, 0, 0, NULL, 100, 0),
(237410, 41, -7369.2, -1587.4, -276.399, 0, 0, 0, NULL, 100, 0),
(237410, 42, -7340.28, -1583, -272.641, 0, 0, 0, NULL, 100, 0),
(237410, 43, -7313.57, -1570.71, -271.525, 0, 0, 0, NULL, 100, 0),
(237410, 44, -7295.44, -1561.83, -272.039, 0, 0, 0, NULL, 100, 0),
(237410, 45, -7246.9, -1564.65, -273.213, 0, 0, 0, NULL, 100, 0),
(237410, 46, -7224.02, -1582.26, -269.595, 0, 0, 0, NULL, 100, 0),
(237410, 47, -7200.92, -1582.77, -261.47, 0, 0, 0, NULL, 100, 0),
(237410, 48, -7164.6, -1585.13, -271.267, 0, 0, 0, NULL, 100, 0),
(237410, 49, -7135.3, -1574.02, -272.017, 0, 0, 0, NULL, 100, 0),
(237410, 50, -7116.29, -1580.97, -271.466, 0, 0, 0, NULL, 100, 0),
(237410, 51, -7090.49, -1590.8, -272.097, 0, 0, 0, NULL, 100, 0),
(237410, 52, -7077.85, -1619.28, -272.097, 0, 0, 0, NULL, 100, 0),
(237410, 53, -7052.08, -1624.29, -272.097, 0, 0, 0, NULL, 100, 0),
(237410, 54, -7019.86, -1621.38, -271.996, 0, 0, 0, NULL, 100, 0),
(237410, 55, -6977.86, -1621.3, -273.121, 0, 0, 0, NULL, 100, 0),
(237410, 56, -6950.43, -1608.54, -271.463, 0, 0, 0, NULL, 100, 0),
(237410, 57, -6934.28, -1590.88, -271.71, 0, 0, 0, NULL, 100, 0),
(237410, 58, -6907.77, -1578.92, -272.108, 0, 0, 0, NULL, 100, 0),
(237410, 59, -6877.01, -1573.64, -271.722, 0, 0, 0, NULL, 100, 0),
(237410, 60, -6853.15, -1555.5, -267.43, 0, 0, 0, NULL, 100, 0),
(237410, 61, -6819.54, -1552.88, -272.478, 0, 0, 0, NULL, 100, 0),
(237410, 62, -6785.01, -1550.58, -271.472, 0, 0, 0, NULL, 100, 0),
(237410, 63, -6819.54, -1552.88, -272.478, 0, 0, 0, NULL, 100, 0),
(237410, 64, -6853.15, -1555.5, -267.43, 0, 0, 0, NULL, 100, 0),
(237410, 65, -6877.01, -1573.64, -271.722, 0, 0, 0, NULL, 100, 0),
(237410, 66, -6907.77, -1578.92, -272.108, 0, 0, 0, NULL, 100, 0),
(237410, 67, -6934.28, -1590.88, -271.71, 0, 0, 0, NULL, 100, 0),
(237410, 68, -6950.43, -1608.54, -271.463, 0, 0, 0, NULL, 100, 0),
(237410, 69, -6977.73, -1621.29, -273.121, 0, 0, 0, NULL, 100, 0),
(237410, 70, -7019.86, -1621.38, -271.996, 0, 0, 0, NULL, 100, 0),
(237410, 71, -7052.08, -1624.29, -272.097, 0, 0, 0, NULL, 100, 0),
(237410, 72, -7077.85, -1619.28, -272.097, 0, 0, 0, NULL, 100, 0),
(237410, 73, -7090.49, -1590.8, -272.097, 0, 0, 0, NULL, 100, 0),
(237410, 74, -7116.29, -1580.97, -271.466, 0, 0, 0, NULL, 100, 0),
(237410, 75, -7135.3, -1574.02, -272.017, 0, 0, 0, NULL, 100, 0),
(237410, 76, -7164.6, -1585.13, -271.267, 0, 0, 0, NULL, 100, 0),
(237410, 77, -7200.92, -1582.77, -261.47, 0, 0, 0, NULL, 100, 0),
(237410, 78, -7223.92, -1582.28, -269.345, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (23741, 237410, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 23419 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1023291 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1023291 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1023291 AND spawnType = 0 WHERE ce.spawnID = 1023291;
DELETE FROM waypoint_data WHERE id = 512;
INSERT INTO creature_entry (spawnID, entry) VALUES (23419, 21448);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (23419, 1, 1, 11376, 1, -7164.7, -3855.56,  15.3137, 1.39094, 300, 0, 0, 7456, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(234190, 1, -7173.3, -3852.22, 15.3137, 0, 0, 0, NULL, 100, 0),
(234190, 2, -7172.24, -3846.01, 15.3137, 0, 0, 0, NULL, 100, 0),
(234190, 3, -7170.27, -3835.18, 15.3137, 0, 0, 0, NULL, 100, 0),
(234190, 4, -7165.5, -3836.05, 15.3137, 0, 0, 0, NULL, 100, 0),
(234190, 5, -7153.95, -3838.11, 15.3137, 0, 0, 0, NULL, 100, 0),
(234190, 6, -7148.61, -3840.25, 15.3137, 0, 0, 0, NULL, 100, 0),
(234190, 7, -7149.65, -3845.81, 15.3137, 0, 0, 0, NULL, 100, 0),
(234190, 8, -7151.4, -3852.36, 15.3137, 0, 0, 0, NULL, 100, 0),
(234190, 9, -7154.35, -3855.87, 15.3137, 0, 0, 0, NULL, 100, 0),
(234190, 10, -7164.7, -3855.56, 15.3137, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (23419, 234190, 0, 0, 0, 2, 0, NULL);

-- Importing creature with guid 23280 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1023153 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1023153 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1023153 AND spawnType = 0 WHERE ce.spawnID = 1023153;
INSERT INTO creature_entry (spawnID, entry) VALUES (23280, 19861);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (23280, 1, 1, 19224, 1, -7139.31, -3761.98,  9.54388, 1.78024, 300, 0, 0, 6986, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (23280, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 18599 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1018479 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1018479 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1018479 AND spawnType = 0 WHERE ce.spawnID = 1018479;
DELETE FROM waypoint_scripts WHERE id = 377;
DELETE FROM waypoint_scripts WHERE id = 377;
DELETE FROM waypoint_data WHERE id = 377;
INSERT INTO creature_entry (spawnID, entry) VALUES (18599, 23995);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (18599, 1, 1, 7049, 1, -4629.94, -3176.12,  41.2235, 6.26344, 360, 0, 0, 1180, 1236, 0, 0, 0, 10);

-- Importing creature with guid 18589 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1018469 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1018469 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1018469 AND spawnType = 0 WHERE ce.spawnID = 1018469;
INSERT INTO creature_entry (spawnID, entry) VALUES (18589, 23835);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (18589, 1, 1, 4670, 1, -3839.59, -4562.36,  8.63999, 3.89208, 360, 0, 0, 4906, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(185890, 1, -3839.59, -4562.36, 8.63999, 0, 0, 0, NULL, 100, 0),
(185890, 2, -3834.42, -4560.12, 8.76499, 0, 0, 0, NULL, 100, 0),
(185890, 3, -3831.5, -4554.75, 9.01716, 0, 0, 0, NULL, 100, 0),
(185890, 4, -3832.31, -4547.42, 9.26716, 0, 0, 0, NULL, 100, 0),
(185890, 5, -3837.41, -4543.65, 9.26499, 0, 0, 0, NULL, 100, 0),
(185890, 6, -3843.69, -4542.71, 9.13999, 0, 0, 0, NULL, 100, 0),
(185890, 7, -3848.83, -4545.31, 8.88999, 0, 0, 0, NULL, 100, 0),
(185890, 8, -3852.15, -4551.4, 8.51499, 0, 0, 0, NULL, 100, 0),
(185890, 9, -3849.58, -4557.87, 8.51499, 0, 0, 0, NULL, 100, 0),
(185890, 10, -3846.69, -4558.22, 8.51499, 0, 0, 0, NULL, 100, 0),
(185890, 11, -3846.69, -4558.22, 8.51499, 3.89208, 50000, 0, NULL, 100, 0),
(185890, 12, -3844.93, -4561.82, 8.51499, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (18589, 185890, 2402, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 21679 with import type REPLACE_ALL
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1021554 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1021554 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1021554 AND spawnType = 0 WHERE ce.spawnID = 1021554;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1021555 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1021555 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1021555 AND spawnType = 0 WHERE ce.spawnID = 1021555;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1021556 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1021556 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1021556 AND spawnType = 0 WHERE ce.spawnID = 1021556;
INSERT INTO creature_entry (spawnID, entry) VALUES (21679, 4250);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (21679, 1, 1, 2726, 0, -5065.41, -1433.43,  -52.0541, 2.67263, 300, 0, 0, 664, 0, 0, 0, 0, 10);
INSERT INTO creature_entry (spawnID, entry) VALUES (21678, 4250);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (21678, 1, 1, 2726, 0, -5065.41, -1433.43,  -52.0541, 2.73209, 300, 0, 0, 664, 0, 0, 0, 0, 10);
INSERT INTO creature_entry (spawnID, entry) VALUES (21677, 4250);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (21677, 1, 1, 2726, 0, -5065.41, -1433.43,  -52.0541, 2.65175, 300, 0, 0, 664, 0, 0, 0, 0, 10);

-- Importing creature with guid 21678 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 21677 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 21580 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1021457 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1021457 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1021457 AND spawnType = 0 WHERE ce.spawnID = 1021457;
INSERT INTO creature_entry (spawnID, entry) VALUES (21580, 10617);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (21580, 1, 1, 9532, 0, -5128.15, -2080.7,  -37.4284, 0.847854, 300, 0, 1, 787, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(215800, 1, -5128.15, -2080.7, -37.4284, 0, 0, 1, NULL, 100, 0),
(215800, 2, -5144.75, -2055.66, -46.1665, 0, 0, 1, NULL, 100, 0),
(215800, 3, -5160.79, -2041.71, -54.7244, 0, 0, 1, NULL, 100, 0),
(215800, 4, -5184.25, -2033.33, -58.4057, 0, 0, 1, NULL, 100, 0),
(215800, 5, -5201.13, -2008.87, -59.3416, 0, 0, 1, NULL, 100, 0),
(215800, 6, -5213.59, -1976.84, -62.5793, 0, 0, 1, NULL, 100, 0),
(215800, 7, -5217.54, -1953.07, -63.4123, 0, 0, 1, NULL, 100, 0),
(215800, 8, -5222.41, -1918.43, -64.0845, 0, 0, 1, NULL, 100, 0),
(215800, 9, -5227.44, -1890.93, -64.2982, 0, 0, 1, NULL, 100, 0),
(215800, 10, -5237.3, -1857.38, -63.3124, 0, 0, 1, NULL, 100, 0),
(215800, 11, -5240.2, -1831, -62.6058, 0, 0, 1, NULL, 100, 0),
(215800, 12, -5232.75, -1803.31, -61.8011, 0, 0, 1, NULL, 100, 0),
(215800, 13, -5220.5, -1781.18, -61.1364, 0, 0, 1, NULL, 100, 0),
(215800, 14, -5207.74, -1765.52, -61.4062, 0, 0, 1, NULL, 100, 0),
(215800, 15, -5183.21, -1749.96, -63.1477, 0, 0, 1, NULL, 100, 0),
(215800, 16, -5155.25, -1748.13, -64.991, 0, 0, 1, NULL, 100, 0),
(215800, 17, -5127.43, -1751.27, -66.9704, 0, 0, 1, NULL, 100, 0),
(215800, 18, -5099.64, -1754.6, -66.3, 0, 0, 1, NULL, 100, 0),
(215800, 19, -5077.77, -1756.72, -66.3745, 0, 0, 1, NULL, 100, 0),
(215800, 20, -5035.87, -1759.26, -65.9249, 0, 0, 1, NULL, 100, 0),
(215800, 21, -5003.5, -1755.74, -64.9413, 0, 0, 1, NULL, 100, 0),
(215800, 22, -4983.33, -1736.51, -63.6987, 0, 0, 1, NULL, 100, 0),
(215800, 23, -4964.09, -1716.17, -61.5792, 0, 0, 1, NULL, 100, 0),
(215800, 24, -4951.02, -1689.86, -57.4292, 0, 0, 1, NULL, 100, 0),
(215800, 25, -4951.84, -1666.27, -51.5895, 0, 0, 1, NULL, 100, 0),
(215800, 26, -4960.94, -1639.82, -46.8559, 0, 0, 1, NULL, 100, 0),
(215800, 27, -4966.78, -1612.74, -43.3814, 0, 0, 1, NULL, 100, 0),
(215800, 28, -4969.74, -1589.2, -43.3987, 0, 0, 1, NULL, 100, 0),
(215800, 29, -4970.27, -1554.24, -46.7418, 0, 0, 1, NULL, 100, 0),
(215800, 30, -4965.65, -1536.53, -47.0204, 0, 0, 1, NULL, 100, 0),
(215800, 31, -4947.95, -1518.8, -48.8465, 0, 0, 1, NULL, 100, 0),
(215800, 32, -4921.59, -1495.78, -48.6897, 0, 0, 1, NULL, 100, 0),
(215800, 33, -4903.99, -1476.15, -49.4282, 0, 0, 1, NULL, 100, 0),
(215800, 34, -4890.35, -1454.33, -51.2389, 0, 0, 1, NULL, 100, 0),
(215800, 35, -4879.88, -1428.38, -50.2016, 0, 0, 1, NULL, 100, 0),
(215800, 36, -4861.08, -1406.41, -52.7791, 0, 0, 1, NULL, 100, 0),
(215800, 37, -4840.36, -1387.58, -53.2012, 0, 0, 1, NULL, 100, 0),
(215800, 38, -4816.1, -1368.69, -54.4062, 0, 0, 1, NULL, 100, 0),
(215800, 39, -4803.63, -1359.65, -54.1829, 0, 0, 1, NULL, 100, 0),
(215800, 40, -4785.89, -1338.23, -52.0609, 0, 0, 1, NULL, 100, 0),
(215800, 41, -4773.8, -1313.74, -49.2194, 0, 0, 1, NULL, 100, 0),
(215800, 42, -4758.39, -1290.4, -48.7364, 0, 0, 1, NULL, 100, 0),
(215800, 43, -4738, -1266.39, -53.0178, 0, 0, 1, NULL, 100, 0),
(215800, 44, -4711.26, -1256.68, -52.6156, 0, 0, 1, NULL, 100, 0),
(215800, 45, -4679.18, -1252.81, -51.4631, 0, 0, 1, NULL, 100, 0),
(215800, 46, -4651.67, -1247.89, -52.6598, 0, 0, 1, NULL, 100, 0),
(215800, 47, -4623.78, -1245.66, -52.9739, 0, 0, 0, NULL, 100, 0),
(215800, 48, -4607.12, -1243.64, -53.0644, 0, 0, 0, NULL, 100, 0),
(215800, 49, -4587.44, -1247.87, -53.8559, 0, 0, 0, NULL, 100, 0),
(215800, 50, -4585.42, -1249.45, -53.8577, 0, 0, 0, NULL, 100, 0),
(215800, 51, -4598.8, -1245.33, -53.5651, 0, 0, 0, NULL, 100, 0),
(215800, 52, -4614.47, -1243.16, -52.9688, 0, 0, 1, NULL, 100, 0),
(215800, 53, -4628.97, -1251.46, -52.7535, 0, 0, 1, NULL, 100, 0),
(215800, 54, -4645.43, -1247.66, -52.6382, 0, 0, 1, NULL, 100, 0),
(215800, 55, -4661.28, -1248.2, -52.8033, 0, 0, 1, NULL, 100, 0),
(215800, 56, -4681.65, -1253.29, -51.3422, 0, 0, 1, NULL, 100, 0),
(215800, 57, -4715.11, -1257.44, -52.7916, 0, 0, 1, NULL, 100, 0),
(215800, 58, -4747.49, -1264.12, -53.1917, 0, 0, 1, NULL, 100, 0),
(215800, 59, -4759.03, -1286.65, -49.0202, 0, 0, 1, NULL, 100, 0),
(215800, 60, -4774.15, -1310.65, -48.9689, 0, 0, 1, NULL, 100, 0),
(215800, 61, -4784.42, -1342.36, -53.1254, 0, 0, 1, NULL, 100, 0),
(215800, 62, -4809.4, -1362.98, -54.1859, 0, 0, 1, NULL, 100, 0),
(215800, 63, -4837.89, -1383.29, -53.6682, 0, 0, 1, NULL, 100, 0),
(215800, 64, -4867.01, -1404.41, -53.3364, 0, 0, 1, NULL, 100, 0),
(215800, 65, -4887.81, -1414.73, -52.4351, 0, 0, 1, NULL, 100, 0),
(215800, 66, -4895.42, -1429.2, -50.8449, 0, 0, 1, NULL, 100, 0),
(215800, 67, -4896.18, -1457.18, -50.691, 0, 0, 1, NULL, 100, 0),
(215800, 68, -4904.7, -1480.07, -49.5298, 0, 0, 1, NULL, 100, 0),
(215800, 69, -4919.07, -1495.05, -48.9062, 0, 0, 1, NULL, 100, 0),
(215800, 70, -4966.22, -1533.18, -47.4609, 0, 0, 1, NULL, 100, 0),
(215800, 71, -4976.06, -1543.9, -47.9824, 0, 0, 1, NULL, 100, 0),
(215800, 72, -4977.48, -1556.97, -47.3905, 0, 0, 1, NULL, 100, 0),
(215800, 73, -4969.22, -1596.89, -42.6042, 0, 0, 1, NULL, 100, 0),
(215800, 74, -4965.56, -1625.02, -44.3561, 0, 0, 1, NULL, 100, 0),
(215800, 75, -4960.27, -1644.88, -47.9104, 0, 0, 1, NULL, 100, 0),
(215800, 76, -4949.77, -1671.03, -52.508, 0, 0, 1, NULL, 100, 0),
(215800, 77, -4947.92, -1689.25, -57.2223, 0, 0, 1, NULL, 100, 0),
(215800, 78, -4956.68, -1709.58, -60.8799, 0, 0, 1, NULL, 100, 0),
(215800, 79, -4963.58, -1727.99, -61.5273, 0, 0, 1, NULL, 100, 0),
(215800, 80, -4951.21, -1744.92, -58.6239, 0, 0, 1, NULL, 100, 0),
(215800, 81, -4937.35, -1760.68, -54.7025, 0, 0, 1, NULL, 100, 0),
(215800, 82, -4914.46, -1782.89, -39.8799, 0, 0, 1, NULL, 100, 0),
(215800, 83, -4896.94, -1794.46, -35.2214, 0, 0, 1, NULL, 100, 0),
(215800, 84, -4880.45, -1807.45, -40.8635, 0, 0, 1, NULL, 100, 0),
(215800, 85, -4863.39, -1822.36, -50.1641, 0, 0, 1, NULL, 100, 0),
(215800, 86, -4862.39, -1836.32, -50.3177, 0, 0, 1, NULL, 100, 0),
(215800, 87, -4861.69, -1862.06, -52.8515, 0, 0, 1, NULL, 100, 0),
(215800, 88, -4872.84, -1883.31, -52.6598, 0, 0, 1, NULL, 100, 0),
(215800, 89, -4870.8, -1892.23, -52.4949, 0, 0, 1, NULL, 100, 0),
(215800, 90, -4849.78, -1918.97, -48.7531, 0, 0, 1, NULL, 100, 0),
(215800, 91, -4830.53, -1939.24, -45.0412, 0, 0, 1, NULL, 100, 0),
(215800, 92, -4811.46, -1962.68, -50.8058, 0, 0, 1, NULL, 100, 0),
(215800, 93, -4803.71, -1987.76, -57.8123, 0, 0, 1, NULL, 100, 0),
(215800, 94, -4807.74, -2007.69, -56.5951, 0, 0, 1, NULL, 100, 0),
(215800, 95, -4818.89, -2033.34, -46.8554, 0, 0, 1, NULL, 100, 0),
(215800, 96, -4830.5, -2059.52, -39.1652, 0, 0, 1, NULL, 100, 0),
(215800, 97, -4830.7, -2073.52, -35.956, 0, 0, 1, NULL, 100, 0),
(215800, 98, -4836.33, -2090.58, -31.3441, 0, 0, 1, NULL, 100, 0),
(215800, 99, -4849.24, -2105.09, -32.3089, 0, 0, 1, NULL, 100, 0),
(215800, 100, -4860.98, -2112.72, -36.2315, 0, 0, 1, NULL, 100, 0),
(215800, 101, -4887.55, -2130.23, -46.5679, 0, 0, 1, NULL, 100, 0),
(215800, 102, -4903.91, -2142.86, -52.8263, 0, 0, 1, NULL, 100, 0),
(215800, 103, -4925.91, -2157.61, -56.6051, 0, 0, 1, NULL, 100, 0),
(215800, 104, -4951.92, -2172.73, -54.9684, 0, 0, 1, NULL, 100, 0),
(215800, 105, -4968.65, -2184.29, -53.8151, 0, 0, 1, NULL, 100, 0),
(215800, 106, -4986.17, -2202.18, -53.7698, 0, 0, 1, NULL, 100, 0),
(215800, 107, -5006.88, -2230.39, -53.1766, 0, 0, 1, NULL, 100, 0),
(215800, 108, -5027.83, -2247.92, -52.7676, 0, 0, 1, NULL, 100, 0),
(215800, 109, -5042.62, -2243.48, -53.2466, 0, 0, 1, NULL, 100, 0),
(215800, 110, -5049.04, -2230.15, -53.9278, 0, 0, 0, NULL, 100, 0),
(215800, 111, -5047.36, -2223.67, -54.0982, 0, 0, 0, NULL, 100, 0),
(215800, 112, -5039.56, -2203.82, -54.5008, 0, 0, 0, NULL, 100, 0),
(215800, 113, -5042.73, -2213.82, -54.4114, 0, 0, 0, NULL, 100, 0),
(215800, 114, -5049.87, -2229.96, -53.9646, 0, 0, 0, NULL, 100, 0),
(215800, 115, -5062.33, -2216.28, -54.636, 0, 0, 1, NULL, 100, 0),
(215800, 116, -5065.97, -2193.27, -55.6623, 0, 0, 1, NULL, 100, 0),
(215800, 117, -5068.2, -2158.4, -53.611, 0, 0, 1, NULL, 100, 0),
(215800, 118, -5074.13, -2135.69, -47.0288, 0, 0, 1, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (21580, 215800, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 21388 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1021265 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1021265 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1021265 AND spawnType = 0 WHERE ce.spawnID = 1021265;
INSERT INTO creature_entry (spawnID, entry) VALUES (21388, 5933);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (21388, 1, 1, 9418, 0, -5065.41, -1433.43,  -52.0541, 2.29257, 7200, 0, 1, 1308, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(213880, 1, -5065.41, -1433.43, -52.0541, 0, 0, 1, NULL, 100, 0),
(213880, 2, -5080.29, -1409.72, -52.2031, 0, 0, 1, NULL, 100, 0),
(213880, 3, -5096.5, -1378.73, -51.7243, 0, 0, 1, NULL, 100, 0),
(213880, 4, -5104.2, -1358, -50.8247, 0, 0, 1, NULL, 100, 0),
(213880, 5, -5112.8, -1324.15, -50.0947, 0, 0, 1, NULL, 100, 0),
(213880, 6, -5111.58, -1301.39, -49.2679, 0, 0, 1, NULL, 100, 0),
(213880, 7, -5107.58, -1280.8, -48.9439, 0, 0, 1, NULL, 100, 0),
(213880, 8, -5096.66, -1257.09, -48.4711, 0, 0, 1, NULL, 100, 0),
(213880, 9, -5082.09, -1233.19, -50.9921, 0, 0, 1, NULL, 100, 0),
(213880, 10, -5065.16, -1214.56, -54.0949, 0, 0, 1, NULL, 100, 0),
(213880, 11, -5048.08, -1208.92, -54.3934, 0, 0, 1, NULL, 100, 0),
(213880, 12, -5025.27, -1212.7, -53.5746, 0, 0, 1, NULL, 100, 0),
(213880, 13, -5007.48, -1223.78, -50.7977, 0, 0, 1, NULL, 100, 0),
(213880, 14, -4988.77, -1237.38, -47.9422, 0, 0, 1, NULL, 100, 0),
(213880, 15, -4965.53, -1252.93, -46.2135, 0, 0, 1, NULL, 100, 0),
(213880, 16, -4945.34, -1262.73, -48.3219, 0, 0, 1, NULL, 100, 0),
(213880, 17, -4957.1, -1257.53, -47.1724, 0, 0, 1, NULL, 100, 0),
(213880, 18, -4974.35, -1245.55, -46.5483, 0, 0, 1, NULL, 100, 0),
(213880, 19, -4997.27, -1229.47, -49.7777, 0, 0, 1, NULL, 100, 0),
(213880, 20, -5014.47, -1217.42, -52.3196, 0, 0, 1, NULL, 100, 0),
(213880, 21, -5032.64, -1209.56, -54.1869, 0, 0, 1, NULL, 100, 0),
(213880, 22, -5056.29, -1214.4, -54.0739, 0, 0, 1, NULL, 100, 0),
(213880, 23, -5073.8, -1225.95, -52.3959, 0, 0, 1, NULL, 100, 0),
(213880, 24, -5089.78, -1243.75, -49.3146, 0, 0, 1, NULL, 100, 0),
(213880, 25, -5098.03, -1259.23, -48.5008, 0, 0, 1, NULL, 100, 0),
(213880, 26, -5106.21, -1282.82, -48.5298, 0, 0, 1, NULL, 100, 0),
(213880, 27, -5109.1, -1303.61, -48.8349, 0, 0, 1, NULL, 100, 0),
(213880, 28, -5108.13, -1322.88, -49.5702, 0, 0, 1, NULL, 100, 0),
(213880, 29, -5104.6, -1343.58, -50.1429, 0, 0, 1, NULL, 100, 0),
(213880, 30, -5101.29, -1364.32, -50.9869, 0, 0, 1, NULL, 100, 0),
(213880, 31, -5092.99, -1385.64, -52.0711, 0, 0, 1, NULL, 100, 0),
(213880, 32, -5081.42, -1403.17, -52.3669, 0, 0, 1, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (21388, 213880, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 20720 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1020597 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1020597 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1020597 AND spawnType = 0 WHERE ce.spawnID = 1020597;
INSERT INTO creature_entry (spawnID, entry) VALUES (20720, 5841);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (20720, 1, 1, 9533, 1, -1201.63, -3099.24,  94.8781, 3.0143, 43200, 0, 0, 1158, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(207200, 1, -1201.63, -3099.24, 94.8781, 0, 0, 0, NULL, 100, 0),
(207200, 2, -1219.21, -3099.76, 95.241, 0, 0, 0, NULL, 100, 0),
(207200, 3, -1272.76, -3089.53, 93.8928, 0, 0, 0, NULL, 100, 0),
(207200, 4, -1315.08, -3106.5, 91.7995, 0, 0, 0, NULL, 100, 0),
(207200, 5, -1330.48, -3095.55, 92.4438, 0, 0, 0, NULL, 100, 0),
(207200, 6, -1341.49, -3060.61, 92.6899, 0, 0, 0, NULL, 100, 0),
(207200, 7, -1354.4, -3030.3, 93.3309, 0, 0, 0, NULL, 100, 0),
(207200, 8, -1386.31, -3005.86, 93.1475, 0, 0, 0, NULL, 100, 0),
(207200, 9, -1430.19, -2974.08, 93.1218, 0, 0, 0, NULL, 100, 0),
(207200, 10, -1436.49, -2943.46, 91.668, 0, 0, 0, NULL, 100, 0),
(207200, 11, -1436.75, -2920.35, 92.5429, 0, 0, 0, NULL, 100, 0),
(207200, 12, -1410.03, -2893.52, 93.1282, 0, 0, 0, NULL, 100, 0),
(207200, 13, -1388.5, -2866.72, 94.5754, 0, 0, 0, NULL, 100, 0),
(207200, 14, -1360.27, -2850.79, 94.705, 0, 0, 0, NULL, 100, 0),
(207200, 15, -1332.8, -2857.77, 93.5965, 0, 0, 0, NULL, 100, 0),
(207200, 16, -1288.33, -2870.37, 93.0108, 0, 0, 0, NULL, 100, 0),
(207200, 17, -1265.32, -2850.79, 94.069, 0, 0, 0, NULL, 100, 0),
(207200, 18, -1239.87, -2834.24, 94.41, 0, 0, 0, NULL, 100, 0),
(207200, 19, -1219.09, -2834.03, 93.9937, 0, 0, 0, NULL, 100, 0),
(207200, 20, -1194.71, -2865.09, 93.5771, 0, 0, 0, NULL, 100, 0),
(207200, 21, -1170.2, -2886.21, 94.3522, 0, 0, 0, NULL, 100, 0),
(207200, 22, -1146.43, -2925.68, 93.1956, 0, 0, 0, NULL, 100, 0),
(207200, 23, -1123.47, -2957.35, 92.8819, 0, 0, 0, NULL, 100, 0),
(207200, 24, -1114.24, -2966.51, 92.4826, 0, 0, 0, NULL, 100, 0),
(207200, 25, -1111.46, -3018.12, 94.8292, 0, 0, 0, NULL, 100, 0),
(207200, 26, -1101.72, -3047.27, 93.4784, 0, 0, 0, NULL, 100, 0),
(207200, 27, -1123.65, -3070.33, 91.8408, 0, 0, 0, NULL, 100, 0),
(207200, 28, -1147.38, -3076.85, 92.3126, 0, 0, 0, NULL, 100, 0),
(207200, 29, -1177.95, -3082.89, 92.8266, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (20720, 207200, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 20588 with import type REPLACE_ALL
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 20575 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -20575 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 20575 AND spawnType = 0 WHERE ce.spawnID = 20575;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 20576 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -20576 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 20576 AND spawnType = 0 WHERE ce.spawnID = 20576;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 20577 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -20577 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 20577 AND spawnType = 0 WHERE ce.spawnID = 20577;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 20578 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -20578 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 20578 AND spawnType = 0 WHERE ce.spawnID = 20578;
DELETE FROM waypoint_data WHERE id = 428;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 20579 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -20579 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 20579 AND spawnType = 0 WHERE ce.spawnID = 20579;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 20580 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -20580 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 20580 AND spawnType = 0 WHERE ce.spawnID = 20580;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 20581 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -20581 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 20581 AND spawnType = 0 WHERE ce.spawnID = 20581;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 20582 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -20582 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 20582 AND spawnType = 0 WHERE ce.spawnID = 20582;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 20583 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -20583 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 20583 AND spawnType = 0 WHERE ce.spawnID = 20583;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 20584 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -20584 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 20584 AND spawnType = 0 WHERE ce.spawnID = 20584;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 20585 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -20585 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 20585 AND spawnType = 0 WHERE ce.spawnID = 20585;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 20586 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -20586 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 20586 AND spawnType = 0 WHERE ce.spawnID = 20586;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 20587 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -20587 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 20587 AND spawnType = 0 WHERE ce.spawnID = 20587;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1020465 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1020465 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1020465 AND spawnType = 0 WHERE ce.spawnID = 1020465;
INSERT INTO creature_entry (spawnID, entry) VALUES (20588, 3275);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (20588, 1, 1, 4874, 1, -1198.8, -3096.75,  94.5592, -3.03911, 275, 0, 0, 328, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (20588, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 20588);
INSERT INTO creature_entry (spawnID, entry) VALUES (20587, 3275);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (20587, 1, 1, 4874, 1, -1286.16, -3084.85,  93.7667, 1.81957, 275, 5, 0, 328, 0, 1, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (20587, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 20587);
INSERT INTO creature_entry (spawnID, entry) VALUES (20586, 3275);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (20586, 1, 1, 4874, 1, -1290.58, -2842.22,  93.5425, 0.894048, 275, 5, 0, 328, 0, 1, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (20586, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 20586);
INSERT INTO creature_entry (spawnID, entry) VALUES (20585, 3275);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (20585, 1, 1, 4874, 1, -1550.39, -2880.64,  91.7917, 3.86662, 275, 5, 0, 328, 0, 1, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (20585, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 20585);
INSERT INTO creature_entry (spawnID, entry) VALUES (20584, 3275);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (20584, 1, 1, 4874, 1, -1246.53, -2746.67,  91.7916, 1.43646, 275, 5, 0, 328, 0, 1, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (20584, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 20584);
INSERT INTO creature_entry (spawnID, entry) VALUES (20583, 3275);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (20583, 1, 1, 4874, 1, -1482.03, -3032.52,  91.7917, 2.08767, 275, 5, 0, 328, 0, 1, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (20583, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 20583);
INSERT INTO creature_entry (spawnID, entry) VALUES (20582, 3275);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (20582, 1, 1, 4874, 1, -1543.73, -2904.48,  91.7917, 1.40305, 275, 5, 0, 328, 0, 1, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (20582, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 20582);
INSERT INTO creature_entry (spawnID, entry) VALUES (20581, 3275);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (20581, 1, 1, 4874, 1, -1352.25, -3052.89,  92.5331, 4.94511, 275, 5, 0, 328, 0, 1, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (20581, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 20581);
INSERT INTO creature_entry (spawnID, entry) VALUES (20580, 3275);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (20580, 1, 1, 4874, 1, -1287.89, -2820.94,  93.1643, 5.7218, 275, 5, 0, 328, 0, 1, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (20580, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 20580);
INSERT INTO creature_entry (spawnID, entry) VALUES (20579, 3275);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (20579, 1, 1, 4874, 1, -1342.26, -3110.99,  91.7917, 2.89803, 275, 0, 0, 328, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (20579, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 20579);
INSERT INTO creature_entry (spawnID, entry) VALUES (20578, 3275);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (20578, 1, 1, 4874, 1, -1220.52, -2777.53,  91.9093, 2.91066, 275, 0, 0, 328, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(205780, 1, -1219.03, -2776.84, 91.9919, 0, 0, 0, NULL, 100, 0),
(205780, 2, -1212.57, -2770.68, 93.7325, 0, 0, 0, NULL, 100, 0),
(205780, 3, -1208.41, -2765.41, 95.6198, 0, 0, 0, NULL, 100, 0),
(205780, 4, -1207.48, -2757.23, 99.1217, 0, 0, 0, NULL, 100, 0),
(205780, 5, -1208.16, -2749.82, 101.693, 0, 0, 0, NULL, 100, 0),
(205780, 6, -1209.29, -2738.67, 102.656, 0, 0, 0, NULL, 100, 0),
(205780, 7, -1210.25, -2729.62, 106.462, 0, 0, 0, NULL, 100, 0),
(205780, 8, -1209.4, -2737.42, 102.615, 0, 0, 0, NULL, 100, 0),
(205780, 9, -1208.88, -2744.85, 102.374, 0, 0, 0, NULL, 100, 0),
(205780, 10, -1208.3, -2751.54, 101.193, 0, 0, 0, NULL, 100, 0),
(205780, 11, -1207.84, -2759.73, 98.0328, 0, 0, 0, NULL, 100, 0),
(205780, 12, -1208.49, -2769.77, 94.3049, 0, 0, 0, NULL, 100, 0),
(205780, 13, -1220.8, -2782.78, 91.6861, 0, 0, 0, NULL, 100, 0),
(205780, 14, -1238.09, -2779.75, 91.6727, 0, 0, 0, NULL, 100, 0),
(205780, 15, -1253.41, -2765.02, 91.7506, 0, 0, 0, NULL, 100, 0),
(205780, 16, -1268.88, -2769.24, 92.4047, 0, 0, 0, NULL, 100, 0),
(205780, 17, -1264.5, -2786.59, 92.9615, 0, 0, 0, NULL, 100, 0),
(205780, 18, -1254.3, -2796.94, 93.5079, 0, 0, 0, NULL, 100, 0),
(205780, 19, -1243.99, -2793.6, 92.7162, 0, 0, 0, NULL, 100, 0),
(205780, 20, -1231.82, -2785.67, 91.7029, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (20578, 205780, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 20578);
INSERT INTO creature_entry (spawnID, entry) VALUES (20577, 3275);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (20577, 1, 1, 4874, 1, -1237.82, -2084.81,  91.5492, 0.041671, 275, 0, 0, 328, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (20577, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 20577);
INSERT INTO creature_entry (spawnID, entry) VALUES (20576, 3275);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (20576, 1, 1, 4874, 1, -1401.19, -2756.57,  91.7917, 0.4748, 275, 0, 0, 328, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (20576, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 20576);
INSERT INTO creature_entry (spawnID, entry) VALUES (20575, 3275);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (20575, 1, 1, 4874, 1, -1159.23, -2948.66,  93.7825, 3.54595, 275, 5, 0, 328, 0, 1, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (20575, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 20575);

-- Importing creature with guid 20587 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 20586 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 20585 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 20584 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 20583 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 20582 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 20581 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 20580 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 20579 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 20578 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 20577 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 20576 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 20575 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 20429 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1020307 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1020307 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1020307 AND spawnType = 0 WHERE ce.spawnID = 1020307;
INSERT INTO creature_entry (spawnID, entry) VALUES (20429, 5830);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (20429, 1, 1, 10876, 0, 578.54, -1335.66,  92.8374, 4.92376, 43200, 0, 0, 1212, 1368, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(204290, 1, 591.098, -1326.51, 92.2916, 0, 0, 0, NULL, 100, 0),
(204290, 2, 605.883, -1321.84, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 3, 624.948, -1316.58, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 4, 643.687, -1307.64, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 5, 668.631, -1300.92, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 6, 692.319, -1303.29, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 7, 718.773, -1311.1, 91.8569, 0, 0, 0, NULL, 100, 0),
(204290, 8, 742.36, -1321.82, 92.6459, 0, 0, 0, NULL, 100, 0),
(204290, 9, 754.919, -1329.23, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 10, 761.752, -1348.34, 91.7288, 0, 0, 0, NULL, 100, 0),
(204290, 11, 758.361, -1361.86, 91.7288, 0, 0, 0, NULL, 100, 0),
(204290, 12, 746.012, -1375.54, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 13, 722.187, -1385.72, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 14, 699.241, -1382.39, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 15, 686.015, -1369.51, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 16, 675.795, -1361.89, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 17, 665.328, -1355.35, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 18, 652.188, -1344.57, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 19, 636.429, -1334.76, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 20, 628.536, -1323.14, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 21, 623.619, -1314.03, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 22, 615.407, -1298.6, 91.7917, 0, 0, 0, NULL, 100, 0),
(204290, 23, 604.114, -1284.03, 91.7917, 0, 0, 0, NULL, 100, 0),
(204290, 24, 587.851, -1269.01, 92.0619, 0, 0, 0, NULL, 100, 0),
(204290, 25, 576.031, -1259.97, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 26, 560.895, -1256.68, 92.0468, 0, 0, 0, NULL, 100, 0),
(204290, 27, 545.482, -1255.18, 91.8714, 0, 0, 0, NULL, 100, 0),
(204290, 28, 534.852, -1253.39, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 29, 526.591, -1260.62, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 30, 510.401, -1285.22, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 31, 510.391, -1306.54, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 32, 515.124, -1321.95, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 33, 521.895, -1336.84, 91.7917, 0, 0, 0, NULL, 100, 0),
(204290, 34, 529.782, -1343.76, 91.7917, 0, 0, 0, NULL, 100, 0),
(204290, 35, 556.792, -1356.99, 91.7917, 0, 0, 0, NULL, 100, 0),
(204290, 36, 567.064, -1368.91, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 37, 574.606, -1382.9, 91.7916, 0, 0, 0, NULL, 100, 0),
(204290, 38, 578.411, -1399.87, 93.3668, 0, 0, 0, NULL, 100, 0),
(204290, 39, 584.315, -1420.1, 91.7917, 0, 0, 0, NULL, 100, 0),
(204290, 40, 588.742, -1440.73, 91.7917, 0, 0, 0, NULL, 100, 0),
(204290, 41, 577.568, -1461.41, 92.6667, 0, 0, 0, NULL, 100, 0),
(204290, 42, 567.409, -1473.39, 92.4488, 0, 0, 0, NULL, 100, 0),
(204290, 43, 555.569, -1487.63, 91.7917, 0, 0, 0, NULL, 100, 0),
(204290, 44, 538.345, -1499.21, 93.1349, 0, 0, 0, NULL, 100, 0),
(204290, 45, 517.216, -1501.07, 94.5103, 0, 0, 0, NULL, 100, 0),
(204290, 46, 492.097, -1496.38, 94.3223, 0, 0, 0, NULL, 100, 0),
(204290, 47, 474.586, -1480.68, 93.079, 0, 0, 0, NULL, 100, 0),
(204290, 48, 464.53, -1462.18, 91.7917, 0, 0, 0, NULL, 100, 0),
(204290, 49, 460.698, -1444.19, 91.7917, 0, 0, 0, NULL, 100, 0),
(204290, 50, 466.94, -1424.12, 91.7917, 0, 0, 0, NULL, 100, 0),
(204290, 51, 470.694, -1413.05, 91.7917, 0, 0, 0, NULL, 100, 0),
(204290, 52, 482.416, -1399.12, 91.7917, 0, 0, 0, NULL, 100, 0),
(204290, 53, 502.072, -1380.88, 91.7917, 0, 0, 0, NULL, 100, 0),
(204290, 54, 527.991, -1362.11, 91.7917, 0, 0, 0, NULL, 100, 0),
(204290, 55, 542.13, -1349.18, 91.7917, 0, 0, 0, NULL, 100, 0),
(204290, 56, 562.347, -1341.45, 91.8798, 0, 0, 0, NULL, 100, 0),
(204290, 57, 578.54, -1335.66, 92.8374, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (20429, 204290, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 15144 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1015033 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1015033 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1015033 AND spawnType = 0 WHERE ce.spawnID = 1015033;
INSERT INTO creature_entry (spawnID, entry) VALUES (15144, 3237);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (15144, 1, 1, 10914, 0, -2891.01, -2150.02,  94.2478, 2.6034, 275, 0, 0, 734, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(151440, 1, -2891.01, -2150.02, 94.2478, 0, 0, 0, NULL, 100, 0),
(151440, 2, -2903.93, -2136.42, 95.657, 0, 0, 0, NULL, 100, 0),
(151440, 3, -2921.7, -2112.77, 95.5313, 0, 0, 0, NULL, 100, 0),
(151440, 4, -2928.01, -2071.33, 96.0166, 0, 0, 0, NULL, 100, 0),
(151440, 5, -2928.91, -2050.84, 95.785, 0, 0, 0, NULL, 100, 0),
(151440, 6, -2926.84, -2041.01, 96.3781, 0, 0, 0, NULL, 100, 0),
(151440, 7, -2923.56, -2034.24, 96.0931, 0, 0, 0, NULL, 100, 0),
(151440, 8, -2915.72, -2026.93, 95.6855, 0, 0, 0, NULL, 100, 0),
(151440, 9, -2905.62, -2021.31, 93.4736, 0, 0, 0, NULL, 100, 0),
(151440, 10, -2896.02, -2017.06, 91.7219, 0, 0, 0, NULL, 100, 0),
(151440, 11, -2881.85, -2010.83, 91.8045, 0, 0, 0, NULL, 100, 0),
(151440, 12, -2872.75, -2008.67, 92.7372, 0, 0, 0, NULL, 100, 0),
(151440, 13, -2860.24, -2009.61, 94.2234, 0, 0, 0, NULL, 100, 0),
(151440, 14, -2850.68, -2007.45, 95.3112, 0, 0, 0, NULL, 100, 0),
(151440, 15, -2837.95, -2001.67, 96.0295, 0, 0, 0, NULL, 100, 0),
(151440, 16, -2798.05, -1973.73, 93.4632, 0, 0, 0, NULL, 100, 0),
(151440, 17, -2775.55, -1955.8, 94.3558, 0, 0, 0, NULL, 100, 0),
(151440, 18, -2770.03, -1954.5, 94.3486, 0, 0, 0, NULL, 100, 0),
(151440, 19, -2737.42, -1955.19, 94.2536, 0, 0, 0, NULL, 100, 0),
(151440, 20, -2710.84, -1959.71, 96.7887, 0, 0, 0, NULL, 100, 0),
(151440, 21, -2667.95, -1973.89, 97.7269, 0, 0, 0, NULL, 100, 0),
(151440, 22, -2649.13, -1983.14, 98.7822, 0, 0, 0, NULL, 100, 0),
(151440, 23, -2640.2, -1988.65, 97.8667, 0, 0, 0, NULL, 100, 0),
(151440, 24, -2613.39, -2005.2, 91.7337, 0, 0, 0, NULL, 100, 0),
(151440, 25, -2554.57, -2041.31, 92.0596, 0, 0, 0, NULL, 100, 0),
(151440, 26, -2525.49, -2064.46, 93.7675, 0, 0, 0, NULL, 100, 0),
(151440, 27, -2520.19, -2075.43, 93.7559, 0, 0, 0, NULL, 100, 0),
(151440, 28, -2518.56, -2091.79, 93.4852, 0, 0, 0, NULL, 100, 0),
(151440, 29, -2521.19, -2119.21, 92.1279, 0, 0, 0, NULL, 100, 0),
(151440, 30, -2523.41, -2133.03, 95.6538, 0, 0, 0, NULL, 100, 0),
(151440, 31, -2523.99, -2157.74, 96.1462, 0, 0, 0, NULL, 100, 0),
(151440, 32, -2513.76, -2189.22, 96.6454, 0, 0, 0, NULL, 100, 0),
(151440, 33, -2511.82, -2202.38, 95.8735, 0, 0, 0, NULL, 100, 0),
(151440, 34, -2511.2, -2208.1, 94.7895, 0, 0, 0, NULL, 100, 0),
(151440, 35, -2512.97, -2220.95, 92.1185, 0, 0, 0, NULL, 100, 0),
(151440, 36, -2517, -2236.07, 92.0664, 0, 0, 0, NULL, 100, 0),
(151440, 37, -2526.55, -2255.15, 94.2318, 0, 0, 0, NULL, 100, 0),
(151440, 38, -2532.38, -2268.06, 95.292, 0, 0, 0, NULL, 100, 0),
(151440, 39, -2541.75, -2277.29, 95.0165, 0, 0, 0, NULL, 100, 0),
(151440, 40, -2564.64, -2288.88, 92.3051, 0, 0, 0, NULL, 100, 0),
(151440, 41, -2598.33, -2299.11, 91.6696, 0, 0, 0, NULL, 100, 0),
(151440, 42, -2623.49, -2294.38, 93.5493, 0, 0, 0, NULL, 100, 0),
(151440, 43, -2652.46, -2282.06, 91.8607, 0, 0, 0, NULL, 100, 0),
(151440, 44, -2707.78, -2258.55, 91.6669, 0, 0, 0, NULL, 100, 0),
(151440, 45, -2712.81, -2253.49, 93.2486, 0, 0, 0, NULL, 100, 0),
(151440, 46, -2719.92, -2249.24, 94.5715, 0, 0, 0, NULL, 100, 0),
(151440, 47, -2722.89, -2247.63, 93.545, 0, 0, 0, NULL, 100, 0),
(151440, 48, -2727.47, -2243.92, 91.6671, 0, 0, 0, NULL, 100, 0),
(151440, 49, -2742.73, -2228.33, 92.8483, 0, 0, 0, NULL, 100, 0),
(151440, 50, -2753.01, -2206.2, 96.3717, 0, 0, 0, NULL, 100, 0),
(151440, 51, -2786.31, -2184.29, 95.7868, 0, 0, 0, NULL, 100, 0),
(151440, 52, -2826.44, -2175.95, 96.4113, 0, 0, 0, NULL, 100, 0),
(151440, 53, -2833.23, -2174.24, 95.3236, 0, 0, 0, NULL, 100, 0),
(151440, 54, -2840.01, -2172.53, 92.9766, 0, 0, 0, NULL, 100, 0),
(151440, 55, -2850.23, -2170.11, 91.6667, 0, 0, 0, NULL, 100, 0),
(151440, 56, -2874.56, -2162.59, 92.2099, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (15144, 151440, 0, 0, 0, 0, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 15144);

-- Importing creature with guid 15142 with import type REPLACE_ALL
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 15137 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -15137 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 15137 AND spawnType = 0 WHERE ce.spawnID = 15137;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 15138 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -15138 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 15138 AND spawnType = 0 WHERE ce.spawnID = 15138;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 15139 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -15139 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 15139 AND spawnType = 0 WHERE ce.spawnID = 15139;
DELETE FROM waypoint_data WHERE id = 151390;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 15140 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -15140 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 15140 AND spawnType = 0 WHERE ce.spawnID = 15140;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1015030 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1015030 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1015030 AND spawnType = 0 WHERE ce.spawnID = 1015030;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1015031 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1015031 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1015031 AND spawnType = 0 WHERE ce.spawnID = 1015031;
INSERT INTO creature_entry (spawnID, entry) VALUES (15142, 3236);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (15142, 1, 1, 1453, 0, -2895.78, -2158.99,  91.6667, 2.60247, 275, 0, 0, 449, 0, 0, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 15142);
INSERT INTO creature_entry (spawnID, entry) VALUES (15141, 3236);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (15141, 1, 1, 1453, 0, -2876.98, -2149.49,  92.9047, 2.37849, 275, 0, 0, 449, 0, 0, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 15141);
INSERT INTO creature_entry (spawnID, entry) VALUES (15140, 3236);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (15140, 1, 1, 1453, 0, -2989.56, -2018.65,  92.6055, 6.22439, 275, 0, 0, 449, 0, 0, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 15140);
INSERT INTO creature_entry (spawnID, entry) VALUES (15139, 3236);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (15139, 1, 1, 1453, 0, -2979.44, -2017.6,  92.7704, 6.22656, 275, 0, 0, 449, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(12949707, 1, -2998.13, -2012.47, 91.7341, 0, 0, 0, NULL, 100, 0),
(12949707, 2, -3048.21, -1999.61, 91.8675, 0, 0, 0, NULL, 100, 0),
(12949707, 3, -3106.65, -2003.4, 98.1789, 0, 0, 0, NULL, 100, 0),
(12949707, 4, -3143.73, -2004.96, 91.6704, 0, 0, 0, NULL, 100, 0),
(12949707, 5, -3267.98, -2026.34, 92.5754, 0, 0, 0, NULL, 100, 0),
(12949707, 6, -3583.22, -1921.61, 94.3068, 0, 13000, 0, NULL, 100, 0),
(12949707, 7, -3645.08, -1956.1, 91.9535, 0, 0, 0, NULL, 100, 0),
(12949707, 8, -3702.84, -2056.81, 93.3177, 0, 0, 0, NULL, 100, 0),
(12949707, 9, -3724.26, -2202.7, 91.7286, 0, 0, 0, NULL, 100, 0),
(12949707, 10, -3696.85, -2312.4, 92.1689, 0, 0, 0, NULL, 100, 0),
(12949707, 11, -3667.83, -2352.59, 91.6668, 0, 0, 0, NULL, 100, 0),
(12949707, 12, -3657.77, -2356.66, 91.6667, 0, 0, 0, NULL, 100, 0),
(12949707, 13, -3648.58, -2356.08, 91.757, 0, 0, 0, NULL, 100, 0),
(12949707, 14, -3639.44, -2353.15, 92.3895, 0, 0, 0, NULL, 100, 0),
(12949707, 15, -3630.63, -2346.35, 92.1355, 0, 0, 0, NULL, 100, 0),
(12949707, 16, -3597.42, -2300.35, 92.6799, 0, 0, 0, NULL, 100, 0),
(12949707, 17, -3563.12, -2268.77, 91.7997, 0, 0, 0, NULL, 100, 0),
(12949707, 18, -3520.45, -2253.33, 92.7574, 0, 0, 0, NULL, 100, 0),
(12949707, 19, -3488.97, -2234.86, 93.676, 0, 0, 0, NULL, 100, 0),
(12949707, 20, -3387.12, -2196.93, 95.1846, 0, 0, 0, NULL, 100, 0),
(12949707, 21, -3292.87, -2185.51, 92.5575, 0, 14000, 0, NULL, 100, 0),
(12949707, 22, -3206.89, -2075.94, 94.7149, 0, 0, 0, NULL, 100, 0),
(12949707, 23, -3154.15, -2065.12, 92.807, 0, 11000, 0, NULL, 100, 0),
(12949707, 24, -3144.05, -2075.1, 91.9836, 0, 0, 0, NULL, 100, 0),
(12949707, 25, -3125.87, -2101.69, 91.6774, 0, 0, 0, NULL, 100, 0),
(12949707, 26, -3105.69, -2123.68, 91.8374, 0, 0, 0, NULL, 100, 0),
(12949707, 27, -3082.92, -2132.46, 91.7132, 0, 0, 0, NULL, 100, 0),
(12949707, 28, -2983.82, -2114.47, 93.6036, 0, 0, 0, NULL, 100, 0),
(12949707, 29, -2974.75, -2087.39, 91.6668, 0, 0, 0, NULL, 100, 0),
(12949707, 30, -2958.35, -2057.43, 96.6284, 0, 0, 0, NULL, 100, 0),
(12949707, 31, -2973.28, -2017.21, 94.3499, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (15139, 12949707, 0, 0, 0, 0, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 15139);
INSERT INTO creature_entry (spawnID, entry) VALUES (15138, 3236);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (15138, 1, 1, 1453, 0, -2985.06, -2010.02,  91.8387, 0.035329, 275, 0, 0, 449, 0, 0, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 15138);
INSERT INTO creature_entry (spawnID, entry) VALUES (15137, 3236);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (15137, 1, 1, 1453, 0, -2969.24, -2018.15,  95.2449, 0.119507, 275, 0, 0, 449, 0, 0, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 15137);

-- Importing creature with guid 15141 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 15140 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 15139 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 15138 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 15137 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 14008 with import type REPLACE_ALL
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 13991 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -13991 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 13991 AND spawnType = 0 WHERE ce.spawnID = 13991;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 13992 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -13992 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 13992 AND spawnType = 0 WHERE ce.spawnID = 13992;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 13993 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -13993 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 13993 AND spawnType = 0 WHERE ce.spawnID = 13993;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 13994 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -13994 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 13994 AND spawnType = 0 WHERE ce.spawnID = 13994;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 13995 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -13995 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 13995 AND spawnType = 0 WHERE ce.spawnID = 13995;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 13996 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -13996 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 13996 AND spawnType = 0 WHERE ce.spawnID = 13996;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 13997 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -13997 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 13997 AND spawnType = 0 WHERE ce.spawnID = 13997;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 13998 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -13998 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 13998 AND spawnType = 0 WHERE ce.spawnID = 13998;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 13999 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -13999 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 13999 AND spawnType = 0 WHERE ce.spawnID = 13999;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 14000 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -14000 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 14000 AND spawnType = 0 WHERE ce.spawnID = 14000;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 14001 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -14001 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 14001 AND spawnType = 0 WHERE ce.spawnID = 14001;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 14002 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -14002 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 14002 AND spawnType = 0 WHERE ce.spawnID = 14002;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 14003 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -14003 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 14003 AND spawnType = 0 WHERE ce.spawnID = 14003;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 14004 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -14004 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 14004 AND spawnType = 0 WHERE ce.spawnID = 14004;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 14005 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -14005 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 14005 AND spawnType = 0 WHERE ce.spawnID = 14005;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 14006 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -14006 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 14006 AND spawnType = 0 WHERE ce.spawnID = 14006;
DELETE FROM waypoint_data WHERE id = 264;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 14008 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -14008 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 14008 AND spawnType = 0 WHERE ce.spawnID = 14008;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1013945 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1013945 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1013945 AND spawnType = 0 WHERE ce.spawnID = 1013945;
INSERT INTO creature_entry (spawnID, entry) VALUES (14008, 3397);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14008, 1, 1, 9447, 1, -1250.49, -3090.42,  92.84, 0.129407, 275, 5, 0, 235, 295, 1, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (14008, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 14008);
INSERT INTO creature_entry (spawnID, entry) VALUES (14007, 3397);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14007, 1, 1, 9447, 1, -1198.92, -3101.53,  94.8262, 3.0509, 275, 0, 0, 235, 295, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (14007, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 14007);
INSERT INTO creature_entry (spawnID, entry) VALUES (14006, 3397);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14006, 1, 1, 9447, 1, -1150.59, -2887.54,  94.5679, 2.98672, 275, 0, 0, 235, 295, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(140060, 1, -1152.7, -2888.18, 94.2872, 0, 0, 0, NULL, 100, 0),
(140060, 2, -1160.62, -2878.49, 93.8075, 0, 0, 0, NULL, 100, 0),
(140060, 3, -1166.54, -2865.2, 93.1972, 0, 0, 0, NULL, 100, 0),
(140060, 4, -1172.8, -2848.43, 94.1121, 0, 0, 0, NULL, 100, 0),
(140060, 5, -1183.19, -2834.33, 94.465, 0, 0, 0, NULL, 100, 0),
(140060, 6, -1194.96, -2825.74, 94.4492, 0, 0, 0, NULL, 100, 0),
(140060, 7, -1210.38, -2820.24, 94.2429, 0, 0, 0, NULL, 100, 0),
(140060, 8, -1215.94, -2833.65, 94.0845, 0, 0, 0, NULL, 100, 0),
(140060, 9, -1211.19, -2846.81, 93.8226, 0, 0, 0, NULL, 100, 0),
(140060, 10, -1202.4, -2861.39, 93.4079, 0, 0, 0, NULL, 100, 0),
(140060, 11, -1191.63, -2870.3, 93.2234, 0, 0, 0, NULL, 100, 0),
(140060, 12, -1178.12, -2881.23, 93.9462, 0, 0, 0, NULL, 100, 0),
(140060, 13, -1160.47, -2902.93, 93.3969, 0, 0, 0, NULL, 100, 0),
(140060, 14, -1153.32, -2914.97, 93.334, 0, 0, 0, NULL, 100, 0),
(140060, 15, -1140.44, -2935.23, 93.1391, 0, 0, 0, NULL, 100, 0),
(140060, 16, -1123.51, -2945.79, 93.349, 0, 0, 0, NULL, 100, 0),
(140060, 17, -1106.8, -2939.64, 92.3155, 0, 0, 0, NULL, 100, 0),
(140060, 18, -1102, -2925.07, 92.5217, 0, 0, 0, NULL, 100, 0),
(140060, 19, -1106.93, -2917.29, 92.25, 0, 0, 0, NULL, 100, 0),
(140060, 20, -1129.37, -2905.49, 93.9688, 0, 0, 0, NULL, 100, 0),
(140060, 21, -1129.37, -2905.49, 93.9688, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (14006, 140060, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 14006);
INSERT INTO creature_entry (spawnID, entry) VALUES (14005, 3397);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14005, 1, 1, 9447, 1, -1334.89, -3135.21,  91.7917, 0.825739, 275, 0, 0, 235, 295, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (14005, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 14005);
INSERT INTO creature_entry (spawnID, entry) VALUES (14004, 3397);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14004, 1, 1, 9447, 1, -1320.35, -3127.96,  91.7917, 2.60621, 275, 0, 0, 235, 295, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (14004, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 14004);
INSERT INTO creature_entry (spawnID, entry) VALUES (14003, 3397);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14003, 1, 1, 9447, 1, -1493.2, -3043.02,  91.7917, 5.10249, 275, 5, 0, 235, 295, 1, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (14003, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 14003);
INSERT INTO creature_entry (spawnID, entry) VALUES (14002, 3397);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14002, 1, 1, 9447, 1, -1160.35, -3168.6,  91.7917, 0.357612, 275, 0, 0, 235, 295, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (14002, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 14002);
INSERT INTO creature_entry (spawnID, entry) VALUES (14001, 3397);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14001, 1, 1, 9447, 1, -1179.61, -2985.06,  93.4407, 4.23902, 275, 5, 0, 235, 295, 1, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (14001, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 14001);
INSERT INTO creature_entry (spawnID, entry) VALUES (14000, 3397);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (14000, 1, 1, 9447, 1, -1284.39, -2915.93,  93.8497, 1.24586, 275, 5, 0, 235, 295, 1, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (14000, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 14000);
INSERT INTO creature_entry (spawnID, entry) VALUES (13999, 3397);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (13999, 1, 1, 9447, 1, -1219.27, -3081.98,  94.3349, 2.04425, 275, 5, 0, 235, 295, 1, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (13999, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 13999);
INSERT INTO creature_entry (spawnID, entry) VALUES (13998, 3397);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (13998, 1, 1, 9447, 1, -1351.32, -2986.78,  92.9249, 3.40368, 275, 5, 0, 235, 295, 1, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (13998, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 13998);
INSERT INTO creature_entry (spawnID, entry) VALUES (13997, 3397);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (13997, 1, 1, 9447, 1, -1417.4, -2738.95,  91.7917, 2.42907, 275, 0, 0, 235, 295, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (13997, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 13997);
INSERT INTO creature_entry (spawnID, entry) VALUES (13996, 3397);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (13996, 1, 1, 9447, 1, -1147.34, -2797.74,  92.0417, 1.04386, 275, 3, 0, 235, 295, 1, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (13996, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 13996);
INSERT INTO creature_entry (spawnID, entry) VALUES (13995, 3397);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (13995, 1, 1, 9447, 1, -1127.29, -2796.91,  92.0417, 0.976338, 275, 0, 0, 235, 295, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (13995, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 13995);
INSERT INTO creature_entry (spawnID, entry) VALUES (13994, 3397);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (13994, 1, 1, 9447, 1, -1394.55, -2734.76,  91.7917, 2.35791, 275, 0, 0, 235, 295, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (13994, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 13994);
INSERT INTO creature_entry (spawnID, entry) VALUES (13993, 3397);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (13993, 1, 1, 9447, 1, -1210.58, -2725.92,  106.76, 5.34415, 275, 0, 0, 235, 295, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (13993, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 13993);
INSERT INTO creature_entry (spawnID, entry) VALUES (13992, 3397);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (13992, 1, 1, 9447, 1, -1168.91, -2043.24,  92.2584, 0.481511, 275, 0, 0, 235, 295, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (13992, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 13992);
INSERT INTO creature_entry (spawnID, entry) VALUES (13991, 3397);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (13991, 1, 1, 9447, 1, -1168.06, -2040.54,  92.2584, 0.481264, 275, 0, 0, 235, 295, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (13991, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 13991);

-- Importing creature with guid 14007 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 14006 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 14005 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 14004 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 14003 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 14002 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 14001 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 14000 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 13999 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 13998 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 13997 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 13996 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 13995 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 13994 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 13993 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 13992 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 13991 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 12166 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1012121 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1012121 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1012121 AND spawnType = 0 WHERE ce.spawnID = 1012121;
INSERT INTO creature_entry (spawnID, entry) VALUES (12166, 3455);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (12166, 1, 1, 0, 1, -2093.53, -3496.47,  130.084, 3.008, 180, 0, 0, 449, 0, 0, 0, 0, 10);
INSERT INTO spawn_group VALUES (2, 0, 12166);

-- Importing creature with guid 51892 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1051585 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1051585 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1051585 AND spawnType = 0 WHERE ce.spawnID = 1051585;
INSERT INTO creature_entry (spawnID, entry) VALUES (51892, 10200);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (51892, 1, 1, 0, 0, 8021.76, -3946.8,  687.148, 6.22522, 9900, 0, 0, 3758, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(518920, 1, 8028.81, -3947.21, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 2, 8037.27, -3942.65, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 3, 8046.58, -3938.14, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 4, 8052.15, -3930.23, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 5, 8053.36, -3919.33, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 6, 8042.46, -3905.25, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 7, 8036.6, -3897.28, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 8, 8026.45, -3889.37, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 9, 8018.81, -3883.73, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 10, 8010.6, -3876.31, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 11, 8000.86, -3872.12, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 12, 7992.01, -3874.47, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 13, 7985.67, -3879.9, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 14, 7980.29, -3887.23, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 15, 7982.96, -3899.09, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 16, 7988.21, -3905.68, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 17, 8000.71, -3905.21, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 18, 8007.1, -3908.7, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 19, 8012.04, -3917.68, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 20, 8013.97, -3929.08, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 21, 8014.29, -3941.99, 687.148, 0, 0, 0, NULL, 100, 0),
(518920, 22, 8018.94, -3946.64, 687.148, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (51892, 518920, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 51890 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1051583 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1051583 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1051583 AND spawnType = 0 WHERE ce.spawnID = 1051583;
INSERT INTO creature_entry (spawnID, entry) VALUES (51890, 10202);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (51890, 1, 1, 0, 0, 5986.52, -5055.29,  795.339, 0.409894, 43200, 0, 0, 8883, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(518900, 1, 6019.5, -5040.97, 784.711, 0, 0, 0, NULL, 100, 0),
(518900, 2, 6064.57, -5021.38, 783.769, 0, 0, 0, NULL, 100, 0),
(518900, 3, 6110.54, -5029.36, 791.502, 0, 0, 0, NULL, 100, 0),
(518900, 4, 6143.76, -5048.5, 788.654, 0, 0, 0, NULL, 100, 0),
(518900, 5, 6182.44, -5064.91, 783.375, 0, 0, 0, NULL, 100, 0),
(518900, 6, 6224.19, -5048.12, 777.578, 0, 0, 0, NULL, 100, 0),
(518900, 7, 6255.66, -5044.65, 774.014, 0, 0, 0, NULL, 100, 0),
(518900, 8, 6295.05, -5045.93, 768.919, 0, 0, 0, NULL, 100, 0),
(518900, 9, 6333.05, -5046.23, 760.895, 0, 0, 0, NULL, 100, 0),
(518900, 10, 6378.59, -5045.98, 751.77, 0, 0, 0, NULL, 100, 0),
(518900, 11, 6408.91, -5042.45, 747.104, 0, 0, 0, NULL, 100, 0),
(518900, 12, 6438.63, -5018.77, 739.467, 0, 0, 0, NULL, 100, 0),
(518900, 13, 6466.92, -4989.89, 733.592, 0, 0, 0, NULL, 100, 0),
(518900, 14, 6484.62, -4964.16, 728.388, 0, 0, 0, NULL, 100, 0),
(518900, 15, 6500.9, -4934.62, 725.975, 0, 0, 0, NULL, 100, 0),
(518900, 16, 6510.16, -4904.85, 721.889, 0, 0, 0, NULL, 100, 0),
(518900, 17, 6535.35, -4877.96, 709.66, 0, 0, 0, NULL, 100, 0),
(518900, 18, 6559.99, -4860.92, 701.898, 0, 0, 0, NULL, 100, 0),
(518900, 19, 6560.69, -4846.94, 702.04, 0, 0, 0, NULL, 100, 0),
(518900, 20, 6551.32, -4819.47, 703.502, 0, 0, 0, NULL, 100, 0),
(518900, 21, 6550.17, -4785.08, 698.355, 0, 0, 0, NULL, 100, 0),
(518900, 22, 6574.48, -4751.12, 696.631, 0, 0, 0, NULL, 100, 0),
(518900, 23, 6583.19, -4721.82, 699.985, 0, 0, 0, NULL, 100, 0),
(518900, 24, 6580.94, -4680.03, 700.68, 0, 0, 0, NULL, 100, 0),
(518900, 25, 6568.84, -4646.38, 694.206, 0, 0, 0, NULL, 100, 0),
(518900, 26, 6559.54, -4618.04, 699.164, 0, 0, 0, NULL, 100, 0),
(518900, 27, 6552.84, -4585.88, 703.999, 0, 0, 0, NULL, 100, 0),
(518900, 28, 6525.88, -4563, 713.348, 0, 0, 0, NULL, 100, 0),
(518900, 29, 6495.58, -4547.87, 716.137, 0, 0, 0, NULL, 100, 0),
(518900, 30, 6458.24, -4526.25, 723.902, 0, 0, 0, NULL, 100, 0),
(518900, 31, 6450.27, -4486.06, 733.411, 0, 0, 0, NULL, 100, 0),
(518900, 32, 6460.92, -4454.01, 729.607, 0, 0, 0, NULL, 100, 0),
(518900, 33, 6449.63, -4421.93, 723.395, 0, 0, 0, NULL, 100, 0),
(518900, 34, 6424.51, -4403.56, 722.54, 0, 0, 0, NULL, 100, 0),
(518900, 35, 6389.61, -4387.15, 731.328, 0, 0, 0, NULL, 100, 0),
(518900, 36, 6364.21, -4363.73, 738.533, 0, 0, 0, NULL, 100, 0),
(518900, 37, 6362.86, -4328.88, 726.908, 0, 0, 0, NULL, 100, 0),
(518900, 38, 6356.48, -4298.46, 720.34, 0, 0, 0, NULL, 100, 0),
(518900, 39, 6338.97, -4285.14, 725.533, 0, 0, 0, NULL, 100, 0),
(518900, 40, 6317.18, -4280.97, 733.071, 0, 0, 0, NULL, 100, 0),
(518900, 41, 6280.01, -4269.13, 735.894, 0, 0, 0, NULL, 100, 0),
(518900, 42, 6258.96, -4251.5, 735.476, 0, 0, 0, NULL, 100, 0),
(518900, 43, 6260.61, -4236.57, 734.086, 0, 0, 0, NULL, 100, 0),
(518900, 44, 6286.39, -4221.5, 724.38, 0, 0, 0, NULL, 100, 0),
(518900, 45, 6327.71, -4218.92, 721.23, 0, 0, 0, NULL, 100, 0),
(518900, 46, 6365.36, -4205.96, 711.091, 0, 0, 0, NULL, 100, 0),
(518900, 47, 6380.78, -4186.57, 705.823, 0, 0, 0, NULL, 100, 0),
(518900, 48, 6365.36, -4205.96, 711.091, 0, 0, 0, NULL, 100, 0),
(518900, 49, 6327.71, -4218.92, 721.23, 0, 0, 0, NULL, 100, 0),
(518900, 50, 6286.39, -4221.5, 724.38, 0, 0, 0, NULL, 100, 0),
(518900, 51, 6260.61, -4236.57, 734.086, 0, 0, 0, NULL, 100, 0),
(518900, 52, 6258.96, -4251.5, 735.476, 0, 0, 0, NULL, 100, 0),
(518900, 53, 6280.01, -4269.13, 735.894, 0, 0, 0, NULL, 100, 0),
(518900, 54, 6317.18, -4280.97, 733.071, 0, 0, 0, NULL, 100, 0),
(518900, 55, 6338.89, -4285.09, 725.661, 0, 0, 0, NULL, 100, 0),
(518900, 56, 6356.4, -4298.41, 720.158, 0, 0, 0, NULL, 100, 0),
(518900, 57, 6362.86, -4328.88, 726.908, 0, 0, 0, NULL, 100, 0),
(518900, 58, 6364.21, -4363.73, 738.533, 0, 0, 0, NULL, 100, 0),
(518900, 59, 6389.61, -4387.15, 731.328, 0, 0, 0, NULL, 100, 0),
(518900, 60, 6424.51, -4403.56, 722.54, 0, 0, 0, NULL, 100, 0),
(518900, 61, 6449.63, -4421.93, 723.395, 0, 0, 0, NULL, 100, 0),
(518900, 62, 6460.92, -4454.01, 729.607, 0, 0, 0, NULL, 100, 0),
(518900, 63, 6450.27, -4486.06, 733.411, 0, 0, 0, NULL, 100, 0),
(518900, 64, 6458.24, -4526.25, 723.902, 0, 0, 0, NULL, 100, 0),
(518900, 65, 6495.58, -4547.87, 716.137, 0, 0, 0, NULL, 100, 0),
(518900, 66, 6525.88, -4563, 713.348, 0, 0, 0, NULL, 100, 0),
(518900, 67, 6552.84, -4585.88, 703.999, 0, 0, 0, NULL, 100, 0),
(518900, 68, 6559.54, -4618.04, 699.164, 0, 0, 0, NULL, 100, 0),
(518900, 69, 6568.84, -4646.38, 694.206, 0, 0, 0, NULL, 100, 0),
(518900, 70, 6580.94, -4680.03, 700.68, 0, 0, 0, NULL, 100, 0),
(518900, 71, 6583.19, -4721.82, 699.985, 0, 0, 0, NULL, 100, 0),
(518900, 72, 6574.48, -4751.12, 696.631, 0, 0, 0, NULL, 100, 0),
(518900, 73, 6550.17, -4785.08, 698.355, 0, 0, 0, NULL, 100, 0),
(518900, 74, 6551.27, -4819.07, 703.502, 0, 0, 0, NULL, 100, 0),
(518900, 75, 6560.69, -4846.88, 702.222, 0, 0, 0, NULL, 100, 0),
(518900, 76, 6560, -4860.85, 701.915, 0, 0, 0, NULL, 100, 0),
(518900, 77, 6535.35, -4877.96, 709.66, 0, 0, 0, NULL, 100, 0),
(518900, 78, 6510.16, -4904.85, 721.889, 0, 0, 0, NULL, 100, 0),
(518900, 79, 6500.9, -4934.62, 725.975, 0, 0, 0, NULL, 100, 0),
(518900, 80, 6484.62, -4964.16, 728.388, 0, 0, 0, NULL, 100, 0),
(518900, 81, 6466.92, -4989.89, 733.592, 0, 0, 0, NULL, 100, 0),
(518900, 82, 6438.63, -5018.77, 739.511, 0, 0, 0, NULL, 100, 0),
(518900, 83, 6408.91, -5042.45, 747.104, 0, 0, 0, NULL, 100, 0),
(518900, 84, 6378.59, -5045.98, 751.77, 0, 0, 0, NULL, 100, 0),
(518900, 85, 6333.25, -5046.22, 760.899, 0, 0, 0, NULL, 100, 0),
(518900, 86, 6295.05, -5045.93, 768.919, 0, 0, 0, NULL, 100, 0),
(518900, 87, 6255.66, -5044.65, 774.014, 0, 0, 0, NULL, 100, 0),
(518900, 88, 6224.25, -5048.05, 777.68, 0, 0, 0, NULL, 100, 0),
(518900, 89, 6182.59, -5064.94, 783.429, 0, 0, 0, NULL, 100, 0),
(518900, 90, 6143.76, -5048.5, 788.654, 0, 0, 0, NULL, 100, 0),
(518900, 91, 6110.54, -5029.36, 791.502, 0, 0, 0, NULL, 100, 0),
(518900, 92, 6064.57, -5021.38, 783.769, 0, 0, 0, NULL, 100, 0),
(518900, 93, 6019.5, -5040.97, 784.711, 0, 0, 0, NULL, 100, 0),
(518900, 94, 5977.62, -5059.16, 798.207, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (51890, 518900, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 51883 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1051576 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1051576 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1051576 AND spawnType = 0 WHERE ce.spawnID = 1051576;
INSERT INTO creature_entry (spawnID, entry) VALUES (51883, 3773);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (51883, 1, 1, 0, 1, 2230.63, 222.503,  134.671, 2.16137, 9900, 0, 1, 684, 693, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(518830, 1, 2230.63, 222.503, 134.671, 0, 0, 0, NULL, 100, 0),
(518830, 2, 2224.84, 204.382, 131.746, 0, 0, 0, NULL, 100, 0),
(518830, 3, 2224.4, 186.195, 128.543, 0, 0, 0, NULL, 100, 0),
(518830, 4, 2224.78, 165.252, 121.905, 0, 0, 0, NULL, 100, 0),
(518830, 5, 2228.54, 144.604, 115.583, 0, 0, 0, NULL, 100, 0),
(518830, 6, 2234.32, 129.033, 110.983, 0, 0, 0, NULL, 100, 0),
(518830, 7, 2247, 104.069, 105.57, 0, 0, 0, NULL, 100, 0),
(518830, 8, 2259.2, 85.3287, 102.284, 0, 0, 0, NULL, 100, 0),
(518830, 9, 2272.22, 73.9449, 99.1084, 0, 0, 0, NULL, 100, 0),
(518830, 10, 2289.47, 65.1545, 96.7528, 0, 0, 0, NULL, 100, 0),
(518830, 11, 2303.84, 63.2344, 95.158, 0, 0, 0, NULL, 100, 0),
(518830, 12, 2324.82, 62.3018, 92.3022, 0, 0, 0, NULL, 100, 0),
(518830, 13, 2315.04, 61.6474, 94.0507, 0, 0, 0, NULL, 100, 0),
(518830, 14, 2294.27, 64.415, 96.8118, 0, 0, 0, NULL, 100, 0),
(518830, 15, 2274.86, 73.5537, 98.9708, 0, 0, 0, NULL, 100, 0),
(518830, 16, 2262.32, 85.8406, 101.911, 0, 0, 0, NULL, 100, 0),
(518830, 17, 2251.13, 95.9071, 104.024, 0, 0, 0, NULL, 100, 0),
(518830, 18, 2236.55, 110.108, 108.169, 0, 0, 0, NULL, 100, 0),
(518830, 19, 2227.55, 124.708, 110.386, 0, 0, 0, NULL, 100, 0),
(518830, 20, 2222.8, 143.517, 116.078, 0, 0, 0, NULL, 100, 0),
(518830, 21, 2219.98, 164.32, 122.125, 0, 0, 0, NULL, 100, 0),
(518830, 22, 2218.41, 184.548, 128.982, 0, 0, 0, NULL, 100, 0),
(518830, 23, 2219.25, 198.519, 131.665, 0, 0, 0, NULL, 100, 0),
(518830, 24, 2220.35, 219.486, 133.996, 0, 0, 0, NULL, 100, 0),
(518830, 25, 2222.08, 244.73, 134.205, 0, 0, 0, NULL, 100, 0),
(518830, 26, 2224.54, 261.256, 134.332, 0, 0, 0, NULL, 100, 0),
(518830, 27, 2229.94, 281.548, 133.522, 0, 0, 0, NULL, 100, 0),
(518830, 28, 2236.28, 301.552, 133.194, 0, 0, 0, NULL, 100, 0),
(518830, 29, 2240.8, 314.802, 133.414, 0, 0, 0, NULL, 100, 0),
(518830, 30, 2246.95, 334.862, 130.499, 0, 0, 0, NULL, 100, 0),
(518830, 31, 2252.28, 347.803, 129.388, 0, 0, 0, NULL, 100, 0),
(518830, 32, 2261.55, 366.643, 129.972, 0, 0, 0, NULL, 100, 0),
(518830, 33, 2270.85, 385.474, 129.803, 0, 0, 0, NULL, 100, 0),
(518830, 34, 2280.35, 399.463, 130.36, 0, 0, 0, NULL, 100, 0),
(518830, 35, 2293.54, 412.131, 130.348, 0, 0, 0, NULL, 100, 0),
(518830, 36, 2304.32, 421.067, 129.61, 0, 0, 0, NULL, 100, 0),
(518830, 37, 2321.47, 433.147, 128.85, 0, 0, 0, NULL, 100, 0),
(518830, 38, 2336.44, 441.356, 123.324, 0, 0, 0, NULL, 100, 0),
(518830, 39, 2355.44, 450.316, 116.676, 0, 0, 0, NULL, 100, 0),
(518830, 40, 2365.59, 455.13, 114.194, 0, 0, 0, NULL, 100, 0),
(518830, 41, 2353.16, 449.219, 117.544, 0, 0, 0, NULL, 100, 0),
(518830, 42, 2340.28, 443.741, 122.071, 0, 0, 0, NULL, 100, 0),
(518830, 43, 2327.48, 438.06, 126.87, 0, 0, 0, NULL, 100, 0),
(518830, 44, 2310.08, 428.21, 128.501, 0, 0, 0, NULL, 100, 0),
(518830, 45, 2299.13, 419.506, 129.95, 0, 0, 0, NULL, 100, 0),
(518830, 46, 2288.76, 410.104, 130.291, 0, 0, 0, NULL, 100, 0),
(518830, 47, 2274.88, 394.388, 129.902, 0, 0, 0, NULL, 100, 0),
(518830, 48, 2267.13, 382.739, 129.732, 0, 0, 0, NULL, 100, 0),
(518830, 49, 2257.43, 364.647, 129.96, 0, 0, 0, NULL, 100, 0),
(518830, 50, 2251.87, 351.801, 129.572, 0, 0, 0, NULL, 100, 0),
(518830, 51, 2244.71, 332.077, 131.14, 0, 0, 0, NULL, 100, 0),
(518830, 52, 2240.67, 318.68, 133.11, 0, 0, 0, NULL, 100, 0),
(518830, 53, 2236.63, 305.277, 133.475, 0, 0, 0, NULL, 100, 0),
(518830, 54, 2231.74, 284.871, 133.537, 0, 0, 0, NULL, 100, 0),
(518830, 55, 2230.47, 271.257, 134.44, 0, 0, 0, NULL, 100, 0),
(518830, 56, 2229.77, 252.587, 134.871, 0, 0, 0, NULL, 100, 0),
(518830, 57, 2228.25, 238.221, 135.417, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (51883, 518830, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 6619 with import type REPLACE_ALL
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 6619 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -6619 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 6619 AND spawnType = 0 WHERE ce.spawnID = 6619;
INSERT INTO creature_entry (spawnID, entry) VALUES (6619, 5195);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (6619, 1, 1, 2328, 0, 2158.74, -4661.51,  49.9269, 2.00713, 300, 0, 0, 198, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (6619, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 6618 with import type REPLACE_ALL
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 6618 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -6618 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 6618 AND spawnType = 0 WHERE ce.spawnID = 6618;
INSERT INTO creature_entry (spawnID, entry) VALUES (6618, 14541);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (6618, 1, 1, 14574, 0, 2155.59, -4649.18,  50.0618, 5.3058, 300, 0, 0, 198, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (6618, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 6617 with import type REPLACE_ALL
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 6617 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -6617 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 6617 AND spawnType = 0 WHERE ce.spawnID = 6617;
INSERT INTO creature_entry (spawnID, entry) VALUES (6617, 14539);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (6617, 1, 1, 14575, 0, 2153.65, -4657.14,  49.9992, 0.484856, 300, 0, 0, 198, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (6617, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 6616 with import type REPLACE_ALL
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 6616 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -6616 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 6616 AND spawnType = 0 WHERE ce.spawnID = 6616;
INSERT INTO creature_entry (spawnID, entry) VALUES (6616, 12351);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (6616, 1, 1, 2327, 0, 2162.76, -4659.62,  49.9817, 2.25148, 300, 0, 0, 198, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (6616, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 6615 with import type REPLACE_ALL
INSERT INTO creature_entry (spawnID, entry) VALUES (6615, 358);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (6615, 1, 1, 247, 0, 2165, -4656.42,  50.5176, 2.79916, 300, 0, 0, 198, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (6615, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 6614 with import type REPLACE_ALL
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 6614 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -6614 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 6614 AND spawnType = 0 WHERE ce.spawnID = 6614;
INSERT INTO creature_entry (spawnID, entry) VALUES (6614, 14540);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (6614, 1, 1, 14573, 0, 2161.67, -4648.08,  50.5901, 4.18879, 300, 0, 0, 198, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (6614, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 6459 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (6459, 3190);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (6459, 1, 1, 4075, 1, 1279.03, -4188.68,  28.0245, 3.52557, 300, 0, 0, 156, 0, 0, 0, 5, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (6459, NULL, 0, 0, 0, 257, 0, NULL);

-- Importing creature with guid 51882 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1051575 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1051575 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1051575 AND spawnType = 0 WHERE ce.spawnID = 1051575;
INSERT INTO creature_entry (spawnID, entry) VALUES (51882, 10641);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (51882, 1, 1, 0, 0, 2971.35, -982.098,  198.683, 1.72311, 9900, 0, 1, 713, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(518820, 1, 2971.35, -982.098, 198.683, 0, 0, 0, NULL, 100, 0),
(518820, 2, 2971.69, -962.484, 198.427, 0, 0, 0, NULL, 100, 0),
(518820, 3, 2968.38, -941.766, 200.709, 0, 0, 0, NULL, 100, 0),
(518820, 4, 2965.15, -928.148, 198.484, 0, 0, 0, NULL, 100, 0),
(518820, 5, 2952.84, -910.757, 197.443, 0, 0, 0, NULL, 100, 0),
(518820, 6, 2938.87, -905.129, 197.1, 0, 0, 0, NULL, 100, 0),
(518820, 7, 2919.63, -903.527, 198.156, 0, 0, 0, NULL, 100, 0),
(518820, 8, 2903.41, -903.434, 197.124, 0, 0, 0, NULL, 100, 0),
(518820, 9, 2885.07, -906.787, 197.56, 0, 0, 0, NULL, 100, 0),
(518820, 10, 2867.5, -918.221, 198.914, 0, 0, 0, NULL, 100, 0),
(518820, 11, 2856.64, -927.045, 199.21, 0, 0, 0, NULL, 100, 0),
(518820, 12, 2840.31, -940.244, 198.58, 0, 0, 0, NULL, 100, 0),
(518820, 13, 2819.92, -952.547, 196.251, 0, 0, 0, NULL, 100, 0),
(518820, 14, 2812.92, -966.39, 195.478, 0, 0, 0, NULL, 100, 0),
(518820, 15, 2813.45, -983.663, 197.86, 0, 0, 0, NULL, 100, 0),
(518820, 16, 2821.78, -1002.94, 197.072, 0, 0, 0, NULL, 100, 0),
(518820, 17, 2830.53, -1018.29, 198.96, 0, 0, 0, NULL, 100, 0),
(518820, 18, 2839.1, -1026.95, 197.752, 0, 0, 0, NULL, 100, 0),
(518820, 19, 2862.39, -1035.76, 197.902, 0, 0, 0, NULL, 100, 0),
(518820, 20, 2876.33, -1037.06, 198.183, 0, 0, 0, NULL, 100, 0),
(518820, 21, 2897.22, -1039.2, 198.248, 0, 0, 0, NULL, 100, 0),
(518820, 22, 2911.11, -1040.89, 197.331, 0, 0, 0, NULL, 100, 0),
(518820, 23, 2934.68, -1045.41, 196.986, 0, 0, 0, NULL, 100, 0),
(518820, 24, 2946.64, -1040.84, 197.255, 0, 0, 0, NULL, 100, 0),
(518820, 25, 2958.37, -1029.79, 197.856, 0, 0, 0, NULL, 100, 0),
(518820, 26, 2968.66, -1011.25, 197.917, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (51882, 518820, 0, 0, 0, 0, 0, NULL);

-- Importing creature with guid 51870 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1051565 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1051565 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1051565 AND spawnType = 0 WHERE ce.spawnID = 1051565;
INSERT INTO creature_entry (spawnID, entry) VALUES (51870, 3792);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (51870, 1, 1, 0, 0, 3150.4, -1170.14,  217.299, 5.14352, 9900, 0, 1, 1107, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(518700, 1, 3150.4, -1170.14, 217.299, 0, 0, 0, NULL, 100, 0),
(518700, 2, 3150.36, -1176.92, 217.213, 0, 0, 0, NULL, 100, 0),
(518700, 3, 3151.55, -1184.7, 216.956, 0, 0, 0, NULL, 100, 0),
(518700, 4, 3154.04, -1195.63, 216.413, 0, 0, 0, NULL, 100, 0),
(518700, 5, 3157.34, -1200.89, 215.839, 0, 0, 0, NULL, 100, 0),
(518700, 6, 3162.61, -1203.64, 215.187, 0, 0, 0, NULL, 100, 0),
(518700, 7, 3167.5, -1202.5, 214.796, 0, 0, 0, NULL, 100, 0),
(518700, 8, 3169.95, -1199.82, 214.695, 0, 0, 0, NULL, 100, 0),
(518700, 9, 3174.42, -1191.44, 214.546, 0, 0, 0, NULL, 100, 0),
(518700, 10, 3176.17, -1184.18, 214.374, 0, 0, 0, NULL, 100, 0),
(518700, 11, 3175.64, -1176.01, 214.257, 0, 0, 0, NULL, 100, 0),
(518700, 12, 3170.87, -1170.89, 214.671, 0, 0, 0, NULL, 100, 0),
(518700, 13, 3165.25, -1166.6, 215.212, 0, 0, 0, NULL, 100, 0),
(518700, 14, 3156.91, -1163.95, 216.298, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (51870, 518700, 0, 0, 0, 0, 0, NULL);

-- Importing creature with guid 51845 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1051544 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1051544 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1051544 AND spawnType = 0 WHERE ce.spawnID = 1051544;
INSERT INTO creature_entry (spawnID, entry) VALUES (51845, 3056);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (51845, 1, 1, 0, 0, -758.331, 132.167,  2.77833, 6.16915, 9900, 0, 0, 247, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(518450, 1, -758.331, 132.167, 2.77833, 0, 0, 0, NULL, 100, 0),
(518450, 2, -779.039, 138.003, -3.39281, 0, 0, 0, NULL, 100, 0),
(518450, 3, -801.73, 140.724, -8.18451, 0, 0, 0, NULL, 100, 0),
(518450, 4, -817.906, 137.464, -10.6107, 0, 0, 0, NULL, 100, 0),
(518450, 5, -830.672, 150.509, -10.049, 0, 0, 0, NULL, 100, 0),
(518450, 6, -824.121, 172.511, -10.2011, 0, 0, 0, NULL, 100, 0),
(518450, 7, -824.765, 186.873, -9.98871, 0, 0, 0, NULL, 100, 0),
(518450, 8, -831.983, 206.749, -9.48999, 0, 0, 0, NULL, 100, 0),
(518450, 9, -844.891, 227.419, -3.09247, 0, 0, 0, NULL, 100, 0),
(518450, 10, -857.643, 244.088, 8.02225, 0, 0, 0, NULL, 100, 0),
(518450, 11, -872.992, 266.185, 19.1681, 0, 0, 0, NULL, 100, 0),
(518450, 12, -882.134, 276.547, 22.0825, 0, 0, 0, NULL, 100, 0),
(518450, 13, -889.801, 296.096, 23.3952, 0, 0, 0, NULL, 100, 0),
(518450, 14, -902.935, 328.538, 23.2227, 0, 0, 0, NULL, 100, 0),
(518450, 15, -911.667, 347.622, 22.7623, 0, 0, 0, NULL, 100, 0),
(518450, 16, -920.617, 366.61, 17.7281, 0, 0, 0, NULL, 100, 0),
(518450, 17, -929.676, 385.545, 15.6721, 0, 0, 0, NULL, 100, 0),
(518450, 18, -939.391, 404.154, 17.81, 0, 0, 0, NULL, 100, 0),
(518450, 19, -952.278, 430.415, 22.445, 0, 0, 0, NULL, 100, 0),
(518450, 20, -962.798, 448.574, 27.8895, 0, 0, 0, NULL, 100, 0),
(518450, 21, -974.157, 466.224, 33.3872, 0, 0, 0, NULL, 100, 0),
(518450, 22, -986.256, 481.866, 36.9113, 0, 0, 0, NULL, 100, 0),
(518450, 23, -1009.33, 498.011, 39.3174, 0, 0, 0, NULL, 100, 0),
(518450, 24, -1028.78, 505.895, 38.9738, 0, 0, 0, NULL, 100, 0),
(518450, 25, -1050.18, 512.74, 37.6667, 0, 0, 0, NULL, 100, 0),
(518450, 26, -1067.73, 514.1, 36.464, 0, 0, 0, NULL, 100, 0),
(518450, 27, -1088.43, 510.596, 33.6737, 0, 0, 0, NULL, 100, 0),
(518450, 28, -1109.12, 506.993, 31.1725, 0, 0, 0, NULL, 100, 0),
(518450, 29, -1136.63, 501.823, 24.1533, 0, 0, 0, NULL, 100, 0),
(518450, 30, -1156.81, 493.641, 17.2383, 0, 0, 0, NULL, 100, 0),
(518450, 31, -1171.16, 480.67, 11.4534, 0, 0, 0, NULL, 100, 0),
(518450, 32, -1185.96, 465.781, 6.86526, 0, 0, 0, NULL, 100, 0),
(518450, 33, -1207.32, 447.693, 5.38923, 0, 0, 0, NULL, 100, 0),
(518450, 34, -1218, 433.971, 4.80097, 0, 0, 0, NULL, 100, 0),
(518450, 35, -1233.59, 412.767, 3.49479, 0, 0, 0, NULL, 100, 0),
(518450, 36, -1231.27, 384.682, -1.09937, 0, 0, 0, NULL, 100, 0),
(518450, 37, -1215.84, 369.726, -1.82196, 0, 0, 0, NULL, 100, 0),
(518450, 38, -1195.58, 360.879, 4.17312, 0, 0, 0, NULL, 100, 0),
(518450, 39, -1176.72, 351.738, 10.6938, 0, 0, 0, NULL, 100, 0),
(518450, 40, -1156.65, 345.599, 15.7258, 0, 0, 0, NULL, 100, 0),
(518450, 41, -1130.64, 342.17, 22.5894, 0, 0, 0, NULL, 100, 0),
(518450, 42, -1102.56, 342.3, 31.5151, 0, 0, 0, NULL, 100, 0),
(518450, 43, -1080.75, 344.271, 37.1855, 0, 0, 0, NULL, 100, 0),
(518450, 44, -1064.6, 353.161, 35.4317, 0, 0, 0, NULL, 100, 0),
(518450, 45, -1040.97, 368.182, 32.0226, 0, 0, 0, NULL, 100, 0),
(518450, 46, -1023.56, 379.915, 27.8226, 0, 0, 0, NULL, 100, 0),
(518450, 47, -1006.35, 391.956, 24.1773, 0, 0, 0, NULL, 100, 0),
(518450, 48, -988.706, 402.576, 20.7179, 0, 0, 0, NULL, 100, 0),
(518450, 49, -969.884, 398.737, 17.4093, 0, 0, 0, NULL, 100, 0),
(518450, 50, -954.82, 383.869, 18.1765, 0, 0, 0, NULL, 100, 0),
(518450, 51, -940.533, 364.391, 19.0925, 0, 0, 0, NULL, 100, 0),
(518450, 52, -930.116, 346.158, 23.0264, 0, 0, 0, NULL, 100, 0),
(518450, 53, -914.62, 322.843, 27.8011, 0, 0, 0, NULL, 100, 0),
(518450, 54, -907.517, 307.24, 28.4644, 0, 0, 0, NULL, 100, 0),
(518450, 55, -899.405, 286.196, 28.6649, 0, 0, 0, NULL, 100, 0),
(518450, 56, -883.98, 262.895, 23.5318, 0, 0, 0, NULL, 100, 0),
(518450, 57, -875.789, 248.851, 16.681, 0, 0, 0, NULL, 100, 0),
(518450, 58, -872.647, 221.03, 3.03563, 0, 0, 0, NULL, 100, 0),
(518450, 59, -869.937, 200.206, -4.58467, 0, 0, 0, NULL, 100, 0),
(518450, 60, -865.188, 175.769, -11.7553, 0, 0, 0, NULL, 100, 0),
(518450, 61, -856.971, 158.444, -10.8236, 0, 0, 0, NULL, 100, 0),
(518450, 62, -840.617, 142.701, -10.2134, 0, 0, 0, NULL, 100, 0),
(518450, 63, -819.318, 131.755, -10.5982, 0, 0, 0, NULL, 100, 0),
(518450, 64, -794.494, 124.538, -8.37794, 0, 0, 0, NULL, 100, 0),
(518450, 65, -769.619, 126.573, -4.2534, 0, 0, 0, NULL, 100, 0),
(518450, 66, -751.691, 122.151, 0.485237, 0, 0, 0, NULL, 100, 0),
(518450, 67, -727.577, 116.422, 5.93297, 0, 0, 0, NULL, 100, 0),
(518450, 68, -701.034, 114.986, 7.78627, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (51845, 518450, 0, 0, 0, 0, 0, NULL);

-- Importing creature with guid 51842 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1051541 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1051541 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1051541 AND spawnType = 0 WHERE ce.spawnID = 1051541;
INSERT INTO creature_entry (spawnID, entry) VALUES (51842, 5354);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (51842, 1, 1, 0, 0, -5022.19, 629.697,  33.8359, 4.09332, 9900, 0, 1, 2138, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(518420, 1, -5022.19, 629.697, 33.8359, 0, 0, 0, NULL, 100, 0),
(518420, 2, -5043.06, 619.918, 31.7509, 0, 0, 0, NULL, 100, 0),
(518420, 3, -5054.03, 613.627, 30.7986, 0, 0, 0, NULL, 100, 0),
(518420, 4, -5071.43, 601.872, 32.0576, 0, 0, 0, NULL, 100, 0),
(518420, 5, -5089.62, 591.379, 28.9511, 0, 0, 0, NULL, 100, 0),
(518420, 6, -5105.09, 579.888, 30.5049, 0, 0, 0, NULL, 100, 0),
(518420, 7, -5116.35, 576.741, 32.7647, 0, 0, 0, NULL, 100, 0),
(518420, 8, -5136.21, 566.893, 31.5779, 0, 0, 0, NULL, 100, 0),
(518420, 9, -5150.28, 553.108, 30.9446, 0, 0, 0, NULL, 100, 0),
(518420, 10, -5158.92, 542.096, 35.8013, 0, 0, 0, NULL, 100, 0),
(518420, 11, -5167.16, 522.274, 38.6004, 0, 0, 0, NULL, 100, 0),
(518420, 12, -5166.32, 510.923, 38.5869, 0, 0, 0, NULL, 100, 0),
(518420, 13, -5158.1, 492.401, 33.9916, 0, 0, 0, NULL, 100, 0),
(518420, 14, -5147.33, 474.399, 30.2359, 0, 0, 0, NULL, 100, 0),
(518420, 15, -5134.92, 457.462, 26.0815, 0, 0, 0, NULL, 100, 0),
(518420, 16, -5122.42, 440.587, 21.5833, 0, 0, 0, NULL, 100, 0),
(518420, 17, -5109.22, 424.297, 16.5536, 0, 0, 0, NULL, 100, 0),
(518420, 18, -5094.23, 409.595, 11.2969, 0, 0, 0, NULL, 100, 0),
(518420, 19, -5080.25, 393.952, 10.3456, 0, 0, 0, NULL, 100, 0),
(518420, 20, -5070.76, 379.526, 13.7113, 0, 0, 0, NULL, 100, 0),
(518420, 21, -5063.31, 359.271, 18.1229, 0, 0, 0, NULL, 100, 0),
(518420, 22, -5063.43, 338.488, 24.4114, 0, 0, 0, NULL, 100, 0),
(518420, 23, -5059.11, 312.621, 24.1474, 0, 0, 0, NULL, 100, 0),
(518420, 24, -5054.88, 292.803, 29.3839, 0, 0, 0, NULL, 100, 0),
(518420, 25, -5054.42, 273.846, 34.5728, 0, 0, 0, NULL, 100, 0),
(518420, 26, -5070.98, 257.049, 38.0706, 0, 0, 0, NULL, 100, 0),
(518420, 27, -5087.89, 264.232, 40.3637, 0, 0, 0, NULL, 100, 0),
(518420, 28, -5090.11, 281.699, 35.7167, 0, 0, 0, NULL, 100, 0),
(518420, 29, -5081.8, 296.34, 31.364, 0, 0, 0, NULL, 100, 0),
(518420, 30, -5068.38, 313.534, 24.3237, 0, 0, 0, NULL, 100, 0),
(518420, 31, -5063.65, 332.752, 25.1584, 0, 0, 0, NULL, 100, 0),
(518420, 32, -5060.01, 353.435, 20.3948, 0, 0, 0, NULL, 100, 0),
(518420, 33, -5056.83, 373.165, 13.2976, 0, 0, 0, NULL, 100, 0),
(518420, 34, -5051.16, 389.049, 12.1913, 0, 0, 0, NULL, 100, 0),
(518420, 35, -5053.75, 410.56, 11.4389, 0, 0, 0, NULL, 100, 0),
(518420, 36, -5053.37, 435.001, 12.6685, 0, 0, 0, NULL, 100, 0),
(518420, 37, -5047.56, 449.831, 12.9624, 0, 0, 0, NULL, 100, 0),
(518420, 38, -5041.59, 465.988, 9.92711, 0, 0, 0, NULL, 100, 0),
(518420, 39, -5031.07, 492.359, 14.3807, 0, 0, 0, NULL, 100, 0),
(518420, 40, -5017.89, 502.31, 13.9492, 0, 0, 0, NULL, 100, 0),
(518420, 41, -5016.62, 521.595, 14.871, 0, 0, 0, NULL, 100, 0),
(518420, 42, -5014.02, 548.412, 22.152, 0, 0, 0, NULL, 100, 0),
(518420, 43, -5006.89, 566.593, 27.2151, 0, 0, 0, NULL, 100, 0),
(518420, 44, -5005.98, 584.286, 27.2142, 0, 0, 0, NULL, 100, 0),
(518420, 45, -5006.48, 605.232, 30.0563, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (51842, 518420, 0, 0, 0, 0, 0, NULL);

-- Importing creature with guid 51840 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1051539 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1051539 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1051539 AND spawnType = 0 WHERE ce.spawnID = 1051539;
INSERT INTO creature_entry (spawnID, entry) VALUES (51840, 5347);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (51840, 1, 1, 0, 0, -5750.38, 1687.03,  102.643, 0.756947, 9900, 0, 1, 2576, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(518400, 1, -5750.38, 1687.03, 102.643, 0, 0, 0, NULL, 100, 0),
(518400, 2, -5763.85, 1660, 100.207, 0, 0, 0, NULL, 100, 0),
(518400, 3, -5769.64, 1631.26, 97.2781, 0, 0, 0, NULL, 100, 0),
(518400, 4, -5769.97, 1586.22, 95.6392, 0, 0, 0, NULL, 100, 0),
(518400, 5, -5747.22, 1555.38, 91.3163, 0, 0, 0, NULL, 100, 0),
(518400, 6, -5711.87, 1560.67, 92.6679, 0, 0, 0, NULL, 100, 0),
(518400, 7, -5683.44, 1576.4, 93.4776, 0, 0, 0, NULL, 100, 0),
(518400, 8, -5661.57, 1610.62, 95.7846, 0, 0, 0, NULL, 100, 0),
(518400, 9, -5667.14, 1638.34, 99.2109, 0, 0, 0, NULL, 100, 0),
(518400, 10, -5687.9, 1671.34, 102.928, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (51840, 518400, 0, 0, 0, 0, 0, NULL);

-- Importing creature with guid 51838 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1051537 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1051537 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1051537 AND spawnType = 0 WHERE ce.spawnID = 1051537;
INSERT INTO creature_entry (spawnID, entry) VALUES (51838, 14471);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (51838, 1, 1, 0, 0, -7970.9, 1507.22,  -1.55187, 3.46744, 43200, 0, 0, 31440, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(518380, 1, -7970.9, 1507.22, -1.55187, 0, 0, 0, NULL, 100, 0),
(518380, 2, -7943.04, 1493.79, -6.62616, 0, 0, 0, NULL, 100, 0),
(518380, 3, -7918.48, 1468.86, -5.76875, 0, 0, 0, NULL, 100, 0),
(518380, 4, -7894.92, 1446.88, -9.58654, 0, 0, 0, NULL, 100, 0),
(518380, 5, -7887.06, 1414.65, -4.75843, 0, 0, 0, NULL, 100, 0),
(518380, 6, -7879.74, 1375.46, -8.27366, 0, 0, 0, NULL, 100, 0),
(518380, 7, -7863.91, 1348.84, -6.65281, 0, 0, 0, NULL, 100, 0),
(518380, 8, -7839.48, 1331.17, -9.64498, 0, 0, 0, NULL, 100, 0),
(518380, 9, -7799.42, 1305, -5.83755, 0, 0, 0, NULL, 100, 0),
(518380, 10, -7775.25, 1267.37, -4.49419, 0, 0, 0, NULL, 100, 0),
(518380, 11, -7796.92, 1231.14, -4.35764, 0, 0, 0, NULL, 100, 0),
(518380, 12, -7814.78, 1204.89, -0.534356, 0, 0, 0, NULL, 100, 0),
(518380, 13, -7823.56, 1158.29, 1.64553, 0, 0, 0, NULL, 100, 0),
(518380, 14, -7818.53, 1123.67, 0.376355, 0, 0, 0, NULL, 100, 0),
(518380, 15, -7803.92, 1086.93, 4.71193, 0, 0, 0, NULL, 100, 0),
(518380, 16, -7769.98, 1061.28, 1.14028, 0, 0, 0, NULL, 100, 0),
(518380, 17, -7748.98, 1029.69, 2.54678, 0, 0, 0, NULL, 100, 0),
(518380, 18, -7725.58, 994.818, 1.86259, 0, 0, 0, NULL, 100, 0),
(518380, 19, -7700.38, 961.477, -1.84493, 0, 0, 0, NULL, 100, 0),
(518380, 20, -7660.69, 939.871, 0.607142, 0, 0, 0, NULL, 100, 0),
(518380, 21, -7628.97, 925.089, 0.414325, 0, 0, 0, NULL, 100, 0),
(518380, 22, -7594.46, 932.484, 1.85109, 0, 0, 0, NULL, 100, 0),
(518380, 23, -7550.45, 954.034, 0.91145, 0, 0, 0, NULL, 100, 0),
(518380, 24, -7519.02, 969.428, 1.44406, 0, 0, 0, NULL, 100, 0),
(518380, 25, -7476.11, 979.392, 2.04286, 0, 0, 0, NULL, 100, 0),
(518380, 26, -7428.56, 978.903, 3.13144, 0, 0, 0, NULL, 100, 0),
(518380, 27, -7390.67, 1013.55, 3.49037, 0, 0, 0, NULL, 100, 0),
(518380, 28, -7373.88, 1046.14, 4.74369, 0, 0, 0, NULL, 100, 0),
(518380, 29, -7364.93, 1093.09, 1.32902, 0, 0, 0, NULL, 100, 0),
(518380, 30, -7347.16, 1145.42, 1.2884, 0, 0, 0, NULL, 100, 0),
(518380, 31, -7364.33, 1159.79, 3.23603, 0, 0, 0, NULL, 100, 0),
(518380, 32, -7399.25, 1182.98, 1.84387, 0, 0, 0, NULL, 100, 0),
(518380, 33, -7429.51, 1202.9, 3.02961, 0, 0, 0, NULL, 100, 0),
(518380, 34, -7468.98, 1233.92, 3.40704, 0, 0, 0, NULL, 100, 0),
(518380, 35, -7511.83, 1250.47, 3.33374, 0, 0, 0, NULL, 100, 0),
(518380, 36, -7540.86, 1269.11, 6.09194, 0, 0, 0, NULL, 100, 0),
(518380, 37, -7555.66, 1299.83, 2.85051, 0, 0, 0, NULL, 100, 0),
(518380, 38, -7569.88, 1333.56, 3.0119, 0, 0, 0, NULL, 100, 0),
(518380, 39, -7598.24, 1373.45, 4.0046, 0, 0, 0, NULL, 100, 0),
(518380, 40, -7626.76, 1409.48, 3.85885, 0, 0, 0, NULL, 100, 0),
(518380, 41, -7655.34, 1460.77, 3.68569, 0, 0, 0, NULL, 100, 0),
(518380, 42, -7697.58, 1485.36, 4.42573, 0, 0, 0, NULL, 100, 0),
(518380, 43, -7728.65, 1513.85, -0.668692, 0, 0, 0, NULL, 100, 0),
(518380, 44, -7740.85, 1555.4, 1.30007, 0, 0, 0, NULL, 100, 0),
(518380, 45, -7763.3, 1582.23, -0.216693, 0, 0, 0, NULL, 100, 0),
(518380, 46, -7787.74, 1593.93, 2.87125, 0, 0, 0, NULL, 100, 0),
(518380, 47, -7832.06, 1601.15, 4.1155, 0, 0, 0, NULL, 100, 0),
(518380, 48, -7870.67, 1592.22, 2.56394, 0, 0, 0, NULL, 100, 0),
(518380, 49, -7908.88, 1561.9, -2.73442, 0, 0, 0, NULL, 100, 0),
(518380, 50, -7932.15, 1528.09, -0.895027, 0, 0, 0, NULL, 100, 0),
(518380, 51, -7946.88, 1508.12, -6.62967, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (51838, 518380, 0, 0, 0, 0, 0, NULL);

-- Importing creature with guid 57800 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1057215 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1057215 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1057215 AND spawnType = 0 WHERE ce.spawnID = 1057215;
INSERT INTO creature_entry (spawnID, entry) VALUES (57800, 16797);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (57800, 530, 1, 0, 1, 74.0635, 4333.11,  101.473, 3.12649, 300, 0, 0, 2136, 5751, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(578000, 1, 74.0635, 4333.11, 101.473, 0, 0, 0, NULL, 100, 0),
(578000, 2, 59.6808, 4332.9, 96.1607, 0, 0, 0, NULL, 100, 0),
(578000, 3, 73.1186, 4333.16, 101.055, 0, 0, 0, NULL, 100, 0),
(578000, 4, 90.3435, 4333.23, 101.483, 0, 0, 0, NULL, 100, 0),
(578000, 5, 122.918, 4333.39, 104.867, 0, 0, 0, NULL, 100, 0),
(578000, 6, 154.94, 4333.25, 107.505, 0, 0, 0, NULL, 100, 0),
(578000, 7, 122.918, 4333.39, 104.867, 0, 0, 0, NULL, 100, 0),
(578000, 8, 90.3435, 4333.23, 101.483, 0, 0, 0, NULL, 100, 0),
(578000, 9, 74.0635, 4333.11, 101.473, 0, 0, 0, NULL, 100, 0),
(578000, 10, 59.6808, 4332.9, 96.1607, 0, 0, 0, NULL, 100, 0),
(578000, 11, 73.1186, 4333.16, 101.055, 0, 0, 0, NULL, 100, 0),
(578000, 12, 90.3435, 4333.23, 101.483, 0, 0, 0, NULL, 100, 0),
(578000, 13, 122.918, 4333.39, 104.867, 0, 0, 0, NULL, 100, 0),
(578000, 14, 154.94, 4333.25, 107.505, 0, 0, 0, NULL, 100, 0),
(578000, 15, 192.09, 4333.34, 116.444, 0, 0, 0, NULL, 100, 0),
(578000, 16, 154.94, 4333.25, 107.505, 0, 0, 0, NULL, 100, 0),
(578000, 17, 122.918, 4333.39, 104.867, 0, 0, 0, NULL, 100, 0),
(578000, 18, 90.3435, 4333.23, 101.483, 0, 0, 0, NULL, 100, 0),
(578000, 19, 74.0635, 4333.11, 101.473, 0, 0, 0, NULL, 100, 0),
(578000, 20, 154.94, 4333.25, 107.505, 0, 0, 0, NULL, 100, 0),
(578000, 21, 192.09, 4333.34, 116.444, 0, 0, 0, NULL, 100, 0),
(578000, 22, 122.918, 4333.39, 104.867, 0, 0, 0, NULL, 100, 0),
(578000, 23, 90.3435, 4333.23, 101.483, 0, 0, 0, NULL, 100, 0),
(578000, 24, 74.0635, 4333.11, 101.473, 0, 0, 0, NULL, 100, 0),
(578000, 25, 59.6808, 4332.9, 96.1607, 0, 0, 0, NULL, 100, 0),
(578000, 26, 73.1186, 4333.16, 101.055, 0, 0, 0, NULL, 100, 0),
(578000, 27, 90.3435, 4333.23, 101.483, 0, 0, 0, NULL, 100, 0),
(578000, 28, 122.918, 4333.39, 104.867, 0, 0, 0, NULL, 100, 0),
(578000, 29, 154.94, 4333.25, 107.505, 0, 0, 0, NULL, 100, 0),
(578000, 30, 192.09, 4333.34, 116.444, 0, 0, 0, NULL, 100, 0),
(578000, 31, 90.3435, 4333.23, 101.483, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (57800, 578000, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 58438 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1057851 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1057851 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1057851 AND spawnType = 0 WHERE ce.spawnID = 1057851;
INSERT INTO creature_entry (spawnID, entry) VALUES (58438, 16887);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (58438, 530, 1, 0, 0, -759.964, 2726.03,  170.241, 3.61563, 300, 0, 0, 2061, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(584380, 1, -783.914, 2723.4, 183.657, 0, 0, 0, NULL, 100, 0),
(584380, 2, -786.42, 2723.46, 185.405, 0, 0, 0, NULL, 100, 0),
(584380, 3, -808.463, 2736.65, 181.599, 0, 0, 0, NULL, 100, 0),
(584380, 4, -816.166, 2766.54, 161.21, 0, 0, 0, NULL, 100, 0),
(584380, 5, -804.812, 2783.54, 171.044, 0, 0, 0, NULL, 100, 0),
(584380, 6, -788.845, 2792.33, 180.266, 0, 0, 0, NULL, 100, 0),
(584380, 7, -767.465, 2790.21, 182.102, 0, 0, 0, NULL, 100, 0),
(584380, 8, -758.01, 2776.67, 172.046, 0, 0, 0, NULL, 100, 0),
(584380, 9, -756.492, 2756.31, 160.269, 0, 0, 0, NULL, 100, 0),
(584380, 10, -759.216, 2744.93, 165.824, 0, 0, 0, NULL, 100, 0),
(584380, 11, -779.311, 2724.41, 180.602, 0, 0, 0, NULL, 100, 0),
(584380, 12, -805.504, 2735.04, 170.241, 0, 0, 0, NULL, 100, 0),
(584380, 13, -815.885, 2754.42, 166.657, 0, 0, 0, NULL, 100, 0),
(584380, 14, -817.074, 2763.14, 165.824, 0, 0, 0, NULL, 100, 0),
(584380, 15, -806.469, 2781.89, 164.768, 0, 0, 0, NULL, 100, 0),
(584380, 16, -795.135, 2789.42, 164.991, 0, 0, 0, NULL, 100, 0),
(584380, 17, -788.456, 2793.94, 166.991, 0, 0, 0, NULL, 100, 0),
(584380, 18, -772.857, 2792.4, 170.241, 0, 0, 0, NULL, 100, 0),
(584380, 19, -761.77, 2779.59, 164.463, 0, 0, 0, NULL, 100, 0),
(584380, 20, -754.474, 2756.64, 153.185, 0, 0, 0, NULL, 100, 0),
(584380, 21, -759.964, 2726.03, 170.241, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (58438, 584380, 0, 0, 0, 0, 0, NULL);

-- Importing creature with guid 59482 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1058894 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1058894 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1058894 AND spawnType = 0 WHERE ce.spawnID = 1058894;
INSERT INTO creature_entry (spawnID, entry) VALUES (59482, 17084);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (59482, 530, 1, 0, 1, -1144.66, 4221.21,  53.5334, 4.05735, 300, 0, 0, 5306, 2620, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(594820, 1, -1144.66, 4221.21, 53.5334, 0, 0, 0, NULL, 100, 0),
(594820, 2, -1149.9, 4214.39, 54.4754, 0, 0, 0, NULL, 100, 0),
(594820, 3, -1148.99, 4220.94, 53.6938, 0, 0, 0, NULL, 100, 0),
(594820, 4, -1132.48, 4220.84, 52.6829, 0, 0, 0, NULL, 100, 0),
(594820, 5, -1131.28, 4211.73, 50.5531, 0, 0, 0, NULL, 100, 0),
(594820, 6, -1132.46, 4220.69, 52.6558, 0, 0, 0, NULL, 100, 0),
(594820, 7, -1148.99, 4220.94, 53.6938, 0, 0, 0, NULL, 100, 0),
(594820, 8, -1149.9, 4214.39, 54.4754, 0, 0, 0, NULL, 100, 0),
(594820, 9, -1144.66, 4221.21, 53.5334, 0, 0, 0, NULL, 100, 0),
(594820, 10, -1113.76, 4220.15, 53.6766, 0, 0, 0, NULL, 100, 0),
(594820, 11, -1106.38, 4213.52, 55.1924, 0, 0, 0, NULL, 100, 0),
(594820, 12, -1113.79, 4220.23, 53.684, 0, 0, 0, NULL, 100, 0),
(594820, 13, -1132.25, 4220.94, 52.6656, 0, 0, 0, NULL, 100, 0),
(594820, 14, -1131.51, 4211.61, 50.526, 0, 0, 0, NULL, 100, 0),
(594820, 15, -1132.25, 4220.94, 52.6656, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (59482, 594820, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 59482);

-- Importing creature with guid 63387 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1062734 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1062734 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1062734 AND spawnType = 0 WHERE ce.spawnID = 1062734;
INSERT INTO creature_entry (spawnID, entry) VALUES (63387, 17664);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (63387, 530, 1, 0, 1, -1936.22, -10803.3,  86.3798, 5.1831, 300, 0, 0, 1131, 408, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(633870, 1, -1936.42, -10825.6, 78.7548, 0, 0, 0, NULL, 100, 0),
(633870, 2, -1940.26, -10852.7, 69.5764, 0, 0, 0, NULL, 100, 0),
(633870, 3, -1934.36, -10883.7, 64.2909, 0, 0, 0, NULL, 100, 0),
(633870, 4, -1921.16, -10884.2, 64.6192, 0, 0, 0, NULL, 100, 0),
(633870, 5, -1912.77, -10884.8, 62.5705, 0, 0, 0, NULL, 100, 0),
(633870, 6, -1899.85, -10885.5, 66.1877, 0, 0, 0, NULL, 100, 0),
(633870, 7, -1891.25, -10885.3, 66.968, 0, 9000, 0, NULL, 100, 0),
(633870, 8, -1890.56, -10888.2, 66.968, 0, 0, 0, NULL, 100, 0),
(633870, 9, -1893.75, -10881.2, 66.968, 0, 6000, 0, NULL, 100, 0),
(633870, 10, -1881.7, -10886.8, 66.843, 0, 0, 0, NULL, 100, 0),
(633870, 11, -1907.6, -10877.2, 63.8809, 0, 0, 0, NULL, 100, 0),
(633870, 12, -1929.88, -10899, 63.1309, 0, 0, 0, NULL, 100, 0),
(633870, 13, -1928.44, -10915.9, 62.2858, 0, 5000, 0, NULL, 100, 0),
(633870, 14, -1903.14, -10949.9, 61.3822, 0, 0, 0, NULL, 100, 0),
(633870, 15, -1881.35, -10984.6, 60.8757, 0, 0, 0, NULL, 100, 0),
(633870, 16, -1884.31, -11018.9, 60.1304, 0, 0, 0, NULL, 100, 0),
(633870, 17, -1886.83, -11051.3, 59.4839, 0, 5000, 0, NULL, 100, 0),
(633870, 18, -1862.6, -11079.8, 61.1763, 0, 0, 0, NULL, 100, 0),
(633870, 19, -1831.74, -11092.6, 63.6177, 0, 0, 0, NULL, 100, 0),
(633870, 20, -1810.02, -11123.6, 64.2915, 0, 0, 0, NULL, 100, 0),
(633870, 21, -1790.78, -11161.4, 64.3324, 0, 0, 0, NULL, 100, 0),
(633870, 22, -1775.8, -11189.2, 63.9911, 0, 5000, 0, NULL, 100, 0),
(633870, 23, -1779.32, -11226.9, 61.8887, 0, 0, 0, NULL, 100, 0),
(633870, 24, -1784.68, -11264.7, 59.0894, 0, 0, 0, NULL, 100, 0),
(633870, 25, -1776.88, -11301.8, 57.3209, 0, 0, 0, NULL, 100, 0),
(633870, 26, -1763.04, -11327.5, 56.3828, 0, 5000, 0, NULL, 100, 0),
(633870, 27, -1747.19, -11361.5, 54.6775, 0, 0, 0, NULL, 100, 0),
(633870, 28, -1735.95, -11403.6, 49.3072, 0, 5000, 0, NULL, 100, 0),
(633870, 29, -1738.47, -11406, 48.9322, 0, 9000, 0, NULL, 100, 0),
(633870, 30, -1737.17, -11394.6, 50.7059, 0, 0, 0, NULL, 100, 0),
(633870, 31, -1744.32, -11367.4, 54.3309, 0, 0, 0, NULL, 100, 0),
(633870, 32, -1757.14, -11339.6, 55.9275, 0, 0, 0, NULL, 100, 0),
(633870, 33, -1772.24, -11312.3, 56.9459, 0, 0, 0, NULL, 100, 0),
(633870, 34, -1783.1, -11285.3, 58.1494, 0, 5000, 0, NULL, 100, 0),
(633870, 35, -1786.24, -11251.9, 59.9644, 0, 0, 0, NULL, 100, 0),
(633870, 36, -1778.22, -11222.9, 62.3887, 0, 0, 0, NULL, 100, 0),
(633870, 37, -1778.8, -11190.9, 63.9911, 0, 0, 0, NULL, 100, 0),
(633870, 38, -1794.9, -11152.7, 64.5824, 0, 0, 0, NULL, 100, 0),
(633870, 39, -1812.15, -11115.6, 64.5415, 0, 0, 0, NULL, 100, 0),
(633870, 40, -1834.33, -11092, 63.4263, 0, 5000, 0, NULL, 100, 0),
(633870, 41, -1859.51, -11076.9, 61.3013, 0, 0, 0, NULL, 100, 0),
(633870, 42, -1884.71, -11057.7, 59.6089, 0, 0, 0, NULL, 100, 0),
(633870, 43, -1880.73, -11023.8, 60.1304, 0, 0, 0, NULL, 100, 0),
(633870, 44, -1881.92, -10987.8, 60.7507, 0, 0, 0, NULL, 100, 0),
(633870, 45, -1899.09, -10954.2, 61.3946, 0, 5000, 0, NULL, 100, 0),
(633870, 46, -1917.23, -10930.1, 61.6608, 0, 0, 0, NULL, 100, 0),
(633870, 47, -1936.52, -10893.6, 63.291, 0, 0, 0, NULL, 100, 0),
(633870, 48, -1938.69, -10864.2, 67.2014, 0, 9000, 0, NULL, 100, 0),
(633870, 49, -1941.42, -10849.6, 70.3264, 0, 0, 0, NULL, 100, 0),
(633870, 50, -1934.65, -10841.3, 73.5764, 0, 0, 0, NULL, 100, 0),
(633870, 51, -1935.68, -10822, 79.7548, 0, 0, 0, NULL, 100, 0),
(633870, 52, -1936.22, -10803.3, 86.3798, 0, 12000, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (63387, 633870, 2346, 0, 0, 354, 0, NULL);

-- Importing creature with guid 65501 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1064837 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1064837 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1064837 AND spawnType = 0 WHERE ce.spawnID = 1064837;
DELETE FROM waypoint_data WHERE id = 1425;
INSERT INTO creature_entry (spawnID, entry) VALUES (65501, 18230);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (65501, 530, 1, 0, 0, 9885.4, -7178.18,  31.0353, 2.1744, 120, 0, 0, 1, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(655010, 1, 9869.29, -7166.13, 31.0036, 0, 5, 0, NULL, 100, 0),
(655010, 2, 9862.82, -7177.04, 31.5903, 0, 5, 0, NULL, 100, 0),
(655010, 3, 9895.4, -7182.6, 31.0239, 0, 5, 0, NULL, 100, 0),
(655010, 4, 9914.88, -7200.83, 30.8838, 0, 5, 0, NULL, 100, 0),
(655010, 5, 9897.5, -7200.83, 31.0521, 0, 5, 0, NULL, 100, 0),
(655010, 6, 9883.4, -7172.36, 31.0271, 0, 5, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (65501, 655010, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 65572 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1064903 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1064903 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1064903 AND spawnType = 0 WHERE ce.spawnID = 1064903;
INSERT INTO creature_entry (spawnID, entry) VALUES (65572, 18281);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (65572, 530, 1, 0, 0, -566.173, 5350.9,  17.5357, 2.69483, 300, 0, 0, 15474, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(655720, 1, -599.054, 5366.65, 17.501, 0, 0, 0, NULL, 100, 0),
(655720, 2, -614.275, 5357.16, 17.6243, 0, 0, 0, NULL, 100, 0),
(655720, 3, -625.116, 5342.09, 17.4993, 0, 0, 0, NULL, 100, 0),
(655720, 4, -644.018, 5331.33, 18.5012, 0, 0, 0, NULL, 100, 0),
(655720, 5, -677.398, 5321.68, 17.6211, 0, 0, 0, NULL, 100, 0),
(655720, 6, -685.358, 5303.08, 17.6211, 0, 0, 0, NULL, 100, 0),
(655720, 7, -697.722, 5260.44, 17.8797, 0, 0, 0, NULL, 100, 0),
(655720, 8, -685.358, 5303.08, 17.6211, 0, 0, 0, NULL, 100, 0),
(655720, 9, -677.398, 5321.68, 17.6211, 0, 0, 0, NULL, 100, 0),
(655720, 10, -644.018, 5331.33, 18.5012, 0, 0, 0, NULL, 100, 0),
(655720, 11, -625.116, 5342.09, 17.4993, 0, 0, 0, NULL, 100, 0),
(655720, 12, -614.275, 5357.16, 17.6243, 0, 0, 0, NULL, 100, 0),
(655720, 13, -599.054, 5366.65, 17.501, 0, 0, 0, NULL, 100, 0),
(655720, 14, -566.173, 5350.9, 17.5357, 0, 0, 0, NULL, 100, 0),
(655720, 15, -544.966, 5344.5, 17.7857, 0, 0, 0, NULL, 100, 0),
(655720, 16, -527.839, 5356.74, 18.125, 0, 0, 0, NULL, 100, 0),
(655720, 17, -511.706, 5366.04, 18, 0, 0, 0, NULL, 100, 0),
(655720, 18, -490.089, 5365.22, 17.8798, 0, 0, 0, NULL, 100, 0),
(655720, 19, -511.706, 5366.04, 18, 0, 0, 0, NULL, 100, 0),
(655720, 20, -527.839, 5356.74, 18.125, 0, 0, 0, NULL, 100, 0),
(655720, 21, -544.966, 5344.5, 17.7857, 0, 0, 0, NULL, 100, 0),
(655720, 22, -566.173, 5350.9, 17.5357, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (65572, 655720, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 65572);

-- Importing creature with guid 65808 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1065139 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1065139 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1065139 AND spawnType = 0 WHERE ce.spawnID = 1065139;
DELETE FROM waypoint_data WHERE id = 1442;
INSERT INTO creature_entry (spawnID, entry) VALUES (65808, 18416);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (65808, 530, 1, 0, 0, -2541.65, 7323.38,  6.99469, 3.88646, 300, 0, 0, 4572, 2705, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(658080, 1, -2552.39, 7337.85, 7.42217, 0, 0, 0, NULL, 100, 0),
(658080, 2, -2547.39, 7352.35, 7.04717, 0, 0, 0, NULL, 100, 0),
(658080, 3, -2544.84, 7358.26, 7.04717, 0, 0, 0, NULL, 100, 0),
(658080, 4, -2562.1, 7370.44, 10.0082, 0, 0, 0, NULL, 100, 0),
(658080, 5, -2567.58, 7369.37, 10.2731, 0, 0, 0, NULL, 100, 0),
(658080, 6, -2566.17, 7349.84, 9.92218, 0, 0, 0, NULL, 100, 0),
(658080, 7, -2567.79, 7341.59, 9.87253, 0, 0, 0, NULL, 100, 0),
(658080, 8, -2578, 7329.61, 12.4548, 0, 0, 0, NULL, 100, 0),
(658080, 9, -2585.89, 7320.89, 13.9548, 0, 0, 0, NULL, 100, 0),
(658080, 10, -2605.1, 7337.93, 22.8185, 0, 0, 0, NULL, 100, 0),
(658080, 11, -2617.94, 7333.2, 24.0994, 0, 0, 0, NULL, 100, 0),
(658080, 12, -2619.5, 7314.92, 24.7855, 0, 0, 0, NULL, 100, 0),
(658080, 13, -2620.33, 7306.87, 22.2298, 0, 0, 0, NULL, 100, 0),
(658080, 14, -2622.22, 7295.57, 20.8845, 0, 0, 0, NULL, 100, 0),
(658080, 15, -2634.28, 7283.98, 22.6251, 0, 0, 0, NULL, 100, 0),
(658080, 16, -2645.37, 7281.6, 26.8452, 0, 0, 0, NULL, 100, 0),
(658080, 17, -2658.31, 7277.18, 31.3458, 0, 0, 0, NULL, 100, 0),
(658080, 18, -2656.35, 7260.41, 27.723, 0, 0, 0, NULL, 100, 0),
(658080, 19, -2654.6, 7223.49, 21.6877, 0, 0, 0, NULL, 100, 0),
(658080, 20, -2649.21, 7219.17, 21.3127, 0, 0, 0, NULL, 100, 0),
(658080, 21, -2639.12, 7217.48, 21.1877, 0, 0, 0, NULL, 100, 0),
(658080, 22, -2604.8, 7231.36, 14.2914, 0, 0, 0, NULL, 100, 0),
(658080, 23, -2598.99, 7233.28, 13.0685, 0, 0, 0, NULL, 100, 0),
(658080, 24, -2589.09, 7238.98, 13.305, 0, 0, 0, NULL, 100, 0),
(658080, 25, -2572.71, 7258.2, 14.18, 0, 0, 0, NULL, 100, 0),
(658080, 26, -2553.04, 7268.31, 14.8673, 0, 0, 0, NULL, 100, 0),
(658080, 27, -2549.93, 7292.21, 13.5737, 0, 0, 0, NULL, 100, 0),
(658080, 28, -2543.94, 7296.78, 12.03, 0, 0, 0, NULL, 100, 0),
(658080, 29, -2528.52, 7302.51, 7.70912, 0, 0, 0, NULL, 100, 0),
(658080, 30, -2525.95, 7310.64, 6.42796, 0, 0, 0, NULL, 100, 0),
(658080, 31, -2541.65, 7323.38, 6.99469, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (65808, 658080, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 66605 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1310726 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1310726 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1310726 AND spawnType = 0 WHERE ce.spawnID = 1310726;
DELETE FROM waypoint_data WHERE id = 12949497;
INSERT INTO creature_entry (spawnID, entry) VALUES (66605, 18482);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (66605, 530, 1, 0, 1, -1954.98, 4752.3,  -2.76344, 1.01889, 300, 0, 0, 3316, 2620, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(666050, 1, -1954.98, 4752.3, -2.76344, 0, 0, 0, NULL, 100, 0),
(666050, 2, -1957.74, 4762.07, -2.1277, 0, 0, 0, NULL, 100, 0),
(666050, 3, -1958.93, 4774.6, -1.30567, 0, 0, 0, NULL, 100, 0),
(666050, 4, -1959.28, 4786.45, -0.680666, 0, 0, 0, NULL, 100, 0),
(666050, 5, -1957.06, 4799.9, -0.055666, 0, 0, 0, NULL, 100, 0),
(666050, 6, -1952.97, 4812.15, 0.454229, 0, 0, 0, NULL, 100, 0),
(666050, 7, -1947.46, 4824.4, 0.695318, 0, 0, 0, NULL, 100, 0),
(666050, 8, -1940.22, 4836.16, 1.11686, 0, 0, 0, NULL, 100, 0),
(666050, 9, -1931.71, 4848.86, 1.7353, 0, 0, 0, NULL, 100, 0),
(666050, 10, -1926.99, 4857.38, 1.7353, 0, 0, 0, NULL, 100, 0),
(666050, 11, -1922.21, 4869.19, 1.88885, 0, 0, 0, NULL, 100, 0),
(666050, 12, -1918.47, 4883.84, 2.15545, 0, 0, 0, NULL, 100, 0),
(666050, 13, -1918.33, 4893.27, 2.52044, 0, 0, 0, NULL, 100, 0),
(666050, 14, -1921.36, 4904.43, 4.2868, 0, 0, 0, NULL, 100, 0),
(666050, 15, -1926.93, 4916.87, 7.85235, 0, 0, 0, NULL, 100, 0),
(666050, 16, -1932.68, 4930.55, 12.625, 0, 0, 0, NULL, 100, 0),
(666050, 17, -1936.97, 4938.17, 15.9194, 0, 0, 0, NULL, 100, 0),
(666050, 18, -1943.33, 4944.55, 19.0086, 0, 0, 0, NULL, 100, 0),
(666050, 19, -1949.69, 4955.15, 22.9942, 0, 0, 0, NULL, 100, 0),
(666050, 20, -1957.24, 4971.25, 26.8332, 0, 0, 0, NULL, 100, 0),
(666050, 21, -1959.03, 4989.21, 29.7596, 0, 0, 0, NULL, 100, 0),
(666050, 22, -1959.9, 5001.7, 31.2713, 0, 0, 0, NULL, 100, 0),
(666050, 23, -1958.82, 5016.85, 32.7506, 0, 0, 0, NULL, 100, 0),
(666050, 24, -1955.5, 5027.16, 32.0822, 0, 0, 0, NULL, 100, 0),
(666050, 25, -1949.07, 5037.3, 27.6707, 0, 0, 0, NULL, 100, 0),
(666050, 26, -1940.84, 5049.16, 22.1251, 0, 0, 0, NULL, 100, 0),
(666050, 27, -1933.55, 5052.72, 20.4811, 0, 0, 0, NULL, 100, 0),
(666050, 28, -1932.81, 5055.97, 19.7382, 0, 0, 0, NULL, 100, 0),
(666050, 29, -1935.72, 5059.01, 19.1336, 0, 0, 0, NULL, 100, 0),
(666050, 30, -1938.68, 5057.72, 19.4552, 0, 0, 0, NULL, 100, 0),
(666050, 31, -1940.78, 5053.98, 20.796, 0, 0, 0, NULL, 100, 0),
(666050, 32, -1940.79, 5050.4, 21.7823, 0, 0, 0, NULL, 100, 0),
(666050, 33, -1943.93, 5045.26, 23.8994, 0, 0, 0, NULL, 100, 0),
(666050, 34, -1948.46, 5038.44, 27.0238, 0, 0, 0, NULL, 100, 0),
(666050, 35, -1952.35, 5033.81, 29.4981, 0, 0, 0, NULL, 100, 0),
(666050, 36, -1955.94, 5026.93, 32.1903, 0, 0, 0, NULL, 100, 0),
(666050, 37, -1958.85, 5018.87, 32.8793, 0, 0, 0, NULL, 100, 0),
(666050, 38, -1960.39, 5008.15, 32.1039, 0, 0, 0, NULL, 100, 0),
(666050, 39, -1959.65, 4995.39, 30.6049, 0, 0, 0, NULL, 100, 0),
(666050, 40, -1958.53, 4982.48, 28.7252, 0, 0, 0, NULL, 100, 0),
(666050, 41, -1956.05, 4969.47, 26.4947, 0, 0, 0, NULL, 100, 0),
(666050, 42, -1951.65, 4957.23, 23.6336, 0, 0, 0, NULL, 100, 0),
(666050, 43, -1946.11, 4948.34, 20.8303, 0, 0, 0, NULL, 100, 0),
(666050, 44, -1939.52, 4941.4, 17.3329, 0, 0, 0, NULL, 100, 0),
(666050, 45, -1932.44, 4928.97, 12.0459, 0, 0, 0, NULL, 100, 0),
(666050, 46, -1927.23, 4916.59, 7.91558, 0, 0, 0, NULL, 100, 0),
(666050, 47, -1922.28, 4905.16, 4.57122, 0, 0, 0, NULL, 100, 0),
(666050, 48, -1915.32, 4900.36, 2.12908, 0, 0, 0, NULL, 100, 0),
(666050, 49, -1909.36, 4900.6, 1.0324, 0, 0, 0, NULL, 100, 0),
(666050, 50, -1901.19, 4904.92, -0.753, 0, 0, 0, NULL, 100, 0),
(666050, 51, -1893.88, 4911.84, -3.17073, 0, 0, 0, NULL, 100, 0),
(666050, 52, -1886.2, 4920.9, -6.79732, 0, 0, 0, NULL, 100, 0),
(666050, 53, -1878.95, 4930.25, -11.2615, 0, 0, 0, NULL, 100, 0),
(666050, 54, -1873.74, 4937.32, -14.0713, 0, 0, 0, NULL, 100, 0),
(666050, 55, -1869.68, 4942.98, -16.167, 0, 0, 0, NULL, 100, 0),
(666050, 56, -1866.75, 4948.75, -17.9031, 0, 0, 0, NULL, 100, 0),
(666050, 57, -1864.94, 4956.15, -19.6249, 0, 0, 0, NULL, 100, 0),
(666050, 58, -1864.55, 4965.34, -21.1507, 0, 0, 0, NULL, 100, 0),
(666050, 59, -1865.52, 4969.61, -21.6116, 0, 0, 0, NULL, 100, 0),
(666050, 60, -1867.73, 4970.08, -21.522, 0, 0, 0, NULL, 100, 0),
(666050, 61, -1869.4, 4967.93, -21.2641, 0, 0, 0, NULL, 100, 0),
(666050, 62, -1868.56, 4963.51, -20.6031, 0, 0, 0, NULL, 100, 0),
(666050, 63, -1866.92, 4958.65, -20.0329, 0, 0, 0, NULL, 100, 0),
(666050, 64, -1867.13, 4953.81, -18.9405, 0, 0, 0, NULL, 100, 0),
(666050, 65, -1868.12, 4949.35, -17.8817, 0, 0, 0, NULL, 100, 0),
(666050, 66, -1870.75, 4943.62, -16.1922, 0, 0, 0, NULL, 100, 0),
(666050, 67, -1874.54, 4937.91, -14.2408, 0, 0, 0, NULL, 100, 0),
(666050, 68, -1879.87, 4930.65, -11.2194, 0, 0, 0, NULL, 100, 0),
(666050, 69, -1886.92, 4921.4, -6.74385, 0, 0, 0, NULL, 100, 0),
(666050, 70, -1894.73, 4912.09, -2.99702, 0, 0, 0, NULL, 100, 0),
(666050, 71, -1901.48, 4905.4, -0.835275, 0, 0, 0, NULL, 100, 0),
(666050, 72, -1908.66, 4897.94, 1.18841, 0, 0, 0, NULL, 100, 0),
(666050, 73, -1915.19, 4888.85, 2.12908, 0, 0, 0, NULL, 100, 0),
(666050, 74, -1919.66, 4879.01, 2.12908, 0, 0, 0, NULL, 100, 0),
(666050, 75, -1922.47, 4867.42, 2.00408, 0, 0, 0, NULL, 100, 0),
(666050, 76, -1927.96, 4855.98, 1.7353, 0, 0, 0, NULL, 100, 0),
(666050, 77, -1934.88, 4844.2, 1.50553, 0, 0, 0, NULL, 100, 0),
(666050, 78, -1943.72, 4831.17, 0.770635, 0, 0, 0, NULL, 100, 0),
(666050, 79, -1950.08, 4818.39, 0.579229, 0, 0, 0, NULL, 100, 0),
(666050, 80, -1957.06, 4801.95, 0.113653, 0, 0, 0, NULL, 100, 0),
(666050, 81, -1958.39, 4790.06, -0.430666, 0, 0, 0, NULL, 100, 0),
(666050, 82, -1958.35, 4777.87, -1.18067, 0, 0, 0, NULL, 100, 0),
(666050, 83, -1956.92, 4764.74, -1.93239, 0, 0, 0, NULL, 100, 0),
(666050, 84, -1956.38, 4755.67, -2.52003, 0, 0, 0, NULL, 100, 0),
(666050, 85, -1952.63, 4747.66, -2.88844, 0, 0, 0, NULL, 100, 0),
(666050, 86, -1947.63, 4738.14, -3.12416, 0, 0, 0, NULL, 100, 0),
(666050, 87, -1943.56, 4726.11, -3.1066, 0, 0, 0, NULL, 100, 0),
(666050, 88, -1940.99, 4712.54, -2.7316, 0, 0, 0, NULL, 100, 0),
(666050, 89, -1941.02, 4699.2, -2.2491, 0, 0, 0, NULL, 100, 0),
(666050, 90, -1941.77, 4688.91, -2.1241, 0, 0, 0, NULL, 100, 0),
(666050, 91, -1945.52, 4676.24, -1.86056, 0, 0, 0, NULL, 100, 0),
(666050, 92, -1949.9, 4664.79, -1.32178, 0, 0, 0, NULL, 100, 0),
(666050, 93, -1953.65, 4652.4, -0.922855, 0, 0, 0, NULL, 100, 0),
(666050, 94, -1955.7, 4641.48, -0.19678, 0, 0, 0, NULL, 100, 0),
(666050, 95, -1954.83, 4628.33, 0.672545, 0, 0, 0, NULL, 100, 0),
(666050, 96, -1953.23, 4618.67, 1.83209, 0, 0, 0, NULL, 100, 0),
(666050, 97, -1949.52, 4608.74, 3.14838, 0, 0, 0, NULL, 100, 0),
(666050, 98, -1947.03, 4598.14, 4.95848, 0, 0, 0, NULL, 100, 0),
(666050, 99, -1946.53, 4587.87, 6.83299, 0, 0, 0, NULL, 100, 0),
(666050, 100, -1948.29, 4576.44, 8.56211, 0, 0, 0, NULL, 100, 0),
(666050, 101, -1952.72, 4567.67, 9.98936, 0, 0, 0, NULL, 100, 0),
(666050, 102, -1958.15, 4560.35, 11.1732, 0, 0, 0, NULL, 100, 0),
(666050, 103, -1965.71, 4551.65, 12.3972, 0, 0, 0, NULL, 100, 0),
(666050, 104, -1971.08, 4543.3, 13.1436, 0, 0, 0, NULL, 100, 0),
(666050, 105, -1975.56, 4532.99, 13.4857, 0, 0, 0, NULL, 100, 0),
(666050, 106, -1977.15, 4525.12, 13.4151, 0, 0, 0, NULL, 100, 0),
(666050, 107, -1978.05, 4513.06, 12.8607, 0, 0, 0, NULL, 100, 0),
(666050, 108, -1978.59, 4501.11, 12.1814, 0, 0, 0, NULL, 100, 0),
(666050, 109, -1981.74, 4483.69, 11.3608, 0, 0, 0, NULL, 100, 0),
(666050, 110, -1987.18, 4470.06, 10.3305, 0, 0, 0, NULL, 100, 0),
(666050, 111, -1996.76, 4455, 8.79363, 0, 0, 0, NULL, 100, 0),
(666050, 112, -2004.44, 4442.11, 7.64109, 0, 0, 0, NULL, 100, 0),
(666050, 113, -2007.97, 4429.9, 6.49989, 0, 0, 0, NULL, 100, 0),
(666050, 114, -2013.41, 4410.7, 4.32118, 0, 0, 0, NULL, 100, 0),
(666050, 115, -2023.1, 4394.42, 2.78038, 0, 0, 0, NULL, 100, 0),
(666050, 116, -2032.5, 4383.39, 2.05858, 0, 0, 0, NULL, 100, 0),
(666050, 117, -2043.49, 4372.46, 1.70672, 0, 0, 0, NULL, 100, 0),
(666050, 118, -2051.5, 4359.69, 2.09943, 0, 0, 0, NULL, 100, 0),
(666050, 119, -2054.1, 4350.86, 2.43354, 0, 0, 0, NULL, 100, 0),
(666050, 120, -2056.37, 4340.22, 2.72443, 0, 0, 0, NULL, 100, 0),
(666050, 121, -2056.54, 4330.68, 2.99682, 0, 0, 0, NULL, 100, 0),
(666050, 122, -2056.66, 4324.11, 2.99682, 0, 0, 0, NULL, 100, 0),
(666050, 123, -2056.21, 4332.02, 2.99682, 0, 0, 0, NULL, 100, 0),
(666050, 124, -2056.04, 4340.76, 2.72443, 0, 0, 0, NULL, 100, 0),
(666050, 125, -2054.2, 4351.41, 2.34943, 0, 0, 0, NULL, 100, 0),
(666050, 126, -2051.37, 4359.92, 2.09943, 0, 0, 0, NULL, 100, 0),
(666050, 127, -2045.29, 4369.33, 1.69903, 0, 0, 0, NULL, 100, 0),
(666050, 128, -2036.63, 4378.9, 1.95672, 0, 0, 0, NULL, 100, 0),
(666050, 129, -2028.42, 4387.35, 2.14171, 0, 0, 0, NULL, 100, 0),
(666050, 130, -2020.79, 4397.27, 2.93223, 0, 0, 0, NULL, 100, 0),
(666050, 131, -2014.12, 4409.57, 4.22499, 0, 0, 0, NULL, 100, 0),
(666050, 132, -2010.11, 4421.34, 5.47364, 0, 0, 0, NULL, 100, 0),
(666050, 133, -2006.27, 4435.88, 7.07663, 0, 0, 0, NULL, 100, 0),
(666050, 134, -2000.81, 4450.34, 8.24387, 0, 0, 0, NULL, 100, 0),
(666050, 135, -1991.51, 4462.02, 9.33684, 0, 0, 0, NULL, 100, 0),
(666050, 136, -1984.12, 4478.01, 10.909, 0, 0, 0, NULL, 100, 0),
(666050, 137, -1979.11, 4494.06, 11.9609, 0, 0, 0, NULL, 100, 0),
(666050, 138, -1977.87, 4513.34, 12.8607, 0, 0, 0, NULL, 100, 0),
(666050, 139, -1977.56, 4527.16, 13.4319, 0, 0, 0, NULL, 100, 0),
(666050, 140, -1974.89, 4537.26, 13.5186, 0, 0, 0, NULL, 100, 0),
(666050, 141, -1965.71, 4550.7, 12.5115, 0, 0, 0, NULL, 100, 0),
(666050, 142, -1956.55, 4560.43, 11.1547, 0, 0, 0, NULL, 100, 0),
(666050, 143, -1949.81, 4570.69, 9.50242, 0, 0, 0, NULL, 100, 0),
(666050, 144, -1945.96, 4583.91, 7.29063, 0, 0, 0, NULL, 100, 0),
(666050, 145, -1947.07, 4599.29, 4.71727, 0, 0, 0, NULL, 100, 0),
(666050, 146, -1951.11, 4613.88, 2.40741, 0, 0, 0, NULL, 100, 0),
(666050, 147, -1955.16, 4628.07, 0.703918, 0, 0, 0, NULL, 100, 0),
(666050, 148, -1955.75, 4639.44, -0.117313, 0, 0, 0, NULL, 100, 0),
(666050, 149, -1952.89, 4651.96, -0.86951, 0, 0, 0, NULL, 100, 0),
(666050, 150, -1949.48, 4665.71, -1.32178, 0, 0, 0, NULL, 100, 0),
(666050, 151, -1945.59, 4678.89, -2.18648, 0, 0, 0, NULL, 100, 0),
(666050, 152, -1942.09, 4687.91, -2.1241, 0, 0, 0, NULL, 100, 0),
(666050, 153, -1941.37, 4696.74, -2.2491, 0, 0, 0, NULL, 100, 0),
(666050, 154, -1941.16, 4708.61, -2.7316, 0, 0, 0, NULL, 100, 0),
(666050, 155, -1942.18, 4717.76, -2.85513, 0, 0, 0, NULL, 100, 0),
(666050, 156, -1945.63, 4731.17, -3.2316, 0, 0, 0, NULL, 100, 0),
(666050, 157, -1949.97, 4740.83, -2.9258, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (66605, 666050, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 66606 with import type REPLACE_ALL
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1310727 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1310727 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1310727 AND spawnType = 0 WHERE ce.spawnID = 1310727;
INSERT INTO creature_entry (spawnID, entry) VALUES (66606, 18483);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (66606, 530, 1, 0, 1, -1956.58, 4751.76,  -2.86066, 2.00512, 300, 0, 0, 5341, 0, 0, 0, 0, 10);

-- Importing creature with guid 66701 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1066028 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1066028 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1066028 AND spawnType = 0 WHERE ce.spawnID = 1066028;
INSERT INTO creature_entry (spawnID, entry) VALUES (66701, 18541);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (66701, 530, 1, 0, 0, -2531.76, 5367.72,  27.4804, 1.94111, 300, 0, 0, 5527, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(667010, 1, -2531.76, 5367.72, 27.4804, 0, 0, 0, NULL, 100, 0),
(667010, 2, -2522.24, 5363.87, 27.5043, 0, 0, 0, NULL, 100, 0),
(667010, 3, -2511.57, 5370.14, 27.1955, 0, 0, 0, NULL, 100, 0),
(667010, 4, -2509.84, 5376.43, 27.18, 0, 0, 0, NULL, 100, 0),
(667010, 5, -2514.84, 5388.67, 27.4247, 0, 0, 0, NULL, 100, 0),
(667010, 6, -2525.9, 5389.71, 27.2172, 0, 0, 0, NULL, 100, 0),
(667010, 7, -2531.96, 5386.35, 27.4465, 0, 0, 0, NULL, 100, 0),
(667010, 8, -2535.82, 5377.08, 27.4849, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (66701, 667010, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 66701);

-- Importing creature with guid 68323 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1067566 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1067566 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1067566 AND spawnType = 0 WHERE ce.spawnID = 1067566;
INSERT INTO creature_entry (spawnID, entry) VALUES (68323, 19012);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (68323, 530, 1, 0, 1, -2539.58, 7356.25,  7.04717, 0.821933, 300, 0, 0, 4979, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(683230, 1, -2539.58, 7356.25, 7.04717, 0, 0, 0, NULL, 100, 0),
(683230, 2, -2542.44, 7364.19, 7.07721, 0, 0, 0, NULL, 100, 0),
(683230, 3, -2541.9, 7364.77, 7.01275, 0, 45000, 0, NULL, 100, 0),
(683230, 4, -2539.58, 7356.25, 7.04717, 0, 0, 0, NULL, 100, 0),
(683230, 5, -2533.32, 7352.42, 9.28346, 0, 0, 0, NULL, 100, 0),
(683230, 6, -2530.66, 7354.05, 9.28346, 0, 60000, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (68323, 683230, 0, 0, 0, 257, 233, NULL);

-- Importing creature with guid 68374 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1067617 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1067617 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1067617 AND spawnType = 0 WHERE ce.spawnID = 1067617;
INSERT INTO creature_entry (spawnID, entry) VALUES (68374, 19073);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (68374, 530, 1, 0, 0, -4051.89, -11864.4,  1.78296, 3.62379, 300, 0, 0, 3052, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(683740, 1, -4052.72, -11857.1, 3.03583, 0, 0, 0, NULL, 100, 0),
(683740, 2, -4048.71, -11841.6, 6.74649, 0, 0, 0, NULL, 100, 0),
(683740, 3, -4046.95, -11816.2, 8.90817, 0, 0, 0, NULL, 100, 0),
(683740, 4, -4051.17, -11794.7, 8.94265, 0, 0, 0, NULL, 100, 0),
(683740, 5, -4052.16, -11775.8, 9.00285, 0, 0, 0, NULL, 100, 0),
(683740, 6, -4049.84, -11771.4, 7.90161, 0, 0, 0, NULL, 100, 0),
(683740, 7, -4048.76, -11763, 3.61992, 0, 0, 0, NULL, 100, 0),
(683740, 8, -4050.67, -11754.4, 3.97728, 0, 0, 0, NULL, 100, 0),
(683740, 9, -4053.72, -11745.2, 3.52118, 0, 0, 0, NULL, 100, 0),
(683740, 10, -4060.91, -11740.2, 2.40871, 0, 0, 0, NULL, 100, 0),
(683740, 11, -4071.56, -11739.9, 2.33454, 0, 0, 0, NULL, 100, 0),
(683740, 12, -4082.88, -11746.9, 1.58454, 0, 0, 0, NULL, 100, 0),
(683740, 13, -4088.72, -11757.1, 2.70954, 0, 0, 0, NULL, 100, 0),
(683740, 14, -4092, -11770.2, 3.03216, 0, 0, 0, NULL, 100, 0),
(683740, 15, -4094.43, -11786.6, 1.28216, 0, 0, 0, NULL, 100, 0),
(683740, 16, -4096.99, -11802.2, 2.85965, 0, 0, 0, NULL, 100, 0),
(683740, 17, -4096.63, -11823.2, 3.48465, 0, 0, 0, NULL, 100, 0),
(683740, 18, -4093.69, -11846.4, 1.73607, 0, 0, 0, NULL, 100, 0),
(683740, 19, -4086.74, -11858.9, 1.36107, 0, 0, 0, NULL, 100, 0),
(683740, 20, -4082.61, -11871.9, -0.556801, 0, 0, 0, NULL, 100, 0),
(683740, 21, -4074.99, -11883.1, -1.8068, 0, 0, 0, NULL, 100, 0),
(683740, 22, -4059.66, -11891, -2.02328, 0, 0, 0, NULL, 100, 0),
(683740, 23, -4050.71, -11903.5, -2.04217, 0, 0, 0, NULL, 100, 0),
(683740, 24, -4050.01, -11914.3, -1.41717, 0, 0, 0, NULL, 100, 0),
(683740, 25, -4051.99, -11923.4, 0.207832, 0, 0, 0, NULL, 100, 0),
(683740, 26, -4046.21, -11935.2, 0.302898, 0, 0, 0, NULL, 100, 0),
(683740, 27, -4034.59, -11946.7, 2.1779, 0, 0, 0, NULL, 100, 0),
(683740, 28, -4016.19, -11956.6, 2.96377, 0, 0, 0, NULL, 100, 0),
(683740, 29, -3987.95, -11960.8, 0.790219, 0, 0, 0, NULL, 100, 0),
(683740, 30, -3963.28, -11951.5, -2.41474, 0, 0, 0, NULL, 100, 0),
(683740, 31, -3964.19, -11932.9, -0.617626, 0, 0, 0, NULL, 100, 0),
(683740, 32, -3987.75, -11914.7, 0.598576, 0, 0, 0, NULL, 100, 0),
(683740, 33, -3996.59, -11896.2, -0.188298, 0, 0, 0, NULL, 100, 0),
(683740, 34, -4000.58, -11878.7, -0.273137, 0, 0, 0, NULL, 100, 0),
(683740, 35, -4004.18, -11865.8, 0.166556, 0, 0, 0, NULL, 100, 0),
(683740, 36, -4014.64, -11859, 0.0579128, 0, 0, 0, NULL, 100, 0),
(683740, 37, -4026.57, -11861.9, -0.0800333, 0, 0, 0, NULL, 100, 0),
(683740, 38, -4034.73, -11867.3, -0.8302, 0, 0, 0, NULL, 100, 0),
(683740, 39, -4034.98, -11870.9, 0.101723, 0, 0, 0, NULL, 100, 0),
(683740, 40, -4039.96, -11874, 0.101723, 0, 0, 0, NULL, 100, 0),
(683740, 41, -4045.71, -11877, -0.148277, 0, 0, 0, NULL, 100, 0),
(683740, 42, -4049.79, -11872.8, 0.337155, 0, 0, 0, NULL, 100, 0),
(683740, 43, -4051.89, -11864.4, 1.78296, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (68374, 683740, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 96619 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1094830 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1094830 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1094830 AND spawnType = 0 WHERE ce.spawnID = 1094830;
INSERT INTO creature_entry (spawnID, entry) VALUES (96619, 25138);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (96619, 530, 1, 22928, 0, -1842.62, 5313.06,  -12.4282, 3.14159, 180, 0, 0, 7181, 0, 2, 0, 0, 10);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (8025158, 0, 1, 25, 0, 0, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (8025159, 0, 1, 22, 0, 0, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (8025159, 0, 16, 9739, 2, 0, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (8025160, 0, 1, 18, 0, 0, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (8025161, 0, 1, 5, 0, 0, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (8025162, 0, 1, 21, 0, 0, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1012, 0, 1, 274, 0, 0, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (8025163, 0, 1, 11, 0, 0, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (8025164, 0, 1, 273, 0, 0, 0, 0, 0, 0);
INSERT INTO waypoint_data VALUES 
(966190, 1, -1842.62, 5313.06, -12.4282, 0, 2000, 0, NULL, 100, 0),
(966190, 2, -1842.62, 5313.06, -12.4282, 2.47837, 4000, 0, 8025158, 100, 0),
(966190, 3, -1842.62, 5313.06, -12.4282, 0, 4000, 0, NULL, 100, 0),
(966190, 4, -1842.62, 5313.06, -12.4282, 0, 12000, 0, 8025159, 100, 0),
(966190, 5, -1842.62, 5313.06, -12.4282, 0, 4000, 0, 8025160, 100, 0),
(966190, 6, -1850.03, 5310.6, -12.4282, 0, 0, 0, NULL, 100, 0),
(966190, 7, -1852.46, 5313.78, -12.4282, 0, 0, 0, NULL, 100, 0),
(966190, 8, -1851.37, 5318.35, -12.4282, 0, 0, 0, NULL, 100, 0),
(966190, 9, -1843.58, 5320.41, -12.4282, 0, 0, 0, NULL, 100, 0),
(966190, 10, -1840.26, 5319.29, -12.4282, 0, 1000, 0, NULL, 100, 0),
(966190, 11, -1840.26, 5319.29, -12.4282, 4.50295, 4000, 0, 50, 100, 0),
(966190, 12, -1840.26, 5319.29, -12.4282, 0, 13000, 0, 8025161, 100, 0),
(966190, 13, -1840.26, 5319.29, -12.4282, 0, 4000, 0, 8025162, 100, 0),
(966190, 14, -1835.1, 5320.24, -12.4282, 0, 0, 0, NULL, 100, 0),
(966190, 15, -1835.24, 5316.95, -12.4282, 0, 1000, 0, NULL, 100, 0),
(966190, 16, -1835.23, 5317.26, -12.4282, 3.14159, 20000, 0, NULL, 100, 0),
(966190, 17, -1835.23, 5317.26, -12.4282, 0, 11000, 0, 14, 100, 0),
(966190, 18, -1840.23, 5311.87, -12.4282, 0, 0, 0, NULL, 100, 0),
(966190, 19, -1842.64, 5313, -12.4282, 0, 1000, 0, NULL, 100, 0),
(966190, 20, -1842.64, 5313, -12.4282, 1.23918, 4000, 0, 50, 100, 0),
(966190, 21, -1842.64, 5313, -12.4282, 0, 7000, 0, 1012, 100, 0),
(966190, 22, -1842.64, 5313, -12.4282, 0, 3000, 0, 50, 100, 0),
(966190, 23, -1842.64, 5313, -12.4282, 0, 7000, 0, 8025163, 100, 0),
(966190, 24, -1847.88, 5310.52, -12.4282, 0, 0, 0, NULL, 100, 0),
(966190, 25, -1851.1, 5314.04, -12.4282, 0, 1000, 0, NULL, 100, 0),
(966190, 26, -1851.1, 5314.04, -12.4282, 0.0872665, 5000, 0, NULL, 100, 0),
(966190, 27, -1851.1, 5314.04, -12.4282, 0, 21000, 0, 14, 100, 0),
(966190, 28, -1846.24, 5319.44, -12.4282, 0, 0, 0, NULL, 100, 0),
(966190, 29, -1843.72, 5318.52, -12.4282, 0, 0, 0, NULL, 100, 0),
(966190, 30, -1843.72, 5318.52, -12.4282, 5.44543, 7000, 0, 50, 100, 0),
(966190, 31, -1843.72, 5318.52, -12.4282, 0, 4000, 0, 8025164, 100, 0),
(966190, 32, -1843.72, 5318.52, -12.4282, 0, 11000, 0, 12, 100, 0),
(966190, 33, -1836.77, 5321.29, -12.4282, 0, 0, 0, NULL, 100, 0),
(966190, 34, -1835.24, 5316.95, -12.4282, 0, 21000, 0, NULL, 100, 0),
(966190, 35, -1835.24, 5316.95, -12.4282, 3.14159, 10000, 0, 14, 100, 0),
(966190, 36, -1840.23, 5311.87, -12.4282, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (96619, 966190, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 41792 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1041606 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1041606 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1041606 AND spawnType = 0 WHERE ce.spawnID = 1041606;
INSERT INTO creature_entry (spawnID, entry) VALUES (41792, 15937);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (41792, 530, 1, 0, 1, 8869.87, -5775.96,  0.405, 1.25926, 300, 0, 1, 183, 178, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(417920, 1, 8880.69, -5754.61, 0.25531, 0, 0, 0, NULL, 100, 0),
(417920, 2, 8873.34, -5735.01, 0.43308, 0, 0, 0, NULL, 100, 0),
(417920, 3, 8862.79, -5722.62, 0.42263, 0, 0, 0, NULL, 100, 0),
(417920, 4, 8846.43, -5720.66, 0.49396, 0, 0, 0, NULL, 100, 0),
(417920, 5, 8836.92, -5726.74, 0.83736, 0, 0, 0, NULL, 100, 0),
(417920, 6, 8828.79, -5743.62, 0.8259, 0, 0, 0, NULL, 100, 0),
(417920, 7, 8818.04, -5755.89, 0.64673, 0, 0, 0, NULL, 100, 0),
(417920, 8, 8804.55, -5762.79, 0.51199, 0, 0, 0, NULL, 100, 0),
(417920, 9, 8791.79, -5762.27, 0.45083, 0, 0, 0, NULL, 100, 0),
(417920, 10, 8777.22, -5753.97, 0.55571, 0, 0, 0, NULL, 100, 0),
(417920, 11, 8776.86, -5738.23, 0.61609, 0, 0, 0, NULL, 100, 0),
(417920, 12, 8785.23, -5722.79, 0.75764, 0, 0, 0, NULL, 100, 0),
(417920, 13, 8786.4, -5701.79, 0.67103, 0, 0, 0, NULL, 100, 0),
(417920, 14, 8771.28, -5696.08, 0.67596, 0, 0, 0, NULL, 100, 0),
(417920, 15, 8758.6, -5694.22, 0.55669, 0, 0, 0, NULL, 100, 0),
(417920, 16, 8746.01, -5704.51, 0.50786, 0, 0, 0, NULL, 100, 0),
(417920, 17, 8735.82, -5722.94, 0.94607, 0, 0, 0, NULL, 100, 0),
(417920, 18, 8720.27, -5730.86, 0.89406, 0, 0, 0, NULL, 100, 0),
(417920, 19, 8706.96, -5730.08, 0.77437, 0, 0, 0, NULL, 100, 0),
(417920, 20, 8693.58, -5720.97, 0.86999, 0, 0, 0, NULL, 100, 0),
(417920, 21, 8679.18, -5710.54, 1.10842, 0, 0, 0, NULL, 100, 0),
(417920, 22, 8656.02, -5704.34, 1.80973, 0, 0, 0, NULL, 100, 0),
(417920, 23, 8640.98, -5691.51, 0.4769, 0, 0, 0, NULL, 100, 0),
(417920, 24, 8635.42, -5689.71, 0.53144, 0, 0, 0, NULL, 100, 0),
(417920, 25, 8620.44, -5687.68, 0.86486, 0, 0, 0, NULL, 100, 0),
(417920, 26, 8601.69, -5688.44, 0.49338, 1.93, 36000, 0, NULL, 100, 0),
(417920, 27, 8619.99, -5679.97, 0.75128, 0, 0, 0, NULL, 100, 0),
(417920, 28, 8635.67, -5671.58, 0.53144, 0, 0, 0, NULL, 100, 0),
(417920, 29, 8648.3, -5670.15, 0.49488, 0, 0, 0, NULL, 100, 0),
(417920, 30, 8664.99, -5679.42, 0.55752, 0, 0, 0, NULL, 100, 0),
(417920, 31, 8679.18, -5710.54, 1.10842, 0, 0, 0, NULL, 100, 0),
(417920, 32, 8693.58, -5720.97, 0.86999, 0, 0, 0, NULL, 100, 0),
(417920, 33, 8706.96, -5730.08, 0.77437, 0, 0, 0, NULL, 100, 0),
(417920, 34, 8721.72, -5745.01, 0.62278, 0, 0, 0, NULL, 100, 0),
(417920, 35, 8735.53, -5753.92, 1.24721, 0, 0, 0, NULL, 100, 0),
(417920, 36, 8767.19, -5758.18, 0.50536, 0, 0, 0, NULL, 100, 0),
(417920, 37, 8791.79, -5762.27, 0.45083, 0, 0, 0, NULL, 100, 0),
(417920, 38, 8804.55, -5762.79, 0.51199, 0, 0, 0, NULL, 100, 0),
(417920, 39, 8818.04, -5755.89, 0.64673, 0, 0, 0, NULL, 100, 0),
(417920, 40, 8828.79, -5743.62, 0.8259, 0, 0, 0, NULL, 100, 0),
(417920, 41, 8836.92, -5726.74, 0.83736, 0, 0, 0, NULL, 100, 0),
(417920, 42, 8846.43, -5720.66, 0.49396, 0, 0, 0, NULL, 100, 0),
(417920, 43, 8873.34, -5735.01, 0.43308, 0, 0, 0, NULL, 100, 0),
(417920, 44, 8880.69, -5754.61, 0.25531, 6.105, 60000, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (41792, 417920, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 41792);

-- Importing creature with guid 68851 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1068092 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1068092 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1068092 AND spawnType = 0 WHERE ce.spawnID = 1068092;
INSERT INTO creature_entry (spawnID, entry) VALUES (68851, 19282);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (68851, 530, 1, 0, 1, 54.6004, 1805.33,  49.121, 1.30409, 300, 0, 0, 4274, 2568, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(688510, 1, 61.3941, 1830.15, 42.7395, 0, 0, 0, NULL, 100, 0),
(688510, 2, 56.415, 1842.11, 42.807, 0, 0, 0, NULL, 100, 0),
(688510, 3, 45.9743, 1851.92, 45.3189, 0, 0, 0, NULL, 100, 0),
(688510, 4, 24.9654, 1857.52, 54.0798, 0, 0, 0, NULL, 100, 0),
(688510, 5, 12.3863, 1857.22, 59.0409, 0, 0, 0, NULL, 100, 0),
(688510, 6, 2.08952, 1853.45, 63.623, 0, 0, 0, NULL, 100, 0),
(688510, 7, -10.9102, 1844.12, 69.4542, 0, 0, 0, NULL, 100, 0),
(688510, 8, -15.7601, 1833.52, 71.1901, 0, 0, 0, NULL, 100, 0),
(688510, 9, -13.4923, 1816.22, 66.9124, 0, 0, 0, NULL, 100, 0),
(688510, 10, -13.5711, 1802.38, 64.3102, 0, 0, 0, NULL, 100, 0),
(688510, 11, -13.0287, 1783.14, 61.3505, 0, 0, 0, NULL, 100, 0),
(688510, 12, -3.03895, 1762, 55.1634, 0, 0, 0, NULL, 100, 0),
(688510, 13, 9.05339, 1752.68, 51.4746, 0, 0, 0, NULL, 100, 0),
(688510, 14, 21.6882, 1744.58, 48.8908, 0, 0, 0, NULL, 100, 0),
(688510, 15, 30.3467, 1750.99, 49.4121, 0, 0, 0, NULL, 100, 0),
(688510, 16, 42.704, 1761.81, 49.7766, 0, 0, 0, NULL, 100, 0),
(688510, 17, 48.1866, 1777.87, 51.4582, 0, 0, 0, NULL, 100, 0),
(688510, 18, 55.3523, 1790.65, 51.1114, 0, 0, 0, NULL, 100, 0),
(688510, 19, 57.8812, 1800.58, 49.1871, 0, 0, 0, NULL, 100, 0),
(688510, 20, 59.4079, 1812.94, 46.4711, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (68851, 688510, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 68851);

-- Importing creature with guid 40619 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1040433 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1040433 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1040433 AND spawnType = 0 WHERE ce.spawnID = 1040433;
INSERT INTO creature_entry (spawnID, entry) VALUES (40619, 23267);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (40619, 530, 1, 0, 0, -5115.44, 144.561,  130.161, 1.37753, 300, 0, 0, 36900, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(406190, 1, -5115.44, 144.561, 130.161, 0, 0, 0, NULL, 100, 0),
(406190, 2, -5127.7, 139.548, 130.185, 0, 0, 0, NULL, 100, 0),
(406190, 3, -5122.23, 120.374, 129.984, 0, 0, 0, NULL, 100, 0),
(406190, 4, -5117.04, 117.673, 129.909, 0, 0, 0, NULL, 100, 0),
(406190, 5, -5107.68, 117.856, 129.841, 0, 0, 0, NULL, 100, 0),
(406190, 6, -5100.67, 122.349, 130.089, 0, 0, 0, NULL, 100, 0),
(406190, 7, -5105.3, 139.746, 130.135, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (40619, 406190, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 68916 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1068156 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1068156 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1068156 AND spawnType = 0 WHERE ce.spawnID = 1068156;
INSERT INTO creature_entry (spawnID, entry) VALUES (68916, 19335);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (68916, 530, 1, 0, 1, -500.38, 1718.96,  60.8718, 3.73401, 300, 0, 0, 4274, 2568, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(689160, 1, -516.502, 1708.1, 57.8987, 0, 0, 0, NULL, 100, 0),
(689160, 2, -533.43, 1692.13, 53.8775, 0, 0, 0, NULL, 100, 0),
(689160, 3, -532.113, 1686.31, 51.6518, 0, 0, 0, NULL, 100, 0),
(689160, 4, -526.864, 1677.81, 47.3628, 0, 0, 0, NULL, 100, 0),
(689160, 5, -510.774, 1675.96, 47.5125, 0, 0, 0, NULL, 100, 0),
(689160, 6, -490.478, 1679.86, 49.2432, 0, 0, 0, NULL, 100, 0),
(689160, 7, -482.058, 1687.76, 51.389, 0, 0, 0, NULL, 100, 0),
(689160, 8, -480.32, 1701.18, 55.1524, 0, 0, 0, NULL, 100, 0),
(689160, 9, -489.208, 1714.01, 58.4839, 0, 0, 0, NULL, 100, 0),
(689160, 10, -496.469, 1719.32, 60.7473, 0, 0, 0, NULL, 100, 0),
(689160, 11, -505.473, 1714.47, 59.5528, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (68916, 689160, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 68916);

-- Importing creature with guid 69051 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1068283 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1068283 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1068283 AND spawnType = 0 WHERE ce.spawnID = 1068283;
INSERT INTO creature_entry (spawnID, entry) VALUES (69051, 19354);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (69051, 530, 1, 0, 1, -617.618, 4800.32,  38.5306, 6.0912, 300, 0, 0, 30954, 5240, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(690510, 1, -546.551, 4799.89, 33.8391, 0, 0, 0, NULL, 100, 0),
(690510, 2, -512.581, 4799.93, 32.0993, 0, 0, 0, NULL, 100, 0),
(690510, 3, -480.925, 4799.97, 28.2566, 0, 0, 0, NULL, 100, 0),
(690510, 4, -512.627, 4799.87, 32.0993, 0, 0, 0, NULL, 100, 0),
(690510, 5, -542.188, 4799.72, 33.7141, 0, 0, 0, NULL, 100, 0),
(690510, 6, -570.381, 4800.45, 34.6021, 0, 0, 0, NULL, 100, 0),
(690510, 7, -592.381, 4800.3, 35.8521, 0, 0, 0, NULL, 100, 0),
(690510, 8, -617.618, 4800.32, 38.5306, 0, 0, 0, NULL, 100, 0),
(690510, 9, -659.412, 4799.82, 49.0951, 0, 0, 0, NULL, 100, 0),
(690510, 10, -582.42, 4800.24, 34.9771, 0, 0, 0, NULL, 100, 0),
(690510, 11, -546.551, 4799.89, 33.8391, 0, 0, 0, NULL, 100, 0),
(690510, 12, -542.188, 4799.72, 33.7141, 0, 0, 0, NULL, 100, 0),
(690510, 13, -570.381, 4800.45, 34.6021, 0, 0, 0, NULL, 100, 0),
(690510, 14, -592.381, 4800.3, 35.8521, 0, 0, 0, NULL, 100, 0),
(690510, 15, -617.618, 4800.32, 38.5306, 0, 0, 0, NULL, 100, 0),
(690510, 16, -659.412, 4799.82, 49.0951, 0, 0, 0, NULL, 100, 0),
(690510, 17, -512.581, 4799.93, 32.0993, 0, 0, 0, NULL, 100, 0),
(690510, 18, -480.925, 4799.97, 28.2566, 0, 0, 0, NULL, 100, 0),
(690510, 19, -512.627, 4799.87, 32.0993, 0, 0, 0, NULL, 100, 0),
(690510, 20, -542.188, 4799.72, 33.7141, 0, 0, 0, NULL, 100, 0),
(690510, 21, -570.381, 4800.45, 34.6021, 0, 0, 0, NULL, 100, 0),
(690510, 22, -592.381, 4800.3, 35.8521, 0, 0, 0, NULL, 100, 0),
(690510, 23, -617.618, 4800.32, 38.5306, 0, 0, 0, NULL, 100, 0),
(690510, 24, -659.412, 4799.82, 49.0951, 0, 0, 0, NULL, 100, 0),
(690510, 25, -582.42, 4800.24, 34.9771, 0, 0, 0, NULL, 100, 0),
(690510, 26, -512.627, 4799.87, 32.0993, 0, 0, 0, NULL, 100, 0),
(690510, 27, -542.188, 4799.72, 33.7141, 0, 0, 0, NULL, 100, 0),
(690510, 28, -570.381, 4800.45, 34.6021, 0, 0, 0, NULL, 100, 0),
(690510, 29, -592.381, 4800.3, 35.8521, 0, 0, 0, NULL, 100, 0),
(690510, 30, -617.618, 4800.32, 38.5306, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (69051, 690510, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 69634 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1068863 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1068863 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1068863 AND spawnType = 0 WHERE ce.spawnID = 1068863;
INSERT INTO creature_entry (spawnID, entry) VALUES (69634, 19442);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (69634, 530, 1, 0, 0, -1057.88, 1996.64,  67.3418, 1.04782, 300, 0, 0, 3734, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(696340, 1, -1057.88, 1996.64, 67.3418, 0, 0, 0, NULL, 100, 0),
(696340, 2, -1050.19, 2009.98, 67.0646, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (69634, 696340, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 69634);

-- Importing creature with guid 69855 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1069084 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1069084 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1069084 AND spawnType = 0 WHERE ce.spawnID = 1069084;
DELETE FROM waypoint_data WHERE id = 12949501;
INSERT INTO creature_entry (spawnID, entry) VALUES (69855, 19494);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (69855, 530, 1, 0, 0, 2240.61, 2231.97,  101.754, 0.156277, 300, 0, 0, 6280, 2991, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(698550, 1, 2235.43, 2232.7, 101.374, 0, 0, 0, NULL, 100, 0),
(698550, 2, 2234.17, 2236.42, 101.136, 0, 0, 0, NULL, 100, 0),
(698550, 3, 2237.61, 2241.54, 101.158, 0, 0, 0, NULL, 100, 0),
(698550, 4, 2241.38, 2242.14, 101.389, 0, 0, 0, NULL, 100, 0),
(698550, 5, 2243.61, 2240.11, 101.623, 0, 0, 0, NULL, 100, 0),
(698550, 6, 2243.74, 2236.39, 101.784, 0, 0, 0, NULL, 100, 0),
(698550, 7, 2240.61, 2231.97, 101.754, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (69855, 698550, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 69855);

-- Importing creature with guid 69957 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1069186 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1069186 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1069186 AND spawnType = 0 WHERE ce.spawnID = 1069186;
INSERT INTO creature_entry (spawnID, entry) VALUES (69957, 19544);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (69957, 530, 1, 0, 1, 2308.53, 2152.49,  89.9053, 5.59527, 300, 0, 0, 6803, 5982, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(699570, 1, 2304.67, 2153.75, 90.0303, 0, 0, 0, NULL, 100, 0),
(699570, 2, 2308.53, 2152.49, 89.9053, 0, 0, 0, NULL, 100, 0),
(699570, 3, 2315.14, 2149.99, 87.9053, 0, 0, 0, NULL, 100, 0),
(699570, 4, 2317.1, 2141.94, 86.4053, 0, 0, 0, NULL, 100, 0),
(699570, 5, 2312.28, 2133.23, 86.0655, 0, 0, 0, NULL, 100, 0),
(699570, 6, 2304.42, 2127.04, 83.9405, 0, 0, 0, NULL, 100, 0),
(699570, 7, 2300.4, 2122.59, 82.5655, 0, 0, 0, NULL, 100, 0),
(699570, 8, 2293.57, 2129.68, 83.018, 0, 0, 0, NULL, 100, 0),
(699570, 9, 2280.72, 2142.29, 84.0518, 0, 0, 0, NULL, 100, 0),
(699570, 10, 2277.71, 2145.19, 83.1911, 0, 0, 0, NULL, 100, 0),
(699570, 11, 2271.71, 2150.19, 79.6911, 0, 0, 0, NULL, 100, 0),
(699570, 12, 2266.21, 2154.59, 81.3303, 0, 0, 0, NULL, 100, 0),
(699570, 13, 2256.74, 2164.51, 82.0803, 0, 0, 0, NULL, 100, 0),
(699570, 14, 2266.21, 2154.59, 81.3303, 0, 0, 0, NULL, 100, 0),
(699570, 15, 2271.71, 2150.19, 79.6911, 0, 0, 0, NULL, 100, 0),
(699570, 16, 2277.71, 2145.19, 83.1911, 0, 0, 0, NULL, 100, 0),
(699570, 17, 2280.72, 2142.29, 84.0518, 0, 0, 0, NULL, 100, 0),
(699570, 18, 2293.57, 2129.68, 83.018, 0, 0, 0, NULL, 100, 0),
(699570, 19, 2300.18, 2122.35, 82.5655, 0, 0, 0, NULL, 100, 0),
(699570, 20, 2304.19, 2126.82, 83.9405, 0, 0, 0, NULL, 100, 0),
(699570, 21, 2312.28, 2133.23, 86.0655, 0, 0, 0, NULL, 100, 0),
(699570, 22, 2317.1, 2141.94, 86.4053, 0, 0, 0, NULL, 100, 0),
(699570, 23, 2315.14, 2149.99, 87.9053, 0, 0, 0, NULL, 100, 0),
(699570, 24, 2308.53, 2152.49, 89.9053, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (69957, 699570, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 69959 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1069188 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1069188 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1069188 AND spawnType = 0 WHERE ce.spawnID = 1069188;
INSERT INTO creature_entry (spawnID, entry) VALUES (69959, 19546);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (69959, 530, 1, 0, 1, 2162.67, 2239.2,  74.9615, 3.86508, 300, 0, 0, 6803, 4786, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(699590, 1, 2161.25, 2246.41, 75.0865, 0, 0, 0, NULL, 100, 0),
(699590, 2, 2167.95, 2252.32, 75.3343, 0, 0, 0, NULL, 100, 0),
(699590, 3, 2176.67, 2259.39, 76.0843, 0, 0, 0, NULL, 100, 0),
(699590, 4, 2167.95, 2252.32, 75.3343, 0, 0, 0, NULL, 100, 0),
(699590, 5, 2161.25, 2246.41, 75.0865, 0, 0, 0, NULL, 100, 0),
(699590, 6, 2162.67, 2239.2, 74.9615, 0, 0, 0, NULL, 100, 0),
(699590, 7, 2170.98, 2229.72, 75.3562, 0, 0, 0, NULL, 100, 0),
(699590, 8, 2173.58, 2222.55, 75.4812, 0, 0, 0, NULL, 100, 0),
(699590, 9, 2172.49, 2216.01, 74.8562, 0, 0, 0, NULL, 100, 0),
(699590, 10, 2171.12, 2206.34, 73.3562, 0, 0, 0, NULL, 100, 0),
(699590, 11, 2171.41, 2196.35, 72.1642, 0, 0, 0, NULL, 100, 0),
(699590, 12, 2173.67, 2191.28, 71.6642, 0, 0, 0, NULL, 100, 0),
(699590, 13, 2177.41, 2192.71, 72.9142, 0, 0, 0, NULL, 100, 0),
(699590, 14, 2173.67, 2191.28, 71.6642, 0, 0, 0, NULL, 100, 0),
(699590, 15, 2171.41, 2196.13, 72.1642, 0, 0, 0, NULL, 100, 0),
(699590, 16, 2171.12, 2206.34, 73.3562, 0, 0, 0, NULL, 100, 0),
(699590, 17, 2172.49, 2216.01, 74.8562, 0, 0, 0, NULL, 100, 0),
(699590, 18, 2173.58, 2222.55, 75.4812, 0, 0, 0, NULL, 100, 0),
(699590, 19, 2170.98, 2229.72, 75.3562, 0, 0, 0, NULL, 100, 0),
(699590, 20, 2162.67, 2239.2, 74.9615, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (69959, 699590, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 70177 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1310619 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1310619 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1310619 AND spawnType = 0 WHERE ce.spawnID = 1310619;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1310815 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1310815 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1310815 AND spawnType = 0 WHERE ce.spawnID = 1310815;
DELETE FROM waypoint_data WHERE id = 12949525;
INSERT INTO creature_entry (spawnID, entry) VALUES (70177, 19684);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (70177, 530, 1, 0, 0, -1560.23, 5327.7,  11.5866, 5.93463, 540, 0, 0, 7097, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(701770, 1, -1560.23, 5327.7, 11.5864, 0, 20000, 0, NULL, 100, 0),
(701770, 2, -1596.7, 5340.36, 17.2558, 0, 0, 0, NULL, 100, 0),
(701770, 3, -1638.51, 5354.75, 15.8854, 0, 0, 0, NULL, 100, 0),
(701770, 4, -1685.49, 5370.25, 7.72029, 0, 20000, 0, NULL, 100, 0),
(701770, 5, -1638.67, 5354.71, 15.8777, 0, 0, 0, NULL, 100, 0),
(701770, 6, -1596.5, 5340.39, 17.2532, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (70177, 701770, 0, 0, 0, 0, 0, NULL);

-- Importing creature with guid 71806 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1071001 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1071001 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1071001 AND spawnType = 0 WHERE ce.spawnID = 1071001;
INSERT INTO creature_entry (spawnID, entry) VALUES (71806, 20202);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (71806, 530, 1, 0, 0, 4672.65, 2833.64,  119.721, 2.08893, 300, 0, 0, 27044, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(718060, 1, 4691.32, 2831.76, 115.228, 0, 0, 0, NULL, 100, 0),
(718060, 2, 4711.75, 2826.13, 110.992, 0, 0, 0, NULL, 100, 0),
(718060, 3, 4722.81, 2820.74, 108.242, 0, 0, 0, NULL, 100, 0),
(718060, 4, 4730.56, 2807.48, 104.117, 0, 0, 0, NULL, 100, 0),
(718060, 5, 4735.9, 2790.55, 97.9759, 0, 0, 0, NULL, 100, 0),
(718060, 6, 4735.55, 2772.66, 94.9759, 0, 0, 0, NULL, 100, 0),
(718060, 7, 4746.17, 2759.38, 88.9043, 0, 0, 0, NULL, 100, 0),
(718060, 8, 4758.18, 2753.98, 84.2793, 0, 0, 0, NULL, 100, 0),
(718060, 9, 4775.82, 2729.57, 81.3475, 0, 0, 0, NULL, 100, 0),
(718060, 10, 4797.15, 2714.69, 83.2225, 0, 0, 0, NULL, 100, 0),
(718060, 11, 4816.37, 2706.6, 87.9414, 0, 0, 0, NULL, 100, 0),
(718060, 12, 4797.17, 2714.67, 83.2225, 0, 0, 0, NULL, 100, 0),
(718060, 13, 4775.84, 2729.55, 81.3475, 0, 0, 0, NULL, 100, 0),
(718060, 14, 4758.18, 2753.98, 84.2793, 0, 0, 0, NULL, 100, 0),
(718060, 15, 4746.27, 2759.26, 88.9043, 0, 0, 0, NULL, 100, 0),
(718060, 16, 4735.64, 2772.54, 94.9759, 0, 0, 0, NULL, 100, 0),
(718060, 17, 4735.9, 2790.55, 97.9759, 0, 0, 0, NULL, 100, 0),
(718060, 18, 4730.56, 2807.48, 104.117, 0, 0, 0, NULL, 100, 0),
(718060, 19, 4722.81, 2820.74, 108.242, 0, 0, 0, NULL, 100, 0),
(718060, 20, 4712.02, 2825.99, 110.992, 0, 0, 0, NULL, 100, 0),
(718060, 21, 4691.32, 2831.76, 115.228, 0, 0, 0, NULL, 100, 0),
(718060, 22, 4672.65, 2833.64, 119.721, 0, 1000, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (71806, 718060, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 71806);

-- Importing creature with guid 72380 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1071571 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1071571 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1071571 AND spawnType = 0 WHERE ce.spawnID = 1071571;
INSERT INTO creature_entry (spawnID, entry) VALUES (72380, 20410);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (72380, 530, 1, 0, 1, 2288.35, 2051.09,  69.9698, 1.8877, 300, 0, 0, 7437, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(723800, 1, 2293.53, 2058.44, 72.1724, 0, 0, 0, NULL, 100, 0),
(723800, 2, 2298.7, 2070.5, 74.8287, 0, 0, 0, NULL, 100, 0),
(723800, 3, 2305.21, 2079.17, 75.7038, 0, 0, 0, NULL, 100, 0),
(723800, 4, 2305.77, 2084.44, 76.5788, 0, 0, 0, NULL, 100, 0),
(723800, 5, 2302.87, 2090.55, 77.0788, 0, 0, 0, NULL, 100, 0),
(723800, 6, 2300.35, 2092.86, 77.5788, 0, 0, 0, NULL, 100, 0),
(723800, 7, 2296.73, 2097.01, 77.7609, 0, 0, 0, NULL, 100, 0),
(723800, 8, 2295.74, 2100.04, 78.143, 0, 0, 0, NULL, 100, 0),
(723800, 9, 2293.43, 2105.37, 77.893, 0, 0, 0, NULL, 100, 0),
(723800, 10, 2287.4, 2103.31, 71.393, 0, 0, 0, NULL, 100, 0),
(723800, 11, 2283.01, 2090.36, 65.8859, 0, 0, 0, NULL, 100, 0),
(723800, 12, 2278.57, 2080, 65.7609, 0, 0, 0, NULL, 100, 0),
(723800, 13, 2276.91, 2071.6, 66.0109, 0, 0, 0, NULL, 100, 0),
(723800, 14, 2276.64, 2060.94, 65.5769, 0, 0, 0, NULL, 100, 0),
(723800, 15, 2278.97, 2052.99, 65.0769, 0, 0, 0, NULL, 100, 0),
(723800, 16, 2284.63, 2046.86, 67.3269, 0, 0, 0, NULL, 100, 0),
(723800, 17, 2288.35, 2051.09, 69.9698, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (72380, 723800, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 72380);

-- Importing creature with guid 73833 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1073005 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1073005 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1073005 AND spawnType = 0 WHERE ce.spawnID = 1073005;
INSERT INTO creature_entry (spawnID, entry) VALUES (73833, 21023);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (73833, 530, 1, 0, 0, 1650.71, 6811.52,  132.554, 0.402455, 300, 0, 0, 8562, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(738330, 1, 1650.71, 6811.52, 132.554, 0, 0, 0, NULL, 100, 0),
(738330, 2, 1628.18, 6815.86, 131.363, 0, 0, 0, NULL, 100, 0),
(738330, 3, 1599.08, 6824.65, 129.789, 0, 0, 0, NULL, 100, 0),
(738330, 4, 1581.41, 6824.94, 128.516, 0, 0, 0, NULL, 100, 0),
(738330, 5, 1551.04, 6811.22, 124.748, 2.93215, 60000, 0, NULL, 100, 0),
(738330, 6, 1622.58, 6818.29, 130.94, 0, 0, 0, NULL, 100, 0),
(738330, 7, 1656.38, 6811.23, 133.155, 0, 0, 0, NULL, 100, 0),
(738330, 8, 1669.52, 6812.48, 134.498, 0, 0, 0, NULL, 100, 0),
(738330, 9, 1702.39, 6827.35, 136.037, 0, 0, 0, NULL, 100, 0),
(738330, 10, 1725.43, 6823.91, 136.095, 0, 0, 0, NULL, 100, 0),
(738330, 11, 1761.48, 6809.95, 137.23, 0, 0, 0, NULL, 100, 0),
(738330, 12, 1795.98, 6805.21, 137.443, 0, 0, 0, NULL, 100, 0),
(738330, 13, 1832.18, 6817.38, 139.14, 0, 0, 0, NULL, 100, 0),
(738330, 14, 1854.52, 6825.78, 141.873, 0, 0, 0, NULL, 100, 0),
(738330, 15, 1897.89, 6834, 146.274, 0, 0, 0, NULL, 100, 0),
(738330, 16, 1909.73, 6837.44, 147.499, 3.14159, 30000, 0, NULL, 100, 0),
(738330, 17, 1897.89, 6834, 146.274, 0, 0, 0, NULL, 100, 0),
(738330, 18, 1854.52, 6825.78, 141.873, 0, 0, 0, NULL, 100, 0),
(738330, 19, 1833.45, 6816.63, 139.114, 0, 0, 0, NULL, 100, 0),
(738330, 20, 1797.23, 6805.47, 137.537, 0, 0, 0, NULL, 100, 0),
(738330, 21, 1766.81, 6807.92, 137.18, 0, 0, 0, NULL, 100, 0),
(738330, 22, 1724.68, 6823.42, 136.287, 0, 0, 0, NULL, 100, 0),
(738330, 23, 1703.28, 6827.58, 136.037, 0, 0, 0, NULL, 100, 0),
(738330, 24, 1666.31, 6811.95, 134.051, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (73833, 738330, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 73833);

-- Importing creature with guid 74219 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1073385 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1073385 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1073385 AND spawnType = 0 WHERE ce.spawnID = 1073385;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1073386 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1073386 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1073386 AND spawnType = 0 WHERE ce.spawnID = 1073386;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1073387 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1073387 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1073387 AND spawnType = 0 WHERE ce.spawnID = 1073387;
INSERT INTO creature_entry (spawnID, entry) VALUES (74219, 21153);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (74219, 530, 1, 0, 1, -3014.28, 2567.43,  141.622, 5, 300, 0, 0, 17742, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(742190, 1, -3014.28, 2567.43, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 2, -2997.18, 2547.18, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 3, -2975.06, 2527.35, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 4, -2944.82, 2526, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 5, -2925.33, 2546.51, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 6, -2919.25, 2565.87, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 7, -2920.08, 2589.36, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 8, -2931.99, 2608.76, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 9, -2955.63, 2618.87, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 10, -2977.5, 2619.67, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 11, -3014.67, 2609.54, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 12, -3055.04, 2590.46, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 13, -3085.54, 2571.63, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 14, -3114.72, 2555.92, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 15, -3143.01, 2552.03, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 16, -3176.69, 2560.46, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 17, -3190.57, 2587.82, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 18, -3187.1, 2615.27, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 19, -3164.5, 2634.34, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 20, -3137.51, 2639.93, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 21, -3099.44, 2633.7, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 22, -3074.34, 2622.92, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 23, -3057.4, 2608.77, 141.622, 0, 0, 1, NULL, 100, 0),
(742190, 24, -3045.57, 2592.94, 140.845, 0, 0, 1, NULL, 100, 0),
(742190, 25, -3046.09, 2591.68, 139.928, 0, 0, 1, NULL, 100, 0),
(742190, 26, -3028.98, 2582, 141.622, 0, 0, 1, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (74219, 742190, 17722, 0, 0, 1, 0, NULL);

-- Importing creature with guid 75406 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1074567 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1074567 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1074567 AND spawnType = 0 WHERE ce.spawnID = 1074567;
DELETE FROM waypoint_data WHERE id = 12949516;
INSERT INTO creature_entry (spawnID, entry) VALUES (75406, 21499);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (75406, 530, 1, 0, 0, -3227.25, 2925.98,  127.161, 2.3555, 300, 0, 0, 6761, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(754060, 1, -3226.74, 2913.81, 127.411, 0, 0, 0, NULL, 100, 0),
(754060, 2, -3241.95, 2900.24, 124.622, 0, 0, 0, NULL, 100, 0),
(754060, 3, -3257.54, 2892.95, 124.148, 0, 0, 0, NULL, 100, 0),
(754060, 4, -3260.38, 2902.82, 126.747, 0, 0, 0, NULL, 100, 0),
(754060, 5, -3257.54, 2892.95, 124.148, 0, 0, 0, NULL, 100, 0),
(754060, 6, -3242.13, 2900.16, 124.622, 0, 0, 0, NULL, 100, 0),
(754060, 7, -3226.74, 2913.81, 127.411, 0, 0, 0, NULL, 100, 0),
(754060, 8, -3227.25, 2925.98, 127.161, 0, 0, 0, NULL, 100, 0),
(754060, 9, -3230.34, 2935.83, 126.66, 0, 0, 0, NULL, 100, 0),
(754060, 10, -3235.28, 2949.02, 126.547, 0, 0, 0, NULL, 100, 0),
(754060, 11, -3242.43, 2956.18, 128.672, 0, 0, 0, NULL, 100, 0),
(754060, 12, -3252.24, 2960.57, 131.672, 0, 0, 0, NULL, 100, 0),
(754060, 13, -3268.66, 2966.76, 133.217, 0, 0, 0, NULL, 100, 0),
(754060, 14, -3281.35, 2975.18, 135.217, 0, 0, 0, NULL, 100, 0),
(754060, 15, -3285.78, 2985.24, 137.592, 0, 0, 0, NULL, 100, 0),
(754060, 16, -3275.05, 2993.83, 138.342, 0, 0, 0, NULL, 100, 0),
(754060, 17, -3262.91, 3001.22, 138.584, 0, 0, 0, NULL, 100, 0),
(754060, 18, -3275.05, 2993.83, 138.342, 0, 0, 0, NULL, 100, 0),
(754060, 19, -3285.81, 2985.29, 137.592, 0, 0, 0, NULL, 100, 0),
(754060, 20, -3281.35, 2975.18, 135.217, 0, 0, 0, NULL, 100, 0),
(754060, 21, -3268.66, 2966.76, 133.217, 0, 0, 0, NULL, 100, 0),
(754060, 22, -3252.24, 2960.57, 131.672, 0, 0, 0, NULL, 100, 0),
(754060, 23, -3242.43, 2956.18, 128.672, 0, 0, 0, NULL, 100, 0),
(754060, 24, -3235.28, 2949.02, 126.547, 0, 0, 0, NULL, 100, 0),
(754060, 25, -3230.34, 2935.83, 126.66, 0, 0, 0, NULL, 100, 0),
(754060, 26, -3227.25, 2925.98, 127.161, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (75406, 754060, 0, 0, 0, 0, 0, NULL);

-- Importing creature with guid 75407 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1074568 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1074568 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1074568 AND spawnType = 0 WHERE ce.spawnID = 1074568;
DELETE FROM waypoint_data WHERE id = 12949512;
INSERT INTO creature_entry (spawnID, entry) VALUES (75407, 21500);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (75407, 530, 1, 0, 1, -3342.88, 2942.85,  170.004, 2.49643, 300, 0, 0, 20932, 2991, 2, 0, 0, 10);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1137, 1, 1, 1, 0, 0, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1138, 1, 30, 0, 0, 0, 0, 0, 0, 2.79253);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1138, 2, 1, 1, 0, 0, 0, 0, 0, 0);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1138, 7, 1, 15, 0, 0, 0, 0, 0, 0);
INSERT INTO waypoint_data VALUES 
(754070, 1, -3344.89, 2947.08, 170.004, 0, 0, 0, NULL, 100, 0),
(754070, 2, -3347.48, 2971.63, 170.254, 0, 0, 0, NULL, 100, 0),
(754070, 3, -3376.49, 2993.47, 170.129, 0, 20000, 0, 1137, 100, 0),
(754070, 4, -3391.43, 2995.69, 170.004, 0, 0, 0, NULL, 100, 0),
(754070, 5, -3425, 2993.65, 170.129, 0, 0, 0, NULL, 100, 0),
(754070, 6, -3431.53, 2967.77, 170.629, 0, 20000, 0, 1137, 100, 0),
(754070, 7, -3431.22, 2965.88, 170.504, 0, 0, 0, NULL, 100, 0),
(754070, 8, -3399.56, 2957.2, 170.004, 0, 0, 0, NULL, 100, 0),
(754070, 9, -3388.71, 2936.74, 170.004, 0, 20000, 0, 1137, 100, 0),
(754070, 10, -3342.88, 2942.85, 170.004, 0, 40000, 0, 1138, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (75407, 754070, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 75407);

-- Importing creature with guid 76076 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1075224 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1075224 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1075224 AND spawnType = 0 WHERE ce.spawnID = 1075224;
DELETE FROM waypoint_data WHERE id = 1549;
INSERT INTO creature_entry (spawnID, entry) VALUES (76076, 21774);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (76076, 530, 1, 0, 1, -3970.86, 2191.12,  101.885, 0.523995, 300, 0, 0, 55890, 3155, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(760760, 1, -3959.92, 2188.49, 101.799, 0, 0, 0, NULL, 100, 0),
(760760, 2, -3951.73, 2200.17, 101.791, 0, 0, 0, NULL, 100, 0),
(760760, 3, -3958.36, 2210.04, 101.916, 0, 0, 0, NULL, 100, 0),
(760760, 4, -3967.2, 2208.17, 101.76, 0, 0, 0, NULL, 100, 0),
(760760, 5, -3972.31, 2201.52, 101.76, 0, 0, 0, NULL, 100, 0),
(760760, 6, -3992.19, 2215.02, 103.635, 0, 0, 0, NULL, 100, 0),
(760760, 7, -4014.11, 2218.04, 108.692, 0, 0, 0, NULL, 100, 0),
(760760, 8, -4023.19, 2214.69, 109.942, 0, 0, 0, NULL, 100, 0),
(760760, 9, -4019.97, 2183.27, 107.499, 0, 0, 0, NULL, 100, 0),
(760760, 10, -4000.39, 2174.05, 104.374, 0, 0, 0, NULL, 100, 0),
(760760, 11, -3970.86, 2191.12, 101.885, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (76076, 760760, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 76114 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1075262 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1075262 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1075262 AND spawnType = 0 WHERE ce.spawnID = 1075262;
INSERT INTO creature_entry (spawnID, entry) VALUES (76114, 21801);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (76114, 530, 1, 0, 0, -5140.63, 537.607,  225.587, 0.0201144, 300, 0, 0, 104790, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(218010, 1, -5140.63, 537.607, 225.587, 0, 0, 0, NULL, 100, 0),
(218010, 2, -5047.68, 660.953, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 3, -5011.98, 674.35, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 4, -4981.46, 669.827, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 5, -4959.01, 632.275, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 6, -4943.98, 608.147, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 7, -4929.5, 593.456, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 8, -4910.32, 557.001, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 9, -4886.5, 522.941, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 10, -4860.48, 492.944, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 11, -4853.53, 472.772, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 12, -4853.9, 449.395, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 13, -4862.35, 417.347, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 14, -4904.07, 347.145, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 15, -4938.95, 289.322, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 16, -4959.22, 247.53, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 17, -4971.73, 201.83, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 18, -4979.26, 156.942, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 19, -4987.87, 132.527, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 20, -4998.63, 107.514, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 21, -5020.08, 73.2122, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 22, -5061.16, 55.5101, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 23, -5077.84, 49.8271, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 24, -5112.82, 49.4327, 148.864, 0, 0, 0, NULL, 100, 0),
(218010, 25, -5175.66, 65.2347, 163.392, 0, 0, 0, NULL, 100, 0),
(218010, 26, -5198.99, 107.904, 173.198, 0, 0, 0, NULL, 100, 0),
(218010, 27, -5165.27, 219.071, 198.225, 0, 0, 0, NULL, 100, 0),
(218010, 28, -5113.01, 300.721, 198.225, 0, 0, 0, NULL, 100, 0),
(218010, 29, -5109.76, 353.395, 226.753, 0, 0, 0, NULL, 100, 0),
(218010, 30, -5113.03, 378.997, 240.614, 0, 0, 0, NULL, 100, 0),
(218010, 31, -5122.2, 411.497, 243.281, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (76114, 218010, 0, 0, 33554432, 1, 0, 55971);
INSERT INTO spawn_group VALUES (2, 0, 76114);

-- Importing creature with guid 76771 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1075911 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1075911 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1075911 AND spawnType = 0 WHERE ce.spawnID = 1075911;
INSERT INTO creature_entry (spawnID, entry) VALUES (76771, 21981);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (76771, 530, 1, 0, 1, 2725.66, 5433,  137.267, 5.78059, 300, 0, 0, 5060, 2933, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(767710, 1, 2725.66, 5433, 137.267, 0, 0, 0, NULL, 100, 0),
(767710, 2, 2740.28, 5424.16, 136.886, 0, 0, 0, NULL, 100, 0),
(767710, 3, 2778.74, 5427.51, 137.316, 0, 0, 0, NULL, 100, 0),
(767710, 4, 2798.09, 5437.46, 139.44, 0, 0, 0, NULL, 100, 0),
(767710, 5, 2808.7, 5436.92, 141.731, 0, 0, 0, NULL, 100, 0),
(767710, 6, 2819.96, 5431.45, 143.146, 0, 0, 0, NULL, 100, 0),
(767710, 7, 2830.02, 5442.56, 144.589, 0, 0, 0, NULL, 100, 0),
(767710, 8, 2837.54, 5444.86, 145.824, 0, 0, 0, NULL, 100, 0),
(767710, 9, 2913, 5537.76, 145.732, 0, 0, 0, NULL, 100, 0),
(767710, 10, 2924.75, 5609.17, 147.001, 0, 0, 0, NULL, 100, 0),
(767710, 11, 2997.06, 5624.45, 145.459, 0, 0, 0, NULL, 100, 0),
(767710, 12, 3035.74, 5648.66, 142.758, 0, 0, 0, NULL, 100, 0),
(767710, 13, 3064.1, 5637.02, 142.75, 0, 0, 0, NULL, 100, 0),
(767710, 14, 3091.82, 5638.48, 143.045, 0, 0, 0, NULL, 100, 0),
(767710, 15, 3112.31, 5598.4, 145.824, 0, 0, 0, NULL, 100, 0),
(767710, 16, 3137.98, 5586.58, 144.544, 0, 0, 0, NULL, 100, 0),
(767710, 17, 3156.05, 5588.72, 142.902, 0, 0, 0, NULL, 100, 0),
(767710, 18, 3168.7, 5565.94, 141.345, 0, 0, 0, NULL, 100, 0),
(767710, 19, 3168.57, 5555.67, 142.126, 0, 0, 0, NULL, 100, 0),
(767710, 20, 3126.19, 5532.68, 146.197, 0, 0, 0, NULL, 100, 0),
(767710, 21, 3141.66, 5472.75, 146.822, 0, 0, 0, NULL, 100, 0),
(767710, 22, 3285.31, 5440.08, 146.773, 0, 0, 0, NULL, 100, 0),
(767710, 23, 3306.93, 5437.95, 145.651, 0, 0, 0, NULL, 100, 0),
(767710, 24, 3333.17, 5413.44, 144.769, 0, 0, 0, NULL, 100, 0),
(767710, 25, 3366.1, 5415.71, 147.311, 0, 0, 0, NULL, 100, 0),
(767710, 26, 3378.88, 5404.93, 147.945, 0, 0, 0, NULL, 100, 0),
(767710, 27, 3398.48, 5404.43, 148.308, 0, 0, 0, NULL, 100, 0),
(767710, 28, 3398.48, 5404.43, 148.308, 0, 0, 0, NULL, 100, 0),
(767710, 29, 3378.88, 5404.93, 147.945, 0, 0, 0, NULL, 100, 0),
(767710, 30, 3366.1, 5415.71, 147.311, 0, 0, 0, NULL, 100, 0),
(767710, 31, 3333.17, 5413.44, 144.769, 0, 0, 0, NULL, 100, 0),
(767710, 32, 3306.93, 5437.95, 145.651, 0, 0, 0, NULL, 100, 0),
(767710, 33, 3285.31, 5440.08, 146.773, 0, 0, 0, NULL, 100, 0),
(767710, 34, 3141.66, 5472.75, 146.822, 0, 0, 0, NULL, 100, 0),
(767710, 35, 3126.19, 5532.68, 146.197, 0, 0, 0, NULL, 100, 0),
(767710, 36, 3168.57, 5555.67, 142.126, 0, 0, 0, NULL, 100, 0),
(767710, 37, 3168.7, 5565.94, 141.345, 0, 0, 0, NULL, 100, 0),
(767710, 38, 3156.05, 5588.72, 142.902, 0, 0, 0, NULL, 100, 0),
(767710, 39, 3137.98, 5586.58, 144.544, 0, 0, 0, NULL, 100, 0),
(767710, 40, 3112.31, 5598.4, 145.824, 0, 0, 0, NULL, 100, 0),
(767710, 41, 3091.82, 5638.48, 143.045, 0, 0, 0, NULL, 100, 0),
(767710, 42, 3064.1, 5637.02, 142.75, 0, 0, 0, NULL, 100, 0),
(767710, 43, 3035.74, 5648.66, 142.758, 0, 0, 0, NULL, 100, 0),
(767710, 44, 2997.06, 5624.45, 145.459, 0, 0, 0, NULL, 100, 0),
(767710, 45, 2924.75, 5609.17, 147.001, 0, 0, 0, NULL, 100, 0),
(767710, 46, 2913, 5537.76, 145.732, 0, 0, 0, NULL, 100, 0),
(767710, 47, 2837.54, 5444.86, 145.824, 0, 0, 0, NULL, 100, 0),
(767710, 48, 2830.02, 5442.56, 144.589, 0, 0, 0, NULL, 100, 0),
(767710, 49, 2819.96, 5431.45, 143.146, 0, 0, 0, NULL, 100, 0),
(767710, 50, 2808.7, 5436.92, 141.731, 0, 0, 0, NULL, 100, 0),
(767710, 51, 2798.09, 5437.46, 139.44, 0, 0, 0, NULL, 100, 0),
(767710, 52, 2778.74, 5427.51, 137.316, 0, 0, 0, NULL, 100, 0),
(767710, 53, 2740.28, 5424.16, 136.886, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (76771, 767710, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 77084 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1076224 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1076224 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1076224 AND spawnType = 0 WHERE ce.spawnID = 1076224;
INSERT INTO creature_entry (spawnID, entry) VALUES (77084, 22006);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (77084, 530, 1, 0, 1, -3225.12, 246.817,  195.679, 4.87723, 300, 0, 0, 83835, 18930, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(770840, 1, -3225.12, 246.817, 195.679, 0, 0, 0, NULL, 100, 0),
(770840, 2, -3240.03, 255.9, 201.579, 0, 0, 0, NULL, 100, 0),
(770840, 3, -3257.98, 269.678, 201.579, 0, 0, 0, NULL, 100, 0),
(770840, 4, -3274.72, 279.532, 201.579, 0, 0, 0, NULL, 100, 0),
(770840, 5, -3285.86, 310.516, 201.579, 0, 0, 0, NULL, 100, 0),
(770840, 6, -3275.1, 325.252, 201.579, 0, 0, 0, NULL, 100, 0),
(770840, 7, -3254.87, 341.812, 201.579, 0, 0, 0, NULL, 100, 0),
(770840, 8, -3227.61, 331.076, 201.579, 0, 0, 0, NULL, 100, 0),
(770840, 9, -3218.13, 316.963, 201.579, 0, 0, 0, NULL, 100, 0),
(770840, 10, -3217.59, 298.951, 201.579, 0, 0, 0, NULL, 100, 0),
(770840, 11, -3217.09, 283.996, 201.579, 0, 0, 0, NULL, 100, 0),
(770840, 12, -3207.64, 260.908, 203.19, 0, 0, 0, NULL, 100, 0),
(770840, 13, -3212.76, 247.542, 203.19, 0, 0, 0, NULL, 100, 0),
(770840, 14, -3220.4, 239.538, 203.19, 0, 0, 0, NULL, 100, 0),
(770840, 15, -3252.73, 239.407, 172.163, 0, 0, 0, NULL, 100, 0),
(770840, 16, -3266.67, 280.529, 161.968, 0, 0, 0, NULL, 100, 0),
(770840, 17, -3237.02, 300.281, 161.968, 0, 0, 0, NULL, 100, 0),
(770840, 18, -3205.82, 285.144, 183.413, 0, 0, 0, NULL, 100, 0),
(770840, 19, -3205.76, 262.642, 184.707, 0, 0, 0, NULL, 100, 0),
(770840, 20, -3213.83, 246.444, 194.429, 0, 0, 0, NULL, 100, 0),
(770840, 21, -3225.12, 246.817, 195.679, 0, 0, 0, NULL, 100, 0),
(770840, 22, -3240.03, 255.9, 201.579, 0, 0, 0, NULL, 100, 0),
(770840, 23, -3257.98, 269.678, 201.579, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (77084, 770840, 20684, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 77084);

-- Importing creature with guid 78783 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1077901 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1077901 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1077901 AND spawnType = 0 WHERE ce.spawnID = 1077901;
DELETE FROM waypoint_data WHERE id = 12949484;
INSERT INTO creature_entry (spawnID, entry) VALUES (78783, 22429);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (78783, 530, 1, 0, 0, -1668.65, 5531.65,  2.08439, 4.30748, 300, 0, 0, 5914, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(787830, 1, -1668.65, 5531.65, 2.08439, 0, 0, 0, NULL, 100, 0),
(787830, 2, -1675.54, 5539.93, 4.80874, 0, 0, 0, NULL, 100, 0),
(787830, 3, -1669.46, 5544.53, 5.41335, 0, 0, 0, NULL, 100, 0),
(787830, 4, -1656.4, 5534.61, 6.1091, 0, 0, 0, NULL, 100, 0),
(787830, 5, -1642.7, 5537.17, 6.17271, 0, 0, 0, NULL, 100, 0),
(787830, 6, -1641.73, 5532.27, 5.28943, 0, 0, 0, NULL, 100, 0),
(787830, 7, -1638.77, 5516.78, -3.27623, 0, 0, 0, NULL, 100, 0),
(787830, 8, -1633.4, 5489.19, -14.5382, 0, 0, 0, NULL, 100, 0),
(787830, 9, -1629.01, 5484.99, -14.4974, 0, 0, 0, NULL, 100, 0),
(787830, 10, -1624.56, 5486.95, -14.3688, 0, 0, 0, NULL, 100, 0),
(787830, 11, -1615.76, 5487.4, -14.0485, 0, 0, 0, NULL, 100, 0),
(787830, 12, -1606.83, 5480.52, -14.0989, 0, 0, 0, NULL, 100, 0),
(787830, 13, -1602.52, 5469.73, -14.0999, 0, 0, 0, NULL, 100, 0),
(787830, 14, -1604.27, 5462.54, -14.1001, 0, 0, 0, NULL, 100, 0),
(787830, 15, -1612.63, 5453.88, -14.0904, 0, 0, 0, NULL, 100, 0),
(787830, 16, -1623.32, 5451.61, -14.0784, 0, 0, 0, NULL, 100, 0),
(787830, 17, -1633.86, 5457.56, -14.1973, 0, 0, 0, NULL, 100, 0),
(787830, 18, -1637.26, 5469.76, -14.6911, 0, 0, 0, NULL, 100, 0),
(787830, 19, -1632.43, 5481.02, -14.5212, 0, 0, 0, NULL, 100, 0),
(787830, 20, -1637.26, 5469.76, -14.6911, 0, 0, 0, NULL, 100, 0),
(787830, 21, -1633.86, 5457.56, -14.1973, 0, 0, 0, NULL, 100, 0),
(787830, 22, -1623.32, 5451.61, -14.0784, 0, 0, 0, NULL, 100, 0),
(787830, 23, -1612.84, 5453.65, -14.09, 0, 0, 0, NULL, 100, 0),
(787830, 24, -1604.27, 5462.54, -14.1001, 0, 0, 0, NULL, 100, 0),
(787830, 25, -1602.52, 5469.73, -14.0999, 0, 0, 0, NULL, 100, 0),
(787830, 26, -1606.83, 5480.52, -14.0989, 0, 0, 0, NULL, 100, 0),
(787830, 27, -1615.76, 5487.4, -14.0485, 0, 0, 0, NULL, 100, 0),
(787830, 28, -1624.56, 5486.95, -14.3688, 0, 0, 0, NULL, 100, 0),
(787830, 29, -1629.01, 5484.99, -14.4974, 0, 0, 0, NULL, 100, 0),
(787830, 30, -1633.36, 5489, -14.5579, 0, 0, 0, NULL, 100, 0),
(787830, 31, -1638.77, 5516.78, -3.27623, 0, 0, 0, NULL, 100, 0),
(787830, 32, -1641.73, 5532.27, 5.28943, 0, 0, 0, NULL, 100, 0),
(787830, 33, -1642.7, 5537.17, 6.17271, 0, 0, 0, NULL, 100, 0),
(787830, 34, -1656.22, 5534.65, 6.13308, 0, 0, 0, NULL, 100, 0),
(787830, 35, -1669.46, 5544.53, 5.41335, 0, 0, 0, NULL, 100, 0),
(787830, 36, -1675.54, 5539.93, 4.80874, 0, 0, 0, NULL, 100, 0),
(787830, 37, -1668.77, 5531.67, 2.19737, 0, 0, 0, NULL, 100, 0),
(787830, 38, -1648.67, 5528.29, -5.18554, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (78783, 787830, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 81720 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1080771 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1080771 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1080771 AND spawnType = 0 WHERE ce.spawnID = 1080771;
INSERT INTO creature_entry (spawnID, entry) VALUES (81720, 16224);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (81720, 530, 1, 0, 1, 7638.23, -6842.24,  84.2546, 4.69922, 300, 0, 0, 233, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(817200, 1, 7638.23, -6842.24, 84.2546, 2.07694, 0, 0, NULL, 100, 0),
(817200, 2, 7636.98, -6839.96, 83.2448, 0, 0, 0, NULL, 100, 0),
(817200, 3, 7637.41, -6837.57, 82.8876, 0, 0, 0, NULL, 100, 0),
(817200, 4, 7639.8, -6835.54, 82.5945, 0, 0, 0, NULL, 100, 0),
(817200, 5, 7643.04, -6835.12, 82.5821, 0, 0, 0, NULL, 100, 0),
(817200, 6, 7644.66, -6837.57, 83.2017, 0, 0, 0, NULL, 100, 0),
(817200, 7, 7644.05, -6838.9, 83.9032, 0, 60000, 0, NULL, 100, 0),
(817200, 8, 7645.19, -6836.05, 83.1027, 0, 0, 0, NULL, 100, 0),
(817200, 9, 7642.42, -6834.73, 82.5953, 0, 0, 0, NULL, 100, 0),
(817200, 10, 7639.88, -6835.1, 82.5301, 0, 0, 0, NULL, 100, 0),
(817200, 11, 7637.52, -6837.24, 82.7524, 0, 0, 0, NULL, 100, 0),
(817200, 12, 7636.07, -6839.55, 82.9451, 0, 0, 0, NULL, 100, 0),
(817200, 13, 7637.41, -6842.98, 83.7097, 0, 10000, 0, NULL, 100, 0),
(817200, 14, 7638.23, -6842.24, 84.2546, 0, 0, 0, NULL, 100, 0),
(817200, 15, 7638.23, -6842.24, 84.2546, 2.07694, 60000, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (81720, 817200, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 84491 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1083512 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1083512 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1083512 AND spawnType = 0 WHERE ce.spawnID = 1083512;
DELETE FROM waypoint_data WHERE id = 12949511;
INSERT INTO creature_entry (spawnID, entry) VALUES (84491, 21501);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (84491, 530, 1, 0, 1, -3334.17, 2837.7,  135.984, 1.83354, 120, 0, 0, 26168, 0, 2, 0, 0, 10);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1135, 0, 1, 1, 0, 0, 0, 0, 0, 0);
INSERT INTO waypoint_data VALUES 
(844910, 1, -3299.58, 2868.22, 130.701, 0, 0, 0, NULL, 100, 0),
(844910, 2, -3289.77, 2916.61, 133.115, 0, 0, 0, NULL, 100, 0),
(844910, 3, -3252.38, 2948.88, 129.422, 0, 1000, 0, 1135, 100, 0),
(844910, 4, -3239.19, 2966.78, 129.884, 0, 0, 0, NULL, 100, 0),
(844910, 5, -3247.85, 2998.91, 136.634, 0, 0, 0, NULL, 100, 0),
(844910, 6, -3288.14, 2999.96, 144.092, 0, 1000, 0, 1135, 100, 0),
(844910, 7, -3308.65, 3031.86, 154.827, 0, 0, 0, NULL, 100, 0),
(844910, 8, -3303.04, 3041.18, 147.763, 0, 0, 0, NULL, 100, 0),
(844910, 9, -3292.48, 3052.73, 142.351, 0, 0, 0, NULL, 100, 0),
(844910, 10, -3278.9, 3068.09, 140.386, 0, 1000, 0, 1135, 100, 0),
(844910, 11, -3232.77, 3084.74, 133.397, 0, 0, 0, NULL, 100, 0),
(844910, 12, -3211.11, 3085.93, 127.147, 0, 0, 0, NULL, 100, 0),
(844910, 13, -3181.15, 3047.84, 115.059, 0, 1000, 0, 1135, 100, 0),
(844910, 14, -3145.66, 3017.09, 103.954, 0, 1000, 0, 1135, 100, 0),
(844910, 15, -3113.46, 2982.99, 94.0918, 0, 0, 0, NULL, 100, 0),
(844910, 16, -3121.42, 2950.28, 93.4686, 0, 1000, 0, 1135, 100, 0),
(844910, 17, -3135.4, 2932.67, 92.8892, 0, 0, 0, NULL, 100, 0),
(844910, 18, -3160.57, 2932.17, 98.0142, 0, 0, 0, NULL, 100, 0),
(844910, 19, -3164.64, 2901.16, 94.8892, 0, 1000, 0, 1135, 100, 0),
(844910, 20, -3178.99, 2878.3, 97.1525, 0, 0, 0, NULL, 100, 0),
(844910, 21, -3167.74, 2847.56, 89.4751, 0, 0, 0, NULL, 100, 0),
(844910, 22, -3167.84, 2814.7, 87.4386, 0, 1000, 0, 1135, 100, 0),
(844910, 23, -3187.91, 2805.05, 91.7393, 0, 0, 0, NULL, 100, 0),
(844910, 24, -3203.45, 2791.52, 98.9345, 0, 0, 0, NULL, 100, 0),
(844910, 25, -3218.1, 2792.45, 102.067, 0, 0, 0, NULL, 100, 0),
(844910, 26, -3232.44, 2798.1, 113.685, 0, 0, 0, NULL, 100, 0),
(844910, 27, -3246.54, 2797.87, 119.127, 0, 0, 0, NULL, 100, 0),
(844910, 28, -3272.88, 2795.56, 123.037, 0, 1000, 0, 1135, 100, 0),
(844910, 29, -3317.06, 2800.27, 123.121, 0, 0, 0, NULL, 100, 0),
(844910, 30, -3324.02, 2809.07, 123.047, 0, 0, 0, NULL, 100, 0),
(844910, 31, -3334.17, 2837.7, 135.984, 0, 1000, 0, 1135, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (84491, 844910, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 84491);

-- Importing creature with guid 85987 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1084762 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1084762 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1084762 AND spawnType = 0 WHERE ce.spawnID = 1084762;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1084765 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1084765 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1084765 AND spawnType = 0 WHERE ce.spawnID = 1084765;
INSERT INTO creature_entry (spawnID, entry) VALUES (85987, 19443);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (85987, 530, 1, 0, 1, -179.792, 2839.99,  23.6469, 5.36435, 120, 0, 0, 20632, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(859870, 1, -179.792, 2839.99, 23.6469, 0, 0, 0, NULL, 100, 0),
(859870, 2, -168.066, 2837.73, 26.3656, 0, 0, 0, NULL, 100, 0),
(859870, 3, -149.342, 2826.91, 33.6776, 0, 0, 0, NULL, 100, 0),
(859870, 4, -149.34, 2813.56, 35.254, 0, 0, 0, NULL, 100, 0),
(859870, 5, -160.443, 2807.62, 32.6923, 0, 0, 0, NULL, 100, 0),
(859870, 6, -172.671, 2803.53, 29.624, 0, 0, 0, NULL, 100, 0),
(859870, 7, -185.568, 2808.79, 25.5368, 0, 0, 0, NULL, 100, 0),
(859870, 8, -186.339, 2797.66, 26.3301, 0, 0, 0, NULL, 100, 0),
(859870, 9, -153.882, 2788.7, 36.8575, 0, 0, 0, NULL, 100, 0),
(859870, 10, -134.55, 2778.1, 41.0585, 0, 0, 0, NULL, 100, 0),
(859870, 11, -120.456, 2753.94, 49.374, 0, 0, 0, NULL, 100, 0),
(859870, 12, -110.122, 2737.95, 52.1691, 0, 0, 0, NULL, 100, 0),
(859870, 13, -108.798, 2709.02, 51.0045, 0, 0, 0, NULL, 100, 0),
(859870, 14, -125.753, 2695.99, 46.9254, 0, 0, 0, NULL, 100, 0),
(859870, 15, -165.775, 2686.86, 43.3931, 0, 0, 0, NULL, 100, 0),
(859870, 16, -166.924, 2686.5, 43.273, 0, 0, 0, NULL, 100, 0),
(859870, 17, -170.619, 2661.56, 42.248, 0, 0, 0, NULL, 100, 0),
(859870, 18, -166.914, 2633.8, 41.5524, 0, 0, 0, NULL, 100, 0),
(859870, 19, -165.275, 2618.52, 40.1223, 0, 0, 0, NULL, 100, 0),
(859870, 20, -164.524, 2612.24, 40.0821, 0, 0, 0, NULL, 100, 0),
(859870, 21, -162.776, 2603.1, 40.0698, 0, 0, 0, NULL, 100, 0),
(859870, 22, -165.048, 2590.72, 39.9081, 0, 0, 0, NULL, 100, 0),
(859870, 23, -144.798, 2569.51, 41.1089, 0, 0, 0, NULL, 100, 0),
(859870, 24, -146.141, 2544.73, 40.9807, 0, 0, 0, NULL, 100, 0),
(859870, 25, -156.852, 2539.49, 42.9517, 0, 0, 0, NULL, 100, 0),
(859870, 26, -170.491, 2527.82, 41.6219, 0, 0, 0, NULL, 100, 0),
(859870, 27, -177.336, 2516.9, 41.1118, 0, 0, 0, NULL, 100, 0),
(859870, 28, -176.385, 2510.59, 41.9498, 0, 0, 0, NULL, 100, 0),
(859870, 29, -168.844, 2501.02, 43.7949, 0, 0, 0, NULL, 100, 0),
(859870, 30, -145.847, 2499.76, 45.6119, 0, 0, 0, NULL, 100, 0),
(859870, 31, -131.441, 2496, 46.5555, 0, 0, 0, NULL, 100, 0),
(859870, 32, -124.868, 2492.81, 46.937, 0, 0, 0, NULL, 100, 0),
(859870, 33, -118.393, 2470.39, 46.9573, 0, 0, 0, NULL, 100, 0),
(859870, 34, -134.591, 2450.62, 46.1898, 0, 0, 0, NULL, 100, 0),
(859870, 35, -139.636, 2439.6, 48.6551, 0, 0, 0, NULL, 100, 0),
(859870, 36, -153.073, 2420.76, 44.3387, 0, 0, 0, NULL, 100, 0),
(859870, 37, -164.315, 2415.92, 48.3661, 0, 0, 0, NULL, 100, 0),
(859870, 38, -176.028, 2408.14, 48.9576, 0, 0, 0, NULL, 100, 0),
(859870, 39, -181.309, 2397.23, 50.1657, 0, 0, 0, NULL, 100, 0),
(859870, 40, -174.265, 2386.54, 51.8884, 0, 0, 0, NULL, 100, 0),
(859870, 41, -171.309, 2382.06, 53.208, 0, 0, 0, NULL, 100, 0),
(859870, 42, -168.726, 2366.75, 56.1528, 0, 0, 0, NULL, 100, 0),
(859870, 43, -168.079, 2352.93, 58.4996, 0, 0, 0, NULL, 100, 0),
(859870, 44, -168.341, 2341.55, 60.146, 0, 0, 0, NULL, 100, 0),
(859870, 45, -175.721, 2327.79, 62.1515, 0, 0, 0, NULL, 100, 0),
(859870, 46, -193.092, 2317.19, 56.1099, 0, 0, 0, NULL, 100, 0),
(859870, 47, -211.19, 2312.89, 49.6796, 0, 0, 0, NULL, 100, 0),
(859870, 48, -243.001, 2311.8, 51.3676, 0, 0, 0, NULL, 100, 0),
(859870, 49, -257.705, 2324.3, 56.9862, 0, 0, 0, NULL, 100, 0),
(859870, 50, -267.846, 2334.5, 54.922, 0, 0, 0, NULL, 100, 0),
(859870, 51, -284.024, 2347.27, 51.854, 0, 0, 0, NULL, 100, 0),
(859870, 52, -285.592, 2348.85, 51.3028, 0, 0, 0, NULL, 100, 0),
(859870, 53, -277.639, 2384.37, 49.047, 0, 0, 0, NULL, 100, 0),
(859870, 54, -277.421, 2389, 49.5565, 0, 0, 0, NULL, 100, 0),
(859870, 55, -282.473, 2415.65, 47.1102, 0, 0, 0, NULL, 100, 0),
(859870, 56, -290.638, 2417.86, 45.7617, 0, 0, 0, NULL, 100, 0),
(859870, 57, -296.991, 2425.08, 44.6878, 0, 0, 0, NULL, 100, 0),
(859870, 58, -297.409, 2429.13, 44.2896, 0, 0, 0, NULL, 100, 0),
(859870, 59, -292.818, 2446.39, 42.9986, 0, 0, 0, NULL, 100, 0),
(859870, 60, -297.474, 2458.02, 41.8229, 0, 0, 0, NULL, 100, 0),
(859870, 61, -299.085, 2461.29, 41.3023, 0, 0, 0, NULL, 100, 0),
(859870, 62, -291.531, 2472.15, 40.8264, 0, 0, 0, NULL, 100, 0),
(859870, 63, -315.733, 2479.06, 38.8889, 0, 0, 0, NULL, 100, 0),
(859870, 64, -333.155, 2479.06, 30.4347, 0, 0, 0, NULL, 100, 0),
(859870, 65, -362.027, 2481.3, 26.7681, 0, 0, 0, NULL, 100, 0),
(859870, 66, -381.832, 2489.02, 35.4477, 0, 0, 0, NULL, 100, 0),
(859870, 67, -383.683, 2499.09, 43.0709, 0, 0, 0, NULL, 100, 0),
(859870, 68, -373.025, 2518.31, 44.5293, 0, 0, 0, NULL, 100, 0),
(859870, 69, -356.454, 2524.72, 43.928, 0, 0, 0, NULL, 100, 0),
(859870, 70, -341.865, 2517.72, 42.6987, 0, 0, 0, NULL, 100, 0),
(859870, 71, -325.487, 2514.35, 39.746, 0, 0, 0, NULL, 100, 0),
(859870, 72, -317.691, 2517.74, 42.2765, 0, 0, 0, NULL, 100, 0),
(859870, 73, -307.929, 2522.57, 42.4715, 0, 0, 0, NULL, 100, 0),
(859870, 74, -303.355, 2545.35, 42.8876, 0, 0, 0, NULL, 100, 0),
(859870, 75, -313.627, 2562.33, 44.0172, 0, 0, 0, NULL, 100, 0),
(859870, 76, -319.815, 2564.86, 44.6563, 0, 0, 0, NULL, 100, 0),
(859870, 77, -315.39, 2592.44, 41.6054, 0, 0, 0, NULL, 100, 0),
(859870, 78, -315.032, 2593.06, 41.3715, 0, 0, 0, NULL, 100, 0),
(859870, 79, -333.152, 2616.68, 42.4043, 0, 0, 0, NULL, 100, 0),
(859870, 80, -352.777, 2633.74, 40.4505, 0, 0, 0, NULL, 100, 0),
(859870, 81, -360.199, 2656.17, 43.7213, 0, 0, 0, NULL, 100, 0),
(859870, 82, -344.704, 2680.91, 36.1579, 0, 0, 0, NULL, 100, 0),
(859870, 83, -334.635, 2700.04, 30.3424, 0, 0, 0, NULL, 100, 0),
(859870, 84, -324.906, 2712.12, 26.4365, 0, 0, 0, NULL, 100, 0),
(859870, 85, -322.978, 2724.36, 23.9624, 0, 0, 0, NULL, 100, 0),
(859870, 86, -326.352, 2734.54, 22.4438, 0, 0, 0, NULL, 100, 0),
(859870, 87, -342.591, 2742.29, 22.4451, 0, 0, 0, NULL, 100, 0),
(859870, 88, -368.749, 2739.92, 27.6375, 0, 0, 0, NULL, 100, 0),
(859870, 89, -392.21, 2734, 34.5519, 0, 0, 0, NULL, 100, 0),
(859870, 90, -414.431, 2741.14, 39.5583, 0, 0, 0, NULL, 100, 0),
(859870, 91, -436.194, 2742.23, 45.3562, 0, 0, 0, NULL, 100, 0),
(859870, 92, -462.982, 2756.81, 49.8581, 0, 0, 0, NULL, 100, 0),
(859870, 93, -480.594, 2778.59, 50.4792, 0, 0, 0, NULL, 100, 0),
(859870, 94, -476.001, 2799.57, 45.9565, 0, 0, 0, NULL, 100, 0),
(859870, 95, -458.507, 2835.35, 35.3313, 0, 0, 0, NULL, 100, 0),
(859870, 96, -431.757, 2863.31, 23.7476, 0, 0, 0, NULL, 100, 0),
(859870, 97, -413.422, 2892.73, 17.2071, 0, 0, 0, NULL, 100, 0),
(859870, 98, -419.229, 2905.47, 19.5118, 0, 0, 0, NULL, 100, 0),
(859870, 99, -443.146, 2933.89, 17.852, 0, 0, 0, NULL, 100, 0),
(859870, 100, -438.84, 2966.17, 8.72354, 0, 0, 0, NULL, 100, 0),
(859870, 101, -444.043, 2936.33, 17.4072, 0, 0, 0, NULL, 100, 0),
(859870, 102, -432.581, 2918.56, 19.3152, 0, 0, 0, NULL, 100, 0),
(859870, 103, -411.357, 2898.61, 17.2336, 0, 0, 0, NULL, 100, 0),
(859870, 104, -429.098, 2867.21, 22.665, 0, 0, 0, NULL, 100, 0),
(859870, 105, -457.427, 2836.47, 34.7506, 0, 0, 0, NULL, 100, 0),
(859870, 106, -474.191, 2804.11, 44.8159, 0, 0, 0, NULL, 100, 0),
(859870, 107, -481.076, 2783.36, 50.0908, 0, 0, 0, NULL, 100, 0),
(859870, 108, -465.552, 2759.48, 50.0374, 0, 0, 0, NULL, 100, 0),
(859870, 109, -440.541, 2744.55, 46.125, 0, 0, 0, NULL, 100, 0),
(859870, 110, -420.084, 2742.42, 41.1034, 0, 0, 0, NULL, 100, 0),
(859870, 111, -397.944, 2734.92, 35.902, 0, 0, 0, NULL, 100, 0),
(859870, 112, -372.986, 2738.61, 28.6479, 0, 0, 0, NULL, 100, 0),
(859870, 113, -348.446, 2742.21, 23.4238, 0, 0, 0, NULL, 100, 0),
(859870, 114, -333.768, 2740.03, 21.8141, 0, 0, 0, NULL, 100, 0),
(859870, 115, -324.904, 2731.12, 22.8369, 0, 0, 0, NULL, 100, 0),
(859870, 116, -323.997, 2714.82, 25.2085, 0, 0, 0, NULL, 100, 0),
(859870, 117, -332.706, 2702.87, 29.4334, 0, 0, 0, NULL, 100, 0),
(859870, 118, -343.423, 2683.02, 35.1899, 0, 0, 0, NULL, 100, 0),
(859870, 119, -359.622, 2658.23, 43.5518, 0, 0, 0, NULL, 100, 0),
(859870, 120, -353.887, 2635.82, 39.4861, 0, 0, 0, NULL, 100, 0),
(859870, 121, -344.057, 2624.18, 43.1001, 0, 0, 0, NULL, 100, 0),
(859870, 122, -325.477, 2613.26, 41.9975, 0, 0, 0, NULL, 100, 0),
(859870, 123, -315.01, 2608.49, 41.1963, 0, 0, 0, NULL, 100, 0),
(859870, 124, -325.072, 2578.15, 44.6071, 0, 0, 0, NULL, 100, 0),
(859870, 125, -325.684, 2576.81, 44.8529, 0, 0, 0, NULL, 100, 0),
(859870, 126, -313.842, 2562.75, 43.8663, 0, 0, 0, NULL, 100, 0),
(859870, 127, -306.63, 2559.59, 43.3148, 0, 0, 0, NULL, 100, 0),
(859870, 128, -303.079, 2544.06, 42.3024, 0, 0, 0, NULL, 100, 0),
(859870, 129, -324.127, 2514.44, 40.2918, 0, 0, 0, NULL, 100, 0),
(859870, 130, -333.202, 2515.88, 41.902, 0, 0, 0, NULL, 100, 0),
(859870, 131, -354.449, 2524.19, 43.3304, 0, 0, 0, NULL, 100, 0),
(859870, 132, -359.78, 2523.49, 44.5311, 0, 0, 0, NULL, 100, 0),
(859870, 133, -378.132, 2510.01, 43.7447, 0, 0, 0, NULL, 100, 0),
(859870, 134, -382.583, 2491, 37.3352, 0, 0, 0, NULL, 100, 0),
(859870, 135, -368.784, 2483.06, 28.0928, 0, 0, 0, NULL, 100, 0),
(859870, 136, -337.769, 2479.35, 28.9518, 0, 0, 0, NULL, 100, 0),
(859870, 137, -318.876, 2478.92, 37.7012, 0, 0, 0, NULL, 100, 0),
(859870, 138, -301.946, 2480.98, 40.5145, 0, 0, 0, NULL, 100, 0),
(859870, 139, -300.034, 2481.13, 40.5369, 0, 0, 0, NULL, 100, 0),
(859870, 140, -291.654, 2471.88, 40.9113, 0, 0, 0, NULL, 100, 0),
(859870, 141, -295.273, 2454.31, 42.0707, 0, 0, 0, NULL, 100, 0),
(859870, 142, -291.917, 2442.61, 43.5853, 0, 0, 0, NULL, 100, 0),
(859870, 143, -290.83, 2436.46, 44.0534, 0, 0, 0, NULL, 100, 0),
(859870, 144, -296.628, 2422.13, 44.7556, 0, 0, 0, NULL, 100, 0),
(859870, 145, -284.936, 2416.83, 46.5253, 0, 0, 0, NULL, 100, 0),
(859870, 146, -277.901, 2404.4, 49.4697, 0, 0, 0, NULL, 100, 0),
(859870, 147, -276.788, 2401.74, 49.5749, 0, 0, 0, NULL, 100, 0),
(859870, 148, -275.867, 2368.15, 50.0218, 0, 0, 0, NULL, 100, 0),
(859870, 149, -280.12, 2358.43, 51.4204, 0, 0, 0, NULL, 100, 0),
(859870, 150, -271.663, 2337.66, 53.9042, 0, 0, 0, NULL, 100, 0),
(859870, 151, -260.852, 2327.6, 56.1625, 0, 0, 0, NULL, 100, 0),
(859870, 152, -246.725, 2313.88, 53.0292, 0, 0, 0, NULL, 100, 0),
(859870, 153, -233.433, 2311.11, 50.2891, 0, 0, 0, NULL, 100, 0),
(859870, 154, -194.846, 2316.22, 54.1394, 0, 0, 0, NULL, 100, 0),
(859870, 155, -184.322, 2321.59, 61.9018, 0, 0, 0, NULL, 100, 0),
(859870, 156, -169.679, 2336.9, 61.2792, 0, 0, 0, NULL, 100, 0),
(859870, 157, -167.838, 2348.53, 59.1513, 0, 0, 0, NULL, 100, 0),
(859870, 158, -168.297, 2362.32, 57.0196, 0, 0, 0, NULL, 100, 0),
(859870, 159, -169.383, 2376.63, 54.4596, 0, 0, 0, NULL, 100, 0),
(859870, 160, -171.814, 2382.75, 52.9755, 0, 0, 0, NULL, 100, 0),
(859870, 161, -180.183, 2392.34, 51.0149, 0, 0, 0, NULL, 100, 0),
(859870, 162, -181.274, 2398.44, 49.9924, 0, 0, 0, NULL, 100, 0),
(859870, 163, -175.461, 2408.53, 48.8857, 0, 0, 0, NULL, 100, 0),
(859870, 164, -157.514, 2417.72, 45.0386, 0, 0, 0, NULL, 100, 0),
(859870, 165, -146.333, 2426.92, 48.3031, 0, 0, 0, NULL, 100, 0),
(859870, 166, -137.96, 2445.38, 46.1259, 0, 0, 0, NULL, 100, 0),
(859870, 167, -118.393, 2470.39, 46.9573, 0, 0, 0, NULL, 100, 0),
(859870, 168, -124.868, 2492.81, 46.937, 0, 0, 0, NULL, 100, 0),
(859870, 169, -131.441, 2496, 46.5555, 0, 0, 0, NULL, 100, 0),
(859870, 170, -145.847, 2499.76, 45.6119, 0, 0, 0, NULL, 100, 0),
(859870, 171, -168.844, 2501.02, 43.7949, 0, 0, 0, NULL, 100, 0),
(859870, 172, -176.385, 2510.59, 41.9498, 0, 0, 0, NULL, 100, 0),
(859870, 173, -177.336, 2516.9, 41.1118, 0, 0, 0, NULL, 100, 0),
(859870, 174, -170.491, 2527.82, 41.6219, 0, 0, 0, NULL, 100, 0),
(859870, 175, -156.852, 2539.49, 42.9517, 0, 0, 0, NULL, 100, 0),
(859870, 176, -146.141, 2544.73, 40.9807, 0, 0, 0, NULL, 100, 0),
(859870, 177, -144.798, 2569.51, 41.1089, 0, 0, 0, NULL, 100, 0),
(859870, 178, -165.048, 2590.72, 39.9081, 0, 0, 0, NULL, 100, 0),
(859870, 179, -162.776, 2603.1, 40.0698, 0, 0, 0, NULL, 100, 0),
(859870, 180, -164.524, 2612.24, 40.0821, 0, 0, 0, NULL, 100, 0),
(859870, 181, -165.275, 2618.52, 40.1223, 0, 0, 0, NULL, 100, 0),
(859870, 182, -166.914, 2633.8, 41.5524, 0, 0, 0, NULL, 100, 0),
(859870, 183, -170.619, 2661.56, 42.248, 0, 0, 0, NULL, 100, 0),
(859870, 184, -166.924, 2686.5, 43.273, 0, 0, 0, NULL, 100, 0),
(859870, 185, -165.775, 2686.86, 43.3931, 0, 0, 0, NULL, 100, 0),
(859870, 186, -125.753, 2695.99, 46.9254, 0, 0, 0, NULL, 100, 0),
(859870, 187, -108.798, 2709.02, 51.0045, 0, 0, 0, NULL, 100, 0),
(859870, 188, -110.122, 2737.95, 52.1691, 0, 0, 0, NULL, 100, 0),
(859870, 189, -120.456, 2753.94, 49.374, 0, 0, 0, NULL, 100, 0),
(859870, 190, -134.55, 2778.1, 41.0585, 0, 0, 0, NULL, 100, 0),
(859870, 191, -153.882, 2788.7, 36.8575, 0, 0, 0, NULL, 100, 0),
(859870, 192, -186.339, 2797.66, 26.3301, 0, 0, 0, NULL, 100, 0),
(859870, 193, -185.568, 2808.79, 25.5368, 0, 0, 0, NULL, 100, 0),
(859870, 194, -172.671, 2803.53, 29.624, 0, 0, 0, NULL, 100, 0),
(859870, 195, -160.443, 2807.62, 32.6923, 0, 0, 0, NULL, 100, 0),
(859870, 196, -149.34, 2813.56, 35.254, 0, 0, 0, NULL, 100, 0),
(859870, 197, -149.342, 2826.91, 33.6776, 0, 0, 0, NULL, 100, 0),
(859870, 198, -168.066, 2837.73, 26.3656, 0, 0, 0, NULL, 100, 0),
(859870, 199, -179.792, 2839.99, 23.6469, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (85987, 859870, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 3416 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1003395 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1003395 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1003395 AND spawnType = 0 WHERE ce.spawnID = 1003395;
INSERT INTO creature_entry (spawnID, entry) VALUES (3416, 11943);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (3416, 1, 1, 11901, 0, 327.574, -4682.93,  16.4578, 5.8253, 200, 0, 0, 102, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (3416, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 86493 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1085212 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1085212 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1085212 AND spawnType = 0 WHERE ce.spawnID = 1085212;
INSERT INTO creature_entry (spawnID, entry) VALUES (86493, 22024);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86493, 530, 1, 0, 0, -3477.82, 2277.81,  64.315, 5.38, 1524, 0, 0, 5233, 2991, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(864930, 1, -3501.24, 2277.36, 65.8561, 0, 0, 0, NULL, 100, 0),
(864930, 2, -3477.82, 2277.81, 64.315, 0, 0, 0, NULL, 100, 0),
(864930, 3, -3468.63, 2285.68, 63.5787, 0, 0, 0, NULL, 100, 0),
(864930, 4, -3455.3, 2289.51, 63.4889, 0, 0, 0, NULL, 100, 0),
(864930, 5, -3387.99, 2289.7, 62.3754, 0, 0, 0, NULL, 100, 0),
(864930, 6, -3314.11, 2279.18, 60.8337, 0, 0, 0, NULL, 100, 0),
(864930, 7, -3387.99, 2289.7, 62.3754, 0, 0, 0, NULL, 100, 0),
(864930, 8, -3455.3, 2289.51, 63.4889, 0, 0, 0, NULL, 100, 0),
(864930, 9, -3468.63, 2285.68, 63.5787, 0, 0, 0, NULL, 100, 0),
(864930, 10, -3477.82, 2277.81, 64.315, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (86493, 864930, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 86748 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1085465 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1085465 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1085465 AND spawnType = 0 WHERE ce.spawnID = 1085465;
DELETE FROM waypoint_data WHERE id = 1823;
INSERT INTO creature_entry (spawnID, entry) VALUES (86748, 20931);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86748, 530, 1, 0, 0, 5100.75, 2970.8,  80.0953, 0.130239, 160, 0, 0, 9335, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(867480, 1, 5067.6, 2955.68, 86.538, 0, 0, 0, NULL, 100, 0),
(867480, 2, 5042.52, 2933.23, 89.3986, 0, 0, 0, NULL, 100, 0),
(867480, 3, 5050.04, 2900.66, 84.3986, 0, 0, 0, NULL, 100, 0),
(867480, 4, 5083.47, 2888.87, 71.6332, 0, 0, 0, NULL, 100, 0),
(867480, 5, 5112.22, 2900.02, 76.8735, 0, 0, 0, NULL, 100, 0),
(867480, 6, 5146.69, 2905.61, 69.4368, 0, 0, 0, NULL, 100, 0),
(867480, 7, 5175.72, 2898.99, 61.4337, 0, 0, 0, NULL, 100, 0),
(867480, 8, 5179.67, 2919.2, 64.8221, 0, 0, 0, NULL, 100, 0),
(867480, 9, 5149.78, 2940.12, 75.5148, 0, 0, 0, NULL, 100, 0),
(867480, 10, 5139.56, 2946.87, 71.6715, 0, 0, 0, NULL, 100, 0),
(867480, 11, 5129.33, 2954.33, 78.97, 0, 0, 0, NULL, 100, 0),
(867480, 12, 5109.65, 2965.54, 83.2212, 0, 0, 0, NULL, 100, 0),
(867480, 13, 5100.75, 2970.8, 80.0953, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (86748, 867480, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 86748);

-- Importing creature with guid 86751 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (86751, 19556);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86751, 530, 1, 0, 1, -1304.1, 6954.56,  31.9313, 1.03594, 8238, 0, 0, 6070000, 1016100, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(867510, 1, -1291.84, 6977.11, 32.5516, 0, 0, 0, NULL, 100, 0),
(867510, 2, -1259.66, 6983.27, 36.6296, 0, 0, 0, NULL, 100, 0),
(867510, 3, -1247.23, 7006.4, 36.223, 0, 0, 0, NULL, 100, 0),
(867510, 4, -1230.71, 7071.58, 35.7016, 0, 0, 0, NULL, 100, 0),
(867510, 5, -1231.08, 7115.72, 55.8359, 0, 0, 0, NULL, 100, 0),
(867510, 6, -1230.2, 7130.87, 57.2655, 0, 0, 0, NULL, 100, 0),
(867510, 7, -1216.36, 7138.53, 57.2655, 0, 0, 0, NULL, 100, 0),
(867510, 8, -1215.14, 7135.89, 57.2655, 0, 100, 0, NULL, 100, 0),
(867510, 9, -1225.93, 7136.47, 57.2655, 0, 0, 0, NULL, 100, 0),
(867510, 10, -1233.04, 7122.99, 57.0135, 0, 0, 0, NULL, 100, 0),
(867510, 11, -1229.82, 7070.42, 35.9467, 0, 0, 0, NULL, 100, 0),
(867510, 12, -1252.88, 6987.42, 37.1017, 0, 0, 0, NULL, 100, 0),
(867510, 13, -1289.45, 6977.98, 32.8316, 0, 0, 0, NULL, 100, 0),
(867510, 14, -1308.59, 6947.6, 31.4855, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (86751, 867510, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 86754 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1085469 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1085469 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1085469 AND spawnType = 0 WHERE ce.spawnID = 1085469;
DELETE FROM waypoint_data WHERE id = 1825;
INSERT INTO creature_entry (spawnID, entry) VALUES (86754, 22357);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86754, 530, 1, 0, 0, -1470.79, 9695.28,  201.915, 2.14272, 4242, 0, 0, 151750, 13548, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(867540, 1, -1470.79, 9695.28, 201.915, 0, 0, 0, NULL, 100, 0),
(867540, 2, -1463.57, 9667.4, 201.119, 0, 0, 0, NULL, 100, 0),
(867540, 3, -1435.97, 9657.15, 201.447, 0, 0, 0, NULL, 100, 0),
(867540, 4, -1413.26, 9667.89, 200.982, 0, 0, 0, NULL, 100, 0),
(867540, 5, -1402.25, 9689.2, 200.954, 0, 0, 0, NULL, 100, 0),
(867540, 6, -1414.64, 9716.7, 202.204, 0, 0, 0, NULL, 100, 0),
(867540, 7, -1441.11, 9724.78, 200.454, 0, 0, 0, NULL, 100, 0),
(867540, 8, -1461.79, 9742.57, 200.447, 0, 0, 0, NULL, 100, 0),
(867540, 9, -1469.65, 9771.82, 199.987, 0, 0, 0, NULL, 100, 0),
(867540, 10, -1469.75, 9806.64, 201.135, 0, 0, 0, NULL, 100, 0),
(867540, 11, -1502.84, 9806.93, 199.243, 0, 0, 0, NULL, 100, 0),
(867540, 12, -1514.05, 9782.88, 199.05, 0, 0, 0, NULL, 100, 0),
(867540, 13, -1507.74, 9758.24, 200.876, 0, 0, 0, NULL, 100, 0),
(867540, 14, -1497.91, 9735.08, 200.83, 0, 0, 0, NULL, 100, 0),
(867540, 15, -1479.92, 9715.55, 200.603, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (86754, 867540, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 42662 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (42662, 27489);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (42662, 1, 1, 0, 0, 1472.61, -4209.17,  43.2693, 4.43314, 600, 0, 0, 7500, 1, 0, 0, 0, 10);

-- Importing creature with guid 28796 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (28796, 23283);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (28796, 530, 1, 0, 0, -5240.3, 682.02,  157.604, 5.72309, 900, 0, 0, 151760, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(232830, 1, -5120.74, 783.188, 208.115, 0, 0, 0, NULL, 100, 0),
(232830, 2, -5331.21, 712.656, 180.571, 0, 0, 0, NULL, 100, 0),
(232830, 3, -5371.6, 573.187, 159.435, 0, 0, 0, NULL, 100, 0),
(232830, 4, -5299.81, 490.375, 150.087, 0, 0, 0, NULL, 100, 0),
(232830, 5, -5207.27, 478.923, 150.909, 0, 0, 0, NULL, 100, 0),
(232830, 6, -5170.65, 533.438, 168.777, 0, 0, 0, NULL, 100, 0),
(232830, 7, -5132.27, 632.126, 177.019, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (28796, 232830, 0, 0, 33554432, 1, 0, NULL);

-- Importing creature with guid 33909 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1033769 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1033769 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1033769 AND spawnType = 0 WHERE ce.spawnID = 1033769;
INSERT INTO creature_entry (spawnID, entry) VALUES (33909, 4841);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (33909, 1, 1, 0, 0, -3949.69, -3471.04,  29.1544, 4.28802, 360, 0, 0, 1902, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(339090, 1, -3942.97, -3455.51, 28.8974, 0, 0, 0, NULL, 0, 0),
(339090, 2, -3943.12, -3456.5, 28.8974, 0, 0, 0, NULL, 0, 0),
(339090, 3, -3948.06, -3467.44, 29.6474, 0, 0, 0, NULL, 0, 0),
(339090, 4, -3950.12, -3472, 29.0224, 0, 0, 0, NULL, 0, 0),
(339090, 5, -3953.3, -3479.05, 28.6474, 0, 0, 0, NULL, 0, 0),
(339090, 6, -3953.3, -3479.05, 28.6474, 0, 0, 0, NULL, 0, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (33909, 339090, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 33909);

-- Importing creature with guid 51821 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1051520 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1051520 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1051520 AND spawnType = 0 WHERE ce.spawnID = 1051520;
INSERT INTO creature_entry (spawnID, entry) VALUES (51821, 5934);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (51821, 1, 1, 0, 0, -5183.81, -1170.22,  45.1108, 4.18436, 43200, 0, 0, 3171, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(518210, 1, -5183.81, -1170.22, 45.1108, 0, 0, 0, NULL, 100, 0),
(518210, 2, -5181.06, -1184.85, 45.9799, 0, 0, 0, NULL, 100, 0),
(518210, 3, -5169.91, -1200.51, 47.1527, 0, 0, 0, NULL, 100, 0),
(518210, 4, -5160, -1216.5, 48.3812, 0, 0, 0, NULL, 100, 0),
(518210, 5, -5173.08, -1229.05, 49.0484, 0, 0, 0, NULL, 100, 0),
(518210, 6, -5194.94, -1230.64, 48.4713, 0, 0, 0, NULL, 100, 0),
(518210, 7, -5207.13, -1229.34, 47.9261, 0, 0, 0, NULL, 100, 0),
(518210, 8, -5215.48, -1216.02, 46.7002, 0, 0, 0, NULL, 100, 0),
(518210, 9, -5207.34, -1200.79, 46.1684, 0, 0, 0, NULL, 100, 0),
(518210, 10, -5184.16, -1170.53, 45.1217, 0, 0, 0, NULL, 100, 0),
(518210, 11, -5183.29, -1153.09, 44.8788, 0, 0, 0, NULL, 100, 0),
(518210, 12, -5183.29, -1153.09, 44.8788, 0, 0, 0, NULL, 100, 0),
(518210, 13, -5177.14, -1130.76, 43.7756, 0, 0, 0, NULL, 100, 0),
(518210, 14, -5164.94, -1121.57, 43.7702, 0, 0, 0, NULL, 100, 0),
(518210, 15, -5145.43, -1115.09, 43.7495, 0, 0, 0, NULL, 100, 0),
(518210, 16, -5138.18, -1109.64, 44.0435, 0, 0, 0, NULL, 100, 0),
(518210, 17, -5154.78, -1119.31, 43.673, 0, 0, 0, NULL, 100, 0),
(518210, 18, -5176.4, -1127.44, 43.7173, 0, 0, 0, NULL, 100, 0),
(518210, 19, -5183.81, -1149.32, 44.3443, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (51821, 518210, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 51809 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1051508 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1051508 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1051508 AND spawnType = 0 WHERE ce.spawnID = 1051508;
INSERT INTO creature_entry (spawnID, entry) VALUES (51809, 3295);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (51809, 1, 1, 0, 0, 1071.4, -3135.26,  67.39, 3.63731, 9900, 0, 0, 449, 0, 0, 0, 0, 10);

-- Importing creature with guid 51808 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1051507 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1051507 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1051507 AND spawnType = 0 WHERE ce.spawnID = 1051507;
INSERT INTO creature_entry (spawnID, entry) VALUES (51808, 5835);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (51808, 1, 1, 0, 1, 1060.82, -3089.71,  105.229, 4.64184, 9900, 0, 0, 314, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(518080, 1, 1061.99, -3086.34, 105.125, 0, 0, 0, NULL, 100, 0),
(518080, 2, 1061.63, -3082.63, 105.125, 0, 0, 0, NULL, 100, 0),
(518080, 3, 1060.18, -3079.97, 105.125, 0, 0, 0, NULL, 100, 0),
(518080, 4, 1058.07, -3075.87, 105.501, 0, 0, 0, NULL, 100, 0),
(518080, 5, 1054.89, -3071.72, 105.125, 0, 0, 0, NULL, 100, 0),
(518080, 6, 1051.25, -3069.85, 105.125, 0, 0, 0, NULL, 100, 0),
(518080, 7, 1047.29, -3069.88, 105.289, 0, 0, 0, NULL, 100, 0),
(518080, 8, 1043.63, -3072.36, 105.72, 0, 0, 0, NULL, 100, 0),
(518080, 9, 1038.3, -3076.43, 105.328, 0, 0, 0, NULL, 100, 0),
(518080, 10, 1033.59, -3075.67, 105.125, 0, 0, 0, NULL, 100, 0),
(518080, 11, 1031.4, -3070.74, 105.125, 0, 0, 0, NULL, 100, 0),
(518080, 12, 1030.16, -3066.55, 105.089, 0, 0, 0, NULL, 100, 0),
(518080, 13, 1031.58, -3060.08, 102.155, 0, 0, 0, NULL, 100, 0),
(518080, 14, 1032.79, -3056.73, 100.41, 0, 0, 0, NULL, 100, 0),
(518080, 15, 1035.06, -3054.11, 98.9656, 0, 0, 0, NULL, 100, 0),
(518080, 16, 1039.28, -3052.89, 98.4566, 0, 0, 0, NULL, 100, 0),
(518080, 17, 1043.78, -3054.49, 96.88, 0, 0, 0, NULL, 100, 0),
(518080, 18, 1049.61, -3058.21, 93.9174, 0, 0, 0, NULL, 100, 0),
(518080, 19, 1053.44, -3062.93, 91.8125, 0, 0, 0, NULL, 100, 0),
(518080, 20, 1055.31, -3067.91, 91.7175, 0, 0, 0, NULL, 100, 0),
(518080, 21, 1057.02, -3073.87, 91.7479, 0, 0, 0, NULL, 100, 0),
(518080, 22, 1058.28, -3077.77, 91.7453, 0, 0, 0, NULL, 100, 0),
(518080, 23, 1059.49, -3082.52, 91.7453, 0, 0, 0, NULL, 100, 0),
(518080, 24, 1059.71, -3086.14, 91.7453, 0, 0, 0, NULL, 100, 0),
(518080, 25, 1054.99, -3091.56, 89.7105, 0, 0, 0, NULL, 100, 0),
(518080, 26, 1051.96, -3093.6, 87.6199, 0, 0, 0, NULL, 100, 0),
(518080, 27, 1047.68, -3095.38, 84.8705, 0, 0, 0, NULL, 100, 0),
(518080, 28, 1043.32, -3097.01, 82.0536, 0, 0, 0, NULL, 100, 0),
(518080, 29, 1039.55, -3097.98, 80.947, 0, 0, 0, NULL, 100, 0),
(518080, 30, 1032.46, -3100.74, 80.947, 0, 0, 0, NULL, 100, 0),
(518080, 31, 1028.6, -3104.47, 80.9988, 0, 0, 0, NULL, 100, 0),
(518080, 32, 1025.1, -3106.79, 80.9988, 0, 0, 0, NULL, 100, 0),
(518080, 33, 1021.45, -3109.21, 80.9988, 0, 0, 0, NULL, 100, 0),
(518080, 34, 1017.71, -3109.38, 80.8207, 0, 0, 0, NULL, 100, 0),
(518080, 35, 1017.25, -3113.85, 80.9386, 0, 0, 0, NULL, 100, 0),
(518080, 36, 1015.24, -3116.81, 81.2594, 0, 0, 0, NULL, 100, 0),
(518080, 37, 1013.11, -3118.45, 81.7116, 0, 0, 0, NULL, 100, 0),
(518080, 38, 1010.26, -3117.7, 81.9643, 0, 0, 0, NULL, 100, 0),
(518080, 39, 1009.36, -3115.54, 81.937, 0, 0, 0, NULL, 100, 0),
(518080, 40, 1009.97, -3111.18, 81.4586, 0, 0, 0, NULL, 100, 0),
(518080, 41, 1012.35, -3109.55, 80.8596, 0, 0, 0, NULL, 100, 0),
(518080, 42, 1013.99, -3110.13, 80.9735, 0, 0, 0, NULL, 100, 0),
(518080, 43, 1019.78, -3109.27, 81.4185, 0, 0, 0, NULL, 100, 0),
(518080, 44, 1023.76, -3107.68, 80.9988, 0, 0, 0, NULL, 100, 0),
(518080, 45, 1026.93, -3105.58, 80.9988, 0, 0, 0, NULL, 100, 0),
(518080, 46, 1036.32, -3099.22, 80.947, 0, 0, 0, NULL, 100, 0),
(518080, 47, 1043.88, -3096.5, 82.475, 0, 0, 0, NULL, 100, 0),
(518080, 48, 1048.61, -3095.18, 85.4422, 0, 0, 0, NULL, 100, 0),
(518080, 49, 1054.6, -3092.41, 89.3336, 0, 0, 0, NULL, 100, 0),
(518080, 50, 1058.68, -3089.17, 91.7453, 0, 0, 0, NULL, 100, 0),
(518080, 51, 1060.6, -3083.75, 91.7453, 0, 0, 0, NULL, 100, 0),
(518080, 52, 1059.44, -3080.28, 91.7453, 0, 0, 0, NULL, 100, 0),
(518080, 53, 1058.01, -3076.83, 91.7453, 0, 0, 0, NULL, 100, 0),
(518080, 54, 1055.77, -3074.57, 91.7423, 0, 0, 0, NULL, 100, 0),
(518080, 55, 1053.49, -3072.35, 91.7432, 0, 0, 0, NULL, 100, 0),
(518080, 56, 1048.93, -3072.95, 91.7355, 0, 0, 0, NULL, 100, 0),
(518080, 57, 1041.48, -3075.29, 91.7333, 0, 0, 0, NULL, 100, 0),
(518080, 58, 1037.06, -3077.86, 91.772, 0, 0, 0, NULL, 100, 0),
(518080, 59, 1030.98, -3080.97, 91.754, 0, 0, 0, NULL, 100, 0),
(518080, 60, 1023.38, -3082.19, 91.7347, 0, 0, 0, NULL, 100, 0),
(518080, 61, 1015.59, -3080.34, 91.7187, 0, 0, 0, NULL, 100, 0),
(518080, 62, 1016.86, -3076.98, 91.7257, 0, 0, 0, NULL, 100, 0),
(518080, 63, 1019.51, -3077.52, 91.7313, 0, 0, 0, NULL, 100, 0),
(518080, 64, 1020.74, -3079.89, 91.7313, 0, 0, 0, NULL, 100, 0),
(518080, 65, 1023.99, -3081.43, 91.7371, 0, 0, 0, NULL, 100, 0),
(518080, 66, 1030.16, -3080.52, 91.7526, 0, 0, 0, NULL, 100, 0),
(518080, 67, 1035.14, -3078.67, 91.7665, 0, 0, 0, NULL, 100, 0),
(518080, 68, 1039.21, -3076.15, 91.7333, 0, 0, 0, NULL, 100, 0),
(518080, 69, 1044.73, -3073.75, 91.7319, 0, 0, 0, NULL, 100, 0),
(518080, 70, 1048.53, -3072.01, 91.7309, 0, 0, 0, NULL, 100, 0),
(518080, 71, 1052.28, -3069.47, 91.7492, 0, 0, 0, NULL, 100, 0),
(518080, 72, 1053.24, -3067.04, 91.762, 0, 0, 0, NULL, 100, 0),
(518080, 73, 1052.29, -3063.9, 91.6417, 0, 0, 0, NULL, 100, 0),
(518080, 74, 1051.01, -3060.32, 93.14, 0, 0, 0, NULL, 100, 0),
(518080, 75, 1046.46, -3055.47, 95.4885, 0, 0, 0, NULL, 100, 0),
(518080, 76, 1043.44, -3054.26, 97.0918, 0, 0, 0, NULL, 100, 0),
(518080, 77, 1039.26, -3053.32, 98.4449, 0, 0, 0, NULL, 100, 0),
(518080, 78, 1035.71, -3054.8, 98.9815, 0, 0, 0, NULL, 100, 0),
(518080, 79, 1033.22, -3056.35, 100.144, 0, 0, 0, NULL, 100, 0),
(518080, 80, 1030.55, -3058.91, 101.872, 0, 0, 0, NULL, 100, 0),
(518080, 81, 1029.75, -3062.89, 103.991, 0, 0, 0, NULL, 100, 0),
(518080, 82, 1028.89, -3066.46, 105.069, 0, 0, 0, NULL, 100, 0),
(518080, 83, 1027.57, -3071.8, 105.125, 0, 0, 0, NULL, 100, 0),
(518080, 84, 1026.92, -3074.34, 105.162, 0, 0, 0, NULL, 100, 0),
(518080, 85, 1029.29, -3078.58, 105.125, 0, 0, 0, NULL, 100, 0),
(518080, 86, 1031.8, -3080.06, 105.125, 0, 0, 0, NULL, 100, 0),
(518080, 87, 1034.15, -3084.09, 105.125, 0, 0, 0, NULL, 100, 0),
(518080, 88, 1035.67, -3087.39, 104.984, 0, 0, 0, NULL, 100, 0),
(518080, 89, 1037.77, -3091.23, 104.72, 0, 0, 0, NULL, 100, 0),
(518080, 90, 1040.77, -3093.77, 105.026, 0, 0, 0, NULL, 100, 0),
(518080, 91, 1044.16, -3096.44, 105.432, 0, 0, 0, NULL, 100, 0),
(518080, 92, 1048.07, -3097.35, 105.649, 0, 0, 0, NULL, 100, 0),
(518080, 93, 1055.78, -3094.85, 105.575, 0, 0, 0, NULL, 100, 0),
(518080, 94, 1059.14, -3093.44, 105.497, 0, 0, 0, NULL, 100, 0),
(518080, 95, 1060.82, -3089.71, 105.229, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (51808, 518080, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 40645 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1040459 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1040459 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1040459 AND spawnType = 0 WHERE ce.spawnID = 1040459;
INSERT INTO creature_entry (spawnID, entry) VALUES (40645, 23269);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (40645, 530, 1, 0, 0, -5028.41, 385.658,  171.015, 2.01292, 300, 0, 0, 29520, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(406450, 1, -5028.41, 385.658, 171.015, 0, 0, 1, NULL, 100, 0),
(406450, 2, -5037.62, 373.17, 171.017, 0, 0, 1, NULL, 100, 0),
(406450, 3, -5051.44, 373.313, 171.018, 0, 0, 1, NULL, 100, 0),
(406450, 4, -5055.75, 366.64, 171.018, 0, 0, 1, NULL, 100, 0),
(406450, 5, -5058.25, 357.087, 171.015, 0, 0, 1, NULL, 100, 0),
(406450, 6, -5055.19, 346, 170.972, 0, 0, 1, NULL, 100, 0),
(406450, 7, -5034.83, 346.222, 171.017, 0, 0, 1, NULL, 100, 0),
(406450, 8, -5025.36, 353.093, 170.691, 0, 0, 1, NULL, 100, 0),
(406450, 9, -5019.04, 362, 170.704, 0, 0, 1, NULL, 100, 0),
(406450, 10, -5018.18, 373.478, 170.964, 0, 0, 1, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (40645, 406450, 0, 0, 0, 1, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 40645);

-- Importing creature with guid 202718 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (202718, 25929);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (202718, 1, 1, 0, 0, 140.26, -4721.44,  17.7517, 0, 120, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (202718, 1);

-- Importing creature with guid 70553 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1069758 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1069758 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1069758 AND spawnType = 0 WHERE ce.spawnID = 1069758;
INSERT INTO creature_entry (spawnID, entry) VALUES (70553, 15605);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (70553, 1, 1, 0, 0, 2793.12, -350.333,  107.189, 1.97254, 25, 0, 0, 2563, 5751, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (70553, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO game_event_creature (guid, event) VALUES (70553, 7);

-- Importing creature with guid 70576 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1069781 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1069781 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1069781 AND spawnType = 0 WHERE ce.spawnID = 1069781;
INSERT INTO creature_entry (spawnID, entry) VALUES (70576, 15586);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (70576, 1, 1, 0, 0, -7155.89, -3770.23,  9.0816, 2.38788, 25, 0, 0, 2563, 5751, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (70576, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO game_event_creature (guid, event) VALUES (70576, 7);

-- Importing creature with guid 39704 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1039525 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1039525 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1039525 AND spawnType = 0 WHERE ce.spawnID = 1039525;
INSERT INTO creature_entry (spawnID, entry) VALUES (39704, 10658);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (39704, 1, 1, 9990, 0, 5126.69, -332.264,  356.926, 2.40376, 300, 8, 0, 42, 0, 1, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (39704, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 42904 with import type REPLACE_ALL
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 42904 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -42904 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 42904 AND spawnType = 0 WHERE ce.spawnID = 42904;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 42905 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -42905 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 42905 AND spawnType = 0 WHERE ce.spawnID = 42905;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 42907 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -42907 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 42907 AND spawnType = 0 WHERE ce.spawnID = 42907;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 42908 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -42908 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 42908 AND spawnType = 0 WHERE ce.spawnID = 42908;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 42909 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -42909 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 42909 AND spawnType = 0 WHERE ce.spawnID = 42909;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 42910 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -42910 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 42910 AND spawnType = 0 WHERE ce.spawnID = 42910;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1042718 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1042718 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1042718 AND spawnType = 0 WHERE ce.spawnID = 1042718;
INSERT INTO creature_entry (spawnID, entry) VALUES (42904, 15441);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (42904, 1, 1, 15566, 1, -7163.64, 1389.07,  2.96657, 2.87979, 300, 0, 0, 6104, 0, 0, 0, 0, 10);
INSERT INTO creature_entry (spawnID, entry) VALUES (42905, 15441);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (42905, 1, 1, 15566, 1, -7164.27, 1403.14,  3.01989, 3.15905, 300, 0, 0, 6104, 0, 0, 0, 0, 10);
INSERT INTO creature_entry (spawnID, entry) VALUES (42906, 15441);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (42906, 1, 1, 15566, 1, -7162.57, 1380.36,  2.92073, 2.05615, 300, 0, 0, 6104, 0, 0, 0, 0, 10);
INSERT INTO creature_entry (spawnID, entry) VALUES (42907, 15441);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (42907, 1, 1, 15566, 1, -7167.11, 1387.39,  2.92818, 1.36136, 300, 0, 0, 6104, 0, 0, 0, 0, 10);
INSERT INTO creature_entry (spawnID, entry) VALUES (42908, 15441);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (42908, 1, 1, 15566, 1, -7169.73, 1405.13,  2.8056, 5.044, 300, 0, 0, 6104, 0, 0, 0, 0, 10);
INSERT INTO creature_entry (spawnID, entry) VALUES (42909, 15441);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (42909, 1, 1, 15566, 1, -7164.14, 1391.24,  2.96657, 3.83972, 300, 0, 0, 6104, 0, 0, 0, 0, 10);
INSERT INTO creature_entry (spawnID, entry) VALUES (42910, 15441);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (42910, 1, 1, 15566, 1, -7178.14, 1390.42,  2.9267, 2.23402, 300, 0, 0, 6104, 0, 0, 0, 0, 10);

-- Importing creature with guid 42905 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 42906 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 42907 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 42908 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 42909 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 42910 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 43120 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1042932 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1042932 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1042932 AND spawnType = 0 WHERE ce.spawnID = 1042932;
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1051481 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1051481 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1051481 AND spawnType = 0 WHERE ce.spawnID = 1051481;
INSERT INTO creature_entry (spawnID, entry) VALUES (43120, 14473);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (43120, 1, 1, 14521, 0, -7803.76, 515.706,  -38.3517, 2.41835, 43200, 25, 0, 12208, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(431200, 1, -7803.76, 515.706, -38.3517, 0, 0, 0, NULL, 100, 0),
(431200, 2, -7836.16, 493.625, -35.2661, 0, 0, 0, NULL, 100, 0),
(431200, 3, -7853.41, 473.48, -33.6597, 0, 0, 0, NULL, 100, 0),
(431200, 4, -7883.19, 436.89, -32.8373, 0, 0, 0, NULL, 100, 0),
(431200, 5, -7902.39, 426.345, -32.1465, 0, 0, 0, NULL, 100, 0),
(431200, 6, -7935.67, 417.699, -30.5425, 0, 0, 0, NULL, 100, 0),
(431200, 7, -7962.83, 424.286, -31.6058, 0, 0, 0, NULL, 100, 0),
(431200, 8, -7985.03, 438.75, -30.1735, 0, 0, 0, NULL, 100, 0),
(431200, 9, -7990.21, 447.782, -30.206, 0, 0, 0, NULL, 100, 0),
(431200, 10, -7990.67, 458.651, -30.2729, 0, 0, 0, NULL, 100, 0),
(431200, 11, -7981.75, 472.983, -28.8862, 0, 0, 0, NULL, 100, 0),
(431200, 12, -7978.36, 486.361, -27.4351, 0, 0, 0, NULL, 100, 0),
(431200, 13, -7976.82, 500.483, -29.771, 0, 0, 0, NULL, 100, 0),
(431200, 14, -7969.69, 514.711, -29.5641, 0, 0, 0, NULL, 100, 0),
(431200, 15, -7956.18, 519.754, -29.4936, 0, 0, 0, NULL, 100, 0),
(431200, 16, -7947.12, 518.579, -28.2687, 0, 0, 0, NULL, 100, 0),
(431200, 17, -7926.89, 512.964, -30.5829, 0, 0, 0, NULL, 100, 0),
(431200, 18, -7906.53, 511.238, -31.4423, 0, 0, 0, NULL, 100, 0),
(431200, 19, -7885.87, 513.818, -32.6041, 0, 0, 0, NULL, 100, 0),
(431200, 20, -7865.16, 522.834, -35.135, 0, 0, 0, NULL, 100, 0),
(431200, 21, -7835.43, 536.469, -35.2565, 0, 0, 0, NULL, 100, 0),
(431200, 22, -7811.89, 531.936, -37.694, 0, 0, 0, NULL, 100, 0),
(431200, 23, -7787.8, 517.67, -39.175, 0, 0, 0, NULL, 100, 0),
(431200, 24, -7758.23, 498.947, -43.8957, 0, 0, 0, NULL, 100, 0),
(431200, 25, -7740.9, 487.13, -44.2471, 0, 0, 0, NULL, 100, 0),
(431200, 26, -7724.68, 473.798, -43.3483, 0, 0, 0, NULL, 100, 0),
(431200, 27, -7711.1, 466.066, -42.1628, 0, 0, 0, NULL, 100, 0),
(431200, 28, -7693.69, 462.775, -41.9354, 0, 0, 0, NULL, 100, 0),
(431200, 29, -7679.48, 465.435, -42.7258, 0, 0, 0, NULL, 100, 0),
(431200, 30, -7671.34, 477.137, -43.9373, 0, 0, 0, NULL, 100, 0),
(431200, 31, -7670.16, 492.271, -43.5576, 0, 0, 0, NULL, 100, 0),
(431200, 32, -7676.01, 503.307, -42.5046, 0, 0, 0, NULL, 100, 0),
(431200, 33, -7687.91, 517.064, -42.9588, 0, 0, 0, NULL, 100, 0),
(431200, 34, -7706.18, 525.816, -43.9323, 0, 0, 0, NULL, 100, 0),
(431200, 35, -7718.32, 525.054, -43.5419, 0, 0, 0, NULL, 100, 0),
(431200, 36, -7751.66, 514.521, -43.4128, 0, 0, 0, NULL, 100, 0),
(431200, 37, -7778.55, 507.992, -41.381, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (43120, 431200, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 43126 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1042938 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1042938 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1042938 AND spawnType = 0 WHERE ce.spawnID = 1042938;
INSERT INTO creature_entry (spawnID, entry) VALUES (43126, 14479);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (43126, 1, 1, 14526, 1, -7952.43, 1900.06,  1.37144, 2.85624, 21000, 0, 0, 3297, 2434, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(431260, 1, -7933.41, 1915.55, 4.74511, 0, 0, 0, NULL, 100, 0),
(431260, 2, -7899.94, 1913.04, 3.63808, 0, 0, 0, NULL, 100, 0),
(431260, 3, -7880.02, 1903.61, 4.84035, 0, 0, 0, NULL, 100, 0),
(431260, 4, -7874.55, 1886.31, 5.24531, 0, 0, 0, NULL, 100, 0),
(431260, 5, -7896.26, 1870.85, 5.10468, 0, 0, 0, NULL, 100, 0),
(431260, 6, -7912.29, 1852.62, 4.84624, 0, 0, 0, NULL, 100, 0),
(431260, 7, -7913.68, 1834.23, 3.93352, 0, 0, 0, NULL, 100, 0),
(431260, 8, -7923.22, 1822.2, 3.55475, 0, 0, 0, NULL, 100, 0),
(431260, 9, -7939.81, 1820.07, 3.94681, 0, 0, 0, NULL, 100, 0),
(431260, 10, -7950.68, 1834.83, 3.7851, 0, 0, 0, NULL, 100, 0),
(431260, 11, -7941.65, 1859.44, 4.83307, 0, 0, 0, NULL, 100, 0),
(431260, 12, -7948.32, 1881.01, 3.5273, 0, 0, 0, NULL, 100, 0),
(431260, 13, -7951.5, 1902.78, 2.1206, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (43126, 431260, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 46187 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1045978 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1045978 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1045978 AND spawnType = 0 WHERE ce.spawnID = 1045978;
INSERT INTO creature_entry (spawnID, entry) VALUES (46187, 20497);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (46187, 1, 1, 18968, 0, 9975.44, 2333.33,  1330.79, 5.48594, 300, 0, 0, 6986, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (46187, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 46205 with import type UPDATE_SPAWNID
UPDATE creature_entry SET spawnID = 46205 WHERE entry = 2079;
UPDATE conditions SET ConditionValue3 = 46205 WHERE ConditionValue3 = 1045996 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = -46205 WHERE SourceEntry = -1045996 AND SourceTypeOrReferenceId = 22;
UPDATE smart_scripts SET entryorguid = -46205 WHERE entryorguid = -1045996 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = 46205 WHERE target_param1 = 1045996 AND target_type = 10;
UPDATE spawn_group SET spawnID = 46205 WHERE spawnID = 1045996 AND spawnType = 0;

-- Importing creature with guid 46216 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1046007 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1046007 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1046007 AND spawnType = 0 WHERE ce.spawnID = 1046007;
INSERT INTO creature_entry (spawnID, entry) VALUES (46216, 14378);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (46216, 1, 1, 14417, 1, 9789.95, 2531.94,  1319.86, 5.42763, 600, 0, 0, 50400, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(462160, 1, 9776.55, 2531.54, 1318.93, 0, 0, 0, NULL, 100, 0),
(462160, 2, 9769.36, 2531.15, 1320.44, 0, 0, 0, NULL, 100, 0),
(462160, 3, 9767.59, 2531.53, 1321.08, 0, 0, 0, NULL, 100, 0),
(462160, 4, 9764.53, 2532.3, 1322.15, 0, 0, 0, NULL, 100, 0),
(462160, 5, 9736.58, 2537.27, 1332.01, 0, 0, 0, NULL, 100, 0),
(462160, 6, 9726.99, 2538.13, 1335.38, 0, 0, 0, NULL, 100, 0),
(462160, 7, 9723.63, 2540.35, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 8, 9721.65, 2542.9, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 9, 9710.68, 2562.7, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 10, 9691.4, 2586.07, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 11, 9666.76, 2605.76, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 12, 9663.6, 2607.97, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 13, 9661.93, 2610.4, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 14, 9661.1, 2613.19, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 15, 9659.82, 2627.53, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 16, 9658.55, 2628.94, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 17, 9657.16, 2629.32, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 18, 9654.96, 2629.39, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 19, 9653.19, 2628.38, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 20, 9652.2, 2626.19, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 21, 9652.27, 2623.64, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 22, 9653.91, 2602.25, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 23, 9654.32, 2600.85, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 24, 9655.22, 2599.56, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 25, 9656.18, 2598.45, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 26, 9657.87, 2597.43, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 27, 9661.17, 2596.11, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 28, 9663.25, 2594.92, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 29, 9665.19, 2593.21, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 30, 9683.42, 2575.19, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 31, 9702.05, 2556.37, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 32, 9711.03, 2542.86, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 33, 9712.06, 2513.48, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 34, 9709.97, 2499.3, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 35, 9686.86, 2471.14, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 36, 9661.4, 2446.29, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 37, 9659.7, 2443.97, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 38, 9658.9, 2441.08, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 39, 9658.38, 2437.55, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 40, 9654.82, 2413.7, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 41, 9654.82, 2413.7, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 42, 9655.17, 2410.48, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 43, 9656.12, 2408.8, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 44, 9657.85, 2407.21, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 45, 9660.05, 2406.62, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 46, 9662.48, 2407.19, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 47, 9664.22, 2408.92, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 48, 9665.18, 2411.42, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 49, 9665.36, 2414.14, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 50, 9664.27, 2432.98, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 51, 9664.56, 2434.64, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 52, 9665.23, 2435.96, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 53, 9667.42, 2437.88, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 54, 9695.22, 2463.22, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 55, 9713.3, 2489.69, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 56, 9719.5, 2512.31, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 57, 9720.29, 2513.85, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 58, 9721.17, 2514.99, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 59, 9722.42, 2515.67, 1335.68, 0, 0, 0, NULL, 100, 0),
(462160, 60, 9729.86, 2518.51, 1334.58, 0, 0, 0, NULL, 100, 0),
(462160, 61, 9735.13, 2520.27, 1332.72, 0, 0, 0, NULL, 100, 0),
(462160, 62, 9736.73, 2520.53, 1332.16, 0, 0, 0, NULL, 100, 0),
(462160, 63, 9766.79, 2522.69, 1321.44, 0, 0, 0, NULL, 100, 0),
(462160, 64, 9776.27, 2523.3, 1318.9, 0, 0, 0, NULL, 100, 0),
(462160, 65, 9807.24, 2524.39, 1320.62, 0, 0, 0, NULL, 100, 0),
(462160, 66, 9827.92, 2524.93, 1321.12, 0, 0, 0, NULL, 100, 0),
(462160, 67, 9852.38, 2525.86, 1321.08, 0, 0, 0, NULL, 100, 0),
(462160, 68, 9878.07, 2526.71, 1319.32, 0, 0, 0, NULL, 100, 0),
(462160, 69, 9894.91, 2529.19, 1316.53, 0, 0, 0, NULL, 100, 0),
(462160, 70, 9909.73, 2533.67, 1316.38, 0, 0, 0, NULL, 100, 0),
(462160, 71, 9910.85, 2535.16, 1316.38, 0, 0, 0, NULL, 100, 0),
(462160, 72, 9911.65, 2537.52, 1316.5, 0, 0, 0, NULL, 100, 0),
(462160, 73, 9913.32, 2539.02, 1316.63, 0, 0, 0, NULL, 100, 0),
(462160, 74, 9922.6, 2545.77, 1317.13, 0, 0, 0, NULL, 100, 0),
(462160, 75, 9939.89, 2553.77, 1316.82, 0, 0, 0, NULL, 100, 0),
(462160, 76, 9944.88, 2555.72, 1316.44, 0, 0, 0, NULL, 100, 0),
(462160, 77, 9946.14, 2555.79, 1316.32, 0, 0, 0, NULL, 100, 0),
(462160, 78, 9947.52, 2555.26, 1316.32, 0, 0, 0, NULL, 100, 0),
(462160, 79, 9970.48, 2541.57, 1316.75, 0, 0, 0, NULL, 100, 0),
(462160, 80, 9982.52, 2526.08, 1316.37, 0, 0, 0, NULL, 100, 0),
(462160, 81, 9983.06, 2524.38, 1316.5, 0, 0, 0, NULL, 100, 0),
(462160, 82, 9982.89, 2522.09, 1316.5, 0, 0, 0, NULL, 100, 0),
(462160, 83, 9981.35, 2520.54, 1316.75, 0, 0, 0, NULL, 100, 0),
(462160, 84, 9978.96, 2519.39, 1317.12, 0, 0, 0, NULL, 100, 0),
(462160, 85, 9968.58, 2510.83, 1317.75, 0, 0, 0, NULL, 100, 0),
(462160, 86, 9967.57, 2509.13, 1317.62, 0, 0, 0, NULL, 100, 0),
(462160, 87, 9966.94, 2506.63, 1317.37, 0, 0, 0, NULL, 100, 0),
(462160, 88, 9964.4, 2498.46, 1316.62, 0, 0, 0, NULL, 100, 0),
(462160, 89, 9963.43, 2496.6, 1316.49, 0, 0, 0, NULL, 100, 0),
(462160, 90, 9961.62, 2494.32, 1316.49, 0, 0, 0, NULL, 100, 0),
(462160, 91, 9949.46, 2481.62, 1316.24, 0, 0, 0, NULL, 100, 0),
(462160, 92, 9949.16, 2480.26, 1316.24, 0, 0, 0, NULL, 100, 0),
(462160, 93, 9949.14, 2478.52, 1316.24, 0, 0, 0, NULL, 100, 0),
(462160, 94, 9949.9, 2444.03, 1323.02, 0, 0, 0, NULL, 100, 0),
(462160, 95, 9950.92, 2406.31, 1328.17, 0, 0, 0, NULL, 100, 0),
(462160, 96, 9950.92, 2406.31, 1328.17, 0, 0, 0, NULL, 100, 0),
(462160, 97, 9951.81, 2372.26, 1330.06, 0, 0, 0, NULL, 100, 0),
(462160, 98, 9952.05, 2347.56, 1330.74, 0, 0, 0, NULL, 100, 0),
(462160, 99, 9953.08, 2310.86, 1336.55, 0, 0, 0, NULL, 100, 0),
(462160, 100, 9953.78, 2289.47, 1341.39, 0, 0, 0, NULL, 100, 0),
(462160, 101, 9953.67, 2270.07, 1340.67, 0, 0, 0, NULL, 100, 0),
(462160, 102, 9955.03, 2250.6, 1335.07, 0, 0, 0, NULL, 100, 0),
(462160, 103, 9954.9, 2229.24, 1331.83, 0, 0, 0, NULL, 100, 0),
(462160, 104, 9955.75, 2200.32, 1328.33, 0, 0, 0, NULL, 100, 0),
(462160, 105, 9955.95, 2176.62, 1327.58, 0, 0, 0, NULL, 100, 0),
(462160, 106, 9955.34, 2148.66, 1327.83, 0, 0, 0, NULL, 100, 0),
(462160, 107, 9953.31, 2119.95, 1327.77, 0, 0, 0, NULL, 100, 0),
(462160, 108, 9957.02, 2095.75, 1327.8, 0, 0, 0, NULL, 100, 0),
(462160, 109, 9952.51, 2076.1, 1327.8, 0, 0, 0, NULL, 100, 0),
(462160, 110, 9952.51, 2073.93, 1327.8, 0, 0, 0, NULL, 100, 0),
(462160, 111, 9953.16, 2071.67, 1327.8, 0, 0, 0, NULL, 100, 0),
(462160, 112, 9954.76, 2069.54, 1327.93, 0, 0, 0, NULL, 100, 0),
(462160, 113, 9973.54, 2042.18, 1328.14, 0, 0, 0, NULL, 100, 0),
(462160, 114, 9974, 2040.55, 1328.14, 0, 0, 0, NULL, 100, 0),
(462160, 115, 9973.74, 2039.33, 1328.14, 0, 0, 0, NULL, 100, 0),
(462160, 116, 9972.79, 2038.65, 1328.14, 0, 0, 0, NULL, 100, 0),
(462160, 117, 9971.4, 2038.56, 1328.14, 0, 0, 0, NULL, 100, 0),
(462160, 118, 9969.6, 2038.98, 1328.14, 0, 0, 0, NULL, 100, 0),
(462160, 119, 9967.37, 2040.95, 1328.26, 0, 0, 0, NULL, 100, 0),
(462160, 120, 9957.31, 2052.81, 1328.37, 0, 0, 0, NULL, 100, 0),
(462160, 121, 9955.88, 2055, 1328.37, 0, 0, 0, NULL, 100, 0),
(462160, 122, 9954.63, 2057.55, 1328.24, 0, 0, 0, NULL, 100, 0),
(462160, 123, 9946.69, 2073.5, 1327.93, 0, 0, 0, NULL, 100, 0),
(462160, 124, 9946.26, 2075.42, 1327.8, 0, 0, 0, NULL, 100, 0),
(462160, 125, 9946.45, 2077.42, 1327.8, 0, 0, 0, NULL, 100, 0),
(462160, 126, 9951.13, 2094.52, 1327.8, 0, 0, 0, NULL, 100, 0),
(462160, 127, 9951.55, 2096.97, 1327.8, 0, 0, 0, NULL, 100, 0),
(462160, 128, 9951.2, 2099.71, 1327.8, 0, 0, 0, NULL, 100, 0),
(462160, 129, 9950.03, 2136.16, 1327.7, 0, 0, 0, NULL, 100, 0),
(462160, 130, 9949.29, 2181.96, 1327.58, 0, 0, 0, NULL, 100, 0),
(462160, 131, 9948.41, 2224.65, 1330.95, 0, 0, 0, NULL, 100, 0),
(462160, 132, 9948.41, 2224.65, 1330.95, 0, 0, 0, NULL, 100, 0),
(462160, 133, 9949.03, 2253.35, 1335.44, 0, 0, 0, NULL, 100, 0),
(462160, 134, 9950.26, 2270.03, 1340.69, 0, 0, 0, NULL, 100, 0),
(462160, 135, 9949.62, 2288.9, 1341.39, 0, 0, 0, NULL, 100, 0),
(462160, 136, 9948.97, 2311.09, 1336.55, 0, 0, 0, NULL, 100, 0),
(462160, 137, 9947.62, 2346.41, 1330.76, 0, 0, 0, NULL, 100, 0),
(462160, 138, 9947.48, 2372.31, 1330.04, 0, 0, 0, NULL, 100, 0),
(462160, 139, 9947.57, 2406.61, 1328.11, 0, 0, 0, NULL, 100, 0),
(462160, 140, 9946.79, 2444.89, 1322.83, 0, 0, 0, NULL, 100, 0),
(462160, 141, 9947.04, 2473.77, 1316.55, 0, 0, 0, NULL, 100, 0),
(462160, 142, 9946.34, 2481.54, 1316.24, 0, 0, 0, NULL, 100, 0),
(462160, 143, 9945.25, 2483.19, 1316.24, 0, 0, 0, NULL, 100, 0),
(462160, 144, 9943.71, 2484.51, 1316.49, 0, 0, 0, NULL, 100, 0),
(462160, 145, 9941.13, 2486.06, 1316.74, 0, 0, 0, NULL, 100, 0),
(462160, 146, 9933.91, 2491.38, 1317.74, 0, 0, 0, NULL, 100, 0),
(462160, 147, 9933.32, 2492.54, 1317.9, 0, 0, 0, NULL, 100, 0),
(462160, 148, 9933.14, 2494.48, 1317.9, 0, 0, 0, NULL, 100, 0),
(462160, 149, 9933.66, 2496.92, 1317.87, 0, 0, 0, NULL, 100, 0),
(462160, 150, 9936.65, 2504.63, 1317.88, 0, 0, 0, NULL, 100, 0),
(462160, 151, 9937.35, 2506.57, 1317.88, 0, 0, 0, NULL, 100, 0),
(462160, 152, 9936.72, 2507.74, 1317.88, 0, 0, 0, NULL, 100, 0),
(462160, 153, 9935.66, 2507.19, 1317.88, 0, 0, 0, NULL, 100, 0),
(462160, 154, 9934.84, 2504.61, 1317.88, 0, 0, 0, NULL, 100, 0),
(462160, 155, 9933.5, 2502.29, 1317.88, 0, 0, 0, NULL, 100, 0),
(462160, 156, 9931.78, 2500.85, 1317.92, 0, 0, 0, NULL, 100, 0),
(462160, 157, 9929.06, 2500.65, 1317.92, 0, 0, 0, NULL, 100, 0),
(462160, 158, 9926.89, 2501.91, 1318.17, 0, 0, 0, NULL, 100, 0),
(462160, 159, 9919.01, 2508.07, 1317.55, 0, 0, 0, NULL, 100, 0),
(462160, 160, 9917.28, 2509.89, 1317.3, 0, 0, 0, NULL, 100, 0),
(462160, 161, 9916.58, 2512.78, 1317.17, 0, 0, 0, NULL, 100, 0),
(462160, 162, 9915.53, 2521.38, 1317.05, 0, 0, 0, NULL, 100, 0),
(462160, 163, 9914.63, 2523.59, 1317.05, 0, 0, 0, NULL, 100, 0),
(462160, 164, 9913.53, 2525.35, 1316.67, 0, 0, 0, NULL, 100, 0),
(462160, 165, 9907.01, 2533.01, 1316.17, 0, 0, 0, NULL, 100, 0),
(462160, 166, 9905.48, 2533.7, 1316, 0, 0, 0, NULL, 100, 0),
(462160, 167, 9903.21, 2534.1, 1315.75, 0, 0, 0, NULL, 100, 0),
(462160, 168, 9896.53, 2534.52, 1316, 0, 0, 0, NULL, 100, 0),
(462160, 169, 9886.23, 2534.43, 1318.75, 0, 0, 0, NULL, 100, 0),
(462160, 170, 9854.98, 2533.49, 1320.89, 0, 0, 0, NULL, 100, 0),
(462160, 171, 9825.17, 2532.56, 1321.06, 0, 0, 0, NULL, 100, 0),
(462160, 172, 9789.95, 2531.94, 1319.86, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (46216, 462160, 9991, 0, 0, 257, 0, NULL);

-- Importing creature with guid 46219 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1046010 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1046010 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1046010 AND spawnType = 0 WHERE ce.spawnID = 1046010;
DELETE FROM waypoint_data WHERE id = 1126;
INSERT INTO creature_entry (spawnID, entry) VALUES (46219, 14379);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (46219, 1, 1, 14418, 1, 10172.5, 2350.92,  1327.93, 0.33403, 600, 0, 0, 50400, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(462190, 1, 10174.8, 2346.64, 1328.18, 0, 0, 0, NULL, 100, 0),
(462190, 2, 10182.5, 2339.69, 1328.06, 0, 0, 0, NULL, 100, 0),
(462190, 3, 10185.9, 2333.11, 1327.83, 0, 0, 0, NULL, 100, 0),
(462190, 4, 10186.5, 2328.58, 1327.95, 0, 0, 0, NULL, 100, 0),
(462190, 5, 10181.4, 2314.36, 1327.83, 0, 0, 0, NULL, 100, 0),
(462190, 6, 10170.9, 2294.66, 1328.04, 0, 0, 0, NULL, 100, 0),
(462190, 7, 10155.1, 2273.72, 1328.55, 0, 0, 0, NULL, 100, 0),
(462190, 8, 10140.4, 2259.7, 1328.09, 0, 0, 0, NULL, 100, 0),
(462190, 9, 10131, 2249.28, 1327.7, 0, 0, 0, NULL, 100, 0),
(462190, 10, 10126.2, 2246.83, 1327.7, 0, 0, 0, NULL, 100, 0),
(462190, 11, 10120.1, 2247.96, 1327.58, 0, 0, 0, NULL, 100, 0),
(462190, 12, 10114, 2253.09, 1327.58, 0, 0, 0, NULL, 100, 0),
(462190, 13, 10099, 2271.4, 1328.11, 0, 0, 0, NULL, 100, 0),
(462190, 14, 10088.5, 2287.25, 1329.36, 0, 0, 0, NULL, 100, 0),
(462190, 15, 10081.6, 2295.72, 1329.61, 0, 0, 0, NULL, 100, 0),
(462190, 16, 10075.9, 2295.84, 1329.49, 0, 0, 0, NULL, 100, 0),
(462190, 17, 10067.8, 2290.9, 1329.49, 0, 0, 0, NULL, 100, 0),
(462190, 18, 10055.8, 2285.36, 1329.44, 0, 0, 0, NULL, 100, 0),
(462190, 19, 10035.5, 2280.12, 1328.94, 0, 0, 0, NULL, 100, 0),
(462190, 20, 10019.1, 2281.63, 1335.93, 0, 0, 0, NULL, 100, 0),
(462190, 21, 10009.5, 2282.59, 1341.04, 0, 0, 0, NULL, 100, 0),
(462190, 22, 10002.2, 2282.77, 1341.39, 0, 0, 0, NULL, 100, 0),
(462190, 23, 9974.47, 2279.73, 1341.39, 0, 0, 0, NULL, 100, 0),
(462190, 24, 9958.83, 2279.63, 1341.39, 0, 0, 0, NULL, 100, 0),
(462190, 25, 9954.92, 2282.52, 1341.39, 0, 0, 0, NULL, 100, 0),
(462190, 26, 9950.62, 2289.64, 1341.39, 0, 0, 0, NULL, 100, 0),
(462190, 27, 9948.64, 2298.74, 1338.22, 0, 0, 0, NULL, 100, 0),
(462190, 28, 9948.25, 2316.27, 1336.55, 0, 0, 0, NULL, 100, 0),
(462190, 29, 9947.92, 2327.04, 1334.35, 0, 0, 0, NULL, 100, 0),
(462190, 30, 9946.04, 2352.68, 1330.66, 0, 0, 0, NULL, 100, 0),
(462190, 31, 9944.61, 2386.32, 1329.33, 0, 0, 0, NULL, 100, 0),
(462190, 32, 9942.54, 2429.39, 1325.2, 0, 0, 0, NULL, 100, 0),
(462190, 33, 9943.56, 2456.7, 1320.44, 0, 0, 0, NULL, 100, 0),
(462190, 34, 9945.89, 2478.35, 1316.24, 0, 0, 0, NULL, 100, 0),
(462190, 35, 9945.46, 2483.71, 1316.37, 0, 0, 0, NULL, 100, 0),
(462190, 36, 9941.52, 2488.43, 1316.87, 0, 0, 0, NULL, 100, 0),
(462190, 37, 9934.64, 2492.63, 1317.74, 0, 0, 0, NULL, 100, 0),
(462190, 38, 9931.78, 2496.82, 1317.9, 0, 0, 0, NULL, 100, 0),
(462190, 39, 9923.84, 2505.3, 1318.05, 0, 0, 0, NULL, 100, 0),
(462190, 40, 9918.91, 2507.74, 1317.55, 0, 0, 0, NULL, 100, 0),
(462190, 41, 9915.42, 2512.41, 1317.05, 0, 0, 0, NULL, 100, 0),
(462190, 42, 9914.59, 2520.89, 1317.05, 0, 0, 0, NULL, 100, 0),
(462190, 43, 9910.34, 2527.94, 1316.42, 0, 0, 0, NULL, 100, 0),
(462190, 44, 9909.56, 2533.26, 1316.42, 0, 0, 0, NULL, 100, 0),
(462190, 45, 9912.56, 2537.83, 1316.63, 0, 0, 0, NULL, 100, 0),
(462190, 46, 9924.19, 2546.5, 1317.25, 0, 0, 0, NULL, 100, 0),
(462190, 47, 9924.19, 2546.5, 1317.25, 0, 0, 0, NULL, 100, 0),
(462190, 48, 9941.22, 2553.06, 1316.82, 0, 0, 0, NULL, 100, 0),
(462190, 49, 9949.14, 2553.75, 1316.44, 0, 0, 0, NULL, 100, 0),
(462190, 50, 9955.25, 2548.69, 1316.82, 0, 0, 0, NULL, 100, 0),
(462190, 51, 9961.83, 2545.14, 1317.07, 0, 0, 0, NULL, 100, 0),
(462190, 52, 9968.58, 2541.93, 1316.87, 0, 0, 0, NULL, 100, 0),
(462190, 53, 9987.74, 2520.83, 1316.25, 0, 0, 0, NULL, 100, 0),
(462190, 54, 9993.47, 2514.22, 1316.25, 0, 0, 0, NULL, 100, 0),
(462190, 55, 10002.2, 2510.39, 1316.55, 0, 0, 0, NULL, 100, 0),
(462190, 56, 10012.2, 2506.15, 1319.02, 0, 0, 0, NULL, 100, 0),
(462190, 57, 10031.3, 2500.34, 1318.42, 0, 0, 0, NULL, 100, 0),
(462190, 58, 10036.8, 2500.6, 1318.42, 0, 0, 0, NULL, 100, 0),
(462190, 59, 10051.2, 2508.19, 1318.39, 0, 0, 0, NULL, 100, 0),
(462190, 60, 10062.5, 2514.76, 1320.1, 0, 0, 0, NULL, 100, 0),
(462190, 61, 10074.4, 2521.6, 1319.62, 0, 0, 0, NULL, 100, 0),
(462190, 62, 10084.5, 2527.64, 1317.59, 0, 0, 0, NULL, 100, 0),
(462190, 63, 10089.4, 2533.48, 1317.89, 0, 0, 0, NULL, 100, 0),
(462190, 64, 10095.5, 2538.91, 1319.01, 0, 0, 0, NULL, 100, 0),
(462190, 65, 10101.1, 2539.83, 1319.71, 0, 0, 0, NULL, 100, 0),
(462190, 66, 10117, 2542.48, 1321.34, 0, 0, 0, NULL, 100, 0),
(462190, 67, 10124.5, 2544.19, 1321.71, 0, 0, 0, NULL, 100, 0),
(462190, 68, 10131.5, 2546.76, 1321.84, 0, 0, 0, NULL, 100, 0),
(462190, 69, 10136.5, 2550.14, 1321.8, 0, 0, 0, NULL, 100, 0),
(462190, 70, 10139.3, 2553.62, 1321.8, 0, 0, 0, NULL, 100, 0),
(462190, 71, 10139.5, 2557.88, 1321.92, 0, 0, 0, NULL, 100, 0),
(462190, 72, 10138.1, 2563.78, 1322.17, 0, 0, 0, NULL, 100, 0),
(462190, 73, 10135, 2566.4, 1322.55, 0, 0, 0, NULL, 100, 0),
(462190, 74, 10132.3, 2571.3, 1323.33, 0, 0, 0, NULL, 100, 0),
(462190, 75, 10131.5, 2574.56, 1323.71, 0, 0, 0, NULL, 100, 0),
(462190, 76, 10134, 2573.48, 1323.28, 0, 0, 0, NULL, 100, 0),
(462190, 77, 10141.2, 2565.23, 1321.8, 0, 0, 0, NULL, 100, 0),
(462190, 78, 10142.4, 2562.29, 1321.92, 0, 0, 0, NULL, 100, 0),
(462190, 79, 10143.8, 2553.87, 1321.8, 0, 0, 0, NULL, 100, 0),
(462190, 80, 10140.7, 2548.88, 1321.92, 0, 0, 0, NULL, 100, 0),
(462190, 81, 10120.1, 2539.57, 1321.71, 0, 0, 0, NULL, 100, 0),
(462190, 82, 10107.5, 2537.9, 1320.34, 0, 0, 0, NULL, 100, 0),
(462190, 83, 10098.9, 2536.35, 1319.26, 0, 0, 0, NULL, 100, 0),
(462190, 84, 10090.9, 2529.96, 1317.59, 0, 0, 0, NULL, 100, 0),
(462190, 85, 10090.3, 2526.44, 1317.59, 0, 0, 0, NULL, 100, 0),
(462190, 86, 10099.5, 2510.2, 1317.72, 0, 0, 0, NULL, 100, 0),
(462190, 87, 10108.5, 2492.17, 1317.57, 0, 0, 0, NULL, 100, 0),
(462190, 88, 10109.9, 2483.67, 1317.57, 0, 0, 0, NULL, 100, 0),
(462190, 89, 10104.2, 2464.98, 1317.44, 0, 0, 0, NULL, 100, 0),
(462190, 90, 10105.3, 2454.7, 1318.82, 0, 0, 0, NULL, 100, 0),
(462190, 91, 10106.1, 2442.88, 1318.94, 0, 0, 0, NULL, 100, 0),
(462190, 92, 10104.3, 2411.59, 1316.09, 0, 0, 0, NULL, 100, 0),
(462190, 93, 10104.3, 2411.59, 1316.09, 0, 0, 0, NULL, 100, 0),
(462190, 94, 10098.8, 2392.37, 1316.51, 0, 0, 0, NULL, 100, 0),
(462190, 95, 10099.9, 2389.5, 1316.51, 0, 0, 0, NULL, 100, 0),
(462190, 96, 10107.3, 2385.02, 1316.98, 0, 0, 0, NULL, 100, 0),
(462190, 97, 10115.7, 2379.92, 1317.48, 0, 0, 0, NULL, 100, 0),
(462190, 98, 10127.5, 2374.58, 1319.48, 0, 0, 0, NULL, 100, 0),
(462190, 99, 10133.7, 2372.88, 1320.37, 0, 0, 0, NULL, 100, 0),
(462190, 100, 10142.2, 2372.35, 1321.37, 0, 0, 0, NULL, 100, 0),
(462190, 101, 10152.8, 2371.75, 1323.12, 0, 0, 0, NULL, 100, 0),
(462190, 102, 10164, 2367.42, 1324.74, 0, 0, 0, NULL, 100, 0),
(462190, 103, 10169.9, 2361.33, 1326.18, 0, 0, 0, NULL, 100, 0),
(462190, 104, 10172.5, 2350.92, 1327.93, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (46219, 462190, 9991, 0, 0, 257, 0, NULL);

-- Importing creature with guid 46220 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1046011 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1046011 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1046011 AND spawnType = 0 WHERE ce.spawnID = 1046011;
DELETE FROM waypoint_data WHERE id = 1127;
INSERT INTO creature_entry (spawnID, entry) VALUES (46220, 14380);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (46220, 1, 1, 14419, 1, 9667.44, 2324.24,  1327.61, 2.64222, 600, 0, 0, 50400, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(462200, 1, 9662.45, 2327.01, 1327.61, 0, 0, 0, NULL, 100, 0),
(462200, 2, 9658.77, 2330.16, 1327.61, 0, 0, 0, NULL, 100, 0),
(462200, 3, 9650.28, 2338.99, 1327.56, 0, 0, 0, NULL, 100, 0),
(462200, 4, 9647.56, 2342.8, 1327.56, 0, 0, 0, NULL, 100, 0),
(462200, 5, 9644.05, 2347.35, 1327.56, 0, 0, 0, NULL, 100, 0),
(462200, 6, 9740.74, 2340.18, 1328.34, 0, 0, 0, NULL, 100, 0),
(462200, 7, 9733.45, 2342.2, 1327.84, 0, 0, 0, NULL, 100, 0),
(462200, 8, 9728.34, 2347.02, 1327.62, 0, 0, 0, NULL, 100, 0),
(462200, 9, 9724.54, 2351.81, 1327.5, 0, 0, 0, NULL, 100, 0),
(462200, 10, 9719.77, 2357.38, 1327.5, 0, 0, 0, NULL, 100, 0),
(462200, 11, 9716.24, 2359.51, 1327.5, 0, 0, 0, NULL, 100, 0),
(462200, 12, 9713.92, 2358.9, 1327.5, 0, 0, 0, NULL, 100, 0),
(462200, 13, 9713.4, 2357.47, 1327.5, 0, 0, 0, NULL, 100, 0),
(462200, 14, 9715.08, 2355.19, 1327.5, 0, 0, 0, NULL, 100, 0),
(462200, 15, 9722.51, 2348.68, 1327.37, 0, 0, 0, NULL, 100, 0),
(462200, 16, 9731.41, 2339.11, 1327.87, 0, 0, 0, NULL, 100, 0),
(462200, 17, 9738.89, 2335.75, 1328.72, 0, 0, 0, NULL, 100, 0),
(462200, 18, 9742.57, 2333.9, 1328.97, 0, 0, 0, NULL, 100, 0),
(462200, 19, 9748.34, 2329.59, 1328.98, 0, 0, 0, NULL, 100, 0),
(462200, 20, 9752.23, 2325.58, 1328.85, 0, 0, 0, NULL, 100, 0),
(462200, 21, 9755.49, 2319.34, 1328.48, 0, 0, 0, NULL, 100, 0),
(462200, 22, 9756.31, 2314.43, 1328.23, 0, 0, 0, NULL, 100, 0),
(462200, 23, 9760.53, 2310.07, 1327.98, 0, 0, 0, NULL, 100, 0),
(462200, 24, 9762.57, 2305.8, 1327.85, 0, 0, 0, NULL, 100, 0),
(462200, 25, 9771.15, 2298.07, 1327.48, 0, 0, 0, NULL, 100, 0),
(462200, 26, 9779.69, 2289.91, 1327.48, 0, 0, 0, NULL, 100, 0),
(462200, 27, 9783.95, 2287.09, 1327.73, 0, 0, 0, NULL, 100, 0),
(462200, 28, 9791.25, 2283.94, 1327.73, 0, 0, 0, NULL, 100, 0),
(462200, 29, 9798.12, 2283.38, 1327.35, 0, 0, 0, NULL, 100, 0),
(462200, 30, 9804.93, 2283.01, 1327.15, 0, 0, 0, NULL, 100, 0),
(462200, 31, 9814.32, 2278.78, 1327.15, 0, 0, 0, NULL, 100, 0),
(462200, 32, 9822.92, 2276.22, 1327.02, 0, 0, 0, NULL, 100, 0),
(462200, 33, 9832.75, 2277, 1327.02, 0, 0, 0, NULL, 100, 0),
(462200, 34, 9843.3, 2278.08, 1326.74, 0, 0, 0, NULL, 100, 0),
(462200, 35, 9858.15, 2278.75, 1327.24, 0, 0, 0, NULL, 100, 0),
(462200, 36, 9869.67, 2278.27, 1328.65, 0, 0, 0, NULL, 100, 0),
(462200, 37, 9880.76, 2278.4, 1333.95, 0, 0, 0, NULL, 100, 0),
(462200, 38, 9890.19, 2279.55, 1338.64, 0, 0, 0, NULL, 100, 0),
(462200, 39, 9899.26, 2279.79, 1341.39, 0, 0, 0, NULL, 100, 0),
(462200, 40, 9912.74, 2278.86, 1341.39, 0, 0, 0, NULL, 100, 0),
(462200, 41, 9924, 2278.35, 1341.39, 0, 0, 0, NULL, 100, 0),
(462200, 42, 9936.09, 2278.17, 1341.39, 0, 0, 0, NULL, 100, 0),
(462200, 43, 9943.57, 2279.43, 1341.39, 0, 0, 0, NULL, 100, 0),
(462200, 44, 9947.86, 2282.39, 1341.39, 0, 0, 0, NULL, 100, 0),
(462200, 45, 9949.2, 2285, 1341.39, 0, 0, 0, NULL, 100, 0),
(462200, 46, 9949.31, 2287.86, 1341.39, 0, 0, 0, NULL, 100, 0),
(462200, 47, 9949.21, 2293.42, 1341.02, 0, 0, 0, NULL, 100, 0),
(462200, 48, 9948.45, 2304.3, 1336.55, 0, 0, 0, NULL, 100, 0),
(462200, 49, 9948.39, 2313.23, 1336.55, 0, 0, 0, NULL, 100, 0),
(462200, 50, 9948.19, 2323.44, 1335.48, 0, 0, 0, NULL, 100, 0),
(462200, 51, 9947.31, 2336.46, 1331.57, 0, 0, 0, NULL, 100, 0),
(462200, 52, 9946.2, 2354.28, 1330.61, 0, 0, 0, NULL, 100, 0),
(462200, 53, 9945.45, 2375.02, 1329.95, 0, 0, 0, NULL, 100, 0),
(462200, 54, 9944.13, 2409.82, 1327.69, 0, 0, 0, NULL, 100, 0),
(462200, 55, 9944.13, 2409.82, 1327.69, 0, 0, 0, NULL, 100, 0),
(462200, 56, 9943, 2428.54, 1325.31, 0, 0, 0, NULL, 100, 0),
(462200, 57, 9942.94, 2447.72, 1322.24, 0, 0, 0, NULL, 100, 0),
(462200, 58, 9944.98, 2470.11, 1317.46, 0, 0, 0, NULL, 100, 0),
(462200, 59, 9946.16, 2480.07, 1316.24, 0, 0, 0, NULL, 100, 0),
(462200, 60, 9945.3, 2483.65, 1316.37, 0, 0, 0, NULL, 100, 0),
(462200, 61, 9938, 2490, 1317.37, 0, 0, 0, NULL, 100, 0),
(462200, 62, 9932.35, 2496.28, 1317.9, 0, 0, 0, NULL, 100, 0),
(462200, 63, 9923.41, 2503.71, 1318.05, 0, 0, 0, NULL, 100, 0),
(462200, 64, 9919.17, 2505.46, 1317.55, 0, 0, 0, NULL, 100, 0),
(462200, 65, 9915.68, 2505.95, 1317.05, 0, 0, 0, NULL, 100, 0),
(462200, 66, 9907.5, 2505.25, 1316.03, 0, 0, 0, NULL, 100, 0),
(462200, 67, 9899.11, 2503.24, 1318.12, 0, 0, 0, NULL, 100, 0),
(462200, 68, 9888.55, 2501.81, 1317.89, 0, 0, 0, NULL, 100, 0),
(462200, 69, 9882.41, 2498.56, 1315.92, 0, 0, 0, NULL, 100, 0),
(462200, 70, 9878.96, 2494.41, 1315.88, 0, 0, 0, NULL, 100, 0),
(462200, 71, 9876.62, 2488.41, 1315.88, 0, 0, 0, NULL, 100, 0),
(462200, 72, 9876.19, 2481.46, 1316.11, 0, 0, 0, NULL, 100, 0),
(462200, 73, 9877.87, 2476.74, 1317.8, 0, 0, 0, NULL, 100, 0),
(462200, 74, 9880.84, 2470.35, 1317.88, 0, 0, 0, NULL, 100, 0),
(462200, 75, 9883.1, 2465.26, 1317.3, 0, 0, 0, NULL, 100, 0),
(462200, 76, 9886.26, 2458.91, 1317.37, 0, 0, 0, NULL, 100, 0),
(462200, 77, 9886.66, 2453.56, 1317.24, 0, 0, 0, NULL, 100, 0),
(462200, 78, 9883.71, 2439.95, 1316.24, 0, 0, 0, NULL, 100, 0),
(462200, 79, 9878.99, 2428.08, 1316.38, 0, 0, 0, NULL, 100, 0),
(462200, 80, 9863.95, 2399.86, 1316.92, 0, 0, 0, NULL, 100, 0),
(462200, 81, 9849.99, 2376.17, 1316.36, 0, 0, 0, NULL, 100, 0),
(462200, 82, 9843.15, 2368.84, 1315.63, 0, 0, 0, NULL, 100, 0),
(462200, 83, 9840.7, 2365.62, 1316.81, 0, 0, 0, NULL, 100, 0),
(462200, 84, 9837.72, 2360.19, 1316.68, 0, 0, 0, NULL, 100, 0),
(462200, 85, 9838.51, 2348.59, 1314.81, 0, 0, 0, NULL, 100, 0),
(462200, 86, 9839.29, 2340.45, 1314.31, 0, 0, 0, NULL, 100, 0),
(462200, 87, 9839.9, 2333.73, 1314.31, 0, 0, 0, NULL, 100, 0),
(462200, 88, 9839.62, 2321.73, 1315.66, 0, 0, 0, NULL, 100, 0),
(462200, 89, 9839.23, 2310.68, 1318.16, 0, 0, 0, NULL, 100, 0),
(462200, 90, 9840.33, 2308.16, 1318.53, 0, 0, 0, NULL, 100, 0),
(462200, 91, 9843.7, 2303.42, 1319.28, 0, 0, 0, NULL, 100, 0),
(462200, 92, 9846.72, 2298.52, 1320.74, 0, 0, 0, NULL, 100, 0),
(462200, 93, 9849.37, 2289.92, 1323.62, 0, 0, 0, NULL, 100, 0),
(462200, 94, 9850.79, 2280.19, 1326.24, 0, 0, 0, NULL, 100, 0),
(462200, 95, 9851.72, 2269.08, 1327.12, 0, 0, 0, NULL, 100, 0),
(462200, 96, 9851.05, 2260.47, 1327.67, 0, 0, 0, NULL, 100, 0),
(462200, 97, 9849.52, 2250.54, 1327.67, 0, 0, 0, NULL, 100, 0),
(462200, 98, 9846.16, 2241.25, 1327.79, 0, 0, 0, NULL, 100, 0),
(462200, 99, 9842.95, 2232.97, 1327.7, 0, 0, 0, NULL, 100, 0),
(462200, 100, 9838.71, 2226.47, 1327.7, 0, 0, 0, NULL, 100, 0),
(462200, 101, 9835.18, 2221.21, 1327.83, 0, 0, 0, NULL, 100, 0),
(462200, 102, 9830.41, 2218.2, 1327.89, 0, 0, 0, NULL, 100, 0),
(462200, 103, 9824.44, 2216.95, 1327.89, 0, 0, 0, NULL, 100, 0),
(462200, 104, 9816.69, 2216.25, 1327.77, 0, 0, 0, NULL, 100, 0),
(462200, 105, 9806.7, 2217.33, 1327.77, 0, 0, 0, NULL, 100, 0),
(462200, 106, 9798.7, 2220.23, 1327.7, 0, 0, 0, NULL, 100, 0),
(462200, 107, 9791.35, 2224.93, 1327.7, 0, 0, 0, NULL, 100, 0),
(462200, 108, 9781.25, 2231.72, 1327.7, 0, 0, 0, NULL, 100, 0),
(462200, 109, 9769.54, 2238.83, 1327.77, 0, 0, 0, NULL, 100, 0),
(462200, 110, 9759.6, 2244.79, 1327.73, 0, 0, 0, NULL, 100, 0),
(462200, 111, 9747.96, 2250.43, 1327.61, 0, 0, 0, NULL, 100, 0),
(462200, 112, 9739.26, 2255.18, 1327.61, 0, 0, 0, NULL, 100, 0),
(462200, 113, 9727.25, 2261.62, 1327.56, 0, 0, 0, NULL, 100, 0),
(462200, 114, 9719.52, 2265.09, 1327.56, 0, 0, 0, NULL, 100, 0),
(462200, 115, 9712.61, 2270.8, 1327.6, 0, 0, 0, NULL, 100, 0),
(462200, 116, 9704.03, 2278.19, 1327.6, 0, 0, 0, NULL, 100, 0),
(462200, 117, 9698.98, 2283.58, 1327.61, 0, 0, 0, NULL, 100, 0),
(462200, 118, 9696.37, 2289.91, 1327.61, 0, 0, 0, NULL, 100, 0),
(462200, 119, 9690.62, 2298.11, 1327.61, 0, 0, 0, NULL, 100, 0),
(462200, 120, 9681.88, 2308.4, 1327.61, 0, 0, 0, NULL, 100, 0),
(462200, 121, 9673.43, 2317.45, 1327.61, 0, 0, 0, NULL, 100, 0),
(462200, 122, 9667.44, 2324.24, 1327.61, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (46220, 462200, 9991, 0, 0, 257, 0, NULL);

-- Importing creature with guid 46350 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1046141 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1046141 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1046141 AND spawnType = 0 WHERE ce.spawnID = 1046141;
INSERT INTO creature_entry (spawnID, entry) VALUES (46350, 14428);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (46350, 1, 1, 6818, 0, 9816.84, 355.442,  1308.46, 0.660401, 7200, 0, 1, 137, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(463500, 1, 9816.84, 355.442, 1308.46, 0, 0, 0, NULL, 100, 0),
(463500, 2, 9812.42, 362.828, 1308.75, 0, 0, 0, NULL, 100, 0),
(463500, 3, 9813.47, 371.201, 1307.6, 0, 0, 0, NULL, 100, 0),
(463500, 4, 9817.41, 378.589, 1307.89, 0, 0, 0, NULL, 100, 0),
(463500, 5, 9825.73, 384.063, 1307.75, 0, 0, 0, NULL, 100, 0),
(463500, 6, 9834.48, 389.443, 1307.57, 0, 0, 0, NULL, 100, 0),
(463500, 7, 9841.24, 395.25, 1307.57, 0, 0, 0, NULL, 100, 0),
(463500, 8, 9848.03, 401.213, 1306.64, 0, 0, 0, NULL, 100, 0),
(463500, 9, 9859.21, 406.416, 1305.99, 0, 0, 0, NULL, 100, 0),
(463500, 10, 9869.07, 406.038, 1306.54, 0, 0, 0, NULL, 100, 0),
(463500, 11, 9878.76, 405.103, 1307.24, 0, 0, 0, NULL, 100, 0),
(463500, 12, 9885.53, 403.788, 1307.66, 0, 0, 0, NULL, 100, 0),
(463500, 13, 9889.62, 397.836, 1308.96, 0, 0, 0, NULL, 100, 0),
(463500, 14, 9893.75, 390.062, 1310, 0, 0, 0, NULL, 100, 0),
(463500, 15, 9896.48, 384.764, 1309.68, 0, 0, 0, NULL, 100, 0),
(463500, 16, 9893.9, 389.335, 1310, 0, 0, 0, NULL, 100, 0),
(463500, 17, 9891.63, 394.619, 1309.8, 0, 0, 0, NULL, 100, 0),
(463500, 18, 9889.41, 401.267, 1308.38, 0, 0, 0, NULL, 100, 0),
(463500, 19, 9889.07, 410.169, 1306.58, 0, 0, 0, NULL, 100, 0),
(463500, 20, 9889.35, 418.27, 1305.85, 0, 0, 0, NULL, 100, 0),
(463500, 21, 9889.54, 426.962, 1305.15, 0, 0, 0, NULL, 100, 0),
(463500, 22, 9886.55, 436.957, 1304.24, 0, 0, 0, NULL, 100, 0),
(463500, 23, 9880.4, 443.821, 1303.31, 0, 0, 0, NULL, 100, 0),
(463500, 24, 9870.83, 446.341, 1303.2, 0, 0, 0, NULL, 100, 0),
(463500, 25, 9862.34, 445.663, 1305.03, 0, 0, 0, NULL, 100, 0),
(463500, 26, 9853.18, 443.625, 1306.19, 0, 0, 0, NULL, 100, 0),
(463500, 27, 9846.1, 441.481, 1307.79, 0, 0, 0, NULL, 100, 0),
(463500, 28, 9837.69, 438.919, 1307.79, 0, 0, 0, NULL, 100, 0),
(463500, 29, 9844.11, 440.792, 1307.79, 0, 0, 0, NULL, 100, 0),
(463500, 30, 9851.25, 442.721, 1306.9, 0, 0, 0, NULL, 100, 0),
(463500, 31, 9859.72, 444.625, 1305.26, 0, 0, 0, NULL, 100, 0),
(463500, 32, 9868.76, 446.034, 1303.59, 0, 0, 0, NULL, 100, 0),
(463500, 33, 9876.38, 446.37, 1302.64, 0, 0, 0, NULL, 100, 0),
(463500, 34, 9883, 443.218, 1303.42, 0, 0, 0, NULL, 100, 0),
(463500, 35, 9886.39, 436.6, 1304.33, 0, 0, 0, NULL, 100, 0),
(463500, 36, 9888.7, 427.985, 1305.1, 0, 0, 0, NULL, 100, 0),
(463500, 37, 9888.68, 420.185, 1305.81, 0, 0, 0, NULL, 100, 0),
(463500, 38, 9886.92, 411.696, 1306.65, 0, 0, 0, NULL, 100, 0),
(463500, 39, 9881.05, 406.514, 1307.24, 0, 0, 0, NULL, 100, 0),
(463500, 40, 9872.12, 405.889, 1306.77, 0, 0, 0, NULL, 100, 0),
(463500, 41, 9863.95, 407.545, 1306.23, 0, 0, 0, NULL, 100, 0),
(463500, 42, 9857.27, 406.285, 1306.11, 0, 0, 0, NULL, 100, 0),
(463500, 43, 9850.12, 401.789, 1306.54, 0, 0, 0, NULL, 100, 0),
(463500, 44, 9843.36, 396.553, 1307.55, 0, 0, 0, NULL, 100, 0),
(463500, 45, 9836.4, 390.981, 1307.57, 0, 0, 0, NULL, 100, 0),
(463500, 46, 9828.75, 386.198, 1307.59, 0, 0, 0, NULL, 100, 0),
(463500, 47, 9820.42, 380.808, 1307.94, 0, 0, 0, NULL, 100, 0),
(463500, 48, 9814.18, 374.616, 1307.69, 0, 0, 0, NULL, 100, 0),
(463500, 49, 9812.98, 367.559, 1307.73, 0, 0, 0, NULL, 100, 0),
(463500, 50, 9812.92, 362.307, 1308.75, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (46350, 463500, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 46352 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1046143 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1046143 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1046143 AND spawnType = 0 WHERE ce.spawnID = 1046143;
INSERT INTO creature_entry (spawnID, entry) VALUES (46352, 14431);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (46352, 1, 1, 2296, 0, 10652.1, 2065.57,  1330.61, 4.3596, 7200, 0, 0, 156, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(463520, 1, 10630.1, 2031.82, 1331.57, 0, 0, 0, NULL, 100, 0),
(463520, 2, 10600.2, 2014.94, 1329.95, 0, 0, 0, NULL, 100, 0),
(463520, 3, 10555.9, 2005.13, 1327.46, 0, 0, 0, NULL, 100, 0),
(463520, 4, 10523.8, 1974.52, 1327.12, 0, 0, 0, NULL, 100, 0),
(463520, 5, 10499.3, 1950.97, 1321.42, 0, 0, 0, NULL, 100, 0),
(463520, 6, 10459.5, 1936.29, 1319.83, 0, 0, 0, NULL, 100, 0),
(463520, 7, 10420.3, 1924.04, 1320.09, 0, 0, 0, NULL, 100, 0),
(463520, 8, 10369.8, 1909.25, 1321.91, 0, 0, 0, NULL, 100, 0),
(463520, 9, 10420.3, 1924.04, 1320.09, 0, 0, 0, NULL, 100, 0),
(463520, 10, 10459.5, 1936.29, 1319.83, 0, 0, 0, NULL, 100, 0),
(463520, 11, 10499.3, 1950.97, 1321.42, 0, 0, 0, NULL, 100, 0),
(463520, 12, 10523.8, 1974.52, 1327.12, 0, 0, 0, NULL, 100, 0),
(463520, 13, 10555.9, 2005.13, 1327.46, 0, 0, 0, NULL, 100, 0),
(463520, 14, 10600.2, 2014.94, 1329.95, 0, 0, 0, NULL, 100, 0),
(463520, 15, 10630.1, 2031.82, 1331.57, 0, 0, 0, NULL, 100, 0),
(463520, 16, 10652.1, 2065.57, 1330.61, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (46352, 463520, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 46354 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1046145 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1046145 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1046145 AND spawnType = 0 WHERE ce.spawnID = 1046145;
INSERT INTO creature_entry (spawnID, entry) VALUES (46354, 14432);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (46354, 1, 1, 904, 1, 10115.9, 1167.52,  1314.17, 5.5061, 7200, 0, 1, 115, 126, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(463540, 1, 10115.9, 1167.52, 1314.17, 0, 0, 0, NULL, 100, 0),
(463540, 2, 10116.4, 1180.53, 1314.8, 0, 0, 0, NULL, 100, 0),
(463540, 3, 10115, 1189.81, 1313.65, 0, 0, 0, NULL, 100, 0),
(463540, 4, 10110.6, 1199.47, 1311.57, 0, 0, 0, NULL, 100, 0),
(463540, 5, 10103.3, 1203.54, 1311.8, 0, 0, 0, NULL, 100, 0),
(463540, 6, 10096.5, 1201.2, 1312.75, 0, 0, 0, NULL, 100, 0),
(463540, 7, 10088.9, 1196.34, 1315.41, 0, 0, 0, NULL, 100, 0),
(463540, 8, 10082.8, 1189.64, 1316.33, 0, 0, 0, NULL, 100, 0),
(463540, 9, 10076.6, 1182.24, 1316.17, 0, 0, 0, NULL, 100, 0),
(463540, 10, 10072.2, 1176.47, 1316.69, 0, 0, 0, NULL, 100, 0),
(463540, 11, 10068.8, 1169.67, 1318.99, 0, 0, 0, NULL, 100, 0),
(463540, 12, 10063.4, 1163.49, 1319.42, 0, 0, 0, NULL, 100, 0),
(463540, 13, 10057.8, 1157.82, 1319.43, 0, 0, 0, NULL, 100, 0),
(463540, 14, 10055.2, 1151.09, 1320.13, 0, 0, 0, NULL, 100, 0),
(463540, 15, 10057.8, 1157.82, 1319.43, 0, 0, 0, NULL, 100, 0),
(463540, 16, 10063.4, 1163.49, 1319.42, 0, 0, 0, NULL, 100, 0),
(463540, 17, 10068.8, 1169.67, 1318.99, 0, 0, 0, NULL, 100, 0),
(463540, 18, 10072.2, 1176.47, 1316.69, 0, 0, 0, NULL, 100, 0),
(463540, 19, 10076.6, 1182.24, 1316.17, 0, 0, 0, NULL, 100, 0),
(463540, 20, 10082.8, 1189.64, 1316.33, 0, 0, 0, NULL, 100, 0),
(463540, 21, 10088.9, 1196.34, 1315.41, 0, 0, 0, NULL, 100, 0),
(463540, 22, 10096.5, 1201.2, 1312.75, 0, 0, 0, NULL, 100, 0),
(463540, 23, 10103.3, 1203.54, 1311.8, 0, 0, 0, NULL, 100, 0),
(463540, 24, 10110.6, 1199.47, 1311.57, 0, 0, 0, NULL, 100, 0),
(463540, 25, 10115, 1189.81, 1313.65, 0, 0, 0, NULL, 100, 0),
(463540, 26, 10116.4, 1180.53, 1314.8, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (46354, 463540, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 46827 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (46827, 11942);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (46827, 1, 1, 0, 0, 9839.59, 945.991,  1307.27, 5.84685, 120, 0, 0, 1, 0, 0, 0, 5, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (46827, NULL, 0, 0, 0, 4097, 0, NULL);

-- Importing creature with guid 49525 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1049264 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1049264 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1049264 AND spawnType = 0 WHERE ce.spawnID = 1049264;
INSERT INTO creature_entry (spawnID, entry) VALUES (49525, 3562);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (49525, 1, 1, 2203, 1, 9800.17, 2528.08,  1320.35, 4.25791, 300, 0, 0, 1003, 0, 2, 0, 0, 10);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1213, 1, 30, 0, 0, 0, 0, 0, 0, 0.0349066);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1214, 1, 30, 0, 0, 0, 0, 0, 0, 4.69494);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1215, 1, 30, 0, 0, 0, 0, 0, 0, 2.53073);
INSERT INTO waypoint_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES (1216, 1, 30, 0, 0, 0, 0, 0, 0, 3.19395);
INSERT INTO waypoint_data VALUES 
(495250, 1, 9775.32, 2526.24, 1318.84, 0, 0, 0, NULL, 100, 0),
(495250, 2, 9741.11, 2526.47, 1330.59, 0, 0, 0, NULL, 100, 0),
(495250, 3, 9714.14, 2525.51, 1335.69, 0, 210000, 0, 1213, 100, 0),
(495250, 4, 9712.36, 2537.05, 1335.68, 0, 0, 0, NULL, 100, 0),
(495250, 5, 9709.46, 2550.9, 1335.68, 0, 0, 0, NULL, 100, 0),
(495250, 6, 9703.13, 2563.03, 1335.68, 0, 0, 0, NULL, 100, 0),
(495250, 7, 9698.74, 2568.85, 1335.68, 0, 0, 0, NULL, 100, 0),
(495250, 8, 9698.28, 2581.84, 1335.68, 0, 0, 0, NULL, 100, 0),
(495250, 9, 9710.37, 2592.32, 1336.72, 0, 0, 0, NULL, 100, 0),
(495250, 10, 9720.52, 2601.8, 1337.11, 0, 0, 0, NULL, 100, 0),
(495250, 11, 9732.8, 2612.01, 1335.79, 0, 0, 0, NULL, 100, 0),
(495250, 12, 9739.72, 2618.73, 1334.02, 0, 0, 0, NULL, 100, 0),
(495250, 13, 9745.11, 2629.8, 1334.3, 0, 192000, 0, 1214, 100, 0),
(495250, 14, 9742.93, 2627.36, 1334.22, 0, 0, 0, NULL, 100, 0),
(495250, 15, 9739.16, 2617.08, 1334.31, 0, 0, 0, NULL, 100, 0),
(495250, 16, 9732.11, 2611.05, 1335.79, 0, 0, 0, NULL, 100, 0),
(495250, 17, 9713.32, 2594.29, 1336.95, 0, 0, 0, NULL, 100, 0),
(495250, 18, 9703.71, 2586.42, 1335.73, 0, 0, 0, NULL, 100, 0),
(495250, 19, 9696.2, 2571.05, 1335.68, 0, 0, 0, NULL, 100, 0),
(495250, 20, 9706.78, 2555.83, 1335.68, 0, 0, 0, NULL, 100, 0),
(495250, 21, 9712.2, 2543.83, 1335.68, 0, 0, 0, NULL, 100, 0),
(495250, 22, 9713.91, 2525.54, 1335.69, 0, 0, 0, NULL, 100, 0),
(495250, 23, 9713.77, 2513.44, 1335.68, 0, 0, 0, NULL, 100, 0),
(495250, 24, 9712.47, 2502.87, 1335.68, 0, 0, 0, NULL, 100, 0),
(495250, 25, 9708.95, 2493.58, 1335.68, 0, 0, 0, NULL, 100, 0),
(495250, 26, 9703.36, 2484.98, 1335.68, 0, 0, 0, NULL, 100, 0),
(495250, 27, 9699.19, 2479.34, 1335.68, 0, 0, 0, NULL, 100, 0),
(495250, 28, 9702.04, 2469.11, 1335.68, 0, 0, 0, NULL, 100, 0),
(495250, 29, 9716.89, 2456.65, 1336.96, 0, 0, 0, NULL, 100, 0),
(495250, 30, 9732.5, 2443.68, 1336.26, 0, 0, 0, NULL, 100, 0),
(495250, 31, 9758.73, 2423.4, 1334.14, 0, 354000, 0, 1215, 100, 0),
(495250, 32, 9744.31, 2434.67, 1334.22, 0, 0, 0, NULL, 100, 0),
(495250, 33, 9737.4, 2439.93, 1335.79, 0, 0, 0, NULL, 100, 0),
(495250, 34, 9718.78, 2454.4, 1337.07, 0, 0, 0, NULL, 100, 0),
(495250, 35, 9706.88, 2464.56, 1335.72, 0, 0, 0, NULL, 100, 0),
(495250, 36, 9700.29, 2478.86, 1335.68, 0, 0, 0, NULL, 100, 0),
(495250, 37, 9708.25, 2493.77, 1335.68, 0, 0, 0, NULL, 100, 0),
(495250, 38, 9712.78, 2509.64, 1335.68, 0, 0, 0, NULL, 100, 0),
(495250, 39, 9713.75, 2519.07, 1335.69, 0, 0, 0, NULL, 100, 0),
(495250, 40, 9719.04, 2525.65, 1335.69, 0, 0, 0, NULL, 100, 0),
(495250, 41, 9734.46, 2525.73, 1333.02, 0, 0, 0, NULL, 100, 0),
(495250, 42, 9750.11, 2526.17, 1327.45, 0, 0, 0, NULL, 100, 0),
(495250, 43, 9759.05, 2526.22, 1324.19, 0, 0, 0, NULL, 100, 0),
(495250, 44, 9774.12, 2527, 1319.15, 0, 0, 0, NULL, 100, 0),
(495250, 45, 9792.75, 2526.98, 1320.01, 0, 0, 0, NULL, 100, 0),
(495250, 46, 9809.65, 2527.63, 1320.68, 0, 0, 0, NULL, 100, 0),
(495250, 47, 9829.67, 2527.95, 1321.16, 0, 0, 0, NULL, 100, 0),
(495250, 48, 9846.95, 2528.68, 1321.45, 0, 0, 0, NULL, 100, 0),
(495250, 49, 9865.62, 2529.46, 1320.17, 0, 0, 0, NULL, 100, 0),
(495250, 50, 9883.95, 2529.66, 1318.92, 0, 0, 0, NULL, 100, 0),
(495250, 51, 9899.61, 2529.96, 1315.11, 0, 0, 0, NULL, 100, 0),
(495250, 52, 9923.5, 2529.91, 1319.05, 0, 256000, 0, 1216, 100, 0),
(495250, 53, 9900.18, 2530.35, 1315.3, 0, 0, 0, NULL, 100, 0),
(495250, 54, 9871.44, 2529.47, 1319.77, 0, 0, 0, NULL, 100, 0),
(495250, 55, 9850.28, 2529.3, 1321.22, 0, 0, 0, NULL, 100, 0),
(495250, 56, 9825.11, 2528.36, 1321.05, 0, 0, 0, NULL, 100, 0),
(495250, 57, 9800.17, 2528.08, 1320.35, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (49525, 495250, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 49970 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1049686 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1049686 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1049686 AND spawnType = 0 WHERE ce.spawnID = 1049686;
INSERT INTO creature_entry (spawnID, entry) VALUES (49970, 12803);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (49970, 1, 1, 12683, 0, -5704.74, 3379.41,  63.0866, 3.21747, 600, 15, 0, 323700, 0, 1, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (49970, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO spawn_group VALUES (2, 0, 49970);

-- Importing creature with guid 86641 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1085358 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1085358 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1085358 AND spawnType = 0 WHERE ce.spawnID = 1085358;
INSERT INTO creature_entry (spawnID, entry) VALUES (86641, 25945);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86641, 1, 1, 16334, 0, -5519.48, -2294.23,  -57.9951, 3.1765, 300, 0, 0, 1848, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (86641, NULL, 0, 0, 0, 4097, 0, NULL);
INSERT INTO game_event_creature (guid, event) VALUES (86641, 1);

-- Importing creature with guid 150185 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1096571 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1096571 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1096571 AND spawnType = 0 WHERE ce.spawnID = 1096571;
INSERT INTO creature_entry (spawnID, entry) VALUES (150185, 15440);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (150185, 1, 1, 0, 1, -7168.18, 1380.71,  2.91784, 1.09484, 540, 0, 0, 1, 0, 0, 0, 0, 10);

-- Importing creature with guid 137685 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (137685, 15218);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (137685, 1, 1, 0, 0, -1324.94, 87.0497,  129.79, 4.85202, 120, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (137685, 5);

-- Importing creature with guid 200145 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (200145, 14232);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (200145, 1, 1, 0, 0, -2714.64, -3489.85,  34.85, 4.46, 300, 5, 0, 50, 0, 1, 0, 0, 10);

-- Importing creature with guid 207171 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (207171, 27216);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (207171, 1, 1, 0, 0, 1197.28, -4355.3,  21.3794, 0.261799, 120, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (207171, 26);

-- Importing creature with guid 88156 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (88156, 20278);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (88156, 1, 1, 18290, 0, -7119.02, -3779.06,  8.78404, 0, 180, 0, 0, 1, 0, 0, 0, 5, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (88156, NULL, 0, 0, 0, 1, 0, NULL);
INSERT INTO game_event_creature (guid, event) VALUES (88156, 75);

-- Importing creature with guid 88163 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (88163, 11701);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (88163, 1, 1, 11667, 1, -7784.26, -2101.23,  -270.042, 1.31686, 180, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (88163, NULL, 29102, 0, 0, 257, 0, NULL);

-- Importing creature with guid 88298 with import type LK_ONLY
INSERT INTO creature_entry (spawnID, entry) VALUES (88298, 7772);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (88298, 1, 1, 0, 1, -3285, 2003.85,  245.986, 2.584, 900, 0, 0, 7900, 2434, 0, 0, 5, 10);

-- Importing creature with guid 40470 with import type MOVE_UNIQUE_IMPORT_WP
DELETE ce, c1, c2, sg FROM creature_entry ce LEFT JOIN conditions c1 ON c1.ConditionValue3 = 1095321 AND c1.ConditionTypeOrReference = 31 LEFT JOIN conditions c2 ON c1.SourceEntry = -1095321 AND c2.SourceTypeOrReferenceId = 22 LEFT JOIN spawn_group sg ON sg.spawnID = 1095321 AND spawnType = 0 WHERE ce.spawnID = 1095321;
INSERT INTO creature_entry (spawnID, entry) VALUES (40470, 18686);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (40470, 530, 1, 0, 0, -2847.33, 3195.11,  7.4167, 0.091094, 300, 0, 0, 9144, 13525, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(404700, 1, -2843.86, 3205.97, 5.45323, 0, 0, 0, NULL, 100, 0),
(404700, 2, -2829.24, 3218.81, 9.59048, 0, 0, 0, NULL, 100, 0),
(404700, 3, -2814.12, 3235.72, 10.1736, 0, 0, 0, NULL, 100, 0),
(404700, 4, -2792.54, 3259.91, 5.91885, 0, 0, 0, NULL, 100, 0),
(404700, 5, -2774.46, 3280.17, 2.8828, 0, 0, 0, NULL, 100, 0),
(404700, 6, -2751.21, 3306.24, 0.199384, 0, 0, 0, NULL, 100, 0),
(404700, 7, -2728.55, 3318.21, -0.833018, 0, 0, 0, NULL, 100, 0),
(404700, 8, -2719.59, 3331.35, -1.04936, 0, 0, 0, NULL, 100, 0),
(404700, 9, -2708.94, 3349.74, -0.819842, 0, 0, 0, NULL, 100, 0),
(404700, 10, -2694.72, 3374.29, -0.156626, 0, 0, 0, NULL, 100, 0),
(404700, 11, -2685.08, 3390.93, 0.328583, 0, 0, 0, NULL, 100, 0),
(404700, 12, -2675.69, 3407.16, 0.474826, 0, 0, 0, NULL, 100, 0),
(404700, 13, -2669.68, 3416.29, 0.743526, 0, 0, 0, NULL, 100, 0),
(404700, 14, -2660.96, 3425.2, 0.805554, 0, 0, 0, NULL, 100, 0),
(404700, 15, -2654.25, 3430.82, 0.77502, 0, 0, 0, NULL, 100, 0),
(404700, 16, -2645.46, 3437.83, 1.28666, 0, 0, 0, NULL, 100, 0),
(404700, 17, -2639.59, 3442.5, 3.25542, 0, 0, 0, NULL, 100, 0),
(404700, 18, -2630.71, 3449.41, 1.86245, 0, 0, 0, NULL, 100, 0),
(404700, 19, -2621.54, 3456.21, 0.653284, 0, 0, 0, NULL, 100, 0),
(404700, 20, -2617.41, 3460.51, 0.593326, 0, 0, 0, NULL, 100, 0),
(404700, 21, -2611.5, 3471.52, 0.520041, 0, 0, 0, NULL, 100, 0),
(404700, 22, -2610.04, 3475.69, 0.53993, 0, 0, 0, NULL, 100, 0),
(404700, 23, -2606.9, 3480.85, 0.492544, 0, 0, 0, NULL, 100, 0),
(404700, 24, -2590.41, 3502.84, 0.225892, 0, 0, 0, NULL, 100, 0),
(404700, 25, -2582.03, 3512, 0.101641, 0, 0, 0, NULL, 100, 0),
(404700, 26, -2566.36, 3522.06, 0.189451, 0, 0, 0, NULL, 100, 0),
(404700, 27, -2534.38, 3523.07, 1.3404, 0, 0, 0, NULL, 100, 0),
(404700, 28, -2523.33, 3527.83, 1.66667, 0, 0, 0, NULL, 100, 0),
(404700, 29, -2516.31, 3533.95, 1.65006, 0, 0, 0, NULL, 100, 0),
(404700, 30, -2507.47, 3544.67, 1.56155, 0, 0, 0, NULL, 100, 0),
(404700, 31, -2491.83, 3549.68, 1.43199, 0, 0, 0, NULL, 100, 0),
(404700, 32, -2471.56, 3551.98, 1.23732, 0, 0, 0, NULL, 100, 0),
(404700, 33, -2463.14, 3555.66, 1.03546, 0, 0, 0, NULL, 100, 0),
(404700, 34, -2456.3, 3555.25, 0.942503, 0, 0, 0, NULL, 100, 0),
(404700, 35, -2452.53, 3537.02, 0.402467, 0, 0, 0, NULL, 100, 0),
(404700, 36, -2446.32, 3528.59, -0.236494, 0, 0, 0, NULL, 100, 0),
(404700, 37, -2462.93, 3549.41, 1.08819, 0, 0, 0, NULL, 100, 0),
(404700, 38, -2460.92, 3557.5, 1.00403, 0, 0, 0, NULL, 100, 0),
(404700, 39, -2425.49, 3596.2, 0.274173, 0, 0, 0, NULL, 100, 0),
(404700, 40, -2405.2, 3613.88, 0.217946, 0, 0, 0, NULL, 100, 0),
(404700, 41, -2391.31, 3632.83, 0.792431, 0, 0, 0, NULL, 100, 0),
(404700, 42, -2383.97, 3658.54, 0.99065, 0, 0, 0, NULL, 100, 0),
(404700, 43, -2351.54, 3700.72, 0.420721, 0, 0, 0, NULL, 100, 0),
(404700, 44, -2314.59, 3729.83, 0.0297019, 0, 0, 0, NULL, 100, 0),
(404700, 45, -2296.57, 3749.89, 0.363868, 0, 0, 0, NULL, 100, 0),
(404700, 46, -2285.44, 3767.7, 0.291327, 0, 0, 0, NULL, 100, 0),
(404700, 47, -2279.3, 3787.95, 0.314858, 0, 0, 0, NULL, 100, 0),
(404700, 48, -2276.3, 3799.32, 0.521091, 0, 0, 0, NULL, 100, 0),
(404700, 49, -2254.61, 3831.13, 2.70024, 0, 0, 0, NULL, 100, 0),
(404700, 50, -2251.75, 3848.92, 5.15061, 0, 0, 0, NULL, 100, 0),
(404700, 51, -2249.52, 3869.55, 2.39354, 0, 0, 0, NULL, 100, 0),
(404700, 52, -2244.66, 3893.46, 1.58335, 0, 0, 0, NULL, 100, 0),
(404700, 53, -2239.88, 3904.19, 1.10899, 0, 0, 0, NULL, 100, 0),
(404700, 54, -2224.28, 3927.45, 0.706091, 0, 0, 0, NULL, 100, 0),
(404700, 55, -2187.45, 3958.22, 0.921813, 0, 0, 0, NULL, 100, 0),
(404700, 56, -2175.09, 3978.98, 0.929392, 0, 0, 0, NULL, 100, 0),
(404700, 57, -2171.14, 4011.04, 0.450247, 0, 0, 0, NULL, 100, 0),
(404700, 58, -2175.93, 4027.88, 0.519539, 0, 0, 0, NULL, 100, 0),
(404700, 59, -2187.96, 4064.43, 1.4281, 0, 0, 0, NULL, 100, 0),
(404700, 60, -2166.44, 4102.49, 4.27243, 0, 0, 0, NULL, 100, 0),
(404700, 61, -1700.15, 3993.57, 55.6859, 0, 0, 0, NULL, 100, 0),
(404700, 62, -1710.59, 4034.97, 60.6101, 0, 0, 0, NULL, 100, 0),
(404700, 63, -1719.95, 4059.83, 62.7452, 0, 0, 0, NULL, 100, 0),
(404700, 64, -1718.67, 4104.32, 64.3796, 0, 0, 0, NULL, 100, 0),
(404700, 65, -1743.77, 4151.6, 62.3929, 0, 0, 0, NULL, 100, 0),
(404700, 66, -1754.61, 4173.29, 58.3691, 0, 0, 0, NULL, 100, 0),
(404700, 67, -1788.41, 4206.51, 44.1839, 0, 0, 0, NULL, 100, 0),
(404700, 68, -1818.93, 4214.12, 33.1137, 0, 0, 0, NULL, 100, 0),
(404700, 69, -1841.51, 4228.1, 22.0741, 0, 0, 0, NULL, 100, 0),
(404700, 70, -1871.54, 4263.5, 8.22153, 0, 0, 0, NULL, 100, 0),
(404700, 71, -1891, 4295.91, 3.23019, 0, 0, 0, NULL, 100, 0),
(404700, 72, -1911.96, 4351.49, 1.66825, 0, 0, 0, NULL, 100, 0),
(404700, 73, -1910.12, 4389.42, 1.77078, 0, 0, 0, NULL, 100, 0),
(404700, 74, -1907.67, 4417.31, 2.99456, 0, 0, 0, NULL, 100, 0),
(404700, 75, -1917.63, 4452.07, 6.27493, 0, 0, 0, NULL, 100, 0),
(404700, 76, -1929.4, 4473.47, 8.90937, 0, 0, 0, NULL, 100, 0),
(404700, 77, -1942.74, 4489.67, 11.1029, 0, 0, 0, NULL, 100, 0),
(404700, 78, -1958.47, 4508.45, 12.7432, 0, 0, 0, NULL, 100, 0),
(404700, 79, -1975.36, 4519.94, 13.1616, 0, 0, 0, NULL, 100, 0),
(404700, 80, -1982.19, 4524.49, 13.0993, 0, 0, 0, NULL, 100, 0),
(404700, 81, -1983.38, 4507.03, 12.7591, 0, 0, 0, NULL, 100, 0),
(404700, 82, -1984.02, 4496.55, 11.9535, 0, 0, 0, NULL, 100, 0),
(404700, 83, -1984.87, 4482.57, 11.1499, 0, 0, 0, NULL, 100, 0),
(404700, 84, -1978.92, 4509.87, 12.6695, 0, 0, 0, NULL, 100, 0),
(404700, 85, -1978.12, 4527.35, 13.36, 0, 0, 0, NULL, 100, 0),
(404700, 86, -1967.33, 4546.79, 12.8741, 0, 0, 0, NULL, 100, 0),
(404700, 87, -1953.04, 4566.47, 10.1123, 0, 0, 0, NULL, 100, 0),
(404700, 88, -1946.5, 4582.91, 7.44598, 0, 0, 0, NULL, 100, 0),
(404700, 89, -1948.3, 4599.58, 4.61956, 0, 0, 0, NULL, 100, 0),
(404700, 90, -1955.08, 4637.48, -0.0588505, 0, 0, 0, NULL, 100, 0),
(404700, 91, -1943.63, 4685.04, -2.18987, 0, 0, 0, NULL, 100, 0),
(404700, 92, -1941.99, 4705.25, -2.67768, 0, 0, 0, NULL, 100, 0),
(404700, 93, -1945.22, 4727.47, -3.22917, 0, 0, 0, NULL, 100, 0),
(404700, 94, -1951.89, 4748.79, -2.9388, 0, 0, 0, NULL, 100, 0),
(404700, 95, -1957.77, 4773.7, -1.42409, 0, 0, 0, NULL, 100, 0),
(404700, 96, -1957.36, 4798.11, -0.0807409, 0, 0, 0, NULL, 100, 0),
(404700, 97, -1944.66, 4829.17, 0.67, 0, 0, 0, NULL, 100, 0),
(404700, 98, -1929.68, 4851.09, 1.64973, 0, 0, 0, NULL, 100, 0),
(404700, 99, -1918.23, 4883.89, 2.11937, 0, 0, 0, NULL, 100, 0),
(404700, 100, -1901.18, 4906.49, -1.12705, 0, 0, 0, NULL, 100, 0),
(404700, 101, -1881.87, 4926.69, -9.60835, 0, 0, 0, NULL, 100, 0),
(404700, 102, -1870.05, 4946.21, -17.0149, 0, 0, 0, NULL, 100, 0),
(404700, 103, -1866.49, 4960.25, -20.2899, 0, 0, 0, NULL, 100, 0),
(404700, 104, -1860.94, 5033.3, -26.0991, 0, 0, 0, NULL, 100, 0),
(404700, 105, -1869.8, 4962.46, -20.3781, 0, 0, 0, NULL, 100, 0),
(404700, 106, -1884.86, 4924.67, -8.30171, 0, 0, 0, NULL, 100, 0),
(404700, 107, -1904.83, 4905.16, -0.202733, 0, 0, 0, NULL, 100, 0),
(404700, 108, -1922.67, 4912.5, 6.06643, 0, 0, 0, NULL, 100, 0),
(404700, 109, -1937.46, 4934.99, 14.8054, 0, 0, 0, NULL, 100, 0),
(404700, 110, -1953.22, 4961.69, 24.7149, 0, 0, 0, NULL, 100, 0),
(404700, 111, -1958.07, 4992.99, 30.1767, 0, 0, 0, NULL, 100, 0),
(404700, 112, -1958.74, 5013.42, 32.4293, 0, 0, 0, NULL, 100, 0),
(404700, 113, -1947.03, 5042.52, 25.2899, 0, 0, 0, NULL, 100, 0),
(404700, 114, -1942.58, 5058.03, 19.5292, 0, 0, 0, NULL, 100, 0),
(404700, 115, -1963.32, 5083.08, 11.1075, 0, 0, 0, NULL, 100, 0),
(404700, 116, -1984.5, 5100.08, 7.82778, 0, 0, 0, NULL, 100, 0),
(404700, 117, -2004.59, 5114.07, 8.26657, 0, 0, 0, NULL, 100, 0),
(404700, 118, -2029.48, 5123.26, 8.48839, 0, 0, 0, NULL, 100, 0),
(404700, 119, -2055.94, 5125.38, 7.48301, 0, 0, 0, NULL, 100, 0),
(404700, 120, -2089.33, 5115.44, 3.14184, 0, 0, 0, NULL, 100, 0),
(404700, 121, -2112.53, 5107.32, -2.40737, 0, 0, 0, NULL, 100, 0),
(404700, 122, -2137.4, 5108.51, -11.5709, 0, 0, 0, NULL, 100, 0),
(404700, 123, -2155.69, 5115.97, -18.1534, 0, 0, 0, NULL, 100, 0),
(404700, 124, -2170.6, 5125.13, -21.2824, 0, 0, 0, NULL, 100, 0),
(404700, 125, -2182.81, 5131.87, -21.667, 0, 0, 0, NULL, 100, 0),
(404700, 126, -2198.88, 5138.77, -21.2323, 0, 0, 0, NULL, 100, 0),
(404700, 127, -2219.62, 5144.57, -19.6848, 0, 0, 0, NULL, 100, 0),
(404700, 128, -2238.57, 5146.29, -17.4706, 0, 0, 0, NULL, 100, 0),
(404700, 129, -2266.45, 5144.26, -13.0598, 0, 0, 0, NULL, 100, 0),
(404700, 130, -2290.86, 5142.23, -9.17377, 0, 0, 0, NULL, 100, 0),
(404700, 131, -2308.35, 5142.09, -7.11532, 0, 0, 0, NULL, 100, 0),
(404700, 132, -2333.72, 5149.44, -5.0165, 0, 0, 0, NULL, 100, 0),
(404700, 133, -2365.45, 5157.88, -2.8501, 0, 0, 0, NULL, 100, 0),
(404700, 134, -2390.26, 5158.22, -1.19618, 0, 0, 0, NULL, 100, 0),
(404700, 135, -2417.84, 5153.35, 0.775549, 0, 0, 0, NULL, 100, 0),
(404700, 136, -2442.28, 5142.04, 3.53141, 0, 0, 0, NULL, 100, 0),
(404700, 137, -2469.81, 5120.56, 6.76948, 0, 0, 0, NULL, 100, 0),
(404700, 138, -2498.06, 5115.06, 7.87731, 0, 0, 0, NULL, 100, 0),
(404700, 139, -2532.85, 5118.89, 7.54034, 0, 0, 0, NULL, 100, 0),
(404700, 140, -2555.6, 5120.59, 7.69492, 0, 0, 0, NULL, 100, 0),
(404700, 141, -2843.86, 3205.97, 5.45323, 0, 0, 0, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (40470, 404700, 0, 0, 0, 0, 0, NULL);

-- Importing creature with guid 77647 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (77647, 3652);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (77647, 1, 1, 0, 0, -661.625, -2221.64,  14.8407, 5.37196, 72000, 0, 0, 0, 0, 0, 0, 0, 10);

-- Importing creature with guid 77646 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (77646, 10238);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (77646, 1, 1, 0, 0, -1731.53, 941.007,  91.1069, 1.10427, 72000, 8, 0, 0, 0, 1, 0, 0, 10);

-- Importing creature with guid 74034 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (74034, 10236);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (74034, 1, 1, 0, 0, -987.782, 907.594,  91.8254, 1.33284, 72000, 8, 0, 0, 0, 1, 0, 0, 10);

-- Importing creature with guid 152120 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (152120, 24495);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (152120, 1, 1, 0, 0, 1174.58, -4292.92,  21.3573, 6.24828, 120, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (152120, 26);

-- Importing creature with guid 45758 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (45758, 11447);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (45758, 1, 1, 0, 0, -3799.62, 1063.83,  132.806, 1.20424, 21600, 0, 0, 60000, 0, 0, 0, 0, 10);

-- Importing creature with guid 45759 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (45759, 11497);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (45759, 1, 1, 0, 0, -3731.22, 1061.32,  132.345, 1.7815, 21600, 0, 0, 73000, 24340, 0, 0, 0, 10);

-- Importing creature with guid 45760 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (45760, 11498);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (45760, 1, 1, 0, 1, -3690.96, 1077.14,  131.969, 2.65172, 21600, 0, 0, 57000, 0, 0, 0, 0, 10);

-- Importing creature with guid 43494 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (43494, 5435);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (43494, 1, 1, 0, 0, -1144.25, -4336.33,  -8.45723, 4.60283, 900, 4, 0, 741, 0, 1, 0, 0, 10);

-- Importing creature with guid 134677 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (134677, 24527);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (134677, 1, 1, 0, 0, 795.14, -4522.59,  6.31052, 5.20636, 300, 0, 0, 1004, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (134677, 26);

-- Importing creature with guid 69711 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (69711, 22843);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (69711, 530, 1, 21119, 0, -253.422, 5430.91,  28.9293, 2.59318, 120, 0, 0, 4059, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(697110, 1, -253.422, 5430.91, 28.9293, 0, 0, 1, NULL, 100, 0),
(697110, 2, -252.237, 5429.65, 28.7579, 0, 0, 1, NULL, 100, 0),
(697110, 3, -246.087, 5428.44, 29.5424, 0, 0, 1, NULL, 100, 0),
(697110, 4, -239.405, 5431.1, 29.5424, 0, 0, 1, NULL, 100, 0),
(697110, 5, -235.548, 5436.21, 30.9035, 0, 0, 1, NULL, 100, 0),
(697110, 6, -235.02, 5442.25, 30.8757, 0, 0, 1, NULL, 100, 0),
(697110, 7, -238.777, 5445.63, 32.0702, 0, 0, 1, NULL, 100, 0),
(697110, 8, -246.55, 5445.43, 30.8202, 0, 0, 1, NULL, 100, 0),
(697110, 9, -254.38, 5440.28, 29.6793, 0, 0, 1, NULL, 100, 0),
(697110, 10, -255.29, 5432.61, 28.9293, 0, 0, 1, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (69711, 697110, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 69712 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (69712, 22842);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (69712, 530, 1, 21119, 0, -231.478, 5442.25,  29.7544, 0.62634, 120, 0, 0, 50, 0, 2, 0, 0, 10);
INSERT INTO waypoint_data VALUES 
(697120, 1, -231.478, 5442.25, 29.7544, 0, 0, 1, NULL, 100, 0),
(697120, 2, -231.907, 5441.81, 29.406, 0, 0, 1, NULL, 100, 0),
(697120, 3, -233.547, 5436.12, 30.5861, 0, 0, 1, NULL, 100, 0),
(697120, 4, -235.965, 5433.31, 31.3361, 0, 0, 1, NULL, 100, 0),
(697120, 5, -245.663, 5430.38, 32.447, 0, 0, 1, NULL, 100, 0),
(697120, 6, -254.214, 5432.43, 32.5025, 0, 0, 1, NULL, 100, 0),
(697120, 7, -257.63, 5440.65, 31.3655, 0, 0, 1, NULL, 100, 0),
(697120, 8, -250.052, 5446.8, 30.1432, 0, 0, 1, NULL, 100, 0),
(697120, 9, -244.96, 5449.01, 29.6711, 0, 0, 1, NULL, 100, 0),
(697120, 10, -237.615, 5448.88, 29.7544, 0, 0, 1, NULL, 100, 0);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (69712, 697120, 0, 0, 0, 1, 0, NULL);

-- Importing creature with guid 87945 with import type REPLACE_ALL
INSERT INTO creature_entry (spawnID, entry) VALUES (87945, 25866);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (87945, 1, 1, 0, 0, 3886.6, 771.658,  5.00204, 3.78625, 300, 30, 0, 42, 0, 1, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (87945, 1);
INSERT INTO spawn_group VALUES (2, 0, 87945);
INSERT INTO creature_entry (spawnID, entry) VALUES (87946, 25866);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (87946, 1, 1, 0, 0, 3873.34, 786.567,  3.15877, 1.19364, 300, 30, 0, 42, 0, 1, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (87946, 1);
INSERT INTO spawn_group VALUES (2, 0, 87946);
INSERT INTO creature_entry (spawnID, entry) VALUES (87947, 25866);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (87947, 1, 1, 0, 0, 3886.25, 820.325,  1.23753, 1.74656, 300, 30, 0, 42, 0, 1, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (87947, 1);
INSERT INTO spawn_group VALUES (2, 0, 87947);
INSERT INTO creature_entry (spawnID, entry) VALUES (87948, 25866);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (87948, 1, 1, 0, 0, 3922.2, 795.453,  9.05771, 3.07546, 300, 30, 0, 42, 0, 1, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (87948, 1);
INSERT INTO spawn_group VALUES (2, 0, 87948);
INSERT INTO creature_entry (spawnID, entry) VALUES (87954, 25866);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (87954, 1, 1, 0, 0, 3900.86, 799.587,  7.80664, 1.37978, 300, 30, 0, 42, 0, 1, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (87954, 1);
INSERT INTO spawn_group VALUES (2, 0, 87954);
INSERT INTO creature_entry (spawnID, entry) VALUES (87955, 25866);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (87955, 1, 1, 0, 0, 3950.07, 846.313,  7.9843, 4.16433, 300, 30, 0, 42, 0, 1, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (87955, 1);
INSERT INTO spawn_group VALUES (2, 0, 87955);
INSERT INTO creature_entry (spawnID, entry) VALUES (87956, 25866);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (87956, 1, 1, 0, 0, 3982.71, 816.306,  8.04045, 4.0481, 300, 30, 0, 42, 0, 1, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (87956, 1);
INSERT INTO spawn_group VALUES (2, 0, 87956);
INSERT INTO creature_entry (spawnID, entry) VALUES (87957, 25866);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (87957, 1, 1, 0, 0, 3918.32, 760.473,  7.77712, 1.94087, 300, 30, 0, 42, 0, 1, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (87957, 1);
INSERT INTO spawn_group VALUES (2, 0, 87957);

-- Importing creature with guid 87946 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 87947 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 87948 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 87949 with import type REPLACE_ALL
INSERT INTO creature_entry (spawnID, entry) VALUES (87949, 25863);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (87949, 1, 1, 0, 0, 3956.09, 767.42,  7.88992, 5.17483, 300, 30, 0, 42, 0, 1, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (87949, 1);
INSERT INTO spawn_group VALUES (2, 0, 87949);
INSERT INTO creature_entry (spawnID, entry) VALUES (87950, 25863);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (87950, 1, 1, 0, 0, 3988.96, 790.529,  6.19678, 0.208762, 300, 30, 0, 42, 0, 1, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (87950, 1);
INSERT INTO spawn_group VALUES (2, 0, 87950);
INSERT INTO creature_entry (spawnID, entry) VALUES (87951, 25863);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (87951, 1, 1, 0, 0, 3899.13, 764.193,  6.01616, 5.62487, 300, 30, 0, 42, 0, 1, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (87951, 1);
INSERT INTO spawn_group VALUES (2, 0, 87951);
INSERT INTO creature_entry (spawnID, entry) VALUES (87952, 25863);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (87952, 1, 1, 0, 0, 3923.53, 840.173,  2.29151, 2.07879, 300, 30, 0, 42, 0, 1, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (87952, 1);
INSERT INTO spawn_group VALUES (2, 0, 87952);
INSERT INTO creature_entry (spawnID, entry) VALUES (87958, 25863);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (87958, 1, 1, 0, 0, 3922.62, 811.827,  7.79205, 2.56369, 300, 0, 0, 42, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (87958, 1);
INSERT INTO spawn_group VALUES (2, 0, 87958);

-- Importing creature with guid 87950 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 87951 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 87952 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 87953 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (87953, 25924);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (87953, 1, 1, 0, 0, 3916.05, 783.268,  9.0577, 0.717698, 300, 0, 0, 44, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (87953, 1);
INSERT INTO spawn_group VALUES (2, 0, 87953);

-- Importing creature with guid 87954 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 87955 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 87956 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 87957 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 87958 with import type REPLACE_ALL
-- Already batch replaced

-- Importing creature with guid 87959 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (87959, 25949);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (87959, 1, 1, 0, 0, 4196.1, 1172.44,  6.68073, 0.688985, 300, 0, 0, 486, 1357, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (87959, 1);

-- Importing creature with guid 86426 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (86426, 25936);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86426, 1, 1, 0, 0, -2321.77, -614.483,  -9.27059, 5.80089, 300, 0, 0, 484, 0, 0, 0, 0, 10);
INSERT INTO game_event_creature (guid, event) VALUES (86426, 1);

-- Importing creature with guid 77906 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (77906, 14341);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (77906, 1, 1, 0, 1, 5552.09, -718.221,  340.851, 1.64917, 72000, 8, 0, 0, 0, 1, 0, 0, 10);

-- Importing creature with guid 81132 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (81132, 14231);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (81132, 1, 1, 0, 0, -2800.88, -3121.34,  28.9155, 2.29019, 300, 8, 0, 1537, 0, 1, 0, 0, 10);

-- Importing creature with guid 81133 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (81133, 14236);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (81133, 1, 1, 0, 0, -4252.79, -3854.69,  -7.13676, 3.38975, 300, 8, 0, 1604, 0, 1, 0, 0, 10);

-- Importing creature with guid 81134 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (81134, 14235);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (81134, 1, 1, 0, 0, -3985.36, -3723.25,  41.8631, 4.24192, 300, 5, 0, 1829, 0, 1, 0, 0, 10);

-- Importing creature with guid 86813 with import type REPLACE_ALL
INSERT INTO creature_entry (spawnID, entry) VALUES (86813, 5194);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (86813, 1, 1, 0, 0, 2163.92, -4651.9,  50.6112, 3.61204, 180, 0, 0, 0, 0, 0, 0, 0, 10);

-- Importing creature with guid 91691 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (91691, 8387);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (91691, 1, 1, 0, 0, 1747.24, -5859.6,  -90.3817, 3.98543, 300, 0, 0, 1, 0, 0, 0, 0, 10);

-- Importing creature with guid 91692 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (91692, 8388);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (91692, 1, 1, 0, 0, 1747.62, -5862.27,  -90.9249, 3.17332, 300, 0, 0, 1, 0, 0, 0, 0, 10);

-- Importing creature with guid 91693 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (91693, 8389);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (91693, 1, 1, 0, 0, 1748.87, -5863.46,  -90.2665, 2.83088, 300, 0, 0, 1, 0, 0, 0, 0, 10);

-- Importing creature with guid 91694 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (91694, 8394);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (91694, 1, 1, 0, 0, 1746.45, -5861.29,  -91.4165, 3.20841, 300, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (91694, NULL, 0, 0, 0, 1, 0, 66515);

-- Importing creature with guid 91695 with import type MOVE_UNIQUE_IMPORT_WP
INSERT INTO creature_entry (spawnID, entry) VALUES (91695, 8478);
INSERT INTO creature (spawnID, map, spawnMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, MovementType, unit_flags, patch_min, patch_max) VALUES (91695, 1, 1, 0, 0, 1916.44, -5733.37,  10.1936, 4.10397, 300, 0, 0, 1, 0, 0, 0, 0, 10);
INSERT INTO creature_addon (spawnID, path_id, mount, bytes0, bytes1, bytes2, emote, auras) VALUES (91695, NULL, 0, 0, 8, 0, 0, NULL);

-- Formations...
REPLACE INTO creature_formations VALUES 
(NULL, 32617, 0, 0, 2, 1, 0),
(32617, 32762, 3, 1.0471975511966, 2, 1, 0),
(32617, 32728, 3, 5.7595865315813, 2, 1, 0),
(NULL, 21388, 0, 0, 2, 1, 0),
(21388, 21677, 3, 1.5707963267949, 2, 1, 0),
(21388, 21678, 6, 0, 2, 1, 0),
(21388, 21679, 3, 4.7123889803847, 2, 1, 0),
(NULL, 20720, 0, 0, 2, 1, 0),
(20720, 14007, 3, 5.7595865315813, 2, 1, 0),
(20720, 20588, 3, 1.0471975511966, 2, 1, 0),
(NULL, 15144, 0, 0, 2, 1, 0),
(15144, 15135, 15, 0, 2, 1, 0),
(15144, 15142, 12, 4.7123889803847, 2, 1, 0),
(15144, 15141, 9, 1.0471975511966, 2, 1, 0),
(NULL, 15139, 0, 0, 2, 1, 0),
(15139, 15137, 15, 0, 2, 1, 0),
(15139, 15138, 12, 0.78539816339745, 2, 1, 0),
(15139, 15140, 9, 5.4977871437821, 2, 1, 0),
(NULL, 13990, 0, 0, 2, 1, 0),
(13990, 13991, 3, 1.3962634015955, 2, 1, 0),
(13990, 13992, 3, 4.8869219055841, 2, 1, 0),
(NULL, 66605, 0, 0, 2, 1, 0),
(66605, 66606, 2, 4.7123889803847, 2, 1, 0);
