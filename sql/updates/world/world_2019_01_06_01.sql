-- NPC 20242
SET @ENTRY = 20242;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 1, 0, 1, 0, 100, 0, 15000, 30000, 60000, 120000, 1, 0, 0, 0, 0, 0, 0, 21, 0, 100, 0, 0, 0, 0, 0, 0, 'Karaaz - Out of Combat - Say Line 0 ');
