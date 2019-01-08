-- Fix of some startup errors

-- Gossip text
UPDATE gossip_text gt
JOIN broadcast_text bt ON gt.BroadcastTextID0 = bt.ID
SET text0_0 = MaleText, text0_1 = FemaleText
WHERE gt.ID IN (8174,8179,8186,8187,8190,8282,8876,8877,9110,9662,9984,10945,13056,13065,13474,13889,14391,15858,15859,15906,16157,16432,16528,16788,17804,17805,50001,1871964,1871967,1871970,1871978,1871981,1871985,1871986,1871989,1871998,1871999,1872014,1872015,1872016,1872019,1872048,1872054,1872058,1872062,1872063,1872067,1872068,1872069,1872072,1872073,1872074,1872122,1872123,1872158,1872165,1872169,1872172,1872179,1872183,1872184,1872195,1872196,1872239,1872245,1872257,1872258,1872261,1872262,1872264,1872284,1872287,1872293,1872294,1872295,1872331,1872333,1872341,1872343,1872352,1872357,1872384,1872387,1872413,1872435,1872436,1872460,1872519,1872567,1872588,1872600,1872610,1872633,1872723,1872724,1872726,1872748,1872749);

UPDATE gossip_text gt
JOIN broadcast_text bt ON gt.BroadcastTextID1 = bt.ID
SET text1_0 = MaleText, text1_1 = FemaleText
WHERE gt.ID IN (1871986,1871987,1871994,1872004,1872010,1872014,1872015,1872332,1872333,1872335,1872344,1872345,1872346,1872348,1872349,1872351,1872352,1872355,1872357,1872359,1872368,1872372,1872377,1872382,1872511,1872517,1872518);

UPDATE gossip_text gt
JOIN broadcast_text bt ON gt.BroadcastTextID2 = bt.ID
SET text2_0 = MaleText, text2_1 = FemaleText
WHERE gt.ID IN (1872000, 1872060);

UPDATE gossip_text gt
JOIN broadcast_text bt ON gt.BroadcastTextID3 = bt.ID
SET text3_0 = MaleText, text3_1 = FemaleText
WHERE gt.ID = 1872011;

-- Creature_text
DELETE FROM creature_text WHERE CreatureID = 50125;

-- Trainer spell
ALTER TABLE creature_default_trainer
ENGINE='InnoDB',
CHANGE CreatureId CreatureId mediumint(8) unsigned NOT NULL FIRST,
CHANGE TrainerId TrainerId int(10) unsigned NOT NULL AFTER CreatureId,
ADD FOREIGN KEY (CreatureId) REFERENCES creature_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;
ALTER TABLE trainer_spell
ENGINE='InnoDB',
CHANGE SpellId SpellId int(10) unsigned NOT NULL AFTER TrainerId,
ADD FOREIGN KEY (SpellId) REFERENCES spell_template (entry) ON DELETE CASCADE ON UPDATE CASCADE;

-- Creature template
UPDATE creature_template SET dmgschool = 4 WHERE entry = 20079;
UPDATE creature_template SET npcflag = npcflag | 0x1 WHERE entry = 20165;

-- Creature
ALTER TABLE creature
CHANGE modelid modelid mediumint(8) unsigned NULL AFTER spawnMask,
DROP INDEX `PRIMARY`,
ADD PRIMARY KEY spawnID_patch_min (spawnID, patch_min);
UPDATE creature SET modelid = NULL WHERE modelid = 0;
ALTER TABLE creature
ADD FOREIGN KEY (modelid) REFERENCES creature_model_info (modelid) ON DELETE SET NULL ON UPDATE CASCADE;

CREATE TEMPORARY TABLE temporary_tlk_modelid AS SELECT * FROM creature WHERE modelid IN (27105, 26563, 24722, 24723, 24724, 23136);
UPDATE temporary_tlk_modelid SET patch_min = 5;
REPLACE INTO creature SELECT * FROM temporary_tlk_modelid;
UPDATE creature SET modelid = NULL WHERE modelid IN (27105, 26563, 24722, 24723, 24724, 23136) AND patch_min = 0;

CREATE TEMPORARY TABLE temporary_tlk_modelid1 AS SELECT * FROM creature_template WHERE modelid1 IN (28239, 28818, 28240, 28257, 22712, 24722, 24723, 24724);
UPDATE temporary_tlk_modelid1 SET patch = 5;
REPLACE INTO creature_template SELECT * FROM temporary_tlk_modelid1;
UPDATE creature_template SET modelid1 = NULL WHERE modelid1 IN (28239, 28818, 28240, 28257, 22712, 24722, 24723, 24724) AND patch = 0;

CREATE TEMPORARY TABLE temporary_tlk_modelid2 AS SELECT * FROM creature_template WHERE modelid2 IN (28224, 28258);
UPDATE temporary_tlk_modelid2 SET patch = 5;
REPLACE INTO creature_template SELECT * FROM temporary_tlk_modelid2;
UPDATE creature_template SET modelid2 = NULL WHERE modelid2 IN (28224, 28258) AND patch = 0;

CREATE TEMPORARY TABLE temporary_tlk_modelid3 AS SELECT * FROM creature_template WHERE modelid3 IN (28241, 28260, 28259);
UPDATE temporary_tlk_modelid3 SET patch = 5;
REPLACE INTO creature_template SELECT * FROM temporary_tlk_modelid3;
UPDATE creature_template SET modelid3 = NULL WHERE modelid3 IN (28241, 28260, 28259) AND patch = 0;

CREATE TEMPORARY TABLE temporary_tlk_modelid4 AS SELECT * FROM creature_template WHERE modelid4 = 28261;
UPDATE temporary_tlk_modelid4 SET patch = 5;
REPLACE INTO creature_template SELECT * FROM temporary_tlk_modelid4;
UPDATE creature_template SET modelid4 = NULL WHERE modelid4 = 28261 AND patch = 0;