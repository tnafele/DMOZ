/* This is simple demonstration of how to use expat. This program
   reads an XML document from standard input and writes a line with
   the name of each element to standard output indenting child
   elements by one tab stop more than their parent element.
   It must be used with Expat compiled for UTF-8 output.
*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <expat.h>
#include <mysql/mysql.h>
#include "config.h"

typedef struct _UserData {
	int Topic;
	int topic;
	int title;
	int catid;
	int link;
	int external;
	int description;
	int char_len;
	char *char_tmp;
	char *title_is;
	char *catid_is;
	char *about_is;
	char *topic_is;
	char *description_is;
} UserData;

MYSQL *mysql;
MYSQL_STMT *stmt_links, *stmt_descr;

char *getChar_tmp(UserData *pData);
void setChar_tmp(UserData *pData, const char *s, int len);
void saveLink(const char *catid, const char *link);
void saveDescription(const char *link, const char *title, const char *description);
void initDB(void);
void tryFree(void *pointer);
void *mmalloc(int len);

unsigned long counter = 1;

void zeroUserData(UserData *data) {
	memset(data, 0, sizeof(UserData));
}

static void XMLCALL
startElement(void *userData, const char *name, const char **attr)
{
	int i;
	UserData *pData = (UserData *)userData;

	if (DEBUG) puts("** startElement() - start **");
	
	//LINKS:
	if ( 0 == strcmp("Topic", name)
		&& attr[0] && 0 == strcmp("r:id", attr[0])
		&& attr[1] && strlen(attr[1]) >= strlen(BASETOPIC)
		&& 0 == strncmp(attr[1], BASETOPIC, strlen(BASETOPIC)) )
	{
		zeroUserData(pData);
		if (DEBUG) puts("startElement Topic");
		pData->Topic=1;
	}

	if ( pData->Topic ) {

		if ( 0 == strcmp("catid", name) ) {
			if (DEBUG) puts("startElement catid");
			pData->catid=1;
			pData->char_len=0;
		}


		if ( 0 == strcmp("link", name)
			&& attr[0] && 0 == strcmp("r:resource", attr[0])
			&& attr[1] )
		{
			pData->link=1;
			if (DEBUG) puts("startElement link");
			saveLink(pData->catid_is, (char *)attr[1]);
		}
        }


	//CONTENT:
	if ( 0 == strcmp("ExternalPage", name)
		&& attr[0] && 0 == strcmp("about", attr[0])
		&& attr[1])
	{
		zeroUserData(pData);	
		if (DEBUG) puts("startElement External Page");
		pData->external=1;

                char *buf = mmalloc(strlen(attr[1])+1);
                strcpy(buf, attr[1]);
                pData->about_is=buf;
	}

	if ( pData->external ) {

		if ( 0 == strcmp("d:Description", name) ) {
			if (DEBUG) puts("startElement d:Description");
			pData->description=1;
			pData->char_len=0;
		}

		if ( 0 == strcmp("d:Title", name) ) {
			if (DEBUG) puts("startElement d:Title");
			pData->title=1;
			pData->char_len=0;
		}

		if ( 0 == strcmp("topic", name) ) {
			if (DEBUG) puts("startElement topic");
			pData->topic=1;
			pData->char_len=0;
		}
	}
	if (DEBUG) puts("** startElement() - end **");
}

static void XMLCALL
endElement(void *userData, const char *name)
{
	int error=0;
	UserData *pData = (UserData *)userData;

	if (DEBUG) puts("** endElement() - start **");
	if ( pData->Topic ) {
		if ( 0 == strcmp("Topic", name) ) {
			if (DEBUG) puts("endElement Topic");
			tryFree(pData->catid_is);
			zeroUserData(pData);
		}

		if ( 0 == strcmp("link", name) ) {
			if (DEBUG) puts("endElement link");
			pData->link=0;
		}

		if ( 0 == strcmp("catid", name) ) {
			pData->catid=0;
			pData->catid_is = getChar_tmp(pData);
			if (DEBUG) printf("catid = '%s'\n", pData->catid_is);
		}
	}

	if ( pData->external ) {
		if ( 0 == strcmp("ExternalPage", name)) {
			if (DEBUG) puts("endElement ExternalPage");

			if ( 0 == strncmp(pData->topic_is, BASETOPIC, strlen(BASETOPIC)) ) {
				if (pData->about_is == NULL) {
					fprintf(stderr,"endElement(%s): about = NULL!\n", name);
					error = 1;
				}

				if (pData->title_is == NULL) {
					fprintf(stderr,"endElement(%s): title = NULL!\n", name);
					error = 1;
				}

				if (pData->description_is== NULL) {
					char *nulstr = mmalloc(sizeof(char));
					nulstr[0] = '\0';
					pData->description_is = nulstr;
				}
				if (! error )
					saveDescription(pData->about_is, pData->title_is, pData->description_is);
				else
					perror(name);				
			}

			pData->external = 0;
			tryFree(pData->description_is);
			tryFree(pData->about_is);
			tryFree(pData->title_is);
			tryFree(pData->topic_is);
			zeroUserData(pData);
		}

		if ( 0 == strcmp("topic", name)) {
			if (DEBUG) puts("endElement topic");
			pData->topic=0;
			pData->topic_is = getChar_tmp(pData);
			if (DEBUG) printf("topic = '%s'\n", pData->topic_is);
		}

		if ( 0 == strcmp("d:Title", name)) {
			if (DEBUG) puts("endElement d:Title");
			pData->title=0;
			pData->title_is = getChar_tmp(pData);
			if (DEBUG) printf("title = '%s'\n", pData->title_is);
		}

		if ( 0 == strcmp("d:Description", name)) {
			if (DEBUG) puts("endElement d:Description");
			pData->description=0;
			pData->description_is = getChar_tmp(pData);
			if (DEBUG) printf("description = '%s'\n", pData->description_is);
		}

	}
	if (DEBUG) puts("** endElement() - end **");
}

static void XMLCALL
charData(void *userData, const XML_Char *s, int len)
{
	if (DEBUG) puts("** charData() - start **"); 
	UserData *pData = (UserData *)userData;

	if ( (pData->Topic || pData->external) &&
		( pData->topic || pData->title || pData->description || pData->catid )
	) {
		setChar_tmp(pData, s, len);
	}
	
	if (DEBUG) puts("** charData() - end **");
}

void setChar_tmp(UserData *pData, const char *s, int len) {
	if (!len)
		return; 

	if (DEBUG) puts("** setChar_tmp() - start **");
	char *buf;
	int len_old = pData->char_len;
	int len_new = len_old + len;

	if (pData->char_len == 0 && pData->char_tmp) {
		//puts("WHY???????????????!!!!!!!!!!!!!!!!!!!!!!");
		pData->char_tmp = NULL;
	}

	buf = mmalloc(sizeof(char)*(len_new+1));

	if (pData->char_tmp && len_old > 0 ) {
		if (DEBUG) puts("char adding part!!");
		strncpy(buf, pData->char_tmp, len_old);
	}

	strncpy(&buf[len_old], s, len);

	buf[len_new] = '\0';
	tryFree(pData->char_tmp);
	pData->char_tmp = buf;
	pData->char_len = len_new;

	if (DEBUG) puts("** setChar_tmp() - end **");
}

char *getChar_tmp(UserData *pData) {
	if (pData->char_len == 0 || pData->char_tmp == NULL) {
		if (DEBUG) puts("---- No char!!----");
		return NULL;
	}

	if (DEBUG) puts("** getChar_tmp() - start **");
	char *buf;
	int len = pData->char_len;
	buf = mmalloc(sizeof(char)*(len+1));
	strncpy(buf, pData->char_tmp, len);
	buf[len] = '\0';

	tryFree(pData->char_tmp);
	pData->char_len=0;

	if (DEBUG) puts("** getChar_tmp() - end **");
	return buf;
}

void tryFree(void *ptr) {
	if (ptr) {
		if (DEBUG) printf("Free   0x%x\n", ptr);
		free(ptr);
		ptr=NULL;
	}
}

void *mmalloc(int len) {
	void *p = malloc(len);
	if ( !p) {
		perror("Malloc");
		exit(2);
	}
		
	if (DEBUG) printf("Malloc 0x%x\n", p);
	return p;
}

void saveLink(const char *catid, const char *link) {

	printf("INSERT INTO " TABLE_CONTENT " SET Id='%lu', parentId='%s', link='%s'\n", counter, catid, link);

#ifdef DATABASE
	MYSQL_BIND bind[3];
	memset(bind, 0, sizeof(bind));
	
	unsigned long linklen = strlen(link);
	unsigned int id = atoi(catid);

	bind[0].buffer_type = MYSQL_TYPE_LONG;
	bind[0].buffer = &counter;
	bind[0].length = NULL;
	bind[0].is_null = (my_bool*) 0;
	bind[0].is_unsigned = 1;
	
	bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &id;
        bind[1].length = NULL;
        bind[1].is_null = (my_bool*) 0;
        bind[1].is_unsigned = 1;

	bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = (char *)link;
        bind[2].length = &linklen;
        bind[2].is_null = (my_bool*) 0;

        if (mysql_stmt_bind_param(stmt_links, bind)) {
                fprintf(stderr, "%s\n", mysql_error(mysql));
        }

          /* Execute the INSERT statement - 1*/
        if (mysql_stmt_execute(stmt_links)) {
                fprintf(stderr, " mysql_stmt_execute(), 1 failed\n");
                fprintf(stderr, " %s\n", mysql_stmt_error(stmt_links));
        }
	counter++;
#endif
}

void saveDescription(const char *link, const char *title, const char *description) {

	printf("UPDATE " TABLE_CONTENT " SET title='%s', description='%s' WHERE link LIKE '%s'\n", title, description, link);

#ifdef DATABASE
	MYSQL_BIND bind[3];
	memset(bind, 0, sizeof(bind));

	unsigned long linklen = strlen(link);
	unsigned long titlelen = strlen(title);
	unsigned long descrlen = strlen(description);

	bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[0].buffer = (char *)title;
	bind[0].length = &titlelen;
	bind[0].is_null = (my_bool*) 0;

	bind[1].buffer_type = MYSQL_TYPE_BLOB;
	bind[1].buffer = (char *)description;
	bind[1].length = &descrlen;
	bind[1].is_null = (my_bool*) 0;
	
	bind[2].buffer_type = MYSQL_TYPE_STRING;
	bind[2].buffer = (char *)link;
	bind[2].length = &linklen;
	bind[2].is_null = (my_bool*) 0;

	if (mysql_stmt_bind_param(stmt_descr, bind)) {
		fprintf(stderr, "%s\n", mysql_error(mysql));
	}

	if (mysql_stmt_execute(stmt_descr)) {
		fprintf(stderr, " mysql_stmt_execute(), 1 failed\n");
		fprintf(stderr, " %s\n", mysql_stmt_error(stmt_descr));
	}


#endif

}

void initDB() {

	char stat_links[] = "INSERT INTO " TABLE_CONTENT " SET Id=?, parentId=?, link=?";
	char stat_descr[] = "UPDATE " TABLE_CONTENT " SET title=?, description=? WHERE link LIKE ?";
   
	mysql = mysql_init(NULL);
   
	/* Connect to database */
	if (!mysql_real_connect(mysql, MYSQL_HOST, MYSQL_USR, MYSQL_PWD, MYSQL_DB, 0, NULL, 0)) {
		fprintf(stderr, "%s\n", mysql_error(mysql));
		mysql = NULL;
	}

	if ( NULL == (stmt_links =  mysql_stmt_init(mysql)) ) {
                perror("Out of memory!");
        }

	if ( NULL == (stmt_descr =  mysql_stmt_init(mysql)) ) {
		perror("Out of memory!");
	}

	if ( mysql_stmt_prepare(stmt_links, stat_links, strlen(stat_links)) ) {
		fprintf(stderr, "%s\n", mysql_error(mysql));
	}

	if ( mysql_stmt_prepare(stmt_descr, stat_descr, strlen(stat_descr)) ) {
		fprintf(stderr, "%s\n", mysql_error(mysql));
	}
}

void finishDB() {
	if (mysql_stmt_close(stmt_links)) {
		fprintf(stderr, " failed while closing the statement\n");
		fprintf(stderr, " %s\n", mysql_stmt_error(stmt_links));
	}

	if (mysql_stmt_close(stmt_descr)) {
		fprintf(stderr, " failed while closing the statement\n");
		fprintf(stderr, " %s\n", mysql_stmt_error(stmt_descr));
	}

	mysql_close(mysql);
}


int
main(int argc, char *argv[])
{
	char buf[BUFSIZ];

	UserData userData;
	zeroUserData(&userData);

	XML_Parser parser = XML_ParserCreate(NULL);
	int done;

#ifdef DATABASE  
 	initDB();

	if (mysql == NULL || stmt_links == NULL || stmt_descr == NULL)
		exit(2);
#endif

	FILE *fh=NULL;
	if ( argc > 1 ) 
		fh = fopen(argv[1], "r");

  XML_SetUserData(parser, &userData);
  XML_SetElementHandler(parser, startElement, endElement);
  XML_SetCharacterDataHandler(parser, charData);
  do {
	size_t len;
   	if ( fh )
		len = fread(buf, 1, sizeof(buf), fh);
	else
	    	len = fread(buf, 1, sizeof(buf), stdin);

    done = len < sizeof(buf);
    if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) {
      fprintf(stderr,
              "%s at line %" XML_FMT_INT_MOD "u\n",
              XML_ErrorString(XML_GetErrorCode(parser)),
              XML_GetCurrentLineNumber(parser));
      return 1;
	mysql_close(mysql);
    }
  } while (!done);
	XML_ParserFree(parser);

#ifdef DATABASE
	finishDB();
#endif
	return 0;
}
