UPDATE conditions SET ConditionValue1 = 9 WHERE ConditionTypeOrReference = 12 AND ConditionValue1 = 128;
UPDATE game_event_creature SET event = 9 WHERE event = 128;
UPDATE game_event_creature SET event = -9 WHERE event = -128;
UPDATE game_event_gameobject SET event = 9 WHERE event = 128;
UPDATE game_event_gameobject SET event = -9 WHERE event = -128;
UPDATE game_event_pool SET eventEntry = 9 WHERE eventEntry = 128;
-- all other game_event_* table have no reference to this event (checked one by one)

DELETE FROM game_event WHERE entry = 128;

UPDATE game_event SET description = 'Children of Goldshire' WHERE entry = 76;