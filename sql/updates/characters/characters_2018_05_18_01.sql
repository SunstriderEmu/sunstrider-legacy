DROP TABLE IF EXISTS mail;
DROP TABLE IF EXISTS mail_items;
DROP TABLE IF EXISTS item_text;

CREATE TABLE `mail` (
  `id` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'Identifier',
  `messageType` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `stationery` tinyint(3) NOT NULL DEFAULT 41,
  `mailTemplateId` smallint(5) unsigned NOT NULL DEFAULT 0,
  `sender` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'Character Global Unique Identifier',
  `receiver` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'Character Global Unique Identifier',
  `subject` longtext DEFAULT NULL,
  `itemTextId` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'item_text ref',
  `has_items` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `expire_time` int(10) unsigned NOT NULL DEFAULT 0,
  `deliver_time` int(10) unsigned NOT NULL DEFAULT 0,
  `money` int(10) unsigned NOT NULL DEFAULT 0,
  `cod` int(10) unsigned NOT NULL DEFAULT 0,
  `checked` tinyint(3) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `idx_receiver` (`receiver`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Mail System';

CREATE TABLE `mail_items` (
  `mail_id` int(10) unsigned NOT NULL DEFAULT 0,
  `item_guid` int(10) unsigned NOT NULL DEFAULT 0,
  `receiver` int(10) unsigned NOT NULL DEFAULT 0 COMMENT 'Character Global Unique Identifier',
  PRIMARY KEY (`item_guid`),
  KEY `idx_receiver` (`receiver`),
  KEY `idx_mail_id` (`mail_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

ALTER TABLE `item_instance`   
  CHANGE `template` `itemEntry` MEDIUMINT(8) UNSIGNED DEFAULT 0  NOT NULL,
  CHANGE `creator` `creatorGuid` INT(11) UNSIGNED DEFAULT 0  NOT NULL,
  CHANGE `gift_creator` `giftCreatorGuid` INT(11) UNSIGNED DEFAULT 0  NOT NULL,
  CHANGE `stacks` `count` SMALLINT(5) UNSIGNED DEFAULT 0  NOT NULL,
  ADD COLUMN `itemTextId` INT(10) UNSIGNED DEFAULT 0  NOT NULL AFTER `durability`;
   
CREATE TABLE `item_text` (
  `id` int(11) unsigned NOT NULL DEFAULT '0',
  `text` longtext,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Item System';
