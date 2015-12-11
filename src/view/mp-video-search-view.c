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


#include <Eina.h>
#include <Ecore_Evas.h>
#include <Elementary.h>
#include <glib.h>
#include <stdbool.h>
#include <app.h>

#include "mp-util.h"
#include "mp-video-log.h"
#include "mp-video-string-define.h"
#include "mp-video-value-define.h"
#include "mp-video-list-sort-ctrl.h"
#include "mp-util-media-service.h"
#include "mp-video-search-view.h"
#include "mp-video-view-popup-ctrl.h"
#include "mp-launching-video-displayer.h"
#include "mp-util-widget-ctrl.h"
#include "mp-video-util-db-controller.h"
#include "mp-video-list-view-as-ctrl.h"
#include "mp-video-list-option-ctrl.h"
#include "mp-video-rename-ctrl.h"
#include "mp-video-info-ctrl.h"
#include "vp-util.h"
#include "vp-file-util.h"
#include "mp-external-ug.h"
#include "mp-video-nocontent-layout.h"
#include "mp-rotate-ctrl.h"

#define DEF_BUF_STR_LEN					512
#define DEF_BUF_STR_LEN_MAX				4096
#define SEARCH_CHAR_LEN					(VIDEO_FILE_SEARCH_CHAR_LEN_MAX*4+1)

#define PREEDIT_START  					"<preedit>"
#define PREEDIT_END					"</preedit>"

typedef struct
{
	void *pNaviFrameHandle;
	void *pNaviFrameItem;
	void *pVideosGenlist;

	char szSearchWord[SEARCH_CHAR_LEN];
	char *szMediaUrl;
	Eina_List *SearchedIndexList;
	int nPlayVideoIndex;
	bool bShowNormalLayout;
	MpDbUpdateViewLev euLev;
	bool bUpdateList;
	char *pFolderPath;
	int nViewType;
	bool isViewActive;

	ListViewUpdateListItemCbFunc pChangeViewUserCbFunc;
	ViewUpdateListItemCbFunc pChangeViewUpdateFunc;
}st_VideoSearchViewHandle;

typedef struct
{
	//Evas_Object *pDimminglayout;
	Evas_Object *pSearchViewBaselayout;
	Evas_Object *pSearchBarlayout;
	Evas_Object *pSearchBarEntry;
	Evas_Object *pCancelButton;
	Evas_Object *pClearButton;
	Evas_Object *pNocontentsLayout;
}st_VideoSearchViewWidget;


st_VideoSearchViewHandle *g_pSearchViewHandle = NULL;
st_VideoSearchViewWidget *g_pSearchViewWidget = NULL;


Elm_Genlist_Item_Class VideoSearch_Itc;



// Pre-define function for building.
static void __mp_search_view_naviframe_transition_effect_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo);
static void __mp_search_view_append_video_items(void *pVideosGenlist, bool bSearchedView);
static int __mp_search_view_get_sort_type(void);
static void __mp_search_view_arrange_video_list(int nItemSortType, void *pVideosGenlist);
static void __mp_search_view_base_layout_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void __mp_search_view_pop(void *pNaviFrame);
//static void __mp_search_view_warning_popup_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo);
static void __mp_search_view_db_changed_cb(void *pUserData);
static void __mp_search_view_reset(void);
static void __mp_search_view_update_video_items(Evas_Object *pGenlist, int nGenItemIndex, int nPlayVideoIndex);



/*
static void __MpVideoSearchViewDimmingMouseUpCb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pSearchViewWidget) {
		VideoLogInfo("[ERR] g_pSearchViewWidget is NULL.");
		return ;
	}

	VideoLogInfo("");
	if (g_pSearchViewWidget->pDimminglayout) {
		elm_object_signal_emit(g_pSearchViewWidget->pDimminglayout, SIGNAL_SEARCH_VIEW_INTERNAL_DIMMING_HIDE, "");
	}
}
*/

static void __mp_search_view_reset(void)
{
		if (g_pSearchViewHandle)
		{
			mp_util_db_set_update_fun(g_pSearchViewHandle->euLev, NULL);
			g_pSearchViewHandle->nPlayVideoIndex = -1;
			evas_object_smart_callback_del(g_pSearchViewHandle->pNaviFrameHandle, "transition,finished", __mp_search_view_naviframe_transition_effect_cb);

			MP_DEL_OBJ(g_pSearchViewHandle->pVideosGenlist);

			MP_FREE_STRING(g_pSearchViewHandle->szMediaUrl);
			MP_FREE_STRING(g_pSearchViewHandle->pFolderPath);

			if (g_pSearchViewHandle->SearchedIndexList) {
				eina_list_free(g_pSearchViewHandle->SearchedIndexList);
				g_pSearchViewHandle->SearchedIndexList = NULL;
			}
			g_pSearchViewHandle->pNaviFrameHandle = NULL;
			g_pSearchViewHandle->pNaviFrameItem = NULL;
			g_pSearchViewHandle->pChangeViewUserCbFunc = NULL;
			g_pSearchViewHandle->pChangeViewUpdateFunc = NULL;

			free(g_pSearchViewHandle);
			g_pSearchViewHandle = NULL;
		}

		if (g_pSearchViewWidget)
		{
		/*
			if (g_pSearchViewWidget->pDimminglayout)
			{
				evas_object_event_callback_del(g_pSearchViewWidget->pDimminglayout, EVAS_CALLBACK_MOUSE_UP, __MpVideoSearchViewDimmingMouseUpCb);
				evas_object_del(g_pSearchViewWidget->pDimminglayout);
				g_pSearchViewWidget->pDimminglayout = NULL;
			}
		*/

			g_pSearchViewWidget->pSearchViewBaselayout = NULL;

			g_pSearchViewWidget->pSearchBarlayout = NULL;

			g_pSearchViewWidget->pSearchBarEntry = NULL;

			g_pSearchViewWidget->pCancelButton = NULL;

			g_pSearchViewWidget->pNocontentsLayout = NULL;


			free(g_pSearchViewWidget);
			g_pSearchViewWidget = NULL;
		}

}

void mp_search_view_destroy(void)
{

	if (g_pSearchViewHandle)
	{
		mp_util_db_set_update_fun(g_pSearchViewHandle->euLev, NULL);
		mp_util_db_set_backup_fun(g_pSearchViewHandle->euLev, NULL);
		g_pSearchViewHandle->nPlayVideoIndex = -1;
		evas_object_smart_callback_del(g_pSearchViewHandle->pNaviFrameHandle, "transition,finished", __mp_search_view_naviframe_transition_effect_cb);

		MP_DEL_OBJ(g_pSearchViewHandle->pVideosGenlist);
		MP_FREE_STRING(g_pSearchViewHandle->szMediaUrl);
		MP_FREE_STRING(g_pSearchViewHandle->pFolderPath);

		if (g_pSearchViewHandle->SearchedIndexList) {
			eina_list_free(g_pSearchViewHandle->SearchedIndexList);
			g_pSearchViewHandle->SearchedIndexList = NULL;
		}

		free(g_pSearchViewHandle);
		g_pSearchViewHandle = NULL;
	}

	if (g_pSearchViewWidget)
	{
	/*
		if (g_pSearchViewWidget->pDimminglayout)
		{
			evas_object_event_callback_del(g_pSearchViewWidget->pDimminglayout, EVAS_CALLBACK_MOUSE_UP, __MpVideoSearchViewDimmingMouseUpCb);
			evas_object_del(g_pSearchViewWidget->pDimminglayout);
			g_pSearchViewWidget->pDimminglayout = NULL;
		}
	*/

		MP_DEL_OBJ(g_pSearchViewWidget->pCancelButton);
		MP_DEL_OBJ(g_pSearchViewWidget->pSearchBarEntry);
		MP_DEL_OBJ(g_pSearchViewWidget->pSearchBarlayout);
		MP_DEL_OBJ(g_pSearchViewWidget->pNocontentsLayout);

		if (g_pSearchViewWidget->pSearchViewBaselayout != NULL)
		{
			evas_object_event_callback_del(g_pSearchViewWidget->pSearchViewBaselayout, EVAS_CALLBACK_DEL, __mp_search_view_base_layout_del_cb);
			MP_DEL_OBJ(g_pSearchViewWidget->pSearchViewBaselayout);
		}

		free(g_pSearchViewWidget);
		g_pSearchViewWidget = NULL;
	}
}

char *mp_search_view_search_markup_keyword(char *szOriginStr, char *szSearchWord, bool *bResult)
{
	if (!szOriginStr) {
		VideoLogError("[ERR] szOriginStr is NULL.");
		return NULL;
	}

	if (!szSearchWord) {
		VideoLogError("[ERR] szSearchWord is NULL.");
		return NULL;
	}

	if (!bResult) {
		VideoLogError("[ERR] bResult is NULL.");
		return NULL;
	}

	char szStr[DEF_BUF_STR_LEN + 1] = {0,};
	static char szTargetStr[DEF_BUF_STR_LEN + 1] = { 0, };
	int nWordLength = 0;
	int nSearchLength = 0;
	int nCounter = 0;
	bool bFound = FALSE;
	gchar *szMarkupTextStart = NULL;
	gchar *szMarkupTextEnd= NULL;
	gchar *szMarkupText= NULL;
	int r = 222;
	int g = 111;
	int b = 31;
	int a = 255;

	*bResult = TRUE;

	if (g_utf8_validate(szOriginStr, -1, NULL)) {
		strncpy(szStr, szOriginStr, DEF_BUF_STR_LEN);

		nWordLength = strlen(szStr);
		nSearchLength = strlen(szSearchWord);

		for (nCounter = 0; nCounter < nWordLength; nCounter++)
		{
			if (!strncasecmp(szSearchWord, &szStr[nCounter], nSearchLength)) {
				bFound = TRUE;
				break;
			}
		}

		*bResult = bFound;
		memset(szTargetStr, 0x00, DEF_BUF_STR_LEN + 1);

		if (bFound) {
			if (nCounter == 0) {
				szMarkupText = g_markup_escape_text(&szStr[0], nSearchLength);
				szMarkupTextEnd = g_markup_escape_text(&szStr[nSearchLength], nWordLength - nSearchLength);

				if (!szMarkupText && !szMarkupTextEnd) {
					VideoLogInfo("szMarkupText and szMarkupTextEnd are NULL.");
					return NULL;
				}

				//snprintf(szTargetStr, DEF_BUF_STR_LEN, "<match>%s</match>%s", szMarkupText, (char*)szMarkupTextEnd);
				snprintf(szTargetStr, DEF_BUF_STR_LEN, "<color=#%02x%02x%02x%02x>%s</color>%s", r, g, b, a,
						szMarkupText, (char*)szMarkupTextEnd);

				if (szMarkupText) {
					free(szMarkupText);
					szMarkupText = NULL;
				}

				if (szMarkupTextEnd) {
					free(szMarkupTextEnd);
					szMarkupTextEnd = NULL;
				}
			}
			else {
				szMarkupTextStart = g_markup_escape_text(&szStr[0], nCounter);
				szMarkupText = g_markup_escape_text(&szStr[nCounter], nSearchLength);
				szMarkupTextEnd =  g_markup_escape_text(&szStr[nCounter + nSearchLength], nWordLength - (nCounter + nSearchLength));

				if (!szMarkupTextStart && !szMarkupText && !szMarkupTextEnd) {
					VideoLogInfo("szMarkupTextStart and szMarkupText and szMarkupTextEnd are NULL.");
					return NULL;
				}

				//snprintf(szTargetStr, DEF_BUF_STR_LEN, "%s<match>%s</match>%s", (char*)szMarkupTextStart, szMarkupText, (char*)szMarkupTextEnd);
				snprintf(szTargetStr, DEF_BUF_STR_LEN, "%s<color=#%02x%02x%02x%02x>%s</color>%s", (char*)szMarkupTextStart,
						r, g, b, a, szMarkupText, (char*)szMarkupTextEnd);

				if (szMarkupText) {
					free(szMarkupText);
					szMarkupText = NULL;
				}

				if (szMarkupTextStart) {
					free(szMarkupTextStart);
					szMarkupTextStart = NULL;
				}

				if (szMarkupTextEnd) {
					free(szMarkupTextEnd);
					szMarkupTextEnd = NULL;
				}
			}
		}
		else {
			snprintf(szTargetStr, DEF_BUF_STR_LEN, "%s", szStr);
		}
	}

	VideoSecureLogInfo("szTargetStr=%s", szTargetStr);

	return szTargetStr;
}


static void __mp_search_view_trans_finished_cb(void *pUserData, Evas_Object *obj, void *event_info)
{
	if (!obj)
	{
		VideoLogError("[ERR] g_pListRemoveViewHandle is NULL.");
		return;
	}

	mp_widget_ctrl_enable_navi_handle_focus(obj);

	evas_object_smart_callback_del(obj, "transition,finished", __mp_search_view_trans_finished_cb);
}

static Eina_Bool mp_search_view_back_btn_cb(void *pUserData, Elm_Object_Item *pItem)
{
	if (!g_pSearchViewHandle)
	{
		VideoLogInfo("[ERR] g_pListRemoveViewHandle is NULL.");
		return EINA_TRUE;
	}

	// Register transition finished callback.
	mp_widget_ctrl_disable_navi_handle_focus_except_item(g_pSearchViewHandle->pNaviFrameHandle, g_pSearchViewHandle->pNaviFrameItem);
	evas_object_smart_callback_add(g_pSearchViewHandle->pNaviFrameHandle, "transition,finished", __mp_search_view_trans_finished_cb, (void *)g_pSearchViewHandle->pNaviFrameHandle);

	__mp_search_view_pop(g_pSearchViewHandle->pNaviFrameHandle);

	return EINA_TRUE;
}

////////////////////////////////////////////////
// Genlist callback
void __mp_search_view_naviframe_transition_effect_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pSearchViewHandle)
	{
		VideoLogError("[ERR] No exist g_pSearchViewHandle.");
		return;
	}

	VideoLogInfo("");

	//Evas_Object *pTmpContents = NULL;
	//Elm_Object_Item *pTmpItem = NULL;

	if (!g_pSearchViewHandle->pNaviFrameHandle)
	{
		VideoLogInfo("[ERR] No exist naviframe handle.");
		return;
	}

	//pTmpItem = elm_naviframe_top_item_get(g_pSearchViewHandle->pNaviFrameHandle);
	//pTmpContents = elm_object_item_content_get(pTmpItem);

	elm_object_focus_set(g_pSearchViewWidget->pSearchBarEntry, EINA_TRUE);
}


// Genlist item callback.
/*
static void __mp_search_view_warning_popup_cb(void *pUserData,
		Evas_Object *pObject, void *pEventInfo)
{

	VideoLogInfo("");

	mp_util_delete_popup_handle();
}
*/

static void __mp_search_view_select_video_item_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pSearchViewHandle)
	{
		VideoLogError("g_pSearchViewHandle is not exist.");
		return;
	}

	int nVideoItemIndex = 0;
	Elm_Object_Item *pItem = (Elm_Object_Item *)pEventInfo;
	Elm_Object_Item *pSelectedItem = elm_genlist_selected_item_get(pObject);

	if (pItem)
	{
		nVideoItemIndex = (int)elm_object_item_data_get(pItem);
	}
	else
	{

		if (pSelectedItem)
		{
			elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
		}

		return;
	}

	VideoLogInfo("nVideoItemIndex : %d", nVideoItemIndex);

	if (!pSelectedItem)
	{
		VideoLogError("pSelectedItem is NULL");
		return;
	}
	elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);

	Evas_Object *pPopup = evas_object_data_get(pObject, "popup"); // Get popup
	if (pPopup)
	{
		VideoLogError("long press is called");
		return;  // If popup exists, do nothing
	}

	if (!mp_launch_video_allow_check())
	{
		return;
	}

	if (nVideoItemIndex < 0)
	{
		VideoLogWarning("nVideoItemIndex < 0 ===>>> RETURN");
		return;
	}

	if (!mp_util_call_off())
	{
		mp_util_ticker_toast_popup(VIDEOS_NOTIPOP_MSG_UNABLE_TO_PLAY_VIDEO_DURING_CALL, false, false);
		return;
	}

	g_pSearchViewHandle->nPlayVideoIndex = nVideoItemIndex;

	if (g_pSearchViewHandle->szMediaUrl)
	{
		free(g_pSearchViewHandle->szMediaUrl);
		g_pSearchViewHandle->szMediaUrl = NULL;
	}

	g_pSearchViewHandle->szMediaUrl = mp_util_svc_get_video_url(nVideoItemIndex);

	if (g_pSearchViewHandle->szMediaUrl)
	{
		elm_object_focus_allow_set(g_pSearchViewWidget->pSearchBarlayout, EINA_FALSE);
		elm_object_focus_allow_set(g_pSearchViewWidget->pSearchBarEntry, EINA_FALSE);
		elm_object_focus_set(pObject, EINA_TRUE);
		mp_launch_video_play(g_pSearchViewHandle->szMediaUrl, MP_PLAYER_TYPE_VIDEO, NULL);

		free(g_pSearchViewHandle->szMediaUrl);
		g_pSearchViewHandle->szMediaUrl = NULL;
	}
	else
	{
		VideoLogError("[Err] Fail to get video uri from media service.");
	}
}

char *mp_search_view_get_duration_time(int nVideoItemIndex)
{

	char szTmpStr[DEF_BUF_STR_LEN_MAX] = {0, };
	char szTmpStrDurationTime[DEF_BUF_STR_LEN_MAX] = {0, };
	char szTmpStrLastPlayedPos[DEF_BUF_STR_LEN_MAX] = {0, };

	unsigned int nVideoInfoDuration = mp_util_svc_get_video_duration_time(nVideoItemIndex);
	unsigned int nVideoInfoLastTime = mp_util_svc_get_video_last_played_pos(nVideoItemIndex);

	mp_util_convert_time(nVideoInfoDuration, szTmpStrDurationTime, FALSE);
	mp_util_convert_time(nVideoInfoLastTime, szTmpStrLastPlayedPos, TRUE);

	snprintf(szTmpStr, DEF_BUF_STR_LEN_MAX, "%s/%s", szTmpStrLastPlayedPos, szTmpStrDurationTime);

	return strdup(szTmpStr);
}

char *mp_search_view_get_label_of_video_item_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{

	int nVideoItemIndex = (int)pUserData;

	if (!strcmp(pPart, "elm.text.main.left.top"))
	{
		char *szTitle = NULL;
		char *szTitleUtf8 = NULL;
		char *szTitleWithSearchWordColor = NULL;
		bool bResult = FALSE;

		szTitle = mp_util_svc_get_video_title(nVideoItemIndex);

		if (strlen(g_pSearchViewHandle->szSearchWord) > 0)
		{
			szTitleWithSearchWordColor = mp_search_view_search_markup_keyword(szTitle, (char*)g_pSearchViewHandle->szSearchWord, &bResult);
			if (bResult)
			{
				if (szTitle)
				{
					free(szTitle);
					szTitle = NULL;
				}

				if (szTitleWithSearchWordColor) {
				VideoLogInfo("szTitleWithSearchWordColor : %s", szTitleWithSearchWordColor);
				return strdup(szTitleWithSearchWordColor);
				} else {
					return NULL;
				}
			}
		}
		szTitleUtf8 = elm_entry_utf8_to_markup(szTitle);

		if (szTitle)
		{
			free(szTitle);
			szTitle = NULL;
		}

		return szTitleUtf8;
	}
	else if (!strcmp(pPart, "elm.text.sub.left.bottom"))
	{
		return mp_util_svc_get_duration_str_time(nVideoItemIndex);
	}
/*
//	else if (!strcmp(pPart, "elm.text.3"))
	else if (!strcmp(pPart, "elm.text.sub.right.bottom"))
	{
		unsigned long long nFilesize = 0;

		nFilesize = mp_util_svc_get_video_item_size(nVideoItemIndex);
		char *szFileSize = mp_util_get_file_size(nFilesize);
		char *szTitleUtf8 = NULL;
		if (szFileSize)
		{
			szTitleUtf8 = elm_entry_utf8_to_markup(szFileSize);
			free(szFileSize);
			szFileSize = NULL;
		}

		return szTitleUtf8;
	}
*/	else
	{
	}

	return NULL;
}

Evas_Object *mp_search_view_get_icon_of_video_item_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{
	if (!strcmp(pPart, "elm.icon.1"))
	{
		VideoLogError("pPart: elm.icon.1 - thumbnail");

		int nVideoItemIndex = (int)pUserData;
		Evas_Object *pLayout = elm_layout_add(pObject);

		char *pThumbIconUri = mp_util_svc_get_video_thumbnail(nVideoItemIndex);
		Evas_Object *pBg = NULL;
		char edj_path[1024] = {0};

		char *path = app_get_resource_path();
		snprintf(edj_path, 1024, "%s/%s/%s", path , "edje", VIDEO_CUSTOM_THEME);

		pLayout = elm_layout_add(pObject);
		elm_layout_file_set(pLayout, edj_path, "listview.thumbnail.layout");

		if (!pThumbIconUri || !vp_file_exists(pThumbIconUri))
		{
			MP_FREE_STRING(pThumbIconUri);
		}

		pBg = mp_util_create_preload_image(pLayout, pThumbIconUri, VIDEO_ICON_WIDTH);

		if (!elm_layout_content_set(pLayout, "elm.thumbnail.icon", pBg))
			VideoLogError("elm_layout_content_set() is failed.");

		if (pBg)
			evas_object_show(pBg);

		MP_FREE_STRING(pThumbIconUri);

		int nLastPlayedTime = mp_util_svc_get_video_last_played_pos(nVideoItemIndex);
		int nDurationTime = mp_util_svc_get_video_duration_time(nVideoItemIndex);

		if (nLastPlayedTime > 0)
		{
			double nRatio = 0.0;

			if (nDurationTime > 0) {
				nRatio = ((double)nLastPlayedTime) /
						((double)nDurationTime);
			}

			if (nRatio < 0.05) {
				nRatio = 0.05;
			}

			Evas_Object *pProgressBar = elm_progressbar_add(pObject);
			elm_object_style_set(pProgressBar, "video/list_progress");
			elm_progressbar_horizontal_set(pProgressBar, EINA_TRUE);
			evas_object_size_hint_align_set(pProgressBar, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(pProgressBar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_progressbar_value_set(pProgressBar, nRatio);

			if (!elm_layout_content_set(pLayout, "elm.progressbar.icon", pProgressBar))
				VideoLogError("elm_layout_content_set() is failed.");

			if (pProgressBar)	{
				evas_object_show(pProgressBar);
			}
		}

		if (!mp_util_create_video_sub_icon(pLayout, nVideoItemIndex))	{
			VideoLogWarning("Local File or Create sub icon is failed.", nVideoItemIndex);
		}

		if (pLayout)	{
			evas_object_show(pLayout);
		}

		return pLayout;
	}

	return NULL;
}
//
////////////////////////////////////////////////

////////////////////////////////////////////////
// Search bar widget about relevant items of genlist.
static void __mp_search_view_del_no_contents_layout(void)
{
	VideoLogInfo("");

	if (g_pSearchViewWidget->pNocontentsLayout)
	{
		evas_object_del(g_pSearchViewWidget->pNocontentsLayout);
		g_pSearchViewWidget->pNocontentsLayout = NULL;
	}
}

static void *__mp_search_view_add_no_contents_layout(void *pParent)
{
	if (!g_pSearchViewWidget) {
		VideoLogError("g_pSearchViewWidget == NULL");
		return NULL;
	}
	VideoLogInfo("");

	__mp_search_view_del_no_contents_layout();

	if (elm_entry_is_empty(g_pSearchViewWidget->pSearchBarEntry)) {
		VideoLogInfo("entry is empty, no need append no search view");
		return NULL;
	}

	//mp_util_virtual_keypad_disabled_set(EINA_TRUE);
	g_pSearchViewWidget->pNocontentsLayout = mp_create_nocontent_layout(pParent, VIDEOS_SEARCHVIEW_MSG_NO_SEARCH_RESULTS, NULL);

	evas_object_show(g_pSearchViewWidget->pNocontentsLayout);

	return g_pSearchViewWidget->pNocontentsLayout;
}

static void __mp_search_view_is_existed_no_content_layout(void)
{
	if (!g_pSearchViewWidget)
	{
		VideoLogError("g_pSearchViewWidget == NULL");
		return;
	}

	VideoLogInfo("");

	if (g_pSearchViewWidget->pNocontentsLayout)
	{
		elm_object_part_content_unset(g_pSearchViewWidget->pSearchViewBaselayout, "elm.swallow.content");
		__mp_search_view_del_no_contents_layout();
		//mp_util_virtual_keypad_disabled_set(EINA_FALSE);
		elm_object_part_content_set(g_pSearchViewWidget->pSearchViewBaselayout, "elm.swallow.content", g_pSearchViewHandle->pVideosGenlist);
		//elm_object_part_content_set(g_pSearchViewWidget->pSearchViewBaselayout, "elm.swallow.content", g_pSearchViewWidget->pDimminglayout);
		//evas_object_show(g_pSearchViewWidget->pDimminglayout);
		if (g_pSearchViewHandle->pVideosGenlist)
			evas_object_show(g_pSearchViewHandle->pVideosGenlist);
	}
}

static void __mp_search_view_is_existed_genlist(void)
{
	if (!g_pSearchViewWidget)
	{
		VideoLogError("g_pSearchViewWidget == NULL");
		return;
	}


	if (!g_pSearchViewWidget->pNocontentsLayout)
	{
		elm_object_part_content_unset(g_pSearchViewWidget->pSearchViewBaselayout, "elm.swallow.content");
		//evas_object_hide(g_pSearchViewWidget->pDimminglayout);
		if (g_pSearchViewHandle->pVideosGenlist)
			evas_object_hide(g_pSearchViewHandle->pVideosGenlist);
	}
	else
	{
		VideoLogError("g_pSearchViewWidget->pNocontentsLayout is existed.");
	}
}

void mp_search_view_search_genlist_item(const char *szSearchWord)
{
	if (!g_pSearchViewHandle)
	{
		VideoLogError("g_pSearchViewHandle == NULL");
		return;
	}

	if (!szSearchWord)
	{
		VideoLogError("szSearchWord == NULL");
		return;
	}

	VideoLogInfo("");

	//int nVideoListSize = 0;

	//mp_util_svc_destory_video_list();
	//mp_util_svc_extract_video_list_for_search_view(szSearchWord, g_pSearchViewHandle->nViewType);
	//nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	//VideoLogInfo("nVideoListSize : %d", nVideoListSize);

	if (g_pSearchViewHandle->SearchedIndexList) {
		eina_list_free(g_pSearchViewHandle->SearchedIndexList);
		g_pSearchViewHandle->SearchedIndexList = NULL;
	}
	mp_util_svc_index_list_for_search_view(szSearchWord, (void **)&g_pSearchViewHandle->SearchedIndexList);

	if (g_pSearchViewHandle->SearchedIndexList)
	{
		memset(g_pSearchViewHandle->szSearchWord, 0, sizeof(char) * SEARCH_CHAR_LEN);
		strncpy(g_pSearchViewHandle->szSearchWord, szSearchWord, SEARCH_CHAR_LEN - 1);
		__mp_search_view_is_existed_no_content_layout();
		elm_genlist_clear(g_pSearchViewHandle->pVideosGenlist);
		__mp_search_view_append_video_items(g_pSearchViewHandle->pVideosGenlist, true);
		//elm_object_signal_emit(g_pSearchViewWidget->pDimminglayout, SIGNAL_SEARCH_VIEW_INTERNAL_DIMMING_HIDE, "");
	}
	else
	{
		__mp_search_view_is_existed_genlist();
		void *pNoContentLayout = __mp_search_view_add_no_contents_layout(g_pSearchViewWidget->pSearchViewBaselayout);
		elm_object_part_content_set(g_pSearchViewWidget->pSearchViewBaselayout, "elm.swallow.content", pNoContentLayout);
	}
}

static char *mp_search_view_get_fact_str(const char *str)
{
	if (!str)
	{
		VideoLogWarning("str == NULL");
		return NULL;
	}

	int 	nLen 					= strlen(str);
	if (nLen >= SEARCH_CHAR_LEN -1)
	{
		VideoLogError("str length is too large : [%d]", nLen);
		return NULL;
	}

	char 	des_str[SEARCH_CHAR_LEN] 	= {0,};
	char 	*temp_str 				= strstr(str, PREEDIT_START);
	char	*pTempStr2				= strstr(str, PREEDIT_END);

	if (temp_str != NULL && pTempStr2 != NULL)
	{
		if (str != temp_str)
		{
			strncpy(des_str, str, temp_str - str);
		}

		if (strlen(des_str) != 0)
		{
			strncat(des_str, temp_str + strlen(PREEDIT_START), pTempStr2 - temp_str - strlen(PREEDIT_START));
		}
		else
		{
			strncpy(des_str, temp_str + strlen(PREEDIT_START), pTempStr2 - temp_str - strlen(PREEDIT_START));
		}
	}
	else
	{
		strncpy(des_str, str, SEARCH_CHAR_LEN -1);
	}
	VideoSecureLogInfo("get string = %s:%s", des_str, str);
	return strdup(des_str);

}

static void mp_search_view_check_landscape()
{
	if (!g_pSearchViewWidget) {
		return;
	}
	bool bLandscape = mp_rotate_ctrl_check_landspace();

	const char *temp = elm_object_text_get(g_pSearchViewWidget->pSearchBarEntry);
	char *TempUtf8 = elm_entry_markup_to_utf8(temp);
	char *szChangedStr = mp_search_view_get_fact_str(TempUtf8);

	if (bLandscape) {
		elm_object_signal_emit(g_pSearchViewWidget->pSearchBarlayout, "elm,state,show,landscape", "elm");

		if (szChangedStr != NULL && strlen(szChangedStr) > 0) {
			elm_object_signal_emit(g_pSearchViewWidget->pSearchBarlayout, "elm,state,show,clear_button,landscape", "elm");
		}
	} else {
		elm_object_signal_emit(g_pSearchViewWidget->pSearchBarlayout, "elm,state,show,portrait", "elm");

		if (szChangedStr != NULL && strlen(szChangedStr) > 0) {
			elm_object_signal_emit(g_pSearchViewWidget->pSearchBarlayout, "elm,state,show,clear_button,portrait", "elm");
		}
	}

	MP_FREE_STRING(szChangedStr);
	MP_FREE_STRING(TempUtf8);
}

static void __mp_search_view_entry_changed_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");

	if (!g_pSearchViewHandle || !pUserData || !pObject) {
		VideoLogError("g_pSearchViewHandle == NULL");
		return;
	}

	char *szChangedStr = NULL;

//	if (elm_object_focus_get(pUserData)) {
		if (elm_entry_is_empty(pObject)) {
			elm_object_signal_emit(pUserData, "elm,state,eraser,hide", "elm");
			elm_object_disabled_set(g_pSearchViewWidget->pClearButton, EINA_TRUE);
			elm_object_style_set(g_pSearchViewWidget->pClearButton, "clear_dim");
		} else {
			elm_object_signal_emit(pUserData, "elm,state,eraser,show", "elm");
			elm_object_disabled_set(g_pSearchViewWidget->pClearButton, EINA_FALSE);
			elm_object_style_set(g_pSearchViewWidget->pClearButton, "clear");
		}
//	}

	if (!elm_entry_is_empty(pObject)) {
		elm_object_signal_emit(pUserData, "elm,state,guidetext,hide", "elm");
	}

	if (mp_util_svc_get_number_of_video_item_by_type() < 1) {
		VideoLogWarning("There don't have any videos.");
		__mp_search_view_is_existed_genlist();
		void *pNoContentLayout = __mp_search_view_add_no_contents_layout(g_pSearchViewWidget->pSearchViewBaselayout);
		elm_object_part_content_set(g_pSearchViewWidget->pSearchViewBaselayout, "elm.swallow.content", pNoContentLayout);
		return;
	}

	const char *temp = elm_object_text_get(pObject);
	char *TempUtf8 = elm_entry_markup_to_utf8(temp);
	szChangedStr = mp_search_view_get_fact_str(TempUtf8);
	MP_FREE_STRING(TempUtf8);

	VideoSecureLogInfo("Changed word in entry widget : %s", szChangedStr);

	if (szChangedStr != NULL && strlen(szChangedStr) > 0) {
		elm_object_part_content_set(g_pSearchViewWidget->pSearchBarlayout, "searchbar_clear_btn", g_pSearchViewWidget->pClearButton);
		mp_search_view_check_landscape();
		mp_search_view_search_genlist_item(szChangedStr);
	} else {
		elm_object_part_content_unset(g_pSearchViewWidget->pSearchBarlayout, "searchbar_clear_btn");
		evas_object_hide(g_pSearchViewWidget->pClearButton);
		memset(g_pSearchViewHandle->szSearchWord, 0, sizeof(char) * SEARCH_CHAR_LEN);
		//strncpy(g_pSearchViewHandle->szSearchWord, szChangedStr, SEARCH_CHAR_LEN - 1);
		__mp_search_view_is_existed_no_content_layout();
		__mp_search_view_arrange_video_list(__mp_search_view_get_sort_type(), g_pSearchViewHandle->pVideosGenlist);
		//elm_object_signal_emit(g_pSearchViewWidget->pDimminglayout, SIGNAL_SEARCH_VIEW_INTERNAL_DIMMING_SHOW, "");
	}
	MP_FREE_STRING(szChangedStr);

}

static void __mp_search_view_focused_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");

	if (!pUserData || !g_pSearchViewWidget || !g_pSearchViewHandle)
	{
		VideoLogError("[ERR]No have pUserData");
		return;
	}

	if (elm_entry_is_empty(pObject))
	{
		elm_object_signal_emit(pUserData, "elm,state,eraser,hide", "elm");
	}
	else
	{
		elm_object_signal_emit(pUserData, "elm,state,eraser,show", "elm");
	}

	elm_object_signal_emit(pUserData, "elm,state,guidetext,hide", "elm");
	elm_object_signal_emit(pUserData, "cancel,in", "");
	elm_object_focus_allow_set(pObject,EINA_TRUE);
}

static void __mp_search_view_unfocused_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");

	if (!pUserData || !g_pSearchViewWidget || !g_pSearchViewHandle)
	{
		VideoLogError("[ERR]No have pUserData");
		return;
	}

	if (elm_entry_is_empty(pObject))
	{
		elm_object_signal_emit(pUserData, "elm,state,guidetext,show", "elm");
	}
	elm_object_signal_emit(pUserData, "elm,state,eraser,hide", "elm");
	//elm_object_focus_allow_set(pObject, EINA_FALSE);
}


static void __mp_search_view_activated_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");

	if (!pObject)
	{
		VideoLogError("[ERR]No have pObject");
		return;
	}

	elm_object_focus_set(pObject, FALSE);
}

static void __mp_search_view_entry_max_len_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");

	mp_util_ticker_toast_popup(VIDEOS_NOTIPOP_MSG_MAX_NUM_OF_CHARACTERS, false, false);
	//mp_util_info_popup(VIDEOS_NOTIPOP_MSG_MAX_NUM_OF_CHARACTERS, ___MpVideoSearchViewPopUpBackCb, pUserData, VIDEOS_STRING);
}

static void __mp_search_view_eraser_clicked_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");
	if (!pUserData)
	{
		VideoLogError("error userdata");
		return;
	}
	elm_object_part_content_unset(g_pSearchViewWidget->pSearchBarlayout, "searchbar_clear_btn");
	evas_object_hide(g_pSearchViewWidget->pClearButton);
	Evas_Object *pEntry = (Evas_Object *)pUserData;
	elm_entry_entry_set(pEntry, "");
	elm_object_focus_allow_set(pObject,EINA_TRUE);
	elm_object_focus_set(pObject, EINA_TRUE);
}

static void __mp_search_view_bg_clicked_cb(void *pUserData, Evas_Object *pObject, const char *emission, const char *source)
{
	VideoLogInfo("");
	if (!pUserData)
	{
		VideoLogError("error userdata");
		return;
	}

	//elm_object_focus_set(pUserData, EINA_TRUE);
	elm_object_tree_focus_allow_set(pUserData, EINA_TRUE);
}

/*void mp_search_view_search_cancel_btn_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");

	if (!g_pSearchViewHandle)
	{
		VideoLogError("g_pSearchViewHandle is not exist.");
		return;
	}

	elm_naviframe_item_pop(g_pSearchViewHandle->pNaviFrameHandle);
}*/

static void __mp_search_view_search_symbol_clicked_cb(void *pUserData, Evas_Object *pObject, const char *emission, const char *source)
{
	VideoLogInfo("");
	if (!pUserData)
	{
		VideoLogError("error userdata");
		return;
	}
	elm_object_focus_set(pUserData, EINA_TRUE);
}
//
////////////////////////////////////////////////

////////////////////////////////////////////////
//
int __mp_search_view_get_sort_type(void)
{

	int nSortType = MP_MEDIA_SORT_BY_RECENTLY_VIEWED;

	switch (mp_sort_ctrl_get_sort_state())
	{
	/*case MP_LIST_SORT_BY_RECENTLY_VIEWED:
		nSortType = MP_MEDIA_SORT_BY_RECENTLY_VIEWED;
		break;*/

	case MP_LIST_SORT_BY_RECENTLY_ADDED:
		nSortType = MP_MEDIA_SORT_BY_RECENTLY_ADDED;
		break;

	case MP_LIST_SORT_BY_NAME:
		nSortType = MP_MEDIA_SORT_BY_NAME;
		break;

	/*case MP_LIST_SORT_BY_SIZE:
		nSortType = MP_MEDIA_SORT_BY_SIZE;
		break;

	case MP_LIST_SORT_BY_TYPE:
		nSortType = MP_MEDIA_SORT_BY_TYPE;
		break;*/
	}

	return nSortType;
}

void __mp_search_view_append_video_items(void *pVideosGenlist, bool bSearchedView)
{
	VideoLogInfo("");

	if (!pVideosGenlist)
	{
		VideoLogInfo("No exist genlist object.");
		return;
	}

	int nIndex = 0;
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();

//	VideoSearch_Itc.item_style = "video/search_list";
	VideoSearch_Itc.item_style = "2line.top";
	VideoSearch_Itc.func.text_get = (void*)mp_search_view_get_label_of_video_item_cb;
	VideoSearch_Itc.func.content_get = (void*)mp_search_view_get_icon_of_video_item_cb;
	VideoSearch_Itc.func.state_get = NULL;
	VideoSearch_Itc.func.del = NULL;
	if (bSearchedView) {
		Eina_List *pIterateList = NULL;
		void *nTempIndex = (void *)0;
		EINA_LIST_FOREACH(g_pSearchViewHandle->SearchedIndexList, pIterateList, nTempIndex)
		{
			nIndex = (int)nTempIndex;
			VideoLogInfo("searched nIndex : %d", nIndex);
			elm_genlist_item_append(pVideosGenlist, &VideoSearch_Itc, (void*)nIndex,
				NULL, ELM_GENLIST_ITEM_NONE, __mp_search_view_select_video_item_cb, NULL);
		}

	} else {
		for (nIndex = 0; nIndex < nVideoListSize; nIndex++)
		{
			VideoLogInfo("nIndex : %d", nIndex);
			elm_genlist_item_append(pVideosGenlist, &VideoSearch_Itc, (void*)nIndex,
				NULL, ELM_GENLIST_ITEM_NONE, __mp_search_view_select_video_item_cb, NULL);
		}
	}
}

void __mp_search_view_arrange_video_list(int nItemSortType, void *pVideosGenlist)
{
	VideoLogInfo("");

	if (!pVideosGenlist)
	{
		VideoLogInfo("No exist genlist object.");
		return;
	}

	if (!g_pSearchViewHandle || !g_pSearchViewWidget)
	{
		VideoLogInfo("Handles are not existed about search view.");
		return;
	}
	if (g_pSearchViewHandle->SearchedIndexList) {
		eina_list_free(g_pSearchViewHandle->SearchedIndexList);
		g_pSearchViewHandle->SearchedIndexList = NULL;
	}
	VideoLogInfo("");

	int nVideoListSize = 0;
	/*Don't need to re-make new video list, because searched data only works current video list*/
	if (g_pSearchViewHandle->nViewType == MP_LIST_VIEW_AS_FOLDER_LIST && !mp_folder_view_is_item_view()) {
		mp_util_svc_destory_video_list();
		mp_util_svc_extract_video_list_by_item_type(nItemSortType, mp_view_ctrl_get_list_type());
	}
	//MpUtilMediaSvcExtractVideoListByViewType(nItemSortType, nViewType);
	nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	VideoLogInfo("nVideoListSize : %d", nVideoListSize);

	elm_genlist_clear(pVideosGenlist);

	if (nVideoListSize > 0)
	{
		__mp_search_view_append_video_items(pVideosGenlist, false);
	}

	evas_object_show(pVideosGenlist);
}

static void __mp_search_view_clicked_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");
	if (!pObject)
	{
		VideoLogError("[ERR]No have pObject");
		return;
	}
	elm_object_focus_allow_set(pObject,EINA_TRUE);
	elm_object_focus_set(pObject, EINA_TRUE);
}

static void __mp_search_view_soft_back_button_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	VideoLogInfo(" ");
		Evas_Object *pTopNaviFrame = NULL;
		pTopNaviFrame = elm_naviframe_item_pop(g_pSearchViewHandle->pNaviFrameHandle);
		evas_object_del(pTopNaviFrame);
}

static void __mp_search_view_rotate_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!g_pSearchViewWidget) {
		VideoLogError("__mp_search_view_rotate_cb IS null");
		return;
	}
	//mp_util_virtual_keypad_disabled_set(EINA_TRUE);
	bool bLandscape = mp_rotate_ctrl_check_landspace();

	const char *temp = elm_object_text_get(g_pSearchViewWidget->pSearchBarEntry);
	char *TempUtf8 = elm_entry_markup_to_utf8(temp);
	char *szChangedStr = mp_search_view_get_fact_str(TempUtf8);

	if (bLandscape) {
		elm_object_signal_emit(g_pSearchViewWidget->pSearchBarlayout, "elm,state,show,portrait", "elm");

		if (szChangedStr != NULL && strlen(szChangedStr) > 0) {
			elm_object_signal_emit(g_pSearchViewWidget->pSearchBarlayout, "elm,state,show,clear_button,portrait", "elm");
		}
	} else {
		elm_object_signal_emit(g_pSearchViewWidget->pSearchBarlayout, "elm,state,show,landscape", "elm");

		if (szChangedStr != NULL && strlen(szChangedStr) > 0) {
			elm_object_signal_emit(g_pSearchViewWidget->pSearchBarlayout, "elm,state,show,clear_button,landscape", "elm");
		}
	}

	MP_FREE_STRING(szChangedStr);
	MP_FREE_STRING(TempUtf8);
}

static Evas_Object *__mp_search_view_init_internal_layout(void *pParent)
{
	VideoLogInfo("");

	if (!pParent) {
		VideoLogError("[ERR]");
		return NULL;
	}

	if (g_pSearchViewWidget->pSearchViewBaselayout) {
		evas_object_del(g_pSearchViewWidget->pSearchViewBaselayout);
		g_pSearchViewWidget->pSearchViewBaselayout = NULL;
	}
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s/%s/%s", path , "edje", VIDEO_BASIC_NAVIFRAME_EDJ);

	// Create search view base layout.
	g_pSearchViewWidget->pSearchViewBaselayout = elm_layout_add(pParent);
	elm_layout_file_set(g_pSearchViewWidget->pSearchViewBaselayout, edj_path, "search_view_layout");

	// Create search bar layout.
	g_pSearchViewWidget->pSearchBarlayout = elm_layout_add(g_pSearchViewWidget->pSearchViewBaselayout);
	elm_layout_file_set(g_pSearchViewWidget->pSearchBarlayout, edj_path, "searchbar_video");
	elm_object_part_content_set(g_pSearchViewWidget->pSearchViewBaselayout, "search_bar", g_pSearchViewWidget->pSearchBarlayout);
	elm_object_signal_callback_add(g_pSearchViewWidget->pSearchBarlayout, "elm,action,click", "back_button_callback", __mp_search_view_soft_back_button_cb, NULL);
	elm_object_signal_emit(g_pSearchViewWidget->pSearchBarlayout, "elm,state,show,searchbar", "elm");

	// Create entry.
	g_pSearchViewWidget->pSearchBarEntry = elm_entry_add(g_pSearchViewWidget->pSearchBarlayout);
	elm_entry_single_line_set(g_pSearchViewWidget->pSearchBarEntry, EINA_TRUE);
	elm_entry_scrollable_set(g_pSearchViewWidget->pSearchBarEntry, EINA_TRUE);
	elm_object_domain_translatable_part_text_set(g_pSearchViewWidget->pSearchBarEntry, "elm.guide", VIDEOS_STRING, VIDEOS_CTXPOP_MENU_SEARCH_IDS);
	elm_object_domain_part_text_translatable_set(g_pSearchViewWidget->pSearchBarEntry, "elm.guide", VIDEOS_STRING, EINA_TRUE);

	elm_entry_scrollable_set(g_pSearchViewWidget->pSearchBarEntry, EINA_TRUE);
	elm_entry_single_line_set(g_pSearchViewWidget->pSearchBarEntry, EINA_TRUE);
	elm_entry_cnp_mode_set(g_pSearchViewWidget->pSearchBarEntry, ELM_CNP_MODE_PLAINTEXT);
	elm_entry_prediction_allow_set(g_pSearchViewWidget->pSearchBarEntry, EINA_TRUE);
	evas_object_smart_callback_add(g_pSearchViewWidget->pSearchBarEntry, "changed", __mp_search_view_entry_changed_cb, g_pSearchViewWidget->pSearchBarlayout);
	evas_object_smart_callback_add(g_pSearchViewWidget->pSearchBarEntry, "focused", __mp_search_view_focused_cb, g_pSearchViewWidget->pSearchBarlayout);
	evas_object_smart_callback_add(g_pSearchViewWidget->pSearchBarEntry, "unfocused", __mp_search_view_unfocused_cb, g_pSearchViewWidget->pSearchBarlayout);
	evas_object_smart_callback_add(g_pSearchViewWidget->pSearchBarEntry, "preedit,changed", __mp_search_view_entry_changed_cb, g_pSearchViewWidget->pSearchBarlayout);
	evas_object_smart_callback_add(g_pSearchViewWidget->pSearchBarEntry, "activated", __mp_search_view_activated_cb, g_pSearchViewWidget->pSearchBarlayout);
	evas_object_smart_callback_add(g_pSearchViewWidget->pSearchBarEntry, "clicked", __mp_search_view_clicked_cb, g_pSearchViewWidget->pSearchBarlayout);

	g_pSearchViewWidget->pClearButton = elm_button_add(g_pSearchViewWidget->pSearchBarlayout);
	elm_object_style_set(g_pSearchViewWidget->pClearButton, "clear_dim");
	elm_object_disabled_set(g_pSearchViewWidget->pClearButton, EINA_TRUE);
	evas_object_smart_callback_add(g_pSearchViewWidget->pClearButton, "clicked", __mp_search_view_eraser_clicked_cb, g_pSearchViewWidget->pSearchBarEntry);

	static Elm_Entry_Filter_Limit_Size limit_filter_data;
	limit_filter_data.max_char_count = VIDEO_FILE_SEARCH_CHAR_LEN_MAX;
	limit_filter_data.max_byte_count = 0;
	elm_entry_markup_filter_append(g_pSearchViewWidget->pSearchBarEntry, elm_entry_filter_limit_size, &limit_filter_data);
	evas_object_smart_callback_add(g_pSearchViewWidget->pSearchBarEntry, "maxlength,reached", __mp_search_view_entry_max_len_cb,NULL);

	elm_object_part_content_set(g_pSearchViewWidget->pSearchBarlayout, "searchbar", g_pSearchViewWidget->pSearchBarEntry);
	elm_object_part_content_set(g_pSearchViewWidget->pSearchViewBaselayout, "search_bar", g_pSearchViewWidget->pSearchBarlayout);
	//elm_object_signal_callback_add(g_pSearchViewWidget->pSearchViewBaselayout, "elm,eraser,clicked", "elm", __mp_search_view_eraser_clicked_cb, g_pSearchViewWidget->pSearchBarEntry);
	elm_object_signal_callback_add(g_pSearchViewWidget->pSearchViewBaselayout, "elm,bg,clicked", "elm", __mp_search_view_bg_clicked_cb, g_pSearchViewWidget->pSearchBarEntry);
	elm_entry_input_panel_layout_set(g_pSearchViewWidget->pSearchBarEntry, ELM_INPUT_PANEL_LAYOUT_NORMAL);
	elm_entry_input_panel_return_key_type_set(g_pSearchViewWidget->pSearchBarEntry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH);
	evas_object_size_hint_weight_set(g_pSearchViewWidget->pSearchViewBaselayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pSearchViewWidget->pSearchViewBaselayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_signal_callback_add(g_pSearchViewWidget->pSearchViewBaselayout, "elm,action,click", "", __mp_search_view_search_symbol_clicked_cb, g_pSearchViewWidget->pSearchBarEntry);

	return g_pSearchViewWidget->pSearchViewBaselayout;
}

static void __mp_search_view_update_genlist_item(const char *szSearchWord)
{
	if (!g_pSearchViewHandle)
	{
		VideoLogError("g_pSearchViewHandle == NULL");
		return;
	}

	if (!szSearchWord)
	{
		VideoLogError("szSearchWord == NULL");
		return;
	}

	VideoLogInfo("");

	//int nVideoListSize = 0;

	//mp_util_svc_destory_video_list();
	//mp_util_svc_extract_video_list_for_search_view(szSearchWord, g_pSearchViewHandle->nViewType);
	//nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	//VideoLogInfo("nVideoListSize : %d", nVideoListSize);

	if (g_pSearchViewHandle->SearchedIndexList) {
		eina_list_free(g_pSearchViewHandle->SearchedIndexList);
		g_pSearchViewHandle->SearchedIndexList = NULL;
	}
	mp_util_svc_index_list_for_search_view(szSearchWord, (void **)&g_pSearchViewHandle->SearchedIndexList);

	if (g_pSearchViewHandle->pVideosGenlist)
		elm_genlist_clear(g_pSearchViewHandle->pVideosGenlist);

	if (g_pSearchViewHandle->SearchedIndexList)
	{
		memset(g_pSearchViewHandle->szSearchWord, 0, sizeof(char) * SEARCH_CHAR_LEN);
		strncpy(g_pSearchViewHandle->szSearchWord, szSearchWord, SEARCH_CHAR_LEN - 1);
		__mp_search_view_is_existed_no_content_layout();
		__mp_search_view_append_video_items(g_pSearchViewHandle->pVideosGenlist, true);
		//elm_object_signal_emit(g_pSearchViewWidget->pDimminglayout, SIGNAL_SEARCH_VIEW_INTERNAL_DIMMING_HIDE, "");
	}
	else
	{
		__mp_search_view_is_existed_genlist();
		void *pNoContentLayout = __mp_search_view_add_no_contents_layout(g_pSearchViewWidget->pSearchViewBaselayout);
		elm_object_part_content_set(g_pSearchViewWidget->pSearchViewBaselayout, "elm.swallow.content", pNoContentLayout);
	}
}

void mp_search_view_update_genlist()
{
	if (g_pSearchViewHandle && g_pSearchViewHandle->pVideosGenlist) {
		if (g_pSearchViewHandle->nViewType == MP_LIST_VIEW_AS_FOLDER_LIST && mp_folder_view_is_item_view()) {
			Elm_Object_Item *currItem = NULL;
			Elm_Object_Item *nextItem = NULL;
			currItem = elm_genlist_first_item_get(g_pSearchViewHandle->pVideosGenlist);
			int index = 0;
			while (currItem) {
				nextItem = elm_genlist_item_next_get(currItem);
				elm_genlist_item_update(currItem);
				mp_util_svc_update_thumbnail_info(g_pSearchViewHandle->nPlayVideoIndex);
				__mp_search_view_update_video_items((Evas_Object *)g_pSearchViewHandle->pVideosGenlist, index, g_pSearchViewHandle->nPlayVideoIndex);
				currItem = nextItem;
				index++;
			}
		}
	}
}

static void __mp_search_view_db_data_changed(void *pUserData, Evas_Object *pObject)
{
	VideoLogInfo("");

	if (!g_pSearchViewHandle || !pUserData || !pObject)
	{
		VideoLogError("g_pSearchViewHandle == NULL");
		return;
	}

	char *szChangedStr = NULL;

	const char *temp = elm_object_text_get(pObject);
	char *TempUtf8 = elm_entry_markup_to_utf8(temp);
	szChangedStr = mp_search_view_get_fact_str(TempUtf8);
	MP_FREE_STRING(TempUtf8);

	VideoLogInfo("Changed word in entry widget : %s", szChangedStr);

	if (szChangedStr != NULL && strlen(szChangedStr) > 0)
	{
		__mp_search_view_update_genlist_item(szChangedStr);
	}
	else
	{
		memset(g_pSearchViewHandle->szSearchWord, 0, sizeof(char) * SEARCH_CHAR_LEN);
		//strncpy(g_pSearchViewHandle->szSearchWord, szChangedStr, SEARCH_CHAR_LEN - 1);
		__mp_search_view_is_existed_no_content_layout();
		__mp_search_view_arrange_video_list(__mp_search_view_get_sort_type(), g_pSearchViewHandle->pVideosGenlist);
		//elm_object_signal_emit(g_pSearchViewWidget->pDimminglayout, SIGNAL_SEARCH_VIEW_INTERNAL_DIMMING_SHOW, "");
		elm_object_focus_set(g_pSearchViewWidget->pSearchBarEntry, EINA_TRUE);
	}
	MP_FREE_STRING(szChangedStr);

}

static void __mp_search_view_db_changed_cb(void *pUserData)
{
	if (!g_pSearchViewHandle || !g_pSearchViewWidget)
	{
		VideoLogError("Main handles of list view are not existed.");
		return;
	}


	int nSortType = __mp_search_view_get_sort_type();
	if (g_pSearchViewHandle->pFolderPath)
	{
		if (g_pSearchViewHandle->pFolderPath)
		{
			VideoSecureLogDebug("szFolderPath : %s", g_pSearchViewHandle->pFolderPath);
			if (vp_file_exists(g_pSearchViewHandle->pFolderPath))
			{
				//update video-list
				mp_util_svc_destory_video_item_list();
				mp_util_svc_extract_video_list_from_folder(g_pSearchViewHandle->pFolderPath, nSortType, MP_LIST_VIEW_ALL);
				int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
				VideoLogInfo("nVideoListSize : %d", nVideoListSize);
				if (nVideoListSize > 0)
				{
					__mp_search_view_db_data_changed(g_pSearchViewWidget->pSearchBarlayout, g_pSearchViewWidget->pSearchBarEntry);
				}
				else
				{
					elm_naviframe_item_pop(g_pSearchViewHandle->pNaviFrameHandle);
				}
			}
			else
			{
				elm_naviframe_item_pop(g_pSearchViewHandle->pNaviFrameHandle);
			}
		}
	}
	else
	{
		mp_util_svc_destory_video_list();
		mp_util_svc_extract_video_list_by_item_type(nSortType, mp_view_ctrl_get_list_type());

		int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
		VideoLogInfo("nVideoListSize : %d", nVideoListSize);
		if (nVideoListSize > 0)
		{
			__mp_search_view_db_data_changed(g_pSearchViewWidget->pSearchBarlayout, g_pSearchViewWidget->pSearchBarEntry);
		}
		else
		{
			elm_naviframe_item_pop(g_pSearchViewHandle->pNaviFrameHandle);
		}
	}
	Elm_Object_Item *pItem = elm_genlist_first_item_get(g_pSearchViewHandle->pVideosGenlist);
	if (pItem)
	{
		elm_genlist_item_bring_in(pItem, ELM_GENLIST_ITEM_SCROLLTO_IN);
	}

}

bool mp_search_view_get_active_view()
{
	if (!g_pSearchViewHandle)
		return false;
	return g_pSearchViewHandle->isViewActive;
}

void mp_search_view_init(void *pParent)
{
	VideoLogInfo("");

	if (!g_pSearchViewHandle || !g_pSearchViewWidget)
	{
		VideoLogInfo("Handles are not existed about search view.");
		return;
	}
	g_pSearchViewHandle->bShowNormalLayout = TRUE;
	g_pSearchViewWidget->pSearchViewBaselayout = __mp_search_view_init_internal_layout(pParent);
	evas_object_smart_callback_add((Evas_Object*)mp_util_get_main_window_handle(), "wm,rotation,changed", __mp_search_view_rotate_cb, NULL);
	mp_search_view_check_landscape();
	g_pSearchViewHandle->pVideosGenlist = elm_genlist_add(g_pSearchViewWidget->pSearchViewBaselayout);
	evas_object_size_hint_weight_set(g_pSearchViewHandle->pVideosGenlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pSearchViewHandle->pVideosGenlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_genlist_block_count_set(g_pSearchViewHandle->pVideosGenlist, VIDEO_GENLIST_BLOCK_COUNT);
	elm_genlist_mode_set(g_pSearchViewHandle->pVideosGenlist, ELM_LIST_COMPRESS);
	elm_genlist_homogeneous_set(g_pSearchViewHandle->pVideosGenlist, EINA_TRUE);

	g_pSearchViewHandle->isViewActive = true;
	__mp_search_view_arrange_video_list(__mp_search_view_get_sort_type(), g_pSearchViewHandle->pVideosGenlist);
	elm_object_part_content_set(g_pSearchViewWidget->pSearchViewBaselayout, "elm.swallow.content", g_pSearchViewHandle->pVideosGenlist);
	evas_object_event_callback_add(g_pSearchViewWidget->pSearchViewBaselayout, EVAS_CALLBACK_DEL, __mp_search_view_base_layout_del_cb, NULL);

	g_pSearchViewHandle->pNaviFrameItem = elm_naviframe_item_push(g_pSearchViewHandle->pNaviFrameHandle, NULL, NULL, NULL, g_pSearchViewWidget->pSearchViewBaselayout, NULL);
	elm_naviframe_item_pop_cb_set(g_pSearchViewHandle->pNaviFrameItem, mp_search_view_back_btn_cb, g_pSearchViewHandle);
	evas_object_smart_callback_add(g_pSearchViewHandle->pNaviFrameHandle, "transition,finished", __mp_search_view_naviframe_transition_effect_cb, NULL);
	elm_naviframe_item_title_enabled_set(g_pSearchViewHandle->pNaviFrameItem, EINA_FALSE, EINA_FALSE);

	if (mp_util_svc_get_number_of_video_item_by_type() < 1)
	{
		__mp_search_view_is_existed_genlist();
		void *pNoContentLayout = __mp_search_view_add_no_contents_layout(g_pSearchViewWidget->pSearchViewBaselayout);
		elm_object_part_content_set(g_pSearchViewWidget->pSearchViewBaselayout, "elm.swallow.content", pNoContentLayout);
	}

	//mp_util_virtual_keypad_disabled_set(EINA_TRUE);
	mp_util_db_set_update_fun(g_pSearchViewHandle->euLev, __mp_search_view_db_changed_cb);

	mp_util_hide_indicator();

	elm_object_signal_emit(g_pSearchViewWidget->pSearchBarlayout, "elm,state,eraser,hide", "elm");
}

void mp_search_view_push(void *pNaviFrame, char *pFolderPath)
{
	if (!pNaviFrame)
	{
		VideoLogError("[ERR] No existed pNaviFrame.");
		return;
	}

	VideoLogInfo("");
	mp_search_view_destroy();

	g_pSearchViewHandle = (st_VideoSearchViewHandle*)calloc(1, sizeof(st_VideoSearchViewHandle));
	g_pSearchViewWidget = (st_VideoSearchViewWidget*)calloc(1, sizeof(st_VideoSearchViewWidget));

	memset(g_pSearchViewHandle, 0, sizeof(st_VideoSearchViewHandle));
	memset(g_pSearchViewWidget, 0, sizeof(st_VideoSearchViewWidget));
	g_pSearchViewHandle->pNaviFrameHandle = pNaviFrame;
	if (pFolderPath)
	{
		g_pSearchViewHandle->pFolderPath = strdup(pFolderPath);
	}

	g_pSearchViewHandle->nPlayVideoIndex = -1;

	int nViewType = mp_view_as_ctrl_get_type();
	g_pSearchViewHandle->nViewType = nViewType;
	g_pSearchViewHandle->euLev = MP_DB_UPDATE_LEV_1;
	if (nViewType == MP_LIST_VIEW_AS_FOLDER_LIST && g_pSearchViewHandle->pFolderPath)
	{
		g_pSearchViewHandle->euLev = MP_DB_UPDATE_LEV_2;
	}

	mp_search_view_init(g_pSearchViewHandle->pNaviFrameHandle);
}

static void __mp_search_view_pop(void *pNaviFrame)
{
	VideoLogInfo("");

	if (!g_pSearchViewHandle || !g_pSearchViewWidget)
	{
		VideoLogError("Handles are not existed about search view.");
		return;
	}

	bool bUpdateList = FALSE;
	g_pSearchViewHandle->isViewActive = false;
	if (g_pSearchViewHandle->bUpdateList) {
		if (g_pSearchViewHandle->pChangeViewUpdateFunc) {
			bUpdateList = TRUE;
			g_pSearchViewHandle->pChangeViewUpdateFunc(LIST_UPDATE_TYPE_ALL);
		}
	}

	if (!bUpdateList) {
		if (g_pSearchViewHandle->pChangeViewUserCbFunc) {
			g_pSearchViewHandle->pChangeViewUserCbFunc();
		}
	}
}

void mp_search_view_register_update_list_func(ViewUpdateListItemCbFunc pUpdateViewCb, ListViewUpdateListItemCbFunc pChangeViewCb)
{
	if (!pUpdateViewCb || !pChangeViewCb) {
		VideoLogInfo("[ERR]");
		return;
	}

	VideoLogInfo("");

	if (!g_pSearchViewHandle || !g_pSearchViewWidget) {
		VideoLogError("Handles are not existed about search view.");
		return;
	}

	g_pSearchViewHandle->pChangeViewUserCbFunc = pChangeViewCb;
	g_pSearchViewHandle->pChangeViewUpdateFunc = pUpdateViewCb;
}

static void __mp_search_view_update_video_items(Evas_Object *pGenlist, int nGenItemIndex, int nPlayVideoIndex)
{
	if (!pGenlist)
	{
		VideoLogWarning("[WARNING] pNormalVideoItemList is not existed.");
		return;
	}

	VideoLogInfo("%d", nGenItemIndex);

	Elm_Object_Item *pTmpVideoGenlistItem = elm_genlist_nth_item_get((const Evas_Object *)pGenlist, nGenItemIndex);
	if (pTmpVideoGenlistItem)
	{
		//elm_genlist_item_update(pTmpVideoGenlistItem);
		//elm_genlist_item_fields_update(pTmpVideoGenlistItem, "elm.icon.1", ELM_GENLIST_ITEM_FIELD_CONTENT);
		elm_genlist_item_selected_set(pTmpVideoGenlistItem, EINA_FALSE);
		int nLastPlayedTime = mp_util_svc_get_video_last_played_pos(nPlayVideoIndex);
		int nDurationTime = mp_util_svc_get_video_duration_time(nPlayVideoIndex);
		Evas_Object *pIconContent = elm_object_item_part_content_get(pTmpVideoGenlistItem, "elm.icon.1");
		if (!pIconContent)
		{
			VideoLogError("");
			return;
		}
		Evas_Object *pProgressbar = elm_layout_content_get(pIconContent, "elm.progressbar.icon");

		if (nLastPlayedTime > 0)
		{
			double nRatio = 0.0;

			if (nDurationTime > 0) {
				nRatio = ((double)nLastPlayedTime) /
						((double)nDurationTime);
			}
			if (nRatio < 0.05) {
				nRatio = 0.05;
			}
			if (!pProgressbar)
			{
				elm_genlist_item_fields_update(pTmpVideoGenlistItem, "elm.icon.1", ELM_GENLIST_ITEM_FIELD_CONTENT);
			}
			else
			{
				elm_progressbar_value_set(pProgressbar, nRatio);
			}
		}
		else
		{
			if (pProgressbar)
			{
				elm_layout_content_unset(pIconContent, "elm.progressbar.icon");
				MP_DEL_OBJ(pProgressbar);
			}
		}

		elm_genlist_item_fields_update(pTmpVideoGenlistItem, "elm.text.sub.left.bottom", ELM_GENLIST_ITEM_FIELD_TEXT);
	}

}

void mp_search_view_update_played_item(int nIndex)
{
	if (!g_pSearchViewHandle || !g_pSearchViewWidget || (g_pSearchViewHandle->nPlayVideoIndex < 0))
	{
		VideoLogInfo("SearchView of list view are not existed.");
		return;
	}
	if (nIndex >= 0)
	{
		g_pSearchViewHandle->nPlayVideoIndex = nIndex;
	}
	VideoLogInfo("");

	if (g_pSearchViewHandle->pVideosGenlist)
	{
		int nGenItemIndex = g_pSearchViewHandle->nPlayVideoIndex;
		if (g_pSearchViewHandle->SearchedIndexList)
		{
			Eina_List *pIterateList = NULL;
			void *nTempIndex = (void *)0;
			int nIndex = 0;
			EINA_LIST_FOREACH(g_pSearchViewHandle->SearchedIndexList, pIterateList, nTempIndex)
			{
				VideoLogInfo("nTempIndex:index==%d:%d:%d", (int)nTempIndex, nIndex, nGenItemIndex);
				if (g_pSearchViewHandle->nPlayVideoIndex == (int)nTempIndex)
				{
					nGenItemIndex = nIndex;
					break;
				}
				nIndex++;
			}
		}
		mp_util_svc_update_thumbnail_info(g_pSearchViewHandle->nPlayVideoIndex);
		__mp_search_view_update_video_items((Evas_Object *)g_pSearchViewHandle->pVideosGenlist, nGenItemIndex, g_pSearchViewHandle->nPlayVideoIndex);
	}
}

void mp_search_view_change_language(void)
{
	VideoLogInfo("");

	if (!g_pSearchViewHandle || !g_pSearchViewWidget)
	{
		VideoLogError("Handles are not existed about search view.");
		return;
	}

	//elm_object_item_domain_translatable_part_text_set(g_pSearchViewHandle->pNaviFrameItem, "elm.text.title", MP_SYS_STRING, VIDEOS_CTXPOP_MENU_SEARCH);
	//elm_object_domain_translatable_text_set(g_pSearchViewWidget->pCancelButton, VIDEOS_STRING, MP_SK_CANCEL);
	elm_object_domain_translatable_part_text_set(g_pSearchViewWidget->pSearchBarlayout, "elm.guidetext", VIDEOS_STRING, VIDEOS_CTXPOP_MENU_SEARCH);
}


static void
__mp_search_view_base_layout_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	//evas_object_event_callback_del(g_pSearchViewWidget->pSearchViewBaselayout, EVAS_CALLBACK_DEL, __mp_search_view_base_layout_del_cb);

	__mp_search_view_reset();
}

