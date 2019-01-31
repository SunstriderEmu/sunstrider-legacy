-- Remove Old pack58 Table
DROP TABLE IF EXISTS `pack58`;
DROP TABLE IF EXISTS `boost_pack`;

-- Create New boost_pack Table
CREATE TABLE `boost_pack`
(
  `level` smallint(5) UNSIGNED NOT NULL,
  `class` smallint(5) UNSIGNED NOT NULL,
  `type` smallint(5) UNSIGNED NOT NULL,
  
  `item` mediumint(8) UNSIGNED NOT NULL COMMENT 'item id',
  `count` mediumint(8) NOT NULL DEFAULT 1
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

ALTER TABLE `boost_pack` ADD PRIMARY KEY (`level`, `class`,`type`,`item`);

-- Pack 58
INSERT INTO `boost_pack` (`level`, `class`, `type`, `item`, `count`) VALUES
(58, 1, 0, 11815, 1), -- Hand of Justice
(58, 1, 0, 11933, 1), -- Imperial Jewel
(58, 1, 0, 12653, 1), -- Riphook
(58, 1, 0, 13098, 1), -- Painweaver Band
(58, 1, 0, 13361, 2), -- Skullforge Reaver
(58, 1, 0, 13373, 1), -- Band of Flesh
(58, 1, 0, 16730, 1), -- Breastplate of Valor
(58, 1, 0, 16731, 1), -- Helm of Valor
(58, 1, 0, 16732, 1), -- Legplates of Valor
(58, 1, 0, 16733, 1), -- Spaulders of Valor
(58, 1, 0, 16734, 1), -- Boots of Valor
(58, 1, 0, 16735, 1), -- Bracers of Valor
(58, 1, 0, 16736, 1), -- Belt of Valor
(58, 1, 0, 16737, 1), -- Gauntlets of Valor
(58, 1, 0, 22321, 1), -- Heart of Wyrmthalak
(58, 1, 0, 22337, 1), -- Shroud of Domination
(58, 1, 2, 11669, 1), -- Naglering
(58, 1, 2, 11810, 1), -- Force of Will
(58, 1, 2, 12602, 1), -- Draconian Deflector
(58, 1, 2, 12929, 1), -- Emberfury Talisman
(58, 1, 2, 12940, 1), -- Dal'Rend's Sacred Charge
(58, 1, 2, 12952, 1), -- Gyth's Skull
(58, 1, 2, 13380, 1), -- Willey's Portable Howitzer
(58, 1, 2, 13955, 1), -- Stoneform Shoulders
(58, 1, 2, 18383, 1), -- Force Imbued Gauntlets
(58, 1, 2, 18466, 1), -- Royal Seal of Eldre'Thalas
(58, 1, 2, 18503, 1), -- Kromcrush's Chestplate
(58, 1, 2, 18521, 1), -- Grimy Metal Boots
(58, 1, 2, 18689, 1), -- Phantasmal Cloak
(58, 1, 2, 18690, 1), -- Wraithplate Leggings
(58, 1, 2, 18754, 1), -- Fel Hardened Bracers
(58, 1, 2, 19051, 1), -- Girdle of the Dawn
(58, 1, 2, 22331, 1), -- Band of the Steadfast Hero
(58, 2, 0, 11815, 1), -- Hand of Justice
(58, 2, 0, 11933, 1), -- Imperial Jewel
(58, 2, 0, 12583, 1), -- Blackhand Doomsaw
(58, 2, 0, 13098, 1), -- Painweaver Band
(58, 2, 0, 13203, 1), -- Armswake Cloak
(58, 2, 0, 13373, 1), -- Band of Flesh
(58, 2, 0, 16730, 1), -- Breastplate of Valor
(58, 2, 0, 16731, 1), -- Helm of Valor
(58, 2, 0, 16732, 1), -- Legplates of Valor
(58, 2, 0, 16733, 1), -- Spaulders of Valor
(58, 2, 0, 16734, 1), -- Boots of Valor
(58, 2, 0, 16735, 1), -- Bracers of Valor
(58, 2, 0, 16736, 1), -- Belt of Valor
(58, 2, 0, 16737, 1), -- Gauntlets of Valor
(58, 2, 0, 22321, 1), -- Heart of Wyrmthalak
(58, 2, 0, 22401, 1), -- Libram of Hope
(58, 2, 1, 11819, 1), -- Second Wind
(58, 2, 1, 11923, 1), -- The Hammer of Grace
(58, 2, 1, 13178, 1), -- Rosewine Circle
(58, 2, 1, 16722, 1), -- Lightforge Bracers
(58, 2, 1, 16723, 1), -- Lightforge Belt
(58, 2, 1, 16724, 1), -- Lightforge Gauntlets
(58, 2, 1, 16725, 1), -- Lightforge Boots
(58, 2, 1, 16726, 1), -- Lightforge Breastplate
(58, 2, 1, 16727, 1), -- Lightforge Helm
(58, 2, 1, 16728, 1), -- Lightforge Legplates
(58, 2, 1, 16729, 1), -- Lightforge Spaulders
(58, 2, 1, 18472, 1), -- Royal Seal of Eldre'Thalas
(58, 2, 1, 19530, 1), -- Caretaker's Cape
(58, 2, 1, 22327, 1), -- Amulet of the Redeemed
(58, 2, 1, 22334, 1), -- Band of Mending
(58, 2, 1, 22336, 1), -- Draconian Aegis of the Legion
(58, 2, 1, 23201, 1), -- Libram of Divinity
(58, 2, 2, 11669, 1), -- Naglering
(58, 2, 2, 11810, 1), -- Force of Will
(58, 2, 2, 13091, 1), -- Medallion of Grand Marshal Morris
(58, 2, 2, 14620, 1), -- Deathbone Girdle
(58, 2, 2, 14621, 1), -- Deathbone Sabatons
(58, 2, 2, 14622, 1), -- Deathbone Gauntlets
(58, 2, 2, 14623, 1), -- Deathbone Legguards
(58, 2, 2, 14624, 1), -- Deathbone Chestplate
(58, 2, 2, 18048, 1), -- Mastersmith's Hammer
(58, 2, 2, 18384, 1), -- Bile-etched Spaulders
(58, 2, 2, 18472, 1), -- Royal Seal of Eldre'Thalas
(58, 2, 2, 18689, 1), -- Phantasmal Cloak
(58, 2, 2, 18718, 1), -- Grand Crusader's Helm
(58, 2, 2, 18754, 1), -- Fel Hardened Bracers
(58, 2, 2, 21753, 1), -- Gem Studded Band
(58, 2, 2, 22336, 1), -- Draconian Aegis of the Legion
(58, 2, 2, 22400, 1), -- Libram of Truth
(58, 3, 0, 11933, 1), -- Imperial Jewel
(58, 3, 0, 13098, 1), -- Painweaver Band
(58, 3, 0, 13340, 1), -- Cape of the Black Baron
(58, 3, 0, 16674, 1), -- Beaststalker's Tunic
(58, 3, 0, 16675, 1), -- Beaststalker's Boots
(58, 3, 0, 16676, 1), -- Beaststalker's Gloves
(58, 3, 0, 16677, 1), -- Beaststalker's Cap
(58, 3, 0, 16678, 1), -- Beaststalker's Pants
(58, 3, 0, 16679, 1), -- Beaststalker's Mantle
(58, 3, 0, 16680, 1), -- Beaststalker's Belt
(58, 3, 0, 16681, 1), -- Beaststalker's Bindings
(58, 3, 0, 18473, 1), -- Royal Seal of Eldre'Thalas
(58, 3, 0, 18680, 1), -- Ancient Bone Bow
(58, 3, 0, 18701, 1), -- Innervating Band
(58, 3, 0, 19991, 1), -- Devilsaur Eye
(58, 3, 0, 22314, 1), -- Huntsman's Harpoon
(58, 4, 0, 11815, 1), -- Hand of Justice
(58, 4, 0, 13340, 1), -- Cape of the Black Baron
(58, 4, 0, 13361, 1), -- Skullforge Reaver
(58, 4, 0, 13953, 1), -- Silent Fang
(58, 4, 0, 16707, 1), -- Shadowcraft Cap
(58, 4, 0, 16708, 1), -- Shadowcraft Spaulders
(58, 4, 0, 16709, 1), -- Shadowcraft Pants
(58, 4, 0, 16710, 1), -- Shadowcraft Bracers
(58, 4, 0, 16711, 1), -- Shadowcraft Boots
(58, 4, 0, 16712, 1), -- Shadowcraft Gloves
(58, 4, 0, 16713, 1), -- Shadowcraft Belt
(58, 4, 0, 16721, 1), -- Shadowcraft Tunic
(58, 4, 0, 18500, 1), -- Tarnished Elven Ring
(58, 4, 0, 18701, 1), -- Innervating Band
(58, 4, 0, 22321, 1), -- Heart of Wyrmthalak
(58, 4, 0, 22340, 1), -- Pendant of Celerity
(58, 4, 0, 28972, 1), -- Flightblade Throwing Axe
(58, 5, 1, 11819, 1), -- Second Wind
(58, 5, 1, 13392, 1), -- The Postmaster's Seal
(58, 5, 1, 16690, 1), -- Devout Robe
(58, 5, 1, 16691, 1), -- Devout Sandals
(58, 5, 1, 16692, 1), -- Devout Gloves
(58, 5, 1, 16693, 1), -- Devout Crown
(58, 5, 1, 16694, 1), -- Devout Skirt
(58, 5, 1, 16695, 1), -- Devout Mantle
(58, 5, 1, 16696, 1), -- Devout Belt
(58, 5, 1, 16697, 1), -- Devout Bracers
(58, 5, 1, 18469, 1), -- Royal Seal of Eldre'Thalas
(58, 5, 1, 19530, 1), -- Caretaker's Cape
(58, 5, 1, 22254, 1), -- Wand of Eternal Light
(58, 5, 1, 22327, 1), -- Amulet of the Redeemed
(58, 5, 1, 22334, 1), -- Band of Mending
(58, 5, 1, 22394, 1), -- Staff of Metanoia
(58, 5, 3, 0, 1),
(58, 5, 3, 11832, 1), -- Burst of Knowledge
(58, 5, 3, 12926, 1), -- Flaming Band
(58, 5, 3, 12930, 1), -- Briarwood Reed
(58, 5, 3, 12968, 1), -- Frostweaver Cape
(58, 5, 3, 13396, 1), -- Skul's Ghastly Touch
(58, 5, 3, 16682, 1), -- Magister's Boots
(58, 5, 3, 16683, 1), -- Magister's Bindings
(58, 5, 3, 16684, 1), -- Magister's Gloves
(58, 5, 3, 16685, 1), -- Magister's Belt
(58, 5, 3, 16686, 1), -- Magister's Crown
(58, 5, 3, 16687, 1), -- Magister's Leggings
(58, 5, 3, 16688, 1), -- Magister's Robes
(58, 5, 3, 16689, 1), -- Magister's Mantle
(58, 5, 3, 18691, 1), -- Dark Advisor's Pendant
(58, 5, 3, 22335, 1), -- Lord Valthalak's Staff of Command
(58, 5, 3, 22339, 1), -- Rune Band of Wizardry
(58, 7, 0, 11815, 1), -- Hand of Justice
(58, 7, 0, 11933, 1), -- Imperial Jewel
(58, 7, 0, 12621, 2), -- Demonfork
(58, 7, 0, 13098, 1), -- Painweaver Band
(58, 7, 0, 13203, 1), -- Armswake Cloak
(58, 7, 0, 13373, 1), -- Band of Flesh
(58, 7, 0, 16674, 1), -- Beaststalker's Tunic
(58, 7, 0, 16675, 1), -- Beaststalker's Boots
(58, 7, 0, 16676, 1), -- Beaststalker's Gloves
(58, 7, 0, 16677, 1), -- Beaststalker's Cap
(58, 7, 0, 16678, 1), -- Beaststalker's Pants
(58, 7, 0, 16679, 1), -- Beaststalker's Mantle
(58, 7, 0, 16680, 1), -- Beaststalker's Belt
(58, 7, 0, 16681, 1), -- Beaststalker's Bindings
(58, 7, 0, 22321, 1), -- Heart of Wyrmthalak
(58, 7, 0, 22395, 1), -- Totem of Rage
(58, 7, 1, 11819, 1), -- Second Wind
(58, 7, 1, 11923, 1), -- The Hammer of Grace
(58, 7, 1, 13178, 1), -- Rosewine Circle
(58, 7, 1, 16666, 1), -- Vest of Elements
(58, 7, 1, 16667, 1), -- Coif of Elements
(58, 7, 1, 16668, 1), -- Kilt of Elements
(58, 7, 1, 16669, 1), -- Pauldrons of Elements
(58, 7, 1, 16670, 1), -- Boots of Elements
(58, 7, 1, 16671, 1), -- Bindings of Elements
(58, 7, 1, 16672, 1), -- Gauntlets of Elements
(58, 7, 1, 16673, 1), -- Cord of Elements
(58, 7, 1, 19530, 1), -- Caretaker's Cape
(58, 7, 1, 22268, 1), -- Draconic Infused Emblem
(58, 7, 1, 22327, 1), -- Amulet of the Redeemed
(58, 7, 1, 22334, 1), -- Band of Mending
(58, 7, 1, 22336, 1), -- Draconian Aegis of the Legion
(58, 7, 1, 22345, 1), -- Totem of Rebirth
(58, 7, 3, 10829, 1), -- Dragon's Eye
(58, 7, 3, 12926, 1), -- Flaming Band
(58, 7, 3, 12930, 1), -- Briarwood Reed
(58, 7, 3, 12967, 1), -- Bloodmoon Cloak
(58, 7, 3, 13964, 1), -- Witchblade
(58, 7, 3, 16666, 1), -- Vest of Elements
(58, 7, 3, 16667, 1), -- Coif of Elements
(58, 7, 3, 16668, 1), -- Kilt of Elements
(58, 7, 3, 16669, 1), -- Pauldrons of Elements
(58, 7, 3, 16670, 1), -- Boots of Elements
(58, 7, 3, 16671, 1), -- Bindings of Elements
(58, 7, 3, 16672, 1), -- Gauntlets of Elements
(58, 7, 3, 16673, 1), -- Cord of Elements
(58, 7, 3, 18471, 1), -- Royal Seal of Eldre'Thalas
(58, 7, 3, 22336, 1), -- Draconian Aegis of the Legion
(58, 7, 3, 22395, 1), -- Totem of Rage
(58, 7, 3, 22433, 1), -- Don Mauricio's Band of Domination
(58, 8, 3, 10829, 1), -- Dragon's Eye
(58, 8, 3, 11832, 1), -- Burst of Knowledge
(58, 8, 3, 12926, 1), -- Flaming Band
(58, 8, 3, 12930, 1), -- Briarwood Reed
(58, 8, 3, 12967, 1), -- Bloodmoon Cloak
(58, 8, 3, 13938, 1), -- Bonecreeper Stylus
(58, 8, 3, 16682, 1), -- Magister's Boots
(58, 8, 3, 16683, 1), -- Magister's Bindings
(58, 8, 3, 16684, 1), -- Magister's Gloves
(58, 8, 3, 16685, 1), -- Magister's Belt
(58, 8, 3, 16686, 1), -- Magister's Crown
(58, 8, 3, 16687, 1), -- Magister's Leggings
(58, 8, 3, 16688, 1), -- Magister's Robes
(58, 8, 3, 16689, 1), -- Magister's Mantle
(58, 8, 3, 22335, 1), -- Lord Valthalak's Staff of Command
(58, 8, 3, 22433, 1), -- Don Mauricio's Band of Domination
(58, 9, 3, 10829, 1), -- Dragon's Eye
(58, 9, 3, 11832, 1), -- Burst of Knowledge
(58, 9, 3, 12926, 1), -- Flaming Band
(58, 9, 3, 12967, 1), -- Bloodmoon Cloak
(58, 9, 3, 13938, 1), -- Bonecreeper Stylus
(58, 9, 3, 16698, 1), -- Dreadmist Mask
(58, 9, 3, 16699, 1), -- Dreadmist Leggings
(58, 9, 3, 16700, 1), -- Dreadmist Robe
(58, 9, 3, 16701, 1), -- Dreadmist Mantle
(58, 9, 3, 16702, 1), -- Dreadmist Belt
(58, 9, 3, 16703, 1), -- Dreadmist Bracers
(58, 9, 3, 16704, 1), -- Dreadmist Sandals
(58, 9, 3, 16705, 1), -- Dreadmist Wraps
(58, 9, 3, 18467, 1), -- Royal Seal of Eldre'Thalas
(58, 9, 3, 22335, 1), -- Lord Valthalak's Staff of Command
(58, 9, 3, 22433, 1), -- Don Mauricio's Band of Domination
(58, 11, 0, 11815, 1), -- Hand of Justice
(58, 11, 0, 13047, 1), -- Twig of the World Tree
(58, 11, 0, 13340, 1), -- Cape of the Black Baron
(58, 11, 0, 16707, 1), -- Shadowcraft Cap
(58, 11, 0, 16708, 1), -- Shadowcraft Spaulders
(58, 11, 0, 16709, 1), -- Shadowcraft Pants
(58, 11, 0, 16710, 1), -- Shadowcraft Bracers
(58, 11, 0, 16711, 1), -- Shadowcraft Boots
(58, 11, 0, 16712, 1), -- Shadowcraft Gloves
(58, 11, 0, 16713, 1), -- Shadowcraft Belt
(58, 11, 0, 16721, 1), -- Shadowcraft Tunic
(58, 11, 0, 18500, 1), -- Tarnished Elven Ring
(58, 11, 0, 18701, 1), -- Innervating Band
(58, 11, 0, 22321, 1), -- Heart of Wyrmthalak
(58, 11, 0, 22340, 1), -- Pendant of Celerity
(58, 11, 0, 22397, 1), -- Idol of Ferocity
(58, 11, 1, 11819, 1), -- Second Wind
(58, 11, 1, 13392, 1), -- The Postmaster's Seal
(58, 11, 1, 16706, 1), -- Wildheart Vest
(58, 11, 1, 16714, 1), -- Wildheart Bracers
(58, 11, 1, 16715, 1), -- Wildheart Boots
(58, 11, 1, 16716, 1), -- Wildheart Belt
(58, 11, 1, 16717, 1), -- Wildheart Gloves
(58, 11, 1, 16718, 1), -- Wildheart Spaulders
(58, 11, 1, 16719, 1), -- Wildheart Kilt
(58, 11, 1, 16720, 1), -- Wildheart Cowl
(58, 11, 1, 18470, 1), -- Royal Seal of Eldre'Thalas
(58, 11, 1, 19530, 1), -- Caretaker's Cape
(58, 11, 1, 22327, 1), -- Amulet of the Redeemed
(58, 11, 1, 22334, 1), -- Band of Mending
(58, 11, 1, 22394, 1), -- Staff of Metanoia
(58, 11, 1, 22398, 1), -- Idol of Rejuvenation
(58, 11, 3, 11832, 1), -- Burst of Knowledge
(58, 11, 3, 12926, 1), -- Flaming Band
(58, 11, 3, 12930, 1), -- Briarwood Reed
(58, 11, 3, 12967, 1), -- Bloodmoon Cloak
(58, 11, 3, 16706, 1), -- Wildheart Vest
(58, 11, 3, 16714, 1), -- Wildheart Bracers
(58, 11, 3, 16715, 1), -- Wildheart Boots
(58, 11, 3, 16716, 1), -- Wildheart Belt
(58, 11, 3, 16717, 1), -- Wildheart Gloves
(58, 11, 3, 16718, 1), -- Wildheart Spaulders
(58, 11, 3, 16719, 1), -- Wildheart Kilt
(58, 11, 3, 16720, 1), -- Wildheart Cowl
(58, 11, 3, 18289, 1), -- Barbed Thorn Necklace
(58, 11, 3, 22335, 1), -- Lord Valthalak's Staff of Command
(58, 11, 3, 22433, 1); -- Don Mauricio's Band of Domination

-- Pack 70
INSERT INTO `boost_pack` (`level`,`class`, `type`, `item`, `count`) VALUES
(70, 1, 0, 28403, 1), -- Doomplate Chestguard
(70, 1, 0, 27497, 1), -- Doomplate Gauntlets
(70, 1, 0, 28225, 1), -- Doomplate Warhelm
(70, 1, 0, 27870, 1), -- Doomplate Legguards
(70, 1, 0, 27771, 2), -- Doomplate Shoulderguards
(70, 1, 0, 27551, 1), -- Skeletal Necklace of Battlerage
(70, 1, 0, 27892, 1), -- Cloak of the Inciter
(70, 1, 0, 34789, 1), -- Bracers of Slaughter
(70, 1, 0, 27985, 1), -- Deathforge Girdle
(70, 1, 0, 28318, 1), -- Obsidian Clodstompers
(70, 1, 0, 27904, 1), -- Resounding Ring of Glory
(70, 1, 0, 27460, 1), -- Reavers' Ring
(70, 1, 0, 28034, 1), -- Hourglass of the Unraveller
(70, 1, 0, 28288, 1), -- Abacus of Violent Odds
(70, 1, 0, 28367, 1), -- Greatsword of Forlorn Visions
(70, 1, 0, 27794, 1), -- Recoilless Rocket Ripper X-54
(70, 1, 0, 34703, 1), -- Latro's Dancing Blade
(70, 1, 0, 28189, 1), -- Latro's Shifting Sword
(70, 1, 0, 28316, 1), -- Aegis of the Sunbird
(70, 1, 2, 28205, 1), -- Breastplate of the Bold
(70, 1, 2, 27475, 1), -- Gauntlets of the Bold
(70, 1, 2, 27977, 1), -- Legplates of the Bold
(70, 1, 2, 27803, 1), -- Shoulderguards of the Bold
(70, 1, 2, 28350, 1), -- Warhelm of the Bold
(70, 1, 2, 27792, 1), -- Steam-Hinge Chain of Valor
(70, 1, 2, 27804, 1), -- Devilshark Cape
(70, 1, 2, 27459, 1), -- Vambraces of Daring
(70, 1, 2, 27672, 1), -- Girdle of the Immovable
(70, 1, 2, 27813, 1), -- Boots of the Colossus
(70, 1, 2, 27822, 1), -- Crystal Band of Valor
(70, 1, 2, 27740, 1), -- Band of Ursol
(70, 1, 2, 27891, 1), -- Adamantine Figurine
(70, 1, 2, 27529, 1), -- Figurine of the Colossus
(70, 1, 2, 27476, 1), -- Truncheon of Five Hells
(70, 1, 2, 28316, 1), -- Aegis of the Sunbird
(70, 1, 2, 32780, 1), -- The Boomstick
(70, 2, 0, 28203, 1), -- Breastplate of the Righteous
(70, 2, 0, 27535, 1), -- Gauntlets of the Righteous
(70, 2, 0, 28285, 1), -- Helm of the Righteous
(70, 2, 0, 27839, 1), -- Legplates of the Righteous
(70, 2, 0, 27739, 1), -- Spaulders of the Righteous
(70, 2, 0, 27792, 1), -- Steam-Hinge Chain of Valor
(70, 2, 0, 27988, 1), -- Burnoose of Shifting Ages
(70, 2, 0, 27459, 1), -- Vambraces of Daring
(70, 2, 0, 27672, 1), -- Girdle of the Immovable
(70, 2, 0, 32778, 1), -- Boots of Righteous Fortitude
(70, 2, 0, 27822, 1), -- Crystal Band of Valor
(70, 2, 0, 27805, 1), -- Ring of the Silver Hand
(70, 2, 0, 28223, 1), -- Arcanist's Stone
(70, 2, 0, 27529, 1), -- Figurine of the Colossus
(70, 2, 0, 27937, 1), -- Sky Breaker
(70, 2, 0, 28316, 1), -- Aegis of the Sunbird
(70, 2, 0, 27917, 1), -- Libram of the Eternal Rest
(70, 2, 1, 27897, 1), -- Breastplate of Many Graces
(70, 2, 1, 27790, 1), -- Mask of Penance
(70, 2, 1, 27748, 1), -- Cassock of the Loyal
(70, 2, 1, 27539, 1), -- Justice Bearer's Pauldrons
(70, 2, 1, 27457, 1), -- Life Bearer's Gauntlets
(70, 2, 1, 28233, 1), -- Necklace of Resplendent Hope
(70, 2, 1, 27448, 1), -- Cloak of the Everliving
(70, 2, 1, 27489, 1), -- Virtue Bearer's Vambraces
(70, 2, 1, 27548, 1), -- Girdle of Many Blessings
(70, 2, 1, 28221, 1), -- Boots of the Watchful Heart
(70, 2, 1, 28259, 1), -- Cosmic Lifeband
(70, 2, 1, 27491, 1), -- Signet of Repose
(70, 2, 1, 27828, 1), -- Warp-Scarab Brooch
(70, 2, 1, 28223, 1), -- Arcanist's Stone
(70, 2, 1, 27538, 1), -- Lightsworn Hammer
(70, 2, 1, 31292, 1), -- Crystal Pulse Shield
(70, 2, 1, 28296, 1), -- Libram of the Lightbringer
(70, 2, 2, 28403, 1), -- Doomplate Chestguard
(70, 2, 2, 27497, 1), -- Doomplate Gauntlets
(70, 2, 2, 28225, 1), -- Doomplate Warhelm
(70, 2, 2, 27870, 1), -- Doomplate Legguards
(70, 2, 2, 27771, 1), -- Doomplate Shoulderguards
(70, 2, 2, 27551, 1), -- Skeletal Necklace of Battlerage
(70, 2, 2, 27892, 1), -- Cloak of the Inciter
(70, 2, 2, 34789, 1), -- Bracers of Slaughter
(70, 2, 2, 27985, 1), -- Deathforge Girdle
(70, 2, 2, 27884, 1), -- Ornate Boots of the Sanctified
(70, 2, 2, 27904, 1), -- Resounding Ring of Glory
(70, 2, 2, 27460, 1), -- Reavers' Ring
(70, 2, 2, 28034, 1), -- Hourglass of the Unraveller
(70, 2, 2, 28288, 1), -- Abacus of Violent Odds
(70, 2, 2, 28393, 1), -- Warmaul of Infused Light
(70, 2, 2, 27484, 1), -- Libram of Avengement
(70, 3, 0, 28228, 1), -- Beast Lord Cuirass
(70, 3, 0, 27474, 1), -- Beast Lord Handguards
(70, 3, 0, 28275, 1), -- Beast Lord Helm
(70, 3, 0, 27874, 1), -- Beast Lord Leggings
(70, 3, 0, 27801, 1), -- Beast Lord Mantle
(70, 3, 0, 27546, 1), -- Traitor's Noose
(70, 3, 0, 27878, 1), -- Auchenai Death Shroud
(70, 3, 0, 34698, 1), -- Bracers of the Forest Stalker
(70, 3, 0, 27541, 1), -- Archery Belt of the Broken
(70, 3, 0, 27915, 1), -- Sky-Hunter Swift Boots
(70, 3, 0, 27925, 1), -- Ravenclaw Band
(70, 3, 0, 27453, 1), -- Averinn's Ring of Slaying
(70, 3, 0, 28034, 1), -- Hourglass of the Unraveller
(70, 3, 0, 28121, 1), -- Icon of Unyielding Courage
(70, 3, 0, 28263, 1), -- Stellaris
(70, 3, 0, 28397, 1), -- Emberhawk Crossbow
(70, 3, 0, 30611, 1), -- Halaani Razorshaft
(70, 4, 0, 28264, 1), -- Wastewalker Tunic
(70, 4, 0, 27531, 1), -- Wastewalker Gloves
(70, 4, 0, 28224, 1), -- Wastewalker Helm
(70, 4, 0, 27837, 1), -- Wastewalker Leggings
(70, 4, 0, 27797, 1), -- Wastewalker Shoulderpads
(70, 4, 0, 27546, 1), -- Traitor's Noose
(70, 4, 0, 27878, 1), -- Auchenai Death Shroud
(70, 4, 0, 27712, 1), -- Shackles of Quagmirran
(70, 4, 0, 27911, 1), -- Epoch's Whispering Cinch
(70, 4, 0, 27867, 1), -- Boots of the Unjust
(70, 4, 0, 27925, 1), -- Ravenclaw Band
(70, 4, 0, 27453, 1), -- Averinn's Ring of Slaying
(70, 4, 0, 28034, 1), -- Hourglass of the Unraveller
(70, 4, 0, 28121, 1), -- Icon of Unyielding Courage
(70, 4, 0, 34703, 1), -- Latro's Dancing Blade
(70, 4, 0, 28189, 1), -- Latro's Shifting Sword
(70, 4, 0, 34783, 1), -- Nightstrike
(70, 5, 1, 28413, 1), -- Hallowed Crown
(70, 5, 1, 28230, 1), -- Hallowed Garments
(70, 5, 1, 27536, 1), -- Hallowed Handwraps
(70, 5, 1, 27775, 1), -- Hallowed Pauldrons
(70, 5, 1, 27875, 1), -- Hallowed Trousers
(70, 5, 1, 28233, 1), -- Necklace of Resplendent Hope
(70, 5, 1, 27448, 1), -- Cloak of the Everliving
(70, 5, 1, 27452, 1), -- Light Scribe Bands
(70, 5, 1, 27542, 1), -- Cord of Belief
(70, 5, 1, 27919, 1), -- Light-Woven Slippers
(70, 5, 1, 27491, 1), -- Signet of Repose
(70, 5, 1, 27996, 1), -- Ring of Spiritual Precision
(70, 5, 1, 28370, 1), -- Bangle of Endless Blessings
(70, 5, 1, 27828, 1), -- Warp-Scarab Brooch
(70, 5, 1, 28216, 1), -- Dathrohan's Ceremonial Hammer
(70, 5, 1, 28213, 1), -- Lordaeron Medical Guide
(70, 5, 3, 28193, 1), -- Mana-Etched Crown
(70, 5, 3, 27465, 1), -- Mana-Etched Gloves
(70, 5, 3, 27907, 1), -- Mana-Etched Pantaloons
(70, 5, 3, 28191, 1), -- Mana-Etched Vestments
(70, 5, 3, 27796, 1), -- Mana-Etched Spaulders
(70, 5, 3, 28134, 1), -- Brooch of Heightened Potential
(70, 5, 3, 34792, 1), -- Cloak of the Betrayed
(70, 5, 3, 27462, 1), -- Crimson Bracers of Gloom
(70, 5, 3, 27795, 1), -- Sash of Serpentra
(70, 5, 3, 27451, 1), -- Boots of the Darkwalker
(70, 5, 3, 27784, 1), -- Scintillating Coral Band
(70, 5, 3, 28227, 1), -- Sparking Arcanite Ring
(70, 5, 3, 28223, 1), -- Arcanist's Stone
(70, 5, 3, 27683, 1), -- Quagmirran's Eye
(70, 5, 3, 27937, 1), -- Sky Breaker
(70, 5, 3, 28187, 1), -- Star-Heart Lamp
(70, 7, 0, 28231, 1), -- Tidefury Chestpiece
(70, 7, 0, 27510, 1), -- Tidefury Gauntlets
(70, 7, 0, 28349, 2), -- Tidefury Helm
(70, 7, 0, 27909, 1), -- Tidefury Kilt
(70, 7, 0, 27802, 1), -- Tidefury Shoulderguards
(70, 7, 0, 28134, 1), -- Brooch of Heightened Potential
(70, 7, 0, 28269, 1), -- Baba's Cloak of Arcanistry
(70, 7, 0, 27522, 1), -- World's End Bracers
(70, 7, 0, 27743, 1), -- Girdle of Living Flame
(70, 7, 0, 27845, 1), -- Magma Plume Boots
(70, 7, 0, 32779, 1), -- Band of Frigid Elements
(70, 7, 0, 28227, 1), -- Sparking Arcanite Ring
(70, 7, 0, 28223, 1), -- Arcanist's Stone
(70, 7, 0, 28418, 1), -- Shiffar's Nexus-Horn
(70, 7, 0, 27741, 1), -- Bleeding Hollow Warhammer
(70, 7, 0, 27910, 1), -- Silvermoon Crest Shield
(70, 7, 0, 28248, 1), -- Totem of the Void
(70, 7, 1, 27936, 1), -- Greaves of Desolation
(70, 7, 1, 28401, 1), -- Hauberk of Desolation
(70, 7, 1, 27528, 1), -- Gauntlets of Desolation
(70, 7, 1, 28192, 1), -- Helm of Desolation
(70, 7, 1, 27713, 1), -- Pauldrons of Desolation
(70, 7, 1, 27495, 1), -- Soldier's Dog Tags
(70, 7, 1, 27878, 1), -- Auchenai Death Shroud
(70, 7, 1, 34698, 1), -- Bracers of the Forest Stalker
(70, 7, 1, 27541, 1), -- Archery Belt of the Broken
(70, 7, 1, 28384, 1), -- Outland Striders
(70, 7, 1, 27740, 1), -- Band of Ursol
(70, 7, 1, 27904, 1), -- Resounding Ring of Glory
(70, 7, 1, 28034, 1), -- Hourglass of the Unraveller
(70, 7, 1, 28288, 1), -- Abacus of Violent Odds
(70, 7, 1, 27846, 1), -- Claw of the Watcher
(70, 7, 1, 28210, 1), -- Bloodskull Destroyer
(70, 7, 1, 27815, 1), -- Totem of the Astral Winds
(70, 7, 3, 27826, 1), -- Mantle of the Sea Wolf
(70, 7, 3, 27806, 1), -- Fathomheart Gauntlets
(70, 7, 3, 27912, 1), -- Harness of the Deep Currents
(70, 7, 3, 27458, 1), -- Oceansong Kilt
(70, 7, 3, 27759, 1), -- Headdress of the Tides
(70, 7, 3, 28233, 1), -- Necklace of Resplendent Hope
(70, 7, 3, 27448, 1), -- Cloak of the Everliving
(70, 7, 3, 28194, 1), -- Primal Surge Bracers
(70, 7, 3, 27835, 1), -- Stillwater Girdle
(70, 7, 3, 27549, 1), -- Wavefury Boots
(70, 7, 3, 28259, 1), -- Cosmic Lifeband
(70, 7, 3, 27491, 1), -- Signet of Repose
(70, 7, 3, 27828, 1), -- Warp-Scarab Brooch
(70, 7, 3, 28223, 1), -- Arcanist's Stone
(70, 7, 3, 27538, 1), -- Lightsworn Hammer
(70, 7, 3, 27714, 1), -- Swamplight Lantern
(70, 7, 3, 27544, 1), -- Totem of Spontaneous Regrowth
(70, 8, 3, 28278, 1), -- Incanter's Cowl
(70, 8, 3, 27508, 1), -- Incanter's Gloves
(70, 8, 3, 27738, 1), -- Incanter's Pauldrons
(70, 8, 3, 28229, 1), -- Incanter's Robe
(70, 8, 3, 27838, 1), -- Incanter's Trousers
(70, 8, 3, 28134, 1), -- Brooch of Heightened Potential
(70, 8, 3, 34792, 1), -- Cloak of the Betrayed
(70, 8, 3, 27462, 1), -- Crimson Bracers of Gloom
(70, 8, 3, 27795, 1), -- Sash of Serpentra
(70, 8, 3, 27902, 1), -- Silent Slippers of Meditation
(70, 8, 3, 27784, 1), -- Scintillating Coral Band
(70, 8, 3, 28227, 1), -- Sparking Arcanite Ring
(70, 8, 3, 28223, 1), -- Arcanist's Stone
(70, 8, 3, 28418, 1), -- Shiffar's Nexus-Horn
(70, 8, 3, 27905, 1), -- Greatsword of Horrid Dreams
(70, 8, 3, 28187, 1), -- Star-Heart Lamp
(70, 9, 3, 27537, 1), -- Gloves of Oblivion
(70, 9, 3, 28415, 1), -- Hood of Oblivion
(70, 9, 3, 28232, 1), -- Robe of Oblivion
(70, 9, 3, 27778, 1), -- Spaulders of Oblivion
(70, 9, 3, 27948, 1), -- Trousers of Oblivion
(70, 9, 3, 28134, 1), -- Brooch of Heightened Potential
(70, 9, 3, 34792, 1), -- Cloak of the Betrayed
(70, 9, 3, 27462, 1), -- Crimson Bracers of Gloom
(70, 9, 3, 27795, 1), -- Sash of Serpentra
(70, 9, 3, 27902, 1), -- Silent Slippers of Meditation
(70, 9, 3, 27784, 1), -- Scintillating Coral Band
(70, 9, 3, 28227, 1), -- Sparking Arcanite Ring
(70, 9, 3, 28223, 1), -- Arcanist's Stone
(70, 9, 3, 27683, 1), -- Quagmirran's Eye
(70, 9, 3, 27543, 1), -- Starlight Dagger
(70, 9, 3, 28187, 1), -- Star-Heart Lamp
(70, 11, 0, 28348, 1), -- Moonglade Cowl
(70, 11, 0, 28268, 1), -- Natural Mender's Wraps
(70, 11, 0, 34796, 1), -- Robes of Summer Flame
(70, 11, 0, 27800, 1), -- Earthsoul Britches
(70, 11, 0, 27737, 1), -- Moonglade Shoulders
(70, 11, 0, 28233, 1), -- Necklace of Resplendent Hope
(70, 11, 0, 27448, 1), -- Cloak of the Everliving
(70, 11, 0, 27827, 1), -- Lucid Dream Bracers
(70, 11, 0, 28398, 1), -- The Sleeper's Cord
(70, 11, 0, 28251, 1), -- Boots of the Glade-Keeper
(70, 11, 0, 27996, 1), -- Ring of Spiritual Precision
(70, 11, 0, 27491, 1), -- Signet of Repose
(70, 11, 0, 28370, 1), -- Bangle of Endless Blessings
(70, 11, 0, 27828, 1), -- Warp-Scarab Brooch
(70, 11, 0, 27791, 1), -- Serpentcrest Life-Staff
(70, 11, 0, 27886, 1), -- Idol of the Emerald Queen
(70, 11, 1, 27763, 1), -- Crown of the Forest Lord
(70, 11, 1, 27818, 1), -- Starry Robes of the Crescent
(70, 11, 1, 27492, 1), -- Moonchild Leggings
(70, 11, 1, 28255, 1), -- Lunar-Claw Pauldrons
(70, 11, 1, 28214, 1), -- Grips of the Lunar Eclipse
(70, 11, 1, 28134, 1), -- Brooch of Heightened Potential
(70, 11, 1, 28269, 1), -- Baba's Cloak of Arcanistry
(70, 11, 1, 27483, 1), -- Moon-Touched Bands
(70, 11, 1, 27783, 1), -- Moonrage Girdle
(70, 11, 1, 27914, 1), -- Moonstrider Boots
(70, 11, 1, 27784, 1), -- Scintillating Coral Band
(70, 11, 1, 32779, 1), -- Band of Frigid Elements
(70, 11, 1, 28223, 1), -- Arcanist's Stone
(70, 11, 1, 28418, 1), -- Shiffar's Nexus-Horn
(70, 11, 1, 27842, 1), -- Grand Scepter of the Nexus-Kings
(70, 11, 1, 27518, 1), -- Ivory Idol of the Moongoddess
(70, 11, 3, 28264, 1), -- Wastewalker Tunic
(70, 11, 3, 27531, 1), -- Wastewalker Gloves
(70, 11, 3, 28224, 1), -- Wastewalker Helm
(70, 11, 3, 27837, 1), -- Wastewalker Leggings
(70, 11, 3, 27797, 1), -- Wastewalker Shoulderpads
(70, 11, 3, 30710, 1), -- Blood Guard's Necklace of Ferocity
(70, 11, 3, 27892, 1), -- Cloak of the Inciter
(70, 11, 3, 27712, 1), -- Shackles of Quagmirran
(70, 11, 3, 32769, 1), -- Belt of the Raven Lord
(70, 11, 3, 27467, 1), -- Silent-Strider Kneeboots
(70, 11, 3, 27740, 1), -- Band of Ursol
(70, 11, 3, 27925, 1), -- Ravenclaw Band
(70, 11, 3, 28034, 1), -- Hourglass of the Unraveller
(70, 11, 3, 28288, 1), -- Abacus of Violent Odds
(70, 11, 3, 28372, 1), -- Idol of Feral Shadows
(70, 11, 3, 27877, 1); -- Draenic Wildstaff
