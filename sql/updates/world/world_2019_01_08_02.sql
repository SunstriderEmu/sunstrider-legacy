-- Remove creature_gossip
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 14 AND SourceGroup = 11743 AND SourceEntry = 11743 AND ConditionTypeOrReference = 31 AND ConditionValue2 = 10445 AND ConditionValue3 = 43675;
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 14 AND SourceGroup = 11482 AND SourceEntry = 11482 AND ConditionTypeOrReference = 31 AND ConditionValue2 = 14823 AND ConditionValue3 = 56348;
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 14 AND SourceGroup = 7790 AND SourceEntry = 7790 AND ConditionTypeOrReference = 31 AND ConditionValue2 = 15303 AND ConditionValue3 = 52415;
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 14 AND SourceGroup = 9688 AND SourceEntry = 9688 AND ConditionTypeOrReference = 31 AND ConditionValue2 = 19148 AND ConditionValue3 = 89420;
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 14 AND SourceGroup = 9688 AND SourceEntry = 9688 AND ConditionTypeOrReference = 31 AND ConditionValue2 = 19148 AND ConditionValue3 = 89421;
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 14 AND SourceGroup = 9710 AND SourceEntry = 9710 AND ConditionTypeOrReference = 31 AND ConditionValue2 = 19178 AND ConditionValue3 = 86355;
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 14 AND SourceGroup = 10608 AND SourceEntry = 10608 AND ConditionTypeOrReference = 31 AND ConditionValue2 = 26007 AND ConditionValue3 = 95499;
 
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorType, ErrorTextId, ScriptName, Comment, patch_min, patch_max) VALUES
(14,    11743,  11743,  0,  0,  31, 1,  3,  10445,  43675,  0,  0,  0,  '', 'Removed from creature_gossip', 0,  10),
(14,    11482,  11482,  0,  0,  31, 1,  3,  14823,  56348,  0,  0,  0,  '', 'Removed from creature_gossip', 0,  10),
(14,    7790,   7790,   0,  0,  31, 1,  3,  15303,  52415,  0,  0,  0,  '', 'Removed from creature_gossip', 0,  10),
(14,    9688,   9688,   0,  0,  31, 1,  3,  19148,  89420,  0,  0,  0,  '', 'Removed from creature_gossip', 0,  10),
(14,    9688,   9688,   0,  0,  31, 1,  3,  19148,  89421,  0,  0,  0,  '', 'Removed from creature_gossip', 0,  10),
(14,    9710,   9710,   0,  0,  31, 1,  3,  19178,  86355,  0,  0,  0,  '', 'Removed from creature_gossip', 0,  10),
(14,    10608,  10608,  0,  0,  31, 1,  3,  26007,  95499,  0,  0,  0,  '', 'Removed from creature_gossip', 0,  10);
 
DROP TABLE IF EXISTS creature_gossip;