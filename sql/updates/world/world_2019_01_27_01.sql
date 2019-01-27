DELETE FROM `game_event` WHERE `entry`=180;
INSERT INTO `game_event` (`entry`, `start_time`, `end_time`, `occurence`, `length`, `description`, `world_event`, `disabled`, `patch_min`, `patch_max`) VALUES 
(180, '2019-01-01 00:00:00', '2025-12-30 22:00:00', 5184000, 2592000, 'Battle for the Dark Portal', 0, 0, 0, 1);
