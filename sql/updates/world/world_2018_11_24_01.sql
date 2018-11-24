DELETE FROM spell_group WHERE spell_id IN (17797, 17798, 17799, 17800);


UPDATE gameobject_template SET entry = 174797 WHERE name = "Gorishi Hive Hatchery";
UPDATE gameobject_template SET entry = 174793 WHERE name = "Xavian Waterfall";
UPDATE gameobject_template SET entry = 174794 WHERE name = "Mirror Lake Waterfall";
UPDATE gameobject_template SET entry = 177744 WHERE name = "Cliffspring Falls Cave Mouth";
UPDATE gameobject_template SET entry = 174796 WHERE name = "Sandsorrow Watch Water Hole";
UPDATE gameobject_template SET entry = 152598 WHERE name = "the First Tide Pool";
UPDATE gameobject_template SET entry = 152604 WHERE name = "the Second Tide Pool";
UPDATE gameobject_template SET entry = 152605 WHERE name = "the Third Tide Pool";
UPDATE gameobject_template SET entry = 152606, name = "the Fourth Tide Pool" WHERE name = "tthe Fourth Tide Pool";
UPDATE gameobject_template SET entry = 174795 WHERE name = "Auberdine Moonwell";
UPDATE gameobject_template SET entry = 19464 WHERE name = "Dreadmist Peak Pool";
UPDATE gameobject_template SET entry = 178225 WHERE name = "Dire Pool" and entry = 12;

REPLACE INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `castBarCaption`, `faction`, `flags`, `size`, `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `Data6`, `Data7`, `Data8`, `Data9`, `Data10`, `Data11`, `Data12`, `Data13`, `Data14`, `Data15`, `Data16`, `Data17`, `Data18`, `Data19`, `Data20`, `Data21`, `Data22`, `Data23`, `AIName`, `ScriptName`) values('4','6','0','Bonfire Damage','','0', '0','1','0','1','3','7902','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','','');
