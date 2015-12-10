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

static sqlite3 *g_sqlite3_handle = NULL;
static int g_nRefCount = 0;

/* callback functions */
static int __vp_db_util_busy_handler(void *pData, int count)
{
	usleep(50000);

	vp_dbgW("__vp_db_util_busy_handler called : %d", count);

	return 100 - count;
}

/* internal functions */
static sqlite3 *_vp_db_util_connect_db()
{
	sqlite3 *pSqlite = NULL;
	int ret = SQLITE_OK;

	char *app_path = app_get_data_path();
	if (!app_path) {
		vp_dbgE("cannot retrieve app install path");
		return NULL;
	}
	char db_path[1024] = {0,};
	snprintf(db_path, 1024, "%s%s", app_path, ".video-thumb.db");
	vp_dbgI("db_path: %s", db_path);

	/*Connect DB */
	ret = sqlite3_open(db_path, &pSqlite);
	if (SQLITE_OK != ret) {
		vp_dbgE("sqlite3_open fail [0x%x] : %s", ret,
		        sqlite3_errmsg(pSqlite));
		return NULL;
	}

	ret = sqlite3_busy_handler(pSqlite, __vp_db_util_busy_handler, NULL);
	if (SQLITE_OK != ret) {
		vp_dbgE("sqlite3_busy_handler fail [0x%x]", ret);
		sqlite3_close(pSqlite);
		return NULL;
	}

	return pSqlite;
}

static void _vp_db_util_disconnect_db(sqlite3 *db_handle)
{
	if (db_handle == NULL) {
		vp_dbgE("db_handle is NULL");
		return;
	}
	int ret = SQLITE_OK;
	ret = sqlite3_close(db_handle);
	if (SQLITE_OK != ret) {
		vp_dbgE("sqlite3_close fail [0x%x]", ret);
		db_handle = NULL;
	}
}


/* external functions */
bool vp_db_util_connect()
{
	if (g_sqlite3_handle) {
		vp_dbgW("database is already connect");
		g_nRefCount++;
		return TRUE;
	}

	g_sqlite3_handle = _vp_db_util_connect_db();
	if (g_sqlite3_handle == NULL) {
		vp_dbgE("database connect is fail");
		return FALSE;
	}

	g_nRefCount++;

	return TRUE;
}

void vp_db_util_disconnect()
{
	g_nRefCount--;

	if (g_nRefCount < 0) {
		g_nRefCount = 0;
	}

	if (g_nRefCount == 0) {
		if (g_sqlite3_handle == NULL) {
			vp_dbgW("database is already disconnect");
			return;
		}
		_vp_db_util_disconnect_db(g_sqlite3_handle);

		g_sqlite3_handle = NULL;
	}

	return;

}

bool vp_db_util_query_prepare(char *szQuery, sqlite3_stmt **stmt)
{
	if (g_sqlite3_handle == NULL) {
		vp_dbgE("database is not connect");
		return FALSE;
	}

	int err = -1;

	sqlite3 *handle = g_sqlite3_handle;
	vp_dbg("SQL = %s\n", szQuery);

	err = sqlite3_prepare_v2(handle, szQuery, -1, stmt, NULL);
	if (SQLITE_OK != err) {
		vp_dbgE("Query fails [0x%x]: query_string[%s] ", err, szQuery);
		return FALSE;
	}

	vp_dbg("query success\n");

	return TRUE;
}
