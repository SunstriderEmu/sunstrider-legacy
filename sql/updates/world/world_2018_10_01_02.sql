-- Arathi basin npc'
UPDATE creature_template SET ScriptName = "", unit_flags = 0x02000002 
WHERE entry IN (15045,15046,15062,15063,15064,15065,15066,15071,15072,15074,15075,15086,15087,15089,15107,15108);

DROP TABLE IF EXISTS `creature_battleground`;
CREATE TABLE `creature_battleground` (
  `spawnID` int(10) unsigned NOT NULL COMMENT 'Creature''s spawnID',
  `event1` tinyint(3) unsigned NOT NULL COMMENT 'main event',
  `event2` tinyint(3) unsigned NOT NULL COMMENT 'sub event',
  PRIMARY KEY (`spawnID`,`event1`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature battleground indexing system';

DROP TABLE IF EXISTS `battleground_events`;
CREATE TABLE `battleground_events` (
  `map` smallint(5) NOT NULL,
  `event1` tinyint(3) unsigned NOT NULL,
  `event2` tinyint(3) unsigned NOT NULL,
  `description` varchar(255) NOT NULL,
  PRIMARY KEY (`map`,`event1`,`event2`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `gameobject_battleground`;
CREATE TABLE `gameobject_battleground` (
  `spawnID` int(10) unsigned NOT NULL COMMENT 'GameObject''s spawnID',
  `event1` tinyint(3) unsigned NOT NULL COMMENT 'main event',
  `event2` tinyint(3) unsigned NOT NULL COMMENT 'sub event',
  PRIMARY KEY (`spawnID`,`event1`,`event2`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='GameObject battleground indexing system';

insert into `battleground_events` (`map`, `event1`, `event2`, `description`) values
('529','0','0','Stables - neutral'),
('529','0','1','Stables - alliance contested'),
('529','0','2','Stables - horde contested'),
('529','0','3','Stables - alliance owned'),
('529','0','4','Stables - horde owned'),
('529','1','0','Blacksmith - neutral'),
('529','1','1','Blacksmith - alliance contested'),
('529','1','2','Blacksmith - horde contested'),
('529','1','3','Blacksmith - alliance owned'),
('529','1','4','Blacksmith - horde owned'),
('529','2','0','Farm - neutral'),
('529','2','1','Farm - alliance contested'),
('529','2','2','Farm - horde contested'),
('529','2','3','Farm - alliance owned'),
('529','2','4','Farm - horde owned'),
('529','3','0','Lumber Mill - neutral'),
('529','3','1','Lumber Mill - alliance contested'),
('529','3','2','Lumber Mill - horde contested'),
('529','3','3','Lumber Mill - alliance owned'),
('529','3','4','Lumber Mill - horde owned'),
('529','4','0','Gold Mine - neutral'),
('529','4','1','Gold Mine - alliance contested'),
('529','4','2','Gold Mine - horde contested'),
('529','4','3','Gold Mine - alliance owned'),
('529','4','4','Gold Mine - horde owned'),
('529','254','0','doors');

REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311605', '529', '1200.03', '1171.09', '-56.47', '5.15', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311605, '13116');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311606', '529', '1200.03', '1171.09', '-56.47', '5.15', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311606, '13117');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311607', '529', '1017.43', '960.61', '-42.95', '4.88', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311607, '13116');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311608', '529', '1017.43', '960.61', '-42.95', '4.88', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311608, '13117');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311609', '529', '833', '793', '-57.25', '5.27', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311609, '13116');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311610', '529', '833', '793', '-57.25', '5.27', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311610, '13117');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311611', '529', '775.17', '1206.4', '15.79', '1.9', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311611, '13116');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311612', '529', '775.17', '1206.4', '15.79', '1.9', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311612, '13117');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311613', '529', '1207.48', '787', '-83.36', '5.51', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311613, '13116');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311614', '529', '1207.48', '787', '-83.36', '5.51', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311614, '13117');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311615', '529', '1200.75', '802.971', '-103.325', '5.0091', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311615, '15075');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311615, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311616', '529', '990.042', '1014.51', '-42.5199', '6.16101', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311616, '15063');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311616, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311617', '529', '980.162', '989.083', '-43.9306', '0.191986', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311617, '15063');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311617, '69');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311618', '529', '1008.77', '1005.8', '-44.0248', '6.21337', '5', '1');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311618, '15065');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311619', '529', '1017.91', '1008.77', '-44.3462', '1.27155', '5', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311619, '15066');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311620', '529', '979.588', '997.244', '-43.9798', '0.575959', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311620, '15063');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311620, '69');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311621', '529', '996.188', '1003.25', '-42.5221', '4.72984', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311621, '15063');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311621, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311622', '529', '983.693', '1008.41', '-42.5199', '5.35816', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311622, '15063');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311622, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311623', '529', '969.127', '999.597', '-43.9439', '2.3911', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311623, '15063');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311623, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311624', '529', '991.74', '1000.92', '-42.5199', '2.67035', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311624, '15063');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311624, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311625', '529', '1197.5', '860.736', '-98.6642', '1.60941', '5', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311625, '15074');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311626', '529', '1184.96', '1200.12', '-56.3163', '0.733038', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311626, '15086');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311627', '529', '1189.31', '1183.63', '-56.0443', '2.44346', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311627, '15107');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311628', '529', '1169.2', '1163.53', '-56.4816', '1.07924', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311628, '15107');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311629', '529', '1186.86', '884.933', '-103.591', '1.41372', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311629, '15075');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311629, '133');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311630', '529', '979.603', '997.367', '-43.9784', '1.01229', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311630, '15064');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311630, '69');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311631', '529', '1201.63', '1174.92', '-56.3803', '5.14872', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311631, '15087');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311631, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311632', '529', '1187.37', '1199.46', '-56.3711', '5.2709', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311632, '15087');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311633', '529', '1249.79', '794.31', '-102.989', '0.750492', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311633, '15074');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311633, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311634', '529', '1184.03', '834.479', '-102.975', '4.90438', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311634, '15074');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311634, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311635', '529', '1231.03', '786.665', '-102.642', '3.63029', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311635, '15074');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311635, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311636', '529', '1226.12', '816.11', '-102.404', '1.01229', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311636, '15074');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311636, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311637', '529', '1258.31', '775.819', '-105.636', '6.23082', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311637, '15074');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311637, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311638', '529', '1139.79', '809.247', '-99.5951', '2.02458', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311638, '15074');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311639', '529', '1235.97', '802.637', '-103.035', '3.64774', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311639, '15074');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311639, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311640', '529', '1202.03', '810.835', '-103.166', '1.46608', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311640, '15074');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311640, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311641', '529', '1138.93', '811.071', '-99.5951', '5.16617', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311641, '15074');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311642', '529', '751.929', '1198.46', '18.1631', '3.38594', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311642, '15089');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311642, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311643', '529', '1214.26', '803.153', '-102.681', '5.25344', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311643, '15074');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311643, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311644', '529', '786.069', '824.469', '-55.9782', '5.05953', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311644, '15045');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311645', '529', '822.796', '869.174', '-57.843', '4.24193', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311645, '15045');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311646', '529', '841.648', '864.919', '-57.4323', '1.26686', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311646, '15045');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311647', '529', '847.3', '836.659', '-57.8924', '5.89317', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311647, '15045');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311648', '529', '811.552', '792.473', '-57.8178', '0.471185', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311648, '15045');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311649', '529', '822.677', '867.832', '-57.7916', '4.67619', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311649, '15046');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311650', '529', '812.433', '791.921', '-57.7863', '2.91346', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311650, '15046');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311651', '529', '1186.12', '1190.79', '-56.0364', '2.35619', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311651, '15108');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311652', '529', '1191.47', '1185.54', '-56.0253', '2.30383', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311652, '15108');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311653', '529', '1167.13', '1186.34', '-56.2799', '4.59022', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311653, '15108');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311654', '529', '1175.07', '1155.1', '-56.4464', '0.728899', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311654, '15108');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311655', '529', '1171.04', '1144.41', '-56.0605', '3.24631', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311655, '15108');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311656', '529', '1184.85', '1179.24', '-55.9334', '2.21657', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311656, '15108');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311657', '529', '1160.12', '1169.93', '-56.3168', '2.04316', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311657, '15108');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311658', '529', '983.581', '1008.55', '-42.5199', '5.34071', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311658, '15064');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311658, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311659', '529', '767.403', '1118.95', '17.3766', '2.33874', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311659, '15062');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311659, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311660', '529', '760.732', '1083.22', '15.693', '4.03171', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311660, '15062');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311660, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311661', '529', '751.68', '1198.45', '18.232', '3.35103', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311661, '15062');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311661, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311662', '529', '884.481', '1176.69', '9.99647', '4.08407', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311662, '15062');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311662, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311663', '529', '873.048', '1264.44', '18.8553', '1.39626', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311663, '15062');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311663, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311664', '529', '908.642', '1183.09', '5.05999', '0.820305', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311664, '15062');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311664, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311665', '529', '909.461', '1232.72', '6.57686', '3.29867', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311665, '15062');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311665, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311666', '529', '815.829', '1088.29', '9.53132', '0.0523599', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311666, '15062');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311666, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311667', '529', '830.316', '1136.88', '11.3574', '5.14872', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311667, '15062');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311667, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311668', '529', '838.966', '1241.34', '16.6564', '3.75246', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311668, '15062');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311668, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311669', '529', '812.093', '1161.07', '11.6124', '2.3911', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311669, '15062');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311669, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311670', '529', '1186.39', '1190.51', '-56.009', '2.35619', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311670, '15107');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311671', '529', '1232.78', '788.376', '-102.684', '2.6529', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311671, '15075');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311671, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311672', '529', '1158.07', '1165.48', '-56.2712', '4.56288', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311672, '15107');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311673', '529', '1187.5', '1181.29', '-55.9973', '2.40855', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311673, '15107');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311674', '529', '1157.58', '1172.96', '-56.2799', '2.84489', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311674, '15107');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311675', '529', '1153.16', '1154.29', '-56.4169', '2.5387', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311675, '15107');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311676', '529', '791.34', '822.852', '-56.4006', '1.8326', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311676, '15046');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311676, '69');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311677', '529', '815.915', '1088.65', '9.58834', '6.23082', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311677, '15089');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311677, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311678', '529', '849.179', '833.499', '-57.7416', '5.61996', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311678, '15046');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311679', '529', '920.323', '1251.82', '8.46854', '3.40339', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311679, '15089');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311679, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311680', '529', '764.691', '1147.38', '18.9711', '1.74533', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311680, '15089');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311680, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311681', '529', '908.765', '1183.24', '5.00921', '0.331613', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311681, '15089');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311681, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311682', '529', '908.431', '1248.12', '9.23548', '0.890118', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311682, '15089');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311682, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311683', '529', '909.529', '1232.81', '6.55989', '3.47321', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311683, '15089');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311683, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311684', '529', '884.648', '1176.52', '9.96061', '3.94444', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311684, '15089');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311684, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311685', '529', '787.803', '1230.45', '18.6236', '3.89208', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311685, '15089');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311685, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311686', '529', '761.081', '1082.74', '15.5728', '3.80482', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311686, '15089');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311686, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311687', '529', '831.255', '1236.06', '17.2953', '0.506145', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311687, '15089');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311687, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311688', '529', '742.763', '1243.59', '22.7741', '1.3439', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311688, '15089');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311688, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311689', '529', '872.633', '1264.29', '18.8666', '1.44862', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311689, '15089');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311689, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311690', '529', '913.165', '1311.43', '24.9025', '0.558505', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311690, '15089');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311690, '234');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311691', '529', '812.393', '1160.85', '11.6123', '2.33874', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311691, '15089');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311691, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311692', '529', '1207.82', '1198.78', '-56.1779', '2.86234', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311692, '15086');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311692, '69');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311693', '529', '1157.72', '1162.02', '-56.3494', '4.68379', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311693, '15086');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311694', '529', '1195.33', '849.472', '-98.6098', '4.39823', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311694, '15075');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311695', '529', '1258.08', '775.87', '-105.596', '6.0912', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311695, '15075');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311695, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311696', '529', '1250.52', '793.177', '-103.23', '0.471239', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311696, '15075');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311696, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311697', '529', '1229.6', '807.341', '-103.111', '4.17134', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311697, '15075');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311697, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311698', '529', '990.38', '1014.8', '-42.5199', '6.12611', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311698, '15064');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311698, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311699', '529', '996.413', '1002.88', '-42.52', '4.7822', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311699, '15064');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311699, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311700', '529', '968.975', '999.732', '-43.9377', '2.53073', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311700, '15064');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311700, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311701', '529', '1008.8', '1005.81', '-44.0267', '6.24828', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311701, '15072');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311702', '529', '1211.4', '810.13', '-102.83', '1.3439', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311702, '15075');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311702, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311703', '529', '1242.1', '808.662', '-102.936', '0.663225', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311703, '15075');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311703, '233');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311704', '529', '843.426', '857.894', '-57.6725', '2.27435', '5', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311704, '15046');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311705', '529', '815.798', '831.478', '-57.1758', '4.94565', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311705, '15045');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311706', '529', '799.065', '846.511', '-56.8651', '2.39458', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311706, '15045');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311707', '529', '792.457', '823.458', '-56.5414', '2.52741', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311707, '15045');
REPLACE INTO `creature_addon` (spawnID, `emote`) VALUES (311707, '69');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311708', '529', '815.514', '831.668', '-57.1119', '4.46503', '5', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311708, '15046');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311709', '529', '1167.45', '1182.16', '-56.3106', '5.17983', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311709, '15087');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311710', '529', '977.54', '1032.11', '-43.8551', '3.96525', '5', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311710, '15071');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311711', '529', '823.35', '817.641', '-57.6701', '1.54966', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311711, '15046');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311712', '529', '798.336', '847.737', '-56.7132', '5.14592', '0', '2');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311712, '15046');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311713', '529', '1357.17', '1280.23', '-11.2124', '4.22231', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311713, '13116');
REPLACE INTO `creature` (`spawnID`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawndist`, `MovementType`) VALUES ('311714', '529', '714.61', '646.15', '-10.87', '4.34', '0', '0');
REPLACE INTO `creature_entry` (spawnID, entry) VALUES (311714, '13117');

REPLACE INTO gameobject (guid,id,map,position_x,position_y,position_z,orientation,rotation0,rotation1,rotation2,rotation3,state) VALUES 
(166149, 180101, 529, 806.182, 874.272, -55.9937, -2.30383, 0, 0, 0, 0, 1),
(166150, 180100, 529, 806.182, 874.272, -55.9937, -2.30383, 0, 0, 0, 0, 1),
(166151, 180061, 529, 806.182, 874.272, -55.9937, -2.30383, 0, 0, 0, 0, 1),
(166152, 180060, 529, 806.182, 874.272, -55.9937, -2.30383, 0, 0, 0, 0, 1),
(166153, 180059, 529, 806.182, 874.272, -55.9937, -2.30383, 0, 0, 0, 0, 1),
(166154, 180058, 529, 806.182, 874.272, -55.9937, -2.30383, 0, 0, 0, 0, 1),
(166155, 180088, 529, 977.016, 1046.62, -44.8092, -2.60054, 0, 0, 0, 0, 1),
(166156, 180102, 529, 977.016, 1046.62, -44.8092, -2.60054, 0, 0, 0, 0, 1),
(166157, 180102, 529, 977.016, 1046.62, -44.8092, -2.60054, 0, 0, 0, 0, 1),
(166158, 180101, 529, 977.016, 1046.62, -44.8092, -2.60054, 0, 0, 0, 0, 1),
(166159, 180100, 529, 977.016, 1046.62, -44.8092, -2.60054, 0, 0, 0, 0, 1),
(166160, 180061, 529, 977.016, 1046.62, -44.8092, -2.60054, 0, 0, 0, 0, 1),
(166161, 180060, 529, 977.016, 1046.62, -44.8092, -2.60054, 0, 0, 0, 0, 1),
(166162, 180059, 529, 977.016, 1046.62, -44.8092, -2.60054, 0, 0, 0, 0, 1),
(166163, 180058, 529, 977.016, 1046.62, -44.8092, -2.60054, 0, 0, 0, 0, 1),
(166164, 180087, 529, 1166.79, 1200.13, -56.7086, 0.907571, 0, 0, 0, 0, 1),
(166165, 180102, 529, 1166.79, 1200.13, -56.7086, 0.907571, 0, 0, 0, 0, 1),
(166166, 180102, 529, 1166.79, 1200.13, -56.7086, 0.907571, 0, 0, 0, 0, 1),
(166167, 180101, 529, 1166.79, 1200.13, -56.7086, 0.907571, 0, 0, 0, 0, 1),
(166168, 180100, 529, 1166.79, 1200.13, -56.7086, 0.907571, 0, 0, 0, 0, 1),
(166169, 180061, 529, 1166.79, 1200.13, -56.7086, 0.907571, 0, 0, 0, 0, 1),
(166170, 180060, 529, 1166.79, 1200.13, -56.7086, 0.907571, 0, 0, 0, 0, 1),
(166171, 180059, 529, 1166.79, 1200.13, -56.7086, 0.907571, 0, 0, 0, 0, 1),
(166172, 180058, 529, 1166.79, 1200.13, -56.7086, 0.907571, 0, 0, 0, 0, 1),
(166173, 180102, 529, 806.182, 874.272, -55.9937, -2.30383, 0, 0, 0, 0, 1),
(166174, 180102, 529, 806.182, 874.272, -55.9937, -2.30383, 0, 0, 0, 0, 1),
(166175, 180089, 529, 806.182, 874.272, -55.9937, -2.30383, 0, 0, 0, 0, 1),
(166176, 180058, 529, 856.142, 1148.9, 11.1847, -2.30383, 0, 0, 0, 0, 1),
(166177, 180059, 529, 856.142, 1148.9, 11.1847, -2.30383, 0, 0, 0, 0, 1),
(166178, 180060, 529, 856.142, 1148.9, 11.1847, -2.30383, 0, 0, 0, 0, 1),
(166179, 180061, 529, 856.142, 1148.9, 11.1847, -2.30383, 0, 0, 0, 0, 1),
(166180, 180100, 529, 856.142, 1148.9, 11.1847, -2.30383, 0, 0, 0, 0, 1),
(166181, 180101, 529, 856.142, 1148.9, 11.1847, -2.30383, 0, 0, 0, 0, 1),
(166182, 180102, 529, 856.142, 1148.9, 11.1847, -2.30383, 0, 0, 0, 0, 1),
(166183, 180102, 529, 856.142, 1148.9, 11.1847, -2.30383, 0, 0, 0, 0, 1),
(166184, 180090, 529, 856.142, 1148.9, 11.1847, -2.30383, 0, 0, 0, 0, 1),
(166185, 180058, 529, 1146.92, 848.178, -110.917, -0.733038, 0, 0, 0, 0, 1),
(166186, 180059, 529, 1146.92, 848.178, -110.917, -0.733038, 0, 0, 0, 0, 1),
(166187, 180060, 529, 1146.92, 848.178, -110.917, -0.733038, 0, 0, 0, 0, 1),
(166188, 180061, 529, 1146.92, 848.178, -110.917, -0.733038, 0, 0, 0, 0, 1),
(166189, 180100, 529, 1146.92, 848.178, -110.917, -0.733038, 0, 0, 0, 0, 1),
(166190, 180101, 529, 1146.92, 848.178, -110.917, -0.733038, 0, 0, 0, 0, 1),
(166191, 180102, 529, 1146.92, 848.178, -110.917, -0.733038, 0, 0, 0, 0, 1),
(166192, 180102, 529, 1146.92, 848.178, -110.917, -0.733038, 0, 0, 0, 0, 1),
(166193, 180091, 529, 1146.92, 848.178, -110.917, -0.733038, 0, 0, 0, 0, 1),
(166194, 180255, 529, 1284.6, 1281.17, -15.9779, 0.706859, 0.012957, -0.060288, 0.344959, 0.93659, 1),
(166195, 180256, 529, 708.09, 708.448, -17.8342, -2.3911,  0.050291, 0.015127, 0.929217, -0.365784, 1),
(166196, 180227, 529, 834.017, 839.465, -58.286, 3.87463, 0, 0, 0, 1, 1),
(166197, 180225, 529, 816.18, 820.578, -57.735, 4.34587, 0, 0, 0, 1, 1),
(166198, 180227, 529, 857.92, 853.466, -58.5591, 4.79966, 0, 0, 0, 1, 1),
(166199, 180224, 529, 857.871, 828.833, -57.8815, 1.79769, 0, 0, 0, 1, 1),
(166200, 180224, 529, 823.58, 875.868, -58.1679, 5.81195, 0, 0, 0, 1, 1),
(166201, 180225, 529, 837.829, 828.637, -57.5969, 5.96903, 0, 0, 0, 1, 1),
(166202, 180227, 529, 819.573, 873.308, -58.2632, 6.02139, 0, 0, 0, 1, 1),
(166203, 180226, 529, 846.775, 842.62, -58.2415, 2.94959, 0, 0, 0, 1, 1),
(166204, 180226, 529, 837.201, 862.352, -57.461, 2.18166, 0, 0, 0, 1, 1),
(166205, 180227, 529, 860.246, 831.615, -57.9775, 0.59341, 0, 0, 0, 1, 1),
(166206, 180225, 529, 845.916, 880.218, -58.4203, 5.58505, 0, 0, 0, 1, 1),
(166207, 180226, 529, 814.638, 859.498, -57.709, 4.50295, 0, 0, 0, 1, 1),
(166208, 180227, 529, 844.153, 874.653, -58.1478, 5.34071, 0, 0, 0, 1, 1),
(166209, 180227, 529, 844.229, 873.405, -58.0143, 0.733038, 0, 0, 0, 1, 1),
(166210, 180225, 529, 855.921, 843.101, -57.8557, 5.55015, 0, 0, 0, 1, 1),
(166211, 180225, 529, 826.045, 846.452, -57.7163, 0.785397, 0, 0, 0, 1, 1),
(166212, 180224, 529, 825.954, 823.076, -57.8157, 5.60251, 0, 0, 0, 1, 1),
(166213, 180226, 529, 805.43, 792.625, -58.1898, 4.85202, 0, 0, 0, 1, 1),
(166214, 180224, 529, 854.253, 861.604, -58.5657, 2.54818, 0, 0, 0, 1, 1),
(166215, 180224, 529, 809.782, 833.842, -57.0651, 4.64258, 0, 0, 0, 1, 1),
(166216, 180226, 529, 844.047, 827.609, -57.8357, 2.16421, 0, 0, 0, 1, 1),
(166217, 180225, 529, 845.539, 854.315, -57.6762, 6.12611, 0, 0, 0, 1, 1),
(166218, 180224, 529, 851.503, 847.161, -57.9231, 2.91469, 0, 0, 0, 1, 1),
(166219, 180225, 529, 839.844, 846.052, -58.1728, 2.75761, 0, 0, 0, 1, 1),
(166220, 180224, 529, 821.977, 813.516, -58.0313, 3.54302, 0, 0, 0, 1, 1),
(166221, 180226, 529, 817.102, 794.594, -58.0055, 5.48033, 0, 0, 0, 1, 1),
(166222, 180225, 529, 852.552, 820.99, -57.8172, 2.82743, 0, 0, 0, 1, 1),
(166223, 180226, 529, 826.021, 814.397, -57.8523, 0.59341, 0, 0, 0, 1, 1),
(166224, 180224, 529, 840.211, 844.91, -58.1869, 1.09956, 0, 0, 0, 1, 1),
(166225, 180225, 529, 802.771, 841.938, -57.4145, 4.86947, 0, 0, 0, 1, 1),
(166226, 180224, 529, 825.284, 818.208, -57.7442, 6.24828, 0, 0, 0, 1, 1),
(166227, 180226, 529, 830.495, 826.901, -58.0094, 5.21854, 0, 0, 0, 1, 1),
(166228, 180227, 529, 805.43, 792.625, -58.1898, 4.85202, 0, 0, 0, 1, 1),
(166229, 180224, 529, 803.941, 826.62, -57.4124, 2.05949, 0, 0, 0, 1, 1),
(166230, 180227, 529, 830.495, 826.901, -58.0094, 5.21854, 0, 0, 0, 1, 1),
(166231, 180225, 529, 816.998, 851.552, -57.3722, 2.33874, 0, 0, 0, 1, 1),
(166232, 180225, 529, 793.252, 824.083, -56.5513, 3.42085, 0, 0, 0, 1, 1),
(166233, 180225, 529, 840.211, 844.91, -58.1869, 1.09956, 0, 0, 0, 1, 1),
(166234, 180224, 529, 808.805, 797.434, -57.973, 2.63544, 0, 0, 0, 1, 1),
(166235, 180226, 529, 857.92, 853.466, -58.5591, 4.79966, 0, 0, 0, 1, 1),
(166236, 180225, 529, 811.643, 800.111, -57.959, 2.72271, 0, 0, 0, 1, 1),
(166237, 180226, 529, 860.246, 831.615, -57.9775, 0.59341, 0, 0, 0, 1, 1),
(166238, 180227, 529, 817.102, 794.594, -58.0055, 5.48033, 0, 0, 0, 1, 1),
(166239, 180227, 529, 826.021, 814.397, -57.8523, 0.59341, 0, 0, 0, 1, 1),
(166240, 180226, 529, 819.573, 873.308, -58.2632, 6.02139, 0, 0, 0, 1, 1),
(166241, 180227, 529, 814.638, 859.498, -57.709, 4.50295, 0, 0, 0, 1, 1),
(166242, 180226, 529, 844.153, 874.653, -58.1478, 5.34071, 0, 0, 0, 1, 1),
(166243, 180224, 529, 831.149, 856.946, -57.9999, 5.5676, 0, 0, 0, 1, 1),
(166244, 180227, 529, 846.775, 842.62, -58.2415, 2.94959, 0, 0, 0, 1, 1),
(166245, 180225, 529, 825.452, 875.528, -58.1246, 3.94445, 0, 0, 0, 1, 1),
(166246, 180225, 529, 825.284, 818.208, -57.7442, 6.24828, 0, 0, 0, 1, 1),
(166247, 180224, 529, 816.998, 851.552, -57.3722, 2.33874, 0, 0, 0, 1, 1),
(166248, 180225, 529, 857.871, 828.833, -57.8815, 1.79769, 0, 0, 0, 1, 1),
(166249, 180224, 529, 839.844, 846.052, -58.1728, 2.75761, 0, 0, 0, 1, 1),
(166250, 180224, 529, 811.643, 800.111, -57.959, 2.72271, 0, 0, 0, 1, 1),
(166251, 180225, 529, 825.954, 823.076, -57.8157, 5.60251, 0, 0, 0, 1, 1),
(166252, 180224, 529, 838.469, 869.584, -57.8706, 1.46608, 0, 0, 0, 1, 1),
(166253, 180224, 529, 855.921, 843.101, -57.8557, 5.55015, 0, 0, 0, 1, 1),
(166254, 180226, 529, 844.229, 873.405, -58.0143, 0.733038, 0, 0, 0, 1, 1),
(166255, 180224, 529, 826.045, 846.452, -57.7163, 0.785397, 0, 0, 0, 1, 1),
(166256, 180224, 529, 793.252, 824.083, -56.5513, 3.42085, 0, 0, 0, 1, 1),
(166257, 180226, 529, 834.017, 839.465, -58.286, 3.87463, 0, 0, 0, 1, 1),
(166258, 180224, 529, 848.585, 836.068, -57.8643, 5.44543, 0, 0, 0, 1, 1),
(166259, 180222, 529, 825.452, 875.528, -58.1246, 3.94445, 0, 0, 0, 1, 1),
(166260, 180223, 529, 811.987, 861.662, -57.8103, 1.309, 0, 0, 0, 1, 1),
(166261, 180222, 529, 816.998, 851.552, -57.3722, 2.33874, 0, 0, 0, 1, 1),
(166262, 180222, 529, 821.977, 813.516, -58.0313, 3.54302, 0, 0, 0, 1, 1),
(166263, 180220, 529, 802.771, 841.938, -57.4145, 4.86947, 0, 0, 0, 1, 1),
(166264, 180222, 529, 851.503, 847.161, -57.9231, 2.91469, 0, 0, 0, 1, 1),
(166265, 180222, 529, 809.782, 833.842, -57.0651, 4.64258, 0, 0, 0, 1, 1),
(166266, 180220, 529, 840.211, 844.91, -58.1869, 1.09956, 0, 0, 0, 1, 1),
(166267, 180222, 529, 822.15, 864.645, -57.7939, 0.261798, 0, 0, 0, 1, 1),
(166268, 180220, 529, 823.58, 875.868, -58.1679, 5.81195, 0, 0, 0, 1, 1),
(166269, 180220, 529, 857.871, 828.833, -57.8815, 1.79769, 0, 0, 0, 1, 1),
(166270, 180220, 529, 811.678, 790.265, -57.9249, 0.349065, 0, 0, 0, 1, 1),
(166271, 180222, 529, 846.704, 822.094, -58.052, 0.139625, 0, 0, 0, 1, 1),
(166272, 180222, 529, 831.149, 856.946, -57.9999, 5.5676, 0, 0, 0, 1, 1),
(166273, 180223, 529, 845.916, 880.218, -58.4203, 5.58505, 0, 0, 0, 1, 1),
(166274, 180222, 529, 803.941, 826.62, -57.4124, 2.05949, 0, 0, 0, 1, 1),
(166275, 180223, 529, 808.805, 797.434, -57.973, 2.63544, 0, 0, 0, 1, 1),
(166276, 180223, 529, 835.381, 821.814, -57.9953, 0.296705, 0, 0, 0, 1, 1),
(166277, 180223, 529, 845.461, 882.717, -58.5138, 3.03684, 0, 0, 0, 1, 1),
(166278, 180223, 529, 794.964, 850.482, -56.0751, 0.157079, 0, 0, 0, 1, 1),
(166279, 180220, 529, 828.632, 812.08, -57.8347, 1.23918, 0, 0, 0, 1, 1),
(166280, 180220, 529, 854.253, 861.604, -58.5657, 2.54818, 0, 0, 0, 1, 1),
(166281, 180220, 529, 837.829, 828.637, -57.5969, 5.96903, 0, 0, 0, 1, 1),
(166282, 180222, 529, 825.954, 823.076, -57.8157, 5.60251, 0, 0, 0, 1, 1),
(166283, 180220, 529, 822.15, 864.645, -57.7939, 0.261798, 0, 0, 0, 1, 1),
(166284, 180220, 529, 845.539, 854.315, -57.6762, 6.12611, 0, 0, 0, 1, 1),
(166285, 180223, 529, 816.18, 820.578, -57.735, 4.34587, 0, 0, 0, 1, 1),
(166286, 180220, 529, 821.977, 813.516, -58.0313, 3.54302, 0, 0, 0, 1, 1),
(166287, 180222, 529, 835.381, 821.814, -57.9953, 0.296705, 0, 0, 0, 1, 1),
(166288, 180223, 529, 803.941, 826.62, -57.4124, 2.05949, 0, 0, 0, 1, 1),
(166289, 180223, 529, 823.58, 875.868, -58.1679, 5.81195, 0, 0, 0, 1, 1),
(166290, 180222, 529, 848.585, 836.068, -57.8643, 5.44543, 0, 0, 0, 1, 1),
(166291, 180223, 529, 851.503, 847.161, -57.9231, 2.91469, 0, 0, 0, 1, 1),
(166292, 180220, 529, 846.704, 822.094, -58.052, 0.139625, 0, 0, 0, 1, 1),
(166293, 180220, 529, 845.916, 880.218, -58.4203, 5.58505, 0, 0, 0, 1, 1),
(166294, 180222, 529, 826.045, 846.452, -57.7163, 0.785397, 0, 0, 0, 1, 1),
(166295, 180222, 529, 811.987, 861.662, -57.8103, 1.309, 0, 0, 0, 1, 1),
(166296, 180220, 529, 838.469, 869.584, -57.8706, 1.46608, 0, 0, 0, 1, 1),
(166297, 180222, 529, 855.921, 843.101, -57.8557, 5.55015, 0, 0, 0, 1, 1),
(166298, 180223, 529, 811.643, 800.111, -57.959, 2.72271, 0, 0, 0, 1, 1),
(166299, 180223, 529, 828.632, 812.08, -57.8347, 1.23918, 0, 0, 0, 1, 1),
(166300, 180223, 529, 840.211, 844.91, -58.1869, 1.09956, 0, 0, 0, 1, 1),
(166301, 180223, 529, 852.552, 820.99, -57.8172, 2.82743, 0, 0, 0, 1, 1),
(166302, 180223, 529, 811.678, 790.265, -57.9249, 0.349065, 0, 0, 0, 1, 1),
(166303, 180222, 529, 825.284, 818.208, -57.7442, 6.24828, 0, 0, 0, 1, 1),
(166304, 180223, 529, 802.771, 841.938, -57.4145, 4.86947, 0, 0, 0, 1, 1),
(166305, 180223, 529, 854.253, 861.604, -58.5657, 2.54818, 0, 0, 0, 1, 1),
(166306, 180220, 529, 809.782, 833.842, -57.0651, 4.64258, 0, 0, 0, 1, 1),
(166307, 180220, 529, 825.452, 875.528, -58.1246, 3.94445, 0, 0, 0, 1, 1),
(166308, 180223, 529, 837.829, 828.637, -57.5969, 5.96903, 0, 0, 0, 1, 1),
(166309, 180222, 529, 823.58, 875.868, -58.1679, 5.81195, 0, 0, 0, 1, 1),
(166310, 180223, 529, 826.045, 846.452, -57.7163, 0.785397, 0, 0, 0, 1, 1),
(166311, 180223, 529, 825.954, 823.076, -57.8157, 5.60251, 0, 0, 0, 1, 1),
(166312, 180223, 529, 848.585, 836.068, -57.8643, 5.44543, 0, 0, 0, 1, 1),
(166313, 180223, 529, 839.844, 846.052, -58.1728, 2.75761, 0, 0, 0, 1, 1),
(166314, 180220, 529, 852.552, 820.99, -57.8172, 2.82743, 0, 0, 0, 1, 1),
(166315, 180223, 529, 838.469, 869.584, -57.8706, 1.46608, 0, 0, 0, 1, 1),
(166316, 180218, 529, 812.468, 863.412, -57.8908, 2.61799, 0, 0, 0, 1, 1),
(166317, 180218, 529, 836.963, 843.832, -58.2549, 3.87463, 0, 0, 0, 1, 1),
(166318, 180218, 529, 830.999, 821.705, -57.9708, 1.06465, 0, 0, 0, 1, 1),
(166319, 180218, 529, 840.427, 872.116, -58.0577, 2.91469, 0, 0, 0, 1, 1),
(166320, 180218, 529, 817.27, 819.281, -57.7797, 3.17653, 0, 0, 0, 1, 1),
(166321, 180218, 529, 826.195, 860.879, -57.8156, 5.32326, 0, 0, 0, 1, 1),
(166322, 180219, 529, 856.244, 859.265, -58.5607, 5.49779, 0, 0, 0, 1, 1),
(166323, 180218, 529, 811.218, 861.737, -57.8359, 5.46288, 0, 0, 0, 1, 1),
(166324, 180218, 529, 809.455, 800.194, -58.0184, 1.01229, 0, 0, 0, 1, 1),
(166325, 180218, 529, 819.136, 876.383, -58.4074, 5.55015, 0, 0, 0, 1, 1),
(166326, 180218, 529, 854.632, 857.68, -58.5175, 4.29351, 0, 0, 0, 1, 1),
(166327, 180218, 529, 858.298, 849.064, -58.4833, 3.3685, 0, 0, 0, 1, 1),
(166328, 180218, 529, 839.221, 828.745, -57.591, 0.366518, 0, 0, 0, 1, 1),
(166329, 180219, 529, 846.167, 841.721, -58.3017, 1.25664, 0, 0, 0, 1, 1),
(166330, 180218, 529, 816.931, 793.236, -57.9357, 4.39823, 0, 0, 0, 1, 1),
(166331, 180218, 529, 845.042, 879.826, -58.4152, 1.83259, 0, 0, 0, 1, 1),
(166332, 180219, 529, 833.724, 823.595, -58.031, 1.309, 0, 0, 0, 1, 1),
(166333, 180219, 529, 827.689, 862.08, -57.8395, 5.44543, 0, 0, 0, 1, 1),
(166334, 180219, 529, 854.962, 825.815, -57.5597, 0.349065, 0, 0, 0, 1, 1),
(166335, 180218, 529, 826.672, 873.83, -58.1679, 3.42085, 0, 0, 0, 1, 1),
(166336, 180218, 529, 806.112, 793.431, -58.1486, 6.24828, 0, 0, 0, 1, 1),
(166337, 180219, 529, 833.548, 820.18, -57.9918, 2.63544, 0, 0, 0, 1, 1),
(166338, 180218, 529, 817.42, 790.555, -57.9141, 0.139625, 0, 0, 0, 1, 1),
(166339, 180218, 529, 828.236, 812.372, -57.8446, 5.61996, 0, 0, 0, 1, 1),
(166340, 180218, 529, 839.577, 855.204, -57.8964, 6.07375, 0, 0, 0, 1, 1),
(166341, 180218, 529, 819.957, 865.327, -57.5317, 4.64258, 0, 0, 0, 1, 1),
(166342, 180218, 529, 836.371, 845.542, -58.2662, 1.02974, 0, 0, 0, 1, 1),
(166343, 180219, 529, 846.689, 878.15, -58.3168, 0.680677, 0, 0, 0, 1, 1),
(166344, 180218, 529, 838.38, 845.352, -58.1822, 0.715585, 0, 0, 0, 1, 1),
(166345, 180218, 529, 841.295, 858.492, -57.8125, 4.34587, 0, 0, 0, 1, 1),
(166346, 180218, 529, 854.695, 861.679, -58.5732, 5.39307, 0, 0, 0, 1, 1),
(166347, 180218, 529, 842.301, 827.672, -57.747, 0.191985, 0, 0, 0, 1, 1),
(166348, 180220, 529, 808.805, 797.434, -57.973, 2.63544, 0, 0, 0, 1, 1),
(166349, 180223, 529, 793.252, 824.083, -56.5513, 3.42085, 0, 0, 0, 1, 1),
(166350, 180222, 529, 845.461, 882.717, -58.5138, 3.03684, 0, 0, 0, 1, 1),
(166351, 180222, 529, 811.678, 790.265, -57.9249, 0.349065, 0, 0, 0, 1, 1),
(166352, 180223, 529, 831.149, 856.946, -57.9999, 5.5676, 0, 0, 0, 1, 1),
(166353, 180222, 529, 828.632, 812.08, -57.8347, 1.23918, 0, 0, 0, 1, 1),
(166354, 180222, 529, 794.964, 850.482, -56.0751, 0.157079, 0, 0, 0, 1, 1),
(166355, 180220, 529, 811.643, 800.111, -57.959, 2.72271, 0, 0, 0, 1, 1),
(166356, 180223, 529, 822.15, 864.645, -57.7939, 0.261798, 0, 0, 0, 1, 1),
(166357, 180223, 529, 845.539, 854.315, -57.6762, 6.12611, 0, 0, 0, 1, 1),
(166358, 180223, 529, 825.452, 875.528, -58.1246, 3.94445, 0, 0, 0, 1, 1),
(166359, 180220, 529, 848.585, 836.068, -57.8643, 5.44543, 0, 0, 0, 1, 1),
(166360, 180222, 529, 793.252, 824.083, -56.5513, 3.42085, 0, 0, 0, 1, 1),
(166361, 180222, 529, 808.805, 797.434, -57.973, 2.63544, 0, 0, 0, 1, 1),
(166362, 180222, 529, 845.916, 880.218, -58.4203, 5.58505, 0, 0, 0, 1, 1),
(166363, 180222, 529, 816.18, 820.578, -57.735, 4.34587, 0, 0, 0, 1, 1),
(166364, 180222, 529, 838.469, 869.584, -57.8706, 1.46608, 0, 0, 0, 1, 1),
(166365, 180223, 529, 855.921, 843.101, -57.8557, 5.55015, 0, 0, 0, 1, 1),
(166366, 180220, 529, 825.954, 823.076, -57.8157, 5.60251, 0, 0, 0, 1, 1),
(166367, 180220, 529, 793.252, 824.083, -56.5513, 3.42085, 0, 0, 0, 1, 1),
(166368, 180220, 529, 825.284, 818.208, -57.7442, 6.24828, 0, 0, 0, 1, 1),
(166369, 180220, 529, 839.844, 846.052, -58.1728, 2.75761, 0, 0, 0, 1, 1),
(166370, 180220, 529, 845.461, 882.717, -58.5138, 3.03684, 0, 0, 0, 1, 1),
(166371, 180220, 529, 826.045, 846.452, -57.7163, 0.785397, 0, 0, 0, 1, 1),
(166372, 180220, 529, 855.921, 843.101, -57.8557, 5.55015, 0, 0, 0, 1, 1),
(166373, 180223, 529, 809.782, 833.842, -57.0651, 4.64258, 0, 0, 0, 1, 1),
(166374, 180223, 529, 821.977, 813.516, -58.0313, 3.54302, 0, 0, 0, 1, 1),
(166375, 180222, 529, 839.844, 846.052, -58.1728, 2.75761, 0, 0, 0, 1, 1),
(166376, 180222, 529, 845.539, 854.315, -57.6762, 6.12611, 0, 0, 0, 1, 1),
(166377, 180220, 529, 816.18, 820.578, -57.735, 4.34587, 0, 0, 0, 1, 1),
(166378, 180222, 529, 854.253, 861.604, -58.5657, 2.54818, 0, 0, 0, 1, 1),
(166379, 180222, 529, 857.871, 828.833, -57.8815, 1.79769, 0, 0, 0, 1, 1),
(166380, 180224, 529, 811.678, 790.265, -57.9249, 0.349065, 0, 0, 0, 1, 1),
(166381, 180225, 529, 831.149, 856.946, -57.9999, 5.5676, 0, 0, 0, 1, 1),
(166382, 180225, 529, 794.964, 850.482, -56.0751, 0.157079, 0, 0, 0, 1, 1),
(166383, 180225, 529, 808.805, 797.434, -57.973, 2.63544, 0, 0, 0, 1, 1),
(166384, 180225, 529, 851.503, 847.161, -57.9231, 2.91469, 0, 0, 0, 1, 1),
(166385, 180225, 529, 823.58, 875.868, -58.1679, 5.81195, 0, 0, 0, 1, 1),
(166386, 180225, 529, 822.15, 864.645, -57.7939, 0.261798, 0, 0, 0, 1, 1),
(166387, 180224, 529, 802.771, 841.938, -57.4145, 4.86947, 0, 0, 0, 1, 1),
(166388, 180227, 529, 837.201, 862.352, -57.461, 2.18166, 0, 0, 0, 1, 1),
(166389, 180224, 529, 794.964, 850.482, -56.0751, 0.157079, 0, 0, 0, 1, 1),
(166390, 180224, 529, 845.916, 880.218, -58.4203, 5.58505, 0, 0, 0, 1, 1),
(166391, 180224, 529, 837.829, 828.637, -57.5969, 5.96903, 0, 0, 0, 1, 1),
(166392, 180224, 529, 825.452, 875.528, -58.1246, 3.94445, 0, 0, 0, 1, 1),
(166393, 180225, 529, 846.704, 822.094, -58.052, 0.139625, 0, 0, 0, 1, 1),
(166394, 180224, 529, 828.632, 812.08, -57.8347, 1.23918, 0, 0, 0, 1, 1),
(166395, 180227, 529, 844.047, 827.609, -57.8357, 2.16421, 0, 0, 0, 1, 1),
(166396, 180225, 529, 811.987, 861.662, -57.8103, 1.309, 0, 0, 0, 1, 1),
(166397, 180225, 529, 854.253, 861.604, -58.5657, 2.54818, 0, 0, 0, 1, 1),
(166398, 180224, 529, 852.552, 820.99, -57.8172, 2.82743, 0, 0, 0, 1, 1),
(166399, 180225, 529, 835.381, 821.814, -57.9953, 0.296705, 0, 0, 0, 1, 1),
(166400, 180224, 529, 846.704, 822.094, -58.052, 0.139625, 0, 0, 0, 1, 1),
(166401, 180225, 529, 811.678, 790.265, -57.9249, 0.349065, 0, 0, 0, 1, 1),
(166402, 180225, 529, 821.977, 813.516, -58.0313, 3.54302, 0, 0, 0, 1, 1),
(166403, 180224, 529, 811.987, 861.662, -57.8103, 1.309, 0, 0, 0, 1, 1);

UPDATE gameobject SET rotation2 = SIN(orientation / 2), rotation3 = COS(orientation / 2) WHERE map = 529 AND rotation2 = 0 AND rotation3 = 0;

INSERT INTO creature_battleground VALUES
( 311605 , 0 , 3 ),
( 311606 , 0 , 4 ),
( 311607 , 1 , 3 ),
( 311608 , 1 , 4 ),
( 311609 , 2 , 3 ),
( 311610 , 2 , 4 ),
( 311611 , 3 , 3 ),
( 311612 , 3 , 4 ),
( 311613 , 4 , 3 ),
( 311614 , 4 , 4 ),
( 311615 , 3 , 4 ),
( 311616 , 1 , 3 ),
( 311617 , 1 , 3 ),
( 311618 , 1 , 3 ),
( 311619 , 1 , 3 ),
( 311620 , 1 , 3 ),
( 311621 , 1 , 3 ),
( 311622 , 1 , 3 ),
( 311623 , 1 , 3 ),
( 311624 , 1 , 3 ),
( 311625 , 3 , 3 ),
( 311626 , 0 , 3 ),
( 311627 , 0 , 3 ),
( 311628 , 0 , 3 ),
( 311629 , 3 , 4 ),
( 311630 , 1 , 4 ),
( 311631 , 0 , 4 ),
( 311632 , 0 , 4 ),
( 311633 , 3 , 3 ),
( 311634 , 3 , 3 ),
( 311635 , 3 , 3 ),
( 311636 , 3 , 3 ),
( 311637 , 3 , 3 ),
( 311638 , 3 , 3 ),
( 311639 , 3 , 3 ),
( 311640 , 3 , 3 ),
( 311641 , 3 , 3 ),
( 311642 , 3 , 4 ),
( 311643 , 3 , 3 ),
( 311644 , 2 , 3 ),
( 311645 , 2 , 3 ),
( 311646 , 2 , 3 ),
( 311647 , 2 , 3 ),
( 311648 , 2 , 3 ),
( 311649 , 2 , 4 ),
( 311650 , 2 , 4 ),
( 311651 , 0 , 4 ),
( 311652 , 0 , 4 ),
( 311653 , 0 , 4 ),
( 311654 , 0 , 4 ),
( 311655 , 0 , 4 ),
( 311656 , 0 , 4 ),
( 311657 , 0 , 4 ),
( 311658 , 1 , 4 ),
( 311659 , 3 , 3 ),
( 311660 , 3 , 3 ),
( 311661 , 3 , 3 ),
( 311662 , 3 , 3 ),
( 311663 , 3 , 3 ),
( 311664 , 3 , 3 ),
( 311665 , 3 , 3 ),
( 311666 , 3 , 3 ),
( 311667 , 3 , 3 ),
( 311668 , 3 , 3 ),
( 311669 , 3 , 3 ),
( 311670 , 0 , 3 ),
( 311671 , 3 , 4 ),
( 311672 , 0 , 3 ),
( 311673 , 0 , 3 ),
( 311674 , 0 , 3 ),
( 311675 , 0 , 3 ),
( 311676 , 2 , 4 ),
( 311677 , 3 , 4 ),
( 311678 , 2 , 4 ),
( 311679 , 3 , 4 ),
( 311680 , 3 , 4 ),
( 311681 , 3 , 4 ),
( 311682 , 3 , 4 ),
( 311683 , 3 , 4 ),
( 311684 , 3 , 4 ),
( 311685 , 3 , 4 ),
( 311686 , 3 , 4 ),
( 311687 , 3 , 4 ),
( 311688 , 3 , 4 ),
( 311689 , 3 , 4 ),
( 311690 , 3 , 4 ),
( 311691 , 3 , 4 ),
( 311692 , 0 , 3 ),
( 311693 , 0 , 3 ),
( 311694 , 3 , 4 ),
( 311695 , 3 , 4 ),
( 311696 , 3 , 4 ),
( 311697 , 3 , 4 ),
( 311698 , 1 , 4 ),
( 311699 , 1 , 4 ),
( 311700 , 1 , 4 ),
( 311701 , 1 , 4 ),
( 311702 , 3 , 4 ),
( 311703 , 3 , 4 ),
( 311704 , 2 , 4 ),
( 311705 , 2 , 3 ),
( 311706 , 2 , 3 ),
( 311707 , 2 , 3 ),
( 311708 , 2 , 4 ),
( 311709 , 0 , 4 ),
( 311710 , 1 , 4 ),
( 311711 , 2 , 4 ),
( 311712 , 2 , 4 );

INSERT INTO gameobject_battleground VALUES
(166196,2,4),
(166197,2,4),
(166198,2,4),
(166199,2,4),
(166200,2,4),
(166201,2,4),
(166202,2,4),
(166203,2,3),
(166204,2,3),
(166205,2,4),
(166206,2,4),
(166207,2,3),
(166208,2,4),
(166209,2,4),
(166210,2,4),
(166211,2,4),
(166212,2,4),
(166213,2,3),
(166214,2,4),
(166215,2,4),
(166216,2,3),
(166217,2,4),
(166218,2,4),
(166219,2,4),
(166220,2,4),
(166221,2,3),
(166222,2,4),
(166223,2,3),
(166224,2,4),
(166225,2,4),
(166226,2,4),
(166227,2,3),
(166228,2,4),
(166229,2,4),
(166230,2,4),
(166231,2,4),
(166232,2,4),
(166233,2,4),
(166234,2,4),
(166235,2,3),
(166236,2,4),
(166237,2,3),
(166238,2,4),
(166239,2,4),
(166240,2,3),
(166241,2,4),
(166242,2,3),
(166243,2,4),
(166244,2,4),
(166245,2,4),
(166246,2,4),
(166247,2,4),
(166248,2,4),
(166249,2,4),
(166250,2,4),
(166251,2,4),
(166252,2,4),
(166253,2,4),
(166254,2,3),
(166255,2,4),
(166256,2,4),
(166257,2,3),
(166258,2,4),
(166259,2,4),
(166260,2,4),
(166261,2,4),
(166262,2,4),
(166263,2,4),
(166264,2,4),
(166265,2,4),
(166266,2,4),
(166267,2,4),
(166268,2,4),
(166269,2,4),
(166270,2,4),
(166271,2,4),
(166272,2,4),
(166273,2,4),
(166274,2,4),
(166275,2,4),
(166276,2,4),
(166277,2,4),
(166278,2,4),
(166279,2,4),
(166280,2,4),
(166281,2,4),
(166282,2,4),
(166283,2,4),
(166284,2,4),
(166285,2,4),
(166286,2,4),
(166287,2,4),
(166288,2,4),
(166289,2,4),
(166290,2,4),
(166291,2,4),
(166292,2,4),
(166293,2,4),
(166294,2,4),
(166295,2,4),
(166296,2,4),
(166297,2,4),
(166298,2,4),
(166299,2,4),
(166300,2,4),
(166301,2,4),
(166302,2,4),
(166303,2,4),
(166304,2,4),
(166305,2,4),
(166306,2,4),
(166307,2,4),
(166308,2,4),
(166309,2,4),
(166310,2,4),
(166311,2,4),
(166312,2,4),
(166313,2,4),
(166314,2,4),
(166315,2,4),
(166316,2,3),
(166317,2,3),
(166318,2,3),
(166319,2,3),
(166320,2,3),
(166321,2,3),
(166322,2,3),
(166323,2,3),
(166324,2,3),
(166325,2,3),
(166326,2,3),
(166327,2,3),
(166328,2,3),
(166329,2,3),
(166330,2,3),
(166331,2,3),
(166332,2,3),
(166333,2,3),
(166334,2,3),
(166335,2,3),
(166336,2,3),
(166337,2,3),
(166338,2,3),
(166339,2,3),
(166340,2,3),
(166341,2,3),
(166342,2,3),
(166343,2,3),
(166344,2,3),
(166345,2,3),
(166346,2,3),
(166347,2,3),
(166348,2,4),
(166349,2,4),
(166350,2,4),
(166351,2,4),
(166352,2,4),
(166353,2,4),
(166354,2,4),
(166355,2,4),
(166356,2,4),
(166357,2,4),
(166358,2,4),
(166359,2,4),
(166360,2,4),
(166361,2,4),
(166362,2,4),
(166363,2,4),
(166364,2,4),
(166365,2,4),
(166366,2,4),
(166367,2,4),
(166368,2,4),
(166369,2,4),
(166370,2,4),
(166371,2,4),
(166372,2,4),
(166373,2,4),
(166374,2,4),
(166375,2,4),
(166376,2,4),
(166377,2,4),
(166378,2,4),
(166379,2,4),
(166380,2,4),
(166381,2,4),
(166382,2,4),
(166383,2,4),
(166384,2,4),
(166385,2,4),
(166386,2,4),
(166387,2,4),
(166388,2,4),
(166389,2,4),
(166390,2,4),
(166391,2,4),
(166392,2,4),
(166393,2,4),
(166394,2,4),
(166395,2,4),
(166396,2,4),
(166397,2,4),
(166398,2,4),
(166399,2,4),
(166400,2,4),
(166401,2,4),
(166402,2,4),
(166403,2,4),
(166172,0,3),
(166171,0,1),
(166170,0,4),
(166169,0,2),
(166168,0,3),
(166167,0,4),
(166165,0,1),
(166166,0,1),
(166165,0,2),
(166166,0,2),
(166164,0,0),
(166163,1,3),
(166162,1,1),
(166161,1,4),
(166160,1,2),
(166159,1,3),
(166158,1,4),
(166156,1,1),
(166157,1,1),
(166156,1,2),
(166157,1,2),
(166155,1,0),
(166154,2,3),
(166153,2,1),
(166152,2,4),
(166151,2,2),
(166150,2,3),
(166149,2,4),
(166173,2,1),
(166174,2,1),
(166173,2,2),
(166174,2,2),
(166175,2,0),
(166176,3,3),
(166177,3,1),
(166178,3,4),
(166179,3,2),
(166180,3,3),
(166181,3,4),
(166182,3,1),
(166183,3,1),
(166182,3,2),
(166183,3,2),
(166184,3,0),
(166185,4,3),
(166186,4,1),
(166187,4,4),
(166188,4,2),
(166189,4,3),
(166190,4,4),
(166191,4,1),
(166192,4,1),
(166191,4,2),
(166192,4,2),
(166193,4,0),
(166194,254,0),
(166195,254,0);

REPLACE INTO spell_script_names VALUES (21651, "spell_opening");

replace into `conditions` (id, `SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) values
(4262, '13', '2', '24390', '0', '01', '31', '0', '5', '178364', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4263, '13', '2', '24390', '0', '02', '31', '0', '5', '178365', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4264, '13', '2', '24390', '0', '03', '31', '0', '5', '178388', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4265, '13', '2', '24390', '0', '04', '31', '0', '5', '178389', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4266, '13', '2', '24390', '0', '05', '31', '0', '5', '178393', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4267, '13', '2', '24390', '0', '06', '31', '0', '5', '178394', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4268, '13', '2', '24390', '0', '07', '31', '0', '5', '178925', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4269, '13', '2', '24390', '0', '08', '31', '0', '5', '178929', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4270, '13', '2', '24390', '0', '09', '31', '0', '5', '178935', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4271, '13', '2', '24390', '0', '10', '31', '0', '5', '178936', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4272, '13', '2', '24390', '0', '11', '31', '0', '5', '178940', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4273, '13', '2', '24390', '0', '12', '31', '0', '5', '178943', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4274, '13', '2', '24390', '0', '13', '31', '0', '5', '178944', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4275, '13', '2', '24390', '0', '14', '31', '0', '5', '178945', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4276, '13', '2', '24390', '0', '15', '31', '0', '5', '178946', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4277, '13', '2', '24390', '0', '16', '31', '0', '5', '179284', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4278, '13', '2', '24390', '0', '17', '31', '0', '5', '179285', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4279, '13', '2', '24390', '0', '18', '31', '0', '5', '179286', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4280, '13', '2', '24390', '0', '19', '31', '0', '5', '179287', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4281, '13', '2', '24390', '0', '20', '31', '0', '5', '179304', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4282, '13', '2', '24390', '0', '21', '31', '0', '5', '179305', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4283, '13', '2', '24390', '0', '22', '31', '0', '5', '179306', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4284, '13', '2', '24390', '0', '23', '31', '0', '5', '179307', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4285, '13', '2', '24390', '0', '24', '31', '0', '5', '179308', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4286, '13', '2', '24390', '0', '25', '31', '0', '5', '179310', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4287, '13', '2', '24390', '0', '26', '31', '0', '5', '179435', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4288, '13', '2', '24390', '0', '27', '31', '0', '5', '179439', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4289, '13', '2', '24390', '0', '28', '31', '0', '5', '179441', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4290, '13', '2', '24390', '0', '29', '31', '0', '5', '179443', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4291, '13', '2', '24390', '0', '30', '31', '0', '5', '179445', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4292, '13', '2', '24390', '0', '31', '31', '0', '5', '179449', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4293, '13', '2', '24390', '0', '32', '31', '0', '5', '179453', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4294, '13', '2', '24390', '0', '33', '31', '0', '5', '179465', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4295, '13', '2', '24390', '0', '34', '31', '0', '5', '179466', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4296, '13', '2', '24390', '0', '35', '31', '0', '5', '179467', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4297, '13', '2', '24390', '0', '36', '31', '0', '5', '179468', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4298, '13', '2', '24390', '0', '37', '31', '0', '5', '179470', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4299, '13', '2', '24390', '0', '38', '31', '0', '5', '179471', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4300, '13', '2', '24390', '0', '39', '31', '0', '5', '179472', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4301, '13', '2', '24390', '0', '40', '31', '0', '5', '179473', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4302, '13', '2', '24390', '0', '41', '31', '0', '5', '179481', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4303, '13', '2', '24390', '0', '42', '31', '0', '5', '179482', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4304, '13', '2', '24390', '0', '43', '31', '0', '5', '179483', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4305, '13', '2', '24390', '0', '44', '31', '0', '5', '179484', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4306, '13', '2', '24390', '0', '45', '31', '0', '5', '179697', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4307, '13', '2', '24390', '0', '46', '31', '0', '5', '180058', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4308, '13', '2', '24390', '0', '47', '31', '0', '5', '180059', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4309, '13', '2', '24390', '0', '48', '31', '0', '5', '180060', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4310, '13', '2', '24390', '0', '49', '31', '0', '5', '180061', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4311, '13', '2', '24390', '0', '50', '31', '0', '5', '180064', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4312, '13', '2', '24390', '0', '51', '31', '0', '5', '180065', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4313, '13', '2', '24390', '0', '52', '31', '0', '5', '180066', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4314, '13', '2', '24390', '0', '53', '31', '0', '5', '180067', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4315, '13', '2', '24390', '0', '54', '31', '0', '5', '180068', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4316, '13', '2', '24390', '0', '55', '31', '0', '5', '180069', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4317, '13', '2', '24390', '0', '56', '31', '0', '5', '180070', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4318, '13', '2', '24390', '0', '57', '31', '0', '5', '180071', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4319, '13', '2', '24390', '0', '58', '31', '0', '5', '180072', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4320, '13', '2', '24390', '0', '59', '31', '0', '5', '180073', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4321, '13', '2', '24390', '0', '60', '31', '0', '5', '180074', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4322, '13', '2', '24390', '0', '61', '31', '0', '5', '180075', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4323, '13', '2', '24390', '0', '62', '31', '0', '5', '180076', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4324, '13', '2', '24390', '0', '63', '31', '0', '5', '180077', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4325, '13', '2', '24390', '0', '64', '31', '0', '5', '180078', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4326, '13', '2', '24390', '0', '65', '31', '0', '5', '180079', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4327, '13', '2', '24390', '0', '66', '31', '0', '5', '180085', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4328, '13', '2', '24390', '0', '67', '31', '0', '5', '180086', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4329, '13', '2', '24390', '0', '68', '31', '0', '5', '180092', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4330, '13', '2', '24390', '0', '69', '31', '0', '5', '180093', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4331, '13', '2', '24390', '0', '70', '31', '0', '5', '180094', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4332, '13', '2', '24390', '0', '71', '31', '0', '5', '180095', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4333, '13', '2', '24390', '0', '72', '31', '0', '5', '180096', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4334, '13', '2', '24390', '0', '73', '31', '0', '5', '180097', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4335, '13', '2', '24390', '0', '74', '31', '0', '5', '180098', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4336, '13', '2', '24390', '0', '75', '31', '0', '5', '180099', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4337, '13', '2', '24390', '0', '76', '31', '0', '5', '180418', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4338, '13', '2', '24390', '0', '77', '31', '0', '5', '180419', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),
(4339, '13', '2', '24390', '0', '78', '31', '0', '5', '180420', '0', '0', '0', '0', '', 'Spell 24390 "opening" targets entry'),

(4341, '13', '1', '24391', '0', '01', '31', '0', '5', '178364', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4342, '13', '1', '24391', '0', '02', '31', '0', '5', '178365', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4343, '13', '1', '24391', '0', '03', '31', '0', '5', '178388', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4344, '13', '1', '24391', '0', '04', '31', '0', '5', '178389', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4345, '13', '1', '24391', '0', '05', '31', '0', '5', '178393', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4346, '13', '1', '24391', '0', '06', '31', '0', '5', '178394', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4347, '13', '1', '24391', '0', '07', '31', '0', '5', '178925', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4348, '13', '1', '24391', '0', '08', '31', '0', '5', '178929', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4349, '13', '1', '24391', '0', '09', '31', '0', '5', '178935', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4350, '13', '1', '24391', '0', '10', '31', '0', '5', '178936', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4351, '13', '1', '24391', '0', '11', '31', '0', '5', '178940', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4352, '13', '1', '24391', '0', '12', '31', '0', '5', '178943', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4353, '13', '1', '24391', '0', '13', '31', '0', '5', '178944', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4354, '13', '1', '24391', '0', '14', '31', '0', '5', '178945', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4355, '13', '1', '24391', '0', '15', '31', '0', '5', '178946', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4356, '13', '1', '24391', '0', '16', '31', '0', '5', '179284', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4357, '13', '1', '24391', '0', '17', '31', '0', '5', '179285', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4358, '13', '1', '24391', '0', '18', '31', '0', '5', '179286', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4359, '13', '1', '24391', '0', '19', '31', '0', '5', '179287', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4360, '13', '1', '24391', '0', '20', '31', '0', '5', '179304', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4361, '13', '1', '24391', '0', '21', '31', '0', '5', '179305', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4362, '13', '1', '24391', '0', '22', '31', '0', '5', '179306', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4363, '13', '1', '24391', '0', '23', '31', '0', '5', '179307', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4364, '13', '1', '24391', '0', '24', '31', '0', '5', '179308', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4365, '13', '1', '24391', '0', '25', '31', '0', '5', '179310', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4366, '13', '1', '24391', '0', '26', '31', '0', '5', '179435', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4367, '13', '1', '24391', '0', '27', '31', '0', '5', '179439', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4368, '13', '1', '24391', '0', '28', '31', '0', '5', '179441', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4369, '13', '1', '24391', '0', '29', '31', '0', '5', '179443', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4370, '13', '1', '24391', '0', '30', '31', '0', '5', '179445', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4371, '13', '1', '24391', '0', '31', '31', '0', '5', '179449', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4372, '13', '1', '24391', '0', '32', '31', '0', '5', '179453', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4373, '13', '1', '24391', '0', '33', '31', '0', '5', '179465', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4374, '13', '1', '24391', '0', '34', '31', '0', '5', '179466', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4375, '13', '1', '24391', '0', '35', '31', '0', '5', '179467', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4376, '13', '1', '24391', '0', '36', '31', '0', '5', '179468', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4377, '13', '1', '24391', '0', '37', '31', '0', '5', '179470', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4378, '13', '1', '24391', '0', '38', '31', '0', '5', '179471', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4379, '13', '1', '24391', '0', '39', '31', '0', '5', '179472', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4380, '13', '1', '24391', '0', '40', '31', '0', '5', '179473', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4381, '13', '1', '24391', '0', '41', '31', '0', '5', '179481', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4382, '13', '1', '24391', '0', '42', '31', '0', '5', '179482', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4383, '13', '1', '24391', '0', '43', '31', '0', '5', '179483', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4384, '13', '1', '24391', '0', '44', '31', '0', '5', '179484', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4385, '13', '1', '24391', '0', '45', '31', '0', '5', '179697', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4386, '13', '1', '24391', '0', '46', '31', '0', '5', '180058', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4387, '13', '1', '24391', '0', '47', '31', '0', '5', '180059', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4388, '13', '1', '24391', '0', '48', '31', '0', '5', '180060', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4389, '13', '1', '24391', '0', '49', '31', '0', '5', '180061', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4390, '13', '1', '24391', '0', '50', '31', '0', '5', '180064', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4391, '13', '1', '24391', '0', '51', '31', '0', '5', '180065', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4392, '13', '1', '24391', '0', '52', '31', '0', '5', '180066', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4393, '13', '1', '24391', '0', '53', '31', '0', '5', '180067', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4394, '13', '1', '24391', '0', '54', '31', '0', '5', '180068', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4395, '13', '1', '24391', '0', '55', '31', '0', '5', '180069', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4396, '13', '1', '24391', '0', '56', '31', '0', '5', '180070', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4397, '13', '1', '24391', '0', '57', '31', '0', '5', '180071', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4398, '13', '1', '24391', '0', '58', '31', '0', '5', '180072', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4399, '13', '1', '24391', '0', '59', '31', '0', '5', '180073', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4400, '13', '1', '24391', '0', '60', '31', '0', '5', '180074', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4401, '13', '1', '24391', '0', '61', '31', '0', '5', '180075', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4402, '13', '1', '24391', '0', '62', '31', '0', '5', '180076', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4403, '13', '1', '24391', '0', '63', '31', '0', '5', '180077', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4404, '13', '1', '24391', '0', '64', '31', '0', '5', '180078', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4405, '13', '1', '24391', '0', '65', '31', '0', '5', '180079', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4406, '13', '1', '24391', '0', '66', '31', '0', '5', '180085', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4407, '13', '1', '24391', '0', '67', '31', '0', '5', '180086', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4408, '13', '1', '24391', '0', '68', '31', '0', '5', '180092', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4409, '13', '1', '24391', '0', '69', '31', '0', '5', '180093', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4410, '13', '1', '24391', '0', '70', '31', '0', '5', '180094', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4411, '13', '1', '24391', '0', '71', '31', '0', '5', '180095', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4412, '13', '1', '24391', '0', '72', '31', '0', '5', '180096', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4413, '13', '1', '24391', '0', '73', '31', '0', '5', '180097', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4414, '13', '1', '24391', '0', '74', '31', '0', '5', '180098', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4415, '13', '1', '24391', '0', '75', '31', '0', '5', '180099', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4416, '13', '1', '24391', '0', '76', '31', '0', '5', '180418', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4417, '13', '1', '24391', '0', '77', '31', '0', '5', '180419', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry'),
(4418, '13', '1', '24391', '0', '78', '31', '0', '5', '180420', '0', '0', '0', '0', '', 'Spell 24391 "opening" targets entry');