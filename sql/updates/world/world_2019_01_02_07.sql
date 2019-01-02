UPDATE gameobject_template SET data6 = 0 WHERE entry IN (
176511,
175124,
184840,
181927,
181926,
181925,
181924,
181923,
181922,
181921,
179784,
178559);

ALTER TABLE `gameobject_template`   
  CHANGE `data6` `data6` INT(10) DEFAULT 0  NOT NULL;
  
UPDATE gameobject_template SET data6 = 15000 WHERE entry = 175124;
UPDATE gameobject_template SET data6 = 15000 WHERE entry = 176511;
UPDATE gameobject_template SET data6 = -1000 WHERE entry = 178559;
UPDATE gameobject_template SET data6 = 3600000 WHERE entry = 179784;
UPDATE gameobject_template SET data6 = 3600000 WHERE entry = 181921;
UPDATE gameobject_template SET data6 = 3600000 WHERE entry = 181922;
UPDATE gameobject_template SET data6 = 3600000 WHERE entry = 181923;
UPDATE gameobject_template SET data6 = 3600000 WHERE entry = 181924;
UPDATE gameobject_template SET data6 = 3600000 WHERE entry = 181925;
UPDATE gameobject_template SET data6 = 3600000 WHERE entry = 181926;
UPDATE gameobject_template SET data6 = 3600000 WHERE entry = 181927;
UPDATE gameobject_template SET data6 = 15000 WHERE entry = 184840;
