/* This is simple demonstration of how to use expat. This program
   reads an XML document from standard input and writes a line with
   the name of each element to standard output indenting child
   elements by one tab stop more than their parent element.
   It must be used with Expat compiled for UTF-8 output.
*/

#include <stdio.h>
#include <string.h>
#include <expat.h>
#include <mysql/mysql.h>
#include "config.h"

typedef struct _UserData {
	int topic;
	int title;
	int catid;
	int narrow;
	int update;
	int char_len;
	char *topic_is;
	char *title_is;
	char *catid_is;
	char *update_is;
	char *char_tmp;
} UserData;

MYSQL *mysql;
MYSQL_STMT *stmt_parent, *stmt_child;
long unsigned int counter = 1;

void saveParent(const char *catid, const char *topic, const char *title, const char *update);
void saveChild(const char *catid, const char *topic, const char *level);
void initDB(void);
void tryFree(void *pointer);
void *mmalloc(int len);
char *getChar_tmp(UserData *pData);
void setChar_tmp(UserData *pData, const char *s, int len);
void zeroUserData(UserData *data);

static void XMLCALL
startElement(void *userData, const char *name, const char **attr)
{
	if (DEBUG) puts("** startElement() - start **");
	int i;
	UserData *pData = (UserData *)userData;

	if ( 0 == strcmp("Topic", name)
		&& attr[0] && 0 == strcmp("r:id", attr[0])
		&& attr[1] && 0 == strncmp(attr[1], BASETOPIC, strlen(BASETOPIC)) )
	{
		pData->topic=1;
		if (DEBUG) puts("startElement: topic");
		char *buf = mmalloc(strlen(attr[1])+1);
		strcpy(buf, attr[1]);
		pData->topic_is=buf;
	}

	if ( pData->topic ) {
		if ( 0 == strcmp("d:Title", name) ) {
			pData->title=1;
			if (DEBUG) puts("startElement: title");
		}

		if ( 0 == strcmp("catid", name) ) {
			pData->catid=1;
			if (DEBUG) puts("startElement: catid");
		}

		if ( 0 == strncmp("narrow", name, strlen("narrow"))
			&& attr[0] && 0 == strcmp("r:resource", attr[0]) 
			&& attr[1] 
		) {
			pData->narrow=1;
			if (DEBUG) puts("startElement: narrow");
			saveChild(pData->catid_is, (char *)attr[1], name);
		}

		if ( 0 == strcmp("lastUpdate", name) ) {
			pData->update=1;
			if (DEBUG) puts("startElement: update");
		}

	}
	if (DEBUG) puts("** startElement() - end **");
}

static void XMLCALL
endElement(void *userData, const char *name)
{
	int error=0;
	if (DEBUG) puts("** endElement() - start **");
	UserData *pData = (UserData *)userData;

	if ( pData->topic ) {
		if ( 0 == strcmp("Topic", name) ) {
			if (DEBUG) puts("endElement: topic");
			if (pData->catid_is == NULL) {
				fprintf(stderr,"endElement(%s): catid = NULL!\n", name);
				error = 1;
			}

                        if (pData->title_is == NULL) {
				fprintf(stderr,"endElement(%s): title = NULL!\n", name);
				error = 1;
			}
		
                        if (pData->update_is == NULL) {
				char *nulstr = mmalloc(sizeof(char));
				nulstr[0] = '\0';
				pData->update_is = nulstr;
			}
			
			if (! error )
				saveParent(pData->catid_is, pData->topic_is, pData->title_is, pData->update_is);
			else
				perror(name);
			
			pData->topic=0;
			tryFree(pData->topic_is);
			tryFree(pData->title_is);
			tryFree(pData->catid_is);
			tryFree(pData->update_is);
			zeroUserData(pData);
		}

		if ( 0 == strcmp("d:Title", name)) {
			pData->title=0;
			if (DEBUG) puts("endElement: title");
			pData->title_is=getChar_tmp(pData);
		}

		if ( 0 == strcmp("catid", name)) {
			pData->catid=0;
			if (DEBUG) puts("endElement: catid");
			pData->catid_is=getChar_tmp(pData);
		}

		if ( 0 == strncmp("narrow", name, strlen("narrow")) ) {
			pData->narrow=0;
			if (DEBUG) puts("endElement: narrow");
		}

		if ( 0 == strcmp("lastUpdate", name) ) {
			pData->update=0;
			if (DEBUG) puts("endElement: update");
			pData->update_is=getChar_tmp(pData);
		}
	}
	if (DEBUG) puts("** endElement() - end **");
}

static void XMLCALL
charData(void *userData, const XML_Char *s, int len)
{
        if (DEBUG) puts("** charData() - start **"); 
        UserData *pData = (UserData *)userData;

        if ( ( pData->topic ) &&
                ( pData->title || pData->catid || pData->update )
        ) {
                setChar_tmp(pData, s, len);
        }
        
        if (DEBUG) puts("** charData() - end **");

/*
	char *buf;
	UserData *pData = (UserData *)userData;

	if ( pData->topic ) {
		buf = malloc(sizeof(char)*(len+1));
		strncpy(buf, s, len);
		buf[len] = '\0';

		if ( pData->title )
			pData->title_is = buf;

		if ( pData->catid )
			pData->catid_is = buf;

	}
*/
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

void zeroUserData(UserData *data) {
        memset(data, 0, sizeof(UserData));
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

void tryFree(void *ptr) {
        if (ptr) {
                if (DEBUG) printf("Free   0x%x\n", ptr);
                free(ptr);
                ptr=NULL;
        }
}

void saveParent(const char *catid, const char *topic, const char *title, const char *update) {

	printf("UPDATE " TABLE_STRUCTURE " SET topicId='%s', title='%s', lastUpdate='%s' WHERE topic LIKE '%s'\n", catid, title, update, topic);

#ifdef DATABASE
	MYSQL_BIND bind[4];
	memset(bind, 0, sizeof(bind));

	unsigned int id = atoi(catid);
	unsigned long titlelen = strlen(title);
	unsigned long updatelen = strlen(update);
	unsigned long topiclen = strlen(topic);

	bind[0].buffer_type = MYSQL_TYPE_LONG;
	bind[0].buffer = &id;
	bind[0].length = NULL;
	bind[0].is_null = (my_bool*) 0;
	bind[0].is_unsigned = 1;

	bind[1].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[1].buffer = (char *)title;
	bind[1].length = &titlelen;
	bind[1].is_null = (my_bool*) 0;

	bind[2].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[2].buffer = (char *)update;
	bind[2].length = &updatelen;
	bind[2].is_null = (my_bool*) 0;
	
	bind[3].buffer_type = MYSQL_TYPE_STRING;
	bind[3].buffer = (char *)topic;
	bind[3].length = &topiclen;
	bind[3].is_null = (my_bool*) 0;
	
	if (mysql_stmt_bind_param(stmt_parent, bind)) {
		fprintf(stderr, "%s\n", mysql_error(mysql));
	}

	if (mysql_stmt_execute(stmt_parent)) {
		fprintf(stderr, " mysql_stmt_execute(), 1 failed\n");
		fprintf(stderr, " %s\n", mysql_stmt_error(stmt_parent));
	}

#endif
}

void saveChild(const char *catid_str, const char *topic, const char *level) {
	
	printf("INSERT INTO " TABLE_STRUCTURE " SET Id='%u', parentId='%s', level='%s', topic='%s'\n", counter, catid_str, level, topic);

#ifdef DATABASE
	MYSQL_BIND bind[4];
	memset(bind, 0, sizeof(bind));
	
	unsigned int parentId = atoi(catid_str);
	unsigned long levellen = strlen(level);
	unsigned long topiclen = strlen(topic);
	
	bind[0].buffer_type = MYSQL_TYPE_LONG;
	bind[0].buffer = &counter;
	bind[0].length = NULL;
	bind[0].is_null = (my_bool*) 0;
	bind[0].is_unsigned = 1;

	bind[1].buffer_type = MYSQL_TYPE_LONG;
	bind[1].buffer = &parentId;
	bind[1].length = NULL;
	bind[1].is_null = (my_bool*) 0;
	bind[1].is_unsigned = 1;

	bind[2].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[2].buffer = (char *)level;
	bind[2].length = &levellen;
	bind[2].is_null = (my_bool*) 0;

	bind[3].buffer_type = MYSQL_TYPE_STRING;
	bind[3].buffer = (char *)topic;
	bind[3].length = &topiclen;
	bind[3].is_null = (my_bool*) 0;

	if (mysql_stmt_bind_param(stmt_child, bind)) {
		fprintf(stderr, "%s\n", mysql_error(mysql));
	}
	
	if (mysql_stmt_execute(stmt_child)) {
		fprintf(stderr, " mysql_stmt_execute(), 1 failed\n");
		fprintf(stderr, " %s\n", mysql_stmt_error(stmt_child));
	}

	counter++;
#endif
}

void initDB() {

	char stat_parent[] = "UPDATE " TABLE_STRUCTURE " SET topicId=?, title=?, lastUpdate=? WHERE topic LIKE ?"; 
	char stat_child[] = "INSERT INTO " TABLE_STRUCTURE " SET Id=?, parentId=?, level=?, topic=?";
	mysql = mysql_init(NULL);

	if (!mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8")) {
		fprintf(stderr, "%s\n", mysql_error(mysql));
	}
   
	/* Connect to database */
	if (!mysql_real_connect(mysql, MYSQL_HOST, MYSQL_USR, MYSQL_PWD, MYSQL_DB, 0, NULL, 0)) {
		fprintf(stderr, "%s\n", mysql_error(mysql));
		mysql = NULL;
	}

	if ( NULL == (stmt_parent =  mysql_stmt_init(mysql)) ) {
		perror("Out of memory!");
	}

	if ( NULL == (stmt_child =  mysql_stmt_init(mysql)) ) {
		perror("Out of memory!");
	}

	if ( mysql_stmt_prepare(stmt_parent, stat_parent, strlen(stat_parent)) ) {
		fprintf(stderr, "%s\n", mysql_error(mysql));
	}

	if ( mysql_stmt_prepare(stmt_child, stat_child, strlen(stat_child)) ) {
		fprintf(stderr, "%s\n", mysql_error(mysql));
	}
}

void finishDB() {
	if (mysql_stmt_close(stmt_parent)) {
		fprintf(stderr, " failed while closing the statement\n");
		fprintf(stderr, " %s\n", mysql_stmt_error(stmt_parent));
	}
	
	if (mysql_stmt_close(stmt_child)) {
		fprintf(stderr, " failed while closing the statement\n");
		fprintf(stderr, " %s\n", mysql_stmt_error(stmt_child));
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

	if (mysql == NULL)
		exit(2);
#endif

  XML_SetUserData(parser, &userData);
  XML_SetElementHandler(parser, startElement, endElement);
  XML_SetCharacterDataHandler(parser, charData);
  do {
    size_t len = fread(buf, 1, sizeof(buf), stdin);
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
	return(0);
}
