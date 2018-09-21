
UPDATE creature_template SET unit_flags = 33555200 WHERE entry = 15873;

UPDATE conditions SET ConditionValue2 = 180772 WHERE id IN (
273,
274,
275,
276,
277,
278,
279,
280,
281,
282,
283,
284);

replace into `conditions` (id, `SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) values(4261, '13','1','29437','0','0','31','0','5','181288','0','0','0','0','','Spell 29437, requires midsummer bonfire GO');

replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81501', '181288', '1', '1', '145.521', '-4713.82', '18.129', '-2.53', '0', '0', '-0.953607', '0.301053', '120', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81502', '181288', '1', '1', '-7122.51', '-3657.11', '8.82202', '-1.74533', '0', '0', '-0.766045', '0.642787', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81503', '181288', '1', '1', '6855.99', '-4564.4', '708.51', '0.855211', '0', '0', '0.414693', '0.909961', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81504', '181288', '1', '1', '-5513.93', '-2299.73', '-58.0752', '2.44346', '0', '0', '0.939692', '0.342021', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81505', '181288', '1', '1', '-273.242', '-2662.82', '91.695', '-1.8675', '0', '0', '-0.803856', '0.594824', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81506', '181288', '1', '1', '952.992', '776.968', '104.474', '-1.55334', '0', '0', '-0.700908', '0.713252', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81507', '181288', '1', '1', '-2329.42', '-624.806', '-8.27507', '5.5', '0', '0', '0.381661', '-0.924302', '0', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81508', '181288', '1', '1', '-4573.22', '407.388', '41.5461', '2.46091', '0', '0', '0.942641', '0.333809', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81509', '181288', '1', '1', '-1862.36', '3055.71', '0.744157', '2.49582', '0', '0', '0.948324', '0.317305', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81510', '181288', '1', '1', '-3110.59', '-2722.41', '33.4626', '0.226893', '0', '0', '0.113203', '0.993572', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81511', '181288', '1', '1', '2014.65', '-2337.84', '89.5211', '2.37365', '0', '0', '0.927184', '0.374606', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81512', '181288', '530', '1', '41.2448', '2587.44', '68.3453', '-2.28638', '0', '0', '-0.909961', '0.414694', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81513', '181288', '530', '1', '-3059.17', '2374.85', '63.1011', '2.03334', '0', '0', '0.850361', '0.526201', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81514', '181288', '530', '1', '2922.17', '3690.15', '143.809', '5.94709', '0', '0', '0.167258', '-0.985913', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81515', '181288', '530', '1', '-2553.32', '4277.61', '20.614', '-1.36136', '0', '0', '-0.629322', '0.777145', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81516', '181288', '530', '1', '200.9', '7686.96', '22.508', '-0.506145', '0', '0', '-0.25038', '0.968148', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81517', '181288', '530', '1', '-1211.01', '7474.44', '21.9953', '-2.02458', '0', '0', '-0.848048', '0.52992', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81518', '181288', '530', '1', '2282.43', '6134.5', '136.337', '6.01364', '0', '0', '0.134365', '-0.990932', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81519', '181288', '530', '1', '7693.53', '-6836.51', '77.787', '2.7052', '0', '0', '0.976289', '0.216469', '120', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81520', '181288', '530', '1', '9386.86', '-6772.24', '14.412', '-2.9146', '0', '0', '-0.993566', '0.113253', '120', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81521', '181288', '0', '1', '-134.688', '-802.767', '55.0147', '-1.62316', '0', '0', '-0.725376', '0.688353', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81522', '181288', '0', '1', '-10331.4', '-3297.73', '21.9992', '-2.89725', '0', '0', '-0.992546', '0.121868', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81523', '181288', '0', '1', '-14376.7', '115.921', '1.4532', '2.11185', '0', '0', '0.870356', '0.492423', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81524', '181288', '0', '1', '-7596.42', '-2086.6', '125.17', '-0.942478', '0', '0', '-0.453991', '0.891006', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81525', '181288', '0', '1', '-447.95', '-4527.65', '8.59595', '1.53589', '0', '0', '0.694658', '0.71934', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81526', '181288', '0', '1', '587.056', '1365.02', '90.4778', '2.6529', '0', '0', '0.970296', '0.241922', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81527', '181288', '0', '1', '2279.25', '456.009', '33.867', '0.3665', '0', '0', '0.182226', '0.983257', '120', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81528', '181288', '0', '1', '-6704.48', '-2200.91', '248.609', '0.017453', '0', '0', '0.00872639', '0.999962', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81529', '181288', '0', '1', '-1134.84', '-3531.81', '51.0719', '-0.820305', '0', '0', '-0.398749', '0.91706', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81530', '181288', '1', '1', '2558.73', '-481.666', '109.821', '-2.47837', '0', '0', '-0.945519', '0.325567', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81531', '181288', '1', '1', '-55.5039', '1271.35', '91.9489', '1.5708', '0', '0', '0.707108', '0.707106', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81532', '181288', '1', '1', '-3447.55', '-4231.67', '10.6645', '0.802851', '0', '0', '0.390731', '0.920505', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81533', '181288', '1', '1', '6860.03', '-4767.11', '696.833', '-2.63545', '0', '0', '-0.968148', '0.250379', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81534', '181288', '1', '1', '-7216.15', '-3859.66', '11.9664', '4.00788', '0', '0', '0.907651', '-0.419726', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81535', '181288', '1', '1', '-6771.96', '527.151', '-1.40004', '3.1949', '0', '0', '0.999645', '-0.0266505', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81536', '181288', '1', '1', '-4412.02', '3480.24', '12.6312', '6.12709', '0', '0', '0.0779684', '-0.996956', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81537', '181288', '1', '1', '6327.68', '512.61', '17.4723', '0.034907', '0', '0', '0.0174526', '0.999848', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81538', '181288', '530', '1', '-4223.84', '-12318.4', '2.47695', '5.69342', '0', '0', '0.290628', '-0.956836', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81539', '181288', '530', '1', '-2254.78', '-11896.3', '27.4979', '5.24028', '0', '0', '0.49814', '-0.867096', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81540', '181288', '1', '1', '9778.64', '1019.38', '1299.79', '0.261799', '0', '0', '0.130526', '0.991445', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81541', '181288', '0', '1', '188.243', '-2132.53', '102.674', '-1.37881', '0', '0', '-0.636078', '0.771625', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81542', '181288', '0', '1', '989.562', '-1453.47', '61.0011', '4.9105', '0', '0', '0.633712', '-0.773569', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81543', '181288', '0', '1', '-604.148', '-545.813', '36.579', '0.698132', '0', '0', '0.34202', '0.939693', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81544', '181288', '0', '1', '-10657.1', '1054.63', '32.6733', '2.47837', '0', '0', '0.945519', '0.325567', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81545', '181288', '0', '1', '-3448.2', '-938.102', '10.6583', '0.034907', '0', '0', '0.0174526', '0.999848', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81545', '181288', '0', '1', '-9394.21', '37.5017', '59.882', '1.15192', '0', '0', '0.54464', '0.83867', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81546', '181288', '0', '1', '-8245.62', '-2623.9', '133.155', '4.04776', '0', '0', '0.899102', '-0.43774', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81547', '181288', '0', '1', '-10951.5', '-3218.1', '41.3475', '1.91986', '0', '0', '0.819151', '0.573577', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81548', '181288', '0', '1', '-1211.6', '-2676.88', '45.3612', '-0.645772', '0', '0', '-0.317305', '0.948324', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81549', '181288', '0', '1', '-14288.1', '61.8062', '0.68836', '1.37881', '0', '0', '0.636078', '0.771625', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81550', '181288', '0', '1', '-5404.93', '-492.299', '395.597', '-0.506145', '0', '0', '-0.25038', '0.968148', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81551', '181288', '0', '1', '-10704.8', '-1146.38', '24.7909', '2.09439', '0', '0', '0.866024', '0.500002', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81552', '181288', '0', '1', '-5233.16', '-2893.37', '337.286', '-0.226893', '0', '0', '-0.113203', '0.993572', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81553', '181288', '0', '1', '-9434.3', '-2110.36', '65.8038', '0.349066', '0', '0', '0.173648', '0.984808', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81554', '181288', '530', '1', '-3941.98', '2048.49', '95.0656', '4.87742', '0', '0', '0.64642', '-0.762982', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81555', '181288', '530', '1', '-2526.69', '7548.83', '-2.23534', '5.32817', '0', '0', '0.459567', '-0.888143', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81556', '181288', '530', '1', '3121.5', '3753.88', '141.851', '5.85856', '0', '0', '0.210721', '-0.977546', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81557', '181288', '530', '1', '-528.509', '2339.11', '38.7252', '2.14675', '0', '0', '0.878816', '0.477161', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81558', '181288', '530', '1', '-3004.07', '4152.48', '3.64988', '1.75455', '0', '0', '0.769', '0.639249', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81559', '181288', '530', '1', '2019.7', '6587.14', '134.985', '6.23705', '0', '0', '0.0230656', '-0.999734', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81560', '181288', '530', '1', '192.209', '6015.13', '22.1067', '0.00709009', '0', '0', '0.00354504', '0.999994', '300', '0', '1', '');


replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81561', '300068', '1', '1', '-7122.51', '-3657.11', '8.82202', '-1.74533', '0', '0', '-0.766045', '0.642787', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81562', '300068', '1', '1', '6855.99', '-4564.4', '708.51', '0.855211', '0', '0', '0.414693', '0.909961', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81563', '300068', '1', '1', '-5513.93', '-2299.73', '-58.0752', '2.44346', '0', '0', '0.939692', '0.342021', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81564', '300068', '1', '1', '-273.242', '-2662.82', '91.695', '-1.8675', '0', '0', '-0.803856', '0.594824', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81565', '300068', '1', '1', '952.992', '776.968', '104.474', '-1.55334', '0', '0', '-0.700908', '0.713252', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81566', '300068', '1', '1', '-2329.42', '-624.806', '-8.27507', '5.5', '0', '0', '0.381661', '-0.924302', '0', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81567', '300068', '1', '1', '-4573.22', '407.388', '41.5461', '2.46091', '0', '0', '0.942641', '0.333809', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81568', '300068', '1', '1', '-1862.36', '3055.71', '0.744157', '2.49582', '0', '0', '0.948324', '0.317305', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81569', '300068', '1', '1', '-3110.59', '-2722.41', '33.4626', '0.226893', '0', '0', '0.113203', '0.993572', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81570', '300068', '1', '1', '2014.65', '-2337.84', '89.5211', '2.37365', '0', '0', '0.927184', '0.374606', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81571', '300068', '530', '1', '41.2448', '2587.44', '68.3453', '-2.28638', '0', '0', '-0.909961', '0.414694', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81572', '300068', '530', '1', '-3059.17', '2374.85', '63.1011', '2.03334', '0', '0', '0.850361', '0.526201', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81573', '300068', '530', '1', '2922.17', '3690.15', '143.809', '5.94709', '0', '0', '0.167258', '-0.985913', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81574', '300068', '530', '1', '-2553.32', '4277.61', '20.614', '-1.36136', '0', '0', '-0.629322', '0.777145', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81575', '300068', '530', '1', '200.9', '7686.96', '22.508', '-0.506145', '0', '0', '-0.25038', '0.968148', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81576', '300068', '530', '1', '-1211.01', '7474.44', '21.9953', '-2.02458', '0', '0', '-0.848048', '0.52992', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81577', '300068', '530', '1', '2282.43', '6134.5', '136.337', '6.01364', '0', '0', '0.134365', '-0.990932', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81578', '300068', '530', '1', '7693.53', '-6836.51', '77.787', '2.7052', '0', '0', '0.976289', '0.216469', '120', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81579', '300068', '530', '1', '9386.86', '-6772.24', '14.412', '-2.9146', '0', '0', '-0.993566', '0.113253', '120', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81580', '300068', '0', '1', '-134.688', '-802.767', '55.0147', '-1.62316', '0', '0', '-0.725376', '0.688353', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81581', '300068', '0', '1', '-10331.4', '-3297.73', '21.9992', '-2.89725', '0', '0', '-0.992546', '0.121868', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81582', '300068', '0', '1', '-14376.7', '115.921', '1.4532', '2.11185', '0', '0', '0.870356', '0.492423', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81583', '300068', '0', '1', '-7596.42', '-2086.6', '125.17', '-0.942478', '0', '0', '-0.453991', '0.891006', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81584', '300068', '0', '1', '-447.95', '-4527.65', '8.59595', '1.53589', '0', '0', '0.694658', '0.71934', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81585', '300068', '0', '1', '587.056', '1365.02', '90.4778', '2.6529', '0', '0', '0.970296', '0.241922', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81586', '300068', '0', '1', '2279.25', '456.009', '33.867', '0.3665', '0', '0', '0.182226', '0.983257', '120', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81587', '300068', '0', '1', '-6704.48', '-2200.91', '248.609', '0.017453', '0', '0', '0.00872639', '0.999962', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81588', '300068', '0', '1', '-1134.84', '-3531.81', '51.0719', '-0.820305', '0', '0', '-0.398749', '0.91706', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81589', '300068', '1', '1', '2558.73', '-481.666', '109.821', '-2.47837', '0', '0', '-0.945519', '0.325567', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81590', '300068', '1', '1', '-55.5039', '1271.35', '91.9489', '1.5708', '0', '0', '0.707108', '0.707106', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81591', '300068', '1', '1', '-3447.55', '-4231.67', '10.6645', '0.802851', '0', '0', '0.390731', '0.920505', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81592', '300068', '1', '1', '6860.03', '-4767.11', '696.833', '-2.63545', '0', '0', '-0.968148', '0.250379', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81593', '300068', '1', '1', '-7216.15', '-3859.66', '11.9664', '4.00788', '0', '0', '0.907651', '-0.419726', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81594', '300068', '1', '1', '-6771.96', '527.151', '-1.40004', '3.1949', '0', '0', '0.999645', '-0.0266505', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81595', '300068', '1', '1', '-4412.02', '3480.24', '12.6312', '6.12709', '0', '0', '0.0779684', '-0.996956', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81596', '300068', '1', '1', '6327.68', '512.61', '17.4723', '0.034907', '0', '0', '0.0174526', '0.999848', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81597', '300068', '530', '1', '-4223.84', '-12318.4', '2.47695', '5.69342', '0', '0', '0.290628', '-0.956836', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81598', '300068', '530', '1', '-2254.78', '-11896.3', '27.4979', '5.24028', '0', '0', '0.49814', '-0.867096', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81599', '300068', '1', '1', '9778.64', '1019.38', '1299.79', '0.261799', '0', '0', '0.130526', '0.991445', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81600', '300068', '0', '1', '188.243', '-2132.53', '102.674', '-1.37881', '0', '0', '-0.636078', '0.771625', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81601', '300068', '0', '1', '989.562', '-1453.47', '61.0011', '4.9105', '0', '0', '0.633712', '-0.773569', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81602', '300068', '0', '1', '-604.148', '-545.813', '36.579', '0.698132', '0', '0', '0.34202', '0.939693', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81603', '300068', '0', '1', '-10657.1', '1054.63', '32.6733', '2.47837', '0', '0', '0.945519', '0.325567', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81604', '300068', '0', '1', '-3448.2', '-938.102', '10.6583', '0.034907', '0', '0', '0.0174526', '0.999848', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81605', '300068', '0', '1', '-9394.21', '37.5017', '59.882', '1.15192', '0', '0', '0.54464', '0.83867', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81606', '300068', '0', '1', '-8245.62', '-2623.9', '133.155', '4.04776', '0', '0', '0.899102', '-0.43774', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81607', '300068', '0', '1', '-10951.5', '-3218.1', '41.3475', '1.91986', '0', '0', '0.819151', '0.573577', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81608', '300068', '0', '1', '-1211.6', '-2676.88', '45.3612', '-0.645772', '0', '0', '-0.317305', '0.948324', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81609', '300068', '0', '1', '-14288.1', '61.8062', '0.68836', '1.37881', '0', '0', '0.636078', '0.771625', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81610', '300068', '0', '1', '-5404.93', '-492.299', '395.597', '-0.506145', '0', '0', '-0.25038', '0.968148', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81611', '300068', '0', '1', '-10704.8', '-1146.38', '24.7909', '2.09439', '0', '0', '0.866024', '0.500002', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81612', '300068', '0', '1', '-5233.16', '-2893.37', '337.286', '-0.226893', '0', '0', '-0.113203', '0.993572', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81613', '300068', '0', '1', '-9434.3', '-2110.36', '65.8038', '0.349066', '0', '0', '0.173648', '0.984808', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81614', '300068', '530', '1', '-3941.98', '2048.49', '95.0656', '4.87742', '0', '0', '0.64642', '-0.762982', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81615', '300068', '530', '1', '-2526.69', '7548.83', '-2.23534', '5.32817', '0', '0', '0.459567', '-0.888143', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81616', '300068', '530', '1', '3121.5', '3753.88', '141.851', '5.85856', '0', '0', '0.210721', '-0.977546', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81617', '300068', '530', '1', '-528.509', '2339.11', '38.7252', '2.14675', '0', '0', '0.878816', '0.477161', '180', '100', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81618', '300068', '530', '1', '-3004.07', '4152.48', '3.64988', '1.75455', '0', '0', '0.769', '0.639249', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81619', '300068', '530', '1', '2019.7', '6587.14', '134.985', '6.23705', '0', '0', '0.0230656', '-0.999734', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81620', '300068', '530', '1', '192.209', '6015.13', '22.1067', '0.00709009', '0', '0', '0.00354504', '0.999994', '300', '0', '1', '');
replace into `gameobject` (`guid`, `id`, `map`, `spawnMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`) values('81621', '300068', '1', '1', '145.521', '-4713.82', '18.129', '-2.53', '0', '0', '-0.953607', '0.301053', '120', '0', '1', '');

REPLACE INTO game_event_gameobject (guid, event) VALUES
(81501, 1),
(81502, 1),
(81503, 1),
(81504, 1),
(81505, 1),
(81506, 1),
(81507, 1),
(81508, 1),
(81509, 1),
(81510, 1),
(81511, 1),
(81512, 1),
(81513, 1),
(81514, 1),
(81515, 1),
(81516, 1),
(81517, 1),
(81518, 1),
(81519, 1),
(81520, 1),
(81521, 1),
(81522, 1),
(81523, 1),
(81524, 1),
(81525, 1),
(81526, 1),
(81527, 1),
(81528, 1),
(81529, 1),
(81530, 1),
(81531, 1),
(81532, 1),
(81533, 1),
(81534, 1),
(81535, 1),
(81536, 1),
(81537, 1),
(81538, 1),
(81539, 1),
(81540, 1),
(81541, 1),
(81542, 1),
(81543, 1),
(81544, 1),
(81545, 1),
(81545, 1),
(81546, 1),
(81547, 1),
(81548, 1),
(81549, 1),
(81550, 1),
(81551, 1),
(81552, 1),
(81553, 1),
(81554, 1),
(81555, 1),
(81556, 1),
(81557, 1),
(81558, 1),
(81559, 1),
(81560, 1),
(81561, 1),
(81562, 1),
(81563, 1),
(81564, 1),
(81565, 1),
(81566, 1),
(81567, 1),
(81568, 1),
(81569, 1),
(81570, 1),
(81571, 1),
(81572, 1),
(81573, 1),
(81574, 1),
(81575, 1),
(81576, 1),
(81577, 1),
(81578, 1),
(81579, 1),
(81580, 1),
(81581, 1),
(81582, 1),
(81583, 1),
(81584, 1),
(81585, 1),
(81586, 1),
(81587, 1),
(81588, 1),
(81589, 1),
(81590, 1),
(81591, 1),
(81592, 1),
(81593, 1),
(81594, 1),
(81595, 1),
(81596, 1),
(81597, 1),
(81598, 1),
(81599, 1),
(81600, 1),
(81601, 1),
(81602, 1),
(81603, 1),
(81604, 1),
(81605, 1),
(81606, 1),
(81607, 1),
(81608, 1),
(81609, 1),
(81610, 1),
(81611, 1),
(81612, 1),
(81613, 1),
(81614, 1),
(81615, 1),
(81616, 1),
(81617, 1),
(81618, 1),
(81619, 1),
(81620, 1),
(81621, 1);