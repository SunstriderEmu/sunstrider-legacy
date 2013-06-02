/*Table structure for table `irc_channels` */

DROP TABLE IF EXISTS `irc_channels`;
CREATE TABLE `irc_channels` (
  `GUID` int(2) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Unique GUID for the channel',
  `IRC_channel` varchar(25) NOT NULL DEFAULT '#Chat' COMMENT 'Name (including prefix) for the IRC channel to join.',
  `IRC_password` varchar(25) DEFAULT 'drowssap_CRI' COMMENT 'Password assosciated with the specified IRC channel. Case sensitive.',
  `WOW_channel` varchar(25) DEFAULT 'Chat' COMMENT 'Name of the corresponding WOW channel. Case sensitive.',
  `JoinMsg` varchar(512) DEFAULT 'Hi. I was made by Machiavelli (machiaveltman@gmail.com)' COMMENT 'The text the bot needs to say upon joining the specified IRC channel.',
  `RelayChatMask` integer(2) DEFAULT '0' COMMENT 'Determines which notifications to relay. Bitmask. 0 = Nothing, 1 = WoW Channel joins, 2 = WoW Channel leaves, 4 = WoW Channel messages, 8 = IRC channel joins, 16 = IRC Channel leaves, 32 = IRC channel messages',
  PRIMARY KEY (`GUID`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;

/*Table structure for table `irc_configurations` */
DROP TABLE IF EXISTS `irc_configurations`;
CREATE TABLE `irc_configurations` (
  `configuration_index` varchar(255) NOT NULL COMMENT 'Name for the configuration property',
  `configuration_value` varchar(255) NOT NULL COMMENT 'Value for the corresponding property.',
  `configuration_help` text(1000) DEFAULT NULL COMMENT 'Information about the configuration property',
  PRIMARY KEY (`configuration_index`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `irc_configurations` */

insert into `irc_configurations` (`configuration_index`, `configuration_value`, `configuration_help`) values('Clientside.Ping.Timeout','0','Determines whether the bot will internally disconnect from the server after a a previously detected ping interval when no incoming pings were sent. This feature allows the bot to reconnect to the network after the bot\'s network failed and no ping timeout ');
insert into `irc_configurations` (`configuration_index`, `configuration_value`, `configuration_help`) values('ClientSide.Ping.TimeoutTime','240','The amount of time the bot will wait for an incoming ping request after the ping interval before it disconnects.');
insert into `irc_configurations` (`configuration_index`, `configuration_value`, `configuration_help`) values('GameNameGUID','731720','The GUID of the character whose name will be displayed in every IRC->Game channel message. Recommended is to create a character, manually change his name to something along the lines of <IRC>, and then note his GUID here.');
insert into `irc_configurations` (`configuration_index`, `configuration_value`, `configuration_help`) values('Host','chat.wowmania.fr','The host that the bot will connect to.');
insert into `irc_configurations` (`configuration_index`, `configuration_value`, `configuration_help`) values('LoginChannelMode_0','v','The channel mode that the IRC client will try to activate in each active channel on a member who logs in with a security level 0.');
insert into `irc_configurations` (`configuration_index`, `configuration_value`, `configuration_help`) values('LoginChannelMode_1','v','The channel mode that the IRC client will try to activate in each active channel on a member who logs in with a security level 1.');
insert into `irc_configurations` (`configuration_index`, `configuration_value`, `configuration_help`) values('LoginChannelMode_2','v','The channel mode that the IRC client will try to activate in each active channel on a member who logs in with a security level 2.');
insert into `irc_configurations` (`configuration_index`, `configuration_value`, `configuration_help`) values('LoginChannelMode_3','h','The channel mode that the IRC client will try to activate in each active channel on a member who logs in with a security level 3.');
insert into `irc_configurations` (`configuration_index`, `configuration_value`, `configuration_help`) values('LoginChannelMode_4','o','The channel mode that the IRC client will try to activate in each active channel on a member who logs in with a security level 4.');
insert into `irc_configurations` (`configuration_index`, `configuration_value`, `configuration_help`) values('LoginChannelMode_5','q','The channel mode that the IRC client will try to activate in each active channel on a member who logs in with a security level 5.');
insert into `irc_configurations` (`configuration_index`, `configuration_value`, `configuration_help`) values('Nickname','Windrunner','Nickname the bot will use.');
insert into `irc_configurations` (`configuration_index`, `configuration_value`, `configuration_help`) values('Password','','Password the bot will use for the given nickname and server authentication.');
insert into `irc_configurations` (`configuration_index`, `configuration_value`, `configuration_help`) values('Perform.script','','Optional commands that the bot will run after connecting succesfuly. Seperate each raw command with a \\r');
insert into `irc_configurations` (`configuration_index`, `configuration_value`, `configuration_help`) values('Port','6697','The port that the server will connect to');
insert into `irc_configurations` (`configuration_index`, `configuration_value`, `configuration_help`) values('Realname','Windrunner','Real name to be shown');
insert into `irc_configurations` (`configuration_index`, `configuration_value`, `configuration_help`) values('User','Windrunner','Username/ident for the bot.');
insert into `irc_configurations` (`configuration_index`, `configuration_value`, `configuration_help`) values('SSL Connection','1','Determines whether the client will attempt to use an SSL connection to the server. Will not function if the target server/port does not support this.');
insert into `irc_configurations` (`configuration_index`, `configuration_value`, `configuration_help`) values('UserIdleInterval', '0', 'The interval between a logged user''s messages in seconds before he gets flagged as idle. (Will not show up in .gm irc)');

/*Data for the table `command` */
delete from `command` WHERE `name` = 'gm irc' OR `name` LIKE 'irc%';
insert into `command` (`name`, `security`, `help`) values('gm irc','3','Syntax: .gm irc\r\n\r\nDisplays a list of active Game Masters logged in on IRC.');
insert into `command` (`name`, `security`, `help`) values('irc join','5','Syntax: .irc join $channelname [$channelkey]\r\n\r\nThe IRC client will attempt to access the specified channel with channel key if provided.');
insert into `command` (`name`, `security`, `help`) values('irc notice','5','Syntax: .irc notice $target $message\r\n\r\nAttempts to send the target user on the IRC network the IRC client is logged on the message specified in form of a NOTICE command.');
insert into `command` (`name`, `security`, `help`) values('irc privmsg','5','Syntax: .irc privmsg $target $message\r\n\r\nAttempts to send the target user on the IRC network the IRC client is logged on the message specified.');
insert into `command` (`name`, `security`, `help`) values('irc', '5', 'Syntax: .irc $subcommandType');
insert into `command` (`name`, `security`, `help`) values('irc kick', '5', 'Syntax: .irc kick $channel $target [$message]\r\n\r\nAttempts to kick specified user from specified IRC channel.');
insert into `command` (`name`, `security`, `help`) values('irc quit', '5', 'Syntax: .irc quit [$message]\r\n\r\nDisconnects the IRC client from the connected network, causing it to reconnect shortly.');
insert into `command` (`name`, `security`, `help`) values('irc part', '5', 'Syntax: .irc part $channel [$message]\r\n\r\nAttempt to part the specified IRC channel.');

DELETE FROM irc_channels;
INSERT INTO irc_channels VALUES ('', '#staff', 'ovaries', 'de guilde', 'Serveur up', 63);
INSERT INTO irc_channels VALUES ('', '#staff', 'ovaries', 'Guilde', '', 63);
