/* config.h
 * DMOZ config-file
 * tw, 4.07
 */

#ifndef CONFIG_H
#define CONFIG_H

#define MYSQL_HOST	"localhost"
#define MYSQL_USR	"dmozdb"
#define MYSQL_PWD	"piffpaff"
#define MYSQL_DB	"dmoz"

#define TABLE_STRUCTURE	"dmoz_structure"
#define TABLE_CONTENT	"dmoz_content"

#define BASETOPIC	"Top/World/Deutsch/Computer"

#define DATABASE

#ifdef XML_LARGE_SIZE
#if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#define XML_FMT_INT_MOD "I64"
#else
#define XML_FMT_INT_MOD "ll"
#endif
#else
#define XML_FMT_INT_MOD "l"
#endif

#define DEBUG 0

#endif
