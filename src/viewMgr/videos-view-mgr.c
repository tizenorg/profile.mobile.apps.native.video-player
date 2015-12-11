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


#include <stdbool.h>

#include "videos-view-mgr.h"
#include "mp-video-value-define.h"
#include "mp-video-type-define.h"

#include "mp-util.h"
#include "mp-video-log.h"
#include "mp-library-view-mgr.h"
#include "mp-rotate-ctrl.h"

#include "vp-util.h"


enum DeviceOrientationType {
	DEVICE_ORIENTATION_PORTRAIT = 0,
	DEVICE_ORIENTATION_LANDSCAPE,
	DEVICE_ORIENTATION_MAX
};


typedef struct _vp_WindowData {
	Evas_Object *pMainWindow;
	Evas_Object *pConformant;
	Evas_Object *pMainLayout;

	Evas_Object *pLibraryInnerNaviframe;
	Ecore_Idler *pInitValueIdler;

	/*landcape */
	app_device_orientation_e mOrientation;
} VpWindowData;


static VpWindowData *g_WindowData = NULL;


static int __mp_mgr_get_device_orientation_type()
{
	if (g_WindowData
			&& (g_WindowData->mOrientation == APP_DEVICE_ORIENTATION_270
			    || g_WindowData->mOrientation == APP_DEVICE_ORIENTATION_90)) {
		return DEVICE_ORIENTATION_LANDSCAPE;
	} else {
		return DEVICE_ORIENTATION_PORTRAIT;
	}
}


#ifdef ENABLE_LANDSCAPE
static void
__mp_mgr_main_window_rotation_change_cb(void *data, Evas_Object *obj,
					void *event_info)
{
	int changed_angle = elm_win_rotation_get(obj);

	VideoLogWarning("window rotated [%d] => [%d]",
			g_WindowData->mOrientation, changed_angle);
	if (g_WindowData->mOrientation != changed_angle) {
		g_WindowData->mOrientation = changed_angle;

		VideoLogWarning("Rotate phone");
		mp_rotate_ctrl_set_state(changed_angle);
		mp_library_mgr_update_library_view(LIST_UPDATE_TYPE_VIEW);
	}
}
#endif


static void __mp_mgr_window_destroy()
{
	VideoLogInfo("");

	if (g_WindowData != NULL) {
		MP_DEL_IDLER(g_WindowData->pInitValueIdler);
		free(g_WindowData);
		g_WindowData = NULL;
	}
}

static void
__mp_mgr_main_win_del_cb(void *data, Evas_Object *obj,
			 void *mouse_event_rect)
{
	VideoLogInfo("");

	if (NULL == data || data != g_WindowData) {
		VideoLogError(" data[%p] == NULL || data != g_WindowData[%p]!!!",
			      data, g_WindowData);
		return;
	}

	__mp_mgr_window_destroy();

	elm_exit();
}

static Evas_Object *__mp_mgr_create_bg(Evas_Object *pWin)
{
	VideoLogInfo("");

	if (NULL == pWin) {
		VideoLogError("pWin == NULL!!!");
		return NULL;
	}

	/* default BG */
	Evas_Object *bg = NULL;

	bg = elm_bg_add(pWin);
	Eina_Bool ret = elm_bg_file_set(bg,
		"/usr/apps/org.tizen.videos/res/images/core_theme_bg_01.png",
		NULL);
	if (ret != true) {
		VideoLogInfo("failed to set layout bg");
	}
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(bg, EVAS_HINT_FILL,
					 EVAS_HINT_FILL);
	elm_win_resize_object_add(pWin, bg);
	evas_object_show(bg);

	return bg;
}

static Evas_Object *__mp_mgr_create_win(const char *name)
{
	VideoLogInfo("");

	Evas_Object *eo;
	int w;
	int h;

	eo = elm_win_add(NULL, name, ELM_WIN_BASIC);

	if (eo) {
		elm_win_title_set(eo, name);
		evas_object_smart_callback_add(eo, "delete,request",
					       __mp_mgr_main_win_del_cb,
					       g_WindowData);

		elm_win_screen_size_get(eo, NULL, NULL, &w, &h);
		VideoLogInfo("window size:%d,%d", w, h);

		evas_object_resize(eo, w, h);
		elm_win_indicator_mode_set(eo, ELM_WIN_INDICATOR_SHOW);
		elm_win_indicator_opacity_set(eo, ELM_WIN_INDICATOR_TRANSPARENT);
	}

	return eo;
}


const Evas_Object *mp_mgr_create_main_window(const char *pStrName)
{
	if (g_WindowData != NULL) {
		VideoLogWarning("g_WindowData != NULL. destory g_WindowData");
		__mp_mgr_window_destroy();
	}

	g_WindowData = calloc(1, sizeof(VpWindowData));
	if (NULL == g_WindowData) {
		VideoLogError("g_WindowData calloc failed!!!");
		return NULL;
	}

	g_WindowData->pMainWindow = __mp_mgr_create_win(pStrName);


#ifdef ENABLE_LANDSCAPE
	VideoLogWarning("support rotate",
			elm_win_wm_rotation_supported_get(g_WindowData->
					pMainWindow));
	if (elm_win_wm_rotation_supported_get(g_WindowData->pMainWindow)) {
		const int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(g_WindowData->
				pMainWindow, rots, 4);
		evas_object_smart_callback_add(g_WindowData->pMainWindow,
					       "wm,rotation,changed",
					       __mp_mgr_main_window_rotation_change_cb,
					       g_WindowData);
		g_WindowData->mOrientation =
			elm_win_rotation_get(g_WindowData->pMainWindow);
		VideoLogWarning("mOrientation=%d", g_WindowData->mOrientation);
	}
#endif

	/*      evas_object_show(g_WindowData->pMainWindow);*/
	elm_win_conformant_set(g_WindowData->pMainWindow, EINA_TRUE);


	/* create bg */
	__mp_mgr_create_bg(g_WindowData->pMainWindow);


	/* create conformant */
	g_WindowData->pConformant =
		elm_conformant_add(g_WindowData->pMainWindow);


	evas_object_size_hint_weight_set(g_WindowData->pConformant,
					 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(g_WindowData->pMainWindow,
				  g_WindowData->pConformant);
	evas_object_show(g_WindowData->pConformant);
	elm_object_signal_emit(g_WindowData->pConformant,
			       "elm,state,indicator,overlap", "");
	evas_object_data_set(g_WindowData->pConformant, "overlap",
			     (void *) EINA_TRUE);

	mp_util_set_main_conformat((void *) g_WindowData->pConformant);
	mp_util_set_main_window_handle((void *) g_WindowData->pMainWindow);

	return (const Evas_Object *) g_WindowData->pMainWindow;
}

static Eina_Bool __mp_mgr_init_data_idler_cb(void *pUserData)
{
	if (!g_WindowData) {
		VideoLogError("");
		return ECORE_CALLBACK_CANCEL;
	}

	g_WindowData->pInitValueIdler = NULL;

	mp_library_mgr_init_value();

	return ECORE_CALLBACK_CANCEL;
}


void mp_mgr_restart_main_window()
{
	if (NULL == g_WindowData) {
		VideoLogError("g_WindowData == NULL!!!");
		elm_exit();
		return;
	}

	if (g_WindowData->pLibraryInnerNaviframe != NULL) {
		VideoLogInfo("g_WindowData->pLibraryInnerNaviframe != NULL");
		mp_library_mgr_destroy(g_WindowData->pLibraryInnerNaviframe);
		/*pLibraryInnerNaviframe has been detroyed in mp_library_mgr_destroy */
		g_WindowData->pLibraryInnerNaviframe = NULL;
	}

	if (g_WindowData->pMainLayout != NULL) {
		evas_object_del(g_WindowData->pMainLayout);
		g_WindowData->pMainLayout = NULL;
	}

	g_WindowData->pMainLayout = elm_layout_add(g_WindowData->pConformant);
	if (g_WindowData->pMainLayout == NULL) {
		VideoLogError("elm_layout_add fail!!!");
		elm_exit();
		return;
	}
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_BASIC_LAYOUT_EDJ);
	free(path);
	elm_layout_file_set(g_WindowData->pMainLayout, edj_path,
			    "elm/layout/application/video/default");
	evas_object_size_hint_weight_set(g_WindowData->pMainLayout,
					 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_WindowData->pMainLayout,
					EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(g_WindowData->pMainLayout);
	elm_object_content_set(g_WindowData->pConformant,
			       g_WindowData->pMainLayout);

	if (g_WindowData->pLibraryInnerNaviframe != NULL) {
		evas_object_del(g_WindowData->pLibraryInnerNaviframe);
	}
	g_WindowData->pLibraryInnerNaviframe = mp_library_mgr_init();
	MP_DEL_IDLER(g_WindowData->pInitValueIdler);
	g_WindowData->pInitValueIdler =
		ecore_idler_add(__mp_mgr_init_data_idler_cb,
				(void *) g_WindowData);

	elm_object_part_content_set(g_WindowData->pMainLayout,
				    "elm.swallow.content",
				    g_WindowData->pLibraryInnerNaviframe);

	eext_object_event_callback_add(g_WindowData->pLibraryInnerNaviframe,
				       EEXT_CALLBACK_BACK,
				       eext_naviframe_back_cb, NULL);
	eext_object_event_callback_add(g_WindowData->pLibraryInnerNaviframe,
				       EEXT_CALLBACK_MORE,
				       eext_naviframe_more_cb, NULL);

	mp_util_hide_indicator();
}


const Evas_Object *mp_mgr_get_main_window()
{
	if (NULL == g_WindowData) {
		VideoLogWarning("g_WindowData == NULL!!!");
		return NULL;
	}

	return (const Evas_Object *) g_WindowData->pMainWindow;
}


const Evas_Object *mp_mgr_get_conformant()
{
	if (NULL == g_WindowData) {
		VideoLogError("g_WindowData == NULL!!!");
		return NULL;
	}

	return (const Evas_Object *) g_WindowData->pConformant;
}


const Evas_Object *mp_mgr_get_library_naviframe()
{

	if (NULL == g_WindowData) {
		VideoLogError("g_WindowData == NULL!!!");
		return NULL;
	}

	return (const Evas_Object *) g_WindowData->pLibraryInnerNaviframe;
}


const Evas_Object *mp_mgr_get_current_inner_naviframe()
{
	return (const Evas_Object *) g_WindowData->pLibraryInnerNaviframe;
}


const Evas_Object *mp_mgr_get_main_layout()
{
	VideoLogInfo("");

	if (NULL == g_WindowData) {
		VideoLogError("g_WindowData == NULL!!!");
		return NULL;
	}

	return (const Evas_Object *) g_WindowData->pMainLayout;
}


Eina_Bool mp_mgr_is_landscape()
{
	return (__mp_mgr_get_device_orientation_type() ==
		DEVICE_ORIENTATION_LANDSCAPE);
}


const Evas_Object *mp_mgr_get_library_parent()
{
	if (g_WindowData == NULL) {
		VideoLogError("g_WindowData == NULL!!!");
		return NULL;
	}

	return mp_mgr_get_main_layout();
}
