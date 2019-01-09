-- Fix Quest: What's Haunting Witch Hill?
-- NPC 23861
SET @ENTRY = 23861;
UPDATE creature_template SET AIName='SmartAI', ScriptName='', npcflag = 0 WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 1, 0, 54, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Restless Apparition - On Just Summoned - Say Line 0 ');

DELETE FROM creature_text WHERE CreatureID = @ENTRY;
INSERT INTO creature_text (CreatureID, groupid, id, text, type, language, probability, emote, sound, comment) VALUES 
(@ENTRY, 0, 0, "Go away, whoever you are! Witch Hill is mine... mine!", 12, 0, 100, 0, 0, "Restless Apparition - "),
(@ENTRY, 0, 1, "It is too late for Jarl... its hold is too strong...", 12, 0, 100, 0, 0, "Restless Apparition - "),
(@ENTRY, 0, 2, "It is too late for us, living one. Take yourself and your friend away from here before you both are... claimed...", 12, 0, 100, 0, 0, "Restless Apparition - "),
(@ENTRY, 0, 3, "It was... terrible... the demon...", 12, 0, 100, 0, 0, "Restless Apparition - "),
(@ENTRY, 0, 4, "The darkness will consume all... all the living...", 12, 0, 100, 0, 0, "Restless Apparition - "),
(@ENTRY, 0, 5, "The darkness... the corruption... they came too quickly for anyone to know...", 12, 0, 100, 0, 0, "Restless Apparition - "),
(@ENTRY, 0, 6, "The manor... someone else... will soon be consumed...", 12, 0, 100, 0, 0, "Restless Apparition - "),
(@ENTRY, 0, 7, "Why have you come here, outsider? You will only find pain! Our fate will be yours...", 12, 0, 100, 0, 0, "Restless Apparition - ");

-- NPC 23554
SET @ENTRY = 23554;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 1, 0, 0, 0, 100, 0, 5000, 10000, 20000, 25000, 11, 43127, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Risen Spirit - In Combat - Cast "<a href="http://wowhead.com/spell=43127">Intangible Presence</a>" '),
(@ENTRY, 0, 2, 3, 6, 0, 100, 0, 0, 0, 0, 0, 11, 42512, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Risen Spirit - On Death - Cast "<a href="http://wowhead.com/spell=42512">Witch Hill Information Credit</a>" '),
(@ENTRY, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 11, 42511, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Risen Spirit - On Death - Cast "<a href="http://wowhead.com/spell=42511">Summon Restless Apparition</a>" ');

-- NPC 23555
SET @ENTRY = 23555;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 1, 0, 0, 0, 100, 0, 2500, 7500, 15000, 20000, 11, 37933, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Risen Husk - In Combat - Cast "<a href="http://wowhead.com/spell=37933">Consume Flesh</a>" '),
(@ENTRY, 0, 2, 3, 6, 0, 100, 0, 0, 0, 0, 0, 11, 42512, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Risen Husk - On Death - Cast "<a href="http://wowhead.com/spell=42512">Witch Hill Information Credit</a>" '),
(@ENTRY, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 11, 42511, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Risen Husk - On Death - Cast "<a href="http://wowhead.com/spell=42511">Summon Restless Apparition</a>" ');
