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
#include <errno.h>
#include <vconf.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <shortcut.h>
#include <media_key.h>
#include <vconf-keys.h>
#include <Elementary.h>
#include <appcore-common.h>

#include <notification.h>
#include <media_content.h>
#include <metadata_extractor.h>
#include <app_preference.h>

#include <sys/statvfs.h>

#include <runtime_info.h>
/*#include <telephony_network.h>*/
#include <net_connection.h>
#include <telephony.h>
#include <wifi-direct.h>

#include "mp-util.h"
#include "mp-video-log.h"
#include "mp-util-media-service.h"
#include "mp-video-value-define.h"
#include "mp-video-string-define.h"
#include "mp-video-info-ctrl.h"
#ifdef ENABLE_DRM_FEATURE
#include "vp-drm.h"
#endif
#include "vp-util.h"
#include "vp-file-util.h"

enum VIDEO_PLAYER_FILE_SIZE_TYPE {
	SIZE_BYTE = 0,
	SIZE_KB,
	SIZE_MB,
	SIZE_GB
};


#define MP_UTIL_PHONE_PATH 						"/opt/usr/media"
#define MP_UTIL_SD_PATH							"/opt/storage/sdcard"
#define MP_UTIL_USB_PATH						"/opt/storage/usb"
#define MP_UTIL_MEDIA_DIR						"/opt/usr/media/Videos"

#define MP_UTIL_FILE_PREFIX						"file://"

#define FILE_SIZE_LEN_MAX						64
#define BASIC_SIZE 								1024	/*used for file size check*/

#define VIDEO_TIME_MSEC_PER_SEC					1000
#define VIDEO_TIME_SEC_PER_MIN					60
#define VIDEO_TIME_MIN_PER_HOUR					60
#define VIDEO_TIME_SEC_PER_HOUR					(VIDEO_TIME_MIN_PER_HOUR * VIDEO_TIME_SEC_PER_MIN)

#define PREF_MP_UTIL_MAIN_TAB_TYPE  "preference/org.tizen.videos/main_display_tab"


static Evas_Object *pPopUpHandle = NULL;
static Evas_Object *pMainWindowHandle = NULL;
static Evas_Object *pMainConformant = NULL;

mpColorThemeChangedCb g_fThemeChangedCb = NULL;

void mp_util_set_color_theme_changed_cb(mpColorThemeChangedCb fChanged)
{
	g_fThemeChangedCb = fChanged;
}

void mp_util_create_main_theme(void)
{
	mp_util_free_main_theme();
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_CUSTOM_THEME);
	free(path);
	elm_theme_extension_add(NULL, edj_path);
}

void mp_util_free_main_theme(void)
{
	const Eina_List *pExtensionThemeList = NULL;
	Eina_List *l = NULL;
	char *pItem = NULL;

	pExtensionThemeList = elm_theme_extension_list_get(NULL);
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_CUSTOM_THEME);
	free(path);
	if (pExtensionThemeList) {
		EINA_LIST_FOREACH((Eina_List *) pExtensionThemeList, l, pItem) {
			if (pItem != NULL && strcmp(pItem, edj_path) == 0) {
				elm_theme_extension_del(NULL, edj_path);
			}
		}
	}
}

int mp_util_get_main_tab_type()
{
	int nType = 0;

	if (preference_get_int(PREF_MP_UTIL_MAIN_TAB_TYPE, &nType) !=
			PREFERENCE_ERROR_NONE) {
		VideoLogError("[ERR] PREF_MP_UTIL_MAIN_TAB_TYPE fail!!!");
	}

	return nType;
}

void mp_util_set_main_tab_type(int nType)
{
	if (preference_set_int(PREF_MP_UTIL_MAIN_TAB_TYPE, nType) != PREFERENCE_ERROR_NONE) {
		VideoLogError("[ERR] PREF_MP_UTIL_MAIN_TAB_TYPE fail!!!");
	}
}

void mp_util_reset_for_relauching(void)
{
	pPopUpHandle = NULL;
	pMainWindowHandle = NULL;
	pMainConformant = NULL;
}

void mp_util_show_indicator(void)
{
	if (!pMainConformant || !pMainWindowHandle) {
		VideoLogError
		("[ERR] No exist pMainConformant, pMainWindowHandle.");
		return;
	}

	elm_win_indicator_mode_set((Evas_Object *) pMainWindowHandle,
				   ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set((Evas_Object *) pMainWindowHandle,
				      ELM_WIN_INDICATOR_OPAQUE);
	elm_object_signal_emit((Evas_Object *) pMainConformant,
			       "elm,state,indicator,nooverlap", "");
	evas_object_data_set((Evas_Object *) pMainConformant, "overlap",
			     NULL);
}

void mp_util_hide_indicator(void)
{
	if (!pMainConformant || !pMainWindowHandle) {
		VideoLogError
		("[ERR] No exist pMainConformant, pMainWindowHandle.");
		return;
	}

	elm_win_indicator_mode_set((Evas_Object *) pMainWindowHandle,
				   ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set((Evas_Object *) pMainWindowHandle,
				      ELM_WIN_INDICATOR_TRANSPARENT);
	elm_object_signal_emit((Evas_Object *) pMainConformant,
			       "elm,state,indicator,overlap", "");
	evas_object_data_set((Evas_Object *) pMainConformant, "overlap",
			     (void *) EINA_TRUE);
}

void mp_util_set_main_window_handle(void *pWindowHandle)
{
	pMainWindowHandle = pWindowHandle;
}

void mp_util_set_main_conformat(void *pConformant)
{
	pMainConformant = pConformant;
}

void *mp_util_get_main_conformat(void)
{

	if (!pMainConformant) {
		VideoLogError("[ERR] No exist pMainConformant.");
		return NULL;
	}

	return pMainConformant;
}

void *mp_util_get_main_window_handle(void)
{

	if (!pMainWindowHandle) {
		VideoLogError("[ERR] No exist main window handle.");
		return NULL;
	}

	return pMainWindowHandle;
}

void *mp_util_create_title_icon_btn(void *pParent, const char *icon_path,
				    Evas_Smart_Cb pFunc, void *pUserData)
{
	if (!pParent || !icon_path) {
		VideoLogError("invalid parent and icon path");
		return NULL;
	}
	Evas_Object *pIcon = NULL;
	Evas_Object *pTitleBtn = NULL;
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_PLAYER_IMAGE_NAME_EDJ);
	free(path);
	pIcon = elm_image_add((Evas_Object *) pParent);
	elm_image_file_set(pIcon, edj_path, icon_path);
	evas_object_size_hint_aspect_set(pIcon, EVAS_ASPECT_CONTROL_BOTH, 1,
					 1);

	pTitleBtn = elm_button_add((Evas_Object *) pParent);
	elm_object_style_set(pTitleBtn, "naviframe/title_icon");
	elm_object_part_content_set(pTitleBtn, "icon", pIcon);
	evas_object_smart_callback_add(pTitleBtn, "clicked", pFunc,
				       pUserData);

	return pTitleBtn;
}

/*///////////////////////////////////////*/
/* Popup utility*/
void mp_util_delete_popup_handle(void)
{

	if (pPopUpHandle) {
		evas_object_del(pPopUpHandle);
		pPopUpHandle = NULL;
	} else {
		VideoLogInfo("No exist popup.");
	}
}

static void __mp_util_popup_back_cb(void *data, Evas_Object *pObj,
				    void *event_info)
{
	mp_util_delete_popup_handle();
}

bool mp_util_notify_popup(void *pUserData, char *szTitle,
			  char *szContents, void *PopupButtonCallbackFunc)
{
	if (!szContents) {
		VideoLogError("No have popup message is null.");
		return FALSE;
	}
	mp_util_delete_popup_handle();

	pPopUpHandle = elm_popup_add(mp_util_get_main_window_handle());
	eext_object_event_callback_add(pPopUpHandle, EEXT_CALLBACK_BACK,
				       PopupButtonCallbackFunc, NULL);
	evas_object_smart_callback_add(pPopUpHandle, "block,clicked",
				       __mp_util_popup_back_cb,
				       (void *) pUserData);

	evas_object_size_hint_weight_set(pPopUpHandle, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_object_text_set(pPopUpHandle, szContents);
	if (szTitle && (szTitle == VIDEOS_COMMON_HEADER_ERROR)) {
		elm_object_domain_translatable_part_text_set(pPopUpHandle,
				"title,text",
				MP_SYS_STRING,
				szTitle);
	}
	elm_popup_timeout_set(pPopUpHandle, 3.0);

	if (PopupButtonCallbackFunc) {
		evas_object_smart_callback_add(pPopUpHandle, "timeout",
					       PopupButtonCallbackFunc,
					       pUserData);
	} else {
		evas_object_smart_callback_add(pPopUpHandle, "timeout",
					       __mp_util_popup_back_cb,
					       pUserData);
	}
	evas_object_show(pPopUpHandle);

	return TRUE;
}

bool mp_util_info_popup(char *szLabel, Evas_Smart_Cb response_cb,
			Evas_Event_Cb focus_cb, void *pUserData,
			const char *szDomain)
{
	VideoLogInfo("");

	if (!szLabel) {
		VideoLogError("No have popup message or pUserData is null.");
		return FALSE;
	}

	mp_util_delete_popup_handle();

	pPopUpHandle = elm_popup_add(mp_util_get_main_window_handle());

	eext_object_event_callback_add(pPopUpHandle, EEXT_CALLBACK_BACK,
				       response_cb, pUserData);

	Evas_Object *obj_ok = elm_button_add(pPopUpHandle);
	elm_object_style_set(obj_ok, "popup");
	elm_object_domain_translatable_text_set(obj_ok, MP_SYS_STRING,
						VIDEOS_COMMON_BUTTON_OK_IDS);
	elm_object_part_content_set(pPopUpHandle, "button1", obj_ok);
	evas_object_smart_callback_add(obj_ok, "clicked", response_cb,
				       pPopUpHandle);

	if (szLabel) {
		/*elm_object_part_text_set(popup, "title,text", title);*/

		elm_object_domain_translatable_text_set(pPopUpHandle, szDomain,
							szLabel);
	}

	evas_object_show(pPopUpHandle);

	return TRUE;
}

bool mp_util_info_second_popup(Evas_Object *pParent, char *szLabel,
			       Evas_Smart_Cb response_cb,
			       Evas_Event_Cb focus_cb, void *pUserData,
			       const char *szDomain)
{
	VideoLogInfo("");

	if (!szLabel) {
		VideoLogError("No have popup message or pUserData is null.");
		return FALSE;
	}

	mp_util_delete_popup_handle();

	pPopUpHandle = elm_popup_add(pParent);

	eext_object_event_callback_add(pPopUpHandle, EEXT_CALLBACK_BACK,
				       response_cb, pUserData);

	Evas_Object *obj_ok = elm_button_add(pPopUpHandle);
	elm_object_style_set(obj_ok, "popup");
	elm_object_domain_translatable_text_set(obj_ok, MP_SYS_STRING,
						VIDEOS_COMMON_BUTTON_OK_IDS);
	elm_object_part_content_set(pPopUpHandle, "button1", obj_ok);
	evas_object_smart_callback_add(obj_ok, "clicked", response_cb,
				       pPopUpHandle);

	if (szLabel) {
		elm_object_domain_translatable_text_set(pPopUpHandle, szDomain,
							szLabel);
	}

	evas_object_show(pPopUpHandle);

	return TRUE;
}

void mp_util_text_and_check_popup(Evas_Smart_Cb response_cb,
				  void *cancel_cb, void *ok_cb,
				  void *check_changed_cb, char *szLabel,
				  void *pUserData)
{
	if (!szLabel) {
		VideoLogError("No have popup message or pUserData is null.");
		return;
	}
	Evas_Object *pCheck = NULL;
	Evas_Object *pLayout = NULL;
	Evas_Object *pLabel = NULL;
	Evas_Object *pBtn1 = NULL;
	Evas_Object *pBtn2 = NULL;

	mp_util_delete_popup_handle();

	pPopUpHandle = elm_popup_add(mp_util_get_main_window_handle());

	eext_object_event_callback_add(pPopUpHandle, EEXT_CALLBACK_BACK,
				       response_cb, pUserData);

	pLabel = elm_label_add(pPopUpHandle);
	elm_label_line_wrap_set(pLabel, ELM_WRAP_MIXED);
	elm_object_text_set(pLabel, szLabel);
	evas_object_size_hint_weight_set(pLabel, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(pLabel, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	evas_object_show(pLabel);
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_PLAYER_POPUP_PROGRESSBAR_EDJ);
	free(path);
	pLayout = elm_layout_add(pPopUpHandle);
	elm_layout_file_set(pLayout, edj_path,
			    "popup_checkview");
	evas_object_size_hint_weight_set(pLayout, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);

	pCheck = elm_check_add(pPopUpHandle);
	elm_object_style_set(pCheck, "multiline");
	/*elm_object_domain_translatable_text_set(pCheck, VIDEOS_STRING, MP_PERSONAL_DONT_ASK_AGAIN);*/
	evas_object_smart_callback_add(pCheck, "changed", check_changed_cb,
				       pUserData);
	elm_check_state_set(pCheck, FALSE);
	elm_object_text_set(pCheck, MP_PERSONAL_DONT_ASK_AGAIN);
	evas_object_size_hint_align_set(pCheck, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(pCheck, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_show(pCheck);

	elm_object_part_content_set(pLayout, "elm.swallow.content", pLabel);
	elm_object_part_content_set(pLayout, "elm.swallow.end", pCheck);

	evas_object_show(pLayout);
	elm_object_content_set(pPopUpHandle, pLayout);

	pBtn1 = elm_button_add(pPopUpHandle);
	elm_object_style_set(pBtn1, "popup");
	elm_object_text_set(pBtn1, VIDEOS_COMMON_BUTTON_CANCEL);
	elm_object_part_content_set(pPopUpHandle, "button1", pBtn1);
	evas_object_smart_callback_add(pBtn1, "clicked", cancel_cb,
				       pPopUpHandle);

	pBtn2 = elm_button_add(pPopUpHandle);
	elm_object_style_set(pBtn2, "popup");
	elm_object_text_set(pBtn2, VIDEOS_COMMON_BUTTON_OK);
	elm_object_part_content_set(pPopUpHandle, "button2", pBtn2);
	evas_object_smart_callback_add(pBtn2, "clicked", ok_cb, pPopUpHandle);
	evas_object_show(pPopUpHandle);
}

void *mp_util_get_current_popup_object()
{
	return (void *) pPopUpHandle;
}

bool mp_util_loading_popup(MpLoadingPopUpType style, char *szLabel,
			   Evas_Smart_Cb response_cb, void *pUserData)
{
	VideoLogInfo("");
	Evas_Object *pLayout = NULL;
	Evas_Object *pCancelButton = NULL;

	if (!szLabel) {
		VideoLogError("No have popup message or pUserData is null.");
		return FALSE;
	}

	mp_util_delete_popup_handle();

	pPopUpHandle = elm_popup_add(mp_util_get_main_window_handle());

	eext_object_event_callback_add(pPopUpHandle, EEXT_CALLBACK_BACK,
				       response_cb, pUserData);

	evas_object_size_hint_weight_set(pPopUpHandle, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);

	Evas_Object *progressbar = NULL;

	progressbar = elm_progressbar_add(pPopUpHandle);
	elm_object_style_set(progressbar, "process_large");
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	evas_object_show(progressbar);
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_PLAYER_POPUP_PROGRESSBAR_EDJ);
	free(path);
	pLayout = elm_layout_add(pPopUpHandle);
	elm_layout_file_set(pLayout, edj_path,
			    "popup_processing_1button");
	evas_object_size_hint_weight_set(pLayout, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);

	elm_object_part_content_set(pLayout, "elm.swallow.content",
				    progressbar);
	elm_object_part_text_set(pLayout, "elm.text", szLabel);

	elm_object_content_set(pPopUpHandle, pLayout);
	if (style == POPUP_PROCESSING_ONE_BUTTON) {
		pCancelButton = elm_button_add(pPopUpHandle);
		elm_object_style_set(pCancelButton, "popup");
		elm_object_domain_translatable_text_set(pCancelButton,
							VIDEOS_STRING,
							VIDEOS_COMMON_BUTTON_CANCEL_IDS);
		elm_object_part_content_set(pPopUpHandle, "button1",
					    pCancelButton);
		evas_object_smart_callback_add(pCancelButton, "clicked",
					       response_cb, pPopUpHandle);
		evas_object_show(pCancelButton);
	}

	evas_object_show(pPopUpHandle);

	return TRUE;
}

bool mp_util_loading_left_text_popup(MpLoadingPopUpType style,
				     char *szLabel,
				     Evas_Smart_Cb response_cb,
				     void *pUserData)
{
	VideoLogInfo("");
	Evas_Object *pLayout = NULL;
	Evas_Object *pCancelButton = NULL;

	if (!szLabel) {
		VideoLogError("No have popup message or pUserData is null.");
		return FALSE;
	}

	mp_util_delete_popup_handle();

	pPopUpHandle = elm_popup_add(mp_util_get_main_window_handle());

	eext_object_event_callback_add(pPopUpHandle, EEXT_CALLBACK_BACK,
				       response_cb, pUserData);

	evas_object_size_hint_weight_set(pPopUpHandle, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);

	Evas_Object *progressbar = NULL;

	progressbar = elm_progressbar_add(pPopUpHandle);
	elm_object_style_set(progressbar, "process_large");
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	evas_object_show(progressbar);
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_PLAYER_POPUP_PROGRESSBAR_EDJ);
	free(path);
	pLayout = elm_layout_add(pPopUpHandle);
	elm_layout_file_set(pLayout, edj_path,
			    "popup_processingview_1button");
	evas_object_size_hint_weight_set(pLayout, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);

	elm_object_part_content_set(pLayout, "elm.swallow.content",
				    progressbar);
	elm_object_part_text_set(pLayout, "elm.text", szLabel);

	elm_object_content_set(pPopUpHandle, pLayout);
	if (style == POPUP_PROCESSING_ONE_BUTTON) {
		pCancelButton = elm_button_add(pPopUpHandle);
		elm_object_style_set(pCancelButton, "popup");
		elm_object_domain_translatable_text_set(pCancelButton,
							VIDEOS_STRING,
							VIDEOS_COMMON_BUTTON_CANCEL_IDS);
		elm_object_part_content_set(pPopUpHandle, "button1",
					    pCancelButton);
		evas_object_smart_callback_add(pCancelButton, "clicked",
					       response_cb, pPopUpHandle);
		evas_object_show(pCancelButton);
	}

	evas_object_show(pPopUpHandle);

	return TRUE;
}

bool mp_util_yes_no_popup(void *pUserData, char *szTitle,
			  char *szContents,
			  void *PopupYesButtonCallbackFunc,
			  void *PopupNoButtonCallbackFunc)
{

	if (!szTitle || !szContents) {
		VideoLogError("No have popup message or pUserData is null.");
		return FALSE;
	}

	Evas_Object *pButton = NULL;

	mp_util_delete_popup_handle();

	pPopUpHandle = elm_popup_add(mp_util_get_main_window_handle());

	eext_object_event_callback_add(pPopUpHandle, EEXT_CALLBACK_BACK,
				       PopupNoButtonCallbackFunc, NULL);

	evas_object_size_hint_weight_set(pPopUpHandle, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_object_text_set(pPopUpHandle, szContents);
	elm_object_part_text_set(pPopUpHandle, "title,text", szTitle);

	pButton = elm_button_add(pPopUpHandle);
	elm_object_style_set(pButton, "popup");
	elm_object_domain_translatable_text_set(pButton, MP_SYS_STRING,
						VIDEOS_COMMON_BUTTON_YES_IDS);
	elm_object_part_content_set(pPopUpHandle, "button1", pButton);

	if (PopupYesButtonCallbackFunc) {
		evas_object_smart_callback_add(pButton, "clicked",
					       PopupYesButtonCallbackFunc,
					       pUserData);
	}

	pButton = elm_button_add(pPopUpHandle);
	elm_object_domain_translatable_text_set(pButton, MP_SYS_STRING,
						VIDEOS_COMMON_BUTTON_NO_IDS);
	elm_object_style_set(pButton, "popup");
	elm_object_part_content_set(pPopUpHandle, "button2", pButton);

	if (PopupNoButtonCallbackFunc) {
		evas_object_smart_callback_add(pButton, "clicked",
					       PopupNoButtonCallbackFunc,
					       pUserData);
	}
	/*elm_popup_timeout_set(pPopUpHandle, 3.0);*/

	evas_object_show(pPopUpHandle);

	return TRUE;
}

bool mp_util_remove_all_videos_question_popup(void *pUserData,
		MpListViewAsType viewType,
		void
		*PopupDeleteButtonCallbackFunc,
		void
		*PopupCancelButtonCallbackFunc,
		void
		*PopupHWMouseBackUpCbFunc,
		void *PopupHWKeyBackCbFunc)
{
	Evas_Object *pButton = NULL;

	char *tmpTitle = NULL;
	char *tmpStr = NULL;

	mp_util_delete_popup_handle();

	pPopUpHandle = elm_popup_add(mp_util_get_main_window_handle());

	eext_object_event_callback_add(pPopUpHandle, EEXT_CALLBACK_BACK,
				       PopupCancelButtonCallbackFunc, NULL);

	evas_object_size_hint_weight_set(pPopUpHandle, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);

	tmpTitle = g_strdup_printf(VIDEOS_DELETEPOP_HEADER_DELETE);
	elm_object_part_text_set(pPopUpHandle, "title,text", tmpTitle);

	if (viewType == MP_LIST_VIEW_AS_FOLDER_LIST) {
		VideoLogInfo("All folders are selected to delete");
		tmpStr =
			g_strdup_printf
			(VIDEOS_DELETEPOP_MSG_ALL_FOLDERS_WILL_BE_DELETED);
	} else {
		VideoLogInfo("All videos are selected to delete");
		tmpStr =
			g_strdup_printf
			(VIDEOS_DELETEPOP_MSG_ALL_VIDEOS_WILL_BE_DELETED);
	}

	elm_object_part_text_set(pPopUpHandle, "default", tmpStr);

	pButton = elm_button_add(pPopUpHandle);
	elm_object_domain_translatable_text_set(pButton, VIDEOS_STRING,
						VIDEOS_COMMON_BUTTON_CANCEL);
	elm_object_style_set(pButton, "popup");
	elm_object_part_content_set(pPopUpHandle, "button1", pButton);

	if (PopupCancelButtonCallbackFunc) {
		evas_object_smart_callback_add(pButton, "clicked",
					       PopupCancelButtonCallbackFunc,
					       pUserData);
	}

	pButton = elm_button_add(pPopUpHandle);

	elm_object_domain_translatable_text_set(pButton, VIDEOS_STRING,
						VIDEOS_DELETEPOP_BUTTON_DELETE_ALL);

	elm_object_style_set(pButton, "popup");
	elm_object_part_content_set(pPopUpHandle, "button2", pButton);

	if (PopupDeleteButtonCallbackFunc) {
		evas_object_smart_callback_add(pButton, "clicked",
					       PopupDeleteButtonCallbackFunc,
					       pUserData);
	}

	if (PopupHWMouseBackUpCbFunc) {
		evas_object_event_callback_add(pPopUpHandle,
					       EVAS_CALLBACK_MOUSE_UP,
					       PopupHWMouseBackUpCbFunc,
					       pUserData);
	}

	if (PopupHWKeyBackCbFunc) {
		/*evas_object_event_callback_add(pPopUpHandle, EVAS_CALLBACK_KEY_DOWN, PopupHWKeyBackCbFunc, pUserData);*/
	}
	/*elm_popup_timeout_set(pPopUpHandle, 3.0);*/

	evas_object_show(pPopUpHandle);

	return TRUE;
}


bool mp_util_remove_question_popup(int pUserData,
				   MpListViewAsType viewType,
				   void *PopupDeleteButtonCallbackFunc,
				   void *PopupCancelButtonCallbackFunc,
				   void *PopupHWMouseBackUpCbFunc,
				   void *PopupHWKeyBackCbFunc)
{
	Evas_Object *pButton = NULL;

	int nSelected = (int) pUserData;
	char *tmpTitle = NULL;
	char *tmpStr = NULL;

	mp_util_delete_popup_handle();

	pPopUpHandle = elm_popup_add(mp_util_get_main_window_handle());

	eext_object_event_callback_add(pPopUpHandle, EEXT_CALLBACK_BACK,
				       PopupCancelButtonCallbackFunc, NULL);

	evas_object_size_hint_weight_set(pPopUpHandle, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);

	tmpTitle = g_strdup_printf(VIDEOS_DELETEPOP_HEADER_DELETE);
	elm_object_part_text_set(pPopUpHandle, "title,text", tmpTitle);

	if (nSelected < -1) {	/* select videos*/
		if (viewType == MP_LIST_VIEW_AS_FOLDER_LIST) {
			VideoLogInfo("%d folders are selected to delete", -nSelected);
			tmpStr =
				g_strdup_printf
				(VIDEOS_DELETEPOP_MSG_PD_FOLDERS_WILL_BE_DELETED,
				 -nSelected);
		} else {
			VideoLogInfo("%d videos are selected to delete", -nSelected);
			tmpStr =
				g_strdup_printf
				(VIDEOS_DELETEPOP_MSG_PD_VIDEOS_WILL_BE_DELETED,
				 -nSelected);
		}
	} else {
		/* select a video*/
		if (viewType == MP_LIST_VIEW_AS_FOLDER_LIST) {
			VideoLogInfo("The folder is selected to delete. Index: %d.",
				     nSelected);
			tmpStr =
				g_strdup_printf
				(VIDEOS_DELETEPOP_MSG_THIS_FOLDER_WILL_BE_DELETED_IDS);
		} else {
			VideoLogInfo("The video is selected to delete. Index: %d.",
				     nSelected);
			tmpStr =
				g_strdup_printf
				(VIDEOS_DELETEPOP_MSG_ONE_VIDEO_WILL_BE_DELETED);
		}
	}
	elm_object_part_text_set(pPopUpHandle, "default", tmpStr);

	pButton = elm_button_add(pPopUpHandle);
	elm_object_domain_translatable_text_set(pButton, VIDEOS_STRING,
						VIDEOS_COMMON_BUTTON_CANCEL);
	elm_object_style_set(pButton, "popup");
	elm_object_part_content_set(pPopUpHandle, "button1", pButton);

	if (PopupCancelButtonCallbackFunc) {
		evas_object_smart_callback_add(pButton, "clicked",
					       PopupCancelButtonCallbackFunc,
					       (void *)pUserData);
	}

	pButton = elm_button_add(pPopUpHandle);

	elm_object_domain_translatable_text_set(pButton, VIDEOS_STRING,
						VIDEOS_DELETEPOP_BUTTON_DELETE);

	elm_object_style_set(pButton, "popup");
	elm_object_part_content_set(pPopUpHandle, "button2", pButton);

	if (PopupDeleteButtonCallbackFunc) {
		evas_object_smart_callback_add(pButton, "clicked",
					       PopupDeleteButtonCallbackFunc,
					       (void *)pUserData);
	}

	if (PopupHWMouseBackUpCbFunc) {
		evas_object_event_callback_add(pPopUpHandle,
					       EVAS_CALLBACK_MOUSE_UP,
					       PopupHWMouseBackUpCbFunc,
					       (void *)pUserData);
	}

	if (PopupHWKeyBackCbFunc) {
		/*evas_object_event_callback_add(pPopUpHandle, EVAS_CALLBACK_KEY_DOWN, PopupHWKeyBackCbFunc, (void *)pUserData);*/
	}
	/*elm_popup_timeout_set(pPopUpHandle, 3.0);*/

	evas_object_show(pPopUpHandle);

	return TRUE;
}

bool mp_util_remove_folder_question_popup(void *pUserData,
		void *PopupDeleteButtonCallbackFunc,
		void *PopupCancelButtonCallbackFunc,
		void *PopupHWMouseBackUpCbFunc,
		void *PopupHWKeyBackCbFunc)
{
	Evas_Object *pButton = NULL;

	mp_util_delete_popup_handle();

	pPopUpHandle = elm_popup_add(mp_util_get_main_window_handle());

	eext_object_event_callback_add(pPopUpHandle, EEXT_CALLBACK_BACK,
				       PopupCancelButtonCallbackFunc, NULL);

	evas_object_size_hint_weight_set(pPopUpHandle, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);

	elm_object_part_text_set(pPopUpHandle, "title,text", VIDEOS_DELETEPOP_HEADER_DELETE);
	elm_object_domain_translatable_text_set(pPopUpHandle, VIDEOS_STRING, VIDEOS_DELETEPOP_MSG_THIS_FOLDER_WILL_BE_DELETED_IDS);

	pButton = elm_button_add(pPopUpHandle);
	elm_object_domain_translatable_text_set(pButton, VIDEOS_STRING,
						VIDEOS_COMMON_BUTTON_CANCEL_IDS);
	elm_object_style_set(pButton, "popup_button/default");
	elm_object_part_content_set(pPopUpHandle, "button1", pButton);

	if (PopupCancelButtonCallbackFunc) {
		evas_object_smart_callback_add(pButton, "clicked",
					       PopupCancelButtonCallbackFunc,
					       pUserData);
	}

	pButton = elm_button_add(pPopUpHandle);
	elm_object_domain_translatable_text_set(pButton, VIDEOS_STRING,
						VIDEOS_DELETEPOP_BUTTON_DELETE);
	elm_object_style_set(pButton, "popup_button/default");
	elm_object_part_content_set(pPopUpHandle, "button2", pButton);

	if (PopupDeleteButtonCallbackFunc) {
		evas_object_smart_callback_add(pButton, "clicked",
					       PopupDeleteButtonCallbackFunc,
					       pUserData);
	}

	if (PopupHWMouseBackUpCbFunc) {
		evas_object_event_callback_add(pPopUpHandle,
					       EVAS_CALLBACK_MOUSE_UP,
					       PopupHWMouseBackUpCbFunc,
					       pUserData);
	}

	if (PopupHWKeyBackCbFunc) {
		/*evas_object_event_callback_add(pPopUpHandle, EVAS_CALLBACK_KEY_DOWN, PopupHWKeyBackCbFunc, pUserData);*/
	}

	evas_object_show(pPopUpHandle);

	return TRUE;
}


bool mp_util_download_question_popup(void *pUserData, char *szTitle,
				     char *szContents,
				     void *PopupCancelBtnCallbackFunc,
				     void *PopupDownloadBtnCallbackFunc)
{
	Evas_Object *pButton = NULL;

	mp_util_delete_popup_handle();

	pPopUpHandle = elm_popup_add(mp_util_get_main_window_handle());

	evas_object_size_hint_weight_set(pPopUpHandle, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);

	if (szContents)
		elm_object_text_set(pPopUpHandle, szContents);

	if (szTitle)
		elm_object_part_text_set(pPopUpHandle, "title,text", szTitle);

	pButton = elm_button_add(pPopUpHandle);
	elm_object_domain_translatable_text_set(pButton, VIDEOS_STRING,
						VIDEOS_COMMON_BUTTON_CANCEL_IDS);
	elm_object_style_set(pButton, "popup");
	elm_object_part_content_set(pPopUpHandle, "button1", pButton);

	if (PopupCancelBtnCallbackFunc) {
		evas_object_smart_callback_add(pButton, "clicked",
					       PopupCancelBtnCallbackFunc,
					       pUserData);
		eext_object_event_callback_add(pPopUpHandle, EEXT_CALLBACK_BACK,
					       PopupCancelBtnCallbackFunc, NULL);
	}

	pButton = elm_button_add(pPopUpHandle);
	elm_object_domain_translatable_text_set(pButton, VIDEOS_STRING,
						VIDEOS_DELETEPOP_BUTTON_DELETE);
	elm_object_style_set(pButton, "popup");
	elm_object_part_content_set(pPopUpHandle, "button2", pButton);

	if (PopupDownloadBtnCallbackFunc) {
		evas_object_smart_callback_add(pButton, "clicked",
					       PopupDownloadBtnCallbackFunc,
					       pUserData);
	}

	evas_object_show(pPopUpHandle);

	return TRUE;
}

void mp_util_set_label_for_detail(Evas_Object *pBox, char *szString)
{

	Evas_Object *pLabel = NULL;
	if (!pBox) {
		VideoLogError("error pBox");
		return;
	}
	pLabel = elm_label_add(pBox);
	elm_object_style_set(pLabel, "popup/default");
	elm_object_text_set(pLabel, szString);
	/*elm_label_ellipsis_set(pLabel, EINA_TRUE);*/
	elm_label_line_wrap_set(pLabel, EINA_TRUE);
	elm_label_wrap_width_set(pLabel,
				 VIDEO_POPUP_DETAIL_ITEM_W *
				 elm_config_scale_get());
	evas_object_size_hint_weight_set(pLabel, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_size_hint_min_set(pLabel, VIDEO_POPUP_DETAIL_ITEM_W,
				      35 * elm_config_scale_get());

	elm_box_pack_end(pBox, pLabel);
	evas_object_show(pLabel);

/*
	pLabel = elm_entry_add(pPopUpHandle);
	elm_entry_editable_set(pLabel, EINA_FALSE);
	elm_object_focus_set(pLabel, EINA_FALSE);
	elm_object_disabled_set(pLabel, EINA_TRUE);

	elm_object_text_set(pLabel, szString);
	evas_object_size_hint_weight_set(pLabel, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(pLabel, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(pBox, pLabel);
	evas_object_show(pLabel);
*/
}

bool mp_util_progress_popup(char *szLabel)
{
	VideoLogInfo("");

	if (!szLabel) {
		VideoLogError("No have popup message or pUserData is null.");
		return FALSE;
	}

	mp_util_delete_popup_handle();

	pPopUpHandle = elm_popup_add(mp_util_get_main_window_handle());

	eext_object_event_callback_add(pPopUpHandle, EEXT_CALLBACK_BACK,
				       __mp_util_popup_back_cb, NULL);

	evas_object_size_hint_weight_set(pPopUpHandle, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	Evas_Object *progressbar, *box, *label;

	label = elm_label_add(pPopUpHandle);
	elm_object_text_set(label, szLabel);
	evas_object_show(label);

	progressbar = elm_progressbar_add(pPopUpHandle);
	elm_object_style_set(progressbar, "pending_list");
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	evas_object_show(progressbar);

	box = elm_box_add(pPopUpHandle);
	elm_box_pack_end(box, label);
	elm_box_pack_end(box, progressbar);
	evas_object_show(box);
	elm_object_content_set(pPopUpHandle, box);

	evas_object_show(pPopUpHandle);

	return TRUE;
}

void mp_util_status_bar_noti_popup_show(const char *szStr)
{
	if (!szStr) {
		VideoLogInfo("szStr is NULL.");
		return;
	}

	int nRet = notification_status_message_post(szStr);

	if (nRet != 0) {
		VideoLogInfo("notification_status_message_post() : [0x%x]", nRet);
	}
}

void *mp_util_create_button_icon(void *pParent, const char *szIconPath,
				 bool bSizeUp, bool bSizeDown,
				 int nAspectWidth, int nAspectHeight,
				 void *pUserData, char *szButtonStyle,
				 void *func)
{
	Evas_Object *pButton = NULL;
	Evas_Object *pIcon = NULL;

	pButton = elm_button_add(pParent);
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_PLAYER_IMAGE_NAME_EDJ);
	free(path);
	if (szIconPath) {
		pIcon = elm_icon_add(pParent);
		elm_image_file_set(pIcon, edj_path,
				   szIconPath);
		evas_object_size_hint_aspect_set(pIcon,
						 EVAS_ASPECT_CONTROL_NEITHER,
						 nAspectWidth, nAspectHeight);
	}

	if (szButtonStyle) {
		elm_object_style_set(pButton, szButtonStyle);
	}

	if (pIcon) {
		elm_object_part_content_set(pButton, PART_BUTTON_ICON, pIcon);
	}

	if (func) {
		evas_object_smart_callback_add(pButton, "clicked", func,
					       (void *) pUserData);
	}

	elm_object_focus_allow_set(pButton, EINA_FALSE);
	evas_object_size_hint_weight_set(pButton, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_show(pButton);

	return (void *) pButton;
}

void *mp_util_create_style_button(void *pParent, const char *szButtonText,
				  const void *pUserData,
				  char *szButtonStyle, void *func)
{
	Evas_Object *pButton = NULL;

	pButton = elm_button_add(pParent);

	if (szButtonStyle) {
		elm_object_style_set(pButton, szButtonStyle);
	}

	if (szButtonText) {
		elm_object_text_set(pButton, szButtonText);
	}

	if (func) {
		evas_object_smart_callback_add(pButton, "clicked", func,
					       pUserData);
	}

	elm_object_focus_allow_set(pButton, EINA_FALSE);
	evas_object_size_hint_weight_set(pButton, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_show(pButton);

	return (void *) pButton;
}

bool mp_util_call_off(void)
{
	telephony_call_state_e state = TELEPHONY_CALL_STATE_IDLE;
	telephony_handle_list_s tel_list;
	int tel_valid = telephony_init(&tel_list);
	if (tel_valid != TELEPHONY_ERROR_NONE) {
		VideoLogError("telephony is not initialized. ERROR Code is %d",tel_valid);
		return TRUE;
	}

	telephony_h *newhandle = tel_list.handle;
	int s = telephony_call_get_voice_call_state(*newhandle , &state );
	telephony_deinit(&tel_list);

	if (s == TELEPHONY_ERROR_NONE) {
		if (state == TELEPHONY_CALL_STATE_IDLE) {
			return TRUE;
		} else {
			return FALSE;
		}
	} else {
		VideoLogError("ERROR: state error is %d",s);
	}
	return TRUE;
}

bool mp_util_check_local_file_exist(char *uri)
{
	if (!uri) {
		VideoLogError("[ERR]");
		return FALSE;
	}

	if (strstr(uri, MP_UTIL_FILE_PREFIX)) {
		if (!g_file_test
				(uri + strlen(MP_UTIL_FILE_PREFIX),
				 G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
			return FALSE;
		}
		return TRUE;
	} else {
		if (!g_file_test
				(uri, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
			return FALSE;
		}
		return TRUE;
	}
}

char *mp_util_get_file_size(unsigned long long size)
{
	int nCount = 0;

	char *pTmp = (char *)calloc(1, sizeof(char) * FILE_SIZE_LEN_MAX + 1);
	if (!pTmp) {
		return NULL;
	}

	double nSize_t = 0;

	if (size >= (BASIC_SIZE)) {
		nSize_t = (double) size / BASIC_SIZE;
		nCount++;
	}

	while (nSize_t >= (BASIC_SIZE)) {
		nSize_t /= BASIC_SIZE;
		nCount++;
	}

	if (nCount == SIZE_BYTE) {
		snprintf(pTmp, FILE_SIZE_LEN_MAX, "%0.1lf%s", nSize_t,
			 VIDEOS_DETAILVIEW_BODY_B);
	} else if (nCount == SIZE_KB) {
		snprintf(pTmp, FILE_SIZE_LEN_MAX, "%0.1lf%s", nSize_t,
			 VIDEOS_DETAILVIEW_BODY_KB);
	} else if (nCount == SIZE_MB) {
		snprintf(pTmp, FILE_SIZE_LEN_MAX, "%0.1lf%s", nSize_t,
			 VIDEOS_DETAILVIEW_BODY_MB);
	} else if (nCount == SIZE_GB) {
		snprintf(pTmp, FILE_SIZE_LEN_MAX, "%0.1lf%s", nSize_t,
			 VIDEOS_DETAILVIEW_BODY_GB);
	} else {
		snprintf(pTmp, FILE_SIZE_LEN_MAX, "%zu%s", (size_t)0,
			 VIDEOS_DETAILVIEW_BODY_MB);
	}

	return pTmp;
}

bool mp_util_get_file_title_from_file_path(char *szFilepath,
		char *szTitle,
		int nLengthTitle)
{
	if (!szFilepath || !szTitle) {
		VideoLogError("[ERR]");
		return FALSE;
	}

	char *szTitleName = NULL;
	char *szExt = NULL;

	memset(szTitle, 0, nLengthTitle);

	szTitleName = strrchr(szFilepath, '/');

	if (szTitleName) {
		szTitleName++;
	} else {
		szTitleName = szFilepath;
	}

	szExt = strrchr(szTitleName, '.');

	if (szExt) {
		int nTmpStringSize = szExt - szTitleName;
		strncpy(szTitle, szTitleName, nTmpStringSize);
	} else {
		strncpy(szTitle, szTitleName, STR_LEN_MAX - 1);
	}

	return TRUE;
}

char *mp_util_get_file_name_from_file_path(const char *szFilePath)
{
	if (!szFilePath) {
		VideoLogError("File Path is NULL");
		return NULL;
	}
	char *szFile = NULL;
	char *szTitleName = NULL;
	int nLength = 0;

	nLength = strlen(szFilePath);

	if (nLength <= 0) {
		VideoLogError("Invalid buffer length");
		return NULL;
	}

	szTitleName = strrchr(szFilePath, '/');

	if (szTitleName) {
		szTitleName++;
	} else {
		szTitleName = (char *) szFilePath;
	}
	if (!szTitleName) {
		VideoLogError("Invalid buffer");
		return NULL;

	}
	szFile = calloc(1, strlen(szTitleName) + 1);
	if (szFile) {
		strncpy(szFile, szTitleName, strlen(szTitleName));
	}

	return szFile;
}

char *mp_util_get_display_name_from_path_and_title(const char *szFilePath,
		const char *szTitle)
{
	if (!szFilePath || !szTitle) {
		VideoLogError("File Path is NULL");
		return NULL;
	}
	char *szDisplayName = NULL;
	char *szExt = NULL;
	int nLength = 0;

	szExt = strrchr(szFilePath, '.');
	if (szExt) {
		nLength = strlen(szTitle) + strlen(szExt) + 1;
	} else {
		nLength = strlen(szTitle) + 1;
	}

	szDisplayName = calloc(1, nLength);
	if (szDisplayName) {
		if (szExt) {
			snprintf(szDisplayName, nLength, "%s%s", szTitle,
					szExt);
		} else {
			snprintf(szDisplayName, nLength, "%s", szTitle);
		}
	}
	return szDisplayName;
}

bool mp_util_get_icon_path_from_thumbnail(const char *szMediaURL,
		char **szIconURL)
{
	if (!szMediaURL) {
		VideoLogError("szMediaURL is NULL");
		return FALSE;
	}

	if (strlen(szMediaURL) < 1) {
		VideoLogError("szMediaURL is invalid Length");
		return FALSE;
	}

	*szIconURL = NULL;
	char *szExt = strrchr(szMediaURL, '.');
	int nLen = 0;
	if (szExt) {
		nLen = strlen(szExt);
	}

	if (nLen == 4) {
		if (vp_file_exists(szMediaURL)) {
			VideoLogWarning("Success finding icon file. - %s",
					szMediaURL);
			*szIconURL = strdup(szMediaURL);
		}
		return TRUE;
	}

	int nCount = 0;
	int nStrLength = 0;
	int nOutputLen = 0;

	char szExtWildkey[3][5] = { ".jpg", ".png", ".bmp" };
	char *szCheckFilePath = NULL;

	nOutputLen = strlen(szMediaURL) + 5;
	szCheckFilePath = calloc(1, sizeof(char)*nOutputLen);
	if (szCheckFilePath) {
		nStrLength = strlen(szMediaURL);

		for (nCount = 0; nCount < 3; nCount++) {
			memset(szCheckFilePath, 0, nOutputLen);
			strncpy(szCheckFilePath, szMediaURL, nStrLength);
			strncat(szCheckFilePath, szExtWildkey[nCount], 5);

			VideoLogWarning("%s", szCheckFilePath);

			if (vp_file_exists(szCheckFilePath)) {
				VideoLogWarning("Success finding icon file. - %d, %s", nCount,
						szCheckFilePath);
				*szIconURL = strdup(szCheckFilePath);
				break;
			}
		}
	}
	if (szCheckFilePath) {
		free(szCheckFilePath);
		szCheckFilePath = NULL;
	}

	return TRUE;
}

void mp_util_convert_time(int nVideoTime, char *szRetTime,
			  bool bIsHideHour)
{
	if (!szRetTime) {
		VideoLogError("[ERR] No exist szRetTime pointer.");
		return;
	} else {
		memset(szRetTime, 0, STR_LEN_MAX);
	}

	if (nVideoTime <= 0) {
		VideoLogInfo("nVideoTime : %d", nVideoTime);
	}

	int nTime = floor(nVideoTime / VIDEO_TIME_MSEC_PER_SEC);
	int nHour = 0;
	int nMin = 0;
	int nSec = 0;
	int nTmp = 0;

	if (nTime >= VIDEO_TIME_SEC_PER_HOUR) {
		nSec = nTime % VIDEO_TIME_SEC_PER_MIN;
		nTmp = floor(nTime / VIDEO_TIME_SEC_PER_MIN);
		nMin = nTmp % VIDEO_TIME_MIN_PER_HOUR;
		nHour = floor(nTmp / VIDEO_TIME_MIN_PER_HOUR);
	} else if (nTime >= VIDEO_TIME_SEC_PER_MIN) {
		nHour = 0;
		nMin = floor(nTime / VIDEO_TIME_SEC_PER_MIN);
		nSec = nTime % VIDEO_TIME_SEC_PER_MIN;
	} else {
		nHour = 0;
		nMin = 0;
		nSec = nTime % VIDEO_TIME_SEC_PER_MIN;
	}

	if (bIsHideHour) {
		if (nHour) {
			snprintf(szRetTime, STR_LEN_MAX - 1, "%02d:%02d:%02d", nHour,
				 nMin, nSec);
		} else {
			snprintf(szRetTime, STR_LEN_MAX - 1, "00:%02d:%02d", nMin,
				 nSec);
		}
	} else {
		snprintf(szRetTime, STR_LEN_MAX - 1, "%02d:%02d:%02d", nHour,
			 nMin, nSec);
	}

	szRetTime[strlen(szRetTime)] = '\0';
}

void mp_util_convert_sec_time(int nVideoTime, char *szRetTime,
			      bool bIsHideHour)
{
	if (!szRetTime) {
		VideoLogError("[ERR] No exist szRetTime pointer.");
		return;
	} else {
		memset(szRetTime, 0, STR_LEN_MAX);
	}

	if (nVideoTime <= 0) {
		VideoLogInfo("nVideoTime : %d", nVideoTime);
	}

	int nTime = nVideoTime;
	int nHour = 0;
	int nMin = 0;
	int nSec = 0;
	int nTmp = 0;

	if (nTime >= VIDEO_TIME_SEC_PER_HOUR) {
		nSec = nTime % VIDEO_TIME_SEC_PER_MIN;
		nTmp = floor(nTime / VIDEO_TIME_SEC_PER_MIN);
		nMin = nTmp % VIDEO_TIME_MIN_PER_HOUR;
		nHour = floor(nTmp / VIDEO_TIME_MIN_PER_HOUR);
	} else if (nTime >= VIDEO_TIME_SEC_PER_MIN) {
		nHour = 0;
		nMin = floor(nTime / VIDEO_TIME_SEC_PER_MIN);
		nSec = nTime % VIDEO_TIME_SEC_PER_MIN;
	} else {
		nHour = 0;
		nMin = 0;
		nSec = nTime % VIDEO_TIME_SEC_PER_MIN;
	}

	if (bIsHideHour) {
		if (nHour) {
			snprintf(szRetTime, STR_LEN_MAX - 1, "%02d:%02d:%02d", nHour,
				 nMin, nSec);
		} else {
			snprintf(szRetTime, STR_LEN_MAX - 1, "%02d:%02d", nMin, nSec);
		}
	} else {
		snprintf(szRetTime, STR_LEN_MAX - 1, "%02d:%02d:%02d", nHour,
			 nMin, nSec);
	}

	szRetTime[strlen(szRetTime)] = '\0';

}

int mp_util_get_pid(void)
{
	return syscall(__NR_gettid);
}

const char *mp_util_str_str_same_case(const char *str, const char *subStr)
{
	if (!str || !subStr) {
		return NULL;
	}

	int len = strlen(subStr);
	const char *strcpy = str;
	if (len == 0) {
		return NULL;
	}

	while (*strcpy) {
		if (strncasecmp(strcpy, subStr, len) == 0) {
			return strcpy;
		}
		++strcpy;
	}
	return NULL;
}

void mp_util_toast_popup_clicked_cb(void *pUserData,
				    Evas_Object *pObject,
				    void *event_info)
{
	evas_object_del(pObject);
}

void mp_util_toast_popup_timeout_cb(void *pUserData,
				    Evas_Object *pObject,
				    void *event_info)
{
	evas_object_del(pObject);
}

bool mp_util_ticker_toast_popup(const char *text, bool info_style,
				bool bottom)
{
	if (!text) {
		return false;
	}

	/*************************************************************************************
	 *	app control infomaiton
	 *	Key	Value				Description
	 *	0	"default", "info" 	Tickernoti style, (default: "default")
	 *	1	"text"				Text to use for the tickernoti description
	 *	2	"0", "1"			Orientation of tickernoti. (0: top, 1:bottom, default: top)
	 *	3	"1", "2", �� ,"999"	Timeout (1: 1 second, default: -1 means infinite)
	 *************************************************************************************/
	/*
		int	nRet	= 0;

		nRet	= notification_status_message_post(text);
		if (nRet != NOTIFICATION_ERROR_NONE) {
			VideoLogError("# tickernoti [%s] error : [%d]#", text, nRet);
			return false;
		}
	*/
	Evas_Object *pToastPopup = NULL;
	Evas_Object *pWindow = mp_util_get_main_window_handle();

	pToastPopup = elm_popup_add(pWindow);
	elm_popup_align_set(pToastPopup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	evas_object_size_hint_weight_set(pToastPopup, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	eext_object_event_callback_add(pToastPopup, EEXT_CALLBACK_BACK,
				       eext_popup_back_cb, NULL);
	elm_object_text_set(pToastPopup, text);
	evas_object_smart_callback_add(pToastPopup, "block,clicked",
				       mp_util_toast_popup_clicked_cb,
				       pWindow);
	elm_popup_timeout_set(pToastPopup, 2.0);
	evas_object_smart_callback_add(pToastPopup, "timeout",
				       mp_util_toast_popup_timeout_cb, NULL);

	evas_object_show(pToastPopup);

	return true;
}

bool mp_util_ticker_toast_with_timeout_cb_popup(const char *text,
		Evas_Smart_Cb
		timeout_func)
{
	if (!text) {
		return false;
	}

	Evas_Object *pToastPopup = NULL;
	Evas_Object *pWindow = mp_util_get_main_window_handle();

	pToastPopup = elm_popup_add(pWindow);
	elm_popup_align_set(pToastPopup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	evas_object_size_hint_weight_set(pToastPopup, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	eext_object_event_callback_add(pToastPopup, EEXT_CALLBACK_BACK,
				       eext_popup_back_cb, NULL);
	elm_object_text_set(pToastPopup, text);
	evas_object_smart_callback_add(pToastPopup, "block,clicked",
				       mp_util_toast_popup_clicked_cb,
				       pWindow);
	elm_popup_timeout_set(pToastPopup, 2.0);
	evas_object_smart_callback_add(pToastPopup, "timeout", timeout_func,
				       NULL);

	evas_object_show(pToastPopup);

	return true;
}

bool mp_util_get_network_result()
{
	connection_h connection = NULL;
	connection_type_e net_state;
	bool bSuccessful = true;
	int err = 0;

	err = connection_create(&connection);
	if (err != CONNECTION_ERROR_NONE) {
		VideoLogWarning("connection_create error. err is [%d]", err);
		bSuccessful = false;
	}

	if (connection) {
		err = connection_get_type(connection, &net_state);
		connection_destroy(connection);

		if (err != CONNECTION_ERROR_NONE) {
			VideoLogWarning("connection_get_type error. err is [%d]",
					err);
			bSuccessful = false;
		}
		VideoLogInfo("net_state is [%d]", net_state);

		if (net_state == CONNECTION_TYPE_DISCONNECTED) {
			bSuccessful = false;
		}
	}

	return bSuccessful;
}

char *mp_util_get_dir_by_path(const char *file_path)
{
	if (!file_path) {
		return NULL;
	}
	char *result = NULL;
	char *dir_path = NULL;
	int location_index = 0;
	const char *location = NULL;;
	const char *head_dir = NULL;
	dir_path = vp_dir_get(file_path);

	if (!dir_path)
		return NULL;

	if (!strncmp(dir_path, MP_UTIL_PHONE_PATH, strlen(MP_UTIL_PHONE_PATH))) {
		head_dir = MP_UTIL_PHONE_PATH;
		location = VIDEOS_FOLDERVIEW_BODY_DEVICE_MEMORY;
	} else if (!strncmp(dir_path, MP_UTIL_SD_PATH, strlen(MP_UTIL_SD_PATH))) {
		head_dir = MP_UTIL_SD_PATH;
		location = VIDEOS_FOLDERVIEW_BODY_SD_CARD;
	} else if (!strncmp
			(dir_path, MP_UTIL_USB_PATH, strlen(MP_UTIL_USB_PATH))) {
		head_dir = MP_UTIL_USB_PATH;
		location = VIDEOS_FOLDERVIEW_BODY_USB_MEMORY;
	} else {
		result = strdup(dir_path);
	}

	if (head_dir) {
		location_index = strlen(head_dir) + 1;
	}
	if (location) {
		MP_FREE_STRING(result);
		result =
			g_strdup_printf("/%s/%s", location,
					(file_path + location_index));
	}
	VideoLogInfo("dir_path=%s, resul=%s", dir_path, result);

	MP_FREE_STRING(dir_path);

	return result;
}

char *mp_util_get_folder_icon_path(int nVideoFolderIndex)
{
	char *result = strdup(VIDEO_LIST_VIEW_ICON_FOLDER_PHONE);
	char *dir_path = NULL;

	char *file_path = mp_util_svc_get_video_folder_url(nVideoFolderIndex);
	if (!file_path) {
		VideoSecureLogError("file_path is NULL");
		return result;
	}
	MpMediaStorageType storage_type =
		mp_util_svc_get_folder_storage(nVideoFolderIndex);

	if (storage_type == MP_MEDIA_TYPE_STORAGE_INTERNAL) {
		if (result) {
			free(result);
			result = NULL;
		}

		result = strdup(VIDEO_LIST_VIEW_ICON_FOLDER_PHONE);
	} else if (storage_type == MP_MEDIA_TYPE_STORAGE_EXTERNAL) {
		dir_path = vp_dir_get(file_path);

		if (!dir_path) {
			MP_FREE_STRING(file_path);
			VideoSecureLogError("dir_path is NULL");
			return result;
		}

		if (!strncmp(dir_path, MP_UTIL_SD_PATH, strlen(MP_UTIL_SD_PATH))) {
			if (result) {
				free(result);
				result = NULL;
			}

			result = strdup(VIDEO_LIST_VIEW_ICON_FOLDER_SDCARD);
		} else if (!strncmp
				(dir_path, MP_UTIL_USB_PATH, strlen(MP_UTIL_USB_PATH))) {
			if (result) {
				free(result);
				result = NULL;
			}

			result = strdup(VIDEO_LIST_VIEW_ICON_FOLDER_EXTERNAL);

		}
	} else if (storage_type == MP_MEDIA_TYPE_STORAGE_DROPBOX) {
		if (result) {
			free(result);
			result = NULL;
		}

		result = strdup(VIDEO_LIST_VIEW_ICON_FOLDER_DROPBOX);
	} else {
		if (result) {
			free(result);
			result = NULL;
		}

		result = strdup(VIDEO_LIST_VIEW_ICON_FOLDER_PHONE);
	}

	MP_FREE_STRING(dir_path);
	MP_FREE_STRING(file_path);

	return result;
}

bool mp_util_get_personal_status(void)
{
	int bOpened = FALSE;

#ifndef	ENABLE_PERSONAL
	return bOpened;
#endif
	int nErr = 0;
	nErr = vconf_get_bool(MP_UTIL_VCONF_KEY_PERSONAL_TAG, &bOpened);
	if (nErr != 0) {
		VideoLogError("MP_UTIL_VCONF_KEY_PERSONAL_TAG is fail [0x%x]",
			      nErr);
		bOpened = FALSE;
	}
	VideoLogInfo("mp_util_get_personal_status = %d", bOpened);

	return (bool) bOpened;
}

bool mp_util_check_video_personal_status(const char *szPreviewUrl)
{
	Eina_Bool bRet = EINA_FALSE;
#ifndef	ENABLE_PERSONAL
	return bRet;
#endif
	if (szPreviewUrl) {
		bRet =
			eina_str_has_prefix(szPreviewUrl,
					    VIDEO_UTIL_PERSONAL_HEAD_STR);
	}
	VideoSecureLogInfo("eina_str_has_prefix=%s:%d", szPreviewUrl,
			   (int) bRet);
	return (bool) bRet;
}

static int __mp_util_is_file_exist(const char *fullpath)
{
	if (!fullpath) {
		return -1;
	}
	struct stat stFileInfo;

	int intStat;

	intStat = stat(fullpath, &stFileInfo);
	if (intStat == 0) {
		return 0;
	}

	return -1;
}

int mp_util_rename_file(const char *old_path, const char *new_path,
			char **szNewestPath)
{
	int nCount = 0;
	char *szNewPath = NULL;
	char *szExt = NULL;
	char *szNewPathTmp = NULL;

	if (!old_path || !new_path) {
		VideoLogError("Error path");
		return -1;
	}

	if (!vp_file_exists(old_path)) {
		VideoLogInfo("Error : file does not exist");
		return -1;
	}

	if (mp_util_set_dest_file(VIDEO_DOWNLOAD_FILE_DIR) == -1) {
		VideoLogInfo("mp_util_set_dest_file failed");
		return -1;
	}
	szNewPath = g_strdup(new_path);
	szExt = strrchr(new_path, '.');

	if (szExt) {
		int nStrlen = strlen(new_path) - strlen(szExt);
		szNewPathTmp = g_strndup(new_path, nStrlen);
	} else {
		szNewPathTmp = g_strdup(new_path);
	}

	/*rename new file when name is existed*/
	while (__mp_util_is_file_exist(szNewPath) == 0) {
		nCount++;
		MP_FREE_STRING(szNewPath);
		if (szExt) {
			szNewPath =
				g_strdup_printf("%s_%d%s", szNewPathTmp, nCount, szExt);
		} else {
			szNewPath = g_strdup_printf("%s_%d", szNewPathTmp, nCount);
		}
	}
	VideoSecureLogInfo("%s:%s", old_path, szNewPath);
	MP_FREE_STRING(szNewPathTmp);
	*szNewestPath = g_strdup(szNewPath);

	if (!vp_file_mv(old_path, szNewPath)) {
		VideoLogError("move false %d", errno);
		MP_FREE_STRING(szNewPath);
		return -1;
	}
	MP_FREE_STRING(szNewPath);

	return 0;
}

int mp_util_set_dest_file(const char *pDestPath)
{
	if (!pDestPath) {
		VideoLogError("pDestPath is NULL");
		return -1;
	}

	int nRet = 0;

	if (!vp_file_exists(pDestPath)) {
		if (!vp_mkpath(pDestPath)) {
			VideoLogError("Make directory Fail : %s", pDestPath);
			nRet = -1;
		} else {
			nRet = 0;
		}
	}

	return nRet;
}


#ifdef ENABLE_DRM_FEATURE
bool mp_util_check_drm_lock(const char *szMediaURL, bool *bLock)
{
	if (!szMediaURL) {
		VideoLogError("szMediaURL is NULL");
		return FALSE;
	}

	bool bIsDrm = FALSE;

	*bLock = FALSE;

	if (vp_drm_is_drm_file(szMediaURL, &bIsDrm)) {
		if (bIsDrm) {
			bool bIsFWLock = FALSE;
			bool bIsDivX = FALSE;
			bool bIsPlayReadyDRM = FALSE;

			if (vp_drm_is_check_forward_lock(szMediaURL, &bIsFWLock)) {
				if (bIsFWLock) {
					*bLock = TRUE;
				}
			}
			if (vp_drm_is_divx_drm_file(szMediaURL, &bIsDivX)) {
				if (bIsDivX) {
					*bLock = TRUE;
				}
			}

			if (vp_drm_is_playready_drm_file
					(szMediaURL, &bIsPlayReadyDRM)) {
				if (bIsPlayReadyDRM) {
					*bLock = TRUE;
				}
			}
		}
	}

	return TRUE;
}
#endif

char *mp_util_get_title_from_path(char *szFilePath)
{
	if (!szFilePath || strlen(szFilePath) <= 0) {
		VideoLogError("File Path is NULL");
		return NULL;
	}

	char *szTitle = NULL;
	const char *filename = vp_file_get(szFilePath);
	if (!filename) {
		return NULL;
	}
	szTitle = vp_strip_ext(filename);

	return szTitle;
}

int mp_util_rename_the_same_file(const char *old_path,
				 char **szNewestPath)
{
	int nCount = 0;
	char *szNewPath = NULL;
	char *szExt = NULL;
	char *szNewPathTmp = NULL;

	if (!old_path) {
		VideoLogError("Error path");
		return -1;
	}

	szNewPath = g_strdup(old_path);
	szExt = strrchr(old_path, '.');

	if (szExt) {
		int nStrlen = strlen(old_path) - strlen(szExt);
		szNewPathTmp = g_strndup(old_path, nStrlen);
	} else {
		szNewPathTmp = g_strdup(old_path);
	}

	/*rename new file when name is existed*/
	while (__mp_util_is_file_exist(szNewPath) == 0) {
		nCount++;
		MP_FREE_STRING(szNewPath);
		if (szExt) {
			szNewPath =
				g_strdup_printf("%s_%d%s", szNewPathTmp, nCount, szExt);
		} else {
			szNewPath = g_strdup_printf("%s_%d", szNewPathTmp, nCount);
		}
	}
	VideoSecureLogInfo("%s", szNewPath);
	MP_FREE_STRING(szNewPathTmp);
	*szNewestPath = g_strdup(szNewPath);

	MP_FREE_STRING(szNewPath);

	return 0;
}

bool mp_util_check_video_file(const char *szFullPath)
{
	bool bVideo = FALSE;
	char *szExt = NULL;
	if (!szFullPath) {
		VideoLogError("INVALID PATH");
		return FALSE;
	}
	char *szPath = g_strdup(szFullPath);
	szExt = mp_info_ctrl_get_file_extension(szPath);

	MP_FREE_STRING(szPath);

	if (!szExt) {
		return FALSE;
	}
	if (!strcasecmp("AVI", szExt)) {
		bVideo = TRUE;
	} else if (!strcasecmp("ASF", szExt)) {
		bVideo = TRUE;
	} else if (!strcasecmp("MP4", szExt)) {
		bVideo = TRUE;
	} else if (!strcasecmp("MPEG", szExt)) {
		bVideo = TRUE;
	} else if (!strcasecmp("MKV", szExt)) {
		bVideo = TRUE;
	} else if (!strcasecmp("SDP", szExt)) {
		bVideo = TRUE;
	} else if (!strcasecmp("WMV", szExt)) {
		bVideo = TRUE;
	} else if (!strcasecmp("3GP", szExt)) {
		bVideo = TRUE;
	} else if (!strcasecmp("3GPP", szExt)) {
		bVideo = TRUE;
	} else if (!strcasecmp("3G2", szExt)) {
		bVideo = TRUE;
	} else if (!strcasecmp("FLV", szExt)) {
		bVideo = TRUE;
	} else if (!strcasecmp("DIVX", szExt)) {
		bVideo = TRUE;
	} else {
		bVideo = FALSE;
	}
	VideoLogInfo("format = %s:%d", szExt, bVideo);
	MP_FREE_STRING(szExt);
	return bVideo;
}

bool mp_util_check_empty_dir(const char *szDir)
{
	if (!szDir) {
		VideoLogError("INVALID PATH");
		return FALSE;
	}
	int nRet = vp_is_dir_empty(szDir);
	if (nRet == 1) {
		return TRUE;
	}
	return FALSE;
}

bool mp_util_get_subtitle_path(const char *szMediaURL, char **szSubtitle)
{
	if (!szMediaURL) {
		VideoLogError("szMediaURL is NULL");
		return FALSE;
	}

	if (strlen(szMediaURL) < 1) {
		VideoLogError("szMediaURL is invalid Length");
		return FALSE;
	}

	struct stat buf;
	int nCount = 0;
	int nStrLength = 0;
	int nOutputLen = 0;

	char *szExt = NULL;
	char szExtWildkey[6][5] = {".srt", ".sub", ".smi", ".SRT", ".SUB",
					".SMI" };
	char *szCheckFilePath = NULL;

	*szSubtitle = NULL;

	nOutputLen = strlen(szMediaURL) + 5;
	szCheckFilePath = calloc(1, sizeof(char)*nOutputLen);
	if (!szCheckFilePath) {
		VideoLogError("faile dto allocate memory");
		return FALSE;
	}

	szExt = strrchr(szMediaURL, '.');
	if (szExt) {
		nStrLength = strlen(szMediaURL) - strlen(szExt);
	} else {
		nStrLength = strlen(szMediaURL);
	}

	for (nCount = 0; nCount < 6; nCount++) {
		memset(szCheckFilePath, 0, nOutputLen);
		strncpy(szCheckFilePath, szMediaURL, nStrLength);
		strncat(szCheckFilePath, szExtWildkey[nCount], 5);
		if (!stat(szCheckFilePath, &buf)) {
			VideoLogWarning("Success finding subtitle file. - %d, %s",
					nCount, szCheckFilePath);
			*szSubtitle = g_strdup(szCheckFilePath);
			break;
		}
	}

	MP_FREE_STRING(szCheckFilePath);

	return TRUE;
}

bool mp_util_get_file_size_by_path(const char *szMediaURL,
				   unsigned long long *nSize)
{
	if (!szMediaURL) {
		VideoLogError("szMediaURL is NULL");
		return FALSE;
	}

	if (strlen(szMediaURL) < 1) {
		VideoLogError("szMediaURL is invalid Length");
		return FALSE;
	}
	struct stat src_info;
	if (stat(szMediaURL, &src_info)) {
		VideoLogInfo("error");
		return FALSE;
	} else {
		*nSize = (unsigned long long) src_info.st_size;
	}
	return TRUE;
}

Evas_Object *mp_util_create_preload_image(Evas_Object *parent,
		const char *path,
		int nIconWidth)
{
	if (!parent || !path) {
		VideoLogError("Invalid parent");
		return NULL;
	}

	Evas_Object *thumb = NULL;
	int nWidth = 0;
	int nHeight = 0;

	thumb = elm_image_add(parent);

	elm_image_preload_disabled_set(thumb, EINA_FALSE);
	elm_image_smooth_set(thumb, EINA_FALSE);

	elm_image_prescale_set(thumb, nIconWidth);


	elm_image_file_set(thumb, path, NULL);

	elm_image_object_size_get(thumb, &nWidth, &nHeight);

	if (nWidth > nHeight) {
		elm_image_aspect_fixed_set(thumb, EINA_FALSE);
	}

	return thumb;
}

Evas_Object *mp_util_create_wall_preload_image(Evas_Object *parent,
		int nIconWidth)
{
	if (!parent) {
		VideoLogError("Invalid parent");
		return NULL;
	}

	Evas_Object *thumb = NULL;

	thumb = elm_image_add(parent);

	elm_image_preload_disabled_set(thumb, EINA_FALSE);
	elm_image_smooth_set(thumb, EINA_FALSE);

	elm_image_prescale_set(thumb, nIconWidth);

	return thumb;
}

bool mp_util_get_image_size(Evas_Object *pParent, const char *szMediaURL,
			    int *nWidth, int *nHeight)
{
	if (!szMediaURL || !pParent) {
		VideoLogError("szMediaURL or pParent is NULL");
		return FALSE;
	}

	int nWidth_t = 0;
	int nHeight_t = 0;

	Evas *pObject = evas_object_evas_get(pParent);
	if (!pObject) {
		return FALSE;
	}
	Evas_Object *pImage = evas_object_image_add(pObject);
	if (!pImage) {
		return FALSE;
	}
	evas_object_image_file_set(pImage, szMediaURL, NULL);
	evas_object_image_size_get(pImage, &nWidth_t, &nHeight_t);

	VideoLogInfo("Image Width %d, Height %d", nWidth_t, nHeight_t);
	*nWidth = nWidth_t;
	*nHeight = nHeight_t;
	MP_DEL_OBJ(pImage);

	return TRUE;
}

char *mp_util_get_preview_image_path(const char *szFilePath)
{
	if (!szFilePath) {
		VideoLogError("[ERR] No exist szFilePath.");
		return NULL;
	}

	char *szVideoID = NULL;
	char *szFileName = NULL;
	int nPosition = 0;

	nPosition =
		mp_util_svc_get_video_last_play_time_by_video_url(szFilePath);
	if (nPosition == 0) {
		szFileName =
			mp_util_svc_get_video_Thumbnail_by_video_url(szFilePath);
		return szFileName;
	}

	if (!mp_util_svc_get_video_id_by_video_url(szFilePath, &szVideoID)) {
		VideoLogError("mp_util_svc_get_video_id_by_video_url is Fail");
		MP_FREE_STRING(szVideoID);
		szFileName =
			mp_util_svc_get_video_Thumbnail_by_video_url(szFilePath);
		return szFileName;
	}

	szFileName =
		g_strdup_printf("%s/%s.jpg", VIDEO_THUMB_DATA_DIR, szVideoID);
	if (!vp_file_exists(szFileName)) {
		MP_FREE_STRING(szVideoID);
		MP_FREE_STRING(szFileName);
		szFileName =
			mp_util_svc_get_video_Thumbnail_by_video_url(szFilePath);
		return szFileName;
	}

	MP_FREE_STRING(szVideoID);

	return szFileName;
}

bool mp_util_save_preview_image(const char *szFilePath)
{
	if (!szFilePath) {
		VideoLogError("[ERR] No exist szFilePath.");
		return FALSE;
	}

	char *szVideoID = NULL;
	char *szFileName = NULL;
	unsigned int nPosition = 0;
	int nWidth = 0;
	int nHeight = 0;
	unsigned int nDuration = 0;
	char *szTitle = NULL;
	mp_util_svc_get_video_detail_by_video_url(szFilePath, &nWidth,
			&nHeight, &szTitle,
			&nDuration, &nPosition);
	MP_FREE_STRING(szTitle);

	if (nPosition == 0) {
		return FALSE;
	}

	if (!mp_util_svc_get_video_id_by_video_url(szFilePath, &szVideoID)) {
		MP_FREE_STRING(szVideoID);
		VideoLogError("mp_util_svc_get_video_id_by_video_url is Fail");
		return FALSE;
	}

	if (!vp_is_dir(VIDEO_THUMB_DATA_DIR)) {
		if (!vp_mkpath(VIDEO_THUMB_DATA_DIR)) {
			MP_FREE_STRING(szVideoID);
			return FALSE;
		}
	}

	szFileName =
		g_strdup_printf("%s/%s.jpg", VIDEO_THUMB_DATA_DIR, szVideoID);

	mp_util_svc_get_frame_by_position(szFilePath, szFileName, nPosition,
					  nWidth, nHeight);
	VideoLogInfo("szFileName %s", szFileName);

	MP_FREE_STRING(szVideoID);
	MP_FREE_STRING(szFileName);

	return TRUE;
}

static void
__mp_util_progress_resize_cb(void *data, Evas *e, Evas_Object *obj,
			     void *event_info)
{
	int w = 0;
	int h = 0;

	if (!data) {
		VideoLogError("data = NULL !!!");
		return;
	}

	evas_object_geometry_get(mp_util_get_main_window_handle(), NULL, NULL,
				 &w, &h);
	evas_object_resize((Evas_Object *) data, w, h);
}


static void
__mp_util_progress_del_cb(void *data, Evas *e, Evas_Object *obj,
			  void *event_info)
{
	if (!obj) {
		VideoLogError("obj = NULL !!!");
		return;
	}

	evas_object_event_callback_del(mp_util_get_main_window_handle(),
				       EVAS_CALLBACK_RESIZE,
				       __mp_util_progress_resize_cb);
	evas_object_event_callback_del(obj, EVAS_CALLBACK_RESIZE,
				       __mp_util_progress_del_cb);
}


Evas_Object *mp_util_create_embedded_progress()
{
	Evas_Object *progressbar = NULL;
	Evas_Object *layout = NULL;

	int w = 0;
	int h = 0;
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_PLAYER_POPUP_PROGRESSBAR_EDJ);
	free(path);
	layout = elm_layout_add(mp_util_get_main_window_handle());
	Eina_Bool r = FALSE;
	r = elm_layout_file_set(layout, edj_path,
				"popup_embeded_processing");
	if (!r) {
		evas_object_del(layout);
		VideoLogError("layout = NULL !!!");
		return NULL;
	}
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL,
					EVAS_HINT_FILL);

	evas_object_geometry_get(mp_util_get_main_window_handle(), NULL, NULL,
				 &w, &h);

	evas_object_resize(layout, w, h);

	evas_object_event_callback_add(mp_util_get_main_window_handle(),
				       EVAS_CALLBACK_RESIZE,
				       __mp_util_progress_resize_cb,
				       (const void *) layout);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL,
				       __mp_util_progress_del_cb,
				       (const void *) layout);

	evas_object_show(layout);

	progressbar = elm_progressbar_add(layout);
	elm_object_style_set(progressbar, "process_large");
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_show(progressbar);

	elm_object_part_content_set(layout, "elm.swallow.content",
				    progressbar);

	evas_object_propagate_events_set(layout, EINA_FALSE);

	return layout;
}

void mp_util_destroy_embedded_progress(Evas_Object *pProgress)
{
	if (!pProgress) {
		VideoLogError("pProgress == NULL");
		return;
	}

	evas_object_del(pProgress);
	pProgress = NULL;
}

bool mp_util_meta_get_width_and_height(const char *szFilePath,
				       int *nWidth, int *nHeight)
{
	if (!szFilePath) {
		VideoLogError("szFilePath is NULL");
		return FALSE;
	}

	metadata_extractor_h pMetadata = NULL;
	char *szWidth = NULL;
	char *szHeight = NULL;

	int nRet = METADATA_EXTRACTOR_ERROR_NONE;

	nRet = metadata_extractor_create(&pMetadata);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogError("metadata_extractor_create is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet = metadata_extractor_set_path(pMetadata, szFilePath);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogError("metadata_extractor_set_path is fail : 0x%x", nRet);
		goto Exception;
	}

	nRet =
		metadata_extractor_get_metadata(pMetadata, METADATA_VIDEO_WIDTH,
						&szWidth);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogError("metadata_extractor_get_metadata is fail : 0x%x",
			      nRet);
		goto Exception;
	}

	nRet =
		metadata_extractor_get_metadata(pMetadata, METADATA_VIDEO_HEIGHT,
						&szHeight);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		VideoLogError("metadata_extractor_get_metadata is fail : 0x%x",
			      nRet);
		goto Exception;
	}
	if (szWidth) {
		*nWidth = atoi(szWidth);
	} else {
		*nWidth = 0;
	}

	if (szHeight) {
		*nHeight = atoi(szHeight);
	} else {
		*nHeight = 0;
	}

	MP_FREE_STRING(szWidth);
	MP_FREE_STRING(szHeight);

	if (pMetadata) {
		metadata_extractor_destroy(pMetadata);
		pMetadata = NULL;
	}

	return TRUE;
Exception:

	MP_FREE_STRING(szWidth);
	MP_FREE_STRING(szHeight);

	if (pMetadata) {
		metadata_extractor_destroy(pMetadata);
		pMetadata = NULL;
	}

	return FALSE;
}

bool mp_util_delete_wall_file(char *szVideoId)
{
	if (!szVideoId) {
		return FALSE;
	}
	char *szWallDir = NULL;
	bool bRet = TRUE;
	szWallDir = g_strdup_printf("%s/%s", MP_VIDEO_WALL_DIR, szVideoId);
	if (vp_is_dir(szWallDir)) {
		bRet = vp_file_recursive_rm(szWallDir);
	}

	VideoSecureLogInfo("delete video folder= %s:%d", szWallDir, bRet);
	MP_FREE_STRING(szWallDir);
	return TRUE;
}

Evas_Object *mp_util_create_gengrid(Evas_Object *parent, int nWidth,
				    int nHeight, bool bHorizontal)
{
	Evas_Object *pGengrid = elm_gengrid_add(parent);
	if (!pGengrid) {
		VideoLogError("pVideoGrid is not existed.");
		return NULL;
	}
	evas_object_size_hint_weight_set(pGengrid, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pGengrid, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	elm_gengrid_item_size_set(pGengrid, nWidth, nHeight);
	elm_gengrid_align_set(pGengrid, 0.0, 0.0);
	elm_gengrid_horizontal_set(pGengrid, bHorizontal);

	return pGengrid;
}

Evas_Object *mp_util_create_folder_icon(void *pParent,
					const char *icon_path)
{
	if (!pParent || !icon_path) {
		VideoLogError("invalid parent and icon path");
		return NULL;
	}
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_PLAYER_IMAGE_NAME_EDJ);
	free(path);
	Evas_Object *pIcon = elm_image_add(pParent);
	elm_image_file_set(pIcon, edj_path,
			   (const char *) icon_path);
	evas_object_size_hint_align_set(pIcon, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(pIcon, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	if (!g_strcmp0(VIDEO_LIST_VIEW_ICON_FOLDER_DROPBOX, icon_path)) {
		evas_object_color_set(pIcon, 50, 129, 194, 255);
	} else {
		evas_object_color_set(pIcon, 255, 255, 255, 255);
	}
	evas_object_show(pIcon);

	return pIcon;
}

bool mp_util_create_folder_sub_icon(void *pLayout, int nVideoFolderIndex)
{
	if (!pLayout) {
		VideoLogError("invalid parent");
		return FALSE;
	}

	Evas_Object *pTempLayout = (Evas_Object *) pLayout;
	Evas_Object *pIcon = elm_image_add(pLayout);
	Evas_Object *pSubIcon = elm_image_add(pLayout);

	MpMediaStorageType storageType =
		mp_util_svc_get_folder_storage(nVideoFolderIndex);
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_PLAYER_IMAGE_NAME_EDJ);
	free(path);
	if (storageType == MP_MEDIA_TYPE_STORAGE_INTERNAL) {
		elm_image_file_set(pIcon, edj_path,
				   VIDEO_LIST_VIEW_ICON_FOLDER_PHONE);
		elm_image_file_set(pSubIcon, edj_path,
				   VIDEO_LIST_VIEW_ICON_FOLDER_PHONE_EF);
	} else if (storageType == MP_MEDIA_TYPE_STORAGE_DROPBOX) {
		elm_image_file_set(pIcon, edj_path,
				   VIDEO_LIST_VIEW_ICON_FOLDER_DROPBOX);
		elm_image_file_set(pSubIcon, edj_path,
				   VIDEO_LIST_VIEW_ICON_FOLDER_DROPBOX_EF);
	} else if (storageType == MP_MEDIA_TYPE_STORAGE_EXTERNAL) {
	} else {
		VideoLogInfo("Unknown Storage type or not folder item.");
		return FALSE;
	}

	evas_object_size_hint_align_set(pIcon, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(pIcon, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pSubIcon, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(pSubIcon, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);

	elm_layout_content_set(pTempLayout, "elm.folder.icon", pIcon);
	elm_layout_content_set(pTempLayout, "elm.folder.icon.ef", pSubIcon);

	evas_object_show(pIcon);
	evas_object_show(pSubIcon);

	/*      if (!pLayout)*/
	/*              return FALSE;*/

	return TRUE;
}

bool mp_util_create_video_sub_icon(void *pLayout, int nVideoItemIndex)
{
	if (!pLayout) {
		VideoLogError("invalid parent");
		return FALSE;
	}

	Evas_Object *pTempLayout = (Evas_Object *) pLayout;
	Evas_Object *pIcon = elm_image_add(pLayout);
	Evas_Object *pSubIcon = elm_image_add(pLayout);

	MpMediaStorageType storageType =
		mp_util_svc_get_video_storage(nVideoItemIndex);
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_PLAYER_IMAGE_NAME_EDJ);
	free(path);
	if (storageType == MP_MEDIA_TYPE_STORAGE_DROPBOX) {
		elm_image_file_set(pIcon, edj_path,
				   VIDEO_LIST_VIEW_ICON_FOLDER_DROPBOX);
		elm_image_file_set(pSubIcon, edj_path,
				   VIDEO_LIST_VIEW_ICON_FOLDER_DROPBOX_EF);
	} else if (storageType == MP_MEDIA_TYPE_STORAGE_EXTERNAL) {
	} else {
		VideoLogInfo
		("Unknown storage or internal storage item. nVideoItemIndex: %d.",
		 nVideoItemIndex);
		return FALSE;
	}

	evas_object_size_hint_align_set(pIcon, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(pIcon, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pSubIcon, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(pSubIcon, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);

	elm_layout_content_set(pTempLayout, "elm.folder.icon", pIcon);
	elm_layout_content_set(pTempLayout, "elm.folder.icon.ef", pSubIcon);

	evas_object_show(pIcon);
	evas_object_show(pSubIcon);
	/*Prevent fix*/
	/*if (!pLayout)
	   return FALSE;
	 */
	return TRUE;
}

unsigned long long mp_util_get_disk_available_space(const char
		*szFilePath)
{
	if (!szFilePath) {
		VideoLogError("szFilePath is NULL");
		return 0;
	}
	int nRet = 0;
	struct statvfs info;
	if (szFilePath
			&& 0 == strncmp(szFilePath, MP_UTIL_SD_PATH,
					strlen(MP_UTIL_SD_PATH))) {
		if (-1 == statvfs(MP_UTIL_SD_PATH, &info)) {
			return 0;
		}
	} else {
		if (vp_is_dir(szFilePath)) {
			nRet = statvfs(szFilePath, &info);
			VideoLogInfo("this is folder");
		} else {
			nRet = statvfs(MP_UTIL_MEDIA_DIR, &info);
		}
		if (nRet < 0) {
			return 0;
		}
	}

	return (info.f_bsize) * info.f_bavail;
}

void mp_util_virtual_keypad_disabled_set(Eina_Bool bDisabled)
{
	if (!pMainConformant) {
		VideoLogError("[ERR] No exist pMainConformant.");
		return;
	}

	if (bDisabled) {
		VideoLogInfo("Virtual keypad will not compress the layout.");
		elm_object_signal_emit(pMainConformant,
				       "elm,state,clipboard,disable", "");
		elm_object_signal_emit(pMainConformant,
				       "elm,state,virtualkeypad,disable", "");
	} else {
		VideoLogInfo("Virtual keypad will compress the layout.");
		elm_object_signal_emit(pMainConformant,
				       "elm,state,clipboard,enable", "");
		elm_object_signal_emit(pMainConformant,
				       "elm,state,virtualkeypad,enable", "");
	}
}

int mp_util_set_translate_str(Evas_Object *obj, const char *str)
{
	if (!obj || !str) {
		VideoLogError("[ERR] Invalid input data.");
		return -1;
	}

	char *domain = NULL;

	if (strstr(str, "IDS_COM")) {
		domain = "sys_string";
	}

	elm_object_domain_translatable_text_set(obj, domain, str);

	return 0;
}
