-- 165 2.1
UPDATE creature_loot_template clt JOIN conditions c ON c.SourceGroup = clt.Entry AND clt.item = c.SourceEntry SET clt.patch_min = 1 WHERE SourceTypeOrReferenceId = 1 AND ConditionTypeOrReference = 12 AND c.ConditionValue1 = 165;
-- 166 2.3
UPDATE creature_loot_template clt JOIN conditions c ON c.SourceGroup = clt.Entry AND clt.item = c.SourceEntry SET clt.patch_min = 3 WHERE SourceTypeOrReferenceId = 1 AND ConditionTypeOrReference = 12 AND c.ConditionValue1 = 166;
-- 167 2.4
UPDATE creature_loot_template clt JOIN conditions c ON c.SourceGroup = clt.Entry AND clt.item = c.SourceEntry SET clt.patch_min = 4 WHERE SourceTypeOrReferenceId = 1 AND ConditionTypeOrReference = 12 AND c.ConditionValue1 = 167;
-- 176 2.2
UPDATE creature_loot_template clt JOIN conditions c ON c.SourceGroup = clt.Entry AND clt.item = c.SourceEntry SET clt.patch_min = 2 WHERE SourceTypeOrReferenceId = 1 AND ConditionTypeOrReference = 12 AND c.ConditionValue1 = 176;

DELETE FROM conditions WHERE SourceTypeOrReferenceId = 1 AND ConditionTypeOrReference = 12 AND ConditionValue1 IN (165, 166, 167, 176);