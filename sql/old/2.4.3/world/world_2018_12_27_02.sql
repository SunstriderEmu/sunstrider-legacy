UPDATE gossip_menu_option SET patch_min = 5 WHERE MenuID = 751 AND OptionID = 7;
UPDATE gossip_menu_option SET patch_min = 5 WHERE MenuID = 721 AND OptionID = 12;
UPDATE creature_template SET ScriptName = "" WHERE entry = 10618;
UPDATE creature_template SET ScriptName = "" WHERE entry IN (3212, 3215, 3217, 3218, 3219, 3220, 3221, 3222, 3223, 3224);
UPDATE creature_template SET ScriptName = "" WHERE entry IN (1735, 1738, 1745, 1746);
UPDATE creature_template SET ScriptName = "" WHERE entry IN (1423);
UPDATE creature_template SET ScriptName = "" WHERE entry IN (16169);
UPDATE creature_template SET ScriptName = "" WHERE entry IN (16222);
UPDATE creature_template SET ScriptName = "" WHERE entry IN (16264);
UPDATE creature_template SET ScriptName = "" WHERE entry IN (5595, 4262, 5624, 5953, 2209, 1742, 2210, 1743, 1744, 2210, 5725, 727, 13076, 3571, 16733, 16221);


-- Check si "unlearn talents" option from creature 3698, ça fait koi?
/* test 19325:
CONDITION_SPELL - Value1 39181 - Value2 0
CONDITION_ITEM - Value1 31760 - Value2 1
CONDITION_QUESTREWARDED - Value1 10898 - Value2 0
*/

UPDATE creature_template SET ScriptName = "" WHERE entry IN (18038); 
UPDATE creature_template SET ScriptName = "" WHERE entry IN (19687); 
UPDATE creature_template SET ScriptName = "" WHERE entry IN (18549); 
UPDATE creature_template SET ScriptName = "" WHERE entry IN (21657); 
UPDATE creature_template SET ScriptName = "" WHERE entry IN (22932); 
UPDATE creature_template SET ScriptName = "" WHERE entry IN (23309); 
UPDATE creature_template SET npcflag = (npcflag | 1), ScriptName = "npc_kalecgos" WHERE entry IN (24848); 
UPDATE creature_template SET ScriptName = "" WHERE entry IN (25638); 
UPDATE creature_template SET npcflag = (npcflag | 1), ScriptName = "npc_pet_gen_soul_trader" WHERE entry IN (27914); 
UPDATE creature_template SET npcflag = (npcflag | 1), ScriptName = "npc_stable_master" WHERE entry IN (6749,9976,9977,9978,9979,9980,9981,9982,9983,9984,9985,9986,9987,9988,9989,10045,10046,10047,10048,10049,10050,10051,10052,10053,10054,10055,10056,10057,10058,10059,10060,10061,10062,10063,10085,11069,11104,11105,11117,11119,13616,15131,16094,16185,16586,16656,16764,16824,17485,17666,17896,18244,18250,18984,19018,19368,19476,21336,21517,21518,22468,22469,9896,19325,23392,24905,24974,25037,27010,27385,24066,24067,24154,24350,25519,26597,26721,27056,27065,27068,27150,27183,27194,27948,28057,28690,28790,29250,29251,29658,29740,29906,29948,29959,29967,30008,30039,30155,30304,33854,35290,35291,35344);
INSERT IGNORE INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_flags, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, comment) VALUES 
(23704, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 14313, 1, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Cassa Crimsonwing - On Aggro - Cast "<a href="http://wowhead.com/spell=14313"></a>" (No Repeat)');

replace into `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) values('27914','0','0','I have arrived. Shall we set to work, then?','12','0','100','0','0','0','27295','0','Ethereal Soul-Trader');
replace into `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) values('27914','1','0','Ah, more essence to capture...','12','0','100','0','0','0','27336','0','Ethereal Soul-Trader');
replace into `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) values('27914','2','0','Here is your share.','12','0','100','0','0','0','27341','0','Ethereal Soul-Trader');

UPDATE gossip_menu_option SET patch_min = 5 WHERE MenuID = 57098;
UPDATE creature_template SET npcflag = (npcflag | 1), ScriptName = "npc_guard_shattrath_faction" WHERE entry IN (18549, 18568); 

replace into `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) values('9526','0','1','0','1','0','100','512','15000','15000','15000','15000','0','41','0','0','0','0','0','0','1','0','0','0','0','0','0','0','0','Enraged Gryphon - OOC - Despawn');
UPDATE creature_template SET DamageModifier = 7.5, mechanic_immune_mask = 653213695 WHERE entry = 9526;

UPDATE gossip_menu_option SET OptionType = 17 WHERE MenuId = 57064 AND OptionID = 0;

UPDATE gossip_menu_option SET patch_min = 5 WHERE MenuID = 57134 AND OptionID = 1;