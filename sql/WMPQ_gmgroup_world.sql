DROP TABLE IF EXISTS `gmgroups`;
CREATE TABLE `gmgroups` (
	id int not null primary key auto_increment,
	name text not null,
	policy int not null,
	commands text not null
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
