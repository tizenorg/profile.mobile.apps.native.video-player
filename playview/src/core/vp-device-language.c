/*
* Copyright (c) 2000-2015 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include <app.h>
#include <glib.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <Elementary.h>

#include "vp-play-type-define.h"
#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-macro-define.h"
#include "vp-device-language.h"

/* check temp */
#include "vp-play-log.h"


#define VP_LANGLIST_FILE_PATH		"/opt/usr/data/setting/langlist.xml"


typedef struct {
	char *szLanguageCode;	// ex) "en_US.UTF"
	char *szLanguageName;	// "United Kingdom"
	char *szMcc;
} stLanguage;

static Eina_List *g_pLanglist = NULL;	// internal

bool _vp_language_list_destroy(void)
{
	stLanguage *szNode = NULL;
	if (!g_pLanglist) {
		VideoLogError("g_pLanglist is NULL");
		return FALSE;
	}
	EINA_LIST_FREE(g_pLanglist, szNode) {
		if (szNode) {
			VP_FREE(szNode->szLanguageCode);
			VP_FREE(szNode->szLanguageName);
			VP_FREE(szNode->szMcc);
			VP_FREE(szNode);
		}
	}
	g_pLanglist = NULL;

	return TRUE;
}

static void _vp_language_get_list(xmlNodePtr cur)
{
	xmlNode *cur_node = NULL;
	char *pId = NULL;
	char *pName = NULL;
	char *pMcc = NULL;

	for (cur_node = cur; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE) {
			pId =
				(char *) g_strdup((char *)
						  xmlGetProp(cur_node,
							     (const xmlChar *) "id"));
			pName =
				(char *) g_strdup((char *)
						  xmlGetProp(cur_node,
							     (const xmlChar *) "string"));
			pMcc =
				(char *) g_strdup((char *)
						  xmlGetProp(cur_node,
							     (const xmlChar *) "mcc"));
			stLanguage *szNode =
				(stLanguage *) calloc(1, sizeof(stLanguage));
			if (szNode) {
				szNode->szLanguageCode = pId;
				szNode->szLanguageName = pName;
				szNode->szMcc = pMcc;
				g_pLanglist = eina_list_append(g_pLanglist, szNode);
			}
		}
	}
}

static void _vp_language_parse_XML(char *szPath)
{
	if (!szPath) {
		VideoLogError("szPath is NULL");
		return;
	}
	xmlDocPtr doc;
	xmlNodePtr cur;

	doc = xmlParseFile(szPath);

	if (doc == NULL) {
		VideoLogError("Document not parsed successfully");
		return;
	}

	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		VideoLogError("empty document");
		xmlFreeDoc(doc);
		return;
	}
	// make sure root node is "langlist"
	if (xmlStrcmp(cur->name, (const xmlChar *) "langlist")) {
		VideoLogError
		("document of the wrong type, root node != langlist");
		xmlFreeDoc(doc);
		return;
	}

	cur = cur->xmlChildrenNode;
	_vp_language_get_list(cur);

	/* save tree to file */
	if (doc != NULL) {
		//xmlSaveFormatFile (docname, doc, 1);
		xmlFreeDoc(doc);
	}
}

bool _vp_language_list_load(char *szPath)
{
	if (!g_pLanglist) {
		_vp_language_parse_XML(szPath);
	}

	return TRUE;
}

char *vp_language_get_name(const char *szCode)
{
	if (!g_pLanglist) {
		VideoLogError("g_pLanglist IS null");
		return NULL;
	}

	if (!szCode) {
		VideoLogError("szCode IS null");
		return NULL;
	}
	Eina_List *pIterateList = NULL;
	stLanguage *szNode = NULL;

	char *pLangCode = NULL;
	char *pRealCode = NULL;
	char *szName = NULL;
	pLangCode = g_utf8_strup(szCode, -1);

	EINA_LIST_FOREACH(g_pLanglist, pIterateList, szNode) {
		if (szNode) {
			pRealCode = g_utf8_strup(szNode->szLanguageCode, -1);

			if (g_strrstr(pRealCode, pLangCode)) {
				VP_STRDUP(szName, szNode->szLanguageName);
				break;
			}
			VP_FREE(pRealCode);
		}
	}
	VP_FREE(pLangCode);
	VP_FREE(pRealCode);

	return szName;
}

void vp_language_list_destroy()
{
	VideoLogInfo("");
	_vp_language_list_destroy();
}

void vp_language_list_create()
{
	if (NULL == g_pLanglist) {
		VideoLogInfo("");
		_vp_language_list_load(VP_LANGLIST_FILE_PATH);
	}
}
