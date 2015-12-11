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
#include <Ecore_Evas.h>
#include <Elementary.h>

#include "mp-util.h"
#include "mp-video-log.h"
#include "video-player.h"
#include "mp-video-value-define.h"
#include "mp-video-string-define.h"
#include "mp-video-rename-ctrl.h"
#include "mp-util-media-service.h"
#include "mp-util-widget-ctrl.h"
#include "mp-video-info-ctrl.h"
#include "vp-util.h"
#include "vp-file-util.h"
#include "mp-util-preference.h"
#include "mp-util-config.h"
#include "mp-video-list-view-select.h"


typedef struct _VideoRenamePopup {
	//obj
	void *pNaviFrameHandle;
	Evas_Object *pBaseLayout;
	Evas_Object *pGenlist;
	Evas_Object *pPopUpHandle;
	Evas_Object *pPopupCancelBtn;
	Evas_Object *pPopupOkBtn;
	Evas_Object *pEntry;
	Evas_Object *pEntryClearBtn;
	Evas_Object *pEntryLayout;

	//data
	mpRenameCtrlCbFunc RenameCtrlUserCbFunc;
	char *szSaveFileName;
	char *szOldName;
	char *szTempName;
	int nVideoIndex;
	bool bPersonalType;
	bool bSetToEnd;
	Ecore_Idler *pSetLineEndIdler;
}stRenamePopup;


stRenamePopup *g_pRenameHandle = NULL;

//static void _MpRenameDeletePopupHandle(void);
static bool __mp_rename_ctrl_check_valid_text(const char *text, int *nLen);
static bool __mp_rename_ctrl_check_valid_all_space(const char *text);
/*
static void __mp_rename_ctrl_invalid_popup_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	VideoLogInfo("");

	Evas_Object *en = pUserData;

	mp_util_delete_popup_handle();

	elm_entry_cursor_end_set(en);
	evas_object_show(en);
	elm_object_focus_set(en, EINA_TRUE);
}
*/
////////////////////////////////////////////////////////////
// Callback function
////////////////////////////////////////////////////////////
static void __mp_rename_ctrl_cancel_btn_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");

	mp_rename_ctrl_delete_handle();
}

static void __mp_rename_ctrl_timeout_cb(void *pUserData, Evas_Object *pObject, void *event_info)	{
	evas_object_del(pObject);

	elm_entry_cursor_end_set(g_pRenameHandle->pEntry);
	elm_object_focus_set(g_pRenameHandle->pEntry, EINA_TRUE);
}

static void __mp_rename_ctrl_ok_btn_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");

	Evas_Object *en = (Evas_Object *)pUserData;
	if (!en || !g_pRenameHandle) {
		VideoLogError("g_pRenameHandle is NULL");
		return;
	}

	int nDstLen = 0;
	if (!__mp_rename_ctrl_check_valid_text(g_pRenameHandle->szSaveFileName, &nDstLen)) {
		mp_util_ticker_toast_with_timeout_cb_popup(VIDEOS_NOTIPOP_MSG_INVALID_CHARACTERS, __mp_rename_ctrl_timeout_cb);
		return;
	}

	char *szFilePath = mp_util_svc_get_video_url(g_pRenameHandle->nVideoIndex);
	if (!szFilePath) {
		VideoLogError("szFilePath is NULL.");
		mp_rename_ctrl_delete_handle();
		return;
	}

	char *szFileExt = mp_info_ctrl_get_file_extension(szFilePath);
	if (!szFileExt) {
		VideoLogError("szFileExt is NULL.");
		MP_FREE_STRING(szFilePath);
		mp_rename_ctrl_delete_handle();
		return;
	}

	char *szFileName = strrchr(szFilePath, '/');
	if (!szFileName) {
		VideoLogError("szFileName is NULL.");
		MP_FREE_STRING(szFilePath);
		MP_FREE_STRING(szFileExt);
		mp_rename_ctrl_delete_handle();
		return;
	}

	int nLastPlayedTime = mp_util_svc_get_video_last_play_time_by_video_url(szFilePath);

	char *szLocation = NULL;
	szLocation = g_strndup(szFilePath, strlen(szFilePath) - strlen(szFileName));

	if (!szLocation) {
		VideoLogError("szLocation is NULL.");
		MP_FREE_STRING(szFilePath);
		MP_FREE_STRING(szFileExt);
		mp_rename_ctrl_delete_handle();
		return;
	}

	char *szNewFile = g_strdup_printf("%s/%s.%s", szLocation, g_pRenameHandle->szSaveFileName, szFileExt);

	if (!szNewFile || vp_file_exists(szNewFile)) {
		VideoLogInfo("szNewFile is %s.", szNewFile);
		MP_FREE_STRING(szFilePath);
		MP_FREE_STRING(szFileExt);
		MP_FREE_STRING(szLocation);
//		mp_rename_ctrl_delete_handle();

		mp_util_ticker_toast_with_timeout_cb_popup(VIDEOS_NOTIPOP_MSG_ALREADY_EXISTS, __mp_rename_ctrl_timeout_cb);

		return;
	}

	if (strcmp(szNewFile, szFilePath) != 0) {
		VideoLogInfo("Correct new name style");
		char *szNewPath = NULL;
		int ret = mp_util_rename_file((const char *)szFilePath, (const char *)szNewFile, &szNewPath);

		if (!szNewPath) {
			VideoLogError("szNewPath is NULL.");
			MP_FREE_STRING(szFilePath);
			MP_FREE_STRING(szFileExt);
			MP_FREE_STRING(szLocation);
			MP_FREE_STRING(szNewFile);
			mp_rename_ctrl_delete_handle();
			return;
		}

		if (ret != 0) {
			VideoLogError("mp_util_rename_file failed");
		} else {
			mp_util_svc_move_to_db(g_pRenameHandle->nVideoIndex, szNewPath);
			mp_util_svc_set_video_last_played_time_by_url(szNewFile, nLastPlayedTime);
		}

		char *szMediaUrl = mp_util_config_get_preview();
		if (szMediaUrl) {
			if (!strcmp(szFilePath, szMediaUrl)) {
				mp_util_config_set_preview(szNewFile);
				mp_util_svc_set_video_last_played_time_by_url(szNewFile, nLastPlayedTime);
			}
		}

		MP_FREE_STRING(szFilePath);
		MP_FREE_STRING(szFileExt);
		MP_FREE_STRING(szLocation);
		MP_FREE_STRING(szNewFile);

		if (g_pRenameHandle->RenameCtrlUserCbFunc) {
			g_pRenameHandle->RenameCtrlUserCbFunc();
		}
	}

	MP_FREE_STRING(szFileExt);

	mp_rename_ctrl_delete_handle();
}

static void __mp_rename_ctrl_entry_max_len_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");

	//mp_util_info_popup(VIDEOS_NOTIPOP_MSG_MAX_NUM_OF_CHARACTERS, __MpRenameEntryMaxLenBackCb, pUserData, VIDEOS_STRING);
	mp_util_ticker_toast_popup(VIDEOS_NOTIPOP_MSG_MAX_NUM_OF_CHARACTERS, false, false);
}

static void __mp_rename_ctrl_entry_activate_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pRenameHandle) {
		VideoLogError("g_pRenameHandle is NULL");
		return;
	}

	if (!pObject) {
		VideoLogError("pObject is NULL");
		return;
	}

	char *entry_data = (char *)elm_entry_entry_get(pObject);
	char *szFileName = elm_entry_markup_to_utf8(entry_data);

	if (!szFileName) {
		VideoLogError("failed to get text");
		return;
	}
	VideoLogInfo("title is %s", szFileName);

	MP_FREE_STRING(g_pRenameHandle->szSaveFileName);
	if (!strlen(szFileName)) {
		VideoLogWarning("title is empty");
	}
	else {
		g_pRenameHandle->szSaveFileName = g_strdup(szFileName);
	}

	MP_FREE_STRING(szFileName);
}

static Eina_Bool __mp_rename_ctrl_entry_set_line_end(void *pUserData)
{
	if (!g_pRenameHandle) {
		VideoLogError("g_pRenameHandle is NULL");
		return ECORE_CALLBACK_CANCEL;
	}

	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return ECORE_CALLBACK_CANCEL;
	}

	Evas_Object *pObj = pUserData;

	g_pRenameHandle->pSetLineEndIdler = NULL;
	g_pRenameHandle->bSetToEnd = FALSE;

	elm_entry_cursor_line_end_set(pObj);

	return ECORE_CALLBACK_CANCEL;
}

static void __mp_rename_ctrl_entry_clear_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	if (!g_pRenameHandle) {
		VideoLogError("g_pRenameHandle is NULL");
		return;
	}

	Evas_Object *en = (Evas_Object *)data;
	elm_entry_entry_set(en, "");
	MP_FREE_STRING(g_pRenameHandle->szTempName);
}

static void __mp_rename_ctrl_entry_changed_cb(void *pUserData, Evas_Object *pObj, void *pEvent)
{
	if (!g_pRenameHandle) {
		VideoLogError("g_pRenameHandle is NULL");
		return;
	}

	if (!pObj) {
		VideoLogError("pObj is NULL");
		return;
	}
	VideoLogInfo("");

	Evas_Object *en = pObj;

	char *entry_data = (char *)elm_entry_entry_get(en);
	char *szFileName = elm_entry_markup_to_utf8(entry_data);
	int nDstLen = 0;
	bool isEmpty = elm_entry_is_empty(en);

	if (!szFileName) {
		VideoLogError("failed to get text");
		return;
	}

	if (isEmpty) {
		VideoLogInfo("Entry is empty");
		elm_object_part_content_unset(g_pRenameHandle->pEntryLayout, "elm.swallow.end");
		evas_object_hide(g_pRenameHandle->pEntryClearBtn);
	} else {
		elm_object_part_content_set(g_pRenameHandle->pEntryLayout, "elm.swallow.end", g_pRenameHandle->pEntryClearBtn);
		evas_object_show(g_pRenameHandle->pEntryClearBtn);
	}

	MP_FREE_STRING(g_pRenameHandle->szSaveFileName);
	if (isEmpty || !__mp_rename_ctrl_check_valid_all_space(szFileName))
	{
		VideoLogWarning("Title is empty or title consists of only spaces");
		if (g_pRenameHandle->pPopupOkBtn) {
			elm_object_disabled_set(g_pRenameHandle->pPopupOkBtn, EINA_TRUE);
		}
	} else if (!__mp_rename_ctrl_check_valid_text(szFileName, &nDstLen))
	{
		VideoLogWarning("New file name is invalid.");
		if (!g_strcmp0(g_pRenameHandle->szOldName, szFileName)) {
			elm_object_disabled_set(g_pRenameHandle->pPopupOkBtn, EINA_TRUE);
			VideoLogInfo("it is the old name");
		} else {
			int position = elm_entry_cursor_pos_get(en);
			elm_entry_entry_set(en, elm_entry_utf8_to_markup(g_pRenameHandle->szTempName));
			elm_entry_cursor_begin_set(en);
			elm_entry_cursor_pos_set(en, position - 1);

			mp_util_status_bar_noti_popup_show(VIDEOS_NOTIPOP_MSG_INVALID_CHARACTERS);
			MP_FREE_STRING(szFileName);
			return;
		}
	} else {
		g_pRenameHandle->szSaveFileName = g_strdup(szFileName);
		if (!g_strcmp0(g_pRenameHandle->szOldName, szFileName)) {
			elm_object_disabled_set(g_pRenameHandle->pPopupOkBtn, EINA_TRUE);
			VideoLogInfo("it is the old name");
			elm_entry_select_all(en);
			elm_entry_cursor_end_set(en);
		} else {
			elm_object_disabled_set(g_pRenameHandle->pPopupOkBtn, EINA_FALSE);
		}
	}
	MP_FREE_STRING(g_pRenameHandle->szTempName);
	g_pRenameHandle->szTempName = strdup(szFileName);
	if (g_pRenameHandle->bSetToEnd) {
		MP_DEL_IDLER(g_pRenameHandle->pSetLineEndIdler);
		g_pRenameHandle->pSetLineEndIdler = ecore_idler_add(__mp_rename_ctrl_entry_set_line_end, pObj);
	}
	 MP_FREE_STRING(szFileName);
}

static void __mp_rename_ctrl_entry_clicked_cb(void *pUserData, Evas_Object *pObj, void *pEvent)
{
	if (!g_pRenameHandle) {
		VideoLogError("g_pRenameHandle is NULL");
		return;
	}

	Evas_Object *en = (Evas_Object *)pObj;
	elm_entry_select_none(en);
}

static void __mp_rename_ctrl_entry_long_pressed_cb(void *pUserData, Evas_Object *pObj, void *pEvent)
{
	if (!g_pRenameHandle) {
		VideoLogError("g_pRenameHandle is NULL");
		return;
	}

	Evas_Object *en = (Evas_Object *)pObj;
	evas_object_smart_callback_del (en, "clicked", (Evas_Smart_Cb) __mp_rename_ctrl_entry_clicked_cb);
}
////////////////////////////////////////////////////////////
// Internal function
////////////////////////////////////////////////////////////
static bool __mp_rename_ctrl_check_valid_text(const char *text, int *nLen)
{
	if (!text) {
		return FALSE;
	}

	char pattern[] = { '/', '\\', ':', '*', '?', '"', '<', '>', '|', '\0' };
	const char *tmp = text;
	int nSrcLen = 0;

	/* hidden property check */
	if (strncmp(tmp, ".", strlen(".")) == 0) {
		return FALSE;
	}

	/* check bad character */
	while (*tmp != '\0') {
		if (strchr(pattern, *tmp) != NULL) {
			VideoLogError("Invalid text=%s char=%c", text, *tmp);
			*nLen = nSrcLen;
			return FALSE;
		}
		nSrcLen++;
		tmp++;
	}

	*nLen = nSrcLen;
	return TRUE;
}

static bool __mp_rename_ctrl_check_valid_all_space(const char *text)
{
	if (!text) {
		return FALSE;
	}

	const char *tmp = text;

	/* check all space */
	while (*tmp != '\0') {

		if (' ' != *tmp) {
			return TRUE;
		}
		tmp++;
	}

	return FALSE;
}

static void __mp_rename_ctrl_set_save_file_name()
{
	if (!g_pRenameHandle) {
		VideoLogError("g_pRenameHandle is NULL");
		return;
	}

	if (g_pRenameHandle->szSaveFileName) {
		MP_FREE_STRING(g_pRenameHandle->szSaveFileName);
		g_pRenameHandle->szSaveFileName = NULL;
	}
	char *szTitle = NULL;
	char *szPath = mp_util_svc_get_video_url(g_pRenameHandle->nVideoIndex);
	szTitle = mp_util_get_title_from_path(szPath);
	g_pRenameHandle->szSaveFileName = g_strdup(szTitle);

	MP_FREE_STRING(szPath);
	MP_FREE_STRING(szTitle);
}

static void __mp_rename_view_rotate_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!g_pRenameHandle) {
		VideoLogError("__mp_rename_view_rotate_cb IS null");
		return;
	}
	VideoLogInfo("");
	mp_select_view_arrange_video_list(g_pRenameHandle->pGenlist, FALSE);
}

////////////////////////////////////////////////////////////
// External function
////////////////////////////////////////////////////////////

Evas_Object *mp_rename_ctrl_show(void *pNaviFrameHandle, void *pUserData, mpRenameCtrlCbFunc pRenameCtrlCb, int nVideoItemIndex)
{
	VideoLogInfo("");

	if (nVideoItemIndex < 0) {
		VideoLogError("nVideoItemIndex is INVALID = %d", nVideoItemIndex);
		return FALSE;
	}

	mp_rename_ctrl_delete_handle();

	g_pRenameHandle = (stRenamePopup*)calloc(1, sizeof(stRenamePopup));
	if (!g_pRenameHandle) {
		VideoLogError("g_pRenameHandle is NULL");
		return FALSE;
	}

	g_pRenameHandle->pNaviFrameHandle = pNaviFrameHandle;
	g_pRenameHandle->RenameCtrlUserCbFunc = pRenameCtrlCb;
	g_pRenameHandle->nVideoIndex = nVideoItemIndex;
	g_pRenameHandle->bSetToEnd = TRUE;

	char *pVideoFile = mp_util_svc_get_video_url(g_pRenameHandle->nVideoIndex);
	char *szFileExt = mp_info_ctrl_get_file_extension(pVideoFile);
	g_pRenameHandle->bPersonalType = mp_util_check_video_personal_status((const char*)pVideoFile);

	mp_widget_ctrl_disable_navi_handle_focus_except_item(g_pRenameHandle->pNaviFrameHandle, NULL);

	evas_object_smart_callback_add((Evas_Object*)mp_util_get_main_window_handle(), "wm,rotation,changed", __mp_rename_view_rotate_cb, NULL);

	Evas_Object *pLayout = NULL;
	//mp_util_svc_set_update_db_cb_func(pRenameCtrlCb);
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_CUSTOM_THEME);
	free(path);
	//Create popup
	g_pRenameHandle->pBaseLayout = elm_layout_add(g_pRenameHandle->pNaviFrameHandle);
	elm_layout_file_set(g_pRenameHandle->pBaseLayout, edj_path, "transparent_layout");

	g_pRenameHandle->pGenlist = elm_genlist_add(g_pRenameHandle->pBaseLayout);
	elm_scroller_bounce_set(g_pRenameHandle->pGenlist, EINA_FALSE, EINA_TRUE);
	evas_object_size_hint_weight_set(g_pRenameHandle->pGenlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pRenameHandle->pGenlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(g_pRenameHandle->pGenlist);
	elm_object_part_content_set(g_pRenameHandle->pBaseLayout, "base_bg", g_pRenameHandle->pGenlist);
	mp_select_view_arrange_video_list(g_pRenameHandle->pGenlist, FALSE);

	evas_object_size_hint_weight_set(g_pRenameHandle->pBaseLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pRenameHandle->pBaseLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(g_pRenameHandle->pBaseLayout);
	g_pRenameHandle->pPopUpHandle = elm_popup_add(g_pRenameHandle->pNaviFrameHandle);

	//elm_object_style_set(g_pRenameHandle->pPopUpHandle, "no_effect");
	eext_object_event_callback_add(g_pRenameHandle->pPopUpHandle, EEXT_CALLBACK_BACK, __mp_rename_ctrl_cancel_btn_cb, pUserData);

	elm_object_part_text_set(g_pRenameHandle->pPopUpHandle, "title,text", VIDEOS_RENAMEPOP_HEADER_RENAME);
	evas_object_size_hint_weight_set(g_pRenameHandle->pPopUpHandle, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	//evas_object_smart_callback_add(mp_util_get_main_conformat(), "virtualkeypad,state,on", __mp_rename_ctrl_keypad_on, g_pRenameHandle->pPopUpHandle);
	//evas_object_smart_callback_add(mp_util_get_main_conformat(), "virtualkeypad,state,off", __mp_rename_ctrl_keypad_off, g_pRenameHandle->pPopUpHandle);

	pLayout = elm_layout_add(g_pRenameHandle->pPopUpHandle);
	elm_layout_file_set(pLayout, edj_path, "pv.popup.entry");

	//Create entry
	Evas_Object *en = NULL;
	en = elm_entry_add(pLayout);
	elm_entry_single_line_set(en, EINA_TRUE);
	elm_entry_scrollable_set(en, EINA_TRUE);

	elm_entry_cnp_mode_set(en, ELM_CNP_MODE_PLAINTEXT);
	elm_entry_input_panel_layout_set(en, ELM_INPUT_PANEL_LAYOUT_NORMAL);
	elm_entry_editable_set(en, TRUE);
	elm_entry_input_panel_return_key_type_set(en, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT);
	elm_entry_input_panel_return_key_disabled_set(en, EINA_FALSE);
	elm_entry_prediction_allow_set(en, EINA_TRUE);
	g_pRenameHandle->pEntryLayout = pLayout;

	static Elm_Entry_Filter_Limit_Size limit_filter_data;
	limit_filter_data.max_char_count = VIDEO_FILE_SEARCH_CHAR_LEN_MAX;
	limit_filter_data.max_byte_count = 0;
	elm_entry_markup_filter_append(en, elm_entry_filter_limit_size, &limit_filter_data);

	evas_object_smart_callback_add(en, "maxlength,reached", __mp_rename_ctrl_entry_max_len_cb, pUserData);
	evas_object_smart_callback_add(en, "activated", __mp_rename_ctrl_entry_activate_cb, pUserData);
	evas_object_smart_callback_add(en, "changed", __mp_rename_ctrl_entry_changed_cb, pUserData);
	evas_object_smart_callback_add(en, "preedit,changed", __mp_rename_ctrl_entry_changed_cb, pUserData);
	evas_object_smart_callback_add(en, "clicked", __mp_rename_ctrl_entry_clicked_cb, pUserData);
	evas_object_smart_callback_add(en, "longpressed", __mp_rename_ctrl_entry_long_pressed_cb, pUserData);

	__mp_rename_ctrl_set_save_file_name();
	MP_FREE_STRING(g_pRenameHandle->szOldName);
	g_pRenameHandle->szOldName = g_strdup(g_pRenameHandle->szSaveFileName);
	char *pNameUtf8 = elm_entry_utf8_to_markup(g_pRenameHandle->szSaveFileName);
	elm_entry_entry_set(en, pNameUtf8);
	MP_FREE_STRING(pNameUtf8);
	elm_object_focus_set(en, EINA_TRUE);
	g_pRenameHandle->pEntry = en;
	elm_entry_select_all(en);
	elm_entry_cursor_end_set(en);

	elm_object_part_content_set(pLayout, "elm.swallow.content", en);
	elm_object_content_set(g_pRenameHandle->pPopUpHandle, pLayout);

	g_pRenameHandle->pEntryClearBtn = elm_button_add(pLayout);
	elm_object_style_set(g_pRenameHandle->pEntryClearBtn, "clear");
	elm_object_part_content_set(pLayout, "elm.swallow.end", g_pRenameHandle->pEntryClearBtn);
	elm_object_signal_callback_add(g_pRenameHandle->pEntryClearBtn, "elm,action,click", "", __mp_rename_ctrl_entry_clear_cb, (void*)en);
	evas_object_show(g_pRenameHandle->pEntryClearBtn);

	g_pRenameHandle->pPopupCancelBtn = elm_button_add(g_pRenameHandle->pPopUpHandle);
	elm_object_style_set (g_pRenameHandle->pPopupCancelBtn, "popup");
	elm_object_domain_translatable_text_set(g_pRenameHandle->pPopupCancelBtn, VIDEOS_STRING, VIDEOS_COMMON_BUTTON_CANCEL_IDS);
	elm_object_part_content_set(g_pRenameHandle->pPopUpHandle, "button1", g_pRenameHandle->pPopupCancelBtn);
	evas_object_smart_callback_add(g_pRenameHandle->pPopupCancelBtn, "clicked", __mp_rename_ctrl_cancel_btn_cb, pUserData);

	g_pRenameHandle->pPopupOkBtn = elm_button_add(g_pRenameHandle->pPopUpHandle);
	elm_object_style_set (g_pRenameHandle->pPopupOkBtn, "popup");
	//elm_object_text_set(g_pRenameHandle->pPopupOkBtn, VIDEOS_RENAMEPOP_BUTTON_RENAME);
	elm_object_domain_translatable_text_set(g_pRenameHandle->pPopupOkBtn, VIDEOS_STRING, VIDEOS_COMMON_BUTTON_RENAME_IDS);
	elm_object_part_content_set(g_pRenameHandle->pPopUpHandle, "button2", g_pRenameHandle->pPopupOkBtn);
	evas_object_smart_callback_add(g_pRenameHandle->pPopupOkBtn, "clicked", __mp_rename_ctrl_ok_btn_cb, en);
	elm_object_disabled_set(g_pRenameHandle->pPopupOkBtn, EINA_TRUE);

	evas_object_show(g_pRenameHandle->pPopUpHandle);

	MP_FREE_STRING(szFileExt);
	MP_FREE_STRING(pVideoFile);
	return g_pRenameHandle->pBaseLayout;
}

bool mp_rename_ctrl_is_top_view()
{
	if (!g_pRenameHandle)
	{
		VideoLogInfo("RenamePopUp is not shown");
		return FALSE;
	}
	return TRUE;

}

void mp_rename_ctrl_set_focus()
{
	if (!g_pRenameHandle)
	{
		VideoLogInfo("RenamePopUp is not shown");
		return;
	}

	VideoLogInfo("");
	elm_object_focus_set(g_pRenameHandle->pEntry, EINA_TRUE);
}
void mp_rename_ctrl_focus_out_cb(void) {
	if (g_pRenameHandle->pPopUpHandle)
	{
		eext_object_event_callback_del(g_pRenameHandle->pPopUpHandle, EEXT_CALLBACK_BACK, __mp_rename_ctrl_cancel_btn_cb);
	}

	if (g_pRenameHandle->pPopUpHandle)
	{
		evas_object_smart_callback_del(g_pRenameHandle->pPopupCancelBtn, "clicked", __mp_rename_ctrl_cancel_btn_cb);
	}
	MP_DEL_OBJ(g_pRenameHandle->pPopupOkBtn);
	MP_DEL_OBJ(g_pRenameHandle->pPopupCancelBtn);
	MP_DEL_OBJ(g_pRenameHandle->pPopUpHandle);
}

void mp_rename_ctrl_delete_handle(void)
{
	VideoLogInfo("");
	if (!g_pRenameHandle) {
		VideoLogError("g_pRenameHandle is NULL");
		return;
	}
	if (g_pRenameHandle->pPopUpHandle)
	{
		eext_object_event_callback_del(g_pRenameHandle->pPopUpHandle, EEXT_CALLBACK_BACK, __mp_rename_ctrl_cancel_btn_cb);
	}

	if (g_pRenameHandle->pPopUpHandle)
	{
		evas_object_smart_callback_del(g_pRenameHandle->pPopupCancelBtn, "clicked", __mp_rename_ctrl_cancel_btn_cb);
	}

	if (g_pRenameHandle->pPopupOkBtn)
	{
		evas_object_smart_callback_del(g_pRenameHandle->pPopupOkBtn, "clicked", __mp_rename_ctrl_ok_btn_cb);
	}

	mp_widget_ctrl_enable_navi_handle_focus(g_pRenameHandle->pNaviFrameHandle);
	MP_DEL_IDLER(g_pRenameHandle->pSetLineEndIdler);
	MP_DEL_OBJ(g_pRenameHandle->pPopupOkBtn);
	MP_DEL_OBJ(g_pRenameHandle->pPopupCancelBtn);
	MP_DEL_OBJ(g_pRenameHandle->pPopUpHandle);
	MP_FREE_STRING(g_pRenameHandle->szSaveFileName);
	MP_FREE_STRING(g_pRenameHandle->szOldName);
	MP_FREE_STRING(g_pRenameHandle->szTempName);
	//evas_object_smart_callback_del(mp_util_get_main_conformat(), "virtualkeypad,state,on", __mp_rename_ctrl_popup_sip_on_cb);
	//evas_object_smart_callback_del(mp_util_get_main_conformat(), "virtualkeypad,state,off", __mp_rename_ctrl_popup_sip_off_cb);
	Evas_Object *pTopNaviFrame = NULL;
	pTopNaviFrame = elm_naviframe_item_pop(g_pRenameHandle->pNaviFrameHandle);
	evas_object_del(pTopNaviFrame);
	MP_FREE_STRING(g_pRenameHandle);
}

