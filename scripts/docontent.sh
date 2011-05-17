#!/bin/sh

URL=http://rdf.dmoz.org/rdf/content.rdf.u8.gz
SAVEBIN=./src/savecontent

MYSQL_ADMIN=root
MYSQL_DB=dmoz
MYSQL_TMPL=./sql/create_content.sql

mysql -u$MYSQL_ADMIN -p dmoz < $MYSQL_TMPL

wget -O - $URL | gunzip -c | $SAVEBIN
