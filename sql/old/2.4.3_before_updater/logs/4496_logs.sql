DROP TABLE IF EXISTS warden_fails;
CREATE TABLE warden_fails (
  guid int(11) unsigned not null,
  account int(11) unsigned not null,
  check_id int(4) unsigned not null,
  comment text not null default "",
  `time` bigint(11) not null
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
