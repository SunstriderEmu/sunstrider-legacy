
ALTER TABLE `gossip_text`   
  CHANGE `BroadcastTextID0` `BroadcastTextID0` INT(11) UNSIGNED NULL,
  CHANGE `BroadcastTextID1` `BroadcastTextID1` INT(11) UNSIGNED NULL,
  CHANGE `BroadcastTextID2` `BroadcastTextID2` INT(11) UNSIGNED NULL,
  CHANGE `BroadcastTextID3` `BroadcastTextID3` INT(11) UNSIGNED NULL,
  CHANGE `BroadcastTextID4` `BroadcastTextID4` INT(11) UNSIGNED NULL,
  CHANGE `BroadcastTextID5` `BroadcastTextID5` INT(11) UNSIGNED NULL,
  CHANGE `BroadcastTextID6` `BroadcastTextID6` INT(11) UNSIGNED NULL,
  CHANGE `BroadcastTextID7` `BroadcastTextID7` INT(11) UNSIGNED NULL;

UPDATE gossip_text SET BroadcastTextID0 = NULL WHERE BroadcastTextID0 = 0;
UPDATE gossip_text SET BroadcastTextID1 = NULL WHERE BroadcastTextID1 = 0;
UPDATE gossip_text SET BroadcastTextID2 = NULL WHERE BroadcastTextID2 = 0;
UPDATE gossip_text SET BroadcastTextID3 = NULL WHERE BroadcastTextID3 = 0;
UPDATE gossip_text SET BroadcastTextID4 = NULL WHERE BroadcastTextID4 = 0;
UPDATE gossip_text SET BroadcastTextID5 = NULL WHERE BroadcastTextID5 = 0;
UPDATE gossip_text SET BroadcastTextID6 = NULL WHERE BroadcastTextID6 = 0;
UPDATE gossip_text SET BroadcastTextID7 = NULL WHERE BroadcastTextID7 = 0;

ALTER TABLE `gossip_text`   
  ADD CONSTRAINT `broadcast0` FOREIGN KEY (`BroadcastTextID0`) REFERENCES `broadcast_text`(`ID`) ON UPDATE CASCADE ON DELETE SET NULL,
  ADD CONSTRAINT `broadcast1` FOREIGN KEY (`BroadcastTextID1`) REFERENCES `broadcast_text`(`ID`) ON UPDATE CASCADE ON DELETE SET NULL,
  ADD CONSTRAINT `broadcast2` FOREIGN KEY (`BroadcastTextID2`) REFERENCES `broadcast_text`(`ID`) ON UPDATE CASCADE ON DELETE SET NULL,
  ADD CONSTRAINT `broadcast3` FOREIGN KEY (`BroadcastTextID3`) REFERENCES `broadcast_text`(`ID`) ON UPDATE CASCADE ON DELETE SET NULL,
  ADD CONSTRAINT `broadcast4` FOREIGN KEY (`BroadcastTextID4`) REFERENCES `broadcast_text`(`ID`) ON UPDATE CASCADE ON DELETE SET NULL,
  ADD CONSTRAINT `broadcast5` FOREIGN KEY (`BroadcastTextID5`) REFERENCES `broadcast_text`(`ID`) ON UPDATE CASCADE ON DELETE SET NULL,
  ADD CONSTRAINT `broadcast6` FOREIGN KEY (`BroadcastTextID6`) REFERENCES `broadcast_text`(`ID`) ON UPDATE CASCADE ON DELETE SET NULL,
  ADD CONSTRAINT `broadcast7` FOREIGN KEY (`BroadcastTextID7`) REFERENCES `broadcast_text`(`ID`) ON UPDATE CASCADE ON DELETE SET NULL;
