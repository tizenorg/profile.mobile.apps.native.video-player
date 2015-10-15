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

#include "mp-util.h"
#ifdef ENABLE_DRM_FEATURE
#include "mp-drm-ctrl.h"
#endif
#include "mp-video-log.h"
#include "mp-video-value-define.h"
#include "mp-video-string-define.h"
#include "mp-video-util-db-controller.h"

/*used after mediadb update*/
static MpDbUpdateCbFunc g_pDbUpdateFun[MP_DB_UPDATE_LEV_MAX] = {NULL, NULL, 
								NULL, NULL, 
								NULL };
/*used before mediadb update*/
static MpDbUpdateCbFunc g_pDbBackupFun[MP_DB_UPDATE_LEV_MAX] = {NULL, NULL, 
								NULL, NULL, 
								NULL };

bool mp_util_db_set_update_fun(MpDbUpdateViewLev nLev,
			       MpDbUpdateCbFunc pUpdateFun)
{
	if ((nLev < MP_DB_UPDATE_LEV_0) || (nLev >= MP_DB_UPDATE_LEV_MAX)) {
		VideoLogError("invalid lev = %d", nLev);
		return FALSE;
	}
	g_pDbUpdateFun[nLev] = pUpdateFun;
	return TRUE;
}

void mp_util_db_reset_update_fun_up_self(MpDbUpdateViewLev nLev)
{
	MpDbUpdateViewLev nIndex = nLev;
	while (nIndex < MP_DB_UPDATE_LEV_MAX) {
		g_pDbUpdateFun[nIndex] = NULL;
		nIndex++;
	}
}

void mp_util_db_reset_update_fun_except_lev0(void)
{
	g_pDbUpdateFun[MP_DB_UPDATE_LEV_1] = NULL;
	g_pDbUpdateFun[MP_DB_UPDATE_LEV_2] = NULL;
	g_pDbUpdateFun[MP_DB_UPDATE_LEV_3] = NULL;
	g_pDbUpdateFun[MP_DB_UPDATE_LEV_4] = NULL;
}

void mp_util_db_reset_all_update_fun(void)
{
	g_pDbUpdateFun[MP_DB_UPDATE_LEV_0] = NULL;
	g_pDbUpdateFun[MP_DB_UPDATE_LEV_1] = NULL;
	g_pDbUpdateFun[MP_DB_UPDATE_LEV_2] = NULL;
	g_pDbUpdateFun[MP_DB_UPDATE_LEV_3] = NULL;
	g_pDbUpdateFun[MP_DB_UPDATE_LEV_4] = NULL;
}

bool mp_util_db_run_update_fun(MpDbUpdateViewLev nLev, void *pUserData)
{
	if ((nLev < MP_DB_UPDATE_LEV_0) || (nLev >= MP_DB_UPDATE_LEV_MAX)) {
		VideoLogError("invalid lev = %d", nLev);
		return FALSE;
	}
	VideoLogInfo("update lev = %d", nLev);

	if (g_pDbUpdateFun[nLev]) {
		g_pDbUpdateFun[nLev](pUserData);
	}
	return FALSE;
}


bool mp_util_db_run_all_update_fun(void *pUserData0, void *pUserData1,
				   void *pUserData2, void *pUserData3,
				   void *pUserData4)
{
	if (g_pDbUpdateFun[MP_DB_UPDATE_LEV_0]) {
		g_pDbUpdateFun[MP_DB_UPDATE_LEV_0](pUserData0);
	}
	if (g_pDbUpdateFun[MP_DB_UPDATE_LEV_1]) {
		g_pDbUpdateFun[MP_DB_UPDATE_LEV_1](pUserData1);
	}
	if (g_pDbUpdateFun[MP_DB_UPDATE_LEV_2]) {
		g_pDbUpdateFun[MP_DB_UPDATE_LEV_2](pUserData2);
	}
	if (g_pDbUpdateFun[MP_DB_UPDATE_LEV_3]) {
		g_pDbUpdateFun[MP_DB_UPDATE_LEV_3](pUserData3);
	}
	if (g_pDbUpdateFun[MP_DB_UPDATE_LEV_4]) {
		g_pDbUpdateFun[MP_DB_UPDATE_LEV_4](pUserData4);
	}
	VideoLogInfo("update successfully");

	return TRUE;
}

bool mp_util_db_run_update_fun_except_lev0(void *pUserData1,
		void *pUserData2,
		void *pUserData3,
		void *pUserData4)
{
	if (g_pDbUpdateFun[MP_DB_UPDATE_LEV_1]) {
		g_pDbUpdateFun[MP_DB_UPDATE_LEV_1](pUserData1);
	}
	if (g_pDbUpdateFun[MP_DB_UPDATE_LEV_2]) {
		g_pDbUpdateFun[MP_DB_UPDATE_LEV_2](pUserData2);
	}
	if (g_pDbUpdateFun[MP_DB_UPDATE_LEV_3]) {
		g_pDbUpdateFun[MP_DB_UPDATE_LEV_3](pUserData3);
	}
	if (g_pDbUpdateFun[MP_DB_UPDATE_LEV_4]) {
		g_pDbUpdateFun[MP_DB_UPDATE_LEV_4](pUserData4);
	}
	VideoLogInfo("update successfully");

	return TRUE;
}


void mp_util_db_reset_backup_fun_up_self(MpDbUpdateViewLev nLev)
{
	MpDbUpdateViewLev nIndex = nLev;
	while (nIndex < MP_DB_UPDATE_LEV_MAX) {
		g_pDbBackupFun[nIndex] = NULL;
		nIndex++;
	}
}

void mp_util_db_reset_backup_fun_except_lev0(void)
{
	g_pDbBackupFun[MP_DB_UPDATE_LEV_1] = NULL;
	g_pDbBackupFun[MP_DB_UPDATE_LEV_2] = NULL;
	g_pDbBackupFun[MP_DB_UPDATE_LEV_3] = NULL;
	g_pDbBackupFun[MP_DB_UPDATE_LEV_4] = NULL;
}

void mp_util_db_reset_all_backup_fun(void)
{
	g_pDbBackupFun[MP_DB_UPDATE_LEV_0] = NULL;
	g_pDbBackupFun[MP_DB_UPDATE_LEV_1] = NULL;
	g_pDbBackupFun[MP_DB_UPDATE_LEV_2] = NULL;
	g_pDbBackupFun[MP_DB_UPDATE_LEV_3] = NULL;
	g_pDbBackupFun[MP_DB_UPDATE_LEV_4] = NULL;
}


bool mp_util_db_set_backup_fun(MpDbUpdateViewLev nLev,
			       MpDbUpdateCbFunc pUpdateFun)
{
	if ((nLev < MP_DB_UPDATE_LEV_0) || (nLev >= MP_DB_UPDATE_LEV_MAX)) {
		VideoLogError("invalid lev = %d", nLev);
		return FALSE;
	}
	g_pDbBackupFun[nLev] = pUpdateFun;
	return TRUE;
}

bool mp_util_db_run_backup_fun_except_lev0(void *pUserData1,
		void *pUserData2,
		void *pUserData3,
		void *pUserData4)
{
	if (g_pDbBackupFun[MP_DB_UPDATE_LEV_1]) {
		g_pDbBackupFun[MP_DB_UPDATE_LEV_1](pUserData1);
	}
	if (g_pDbBackupFun[MP_DB_UPDATE_LEV_2]) {
		g_pDbBackupFun[MP_DB_UPDATE_LEV_2](pUserData2);
	}
	if (g_pDbBackupFun[MP_DB_UPDATE_LEV_3]) {
		g_pDbBackupFun[MP_DB_UPDATE_LEV_3](pUserData3);
	}
	if (g_pDbBackupFun[MP_DB_UPDATE_LEV_4]) {
		g_pDbBackupFun[MP_DB_UPDATE_LEV_4](pUserData4);
	}
	VideoLogInfo("backup successfully");

	return TRUE;
}

bool mp_util_db_run_all_backup_fun(void *pUserData0, void *pUserData1,
				   void *pUserData2, void *pUserData3,
				   void *pUserData4)
{
	if (g_pDbBackupFun[MP_DB_UPDATE_LEV_0]) {
		g_pDbBackupFun[MP_DB_UPDATE_LEV_0](pUserData0);
	}
	if (g_pDbBackupFun[MP_DB_UPDATE_LEV_1]) {
		g_pDbBackupFun[MP_DB_UPDATE_LEV_1](pUserData1);
	}
	if (g_pDbBackupFun[MP_DB_UPDATE_LEV_2]) {
		g_pDbBackupFun[MP_DB_UPDATE_LEV_2](pUserData2);
	}
	if (g_pDbBackupFun[MP_DB_UPDATE_LEV_3]) {
		g_pDbBackupFun[MP_DB_UPDATE_LEV_3](pUserData3);
	}
	if (g_pDbBackupFun[MP_DB_UPDATE_LEV_4]) {
		g_pDbBackupFun[MP_DB_UPDATE_LEV_4](pUserData4);
	}
	VideoLogInfo("backup successfully");

	return TRUE;
}
