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


#include "vp-play-type-define.h"
#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-macro-define.h"

#include "vp-play-util.h"


#include "vp-multi-path.h"

/* check temp */
#include "vp-play-log.h"


typedef struct _MultiPath {
	char *szURL;
	char *szTitle;
	char *szSubTitle;
	int nStartPosition;
	int nDuration;
	bool bIsSameAP;
} MultiPath;



/* internal functions */
static bool _vp_multi_path_write_multi_path_set_position(char *szURL,
                        int nPosition)
{
	char *app_path = app_get_data_path();
	if (!app_path) {
		VideoLogError("cannot retrieve app install path");
		return FALSE;
	}
	char db_path[1024] = {0,};
	snprintf(db_path, 1024, "%s%s", app_path, "saveposition.ini");
	VideoLogError("db_path: %s", db_path);
	FILE *fp = fopen(db_path, "w");

	if (fp == NULL) {
		VideoLogError("%s is NULL.", db_path);
		return FALSE;
	}

	fprintf(fp, "#saveposition\n");
	fprintf(fp, "uri=%s\n", szURL);
	fprintf(fp, "position=%d\n", nPosition);
	fclose(fp);

	return TRUE;
}



/* external functions */
bool vp_multi_path_add_item(GList **pList,
                            const char *szURL,
                            char *szTitle,
                            char *szSubTitle,
                            int nPosition, int nDuration, bool bIsSameAP)
{
	if (!szURL) {
		VideoLogError("szURL is NULL");
		return FALSE;
	}

	MultiPath *pPath = NULL;

	pPath = calloc(1, sizeof(MultiPath));
	if (pPath == NULL) {
		VideoLogError("pPath alloc fail");
		return FALSE;
	}

	char *szConvert = vp_play_util_remove_prefix_to_url((char *) szURL);
	if (szConvert) {
		VP_STRDUP(pPath->szURL, szConvert);
	} else {
		VP_STRDUP(pPath->szURL, szURL);
	}
	VP_FREE(szConvert);


	szConvert = vp_play_util_remove_prefix_to_url(szSubTitle);
	if (szConvert) {
		VP_STRDUP(pPath->szSubTitle, szConvert);
	} else {
		VP_STRDUP(pPath->szSubTitle, szSubTitle);
	}
	VP_FREE(szConvert);

	VP_STRDUP(pPath->szTitle, szTitle);
	pPath->bIsSameAP = bIsSameAP;

	pPath->nStartPosition = nPosition;
	pPath->nDuration = nDuration;
	*pList = g_list_append(*pList, pPath);

	return TRUE;
}

bool vp_multi_path_clear_item(GList *pList)
{
	if (!pList) {
		VideoLogError("pList is NULL");
		return FALSE;
	}

	int nCount = 0;
	int nIdx = 0;

	nCount = g_list_length(pList);
	for (nIdx = 0; nIdx < nCount; nIdx++) {
		MultiPath *pPath = NULL;
		pPath = (MultiPath *) g_list_nth_data(pList, nIdx);
		if (pPath) {
			VP_FREE(pPath->szURL);
			VP_FREE(pPath->szSubTitle);
			VP_FREE(pPath->szTitle);

			VP_FREE(pPath);
		}
	}

	return TRUE;
}

bool vp_multi_path_set_item_subtitle(const char *szMediaURL,
                                     char *szSubtitleURL, GList *pList)
{
	if (!pList) {
		VideoLogError("pList is NULL");
		return FALSE;
	}

	if (!szMediaURL) {
		VideoLogError("szMediaURL is NULL");
		return FALSE;
	}

	int nCount = 0;
	int nIdx = 0;

	nCount = g_list_length(pList);
	for (nIdx = 0; nIdx < nCount; nIdx++) {
		MultiPath *pPath = NULL;
		pPath = (MultiPath *) g_list_nth_data(pList, nIdx);
		if (pPath == NULL) {
			continue;
		}
		if (pPath->szURL == NULL) {
			continue;
		}
		if (!strcmp(szMediaURL, pPath->szURL)) {
			VP_FREE(pPath->szSubTitle);
			VP_STRDUP(pPath->szSubTitle, szSubtitleURL);
		}
	}

	return TRUE;
}

bool vp_multi_path_set_item_position(const char *szMediaURL,
                                     int nPosition, GList *pList)
{
	if (!pList) {
		VideoLogError("pList is NULL");
		return FALSE;
	}

	if (!szMediaURL) {
		VideoLogError("szMediaURL is NULL");
		return FALSE;
	}

	int nCount = 0;
	int nIdx = 0;

	nCount = g_list_length(pList);
	for (nIdx = 0; nIdx < nCount; nIdx++) {
		MultiPath *pPath = NULL;
		pPath = (MultiPath *) g_list_nth_data(pList, nIdx);
		if (pPath == NULL) {
			continue;
		}
		if (pPath->szURL == NULL) {
			continue;
		}
		if (!strcmp(szMediaURL, pPath->szURL)) {
			pPath->nStartPosition = nPosition;
			_vp_multi_path_write_multi_path_set_position((char *)
			        szMediaURL,
			        nPosition);
		}
	}

	return TRUE;
}

bool vp_multi_path_get_current_item(const char *szMediaURL, char **szURL,
                                    char **szTitle, char **szSubtitle,
                                    int *nPosition, int *nDuration,
                                    bool *bIsSameAP, GList *pList)
{
	if (!szMediaURL) {
		VideoLogError("No exist szMediaID.");
		return FALSE;
	}

	if (pList == NULL) {
		VideoLogError("list is NULL");
		return FALSE;
	}

	int nCount = 0;
	int i = 0;

	nCount = g_list_length(pList);
	for (i = 0; i < nCount; i++) {
		MultiPath *pPath = NULL;
		pPath = (MultiPath *) g_list_nth_data(pList, i);
		if (pPath == NULL) {
			continue;
		}
		if (pPath->szURL == NULL) {
			continue;
		}

		if (!strcmp(szMediaURL, pPath->szURL)) {
			VP_STRDUP(*szURL, pPath->szURL);
			VP_STRDUP(*szSubtitle, pPath->szSubTitle);
			VP_STRDUP(*szTitle, pPath->szTitle);
			*bIsSameAP = pPath->bIsSameAP;
			*nPosition = pPath->nStartPosition;
			*nDuration = pPath->nDuration;

			return TRUE;

		}
	}

	return FALSE;
}


bool vp_multi_path_get_next_item(const char *szMediaURL, char **szURL,
                                 char **szSubtitle, int *nPosition,
                                 int *nDuration, bool bLoop,
                                 GList *pList)
{
	if (!szMediaURL) {
		VideoLogError("No exist szMediaID.");
		return FALSE;
	}

	if (pList == NULL) {
		VideoLogError("list is NULL");
		return FALSE;
	}

	int nCount = 0;
	int i = 0;

	MultiPath *pFirstPathItem = NULL;
	bool bCheck = FALSE;

	nCount = g_list_length(pList);
	for (i = 0; i < nCount; i++) {
		MultiPath *pPath = NULL;
		pPath = (MultiPath *) g_list_nth_data(pList, i);
		if (pPath == NULL) {
			continue;
		}
		if (pPath->szURL == NULL) {
			continue;
		}

		if (pFirstPathItem == NULL) {
			pFirstPathItem = pPath;
		}

		if (bCheck) {
			VP_STRDUP(*szURL, pPath->szURL);
			VP_STRDUP(*szSubtitle, pPath->szSubTitle);
			*nPosition = pPath->nStartPosition;

			return TRUE;
		}

		if (!strcmp(szMediaURL, pPath->szURL)) {
			bCheck = TRUE;
		}

	}

	if (bLoop) {
		if (pFirstPathItem) {
			VP_STRDUP(*szURL, pFirstPathItem->szURL);
			VP_STRDUP(*szSubtitle, pFirstPathItem->szSubTitle);
			*nPosition = pFirstPathItem->nStartPosition;
			*nDuration = pFirstPathItem->nDuration;
		}
	}

	return TRUE;
}

bool vp_multi_path_get_prev_item(const char *szMediaURL, char **szURL,
                                 char **szSubtitle, int *nPosition,
                                 int *nDuration, bool bLoop,
                                 GList *pList)
{
	if (!szMediaURL) {
		VideoLogError("No exist szMediaID.");
		return FALSE;
	}

	if (pList == NULL) {
		VideoLogError("list is NULL");
		return FALSE;
	}

	int nCount = 0;
	int i = 0;


	MultiPath *pLastPathItem = NULL;
	bool bCheck = FALSE;


	nCount = g_list_length(pList);
	for (i = nCount - 1; i >= 0; i--) {
		MultiPath *pPath = NULL;
		pPath = (MultiPath *) g_list_nth_data(pList, i);
		if (pPath == NULL) {
			continue;
		}
		if (pPath->szURL == NULL) {
			continue;
		}

		if (pLastPathItem == NULL) {
			pLastPathItem = pPath;
		}

		if (bCheck) {
			VP_STRDUP(*szURL, pPath->szURL);
			VP_STRDUP(*szSubtitle, pPath->szSubTitle);
			*nPosition = pPath->nStartPosition;
			*nDuration = pPath->nDuration;
			return TRUE;
		}

		if (!strcmp(szMediaURL, pPath->szURL)) {
			bCheck = TRUE;
		}
	}

	if (bLoop) {
		if (pLastPathItem) {
			VP_STRDUP(*szURL, pLastPathItem->szURL);
			VP_STRDUP(*szSubtitle, pLastPathItem->szSubTitle);
			*nPosition = pLastPathItem->nStartPosition;
			*nDuration = pLastPathItem->nDuration;
		}
	}

	return TRUE;
}
