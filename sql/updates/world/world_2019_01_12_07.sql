-- Fix Quest: Spinebreaker Post

-- NPC 21279
SET @ENTRY = 21279;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 1, 0, 20, 0, 100, 0, 10242, 0, 0, 0, 80, 2127901, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Apothecary Albreck - On Quest "<a href="http://wowhead.com/quest=10242">Spinebreaker Post</a>" Rewarded - Run Script <a href="/smartai/script/2127901">2127901</a> '),
(@ENTRY, 0, 2, 3, 40, 0, 100, 0, 10, 21279, 0, 0, 1, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Apothecary Albreck - On Waypoint 10 Reached - Say Line 4 '),
(@ENTRY, 0, 3, 0, 61, 0, 100, 0, 10, 21279, 0, 0, 24, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Apothecary Albreck - On Waypoint 10 Reached - Evade ');

DELETE FROM smart_scripts WHERE entryorguid = @ENTRY*100+1 AND source_type = 9;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY*100+1, 9, 100, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Apothecary Albreck - In Combat - Say Line 0 '),
(@ENTRY*100+1, 9, 101, 0, 0, 0, 100, 0, 5000, 5000, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Apothecary Albreck - In Combat - Say Line 1 '),
(@ENTRY*100+1, 9, 102, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Apothecary Albreck - In Combat - Say Line 2 '),
(@ENTRY*100+1, 9, 103, 0, 0, 0, 100, 0, 0, 0, 100, 100, 11, 36555, 0, 0, 0, 0, 0, 1, 0, 0, 50108, 0, 0, 0, 0, 0, 'Apothecary Albreck - In Combat - Cast "<a href="http://wowhead.com/spell=36555">Summon Cursed Scarabs</a>" '),
(@ENTRY*100+1, 9, 104, 0, 0, 0, 100, 0, 0, 0, 100, 100, 11, 36555, 0, 0, 0, 0, 0, 1, 0, 0, 50108, 0, 0, 0, 0, 0, 'Apothecary Albreck - In Combat - Cast "<a href="http://wowhead.com/spell=36555">Summon Cursed Scarabs</a>" '),
(@ENTRY*100+1, 9, 105, 0, 0, 0, 100, 0, 0, 0, 100, 100, 11, 36555, 0, 0, 0, 0, 0, 1, 0, 0, 50108, 0, 0, 0, 0, 0, 'Apothecary Albreck - In Combat - Cast "<a href="http://wowhead.com/spell=36555">Summon Cursed Scarabs</a>" '),
(@ENTRY*100+1, 9, 106, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Apothecary Albreck - In Combat - Say Line 3 '),
(@ENTRY*100+1, 9, 107, 0, 0, 0, 100, 0, 0, 0, 0, 0, 53, 0, 21279, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Apothecary Albreck - In Combat - Start Waypoint 21279 ');

DELETE FROM conditions WHERE id = 1484;
UPDATE conditions SET ElseGroup = 2 WHERE SourceGroup = 19401 AND SourceEntry = 2;
