-- Transfer server side spells to from spell_template_override to spell_template
ALTER TABLE spell_template
ADD serverSide tinyint(1) DEFAULT 0 NOT NULL AFTER schoolMask,
ADD customAttributesFlags int(11) DEFAULT NULL,
ADD comment text DEFAULT NULL;

ALTER TABLE spell_template_override
ADD serverSide tinyint(1) DEFAULT 0 NOT NULL AFTER schoolMask;

UPDATE spell_template_override sto
LEFT JOIN spell_template st ON st.entry = sto.entry
SET sto.serverSide = 1
WHERE st.entry is NULL;

INSERT INTO spell_template 
SELECT * FROM spell_template_override WHERE serverSide = 1;

DELETE FROM spell_template_override WHERE serverSide = 1;

DELETE FROM creature_formations WHERE leaderGUID = memberGUID;

-- Foreign Keys
ALTER TABLE creature_gossip
ADD FOREIGN KEY (npc_guid) REFERENCES creature (spawnID) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE creature
ADD FOREIGN KEY (spawnID) REFERENCES creature_entry (spawnID) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE creature_entry
ADD FOREIGN KEY (entry) REFERENCES creature_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

-- Delete game_event_creature: unmatched GUIDs
DELETE gec FROM game_event_creature gec LEFT JOIN creature c ON gec.guid = c.spawnID WHERE c.spawnID is NULL;
ALTER TABLE game_event_creature
ADD FOREIGN KEY (guid) REFERENCES creature (spawnID) ON DELETE CASCADE ON UPDATE CASCADE;

-- Delete game_event_model_equip: unmatched GUIDs
DELETE geme FROM game_event_model_equip geme LEFT JOIN creature c ON geme.guid = c.spawnID WHERE c.spawnID is NULL;
ALTER TABLE game_event_model_equip
CHANGE event event int(11) unsigned NOT NULL DEFAULT '0' AFTER equipment_id,
ADD FOREIGN KEY (guid) REFERENCES creature (spawnID) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (event) REFERENCES game_event (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE game_event_npcflag
CHANGE guid guid int(10) unsigned NOT NULL FIRST,
CHANGE event_id event_id int(11) unsigned NOT NULL AFTER guid,
CHANGE npcflag npcflag int(10) unsigned NOT NULL AFTER event_id,
ADD FOREIGN KEY (event_id) REFERENCES game_event (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (guid) REFERENCES creature (spawnID) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE game_event_npc_vendor
CHANGE event event int(11) unsigned NOT NULL FIRST,
CHANGE guid guid int(10) unsigned NOT NULL AFTER event,
CHANGE item item mediumint(8) unsigned NOT NULL AFTER guid,
CHANGE ExtendedCost ExtendedCost mediumint(8) unsigned NOT NULL AFTER incrtime,
ADD FOREIGN KEY (event) REFERENCES game_event (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (guid) REFERENCES creature (spawnID) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (item) REFERENCES item_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE pool_creature
ADD FOREIGN KEY (pool_entry) REFERENCES pool_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (guid) REFERENCES creature (spawnID) ON DELETE CASCADE ON UPDATE CASCADE,
DROP INDEX idx_guid;

ALTER TABLE spawn_group_template
ENGINE='InnoDB';

-- Delete spawn_group: unmatched GUIDs
DELETE sg FROM spawn_group sg LEFT JOIN creature c ON sg.spawnId = c.spawnID WHERE c.spawnID is NULL;
ALTER TABLE spawn_group
ENGINE='InnoDB',
ADD FOREIGN KEY (spawnId) REFERENCES creature (spawnID) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (groupId) REFERENCES spawn_group_template (groupId) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE creature_battleground
ENGINE='InnoDB',
ADD FOREIGN KEY (spawnId) REFERENCES creature (spawnID) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE creature_equip_template
CHANGE creatureID creatureID mediumint(8) unsigned NOT NULL FIRST,
CHANGE id id tinyint(1) NOT NULL DEFAULT '1' AFTER creatureID,
ADD FOREIGN KEY (creatureID) REFERENCES creature_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE creature_template_movement
ENGINE='InnoDB',
CHANGE CreatureId CreatureId mediumint(8) unsigned NOT NULL FIRST,
ADD FOREIGN KEY (creatureId) REFERENCES creature_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE creature_template
CHANGE modelid1 modelid1 mediumint(8) unsigned NULL AFTER difficulty_entry_1,
CHANGE modelid2 modelid2 mediumint(8) unsigned NULL AFTER modelid1,
CHANGE modelid3 modelid3 mediumint(8) unsigned NULL AFTER modelid2,
CHANGE modelid4 modelid4 mediumint(8) unsigned NULL AFTER modelid3,
CHANGE gossip_menu_id gossip_menu_id smallint(5) unsigned NULL AFTER IconName,
CHANGE lootid lootid mediumint(8) unsigned NULL AFTER type_flags,
CHANGE pickpocketloot pickpocketloot mediumint(8) unsigned NULL AFTER lootid,
CHANGE skinloot skinloot mediumint(8) unsigned NULL AFTER pickpocketloot,
CHANGE spell1 spell1 int(10) unsigned NULL AFTER resistance6,
CHANGE spell2 spell2 int(10) unsigned NULL AFTER spell1,
CHANGE spell3 spell3 int(10) unsigned NULL AFTER spell2,
CHANGE spell4 spell4 int(10) unsigned NULL AFTER spell3,
CHANGE spell5 spell5 int(10) unsigned NULL AFTER spell4,
CHANGE spell6 spell6 int(10) unsigned NULL AFTER spell5,
CHANGE spell7 spell7 int(10) unsigned NULL AFTER spell6,
CHANGE spell8 spell8 int(10) unsigned NULL AFTER spell7;
UPDATE creature_template SET modelid1 = NULL WHERE modelid1 = 0;
UPDATE creature_template SET modelid2 = NULL WHERE modelid2 = 0;
UPDATE creature_template SET modelid3 = NULL WHERE modelid3 = 0;
UPDATE creature_template SET modelid4 = NULL WHERE modelid4 = 0;
UPDATE creature_template SET gossip_menu_id = NULL WHERE gossip_menu_id = 0;
UPDATE creature_template SET gossip_menu_id = NULL WHERE entry IN (17370, 17626);
UPDATE creature_template SET lootid = NULL WHERE lootid = 0;
UPDATE creature_template SET pickpocketloot = NULL WHERE pickpocketloot = 0;
UPDATE creature_template SET skinloot = NULL WHERE skinloot = 0;
UPDATE creature_template SET spell1 = NULL WHERE spell1 = 0;
UPDATE creature_template SET spell2 = NULL WHERE spell2 = 0;
UPDATE creature_template SET spell3 = NULL WHERE spell3 = 0;
UPDATE creature_template SET spell4 = NULL WHERE spell4 = 0;
UPDATE creature_template SET spell5 = NULL WHERE spell5 = 0;
UPDATE creature_template SET spell6 = NULL WHERE spell6 = 0;
UPDATE creature_template SET spell7 = NULL WHERE spell7 = 0;
UPDATE creature_template SET spell8 = NULL WHERE spell8 = 0;
ALTER TABLE creature_template
ADD FOREIGN KEY (lootid) REFERENCES creature_loot_template (Entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (pickpocketloot) REFERENCES pickpocketing_loot_template (Entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (skinloot) REFERENCES skinning_loot_template (Entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (gossip_menu_id) REFERENCES gossip_menu (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (spell1) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (spell2) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (spell3) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (spell4) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (spell5) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (spell6) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (spell7) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (spell8) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (modelid1) REFERENCES creature_model_info (modelid) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (modelid2) REFERENCES creature_model_info (modelid) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (modelid3) REFERENCES creature_model_info (modelid) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (modelid4) REFERENCES creature_model_info (modelid) ON DELETE SET NULL ON UPDATE CASCADE;

-- Change creature_movement_override engine
ALTER TABLE creature_movement_override
ENGINE='InnoDB',
ADD FOREIGN KEY (SpawnId) REFERENCES creature (spawnID) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE creature_onkill_reputation
ADD FOREIGN KEY (creature_id) REFERENCES creature_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;


ALTER TABLE gameobject_battleground
ENGINE='InnoDB',
ADD FOREIGN KEY (spawnID) REFERENCES gameobject (guid) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE gameobject_questender
ADD FOREIGN KEY (quest) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE gameobject_queststarter
ADD FOREIGN KEY (quest) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

-- Delete game_event_creature_quest: inexistant quest
DELETE FROM game_event_creature_quest WHERE quest = 24729;
ALTER TABLE game_event_creature_quest
CHANGE id id mediumint(8) unsigned NOT NULL FIRST,
CHANGE quest quest mediumint(8) unsigned NOT NULL AFTER id,
CHANGE event event int(11) unsigned NOT NULL AFTER quest,
ADD FOREIGN KEY (id) REFERENCES creature_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (quest) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (event) REFERENCES game_event (entry) ON DELETE CASCADE ON UPDATE CASCADE;

-- Delete game_event_gameobject: unmatched GUIDs
DELETE geg FROM game_event_gameobject geg LEFT JOIN gameobject g ON geg.guid = g.guid WHERE g.guid is NULL;
ALTER TABLE game_event_gameobject
ADD FOREIGN KEY (guid) REFERENCES gameobject (guid) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE game_event_npc_trainer
ADD FOREIGN KEY (event) REFERENCES game_event (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE game_event_prerequisite
ADD FOREIGN KEY (event_id) REFERENCES game_event (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE game_event_prerequisite
ADD FOREIGN KEY (prerequisite_event) REFERENCES game_event (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE item_loot_template
DROP FOREIGN KEY item_loot_template_ibfk_1;

ALTER TABLE locales_creature_text ENGINE='InnoDB';
DELETE lct FROM locales_creature_text lct LEFT JOIN creature_text ct ON ct.CreatureID = lct.entry WHERE ct.CreatureID is NULL;
ALTER TABLE locales_creature_text
ADD FOREIGN KEY (entry) REFERENCES creature_text (CreatureID) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE locales_quest
ADD FOREIGN KEY (entry) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

-- Add some missing server side spells that were preventing from adding the FK
REPLACE INTO spell_template (entry, category, dispel, mechanic, attributes, attributesEx, attributesEx2, attributesEx3, attributesEx4, attributesEx5, attributesEx6, stances, stancesNot, targets, targetCreatureType, requiresSpellFocus, facingCasterFlags, casterAuraState, targetAuraState, casterAuraStateNot, targetAuraStateNot, castingTimeIndex, recoveryTime, categoryRecoveryTime, interruptFlags, auraInterruptFlags, channelInterruptFlags, procFlags, procChance, procCharges, maxLevel, baseLevel, spellLevel, durationIndex, powerType, manaCost, manaCostPerlevel, manaPerSecond, manaPerSecondPerLevel, rangeIndex, speed, stackAmount, totem1, totem2, reagent1, reagent2, reagent3, reagent4, reagent5, reagent6, reagent7, reagent8, reagentCount1, reagentCount2, reagentCount3, reagentCount4, reagentCount5, reagentCount6, reagentCount7, reagentCount8, equippedItemClass, equippedItemSubClassMask, equippedItemInventoryTypeMask, effect1, effect2, effect3, effectDieSides1, effectDieSides2, effectDieSides3, effectBaseDice1, effectBaseDice2, effectBaseDice3, effectDicePerLevel1, effectDicePerLevel2, effectDicePerLevel3, effectRealPointsPerLevel1, effectRealPointsPerLevel2, effectRealPointsPerLevel3, effectBasePoints1, effectBasePoints2, effectBasePoints3, effectMechanic1, effectMechanic2, effectMechanic3, effectImplicitTargetA1, effectImplicitTargetA2, effectImplicitTargetA3, effectImplicitTargetB1, effectImplicitTargetB2, effectImplicitTargetB3, effectRadiusIndex1, effectRadiusIndex2, effectRadiusIndex3, effectApplyAuraName1, effectApplyAuraName2, effectApplyAuraName3, effectAmplitude1, effectAmplitude2, effectAmplitude3, effectMultipleValue1, effectMultipleValue2, effectMultipleValue3, effectChainTarget1, effectChainTarget2, effectChainTarget3, effectItemType1, effectItemType2, effectItemType3, effectMiscValue1, effectMiscValue2, effectMiscValue3, effectMiscValueB1, effectMiscValueB2, effectMiscValueB3, effectTriggerSpell1, effectTriggerSpell2, effectTriggerSpell3, effectPointsPerComboPoint1, effectPointsPerComboPoint2, effectPointsPerComboPoint3, spellVisual, spellIconID, activeIconID, priority, spellName1, spellName2, spellName3, spellName4, spellName5, spellName6, spellName7, spellName8, spellName9, spellName10, spellName11, spellName12, spellName13, spellName14, spellName15, spellName16, rank1, rank2, rank3, rank4, rank5, rank6, rank7, rank8, rank9, rank10, rank11, rank12, rank13, rank14, rank15, rank16, description1, manaCostPercentage, startRecoveryCategory, startRecoveryTime, maxTargetLevel, spellFamilyName, spellFamilyFlags, maxAffectedTargets, dmgClass, preventionType, dmgMultiplier1, dmgMultiplier2, dmgMultiplier3, totemCategory1, totemCategory2, areaId, schoolMask) VALUES
-- Learn Deadly Poison V
(25347,	0,	0,	0,	65568,	1024,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	14,	0,	0,	15,	0,	0,	0,	101,	0,	0,	0,	70,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	2931,	0,	8925,	0,	0,	0,	0,	0,	1,	0,	1,	0,	0,	0,	0,	0,	-1,	0,	0,	24,	0,	0,	1,	0,	0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	20844,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	92,	513,	0,	0,	'Deadly Poison V',	'',	'Poison mortel V',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'Rang 5',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'Coats a weapon with poison that lasts for 1 hour.Each strike has a 30% chance of poisoning the enemy for $27187o1 Nature damage over $27187d.  Stacks up to 5 times on a single target.',	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	1,	1,	0,	0,	0,	1),
-- Grace of Air Totem III
(25359,	0,	0,	0,	65536,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	0,	0,	101,	0,	0,	56,	56,	4,	0,	310,	0,	0,	0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	-1,	0,	0,	28,	0,	0,	1,	0,	0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	4,	0,	0,	0,	0,	0,	43,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	15463,	0,	0,	83,	0,	0,	0,	0,	0,	0,	0,	0,	319,	337,	0,	0,	'Grace of Air Totem',	'',	'Totem de Grâce aérienne',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'Rang 3',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'Summons a Grace of Air Totem with $s1 health at the feet of the caster.  The totem increases the agility of party members within $10626a1 yards by $10626s1.  Lasts $d.',	0,	133,	1000,	0,	11,	536870912,	0,	1,	1,	1,	1,	1,	3,	0,	0,	8),
-- Cripping Poison II
(11202,	0,	0,	0,	65552,	131104,	8200,	0,	0,	0,	0,	0,	0,	16,	0,	0,	0,	0,	0,	0,	0,	14,	0,	0,	15,	0,	0,	0,	30,	0,	0,	20,	20,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	2,	173555,	0,	54,	0,	0,	1,	0,	0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	1799,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	603,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	215,	163,	0,	0,	'Crippling Poison',	'',	'Poison affaiblissant',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'Rang 2',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'Coats a weapon with poison that lasts for 1 hour.Each strike has a $h% chance of poisoning the enemy, slowing their movement speed by $3409s1% for $3409d.',	0,	0,	0,	0,	8,	16384,	0,	0,	0,	1,	1,	1,	0,	0,	0,	1),
-- Teaches Blood Pact Rank 6
(27488,	0,	0,	0,	537198592,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	14,	0,	0,	15,	0,	0,	0,	101,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	6,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	-1,	0,	0,	36,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	5,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	27268,	0,	0,	0,	0,	0,	5599,	541,	0,	0,	'Blood Pact',	'',	'Pacte de sang',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'Rang 6',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'',	'Teaches Imp Blood Pact (Rank 6).',	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	1,	1,	0,	0,	0,	1);

ALTER TABLE npc_trainer
DROP FOREIGN KEY npc_trainer_ibfk_2,
ADD FOREIGN KEY (entry) REFERENCES creature_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (spell) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE npc_vendor ADD FOREIGN KEY (item) REFERENCES item_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE item_template
CHANGE spellid_1 spellid_1 int(10) unsigned NULL AFTER RangedModRange,
CHANGE spellid_2 spellid_2 int(10) unsigned NULL AFTER spellcategorycooldown_1,
CHANGE spellid_3 spellid_3 int(10) unsigned NULL AFTER spellcategorycooldown_2,
CHANGE spellid_4 spellid_4 int(10) unsigned NULL AFTER spellcategorycooldown_3,
CHANGE spellid_5 spellid_5 int(10) unsigned NULL AFTER spellcategorycooldown_4,
CHANGE DisenchantID DisenchantID mediumint(8) unsigned NULL AFTER ScriptName;
UPDATE item_template SET spellid_1 = NULL WHERE spellid_1 = 0;
UPDATE item_template SET spellid_2 = NULL WHERE spellid_2 = 0;
UPDATE item_template SET spellid_3 = NULL WHERE spellid_3 = 0;
UPDATE item_template SET spellid_4 = NULL WHERE spellid_4 = 0;
UPDATE item_template SET spellid_5 = NULL WHERE spellid_5 = 0;
UPDATE item_template SET DisenchantID = NULL WHERE DisenchantID = 0;
ALTER TABLE item_template
ADD FOREIGN KEY (spellid_1) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (spellid_2) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (spellid_3) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (spellid_4) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (spellid_5) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (DisenchantID) REFERENCES disenchant_loot_template (Entry) ON DELETE SET NULL ON UPDATE CASCADE;
UPDATE item_template SET spellid_2 = 25359 WHERE entry = 21293;
UPDATE item_template SET spellid_2 = 25347 WHERE entry = 21302;

ALTER TABLE page_text
CHANGE next_page next_page mediumint(8) unsigned NULL AFTER text;
UPDATE page_text SET next_page = NULL WHERE next_page = 0;
ALTER TABLE page_text ADD FOREIGN KEY (next_page) REFERENCES page_text (entry) ON DELETE SET NULL ON UPDATE CASCADE;

ALTER TABLE petcreateinfo_spell
CHANGE entry entry mediumint(8) unsigned NOT NULL FIRST,
CHANGE Spell1 Spell1 int(10) unsigned NULL AFTER entry,
CHANGE Spell2 Spell2 int(10) unsigned NULL AFTER Spell1,
CHANGE Spell3 Spell3 int(10) unsigned NULL AFTER Spell2,
CHANGE Spell4 Spell4 int(10) unsigned NULL AFTER Spell3;
UPDATE petcreateinfo_spell SET Spell1 = NULL WHERE Spell1 = 0;
UPDATE petcreateinfo_spell SET Spell2 = NULL WHERE Spell2 = 0;
UPDATE petcreateinfo_spell SET Spell3 = NULL WHERE Spell3 = 0;
UPDATE petcreateinfo_spell SET Spell4 = NULL WHERE Spell4 = 0;
ALTER TABLE petcreateinfo_spell ADD FOREIGN KEY (entry) REFERENCES creature_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;
ALTER TABLE petcreateinfo_spell ADD FOREIGN KEY (Spell1) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE;
ALTER TABLE petcreateinfo_spell ADD FOREIGN KEY (Spell2) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE;
ALTER TABLE petcreateinfo_spell ADD FOREIGN KEY (Spell3) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE;
ALTER TABLE petcreateinfo_spell ADD FOREIGN KEY (Spell4) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE;


ALTER TABLE pet_name_generation
CHANGE entry entry mediumint(8) unsigned NULL AFTER word;
UPDATE pet_name_generation SET entry = NULL WHERE entry = 0;
ALTER TABLE pet_name_generation
ADD FOREIGN KEY (entry) REFERENCES creature_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE pickpocketing_loot_template
DROP FOREIGN KEY pickpocketing_loot_template_ibfk_1;

ALTER TABLE playercreateinfo_item
ADD FOREIGN KEY (itemid) REFERENCES item_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE spell_affect
CHANGE entry entry int(10) unsigned NOT NULL FIRST,
ADD FOREIGN KEY (entry) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE playercreateinfo_spell
CHANGE Spell Spell int(10) unsigned NULL AFTER class,
ADD FOREIGN KEY (Spell) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE player_factionchange_items
DROP INDEX `PRIMARY`,
CHANGE alliance_id alliance_id mediumint(8) unsigned NULL FIRST,
CHANGE horde_id horde_id mediumint(8) unsigned NULL AFTER alliance_comment;
UPDATE player_factionchange_items SET alliance_id = NULL WHERE alliance_id = 0;
UPDATE player_factionchange_items SET horde_id = NULL WHERE horde_id = 0;
UPDATE player_factionchange_items SET horde_id = 31629 WHERE horde_id = 316229;
UPDATE player_factionchange_items SET alliance_comment = (SELECT name FROM item_template WHERE entry = alliance_id);
UPDATE player_factionchange_items SET horde_comment = (SELECT name FROM item_template WHERE entry = horde_id);
ALTER TABLE player_factionchange_items
ADD FOREIGN KEY (alliance_id) REFERENCES item_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (horde_id) REFERENCES item_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE player_factionchange_items_race_specific
CHANGE item1 item1 mediumint(8) unsigned NULL AFTER race1,
CHANGE item2 item2 mediumint(8) unsigned NULL AFTER race2,
ADD FOREIGN KEY (item1) REFERENCES item_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (item2) REFERENCES item_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD PRIMARY KEY race1_item1_race2_item2 (race1, item1, race2, item2);

ALTER TABLE player_factionchange_quests
CHANGE alliance_id alliance_id mediumint(8) unsigned NULL FIRST,
CHANGE alliance_comment alliance_comment varchar(100) COLLATE 'utf8_general_ci' NULL AFTER alliance_id,
CHANGE horde_id horde_id mediumint(8) unsigned NULL AFTER alliance_comment,
CHANGE horde_comment horde_comment varchar(100) COLLATE 'utf8_general_ci' NULL AFTER horde_id;
UPDATE player_factionchange_quests SET alliance_id = NULL WHERE alliance_id = 0;
UPDATE player_factionchange_quests SET horde_id = NULL WHERE horde_id = 0;
ALTER TABLE player_factionchange_quests
ADD FOREIGN KEY (alliance_id) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (horde_id) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;
UPDATE player_factionchange_quests SET alliance_comment = (SELECT Title FROM quest_template WHERE entry = alliance_id);
UPDATE player_factionchange_quests SET horde_comment = (SELECT Title FROM quest_template WHERE entry = horde_id);

ALTER TABLE player_factionchange_quests_race_specific
CHANGE quest1 quest1 mediumint(8) unsigned NULL AFTER race1,
CHANGE quest2 quest2 mediumint(8) unsigned NULL AFTER race2;
UPDATE player_factionchange_quests_race_specific SET quest1 = NULL WHERE quest1 = 0;
UPDATE player_factionchange_quests_race_specific SET quest2 = NULL WHERE quest2 = 0;
DELETE pfqrs FROM player_factionchange_quests_race_specific pfqrs LEFT JOIN quest_template qt ON pfqrs.quest1 = qt.entry WHERE qt.entry is NULL;
DELETE pfqrs FROM player_factionchange_quests_race_specific pfqrs LEFT JOIN quest_template qt ON pfqrs.quest2 = qt.entry WHERE qt.entry is NULL;
ALTER TABLE player_factionchange_quests_race_specific
ADD FOREIGN KEY (quest1) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (quest2) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE player_factionchange_spells
CHANGE alliance_id alliance_id int(10) unsigned NOT NULL FIRST,
CHANGE horde_id horde_id int(10) unsigned NOT NULL AFTER alliance_comment;
ALTER TABLE player_factionchange_spells ADD FOREIGN KEY (alliance_id) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;
ALTER TABLE player_factionchange_spells ADD FOREIGN KEY (horde_id) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;
UPDATE player_factionchange_spells SET alliance_comment = (SELECT spellName1 FROM spell_template WHERE entry = alliance_id);
UPDATE player_factionchange_spells SET horde_comment = (SELECT spellName1 FROM spell_template WHERE entry = horde_id);

ALTER TABLE player_factionchange_spells_specific
CHANGE spell1 spell1 int(10) unsigned NULL AFTER race1,
CHANGE spell2 spell2 int(10) unsigned NULL AFTER race2;
UPDATE player_factionchange_spells_specific SET spell1 = NULL WHERE spell1 = 0;
UPDATE player_factionchange_spells_specific SET spell2 = NULL WHERE spell2 = 0;
ALTER TABLE player_factionchange_spells_specific ADD FOREIGN KEY (spell1) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;
ALTER TABLE player_factionchange_spells_specific ADD FOREIGN KEY (spell2) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;
UPDATE player_factionchange_spells_specific SET comment = (SELECT CONCAT(spellName1, ' - ', REPLACE(rank3, 'g', 'k'))  FROM spell_template WHERE entry = spell1);

ALTER TABLE pool_template
ENGINE='InnoDB';

ALTER TABLE pool_creature
ENGINE='InnoDB',
ADD FOREIGN KEY (guid) REFERENCES creature (spawnID) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (pool_entry) REFERENCES pool_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE pool_gameobject
ENGINE='InnoDB',
ADD FOREIGN KEY (guid) REFERENCES gameobject (guid) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (pool_entry) REFERENCES pool_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE pool_pool
ENGINE='InnoDB',
ADD FOREIGN KEY (pool_id) REFERENCES pool_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (mother_pool) REFERENCES pool_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE prospecting_loot_template
DROP FOREIGN KEY prospecting_loot_template_ibfk_1;

ALTER TABLE quest_details
CHANGE ID ID mediumint(8) unsigned NOT NULL FIRST,
ENGINE='InnoDB',
ADD FOREIGN KEY (ID) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE quest_end_scripts
ADD FOREIGN KEY (id) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE quest_mail_sender
CHANGE QuestId QuestId mediumint(8) unsigned NOT NULL FIRST,
ADD FOREIGN KEY (QuestId) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE quest_offer_reward
ENGINE='InnoDB',
ADD FOREIGN KEY (ID) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE quest_pool
CHANGE quest_id quest_id mediumint(8) unsigned NOT NULL AFTER pool_id,
ADD FOREIGN KEY (quest_id) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE quest_pool_current
CHANGE quest_id quest_id mediumint(8) unsigned NOT NULL AFTER pool_id,
ADD FOREIGN KEY (quest_id) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE quest_request_items
CHANGE ID ID mediumint(8) unsigned NOT NULL FIRST,
ENGINE='InnoDB',
ADD FOREIGN KEY (ID) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE quest_start_scripts
ADD FOREIGN KEY (id) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE quest_start_scripts_tmp
ADD FOREIGN KEY (id) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE quest_template
CHANGE entry entry mediumint(8) unsigned NOT NULL FIRST,
CHANGE ReqItemId1 ReqItemId1 mediumint(8) unsigned NULL AFTER ObjectiveText4,
CHANGE ReqItemId2 ReqItemId2 mediumint(8) unsigned NULL AFTER ReqItemId1,
CHANGE ReqItemId3 ReqItemId3 mediumint(8) unsigned NULL AFTER ReqItemId2,
CHANGE ReqItemId4 ReqItemId4 mediumint(8) unsigned NULL AFTER ReqItemId3,
CHANGE RewChoiceItemId1 RewChoiceItemId1 mediumint(8) unsigned NULL AFTER ReqSpellCast4,
CHANGE RewChoiceItemId2 RewChoiceItemId2 mediumint(8) unsigned NULL AFTER RewChoiceItemId1,
CHANGE RewChoiceItemId3 RewChoiceItemId3 mediumint(8) unsigned NULL AFTER RewChoiceItemId2,
CHANGE RewChoiceItemId4 RewChoiceItemId4 mediumint(8) unsigned NULL AFTER RewChoiceItemId3,
CHANGE RewChoiceItemId5 RewChoiceItemId5 mediumint(8) unsigned NULL AFTER RewChoiceItemId4,
CHANGE RewChoiceItemId6 RewChoiceItemId6 mediumint(8) unsigned NULL AFTER RewChoiceItemId5,
CHANGE RewItemId1 RewItemId1 mediumint(8) unsigned NULL AFTER RewChoiceItemCount6,
CHANGE RewItemId2 RewItemId2 mediumint(8) unsigned NULL AFTER RewItemId1,
CHANGE RewItemId3 RewItemId3 mediumint(8) unsigned NULL AFTER RewItemId2,
CHANGE RewItemId4 RewItemId4 mediumint(8) unsigned NULL AFTER RewItemId3,
CHANGE ReqSpellCast1 ReqSpellCast1 int(10) unsigned NULL AFTER ReqCreatureOrGOCount4,
CHANGE ReqSpellCast2 ReqSpellCast2 int(10) unsigned NULL AFTER ReqSpellCast1,
CHANGE ReqSpellCast3 ReqSpellCast3 int(10) unsigned NULL AFTER ReqSpellCast2,
CHANGE ReqSpellCast4 ReqSpellCast4 int(10) unsigned NULL AFTER ReqSpellCast3,
CHANGE RewSpell RewSpell int(10) unsigned NULL AFTER RewMoneyMaxLevel,
CHANGE RewSpellCast RewSpellCast int(10) unsigned NULL AFTER RewSpell;
UPDATE quest_template SET ReqItemId1 = NULL WHERE ReqItemId1 = 0;
UPDATE quest_template SET ReqItemId2 = NULL WHERE ReqItemId2 = 0;
UPDATE quest_template SET ReqItemId3 = NULL WHERE ReqItemId3 = 0;
UPDATE quest_template SET ReqItemId4 = NULL WHERE ReqItemId4 = 0;
UPDATE quest_template SET RewChoiceItemId1 = NULL WHERE RewChoiceItemId1 = 0;
UPDATE quest_template SET RewChoiceItemId2 = NULL WHERE RewChoiceItemId2 = 0;
UPDATE quest_template SET RewChoiceItemId3 = NULL WHERE RewChoiceItemId3 = 0;
UPDATE quest_template SET RewChoiceItemId4 = NULL WHERE RewChoiceItemId4 = 0;
UPDATE quest_template SET RewChoiceItemId5 = NULL WHERE RewChoiceItemId5 = 0;
UPDATE quest_template SET RewChoiceItemId6 = NULL WHERE RewChoiceItemId6 = 0;
UPDATE quest_template SET RewItemId1 = NULL WHERE RewItemId1 = 0;
UPDATE quest_template SET RewItemId2 = NULL WHERE RewItemId2 = 0;
UPDATE quest_template SET RewItemId3 = NULL WHERE RewItemId3 = 0;
UPDATE quest_template SET RewItemId4 = NULL WHERE RewItemId4 = 0;
UPDATE quest_template SET ReqSpellCast1 = NULL WHERE ReqSpellCast1 = 0;
UPDATE quest_template SET ReqSpellCast2 = NULL WHERE ReqSpellCast2 = 0;
UPDATE quest_template SET ReqSpellCast3 = NULL WHERE ReqSpellCast3 = 0;
UPDATE quest_template SET ReqSpellCast4 = NULL WHERE ReqSpellCast4 = 0;
UPDATE quest_template SET RewSpell = NULL WHERE RewSpell = 0;
UPDATE quest_template SET RewSpellCast = NULL WHERE RewSpellCast = 0;
ALTER TABLE quest_template
ADD FOREIGN KEY (ReqItemId1) REFERENCES item_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (ReqItemId2) REFERENCES item_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (ReqItemId3) REFERENCES item_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (ReqItemId4) REFERENCES item_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (RewChoiceItemId1) REFERENCES item_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (RewChoiceItemId2) REFERENCES item_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (RewChoiceItemId3) REFERENCES item_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (RewChoiceItemId4) REFERENCES item_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (RewChoiceItemId5) REFERENCES item_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (RewChoiceItemId6) REFERENCES item_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (RewItemId1) REFERENCES item_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (RewItemId2) REFERENCES item_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (RewItemId3) REFERENCES item_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (RewItemId4) REFERENCES item_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (ReqSpellCast1) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (ReqSpellCast2) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (ReqSpellCast3) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (ReqSpellCast4) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (RewSpell) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE,
ADD FOREIGN KEY (RewSpellCast) REFERENCES spell_template (entry) ON DELETE SET NULL ON UPDATE CASCADE;

ALTER TABLE recovery_item
CHANGE item_id item_id mediumint(8) unsigned NOT NULL AFTER id,
ADD FOREIGN KEY (item_id) REFERENCES item_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE recovery_spell
ADD FOREIGN KEY (spell_id) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE reference_loot_template
DROP FOREIGN KEY reference_loot_template_ibfk_1;

ALTER TABLE shop_items_random
CHANGE item_id item_id mediumint(8) unsigned NOT NULL AFTER category,
ADD FOREIGN KEY (item_id) REFERENCES item_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE skinning_loot_template
DROP FOREIGN KEY skinning_loot_template_ibfk_1;

ALTER TABLE spell_area
DROP INDEX `PRIMARY`,
CHANGE spell spell int(10) unsigned NOT NULL FIRST,
CHANGE quest_start quest_start mediumint(8) unsigned NULL AFTER area,
CHANGE quest_end quest_end mediumint(8) unsigned NULL AFTER quest_start,
ENGINE='InnoDB',
ADD FOREIGN KEY (spell) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;
UPDATE spell_area SET quest_start = NULL WHERE quest_start = 0;
UPDATE spell_area SET quest_end = NULL WHERE quest_end = 0;
ALTER TABLE spell_area
ADD FOREIGN KEY (quest_start) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (quest_end) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE spell_bonus_data
CHANGE entry entry int(10) unsigned NOT NULL FIRST,
ENGINE='InnoDB',
ADD FOREIGN KEY (entry) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE spell_custom_attr
DROP FOREIGN KEY spell_custom_attr_ibfk_1,
ADD FOREIGN KEY (entry) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE spell_disabled
DROP FOREIGN KEY spell_disabled_ibfk_1,
ADD FOREIGN KEY (entry) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE spell_elixir
DROP FOREIGN KEY spell_elixir_ibfk_1,
ADD FOREIGN KEY (entry) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE spell_group
ENGINE='InnoDB',
ROW_FORMAT=DEFAULT;

ALTER TABLE spell_group_stack_rules
CHANGE group_id group_id int(10) unsigned NOT NULL FIRST,
ENGINE='InnoDB',
ADD FOREIGN KEY (group_id) REFERENCES spell_group (id) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE spell_learn_spell
DROP FOREIGN KEY spell_learn_spell_ibfk_1,
DROP FOREIGN KEY spell_learn_spell_ibfk_2,
ADD FOREIGN KEY (entry) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (SpellID) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE spell_pet_auras
DROP FOREIGN KEY spell_pet_auras_ibfk_1,
DROP FOREIGN KEY spell_pet_auras_ibfk_2,
ADD FOREIGN KEY (spell) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (aura) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE spell_proc_event
DROP FOREIGN KEY spell_proc_event_ibfk_1,
ADD FOREIGN KEY (entry) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE spell_ranks
ENGINE='InnoDB',
ROW_FORMAT=DEFAULT,
CHANGE first_spell_id first_spell_id int(10) unsigned NOT NULL DEFAULT '0' FIRST,
CHANGE spell_id spell_id int(10) unsigned NOT NULL DEFAULT '0' AFTER first_spell_id,
ADD FOREIGN KEY (first_spell_id) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (spell_id) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE spell_required
CHANGE spell_id spell_id int(10) unsigned NOT NULL DEFAULT '0' FIRST,
CHANGE req_spell req_spell int(10) unsigned NOT NULL DEFAULT '0' AFTER spell_id,
ADD FOREIGN KEY (spell_id) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (req_spell) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE spell_scripts
DROP FOREIGN KEY spell_scripts_ibfk_1,
ADD FOREIGN KEY (id) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE spell_target_position
ADD FOREIGN KEY (id) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE spell_threat
DROP FOREIGN KEY spell_threat_ibfk_1,
ADD FOREIGN KEY (entry) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

TRUNCATE TABLE war_effort;
ALTER TABLE war_effort
CHANGE quest_id quest_id mediumint(8) unsigned NOT NULL COMMENT 'Quest ID, one row each' FIRST,
ADD FOREIGN KEY (quest_id) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE areatrigger_involvedrelation
DROP FOREIGN KEY areatrigger_involvedrelation_ibfk_1,
ADD FOREIGN KEY (quest) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE battlemaster_entry
DROP FOREIGN KEY battlemaster_entry_ibfk_1,
ADD FOREIGN KEY (entry) REFERENCES creature_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (bg_template) REFERENCES battleground_template (id) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE creature_questender
DROP FOREIGN KEY creature_questender_ibfk_1,
DROP FOREIGN KEY creature_questender_ibfk_3,
ADD FOREIGN KEY (id) REFERENCES creature_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (quest) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;


ALTER TABLE creature_queststarter
DROP FOREIGN KEY creature_queststarter_ibfk_1,
DROP FOREIGN KEY creature_queststarter_ibfk_2,
ADD FOREIGN KEY (id) REFERENCES creature_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (quest) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE creature_template_addon
DROP FOREIGN KEY creature_template_addon_ibfk_1,
ADD FOREIGN KEY (entry) REFERENCES creature_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE creature_text
DROP FOREIGN KEY creature_text_ibfk_1,
ADD FOREIGN KEY (CreatureID) REFERENCES creature_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE disenchant_loot_template
DROP FOREIGN KEY disenchant_loot_template_ibfk_1,
ADD FOREIGN KEY (Item) REFERENCES item_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE fishing_loot_template DROP FOREIGN KEY fishing_loot_template_ibfk_1;

ALTER TABLE gameobject
DROP FOREIGN KEY gameobject_ibfk_1,
ADD FOREIGN KEY (id) REFERENCES gameobject_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE game_event_battleground_holiday
DROP FOREIGN KEY game_event_battleground_holiday_ibfk_1,
ADD FOREIGN KEY (event) REFERENCES game_event (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE game_event_condition
DROP FOREIGN KEY game_event_condition_ibfk_1,
ADD FOREIGN KEY (event_id) REFERENCES game_event (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE game_event_gameobject_quest
DROP INDEX `PRIMARY`,
CHANGE id id mediumint(8) unsigned NOT NULL FIRST,
CHANGE quest quest mediumint(8) unsigned NOT NULL AFTER id,
CHANGE event event int(11) unsigned NOT NULL AFTER quest,
ADD FOREIGN KEY (id) REFERENCES gameobject_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (quest) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (event) REFERENCES game_event (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE game_event_npc_trainer
DROP FOREIGN KEY game_event_npc_trainer_ibfk_1,
DROP FOREIGN KEY game_event_npc_trainer_ibfk_2,
ADD FOREIGN KEY (spell) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (entry) REFERENCES creature_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE game_event_pool
ADD FOREIGN KEY (eventEntry) REFERENCES game_event (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (pool_entry) REFERENCES pool_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE game_event_quest_condition
CHANGE quest quest mediumint(8) unsigned NOT NULL FIRST,
CHANGE event_id event_id int(11) unsigned NOT NULL AFTER quest,
ADD FOREIGN KEY (quest) REFERENCES quest_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (event_id) REFERENCES game_event (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE broadcast_text
ENGINE='InnoDB';

ALTER TABLE locales_broadcast_text
ENGINE='InnoDB',
ADD FOREIGN KEY (ID) REFERENCES broadcast_text (ID) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE locales_creature
DROP FOREIGN KEY locales_creature_ibfk_1,
ADD FOREIGN KEY (entry) REFERENCES creature_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE locales_gameobject
DROP FOREIGN KEY locales_gameobject_ibfk_1,
ADD FOREIGN KEY (entry) REFERENCES gameobject_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE locales_item
DROP FOREIGN KEY locales_item_ibfk_1,
ADD FOREIGN KEY (entry) REFERENCES item_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE mail_loot_template
DROP FOREIGN KEY mail_loot_template_ibfk_1;

ALTER TABLE pet_levelstats
DROP FOREIGN KEY pet_levelstats_ibfk_1,
ADD FOREIGN KEY (creature_entry) REFERENCES creature_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE player_factionchange_spells_priest_specific
CHANGE spell1 spell1 int(10) unsigned NULL AFTER race1,
CHANGE spell2 spell2 int(10) unsigned NULL AFTER race2;
UPDATE player_factionchange_spells_priest_specific SET spell1 = NULL WHERE spell1 = 0;
UPDATE player_factionchange_spells_priest_specific SET spell2 = NULL WHERE spell2 = 0;
ALTER TABLE player_factionchange_spells_priest_specific
ADD FOREIGN KEY (spell1) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE,
ADD FOREIGN KEY (spell2) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

-- Translations
REPLACE INTO player_factionchange_titles (alliance_id, alliance_comment, horde_id, horde_comment) VALUES
(1,	'Private',	15,	'Scout'),
(2,	'Corporal',	16,	'Grunt'),
(3,	'Sergeant',	17,	'Sergeant'),
(4,	'Master Sergeant',	18,	'Senior Sergeant'),
(5,	'Sergeant Major',	19,	'First Sergeant'),
(6,	'Knight',	20,	'Stone Guard'),
(7,	'Knight-Lieutenant',	21,	'Blood Guard'),
(8,	'Knight-Captain',	22,	'Legionnare'),
(9,	'Knight-Champion',	23,	'Centurion'),
(10,	'Lieutenant Commander',	24,	'Champion'),
(11,	'Commander',	25,	'Lieutenant General'),
(12,	'Marshal',	26,	'General'),
(13,	'Field Marshal',	27,	'Warlord'),
(14,	'Grand Marshal',	28,	'High Warlord'),
(48,	'Justicar',	47,	'Conqueror');

ALTER TABLE skill_discovery_template
DROP FOREIGN KEY skill_discovery_template_ibfk_1,
ADD FOREIGN KEY (spellId) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE skill_extra_item_template
DROP FOREIGN KEY skill_extra_item_template_ibfk_1,
ADD FOREIGN KEY (spellId) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE transport_events
DROP FOREIGN KEY transport_events_ibfk_1,
ADD FOREIGN KEY (entry) REFERENCES transports (entry) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE waypoint_data
CHANGE action action int(11) unsigned NULL AFTER move_type;
UPDATE waypoint_data SET action = NULL WHERE action = 0;
ALTER TABLE waypoint_data
ADD FOREIGN KEY (action) REFERENCES waypoint_scripts (id) ON DELETE CASCADE ON UPDATE CASCADE;
