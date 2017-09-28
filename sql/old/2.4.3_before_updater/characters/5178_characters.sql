ALTER TABLE wrchat_channels ADD COLUMN password varchar(50) not null default '' after irc_channel;
