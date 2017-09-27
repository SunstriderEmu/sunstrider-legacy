ALTER TABLE characters ADD COLUMN `arenaPoints` int(10) UNSIGNED NOT NULL default '0' AFTER arena_pending_points;
UPDATE characters SET arenapoints = CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 1564), ' ', -1) AS UNSIGNED);
UPDATE characters SET arenapoints = arenapoints + arena_pending_points;
UPDATE characters SET arena_pending_points = 0;
