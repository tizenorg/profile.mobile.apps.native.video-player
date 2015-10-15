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


#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <app.h>
#include <stdbool.h>
#include <Ecore_Evas.h>
#include <Elementary.h>

#include <system_settings.h>
#include "mp-util.h"
#include "mp-video-log.h"
#include "mp-external-ug.h"
#include "mp-library-view-mgr.h"
#include "mp-util-media-service.h"
#include "mp-video-string-define.h"
#include "mp-video-list-view-main.h"
#include "mp-video-search-view.h"
#include "videos-view-mgr.h"
#include "mp-rotate-ctrl.h"
#include "mp-video-list-view-as-ctrl.h"
#include "mp-util-preference.h"

typedef enum {
	MP_NONE_VIEW = 0,
	MP_LIBRARY_LIST_VIEW,
	MP_SEARCH_LIST_VIEW,
	MP_MAX_VIEW,
} MpLibraryViewType;

/*static void *pMainLayout = NULL;*/
/*static void *pNaviFrame = NULL;*/
/*static void *pConformant = NULL;*/

void mp_library_mgr_exit_cb(void)
{
	VideoLogInfo("");

	/*///////////////////////////////////////////////////////*/
	/* TO-DO : It needs to include exit call sequence.*/
}

/*//////////////////////////////////////////////////////////////////////////*/
/* External APIs*/

void *mp_library_mgr_init(void)
{
	/*Evas_Object *pConformant = (Evas_Object*)mp_mgr_get_conformant();*/

	/*pMainLayout = MpLibraryViewMgrCreateMainLayout(pConformant);*/

	mp_library_mgr_set_value();

	Evas_Object *pParent = (Evas_Object *) mp_mgr_get_library_parent();
	Evas_Object *pNaviFrame = elm_naviframe_add(pParent);

	evas_object_size_hint_align_set(pNaviFrame, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(pNaviFrame, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	/*elm_object_part_content_set(pParent, "elm.swallow.content", pNaviFrame);*/

	elm_naviframe_prev_btn_auto_pushed_set(pNaviFrame, EINA_FALSE);
	evas_object_show(pNaviFrame);

	mp_list_view_push((void *) pNaviFrame);

	return (void *) pNaviFrame;
}

void mp_library_mgr_destroy(void *pUserData)
{
	VideoLogInfo("");
	if (NULL == pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	mp_list_view_destroy();

	mp_library_mgr_unset_value();

	Evas_Object *pNaviFrame = (Evas_Object *) pUserData;

	if (pNaviFrame) {
		evas_object_del(pNaviFrame);
		/*pNaviFrame = NULL;*/
	}

}

void mp_library_mgr_update_video_list(void)
{

	mp_list_view_update_widget();
}

void mp_library_mgr_update_library_view(MpListUpdateType eUpdateType)
{
	VideoLogInfo("eUpdateType = %d", eUpdateType);

	mp_list_view_rotate_view(eUpdateType);
}

void mp_library_mgr_change_language(void)
{
	VideoLogInfo("");

	char *locale = NULL;
	int retcode =
		system_settings_get_value_string
		(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);

	if (retcode != SYSTEM_SETTINGS_ERROR_NONE) {
		VideoLogInfo
		("[ERR] failed to get updated language!!! [retcode = retcode]",
		 retcode);
	} else {
		if (locale) {
			elm_language_set(locale);
			MP_FREE_STRING(locale);
		}
	}

	mp_list_view_change_language();
}

void mp_library_mgr_change_color(void)
{
	mp_list_view_change_color();
}

void mp_library_mgr_destroy_player(void)
{
	mp_list_view_destroy_player();
}

void mp_library_mgr_set_value(void)
{
	VideoLogInfo("");

	mp_util_svc_init_session();
	mp_util_preference_init();
}

/*it is not necessary to invoke this interface and could use idler to invoke*/
void mp_library_mgr_init_value(void)
{
	VideoLogInfo("");
}

void mp_library_mgr_unset_value(void)
{
	VideoLogInfo("");

	mp_util_delete_popup_handle();
	mp_util_svc_destory_video_list();
	mp_util_svc_finish_session();

	mp_library_mgr_destroy_player();
	mp_rotate_ctrl_destroy();
}

void mp_library_mgr_pause_wall_render(void)
{
	VideoLogInfo("");
#ifdef VS_FEATURE_THUMBNAIL_VIEW	/**/
	int nViewType = mp_view_as_ctrl_get_type();

	if (nViewType == MP_LIST_VIEW_AS_THUMBNAIL_LIST) {
	}
#endif				/**/
}

void mp_library_mgr_resume_wall_render(void)
{
	VideoLogInfo("");
#ifdef VS_FEATURE_THUMBNAIL_VIEW	/**/
	int nViewType = mp_view_as_ctrl_get_type();

	if (nViewType == MP_LIST_VIEW_AS_THUMBNAIL_LIST) {
	}
#endif
}
