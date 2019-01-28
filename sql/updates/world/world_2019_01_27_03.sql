-- Only use the gameobject with quest: How To Serve Goblins
UPDATE gameobject_template SET data1 = 10238, data3 = 5000 WHERE entry = 183936;
UPDATE gameobject_template SET data1 = 10238, data3 = 5000 WHERE entry = 183940;
UPDATE gameobject_template SET data1 = 10238, data3 = 5000 WHERE entry = 183941;

-- Manni
-- GO 183936
SET @ENTRY = 183936;
UPDATE gameobject_template SET AIName='SmartGameObjectAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 1;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 1, 1, 0, 104, 0, 100, 0, 4, 0, 0, 0, 45, 19763, 0, 0, 0, 0, 0, 19, 0, 19763, 5, 0, 0, 0, 0, 0, 'Manni&#039;s Cage - On GO State Loot Activated - Set Data 19763 0 ');

-- NPC 19763
SET @ENTRY = 19763;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 0, 0, 38, 0, 100, 0, 19763, 0, 0, 0, 80, 1976301, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Manni - On Data Set 19763 0 - Run Script <a href="/smartai/script/1976301">1976301</a> ');

DELETE FROM smart_scripts WHERE entryorguid = @ENTRY*100+1 AND source_type = 9;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY*100+1, 9, 100, 0, 0, 0, 100, 0, 0, 0, 0, 0, 59, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Manni - In Combat - Set Run Off '),
(@ENTRY*100+1, 9, 101, 0, 0, 0, 100, 0, 0, 0, 0, 0, 69, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 70.6965, 3209.28, 32.1166, 6.21301, 'Manni - In Combat - Move To Pos '),
(@ENTRY*100+1, 9, 102, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 66, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Manni - In Combat - Set Orientation 0 '),
(@ENTRY*100+1, 9, 103, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Manni - In Combat - Say Line 0 '),
(@ENTRY*100+1, 9, 104, 0, 0, 0, 100, 0, 3000, 3000, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Manni - In Combat - Despawn Instant ');

DELETE FROM locales_creature_text WHERE entry = @ENTRY;
DELETE FROM creature_text WHERE CreatureID = @ENTRY;
INSERT IGNORE INTO creature_text (CreatureID, groupid, id, text, type, language, probability, emote, sound, comment) VALUES 
(@ENTRY, 0, 0, "Thank goodness your got here, it was almost dinner time!", 12, 0, 100, 0, 0, "Manni - Moving out of cage");

-- Moh
-- GO 183940
SET @ENTRY = 183940;
UPDATE gameobject_template SET AIName='SmartGameObjectAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 1;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 1, 1, 0, 104, 0, 100, 0, 4, 0, 0, 0, 45, 19764, 0, 0, 0, 0, 0, 19, 0, 19764, 5, 0, 0, 0, 0, 0, 'Moh&#039;s Cage - On GO State Loot Activated - Set Data 19764 0 ');

-- NPC 19764
SET @ENTRY = 19764;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 0, 0, 38, 0, 100, 0, 19764, 0, 0, 0, 80, 1976401, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Moh - On Data Set 19764 0 - Run Script <a href="/smartai/script/1976401">1976401</a> ');

DELETE FROM smart_scripts WHERE entryorguid = @ENTRY*100+1 AND source_type = 9;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY*100+1, 9, 100, 0, 0, 0, 100, 0, 0, 0, 0, 0, 59, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Manni - In Combat - Set Run Off '),
(@ENTRY*100+1, 9, 101, 0, 0, 0, 100, 0, 0, 0, 0, 0, 69, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, -71.039635, 3135.839600, -4.566130, 5.357289, 'Moh - In Combat - Move To Pos '),
(@ENTRY*100+1, 9, 102, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 66, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Manni - In Combat - Set Orientation 0 '),
(@ENTRY*100+1, 9, 103, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Manni - In Combat - Say Line 0 '),
(@ENTRY*100+1, 9, 104, 0, 0, 0, 100, 0, 3000, 3000, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Manni - In Combat - Despawn Instant ');

DELETE FROM locales_creature_text WHERE entry = @ENTRY;
DELETE FROM creature_text WHERE CreatureID = @ENTRY;
INSERT IGNORE INTO creature_text (CreatureID, groupid, id, text, type, language, probability, emote, sound, comment) VALUES 
(@ENTRY, 0, 0, "I though I was a goner for sure.", 12, 0, 100, 0, 0, "Moh - Moving out of cage");

-- Moh
-- GO 183941
SET @ENTRY = 183941;
UPDATE gameobject_template SET AIName='SmartGameObjectAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 1;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 1, 1, 0, 104, 0, 100, 0, 4, 0, 0, 0, 45, 19766, 0, 0, 0, 0, 0, 19, 0, 19766, 5, 0, 0, 0, 0, 0, 'Jakk&#039;s Cage - On GO State Loot Activated - Set Data 19766 0 ');

-- NPC 19766
SET @ENTRY = 19766;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 0, 0, 38, 0, 100, 0, 19766, 0, 0, 0, 80, 1976601, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Jakk - On Data Set 19766 0 - Run Script <a href="/smartai/script/1976601">1976601</a> ');

DELETE FROM smart_scripts WHERE entryorguid = @ENTRY*100+1 AND source_type = 9;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY*100+1, 9, 100, 0, 0, 0, 100, 0, 0, 0, 0, 0, 59, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Manni - In Combat - Set Run Off '),
(@ENTRY*100+1, 9, 101, 0, 0, 0, 100, 0, 0, 0, 0, 0, 69, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, -119.472229, 3089.431885, 3.443486, 0.513126, 'Jakk - In Combat - Move To Pos '),
(@ENTRY*100+1, 9, 102, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 66, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Manni - In Combat - Set Orientation 0 '),
(@ENTRY*100+1, 9, 103, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Manni - In Combat - Say Line 0 '),
(@ENTRY*100+1, 9, 104, 0, 0, 0, 100, 0, 3000, 3000, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Manni - In Combat - Despawn Instant ');

DELETE FROM locales_creature_text WHERE entry = @ENTRY;
DELETE FROM creature_text WHERE CreatureID = @ENTRY;
INSERT IGNORE INTO creature_text (CreatureID, groupid, id, text, type, language, probability, emote, sound, comment) VALUES 
(@ENTRY, 0, 0, "I don't know which is worse, getting eaten by fel orcs or working for that slave master Razelcraz! Oh well, thanks anyways!", 12, 0, 100, 0, 0, "Jakk - Moving out of cage");
