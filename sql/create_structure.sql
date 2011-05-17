DROP TABLE IF EXISTS `dmoz_structure`;

CREATE TABLE `dmoz_structure` (
	`Id` int(10) unsigned NOT NULL,
	`topicId` int(11) UNSIGNED NOT NULL,
	`parentId` int(11) UNSIGNED NULL,
	`topic` text collate latin1_german2_ci NOT NULL,
	`title` varchar(255) collate latin1_german2_ci NOT NULL default '',
	`lastUpdate` varchar(64) collate latin1_german2_ci default NULL,
	`level` varchar(32) default NULL,
	PRIMARY KEY  (`Id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_german2_ci PACK_KEYS=0 ;
