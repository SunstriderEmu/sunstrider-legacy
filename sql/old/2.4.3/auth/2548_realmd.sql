ALTER TABLE realmlist ADD COLUMN `motd` TEXT NOT NULL after `port`;
ALTER TABLE realmlist ADD COLUMN `last_twitter` TEXT NOT NULL after `motd`;
