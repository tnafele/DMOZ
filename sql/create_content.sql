DROP TABLE IF EXISTS `dmoz_content`;

CREATE TABLE `dmoz_content` (
	`Id` int(10) unsigned NOT NULL,
	`parentId` int(11) NOT NULL default '0',
	`link` text NOT NULL,
	`title` varbinary(255) NOT NULL default '',
	`description` blob,
	PRIMARY KEY  (`Id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci PACK_KEYS=0 ;
