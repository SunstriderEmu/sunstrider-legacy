UPDATE creature SET patch_min = 5 WHERE spawnId = 208042;

UPDATE creature SET patch_max = 4 WHERE spawnId IN (1002001, 1000261, 1000262, 1000263, 1000264, 1000267, 1000343, 1045294, 1078875, 1078876, 1078877, 1079007);
DELETE FROM gameobject WHERE guid = 1012143;

ALTER TABLE `game_event`   
  CHANGE `disabled` `disabled` TINYINT(3) UNSIGNED DEFAULT 0  NOT NULL;

INSERT INTO game_event (entry, start_time, end_time, occurence, length, description, world_event) VALUES (82, "2016-10-30 14:00:00", "2020-12-31 06:00:00", 10080, 180, "Stranglethorn Fishing Extravaganza Turn-ins", 0);