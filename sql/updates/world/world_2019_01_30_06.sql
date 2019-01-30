-- 165 2.1
INSERT INTO npc_vendor SELECT ce.entry AS entry, item, maxcount, incrtime, ExtendedCost, 1 AS patch_min, 10 AS patch_max
FROM game_event_npc_vendor genv
JOIN creature_entry ce ON ce.spawnID = genv.guid
WHERE `Event` = 165 ON DUPLICATE KEY UPDATE npc_vendor.patch_min = 1;

-- 166 2.3
INSERT INTO npc_vendor SELECT ce.entry AS entry, item, maxcount, incrtime, ExtendedCost, 3 AS patch_min, 10 AS patch_max
FROM game_event_npc_vendor genv
JOIN creature_entry ce ON ce.spawnID = genv.guid
WHERE `Event` = 166 ON DUPLICATE KEY UPDATE npc_vendor.patch_min = 3;

-- 167 2.4
INSERT INTO npc_vendor SELECT ce.entry AS entry, item, maxcount, incrtime, ExtendedCost, 4 AS patch_min, 10 AS patch_max
FROM game_event_npc_vendor genv
JOIN creature_entry ce ON ce.spawnID = genv.guid
WHERE `Event` = 167 ON DUPLICATE KEY UPDATE patch_min = 4;

-- 176 2.2
INSERT INTO npc_vendor SELECT ce.entry AS entry, item, maxcount, incrtime, ExtendedCost, 2 AS patch_min, 10 AS patch_max
FROM game_event_npc_vendor genv
JOIN creature_entry ce ON ce.spawnID = genv.guid
WHERE `Event` = 176 ON DUPLICATE KEY UPDATE patch_min = 2;

DELETE FROM game_event_npc_vendor WHERE event IN (165, 166, 167, 176);
