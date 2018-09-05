-- Add new NPC/Gameobject commands
DELETE FROM `command` WHERE `name` IN ('npc spawngroup', 'npc despawngroup', 'gobject spawngroup', 'gobject despawngroup', 'list respawns');
INSERT INTO `command` (`name`, `security`, `help`) VALUES
('npc spawngroup', 5, 'Syntax: .npc spawngroup $groupId [ignorerespawn] [force]'),
('npc despawngroup', 5, 'Syntax: .npc despawngroup $groupId [removerespawntime]'),
('gobject spawngroup', 5, 'Syntax: .gobject spawngroup $groupId [ignorerespawn] [force]'),
('gobject despawngroup', 5, 'Syntax: .gobject despawngroup $groupId [removerespawntime]'),
('list respawns', 3, 'Syntax: .list respawns [distance]	

Lists all pending respawns within <distance> yards, or within current zone if not specified.');

-- Drop now obsolete column
ALTER TABLE command DROP COLUMN ircAllowed;
