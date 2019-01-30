ALTER TABLE `creature_queststarter`   
  ADD COLUMN `patch_min` TINYINT(3) UNSIGNED DEFAULT 0  NOT NULL AFTER `quest`,
  ADD COLUMN `patch_max` TINYINT(3) UNSIGNED DEFAULT 10  NOT NULL AFTER `patch_min`;

-- 165 2.1
UPDATE creature_queststarter cqs JOIN game_event_creature_quest gecq ON cqs.quest = gecq.quest SET cqs.patch_min = 1 WHERE gecq.event = 165;
-- 166 2.3
UPDATE creature_queststarter cqs JOIN game_event_creature_quest gecq ON cqs.quest = gecq.quest SET cqs.patch_min = 3 WHERE gecq.event = 166;
-- 167 2.4
UPDATE creature_queststarter cqs JOIN game_event_creature_quest gecq ON cqs.quest = gecq.quest SET cqs.patch_min = 4 WHERE gecq.event = 167;
-- 176 2.2
UPDATE creature_queststarter cqs JOIN game_event_creature_quest gecq ON cqs.quest = gecq.quest SET cqs.patch_min = 2 WHERE gecq.event = 176;

DELETE FROM game_event_creature_quest WHERE event IN (165, 166, 167, 176);
