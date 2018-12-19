ALTER TABLE `gossip_menu_option`   
  CHANGE `action_poi_id` `action_poi_id` MEDIUMINT(8) UNSIGNED NULL;
  
UPDATE gossip_menu_option SET action_poi_id = NULL WHERE action_poi_id = 0;

ALTER TABLE `gossip_menu_option`   
  ADD CONSTRAINT `pk_poi` FOREIGN KEY (`action_poi_id`) REFERENCES `points_of_interest`(`entry`) ON UPDATE CASCADE ON DELETE SET NULL,
  ADD CONSTRAINT `pk_broadcast1` FOREIGN KEY (`OptionBroadcastTextID`) REFERENCES `broadcast_text`(`ID`) ON UPDATE CASCADE ON DELETE SET NULL
  ADD CONSTRAINT `pk_broadcast2` FOREIGN KEY (`BoxBroadcastTextID`) REFERENCES `broadcast_text`(`ID`) ON UPDATE CASCADE ON DELETE SET NULL;
