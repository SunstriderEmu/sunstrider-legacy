-- fixd instance Blackwing Lair enter text
update access_requirement set quest_failed_text=11903 where id=24;
Replace INTO `trinity_string` VALUES (11903, 'You must complete the quest \"Blackhand\'s Command\" before entering', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

-- fixd instance Hyjal Summit enter text
update access_requirement set quest_failed_text=11904 where id=29;
Replace INTO `trinity_string` VALUES (11904, 'You must complete the quest \"The Vials of Eternity\" before entering.', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

-- fixd instance Serpentshrine Cavern enter text
update access_requirement set quest_failed_text=11905 where id=37;
Replace INTO `trinity_string` VALUES (11905, 'You must complete the quest \"The Mark of Vashj\" before entering.', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

-- fixd instance The Black Morass enter text
update access_requirement set quest_failed_text=11906 where id=16;
Replace INTO `trinity_string` VALUES (11906, 'You can\'t enter Black Morass until you rescue Thrall from Durnholde Keep.', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
