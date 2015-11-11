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
#include "vp-db-common.h"
#include "vp-db-util.h"
#include "vp-file-util.h"

#include "vp-media-content-util.h"

#include "vp-thumb-db.h"


#define THUMB_DB_INSERT_QUERY		"INSERT INTO video_thumb (thumb_idx, file_path, file_size, media_id) VALUES ( %d, '%q', %d, '%q')"
#define THUMB_DB_DELETE_QUERY		"DELETE FROM video_thumb where media_id = '%q'"
#define THUMB_DB_SELECT_QUERY		"select thumb_uuid, thumb_idx, file_path, file_size, media_id from video_thumb where media_id = '%q'"
#define THUMB_DB_COUNT_QUERY		"select count(*) from video_thumb where media_id = '%q'"
#define THUMB_DB_ALL_ITEM_QUERY		"select DISTINCT(media_id) from video_thumb"



typedef struct _ThumbDB {
	char *szMediaURL;
	char *szMediaID;
	char *szFilePath;
	int nFileSize;
	int nTotalCount;
} ThumbDB;


static void _vp_thumb_db_destory_handle(ThumbDB *pThumbDB);


/* callback functions */

/* internal functions */
static void _vp_thumb_db_destory_handle(ThumbDB *pThumbDB)
{
	if (pThumbDB == NULL) {
		vp_dbgE("pThumbDB is NULL");
		return;
	}

	VP_FREE(pThumbDB->szMediaURL);
	VP_FREE(pThumbDB->szMediaID);
	VP_FREE(pThumbDB->szFilePath);

	VP_FREE(pThumbDB);
}

static bool _vp_thumb_db_get_count(char *szMediaID, int *nCount)
{
	if (szMediaID == NULL) {
		vp_dbgE("szMediaID is NULL");
		return FALSE;
	}
	char *szSql = sqlite3_mprintf(THUMB_DB_COUNT_QUERY, szMediaID);
	sqlite3_stmt *stmt = NULL;

	if (!vp_db_util_query_prepare(szSql, &stmt)) {
		if (szSql) {
			sqlite3_free(szSql);
			szSql = NULL;
		}
		return FALSE;
	}

	int nRet = 0;
	nRet = sqlite3_step(stmt);
	if (nRet == SQLITE_ROW) {
		*nCount = (int) sqlite3_column_int(stmt, 0);
		vp_dbgW("Get Thumb Count : %d", *nCount);
	}

	if (stmt) {
		sqlite3_finalize(stmt);
		stmt = NULL;
	}

	if (szSql) {
		sqlite3_free(szSql);
		szSql = NULL;
	}

	return TRUE;
}

static bool _vp_thumb_db_delete_items(char *szMediaID)
{
	if (szMediaID == NULL) {
		vp_dbgE("szMediaID is NULL");
		return FALSE;
	}

	char *szSql = sqlite3_mprintf(THUMB_DB_SELECT_QUERY, szMediaID);
	sqlite3_stmt *stmt = NULL;

	if (!vp_db_util_query_prepare(szSql, &stmt)) {
		if (szSql) {
			sqlite3_free(szSql);
			szSql = NULL;
		}
		return FALSE;
	}

	int nRet = 0;
	nRet = sqlite3_step(stmt);
	while (nRet == SQLITE_ROW) {
		char *szFileName = (char *) sqlite3_column_text(stmt, 2);

		if (vp_file_exists(szFileName)) {
			vp_file_unlink(szFileName);
		}

		nRet = sqlite3_step(stmt);
	}

	if (stmt) {
		sqlite3_finalize(stmt);
		stmt = NULL;
	}

	if (szSql) {
		sqlite3_free(szSql);
		szSql = NULL;
	}

	return TRUE;
}



/* external functions */
vp_thumb_db_h vp_thumb_db_create()
{
	ThumbDB *pThumbDB = calloc(1, sizeof(ThumbDB));

	if (pThumbDB == NULL) {
		vp_dbgE("pThumbDB alloc is fail");
		return NULL;
	}
	if (!vp_db_util_connect()) {
		vp_dbgE("Database connect is fail");
		_vp_thumb_db_destory_handle(pThumbDB);
		return NULL;
	}

	return (vp_thumb_db_h) pThumbDB;
}

void vp_thumb_db_destroy(vp_thumb_db_h hThumbDB)
{
	if (hThumbDB == NULL) {
		vp_dbgE("hThumbDB is NULL");
		return;
	}

	vp_db_util_disconnect();

	ThumbDB *pThumbDB = (ThumbDB *) hThumbDB;

	_vp_thumb_db_destory_handle(pThumbDB);
}

bool vp_thumb_db_set_media_id(vp_thumb_db_h hThumbDB,
                              const char *szMediaID)
{
	if (hThumbDB == NULL) {
		vp_dbgE("hThumbDB is NULL");
		return FALSE;
	}

	ThumbDB *pThumbDB = (ThumbDB *) hThumbDB;

	if (szMediaID == NULL) {
		vp_dbgE("szMediaID is NULL");
		return FALSE;
	}

	VP_FREE(pThumbDB->szMediaID);

	VP_STRDUP(pThumbDB->szMediaID, szMediaID);

	return TRUE;
}

bool vp_thumb_db_set_media_url(vp_thumb_db_h hThumbDB,
                               const char *szMediaURL)
{
	if (hThumbDB == NULL) {
		vp_dbgE("hThumbDB is NULL");
		return FALSE;
	}

	if (szMediaURL == NULL) {
		vp_dbgE("szMediaURL is NULL");
		return FALSE;
	}

	ThumbDB *pThumbDB = (ThumbDB *) hThumbDB;

	VP_FREE(pThumbDB->szMediaURL);
	VP_FREE(pThumbDB->szMediaID);

	VP_STRDUP(pThumbDB->szMediaURL, szMediaURL);

	if (!vp_media_content_util_get_video_id
	        (szMediaURL, &(pThumbDB->szMediaID))) {
		vp_dbgE("vp_media_content_util_get_video_id is fail");
		return FALSE;
	}

	return TRUE;
}


bool vp_thumb_db_get_items(vp_thumb_db_h hThumbDB, GList **pThumbList)
{
	if (hThumbDB == NULL) {
		vp_dbgE("hThumbDB is NULL");
		return FALSE;
	}

	ThumbDB *pThumbDB = (ThumbDB *) hThumbDB;

	if (pThumbDB->szMediaID == NULL) {
		vp_dbgE("szMediaID is NULL");
		return FALSE;
	}

	char *szSql =
	    sqlite3_mprintf(THUMB_DB_SELECT_QUERY, pThumbDB->szMediaID);
	sqlite3_stmt *stmt = NULL;

	if (!vp_db_util_query_prepare(szSql, &stmt)) {
		if (szSql) {
			sqlite3_free(szSql);
			szSql = NULL;
		}
		return FALSE;
	}

	int nRet = 0;
	nRet = sqlite3_step(stmt);
	while (nRet == SQLITE_ROW) {
		/* thumb_uuid, thumb_idx, file_path, file_size, media_id */
		ThumbDBItem *pItem = calloc(1, sizeof(ThumbDBItem));
		if (!pItem) {
			vp_dbgE("allocation failed");
			return FALSE;
		}
		vp_sdbg("UUID : %d", (int) sqlite3_column_int(stmt, 0));
		vp_sdbg("THUMB_IDX : %d", (int) sqlite3_column_int(stmt, 1));
		vp_sdbg("FILE_PATH : %s", (char *) sqlite3_column_text(stmt, 2));
		vp_sdbg("FILE_SIZE : %d", (int) sqlite3_column_int(stmt, 3));
		vp_sdbg("MEDIA_ID : %s", (char *) sqlite3_column_text(stmt, 4));

		pItem->nIdx = (int) sqlite3_column_int(stmt, 1);
		VP_STRDUP(pItem->szThumbPath,
		          (char *) sqlite3_column_text(stmt, 2));
		pItem->nFileSize = (int) sqlite3_column_int(stmt, 3);

		*pThumbList = g_list_append(*pThumbList, pItem);

		nRet = sqlite3_step(stmt);
	}

	if (stmt) {
		sqlite3_finalize(stmt);
		stmt = NULL;
	}

	if (szSql) {
		sqlite3_free(szSql);
		szSql = NULL;
	}

	return TRUE;
}

bool vp_thumb_db_get_all_items_media_id(vp_thumb_db_h hThumbDB,
                                        GList **pThumbList)
{
	if (hThumbDB == NULL) {
		vp_dbgE("hThumbDB is NULL");
		return FALSE;
	}

	char *szSql = sqlite3_mprintf(THUMB_DB_ALL_ITEM_QUERY);
	sqlite3_stmt *stmt = NULL;

	if (!vp_db_util_query_prepare(szSql, &stmt)) {
		if (szSql) {
			sqlite3_free(szSql);
			szSql = NULL;
		}
		return FALSE;
	}

	int nRet = 0;
	nRet = sqlite3_step(stmt);
	while (nRet == SQLITE_ROW) {
		char *szMediaID = NULL;
		VP_STRDUP(szMediaID, (char *) sqlite3_column_text(stmt, 0));
		*pThumbList = g_list_append(*pThumbList, szMediaID);

		nRet = sqlite3_step(stmt);
	}

	if (stmt) {
		sqlite3_finalize(stmt);
		stmt = NULL;
	}

	if (szSql) {
		sqlite3_free(szSql);
		szSql = NULL;
	}


	return TRUE;
}


bool vp_thumb_db_insert(vp_thumb_db_h hThumbDB, const char *szThumbPath,
                        int nIdx)
{
	if (hThumbDB == NULL) {
		vp_dbgE("hThumbDB is NULL");
		return FALSE;
	}

	if (szThumbPath == NULL) {
		vp_dbgE("szThumbPath is NULL");
		return NULL;
	}

	ThumbDB *pThumbDB = (ThumbDB *) hThumbDB;

	if (pThumbDB->szMediaID == NULL) {
		vp_dbgE("szMediaID is NULL");
		return FALSE;
	}

	long long nSize = vp_file_size(szThumbPath);

	char *szSql =
	    sqlite3_mprintf(THUMB_DB_INSERT_QUERY, nIdx, szThumbPath,
	                    (int) nSize, pThumbDB->szMediaID);
	vp_sdbg("%s", szSql);

	sqlite3_stmt *stmt = NULL;

	if (!vp_db_util_query_prepare(szSql, &stmt)) {
		if (szSql) {
			sqlite3_free(szSql);
			szSql = NULL;
		}
		return FALSE;
	}

	sqlite3_step(stmt);

	if (stmt) {
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
		sqlite3_finalize(stmt);
		stmt = NULL;
	}

	if (szSql) {
		sqlite3_free(szSql);
		szSql = NULL;
	}

	return TRUE;
}


bool vp_thumb_db_delete(vp_thumb_db_h hThumbDB)
{
	if (hThumbDB == NULL) {
		vp_dbgE("hThumbDB is NULL");
		return FALSE;
	}

	ThumbDB *pThumbDB = (ThumbDB *) hThumbDB;

	if (pThumbDB->szMediaID == NULL) {
		vp_dbgE("szMediaID is NULL");
		return FALSE;
	}

	_vp_thumb_db_delete_items(pThumbDB->szMediaID);

	char *szSql =
	    sqlite3_mprintf(THUMB_DB_DELETE_QUERY, pThumbDB->szMediaID);
	vp_sdbg("%s", szSql);

	sqlite3_stmt *stmt = NULL;

	if (!vp_db_util_query_prepare(szSql, &stmt)) {
		if (szSql) {
			sqlite3_free(szSql);
			szSql = NULL;
		}
		return FALSE;
	}

	sqlite3_step(stmt);

	if (stmt) {
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
		sqlite3_finalize(stmt);
		stmt = NULL;
	}

	if (szSql) {
		sqlite3_free(szSql);
		szSql = NULL;
	}

	return TRUE;
}


bool vp_thumb_db_item_count(vp_thumb_db_h hThumbDB, int *nCount)
{
	if (hThumbDB == NULL) {
		vp_dbgE("hThumbDB is NULL");
		return FALSE;
	}

	ThumbDB *pThumbDB = (ThumbDB *) hThumbDB;

	return _vp_thumb_db_get_count(pThumbDB->szMediaID, nCount);
}

bool vp_thumb_db_item_free(ThumbDBItem *pItem)
{
	if (pItem == NULL) {
		vp_dbgE("ThumbDBItem is NULL");
		return FALSE;
	}

	VP_FREE(pItem->szThumbPath);
	VP_FREE(pItem);

	return TRUE;
}
