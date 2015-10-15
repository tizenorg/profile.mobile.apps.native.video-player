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

#include "vp-media-content-util.h"

#include "vp-chapter-db.h"
#include "vp-file-util.h"

#define CHAPTER_DB_INSERT_QUERY		"INSERT INTO video_chapter (chapter_idx, file_path, file_size, media_id) VALUES ( %d, '%q', %d, '%q')"
#define CHAPTER_DB_DELETE_QUERY		"DELETE FROM video_chapter where media_id = '%q'"
#define CHAPTER_DB_SELECT_QUERY		"select chapter_uuid, chapter_idx, file_path, file_size, media_id from video_chapter where media_id = '%q'"
#define CHAPTER_DB_COUNT_QUERY		"select count(*) from video_chapter where media_id = '%q'"
#define CHAPTER_DB_ALL_ITEM_QUERY	"select DISTINCT(media_id) from video_chapter"

typedef struct _ChapterDB {
	char *szMediaURL;
	char *szMediaID;
	char *szFilePath;
	int nFileSize;
	int nTotalCount;
} ChapterDB;


static void _vp_chapter_db_destory_handle(ChapterDB *pChapterDB);


/* callback functions */

/* internal functions */
static void _vp_chapter_db_destory_handle(ChapterDB *pChapterDB)
{
	if (pChapterDB == NULL) {
		vp_dbgE("pChapterDB is NULL");
		return;
	}

	VP_FREE(pChapterDB->szMediaURL);
	VP_FREE(pChapterDB->szMediaID);
	VP_FREE(pChapterDB->szFilePath);

	VP_FREE(pChapterDB);
}

static bool _vp_chapter_db_get_count(char *szMediaID, int *nCount)
{
	if (szMediaID == NULL) {
		vp_dbgE("szMediaID is NULL");
		return FALSE;
	}
	char *szSql = sqlite3_mprintf(CHAPTER_DB_COUNT_QUERY, szMediaID);
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

static bool _vp_chapter_db_delete_items(char *szMediaID)
{
	if (szMediaID == NULL) {
		vp_dbgE("szMediaID is NULL");
		return FALSE;
	}

	char *szSql = sqlite3_mprintf(CHAPTER_DB_SELECT_QUERY, szMediaID);
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
vp_chapter_db_h vp_chapter_db_create()
{
	ChapterDB *pChapterDB = calloc(1, sizeof(ChapterDB));

	if (pChapterDB == NULL) {
		vp_dbgE("pChapterDB alloc is fail");
		return NULL;
	}

	if (!vp_db_util_connect()) {
		vp_dbgE("Database connect is fail");
		_vp_chapter_db_destory_handle(pChapterDB);
		return NULL;
	}

	return (vp_chapter_db_h) pChapterDB;
}

void vp_chapter_db_destroy(vp_chapter_db_h hChapterDB)
{
	if (hChapterDB == NULL) {
		vp_dbgE("hChapterDB is NULL");
		return;
	}

	vp_db_util_disconnect();

	ChapterDB *pChapterDB = (ChapterDB *) hChapterDB;

	_vp_chapter_db_destory_handle(pChapterDB);
}

bool vp_chapter_db_set_media_id(vp_chapter_db_h hChapterDB,
				const char *szMediaID)
{
	if (hChapterDB == NULL) {
		vp_dbgE("hChapterDB is NULL");
		return FALSE;
	}

	if (szMediaID == NULL) {
		vp_dbgE("szMediaID is NULL");
		return FALSE;
	}

	ChapterDB *pChapterDB = (ChapterDB *) hChapterDB;

	VP_FREE(pChapterDB->szMediaID);

	VP_STRDUP(pChapterDB->szMediaID, szMediaID);

	return TRUE;
}

bool vp_chapter_db_set_media_url(vp_chapter_db_h hChapterDB,
				 const char *szMediaURL)
{
	if (hChapterDB == NULL) {
		vp_dbgE("hChapterDB is NULL");
		return FALSE;
	}

	if (szMediaURL == NULL) {
		vp_dbgE("szMediaURL is NULL");
		return FALSE;
	}

	ChapterDB *pChapterDB = (ChapterDB *) hChapterDB;

	VP_FREE(pChapterDB->szMediaURL);
	VP_FREE(pChapterDB->szMediaID);

	VP_STRDUP(pChapterDB->szMediaURL, szMediaURL);

	if (!vp_media_content_util_get_video_id
			(szMediaURL, &(pChapterDB->szMediaID))) {
		vp_dbgE("vp_media_content_util_get_video_id is fail");
		return FALSE;
	}

	return TRUE;
}

bool vp_chapter_db_get_items(vp_chapter_db_h hChapterDB,
			     GList **pChapterList)
{
	if (hChapterDB == NULL) {
		vp_dbgE("hChapterDB is NULL");
		return FALSE;
	}

	ChapterDB *pChapterDB = (ChapterDB *) hChapterDB;

	if (pChapterDB->szMediaID == NULL) {
		vp_dbgE("szMediaID is NULL");
		return FALSE;
	}

	char *szSql =
		sqlite3_mprintf(CHAPTER_DB_SELECT_QUERY, pChapterDB->szMediaID);
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
		/* chapter_uuid, chapter_idx, file_path, file_size, media_id */
		ChapterDBItem *pItem = calloc(1, sizeof(ChapterDBItem));
		if (!pItem) {
			vp_dbgE("allocation failed");
			return FALSE;
		}
		vp_sdbg("UUID : %d", (int) sqlite3_column_int(stmt, 0));
		vp_sdbg("CHAPTER_IDX : %d", (int) sqlite3_column_int(stmt, 1));
		vp_sdbg("FILE_PATH : %s", (char *) sqlite3_column_text(stmt, 2));
		vp_sdbg("FILE_SIZE : %d", (int) sqlite3_column_int(stmt, 3));
		vp_sdbg("MEDIA_ID : %s", (char *) sqlite3_column_text(stmt, 4));

		pItem->nIdx = (int) sqlite3_column_int(stmt, 1);
		VP_STRDUP(pItem->szChapterPath,
			  (char *) sqlite3_column_text(stmt, 2));
		pItem->nFileSize = (int) sqlite3_column_int(stmt, 3);

		*pChapterList = g_list_append(*pChapterList, pItem);

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

bool vp_chapter_db_get_all_items_media_id(vp_chapter_db_h hChapterDB,
		GList **pChapterList)
{
	if (hChapterDB == NULL) {
		vp_dbgE("hChapterDB is NULL");
		return FALSE;
	}

	char *szSql = sqlite3_mprintf(CHAPTER_DB_ALL_ITEM_QUERY);
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

		*pChapterList = g_list_append(*pChapterList, szMediaID);

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

bool vp_chapter_db_insert(vp_chapter_db_h hChapterDB,
			  const char *szChapterPath, int nIdx)
{
	if (hChapterDB == NULL) {
		vp_dbgE("hChapterDB is NULL");
		return FALSE;
	}

	if (szChapterPath == NULL) {
		vp_dbgE("szChapterPath is NULL");
		return FALSE;
	}

	ChapterDB *pChapterDB = (ChapterDB *) hChapterDB;

	if (pChapterDB->szMediaID == NULL) {
		vp_dbgE("szMediaID is NULL");
		return FALSE;
	}


	long long nSize = vp_file_size(szChapterPath);

	char *szSql =
		sqlite3_mprintf(CHAPTER_DB_INSERT_QUERY, nIdx, szChapterPath,
				(int) nSize, pChapterDB->szMediaID);
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

bool vp_chapter_db_delete(vp_chapter_db_h hChapterDB)
{
	if (hChapterDB == NULL) {
		vp_dbgE("hChapterDB is NULL");
		return FALSE;
	}

	ChapterDB *pChapterDB = (ChapterDB *) hChapterDB;

	if (pChapterDB->szMediaID == NULL) {
		vp_dbgE("szMediaID is NULL");
		return FALSE;
	}

	_vp_chapter_db_delete_items(pChapterDB->szMediaID);

	char *szSql =
		sqlite3_mprintf(CHAPTER_DB_DELETE_QUERY, pChapterDB->szMediaID);
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

bool vp_chapter_db_item_count(vp_chapter_db_h hChapterDB, int *nCount)
{
	if (hChapterDB == NULL) {
		vp_dbgE("hChapterDB is NULL");
		return FALSE;
	}

	ChapterDB *pChapterDB = (ChapterDB *) hChapterDB;

	return _vp_chapter_db_get_count(pChapterDB->szMediaID, nCount);
}

bool vp_chapter_db_item_free(ChapterDBItem *pItem)
{
	if (pItem == NULL) {
		vp_dbgE("ChapterDBItem is NULL");
		return FALSE;
	}

	VP_FREE(pItem->szChapterPath);
	VP_FREE(pItem);

	return TRUE;
}
