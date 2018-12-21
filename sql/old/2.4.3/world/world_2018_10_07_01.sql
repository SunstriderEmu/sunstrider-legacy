UPDATE creature_template SET faction = 17 WHERE entry = 1706;
DELETE FROM smart_scripts WHERE entryorguid = 1706 AND id = 3;