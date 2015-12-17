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


#include <app.h>
#include <glib.h>
#include <appcore-common.h>

#include <system_settings.h>
#include "video-player.h"
#include "videos-view-mgr.h"
#include "mp-video-value-define.h"
#include "mp-util.h"
#include "mp-video-log.h"
#include "mp-library-view-mgr.h"
#include "mp-launching-video-displayer.h"
#include "mp-video-sound-manager.h"
#include "mp-video-list-view-main.h"
#include "vp-util.h"

#define VIDEOS_WIN_NAME "videos"

static bool appCreate(void *pUserData)
{
	VideoLogWarning("== APP CREATE ==");

	return TRUE;
}

static void __app_color_theme_changed(void *pUserData)
{
	VideoLogWarning("app_color_theme_changed");

	mp_library_mgr_change_color();
}

static void appCreateInternal()
{
	VideoLogWarning("== APP CREATE INTERNAL ==");

	/* Instead specify h/w acceleration in the application xml. */
#ifdef _USE_OPENGL_BACKEND
	/* elm_config_preferred_engine_set("opengl_x11"); */
#else
	/* elm_config_preferred_engine_set("software_x11"); */
#endif

	mp_util_create_main_theme();
	mp_util_set_color_theme_changed_cb(__app_color_theme_changed);

	mp_mgr_create_main_window(VIDEOS_WIN_NAME);

#ifdef _MULTI_WINDOW
	elm_win_wm_desktop_layout_support_set(
	    (Evas_Object *)mp_mgr_get_main_window(), EINA_TRUE);

	evas_object_data_set((Evas_Object *)mp_mgr_get_main_window(),
	                     "id_startup_by", (void *) - 1);
	evas_object_data_set((Evas_Object *)mp_mgr_get_main_window(),
	                     "id_layout_pos", (void *) - 1);
#endif

	evas_object_show((Evas_Object *)mp_mgr_get_main_window());
	elm_win_activate((Evas_Object *)mp_mgr_get_main_window());

	return;
}

static Eina_Bool appControlPost(void *data)
{
	if (mp_mgr_get_library_naviframe() == NULL) {
		mp_mgr_restart_main_window();
	} else {
		VideoLogInfo("appControlPost");
		/* mp_library_mgr_resume_wall_render(); */
	}
	return TRUE;
}


static void appControl(app_control_h pAppSvcHandle, void *pUserData)
{
	VideoLogWarning("== APP SERVICE ==");
	bool bActiveMainwindow = FALSE;
	if (mp_mgr_get_main_window() == NULL) {
		appCreateInternal();
	} else {
		bActiveMainwindow = TRUE;
	}

	if (mp_mgr_get_main_window() == NULL) {
		VideoLogError("mp_mgr_get_main_window return NULL!!!");
		elm_exit();
		return;
	}

	/*
	* for split window
	*/
#ifdef _MULTI_WINDOW
	if (!pAppSvcHandle) {
		VideoLogInfo("[ERR] No exist pAppSveHandle.");
		return;
	}

	static int is_first_run	= 0;

	char *val_startup = NULL;
	char *val_layout = NULL;
	int id = -1;

	int id_startup_by = (int)evas_object_data_get(
	                        (Evas_Object *)mp_mgr_get_main_window(),
	                        "id_startup_by");
	int id_layout_pos = (int)evas_object_data_get(
	                        (Evas_Object *)mp_mgr_get_main_window(),
	                        "id_layout_pos");

	if (app_control_get_extra_data(pAppSvcHandle, "window_startup_type",
	                               &val_startup) != APP_CONTROL_ERROR_NONE) {
		val_startup = strdup("0");
	}

	if (app_control_get_extra_data(pAppSvcHandle, "window_layout_id",
	                               &val_layout) != APP_CONTROL_ERROR_NONE) {
		val_layout = strdup("-1");
	}

	if (id_startup_by == -1) {
		id = elm_win_aux_hint_add(
		         (Evas_Object *)mp_mgr_get_main_window(),
		         "wm.policy.win.startup.by", val_startup);
		evas_object_data_set((Evas_Object *)mp_mgr_get_main_window(),
		                     "id_startup_by", (void *)id);
	} else {
		elm_win_aux_hint_val_set(
		    (Evas_Object *)mp_mgr_get_main_window(), id_startup_by,
		    val_startup);
	}

	if (id_layout_pos == -1) {
		id = elm_win_aux_hint_add(
		         (Evas_Object *)mp_mgr_get_main_window(),
		         "wm.policy.win.zone.desk.layout.pos", val_layout);
		evas_object_data_set((Evas_Object *)mp_mgr_get_main_window(),
		                     "id_layout_pos", (void *)id);
	} else {
		elm_win_aux_hint_val_set(
		    (Evas_Object *)mp_mgr_get_main_window(), id_layout_pos,
		    val_layout);
	}

	VideoLogInfo("val_startup : [%s]", val_startup);
	VideoLogInfo("val_layout : [%s]", val_layout);
	VideoLogInfo("id_startup_by : [%d]", id_startup_by);
	VideoLogInfo("id_layout_pos : [%d]", id_layout_pos);

	free(val_startup);
	free(val_layout);
	val_startup = NULL;
	val_layout = NULL;

	char *operation = NULL;
	int nRet = app_control_get_operation(pAppSvcHandle, &operation);
	if (nRet == APP_CONTROL_ERROR_NONE &&
	        strcmp(operation,
	               "http://tizen.org/appcontrol/operation/main") == 0) {
		if (is_first_run == 1) {
			elm_win_activate(
			    (Evas_Object *)mp_mgr_get_main_window());

			VideoLogInfo("is_first_run is true");
			/* mp_library_mgr_resume_wall_render(); */

			MP_FREE_STRING(operation);

			return;
		}
	}

	MP_FREE_STRING(operation);

	is_first_run = 1;
#endif

	if (bActiveMainwindow) {
		evas_object_show((Evas_Object *)mp_mgr_get_main_window());
		elm_win_activate((Evas_Object *)mp_mgr_get_main_window());
	}
	/* ecore_idler_add(appControlPost, NULL); */

	appControlPost(NULL);
	return;
}

static void appExit(void *pUserData)
{
	VideoLogWarning("== APP EXIT ==");
	mp_util_free_main_theme();
	mp_util_set_color_theme_changed_cb(NULL);

	mp_sound_mgr_deinit();
}

static void appPause(void *pUserData)
{
	VideoLogWarning("== APP PAUSE ==");

	mp_sound_mgr_deinit();
	/* mp_library_mgr_pause_wall_render(); */

	return;
}

static void appResume(void *pUserData)
{
	VideoLogWarning("== APP RESUME ==");

	/* mp_library_mgr_resume_wall_render(); */
	mp_list_view_update_widget_resume();

	return;
}

static void appUpdateLanguage(app_event_info_h pEventInfo, void *pUserData)
{
	VideoLogWarning("== APP CHANGE LANGUAGE ==");

	mp_library_mgr_change_language();

	char *locale = NULL;
	int retcode = system_settings_get_value_string(
	                  SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);

	if (retcode != SYSTEM_SETTINGS_ERROR_NONE) {
		VideoLogInfo(
		    "[ERR] failed to get updated language[retcode = %d]",
		    retcode);
	} else {
		if (locale) {
			elm_language_set(locale);
			MP_FREE_STRING(locale);
		}
	}
}

EXPORT_API int main(int argc, char *argv[])
{
	struct timeval startTimeVal;
	int nRet = -1;
	gettimeofday(&startTimeVal, NULL);

	VideoLogInfo("======================================================");
	VideoLogInfo(" Measuring exec() launching  time - %ld:%ldus",
	             startTimeVal.tv_sec, startTimeVal.tv_usec);
	VideoLogInfo("======================================================");

	ui_app_lifecycle_callback_s st_appEventService;
	app_event_handler_h hLanguageChangedHandle = NULL;

	nRet = ui_app_add_event_handler(&hLanguageChangedHandle,
	                                APP_EVENT_LANGUAGE_CHANGED, appUpdateLanguage, NULL);
	if (nRet != APP_ERROR_NONE) {
		VideoLogError("Fail to add handler for LANGUAGE_CHANGED [%d]",
		              nRet);
		return -1;
	}

	st_appEventService.create = appCreate;
	st_appEventService.terminate = appExit;
	st_appEventService.pause = appPause;
	st_appEventService.resume = appResume;
	st_appEventService.app_control = appControl;
	/*
		st_appEventService.low_memory = appBattery;
		st_appEventService.low_battery = NULL;
		st_appEventService.device_orientation = NULL; //appRotate;
		st_appEventService.language_changed = appUpdateLanguage;
		st_appEventService.region_format_changed = NULL;
	*/
	nRet = ui_app_main(argc, argv, &st_appEventService, NULL);
	if (nRet != APP_ERROR_NONE) {
		VideoLogInfo("[ERR] app_efl_main().");
		return -1;
	}

	VideoLogInfo("escape ui_app_main() loop.");

	return 0;
}

