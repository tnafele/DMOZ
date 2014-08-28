DROP TABLE IF EXISTS `dmoz_structure`;

CREATE TABLE `dmoz_structure` (
	`Id` int(10) unsigned NOT NULL,
	`topicId` int(11) UNSIGNED NOT NULL,
	`parentId` int(11) UNSIGNED NULL,
	`topic` text NOT NULL,
	`title` varchar(255) NOT NULL default '',
	`lastUpdate` varchar(64) default NULL,
	`level` varchar(32) default NULL,
	PRIMARY KEY  (`Id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci PACK_KEYS=0 ;
