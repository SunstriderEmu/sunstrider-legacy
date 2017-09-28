ALTER TABLE characters
ADD COLUMN `exploredZones` longtext AFTER latency;

UPDATE characters SET
exploredZones = SUBSTRING(data, length(SUBSTRING_INDEX(data, ' ', 1332))+2, length(SUBSTRING_INDEX(data, ' ', 1459+1))- length(SUBSTRING_INDEX(data, ' ', 1332)) - 1);
