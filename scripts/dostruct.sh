#!/bin/sh

URL=http://rdf.dmoz.org/rdf/structure.rdf.u8.gz
SAVEBIN=./src/savestruct

MYSQL_ADMIN=root
MYSQL_DB=dmoz
MYSQL_TMPL=./sql/create_structure.sql

mysql -u$MYSQL_ADMIN -p dmoz < $MYSQL_TMPL

wget -O - $URL | gunzip -c | $SAVEBIN
