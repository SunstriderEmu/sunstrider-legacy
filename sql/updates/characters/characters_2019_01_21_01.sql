ALTER TABLE `ai_playerbot_guild_tasks`   
  CHANGE `owner` `owner` INT(11) UNSIGNED NOT NULL,
  CHANGE `guildid` `guildid` INT(11) UNSIGNED NOT NULL,
  CHANGE `time` `time` INT(11) UNSIGNED NOT NULL,
  CHANGE `validIn` `validIn` INT(11) UNSIGNED NULL,
  CHANGE `value` `value` INT(11) UNSIGNED NULL;

ALTER TABLE `ai_playerbot_random_bots`   
  CHANGE `owner` `owner` INT(11) UNSIGNED NOT NULL,
  CHANGE `bot` `bot` INT(11) UNSIGNED NOT NULL,
  CHANGE `time` `time` INT(11) UNSIGNED NOT NULL,
  CHANGE `validIn` `validIn` INT(11) UNSIGNED NULL,
  CHANGE `value` `value` INT(11) UNSIGNED NULL;
