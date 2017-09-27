ALTER TABLE characters ADD playerBytes INT UNSIGNED NOT NULL default '0' AFTER `gender`;
ALTER TABLE characters ADD playerBytes2 INT UNSIGNED NOT NULL default '0' AFTER playerBytes;
ALTER TABLE characters ADD playerFlags INT UNSIGNED NOT NULL default '0' AFTER playerBytes2;

UPDATE characters SET playerBytes = CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(data, ' ',  240), ' ', -1) AS UNSIGNED);
UPDATE characters SET playerBytes2 = CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(data, ' ',  241), ' ', -1) AS UNSIGNED);
UPDATE characters SET playerFlags = CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(data, ' ',  237), ' ', -1) AS UNSIGNED);
