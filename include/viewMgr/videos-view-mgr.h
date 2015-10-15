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


#ifndef _VIDEOS_VIEW_MGR_
#define _VIDEOS_VIEW_MGR_


#include <Evas.h>
#include <stdlib.h>
#include <stdbool.h>

#include <Elementary.h>


#ifdef _cplusplus
extern "C"
{
#endif


const Evas_Object* mp_mgr_create_main_window( const char *pStrName );
void mp_mgr_restart_main_window();
const Evas_Object *mp_mgr_get_main_window();
const Evas_Object *mp_mgr_get_conformant();
const Evas_Object *mp_mgr_get_main_layout();

Eina_Bool mp_mgr_is_landscape();

const Evas_Object* mp_mgr_get_library_naviframe();
const Evas_Object* mp_mgr_get_current_inner_naviframe();

const Evas_Object* mp_mgr_get_library_parent();


#ifdef _cplusplus
}
#endif


#endif // _VIDEOS_VIEW_MGR_

