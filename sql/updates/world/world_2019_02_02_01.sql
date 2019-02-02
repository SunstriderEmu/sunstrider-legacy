-- Fix Bleeding Hollow Archer dmg (set to Bestiary)
UPDATE creature_template SET DamageModifier=4.634, BaseVariance=0.4577, ArmorModifier=1 WHERE entry = 17270;

-- Fix Wounded Blood Elf Pilgrim not attacking
-- NPC 16993
SET @ENTRY = 16993;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY*100+1 AND source_type = 9;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY*100+1, 9, 100, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Wounded Blood Elf Pilgrim - In Combat - Say Line 0 '),
(@ENTRY*100+1, 9, 101, 0, 0, 0, 100, 0, 0, 0, 0, 0, 64, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Wounded Blood Elf Pilgrim - In Combat - Store Targetlist 0 '),
(@ENTRY*100+1, 9, 102, 0, 0, 0, 100, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 14, 0, 22149, 181664, 0, 0, 0, 0, 0, 'Wounded Blood Elf Pilgrim - In Combat - Activate GO '),
(@ENTRY*100+1, 9, 103, 0, 0, 0, 100, 0, 0, 0, 0, 0, 83, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Wounded Blood Elf Pilgrim - In Combat - Set NPC Flag Quest Giver '),
(@ENTRY*100+1, 9, 104, 0, 0, 0, 100, 0, 0, 0, 0, 0, 19, 512, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Wounded Blood Elf Pilgrim - In Combat - Set Unit Flag Immune to NPC '),
(@ENTRY*100+1, 9, 105, 0, 0, 0, 100, 0, 3000, 3000, 0, 0, 53, 0, 16993, 0, 9375, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Wounded Blood Elf Pilgrim - In Combat - Start Waypoint 16993 '),
(@ENTRY*100+1, 9, 106, 0, 0, 0, 100, 0, 0, 0, 0, 0, 2, 495, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Wounded Blood Elf Pilgrim - In Combat - Set Faction 495 ');

-- Fix Darkness Released weird movements & speed
UPDATE creature_template_movement SET Flight = 1 WHERE CreatureId = 22507;
UPDATE creature_template SET speed_run = 0.5 WHERE entry = 22507;
