ALTER TABLE characters
ADD COLUMN `chosenTitle` int(10) UNSIGNED NOT NULL default '0' AFTER yesterdayKills,
ADD COLUMN `watchedFaction` int(10) NOT NULL default '0' AFTER chosenTitle,
ADD COLUMN `drunk` smallint(5) UNSIGNED NOT NULL default '0' AFTER watchedFaction,
ADD COLUMN `health` int(10) UNSIGNED NOT NULL default '0' AFTER drunk,
ADD COLUMN `power1` int(10) UNSIGNED NOT NULL default '0' AFTER health,
ADD COLUMN `power2` int(10) UNSIGNED NOT NULL default '0' AFTER power1,
ADD COLUMN `power3` int(10) UNSIGNED NOT NULL default '0' AFTER power2,
ADD COLUMN `power4` int(10) UNSIGNED NOT NULL default '0' AFTER power3,
ADD COLUMN `power5` int(10) UNSIGNED NOT NULL default '0' AFTER power4;

UPDATE characters SET
chosenTitle = SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ',  648))+2, length(SUBSTRING_INDEX(data, ' ',  648+1))- length(SUBSTRING_INDEX(data, ' ',  648)) - 1),
watchedFaction = SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ', 1519))+2, length(SUBSTRING_INDEX(data, ' ', 1519+1))- length(SUBSTRING_INDEX(data, ' ', 1519)) - 1),
drunk = SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ',  241))+2, length(SUBSTRING_INDEX(data, ' ',  241+1))- length(SUBSTRING_INDEX(data, ' ',  241)) - 1) & 0xFFFE,
health = SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ',   22))+2, length(SUBSTRING_INDEX(data, ' ',   22+1))- length(SUBSTRING_INDEX(data, ' ',   22)) - 1),
power1 = SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ',   23))+2, length(SUBSTRING_INDEX(data, ' ',   23+1))- length(SUBSTRING_INDEX(data, ' ',   23)) - 1),
power2 = SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ',   24))+2, length(SUBSTRING_INDEX(data, ' ',   24+1))- length(SUBSTRING_INDEX(data, ' ',   24)) - 1),
power3 = SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ',   25))+2, length(SUBSTRING_INDEX(data, ' ',   25+1))- length(SUBSTRING_INDEX(data, ' ',   25)) - 1),
power4 = SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ',   26))+2, length(SUBSTRING_INDEX(data, ' ',   26+1))- length(SUBSTRING_INDEX(data, ' ',   26)) - 1),
power5 = SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ',   27))+2, length(SUBSTRING_INDEX(data, ' ',   27+1))- length(SUBSTRING_INDEX(data, ' ',   27)) - 1);

