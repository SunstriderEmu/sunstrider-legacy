ALTER TABLE `characters` ADD COLUMN `bgid` int(10) unsigned NOT NULL default '0' AFTER `latency`;
ALTER TABLE `characters` ADD COLUMN `bgteam` int(10) unsigned NOT NULL default '0' AFTER `bgid`;
ALTER TABLE `characters` ADD COLUMN `bgmap` int(10) unsigned NOT NULL default '0' AFTER `bgteam`;
ALTER TABLE `characters` ADD COLUMN `bgx` float NOT NULL default '0' AFTER `bgmap`;
ALTER TABLE `characters` ADD COLUMN `bgy` float NOT NULL default '0' AFTER `bgx`;
ALTER TABLE `characters` ADD COLUMN `bgz` float NOT NULL default '0' AFTER `bgy`;
ALTER TABLE `characters` ADD COLUMN `bgo` float NOT NULL default '0' AFTER `bgz`;
