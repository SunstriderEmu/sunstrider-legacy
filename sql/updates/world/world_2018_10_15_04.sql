UPDATE instance_spawn_groups SET bossStateId = 1 WHERE spawnGroupId = 29;
UPDATE instance_spawn_groups SET bossStateId = 4 WHERE spawnGroupId = 30;
UPDATE instance_spawn_groups SET bossStateId = 5 WHERE spawnGroupId = 28;
UPDATE instance_spawn_groups SET bossStateId = 3 WHERE spawnGroupId = 27;
UPDATE instance_spawn_groups SET bossStateId = 0 WHERE spawnGroupId = 26;

REPLACE INTO `gameobject` VALUES (166411, 184697, 548, 1, -229.089, -331.166, 32.88, 4.41059, 0, 0, 0.805368, -0.592775, 25, 0, 1, '');
