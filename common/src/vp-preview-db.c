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

#include "vp-preview-db.h"
#include "vp-media-content-util.h"
#include "vp-file-util.h"

#define PREVIEW_DB_INSERT_QUERY		"INSERT INTO video_preview (file_path, file_size, media_id) VALUES ( '%q', %d, '%q')"
#define PREVIEW_DB_DELETE_QUERY		"DELETE FROM video_preview where media_id = '%q'"
#define PREVIEW_DB_SELECT_QUERY		"select preview_uuid, file_path, file_size, last_hit_date, media_id from video_preview where media_id = '%q'"
#define PREVIEW_DB_LAST_HIT_QUERY	"select preview_uuid, file_path, file_size, last_hit_date, media_id from video_preview order by last_hit_date DESC"
#define PREVIEW_DB_COUNT_QUERY		"select count(*) from video_preview where media_id = '%q'"
#define PREVIEW_DB_ALL_ITEM_QUERY	"select DISTINCT(media_id) from video_preview"

typedef struct _PreviewDB {
	char *szMediaURL;
	char *szMediaID;
	char *szFilePath;
	int nFileSize;
} PreviewDB;


static void _vp_preview_db_destory_handle(PreviewDB *pPreviewDB);


/* callback functions */

/* internal functions */
static void _vp_preview_db_destory_handle(PreviewDB *pPreviewDB)
{
	if (pPreviewDB == NULL) {
		vp_dbgE("pPreviewDB is NULL");
		return;
	}

	VP_FREE(pPreviewDB->szMediaURL);
	VP_FREE(pPreviewDB->szMediaID);
	VP_FREE(pPreviewDB->szFilePath);

	VP_FREE(pPreviewDB);
}

static bool _vp_preview_db_get_count(char *szMediaID, int *nCount)
{
	if (szMediaID == NULL) {
		vp_dbgE("szMediaID is NULL");
		return FALSE;
	}

	char *szSql = sqlite3_mprintf(PREVIEW_DB_COUNT_QUERY, szMediaID);
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
		vp_dbgW("Get Chapter Count : %d", *nCount);
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

static bool _vp_preview_db_delete_item(char *szMeidaID)
{
	if (szMeidaID == NULL) {
		vp_dbgE("szMeidaID is NULL");
		return FALSE;
	}

	char *szSql = sqlite3_mprintf(PREVIEW_DB_SELECT_QUERY, szMeidaID);
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

		char *szFileName = (char *) sqlite3_column_text(stmt, 1);
		if (vp_file_exists(szFileName)) {
			vp_file_unlink(szFileName);
		}

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
vp_preview_db_h vp_preview_db_create()
{
	PreviewDB *pPreviewDB = calloc(1, sizeof(PreviewDB));

	if (pPreviewDB == NULL) {
		vp_dbgE("pPreviewDB alloc is fail");
		return NULL;
	}

	if (!vp_db_util_connect()) {
		vp_dbgE("Database connect is fail");
		_vp_preview_db_destory_handle(pPreviewDB);
		return NULL;
	}

	return (vp_preview_db_h) pPreviewDB;
}

void vp_preview_db_destroy(vp_preview_db_h hPreviewDB)
{
	if (hPreviewDB == NULL) {
		vp_dbgE("hPreviewDB is NULL");
		return;
	}

	vp_db_util_disconnect();

	PreviewDB *pPreviewDB = (PreviewDB *) hPreviewDB;

	_vp_preview_db_destory_handle(pPreviewDB);
}

bool vp_preview_db_set_media_id(vp_preview_db_h hPreviewDB,
                                const char *szMediaID)
{
	if (hPreviewDB == NULL) {
		vp_dbgE("hPreviewDB is NULL");
		return FALSE;
	}

	if (szMediaID == NULL) {
		vp_dbgE("szMediaID is NULL");
		return FALSE;
	}

	PreviewDB *pPreviewDB = (PreviewDB *) hPreviewDB;

	VP_FREE(pPreviewDB->szMediaID);

	VP_STRDUP(pPreviewDB->szMediaID, szMediaID);

	return TRUE;
}

bool vp_preview_db_set_media_url(vp_preview_db_h hPreviewDB,
                                 const char *szMediaURL)
{
	if (hPreviewDB == NULL) {
		vp_dbgE("hPreviewDB is NULL");
		return FALSE;
	}

	if (szMediaURL == NULL) {
		vp_dbgE("szMediaURL is NULL");
		return FALSE;
	}

	PreviewDB *pPreviewDB = (PreviewDB *) hPreviewDB;

	VP_FREE(pPreviewDB->szMediaURL);
	VP_FREE(pPreviewDB->szMediaID);

	VP_STRDUP(pPreviewDB->szMediaURL, szMediaURL);

	if (!vp_media_content_util_get_video_id
	        (szMediaURL, &(pPreviewDB->szMediaID))) {
		vp_dbgE("vp_media_content_util_get_video_id is fail");
		return FALSE;
	}

	return TRUE;
}


bool vp_preview_db_get_file_path(vp_preview_db_h hPreviewDB,
                                 char **szFilePath)
{
	if (hPreviewDB == NULL) {
		vp_dbgE("hPreviewDB is NULL");
		return FALSE;
	}

	PreviewDB *pPreviewDB = (PreviewDB *) hPreviewDB;

	if (pPreviewDB->szMediaID == NULL) {
		vp_dbgE("szMediaID is NULL");
		return FALSE;
	}

	char *szSql =
	    sqlite3_mprintf(PREVIEW_DB_SELECT_QUERY, pPreviewDB->szMediaID);
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
		/* preview_uuid, file_path, file_size, media_id */
		vp_sdbg("UUID : %d", (int) sqlite3_column_int(stmt, 0));
		vp_sdbg("FILE_PATH : %s", (char *) sqlite3_column_text(stmt, 1));
		vp_sdbg("FILE_SIZE : %d", (int) sqlite3_column_int(stmt, 2));
		vp_sdbg("LAST_DATE : %d", (int) sqlite3_column_int(stmt, 3));
		vp_sdbg("MEDIA_ID : %s", (char *) sqlite3_column_text(stmt, 4));

		VP_STRDUP(*szFilePath, (char *) sqlite3_column_text(stmt, 1));
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

bool vp_preview_db_get_file_size(vp_preview_db_h hPreviewDB,
                                 int *nFileSize)
{
	if (hPreviewDB == NULL) {
		vp_dbgE("hPreviewDB is NULL");
		return FALSE;
	}

	PreviewDB *pPreviewDB = (PreviewDB *) hPreviewDB;

	if (pPreviewDB->szMediaID == NULL) {
		vp_dbgE("szMediaID is NULL");
		return FALSE;
	}

	char *szSql =
	    sqlite3_mprintf(PREVIEW_DB_SELECT_QUERY, pPreviewDB->szMediaID);
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
		/* preview_uuid, file_path, file_size, media_id */
		vp_sdbg("UUID : %d", (int) sqlite3_column_int(stmt, 0));
		vp_sdbg("FILE_PATH : %s", (char *) sqlite3_column_text(stmt, 1));
		vp_sdbg("FILE_SIZE : %d", (int) sqlite3_column_int(stmt, 2));
		vp_sdbg("LAST_DATE : %d", (int) sqlite3_column_int(stmt, 3));
		vp_sdbg("MEDIA_ID : %s", (char *) sqlite3_column_text(stmt, 4));


		*nFileSize = (int) sqlite3_column_int(stmt, 2);
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

bool vp_preview_db_item_count(vp_preview_db_h hPreviewDB, int *nCount)
{
	if (hPreviewDB == NULL) {
		vp_dbgE("hPreviewDB is NULL");
		return FALSE;
	}

	PreviewDB *pPreviewDB = (PreviewDB *) hPreviewDB;

	return _vp_preview_db_get_count(pPreviewDB->szMediaID, nCount);
}

bool vp_preview_db_get_all_items_media_id(vp_preview_db_h hPreviewDB,
        GList **pPreviewList)
{
	if (hPreviewDB == NULL) {
		vp_dbgE("hPreviewDB is NULL");
		return FALSE;
	}

	char *szSql = sqlite3_mprintf(PREVIEW_DB_ALL_ITEM_QUERY);
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

		*pPreviewList = g_list_append(*pPreviewList, szMediaID);

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

bool vp_preview_db_insert(vp_preview_db_h hPreviewDB,
                          const char *szPreviewPath)
{
	if (hPreviewDB == NULL) {
		vp_dbgE("hPreviewDB is NULL");
		return FALSE;
	}

	if (szPreviewPath == NULL) {
		vp_dbgE("szMediaID is NULL");
		return NULL;
	}

	PreviewDB *pPreviewDB = (PreviewDB *) hPreviewDB;

	if (pPreviewDB->szMediaID == NULL) {
		vp_dbgE("szMediaID is NULL");
		return FALSE;
	}

	long long nSize = vp_file_size(szPreviewPath);

	char *szSql =
	    sqlite3_mprintf(PREVIEW_DB_INSERT_QUERY, szPreviewPath,
	                    (int) nSize, pPreviewDB->szMediaID);
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

bool vp_preview_db_delete(vp_preview_db_h hPreviewDB)
{
	if (hPreviewDB == NULL) {
		vp_dbgE("hPreviewDB is NULL");
		return FALSE;
	}

	PreviewDB *pPreviewDB = (PreviewDB *) hPreviewDB;

	if (pPreviewDB->szMediaID == NULL) {
		vp_dbgE("szMediaID is NULL");
		return FALSE;
	}

	_vp_preview_db_delete_item(pPreviewDB->szMediaID);

	char *szSql =
	    sqlite3_mprintf(PREVIEW_DB_DELETE_QUERY, pPreviewDB->szMediaID);
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
