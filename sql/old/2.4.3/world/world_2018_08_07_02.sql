
-- Remove Burning Abyssal and Channelers C++ Scripts + Add missing trigger flags
UPDATE creature_template SET ScriptName = "", AIName = "SmartAI" where entry IN (17256, 17454);
-- Allow silence on channelers
UPDATE creature_template SET mechanic_immune_mask = mechanic_immune_mask & ~0x100 WHERE entry = 17256;
UPDATE creature_template SET ScriptName = "npc_magtheridon_room", flags_extra = 128 WHERE entry = 17516;
UPDATE creature_template SET flags_extra = 128 WHERE entry = 17474;
-- Change cube cooldown
UPDATE gameobject_template SET data3 = 1000, data10 = 30420, ScriptName = "go_manticron_cube" WHERE entry = 181713;

-- Channelers SmartAI
-- NPC 17256
SET @ENTRY = 17256;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 0, 0, 0, 0, 100, 0, 1800, 16800, 18400, 21400, 11, 30510, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Channeler - In Combat - Cast "<a href="http://wowhead.com/spell=30510">Shadow Bolt Volley</a>" '),
(@ENTRY, 0, 1, 0, 14, 0, 100, 0, 70000, 30, 14500, 15000, 11, 30528, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Channeler - On Friendly at "70000" HP - Cast "<a href="http://wowhead.com/spell=30528">Dark Mending</a>" '),
(@ENTRY, 0, 2, 0, 0, 0, 100, 0, 6000, 12000, 17000, 28000, 11, 30530, 0, 0, 0, 0, 0, 6, 0, 30, 0, 0, 0, 0, 0, 0, 'Hellfire Channeler - In Combat - Cast "<a href="http://wowhead.com/spell=30530">Fear</a>" '),
(@ENTRY, 0, 3, 0, 0, 0, 100, 1, 10000, 30000, 20000, 40000, 11, 30511, 0, 0, 0, 0, 0, 5, 0, 40, 0, 0, 0, 0, 0, 0, 'Hellfire Channeler - In Combat - Cast "<a href="http://wowhead.com/spell=30511">Burning Abyssal</a>" (No Repeat)'),
(@ENTRY, 0, 4, 0, 1, 0, 100, 1, 1000, 1000, 0, 0, 11, 30207, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Channeler - Out of Combat - Cast "<a href="http://wowhead.com/spell=30207">Shadow Grasp</a>" (No Repeat)'),
(@ENTRY, 0, 5, 0, 6, 0, 100, 0, 0, 0, 0, 0, 11, 30531, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Channeler - On Death - Cast "<a href="http://wowhead.com/spell=30531">Soul Transfer</a>" '),
(@ENTRY, 0, 6, 0, 4, 0, 100, 0, 0, 0, 0, 0, 34, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Channeler - On Aggro - Set Instance Data 1 1 '),
(@ENTRY, 0, 7, 0, 37, 0, 100, 0, 0, 0, 0, 0, 8, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hellfire Channeler - On AI Initialize - Set React State Defensive');

-- Burning Abyssal SmartAI
SET @ENTRY = 17454;
UPDATE creature_template SET AIName='SmartAI', ScriptName='' WHERE entry = @ENTRY;
DELETE FROM smart_scripts WHERE entryorguid = @ENTRY AND source_type = 0;
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(@ENTRY, 0, 0, 0, 0, 0, 100, 0, 5000, 7500, 5000, 10000, 11, 37110, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Burning Abyssal - In Combat - Cast "<a href="http://wowhead.com/spell=37110">Fire Blast</a>" ');

-- Magtheridon texts
replace into `creature_text` (`CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `TextRange`, `comment`) values('17257','0','0','Wretched, meddling insects! Release me, and perhaps I will grant you a merciful death!','14','0','100','0','0','10247','0','Magtheridon SAY_TAUNT');
replace into `creature_text` (`CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `TextRange`, `comment`) values('17257','0','1','Vermin! Leeches! Take my blood and choke on it!','14','0','100','0','0','10248','0','Magtheridon SAY_TAUNT');
replace into `creature_text` (`CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `TextRange`, `comment`) values('17257','0','2','Illidan is an arrogant fool! I will crush him and reclaim Outland as my own!','14','0','100','0','0','10249','0','Magtheridon SAY_TAUNT');
replace into `creature_text` (`CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `TextRange`, `comment`) values('17257','0','3','Away, you mindless parasites! My blood is my own!','14','0','100','0','0','10250','0','Magtheridon SAY_TAUNT');
replace into `creature_text` (`CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `TextRange`, `comment`) values('17257','0','4','How long do you believe your pathetic sorcery can hold me?','14','0','100','0','0','10251','0','Magtheridon SAY_TAUNT');
replace into `creature_text` (`CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `TextRange`, `comment`) values('17257','0','5','My blood will be the end of you!','14','0','100','0','0','10252','0','Magtheridon SAY_TAUNT');
replace into `creature_text` (`CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `TextRange`, `comment`) values('17257','1','0','I... am... UNLEASHED!','14','0','100','15','0','10253','0','Magtheridon SAY_FREE');
replace into `creature_text` (`CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `TextRange`, `comment`) values('17257','2','0','Thank you for releasing me. Now... die!','14','0','100','0','0','10254','0','Magtheridon SAY_AGGRO');
replace into `creature_text` (`CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `TextRange`, `comment`) values('17257','3','0','Did you think me weak? Soft? Who is the weak one now?!','14','0','100','0','0','10255','0','Magtheridon SAY_SLAY');
replace into `creature_text` (`CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `TextRange`, `comment`) values('17257','4','0','Not again... NOT AGAIN!','14','0','100','0','0','10256','0','Magtheridon SAY_BANISH');
replace into `creature_text` (`CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `TextRange`, `comment`) values('17257','5','0','I will not be taken so easily. Let the walls of this prison tremble... and FALL!','14','0','100','0','0','10257','0','Magtheridon SAY_PHASE3');
replace into `creature_text` (`CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `TextRange`, `comment`) values('17257','6','0','The Legion... will consume you... all....','14','0','100','0','0','10258','0','Magtheridon SAY_DEATH');
replace into `creature_text` (`CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `TextRange`, `comment`) values('17257','7','0','%s\'s bonds begin to weaken!','16','0','100','0','0','0','0','Magtheridon SAY_EMOTE_BEGIN');
replace into `creature_text` (`CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `TextRange`, `comment`) values('17257','8','0','%s is nearly free of his bonds!','16','0','100','0','0','0','0','Magtheridon SAY_EMOTE_NEARLY');
replace into `creature_text` (`CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `TextRange`, `comment`) values('17257','9','0','%s breaks free','16','0','100','0','0','0','0','Magtheridon SAY_EMOTE_FREE');
replace into `creature_text` (`CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `TextRange`, `comment`) values('17257','10','0','%s begins to cast Blast Nova!','41','0','100','0','0','0','0','Magtheridon SAY_EMOTE_NOVA');

-- Remove old texts
DELETE FROM script_texts WHERE entry IN (-1544000, -1544001, -1544002, -1544003, -1544004, -1544005, -1544006, -1544007, -1544008, -1544009, -1544010, -1544011, -1544012, -1544013, -1544014); 

-- Script names 
replace into `spell_script_names` (`spell_id`, `ScriptName`) values('30410','spell_magtheridon_shadow_grasp');
replace into `spell_script_names` (`spell_id`, `ScriptName`) values('30541','spell_magtheridon_blaze');
replace into `spell_script_names` (`spell_id`, `ScriptName`) values('30658','spell_maghteridon_quake');

-- Spell targets
REPLACE INTO `conditions` (`id`, `SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) values('794','13','1','30410','0','0','31','0','3','17376','0','0','0','0','','SPELL_SHADOW_GRASP - Requires Hellfire Raid Trigger');
REPLACE INTO `conditions` (`id`, `SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) values('802','13','7','30657','0','0','31','0','3','17474','0','0','0','0','','SPELL_QUAKE - Requires Target Trigger');
REPLACE INTO `conditions` (`id`, `SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) values('4247','13','1','30541','0','0','31','0','4','0','0','0','0','0','','SPELL_BLAZE - Targets players');
REPLACE INTO `conditions` (`id`, `SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) values('4248','13','1','30571','0','0','31','0','4','0','0','0','0','0','','SPELL_QUAKE_KNOCKBACK - Targets player');
REPLACE INTO `conditions` (`id`, `SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) values('4249','13','1','30631','0','0','31','0','4','0','0','0','0','0','','SPELL_DEBRIS_DAMAGE - Targets player');

-- Delete channelers from DB (now summoned)
DELETE FROM creature WHERE id = 17256;

DELETE FROM creature_summon_groups where summonerId = 17257;
replace into `creature_summon_groups` (`summonerId`, `summonerType`, `groupId`, `entry`, `position_x`, `position_y`, `position_z`, `orientation`, `summonType`, `summonTime`) values('17257','0','1','17256','-31.7645','-35.8374','0.714268','1.37881','6','6000');
replace into `creature_summon_groups` (`summonerId`, `summonerType`, `groupId`, `entry`, `position_x`, `position_y`, `position_z`, `orientation`, `summonType`, `summonTime`) values('17257','0','1','17256','10.3369','-19.7942','0.714241','2.53073','6','6000');
replace into `creature_summon_groups` (`summonerId`, `summonerType`, `groupId`, `entry`, `position_x`, `position_y`, `position_z`, `orientation`, `summonType`, `summonTime`) values('17257','0','1','17256','10.4777','23.9914','0.714224','3.66519','6','6000');
replace into `creature_summon_groups` (`summonerId`, `summonerType`, `groupId`, `entry`, `position_x`, `position_y`, `position_z`, `orientation`, `summonType`, `summonTime`) values('17257','0','1','17256','-31.9479','40.2201','0.714251','4.93928','6','6000');
replace into `creature_summon_groups` (`summonerId`, `summonerType`, `groupId`, `entry`, `position_x`, `position_y`, `position_z`, `orientation`, `summonType`, `summonTime`) values('17257','0','1','17256','-55.6891','2.17024','0.71428','0.05236','6','6000');

-- Add missing triggers npc
REPLACE INTO `creature` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`) VALUES('311569','17474','544','-13.1443','-26.8186','-0.412466','3.70357');
REPLACE INTO `creature` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`) VALUES('311570','17474','544','-44.3225','22.0454','-0.329129','5.87281');
REPLACE INTO `creature` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`) VALUES('311571','17474','544','-44.0663','21.973','-0.329128','5.15817');
REPLACE INTO `creature` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`) VALUES('311572','17474','544','10.2283','1.97198','-0.320875','1.41097');
REPLACE INTO `creature` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`) VALUES('311573','17474','544','11.2283','2.97198','-0.320875','1.14748');
REPLACE INTO `creature` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`) VALUES('311574','17474','544','-12.4546','35.4987','-0.319343','2.09894');
REPLACE INTO `creature` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`) VALUES('311575','17474','544','-11.6249','31.6464','-0.412482','4.08569');
REPLACE INTO `creature` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`) VALUES('311576','17474','544','-44.6199','-15.1769','-0.329109','1.5311');
REPLACE INTO `creature` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`) VALUES('311577','17474','544','-17.5166','2.10518','-0.320812','5.98416');
REPLACE INTO `creature` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`) VALUES('311578','17474','544','-16.5166','3.10518','-0.320812','0.0813029');
REPLACE INTO `creature` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`) VALUES('311579','17474','544','-14.3701','-24.6551','-0.412466','1.99314');
REPLACE INTO `creature` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`) VALUES('311580','17474','544','-45.6199','-16.1769','-0.329109','5.65524');
