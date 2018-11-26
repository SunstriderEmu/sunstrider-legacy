ALTER TABLE `creature_formations`   
  CHANGE `leaderGUID` `leaderGUID` INT(10) UNSIGNED DEFAULT 0 NULL;

UPDATE creature_formations SET leaderGUID = NULL WHERE leaderGUID = memberGUID;

ALTER TABLE `creature_formations`   
  CHANGE `leaderGUID` `leaderGUID` INT(10) UNSIGNED DEFAULT 0 NULL CHECK (`leaderGUID` != `memberGuid`);
