ALTER TABLE characters
ADD COLUMN `equipmentCache` longtext AFTER exploredZones,
ADD COLUMN `ammoId` int(10) UNSIGNED NOT NULL default '0' AFTER equipmentCache,
ADD COLUMN `knownTitles` longtext AFTER ammoId;

UPDATE characters SET
ammoId = SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ', 1487))+2, length(SUBSTRING_INDEX(data, ' ', 1487+1))- length(SUBSTRING_INDEX(data, ' ', 1487)) - 1),
knownTitles = SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ', 924))+2, length(SUBSTRING_INDEX(data, ' ', 925+1))- length(SUBSTRING_INDEX(data, ' ', 924)) - 1),
equipmentCache = '',
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+2), ' ', -1) AS UNSIGNED)), -- 1
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+3), ' ', -1) AS UNSIGNED)), -- 1
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+18), ' ', -1) AS UNSIGNED)), -- 2
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+19), ' ', -1) AS UNSIGNED)), -- 2
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+34), ' ', -1) AS UNSIGNED)), -- 3
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+35), ' ', -1) AS UNSIGNED)), -- 3 (19 total)
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+50), ' ', -1) AS UNSIGNED)), -- 4
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+51), ' ', -1) AS UNSIGNED)), -- 4
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+66), ' ', -1) AS UNSIGNED)), -- 5
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+67), ' ', -1) AS UNSIGNED)), -- 5
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+82), ' ', -1) AS UNSIGNED)), -- 6
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+83), ' ', -1) AS UNSIGNED)), -- 6
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+98), ' ', -1) AS UNSIGNED)), -- 7
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+99), ' ', -1) AS UNSIGNED)), -- 7
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+114), ' ', -1) AS UNSIGNED)), -- 8
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+115), ' ', -1) AS UNSIGNED)), -- 8
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+130), ' ', -1) AS UNSIGNED)), -- 9
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+131), ' ', -1) AS UNSIGNED)), -- 9
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+146), ' ', -1) AS UNSIGNED)), -- 10
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+147), ' ', -1) AS UNSIGNED)), -- 10
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+162), ' ', -1) AS UNSIGNED)), -- 11
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+163), ' ', -1) AS UNSIGNED)), -- 11
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+178), ' ', -1) AS UNSIGNED)), -- 12
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+179), ' ', -1) AS UNSIGNED)), -- 12
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+194), ' ', -1) AS UNSIGNED)), -- 13
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+195), ' ', -1) AS UNSIGNED)), -- 13
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+210), ' ', -1) AS UNSIGNED)), -- 14
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+211), ' ', -1) AS UNSIGNED)), -- 14
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+226), ' ', -1) AS UNSIGNED)), -- 15
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+227), ' ', -1) AS UNSIGNED)), -- 15
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+242), ' ', -1) AS UNSIGNED)), -- 16
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+243), ' ', -1) AS UNSIGNED)), -- 16
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+258), ' ', -1) AS UNSIGNED)), -- 17
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+259), ' ', -1) AS UNSIGNED)), -- 17
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+274), ' ', -1) AS UNSIGNED)), -- 18
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+275), ' ', -1) AS UNSIGNED)), -- 18
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+290), ' ', -1) AS UNSIGNED)), -- 19
equipmentCache = CONCAT_WS(' ', equipmentCache, CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(`data`, ' ', 345+291), ' ', -1) AS UNSIGNED)); -- 19
