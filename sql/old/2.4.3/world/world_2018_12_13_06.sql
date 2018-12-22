UPDATE creature SET patch_max = 4 WHERE spawnID IN (1096091, 1026435, 1024579, 1026431, 1026426, 1026767, 1024540, 1024663, 1006430, 1046615, 1310980, 1017557, 1090413, 1310979);
UPDATE creature SET patch_min = 5 WHERE spawnID IN (208368);
UPDATE creature SET position_x = 3070, position_y = 3635, position_z = 143, orientation = 0.75 WHERE spawnID = 1017557;
UPDATE creature SET position_x = 3075, position_y = 3633.6, position_z = 143.8, orientation = 2.34 WHERE spawnID = 1090413;
UPDATE creature_template SET minlevel = 33, maxlevel = 33, faction = 130 WHERE entry = 10236;

replace into `pool_template` (`entry`, `max_limit`, `description`) values('358','1','Max 1 Rare in Dire Maul Arena');

replace into `pool_creature` (`guid`, `pool_entry`, `chance`, `description`) values('45758','358','0','Rare Spawn Dire Maul Arena');
replace into `pool_creature` (`guid`, `pool_entry`, `chance`, `description`) values('45759','358','0','Rare Spawn Dire Maul Arena');
replace into `pool_creature` (`guid`, `pool_entry`, `chance`, `description`) values('45760','358','0','Rare Spawn Dire Maul Arena');

UPDATE creature_template SET minlevel = 53, maxlevel = 53, faction = 16 WHERE entry = 14341;

REPLACE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(1848200, 9, 5, 0, 0, 0, 100, 0, 0, 0, 0, 0, 45, 0, 1, 0, 0, 0, 0, 9, 0, 18483, 0, 100, 0, 0, 0, 0, 'Empoor - In Combat - Set Data 0 1 '),
(1848200, 9, 6, 0, 0, 0, 100, 0, 0, 0, 0, 0, 100, 1, 1, 0, 0, 0, 0, 9, 0, 18483, 0, 100, 0, 0, 0, 0, 'Empoor - In Combat - Send Target 1 '),
(1848201, 9, 4, 0, 0, 0, 100, 0, 0, 0, 0, 0, 2, 35, 0, 0, 0, 0, 0, 9, 0, 18483, 0, 100, 0, 0, 0, 0, 'Empoor - In Combat - Set Faction 35 '),
(1848202, 9, 1, 0, 0, 0, 100, 0, 0, 0, 0, 0, 47, 0, 0, 0, 0, 0, 0, 9, 0, 18483, 0, 100, 0, 0, 0, 0, 'Empoor - In Combat - Set Visibility Off '),
(1848202, 9, 3, 0, 0, 0, 100, 0, 0, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 9, 0, 18483, 0, 100, 0, 0, 0, 0, 'Empoor - In Combat - Despawn Instant ');