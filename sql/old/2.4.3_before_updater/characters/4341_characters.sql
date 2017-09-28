DROP TABLE IF EXISTS character_custom_xp;
CREATE TABLE character_custom_xp (
  guid int(11) unsigned not null,
  custom_xp float not null,
  primary key(guid)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
