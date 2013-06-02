-- trinity_string needed for the patch
INSERT INTO trinity_string VALUES(11600, "Player %s has reached completed quests limit for this week. Use '.quest complete force [quest id or shift-click link]' to complete it anyway.", NULL, "Le joueur %s a atteint la limite de validations pour cette semaine. Pour valider cette quete malgre tout, utilisez '.quest complete force [id ou lien vers la quete]'.", NULL, NULL, NULL, NULL, NULL, NULL); 
INSERT INTO trinity_string VALUES(11601, "Player %s already had %u quests completed this week.", NULL, "Le joueur %s a deja eu %u quete(s) validee(s) cette semaine.", NULL, NULL, NULL, NULL, NULL, NULL);
INSERT INTO trinity_string VALUES(11602, "Quest %u completed for player %s.", NULL, "Quete %u validee pour le joueur %s.", NULL, NULL, NULL, NULL, NULL, NULL);
INSERT INTO trinity_string VALUES(11604, "Total quests completed this week : %u.", NULL, "Nombre total de quetes validees cette semaine : %u.",  NULL, NULL, NULL, NULL, NULL, NULL);
-- Security level for commands
INSERT INTO command VALUES ("quest countcomplete", 1, "Syntax: .quest countcomplete <nickname>. If no nick provided, it takes the select player as parameter.");
INSERT INTO command VALUES ("quest totalcount", 1, "Syntax: .quest totalcount. Shows total number of quests completed this week.");
