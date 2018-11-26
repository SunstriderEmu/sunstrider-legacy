-- Creatures
UPDATE creature_entry SET spawnID = spawnID + 1000000;
UPDATE smart_scripts SET entryorguid = entryorguid - 1000000 WHERE entryorguid < 0 AND source_type = 0;
UPDATE smart_scripts SET target_param1 = target_param1 + 1000000 WHERE target_type = 10;

-- GO
UPDATE gameobject SET guid = guid + 1000000;
UPDATE smart_scripts SET entryorguid = entryorguid - 1000000 WHERE entryorguid < 0 AND source_type = 1;
UPDATE smart_scripts SET target_param1 = target_param1 + 1000000 WHERE target_type = 14;
UPDATE gameobject_scripts SET datalong = datalong + 1000000 WHERE command IN (9,11,12);
UPDATE event_scripts SET datalong = datalong + 1000000 WHERE command IN (9,11,12);
UPDATE quest_end_scripts SET datalong = datalong + 1000000 WHERE command IN (9,11,12);
UPDATE quest_start_scripts SET datalong = datalong + 1000000 WHERE command IN (9,11,12);
UPDATE quest_start_scripts_tmp SET datalong = datalong + 1000000 WHERE command IN (9,11,12);
UPDATE spell_scripts SET datalong = datalong + 1000000 WHERE command IN (9,11,12);
UPDATE waypoint_scripts SET datalong = datalong + 1000000 WHERE command IN (9,11,12);

-- Both
UPDATE spawn_group SET spawnId = spawnId + 1000000;
UPDATE conditions SET ConditionValue3 = ConditionValue3 + 1000000 WHERE ConditionValue3 > 0 AND ConditionTypeOrReference = 31;
UPDATE conditions SET SourceEntry = SourceEntry + 1000000 WHERE SourceEntry > 0 AND SourceTypeOrReferenceId = 22;
UPDATE conditions SET SourceEntry = SourceEntry - 1000000 WHERE SourceEntry < 0 AND SourceTypeOrReferenceId = 22;