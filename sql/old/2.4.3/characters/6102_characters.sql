ALTER TABLE item_instance
add column template MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0',
add column creatorGUID INT(10) UNSIGNED NOT NULL DEFAULT '0',
add column giftCreatorGUID INT(10) UNSIGNED NOT NULL DEFAULT '0',
add column `count` INT(10) UNSIGNED NOT NULL DEFAULT 1,
add column duration INT(10) NOT NULL DEFAULT '0',
add column charges TINYTEXT DEFAULT '',
add column flags MEDIUMINT UNSIGNED NOT NULL DEFAULT '0',
add column enchantments TEXT NOT NULL DEFAULT '',
add column randomPropertyId SMALLINT(5) NOT NULL DEFAULT '0',
add column textID INT(10) UNSIGNED NOT NULL DEFAULT '0';
add column durability SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0';