ALTER TABLE `gossip_menu`   
  CHANGE `patch_min` `patch_min` TINYINT(3) UNSIGNED DEFAULT 0  NOT NULL,
  CHANGE `patch_max` `patch_max` TINYINT(3) UNSIGNED DEFAULT 10  NOT NULL;

ALTER TABLE `gossip_menu_option`   
  CHANGE `patch_min` `patch_min` TINYINT(3) UNSIGNED DEFAULT 0  NOT NULL,
  CHANGE `patch_max` `patch_max` TINYINT(3) UNSIGNED DEFAULT 10  NOT NULL;
