ALTER TABLE characters
ADD COLUMN `totalHonorPoints` int(10) UNSIGNED NOT NULL default '0' AFTER arenaPoints,
ADD COLUMN `todayHonorPoints` int(10) UNSIGNED NOT NULL default '0' AFTER totalHonorPoints,
ADD COLUMN `yesterdayHonorPoints` int(10) UNSIGNED NOT NULL default '0' AFTER todayHonorPoints,
ADD COLUMN `totalKills` int(10) UNSIGNED NOT NULL default '0' AFTER yesterdayHonorPoints,
ADD COLUMN `todayKills` smallint(5) UNSIGNED NOT NULL default '0' AFTER totalKills,
ADD COLUMN `yesterdayKills` smallint(5) UNSIGNED NOT NULL default '0' AFTER todayKills;

UPDATE characters SET
totalHonorPoints = SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ', 1562))+2, length(SUBSTRING_INDEX(data, ' ', 1562+1))- length(SUBSTRING_INDEX(data, ' ', 1562)) - 1),
todayHonorPoints = SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ', 1515))+2, length(SUBSTRING_INDEX(data, ' ', 1515+1))- length(SUBSTRING_INDEX(data, ' ', 1515)) - 1),
yesterdayHonorPoints = SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ', 1516))+2, length(SUBSTRING_INDEX(data, ' ', 1516+1))- length(SUBSTRING_INDEX(data, ' ', 1516)) - 1),
totalKills = SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ', 1517))+2, length(SUBSTRING_INDEX(data, ' ', 1517+1))- length(SUBSTRING_INDEX(data, ' ', 1517)) - 1),
todayKills = SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ', 1514))+2, length(SUBSTRING_INDEX(data, ' ', 1514+1))- length(SUBSTRING_INDEX(data, ' ', 1514)) - 1)  & 0x0000FFFF,
yesterdayKills = (SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ', 1514))+2, length(SUBSTRING_INDEX(data, ' ', 1514+1))- length(SUBSTRING_INDEX(data, ' ', 1514)) - 1) & 0xFFFF0000) >> 16;
