DROP TABLE IF EXISTS mon_players;
CREATE TABLE mon_players (
    time int unsigned not null,
    active int unsigned not null,
    queued int unsigned not null
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS mon_timediff;
CREATE TABLE mon_timediff (
    time int unsigned not null,
    diff mediumint unsigned not null
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS mon_maps;
CREATE TABLE mon_maps (
    time int unsigned not null,
    map mediumint unsigned not null,
    players int unsigned not null
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS mon_races;
CREATE TABLE mon_races (
    time int unsigned not null,
    race tinyint unsigned not null,
    players int unsigned not null
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS mon_classes;
CREATE TABLE mon_classes (
    time int unsigned not null,
    `class` tinyint unsigned not null,
    players int unsigned not null
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
