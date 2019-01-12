-- Fix Quest: Boiling Blood

-- GO 184715
SET @ENTRY = 184715;
UPDATE gameobject_template SET AIName='SmartGameObjectAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 1;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 1, 0, 0, 104, 0, 100, 0, 4, 0, 0, 0, 80, 18471501, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cursed Cauldron - On GO State Loot Activated - Run Script <a href="/smartai/script/18471501">18471501</a> ');

DELETE FROM smart_scripts WHERE entryorguid = @ENTRY*100+1 AND source_type = 9;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY*100+1, 9, 100, 0, 0, 0, 100, 0, 0, 0, 0, 0, 11, 36555, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, ' - In Combat - Cast "<a href="http://wowhead.com/spell=36555">Summon Cursed Scarabs</a>" '),
(@ENTRY*100+1, 9, 101, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 32, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, ' - In Combat - Reset GO ');

-- NPC 21306
SET @ENTRY = 21306;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 1, 0, 1, 0, 5, 0, 10000, 15000, 1000, 1000, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cursed Scarab - Out of Combat - Despawn Instant '),
(@ENTRY, 0, 2, 0, 1, 0, 10, 1, 3000, 5000, 0, 0, 2, 14, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cursed Scarab - Out of Combat - Set Faction 14 (No Repeat)'),
(@ENTRY, 0, 3, 0, 54, 0, 100, 0, 0, 0, 0, 0, 89, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cursed Scarab - On Just Summoned - Start Random Movement ');
