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

#ifndef  __VIDEO_MP_UTIL_DB_CONTROLLER__
#define  __VIDEO_MP_UTIL_DB_CONTROLLER__

#include <stdbool.h>
#include <Elementary.h>

typedef enum
{
	MP_DB_UPDATE_LEV_0			= 0,
	MP_DB_UPDATE_LEV_1,
	MP_DB_UPDATE_LEV_2,
	MP_DB_UPDATE_LEV_3,
	MP_DB_UPDATE_LEV_4,
	MP_DB_UPDATE_LEV_MAX,
}MpDbUpdateViewLev;


typedef void (*MpDbUpdateCbFunc)(void *pUserData);


bool mp_util_db_set_update_fun(MpDbUpdateViewLev nLev, MpDbUpdateCbFunc pUpdateFun);
void mp_util_db_reset_update_fun_except_lev0(void);
void mp_util_db_reset_all_update_fun(void);
void mp_util_db_reset_update_fun_up_self(MpDbUpdateViewLev nLev);
bool mp_util_db_run_update_fun(MpDbUpdateViewLev nLev, void *pUserData);
bool mp_util_db_run_all_update_fun(void *pUserData0, void *pUserData1, void *pUserData2, void *pUserData3, void *pUserData4);
bool mp_util_db_run_update_fun_except_lev0(void *pUserData1, void *pUserData2, void *pUserData3, void *pUserData4);

void mp_util_db_reset_backup_fun_up_self(MpDbUpdateViewLev nLev);
void mp_util_db_reset_backup_fun_except_lev0(void);
void mp_util_db_reset_all_backup_fun(void);
bool mp_util_db_set_backup_fun(MpDbUpdateViewLev nLev, MpDbUpdateCbFunc pUpdateFun);
bool mp_util_db_run_backup_fun_except_lev0(void *pUserData1, void *pUserData2, void *pUserData3, void *pUserData4);
bool mp_util_db_run_all_backup_fun(void *pUserData0, void *pUserData1, void *pUserData2, void *pUserData3, void *pUserData4);


#endif

