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

#include <glib.h>
#include <app.h>

#include "vp-util.h"
#include "vp-file-util.h"
#include "mp-video-log.h"
#include "mp-util.h"
#include "video-player.h"
#include "mp-video-string-define.h"
#include "mp-video-value-define.h"
#include "mp-video-type-define.h"

#include "mp-util-media-service.h"
#include "mp-video-list-view-item-of-folder.h"
#include "mp-video-view-popup-ctrl.h"
#include "mp-video-list-sort-ctrl.h"
#include "mp-footer-toolbar.h"
#include "mp-video-search-view.h"
#include "mp-external-ug.h"
#include "mp-launching-video-displayer.h"
#ifdef ENABLE_DRM_FEATURE
#include "mp-drm-ctrl.h"
#endif
#include "videos-view-mgr.h"
#include "mp-video-list-remove-view.h"
#include "mp-video-list-view-as-ctrl.h"
#include "mp-video-list-option-ctrl.h"
#include "mp-video-list-share-via-view.h"

#include "mp-video-list-personal-view.h"
#include "mp-video-rename-ctrl.h"
#include "mp-video-info-ctrl.h"
#include "mp-util-widget-ctrl.h"
#include "mp-video-util-db-controller.h"
#include "mp-video-list-view-select.h"
#include "mp-video-nocontent-layout.h"
#include "mp-video-detail-view.h"
#include "mp-rotate-ctrl.h"

#define NO_CONTENTS_W					720
#define	NO_CONTENTS_H					897


/****************************folder view***************************/

typedef struct
{
	void *pNaviFrameHandle;
	void *pNaviFrameItem;
	void *pVideosGenlist;

	Elm_Genlist_Item_Class *pGenGridItc;
	Elm_Genlist_Item_Class *pGenGridLandscapeItc;
	Elm_Gengrid_Item_Class *pGenGridItc_l;

	int nFolder_index;
	int currentItemCount;
	char *pFolderUrl;
	int nPlayVideoIndex;
	bool bExist;
	bool bLandscape;
	MpDbUpdateViewLev euLev;
	UpdateListItemsCbFunc pUpdateListUserCbFunc;

}st_VideoFolderViewHandle;

typedef struct
{
	Evas_Object *pFolderViewBox;
	Evas_Object *pFolderViewCountlayout;
	Evas_Object *pFolderViewBaselayout;
	Evas_Object *pNocontentsLayout;
	Evas_Object *pViewButton;
//	Evas_Object *pSearchBtn;

	Evas_Object *pFolderViewGenlistTitle;
	Elm_Genlist_Item_Class *pFolderViewGenlistTitleItc;
	Elm_Object_Item *pFolderViewGenlistItemForTitle;
	Eina_List *pVideolistItemList;
}st_VideoFolderViewWidget;

st_VideoFolderViewHandle 	*g_pFolderViewHandle = NULL;
st_VideoFolderViewWidget 	*g_pFolderViewWidget = NULL;
Elm_Genlist_Item_Class 		VideoFolder_Itc;


static bool __mp_folder_item_view_update_video_list(MpListUpdateType eUpdateType);
static void __mp_folder_item_view_db_changed_cb(void *pUserData);
#ifdef ENABLE_LONGPRESS
static void __mp_folder_item_view_delete_video_items(int nVideoItemIndex);
#endif
static void __mp_folder_item_view_db_result_data_update_cb(void);
static void __mp_folder_item_view_result_user_cb(MpListUpdateType eUpdateType);
static void __mp_folder_item_view_result_view_update_cb(void);
//static void __mp_folder_item_view_update_count_layout();
static void __mp_folder_item_view_rotate_cb(void *data, Evas_Object *obj, void *event_info);
static void mp_folder_view_cancel_thumbnail_update(int nIndexGenlist);
static int mp_folder_item_view_thumbnail_to_genlist_index(int nVideoItemIndex);
static int __mp_folder_item_view_get_sort_type(void);

/*
void __mp_folder_item_view_check_search_button_enable(void)
{
	Eina_Bool bActivateSearchButton = EINA_FALSE;

	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	VideoLogInfo("nVideoListSize : %d", nVideoListSize);

	if (nVideoListSize < 1)
	{
		bActivateSearchButton = EINA_TRUE;
	}
	else
	{
		bActivateSearchButton = EINA_FALSE;
	}

	elm_object_disabled_set(g_pFolderViewWidget->pSearchBtn, bActivateSearchButton);
}
*/
static bool __mp_folder_item_view_free_video_genlistItem(void)
{
	if (!g_pFolderViewWidget || !g_pFolderViewWidget->pVideolistItemList) {
		VideoLogError("g_pMainViewWidget->pVideoGenlistItemList is already empty.");
		return FALSE;
	}

	VideoLogInfo("");

	Elm_Object_Item *pTmpVideoGenlistItem = NULL;
	EINA_LIST_FREE(g_pFolderViewWidget->pVideolistItemList, pTmpVideoGenlistItem)
	{
		if (pTmpVideoGenlistItem) {
			int nIndex = (int)elm_object_item_data_get(pTmpVideoGenlistItem);
			//mp_util_svc_cancel_thumbnail_update(nIndex);
			mp_folder_view_cancel_thumbnail_update(nIndex);
			pTmpVideoGenlistItem = NULL;
		}
	}

	g_pFolderViewWidget->pVideolistItemList = NULL;

	return TRUE;
}

static void __mp_folder_item_view_clear_genlist(void *pVideosGenlist)
{
	VideoLogInfo("");
	if (pVideosGenlist && g_pFolderViewWidget) {
		__mp_folder_item_view_free_video_genlistItem();
		elm_genlist_clear(pVideosGenlist);
	}
}

static void __mp_folder_item_view_destroy(void)
{
	VideoLogInfo("");
	if (g_pFolderViewHandle)
	{
		g_pFolderViewHandle->nFolder_index = 0;
		__mp_folder_item_view_clear_genlist(g_pFolderViewHandle->pVideosGenlist);
		MP_DEL_OBJ(g_pFolderViewHandle->pVideosGenlist);

		g_pFolderViewHandle->nPlayVideoIndex = -1;
		MP_FREE_STRING(g_pFolderViewHandle->pFolderUrl);

		free(g_pFolderViewHandle);
		g_pFolderViewHandle = NULL;
	}

	if (g_pFolderViewWidget)
	{
		MP_DEL_ITC(g_pFolderViewWidget->pFolderViewGenlistTitleItc);
		MP_DEL_OBJ(g_pFolderViewWidget->pViewButton);
//		MP_DEL_OBJ(g_pFolderViewWidget->pSearchBtn);
		MP_DEL_OBJ(g_pFolderViewWidget->pNocontentsLayout);
		MP_DEL_OBJ(g_pFolderViewWidget->pFolderViewBaselayout);
		MP_DEL_OBJ(g_pFolderViewWidget->pFolderViewCountlayout);
		MP_DEL_OBJ(g_pFolderViewWidget->pFolderViewBox);

		free(g_pFolderViewWidget);
		g_pFolderViewWidget = NULL;
	}
	evas_object_smart_callback_del((Evas_Object*)mp_util_get_main_window_handle(), "wm,rotation,changed", __mp_folder_item_view_rotate_cb);
}

static void __mp_folder_item_view_reset(void)
{
	VideoLogInfo("");
	if (g_pFolderViewHandle)
	{
		mp_util_db_set_update_fun(g_pFolderViewHandle->euLev, NULL);
		g_pFolderViewHandle->nFolder_index = 0;
		__mp_folder_item_view_clear_genlist(g_pFolderViewHandle->pVideosGenlist);
		g_pFolderViewHandle->pVideosGenlist = NULL;

		MP_FREE_STRING(g_pFolderViewHandle->pFolderUrl);

		g_pFolderViewHandle->nPlayVideoIndex = -1;

		free(g_pFolderViewHandle);
		g_pFolderViewHandle = NULL;
	}

	if (g_pFolderViewWidget)
	{
		MP_DEL_ITC(g_pFolderViewWidget->pFolderViewGenlistTitleItc);
		g_pFolderViewWidget->pFolderViewBaselayout = NULL;
		g_pFolderViewWidget->pNocontentsLayout = NULL;
//		g_pFolderViewWidget->pSearchBtn = NULL;
		g_pFolderViewWidget->pViewButton = NULL;

		free(g_pFolderViewWidget);
		g_pFolderViewWidget = NULL;
	}
	evas_object_smart_callback_del((Evas_Object*)mp_util_get_main_window_handle(), "wm,rotation,changed", __mp_folder_item_view_rotate_cb);
}


static Evas_Object *__mp_folder_item_view_init_base_layout(void *pParent)
{
	if (!pParent)
	{
		VideoLogError("[ERR]");
		return NULL;
	}

	VideoLogInfo("");

	MP_DEL_OBJ(g_pFolderViewWidget->pFolderViewBaselayout);
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s/%s/%s", path , "edje", VIDEO_PLAYER_LISTVIEW_EDJ);

	// Create folder view base layout.
	g_pFolderViewWidget->pFolderViewBaselayout = elm_layout_add(pParent);
	elm_layout_file_set(g_pFolderViewWidget->pFolderViewBaselayout, edj_path, LISTVIEW_EDJ_GROUP);
	evas_object_size_hint_weight_set(g_pFolderViewWidget->pFolderViewBaselayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pFolderViewWidget->pFolderViewBaselayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	return g_pFolderViewWidget->pFolderViewBaselayout;
}

static void __mp_folder_item_view_pop(void *pNaviFrame, void *pUserData)
{
	if (!g_pFolderViewHandle)
	{
		VideoLogInfo("[ERR] g_pFolderViewHandle");
		return;
	}
	mp_ft_ctrl_destroy();

	if (g_pFolderViewHandle && g_pFolderViewHandle->pUpdateListUserCbFunc)
	{
		g_pFolderViewHandle->pUpdateListUserCbFunc(LIST_UPDATE_TYPE_ALL);

		VideoLogInfo("");
	}

	//MpVideoServiceCtrlSetPlayerType(MpVideoServiceCtrlGetPrevPlayerType());

}

static void __mp_folder_item_view_trans_finished_cb(void *pUserData, Evas_Object *obj, void *event_info)
{
	if (!obj)
	{
		VideoLogError("[ERR] g_pAsfDeviceListViewHandle is NULL.");
		return;
	}

	mp_widget_ctrl_enable_navi_handle_focus(obj);

	evas_object_smart_callback_del(obj, "transition,finished", __mp_folder_item_view_trans_finished_cb);
}

// Back button
static Eina_Bool __mp_folder_item_view_back_btn_cb(void *pUserData, Elm_Object_Item *pItem)
{
	if (!g_pFolderViewHandle)
	{
		VideoLogInfo("[ERR] g_pListRemoveViewHandle is NULL.");
		return EINA_TRUE;
	}
	VideoLogInfo("");
	g_pFolderViewHandle->bExist = true;
	mp_widget_ctrl_disable_navi_handle_focus_except_item(g_pFolderViewHandle->pNaviFrameHandle, g_pFolderViewHandle->pNaviFrameItem);
	evas_object_smart_callback_add(g_pFolderViewHandle->pNaviFrameHandle, "transition,finished", __mp_folder_item_view_trans_finished_cb, NULL);
	__mp_folder_item_view_pop(g_pFolderViewHandle->pNaviFrameHandle, NULL);
	mp_folder_view_set_is_item_view(false);

	return EINA_TRUE;
}

static char *__mp_folder_item_view_get_guration_time(int nVideoItemIndex)
{
	VideoLogInfo("");

	char szTmpStr[STR_LEN_MAX] = {0, };
	char szTmpStrDurationTime[STR_LEN_MAX] = {0, };
	char szTmpStrLastPlayedPos[STR_LEN_MAX] = {0, };

	unsigned int nVideoInfoDuration = mp_util_svc_get_video_duration_time(nVideoItemIndex);
	unsigned int nVideoInfoLastTime = mp_util_svc_get_video_last_played_pos(nVideoItemIndex);

	mp_util_convert_time(nVideoInfoDuration, szTmpStrDurationTime, FALSE);
	if (nVideoInfoLastTime > 0)
	{
		mp_util_convert_time(nVideoInfoLastTime, szTmpStrLastPlayedPos, TRUE);
		snprintf(szTmpStr, STR_LEN_MAX, "<match>%s</match>/%s", szTmpStrLastPlayedPos, szTmpStrDurationTime);
	}
	else
	{
		snprintf(szTmpStr, STR_LEN_MAX, "%s", szTmpStrDurationTime);
	}

	return strdup(szTmpStr);
}

static char *__mp_folder_item_view_get_label_of_video_item_cb(void *pUserData, Evas_Object *pObject, const char *pPart)
{

	int nVideoItemIndex = (int)pUserData;
	mp_util_svc_destory_video_item_list();
	mp_util_svc_extract_video_list_from_folder(g_pFolderViewHandle->pFolderUrl, __mp_folder_item_view_get_sort_type(), MP_LIST_VIEW_ALL);
	if (!strcmp(pPart, "elm.text.main.left.top")) {
		char *szTitle;
		szTitle = mp_util_svc_get_video_title(nVideoItemIndex);
		char *szTitleUtf8 = elm_entry_utf8_to_markup(szTitle);
		VideoLogWarning("testestesteste = %d, %s", nVideoItemIndex, szTitleUtf8);

		if (szTitle) {
			free(szTitle);
			szTitle = NULL;
		}

		return szTitleUtf8;
	} else if (!strcmp(pPart, "elm.text.sub.left.bottom"))
	{
		return __mp_folder_item_view_get_guration_time(nVideoItemIndex);
	} else if (!strcmp(pPart, "elm.text"))
	{
		char *szTitle;
		szTitle = mp_util_svc_get_video_title(nVideoItemIndex);
		char *szTitleUtf8 = elm_entry_utf8_to_markup(szTitle);
		VideoLogInfo("%s",szTitle);
		if (szTitle) {
			free(szTitle);
			szTitle = NULL;
		}

		return szTitleUtf8;
	} else if (!strcmp(pPart, "elm.text.2"))
	{
		return __mp_folder_item_view_get_guration_time(nVideoItemIndex);
	}
	/*else if (!strcmp(pPart, "elm.text.sub.right.bottom"))
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
	}*/

	return NULL;
}

#ifdef ENABLE_LONGPRESS
static void __mp_folder_item_view_HW_mouse_back_up_popup_cb(void *pUserData, Evas *pEvas, Evas_Object *pObject, void *pEventInfo)
{
	Evas_Event_Mouse_Up *pEvent = pEventInfo;

	if (pEvent->button == 3)
	{
		mp_util_delete_popup_handle();
	}
}

static void __mp_folder_item_view_HW_key_back_popup_cb(void *pUserData, Evas *pEvas, Evas_Object *pObject, void *pEventInfo)
{
	Evas_Event_Key_Down *pEvent = pEventInfo;

	if (!g_strcmp0(pEvent->keyname, "XF86Back"))
	{
		mp_util_delete_popup_handle();
	}
}
#endif

#ifdef ENABLE_LONGPRESS
static void __mp_folder_item_view_respones_delete_popup_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	mp_util_delete_popup_handle();

	int nVideoItemIndex = (int)pUserData;

	VideoLogInfo("nVideoItemIndex : %d", nVideoItemIndex);

	if (nVideoItemIndex < 0 || nVideoItemIndex >= mp_util_svc_get_video_list_size_for_checking_index())
	{
		VideoLogError("[ERR] list item index.");
		return;
	}

	char *szFileUrl = mp_util_svc_get_video_url(nVideoItemIndex);
	if (szFileUrl)
	{
		VideoLogInfo("Delete file url : %s", szFileUrl);
		vp_file_unlink(szFileUrl);
		free(szFileUrl);
		szFileUrl = NULL;

		mp_util_svc_remove_item_by_list_index(nVideoItemIndex);
		__mp_folder_item_view_delete_video_items(nVideoItemIndex);

		//__mp_folder_item_view_update_count_layout();
	}
}

static void __mp_folder_item_view_respones_cancel_popup_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");

	mp_util_delete_popup_handle();
}

static void __mp_folder_item_view_delete_btn_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");
	mp_option_ctrl_delete_handle();

	int nVideoItemIndex = (int)pUserData;
	MpListViewAsType type = MP_LIST_VIEW_AS_THUMBNAIL_LIST;
	mp_util_remove_question_popup(nVideoItemIndex, type, __mp_folder_item_view_respones_delete_popup_cb, __mp_folder_item_view_respones_cancel_popup_cb, __mp_folder_item_view_HW_mouse_back_up_popup_cb, __mp_folder_item_view_HW_key_back_popup_cb);
}
#endif

Evas_Object *__mp_folder_item_view_get_icon_of_video_item_cb(void *pUserData, Evas_Object *pObject, const char *pPart)
{

	Evas_Object *pLayout = NULL;

	int nVideoItemIndex = (int)pUserData;
	if (!pPart || !pObject)
	{
		return NULL;
	}
	mp_util_svc_destory_video_item_list();
	mp_util_svc_extract_video_list_from_folder(g_pFolderViewHandle->pFolderUrl, __mp_folder_item_view_get_sort_type(), MP_LIST_VIEW_ALL);
	if (!strcmp(pPart, "elm.icon.1"))
	{
		VideoLogInfo("pPart: elm.icon.1 - thumbnail");

		char *pThumbIconUri = mp_util_svc_get_video_thumbnail(nVideoItemIndex);
		Evas_Object *pBg = NULL;
		char edj_path[1024] = {0};

		char *path = app_get_resource_path();
		snprintf(edj_path, 1024, "%s/%s/%s", path , "edje", VIDEO_CUSTOM_THEME);

		pLayout = elm_layout_add(pObject);
		elm_layout_file_set(pLayout, edj_path, "listview.thumbnail.layout");

		if (!pThumbIconUri || !vp_file_exists(pThumbIconUri)) {
			MP_FREE_STRING(pThumbIconUri);
		}

		pBg = mp_util_create_preload_image(pLayout, pThumbIconUri, VIDEO_ICON_WIDTH);
		elm_layout_content_set(pLayout, "elm.thumbnail.icon", pBg);

		if (pBg)
			evas_object_show(pBg);

		MP_FREE_STRING(pThumbIconUri);

		int nLastPlayedTime = mp_util_svc_get_video_last_played_pos(nVideoItemIndex);
		int nDurationTime = mp_util_svc_get_video_duration_time(nVideoItemIndex);

		if (nLastPlayedTime > 0 && nDurationTime > 0) {
			double nRatio = ((double)nLastPlayedTime) / ((double)nDurationTime);
			if (nRatio < 0.05) {
				nRatio = 0.05;
			}

			Evas_Object *pProgressBar = mp_widget_ctrl_create_progressbar(pLayout, nRatio);
			elm_layout_content_set(pLayout, "elm.progressbar.icon", pProgressBar);

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
	} else if (!strcmp(pPart, "elm.swallow.icon"))
	{
		char *pThumbIconUri = mp_util_svc_get_video_thumbnail(nVideoItemIndex);
		Evas_Object *thumb			= NULL;
		int			nWidth			= 0;
		int			nHeight			= 0;

		thumb = elm_image_add(pObject);
		elm_image_preload_disabled_set(thumb, EINA_FALSE);
		elm_image_smooth_set(thumb, EINA_FALSE);
		elm_image_prescale_set(thumb, 50*MP_SCALE);
		elm_image_file_set(thumb, pThumbIconUri, NULL);
		elm_image_object_size_get(thumb, &nWidth, &nHeight);

		if (nWidth > nHeight) {
			elm_image_aspect_fixed_set(thumb, EINA_FALSE);
		}
		if (thumb)
			evas_object_show(thumb);

		MP_FREE_STRING(pThumbIconUri);

		return thumb;
	} else if (!strcmp(pPart, "elm.progressbar.icon"))
	{
		int nLastPlayedTime = mp_util_svc_get_video_last_played_pos(nVideoItemIndex);
		int nDurationTime = mp_util_svc_get_video_duration_time(nVideoItemIndex);

		if (nLastPlayedTime <= 0) {
			return NULL;
		}

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
		evas_object_show(pProgressBar);

		return pProgressBar;
	}

	MP_DEL_OBJ(pLayout);

	return NULL;
}

// Genlist item callback.
static void __mp_folder_item_view_warning_popup_cb(void *pUserData,
		Evas_Object *pObject, void *pEventInfo)
{
	if (!pUserData) {
		VideoLogInfo("No exist pUserData");
		return;
	}

	VideoLogInfo("");

	mp_util_delete_popup_handle();
}

static void __mp_folder_item_view_select_video_item_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!pObject) {
		VideoLogError("[ERR]No have pUserData");
		return;
	}
	Evas_Object *pPopup = evas_object_data_get(pObject, "popup"); // Get popup
	if (pPopup)
	{
		VideoLogError("long press is called");
		return;  // If popup exists, do nothing
	}

	if (!mp_util_call_off()) {
		mp_util_ticker_toast_popup(VIDEOS_NOTIPOP_MSG_UNABLE_TO_PLAY_VIDEO_DURING_CALL, false, false);
		return;
	}

	if (!mp_launch_video_allow_check()) {
		VideoLogWarning("nVideoItemIndex < 0 ===>>> RETURN");
		return;
	}

	int nVideoItemIndex = (int)pUserData;
	int nVideoListSize = g_pFolderViewHandle->currentItemCount;

	VideoLogDebug("nVideoItemIndex = %d--%d", nVideoItemIndex, nVideoListSize);

	if (nVideoItemIndex < 0 || nVideoItemIndex >= nVideoListSize) {
		VideoLogError("nVideoItemIndex < 0 ===>>> RETURN");
		return;
	}

	g_pFolderViewHandle->nPlayVideoIndex = nVideoItemIndex;

	char *szVideoUrl = mp_util_svc_get_video_url(nVideoItemIndex);

	if (szVideoUrl) {
		if (!mp_util_check_local_file_exist(szVideoUrl)) {
			mp_util_notify_popup(NULL, VIDEOS_COMMON_HEADER_ERROR, VIDEOS_NOTIPOP_MSG_FILE_NOT_EXIST, __mp_folder_item_view_warning_popup_cb);
			MP_FREE_STRING(szVideoUrl);
			return;
		}
		mp_launch_video_play(szVideoUrl, MP_PLAYER_TYPE_VIDEO, NULL);
	} else {
		VideoLogInfo("[Err] Fail to get video uri from media service.");
		return;
	}

	MP_FREE_STRING(szVideoUrl);
}

static void mp_folder_view_cancel_thumbnail_update(int nIndexGenlist)
{
	if (!g_pFolderViewWidget) {
		VideoLogError("");
		return;
	}
	bool bLandscape = mp_rotate_ctrl_check_landspace();
	int nVideoItemIndex = 0;
	int nItemCount = VP_GENLIST_THUMB_PORTRAIT_COUNT;

	if (bLandscape) {
		nItemCount = VP_GENLIST_THUMB_LANDSPACE_COUNT;
	}

	nVideoItemIndex = nIndexGenlist * nItemCount;
	int nVideoListSize = g_pFolderViewHandle->currentItemCount;
	VideoLogInfo("realize is %d--%d--%d", nItemCount, nVideoItemIndex, nVideoListSize);

	int nIndex = 0;
	for (; (nIndex < nItemCount) && (nVideoItemIndex < nVideoListSize); nIndex++) {
		mp_util_svc_cancel_thumbnail_update(nVideoItemIndex);
		nVideoItemIndex++;
	}
}

static void  __mp_folder_item_view_video_item_del_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!g_pFolderViewWidget || !event_info || !g_pFolderViewWidget->pVideolistItemList)
	{
		VideoLogError("g_pMainViewWidge is NULL");
		return;
	}

	int nIndexGenlist = 0;
	Elm_Object_Item *pCurrentItem = (Elm_Object_Item *)event_info;
	Eina_List *pCurList = NULL;
	Elm_Object_Item *pTmpVideoItem = NULL;
	EINA_LIST_FOREACH(g_pFolderViewWidget->pVideolistItemList, pCurList, pTmpVideoItem)
	{
		if (pCurrentItem == pTmpVideoItem)
		{
			nIndexGenlist = (int)elm_object_item_data_get(pCurrentItem);
			mp_folder_view_cancel_thumbnail_update(nIndexGenlist);
			pTmpVideoItem = NULL;
			pCurList->data = NULL;
			break;
		}
	}
}

#ifdef VS_FEATURE_THUMBNAIL_VIEW
static void __mp_folder_item_view_append_gengrid_items(Evas_Object *pObjVideosList, int nGenItemIndex)
{
	if (!g_pFolderViewHandle) {
		VideoLogInfo("invalid handle");
		return;
	}
	MP_DEL_GRID_ITC(g_pFolderViewHandle->pGenGridItc_l);
	g_pFolderViewHandle->pGenGridItc_l = elm_gengrid_item_class_new();
	g_pFolderViewHandle->pGenGridItc_l->item_style = "video/album_grid";
	g_pFolderViewHandle->pGenGridItc_l->func.text_get = (void*)__mp_folder_item_view_get_label_of_video_item_cb;
	g_pFolderViewHandle->pGenGridItc_l->func.content_get = (void*)__mp_folder_item_view_get_icon_of_video_item_cb;
	g_pFolderViewHandle->pGenGridItc_l->func.state_get = NULL;
	g_pFolderViewHandle->pGenGridItc_l->func.del = NULL;

	int nCount = (mp_rotate_ctrl_check_landspace() ? VP_GENLIST_THUMB_LANDSPACE_COUNT : VP_GENLIST_THUMB_PORTRAIT_COUNT);
	int nIndex = nGenItemIndex*nCount;
	int nMaxIndex = nIndex + nCount;
	int nVideoListSize = g_pFolderViewHandle->currentItemCount;
	Elm_Object_Item *gridItem = NULL;
	for (; (nIndex < nMaxIndex) && (nIndex < nVideoListSize); nIndex++) {
		gridItem = elm_gengrid_item_append(pObjVideosList, g_pFolderViewHandle->pGenGridItc_l, (void *)nIndex, __mp_folder_item_view_select_video_item_cb, (void *)nIndex);
		elm_gengrid_item_select_mode_set(gridItem, ELM_OBJECT_SELECT_MODE_ALWAYS);
		elm_object_item_data_set(gridItem, (void *)nIndex);
	}
}

static Evas_Object *__mp_folder_item_view_create_gengrid(Evas_Object *parent, int nWidth, int nHeight)
{
	Evas_Object *pGengrid;
	pGengrid = elm_gengrid_add(parent);
	if (!pGengrid) {
		VideoLogError("pVideoGrid is not existed.");
		return NULL;
	}
	evas_object_size_hint_weight_set(pGengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pGengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_gengrid_item_size_set(pGengrid, nWidth, nHeight);
	elm_gengrid_align_set(pGengrid, 0.0, 0.0);
	elm_gengrid_horizontal_set(pGengrid, EINA_TRUE);

	return pGengrid;
}

static void mp_item_of_folder_view_get_icon_dimensions(int *height, int *width)
{
	int winHeight = 0;
	int winWidth = 0;
	elm_win_screen_size_get(mp_util_get_main_window_handle(), NULL, NULL,
				&winWidth, &winHeight);

	bool bLandscape = mp_rotate_ctrl_check_landspace_by_win((Evas_Object*)mp_util_get_main_window_handle());
	if (bLandscape) {
		*width = winHeight / VP_GENLIST_THUMB_LANDSPACE_COUNT;
	} else {
		*width = winWidth / VP_GENLIST_THUMB_PORTRAIT_COUNT;
	}
	*height = VP_ALBUM_THUMB_ICON_HEIGHT * MP_SCALE;
}

static Evas_Object *__mp_folder_item_view_get_icon_of_grid_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{
	if (!g_pFolderViewHandle || !pObject) {
		VideoLogError("g_pMainViewWidgetOfFolderItemList is NULL");
		return NULL;
	}

	int nGenItemIndex = (int)pUserData;
	int nHeight = 0;
	int nWidth = 0;
	mp_item_of_folder_view_get_icon_dimensions(&nHeight, &nWidth);

	Evas_Object *pGengrid = __mp_folder_item_view_create_gengrid(pObject, nWidth, nHeight);
	__mp_folder_item_view_append_gengrid_items(pGengrid, nGenItemIndex);

	return pGengrid;
}

static int mp_folder_item_view_thumbnail_to_genlist_index(int nVideoItemIndex)
{
	if (nVideoItemIndex < 0)
	{
		VideoLogDebug("invalid nVideoItemIndex");
		return 0;
	}
	int nItemCount = VP_GENLIST_THUMB_PORTRAIT_COUNT;
	int nGenlistIndex = 0;
	if (g_pFolderViewHandle->bLandscape)
	{
		nItemCount = VP_GENLIST_THUMB_LANDSPACE_COUNT;
	}
	nGenlistIndex = nVideoItemIndex/nItemCount;
	return nGenlistIndex;
}

static void __mp_folder_item_view_append_grid_layout(void *pGenlist)
{
	if (!pGenlist || !g_pFolderViewHandle) {
		VideoLogError("error handle.");
		return;
	}
	int nIndex = 0;
	int nVideoListSize = g_pFolderViewHandle->currentItemCount;
	if (nVideoListSize <= 0) {
		VideoLogWarning("nVideoListSize = %d", nVideoListSize);
		return;
	}

	MP_DEL_ITC(g_pFolderViewHandle->pGenGridItc);
	g_pFolderViewHandle->pGenGridItc = elm_genlist_item_class_new();
	g_pFolderViewHandle->pGenGridItc->item_style = "video/1icon.thumbnail";
	g_pFolderViewHandle->pGenGridItc->func.text_get = NULL;
	g_pFolderViewHandle->pGenGridItc->func.content_get = (void *)__mp_folder_item_view_get_icon_of_grid_cb;
	g_pFolderViewHandle->pGenGridItc->func.state_get = NULL;
	g_pFolderViewHandle->pGenGridItc->func.del = NULL;
	g_pFolderViewHandle->pGenGridItc->decorate_item_style = NULL;

	MP_DEL_ITC(g_pFolderViewHandle->pGenGridLandscapeItc);
	g_pFolderViewHandle->pGenGridLandscapeItc = elm_genlist_item_class_new();
	g_pFolderViewHandle->pGenGridLandscapeItc->item_style = "video/1icon.thumbnail.landscape";
	g_pFolderViewHandle->pGenGridLandscapeItc->func.text_get = NULL;
	g_pFolderViewHandle->pGenGridLandscapeItc->func.content_get = (void *)__mp_folder_item_view_get_icon_of_grid_cb;
	g_pFolderViewHandle->pGenGridLandscapeItc->func.state_get = NULL;
	g_pFolderViewHandle->pGenGridLandscapeItc->func.del = NULL;
	g_pFolderViewHandle->pGenGridLandscapeItc->decorate_item_style = NULL;

	int nGenlistRow = 0;
	nGenlistRow = mp_folder_item_view_thumbnail_to_genlist_index(nVideoListSize - 1);
	Elm_Object_Item *pTmpVideoGenlistItem = NULL;

	if (mp_rotate_ctrl_check_landspace()) {
		for (nIndex = 0; nIndex <= nGenlistRow; nIndex++) {
			 pTmpVideoGenlistItem = elm_genlist_item_append(pGenlist, g_pFolderViewHandle->pGenGridItc, (void *)nIndex, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			g_pFolderViewWidget->pVideolistItemList = eina_list_append(g_pFolderViewWidget->pVideolistItemList, pTmpVideoGenlistItem);
			elm_object_item_del_cb_set(pTmpVideoGenlistItem, __mp_folder_item_view_video_item_del_cb);
		}
	} else {
		for (nIndex = 0; nIndex <= nGenlistRow; nIndex++) {
			 pTmpVideoGenlistItem = elm_genlist_item_append(pGenlist, g_pFolderViewHandle->pGenGridLandscapeItc, (void *)nIndex, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			g_pFolderViewWidget->pVideolistItemList = eina_list_append(g_pFolderViewWidget->pVideolistItemList, pTmpVideoGenlistItem);
			elm_object_item_del_cb_set(pTmpVideoGenlistItem, __mp_folder_item_view_video_item_del_cb);
		}
	}
}
#endif

/*static void __mp_folder_item_view_append_video_items(void *pVideosGenlist)
{
	VideoLogInfo("");

	int nIndex = 0;
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();

	VideoFolder_Itc.item_style = "2line.top";
	VideoFolder_Itc.func.text_get = (void*)__mp_folder_item_view_get_label_of_video_item_cb;
	VideoFolder_Itc.func.content_get = (void*)__mp_folder_item_view_get_icon_of_video_item_cb;
	VideoFolder_Itc.func.state_get = NULL;
	VideoFolder_Itc.func.del = NULL;
	VideoFolder_Itc.decorate_item_style = NULL;//"mode/slide4";
	VideoFolder_Itc.decorate_all_item_style = NULL;//"edit_default";
	Elm_Object_Item *pTmpVideoGenlistItem = NULL;

	for (nIndex = 0; nIndex < nVideoListSize; nIndex++)
	{
		pTmpVideoGenlistItem = elm_genlist_item_append(pVideosGenlist, &VideoFolder_Itc,
			(void*)nIndex, NULL, ELM_GENLIST_ITEM_NONE, __mp_folder_item_view_select_video_item_cb, NULL);

		g_pFolderViewWidget->pVideolistItemList = eina_list_append(g_pFolderViewWidget->pVideolistItemList, pTmpVideoGenlistItem);
		elm_object_item_del_cb_set(pTmpVideoGenlistItem, __mp_folder_item_view_video_item_del_cb);
	}
}*/


static void __mp_folder_item_view_hide_genlist(void)
{
	if (!g_pFolderViewWidget || !g_pFolderViewHandle)
	{
		VideoLogError("g_pSearchViewWidget == NULL");
		return;
	}

	elm_object_part_content_unset(g_pFolderViewWidget->pFolderViewBaselayout, SWALLOW_LISTVIEW_CONTENT);
	if (g_pFolderViewHandle->pVideosGenlist)
	{
		evas_object_hide(g_pFolderViewHandle->pVideosGenlist);
	}
}

static void __mp_folder_item_view_hide_no_layout(void)
{
	if (!g_pFolderViewWidget || !g_pFolderViewHandle)
	{
		VideoLogError("g_pSearchViewWidget == NULL");
		return;
	}
	if (g_pFolderViewWidget->pNocontentsLayout)
	{
		elm_object_part_content_unset(g_pFolderViewWidget->pFolderViewBaselayout, SWALLOW_LISTVIEW_CONTENT);
		MP_DEL_OBJ(g_pFolderViewWidget->pNocontentsLayout);
	}
}

static Evas_Object *__mp_folder_item_view_add_no_contents_layout()
{
	if (!g_pFolderViewWidget)
	{
		VideoLogError("g_pFolderViewWidget is not existed.");
		return NULL;
	}
	VideoLogInfo("");
	MP_DEL_OBJ(g_pFolderViewWidget->pNocontentsLayout);
	g_pFolderViewWidget->pNocontentsLayout = mp_create_nocontent_layout(g_pFolderViewWidget->pFolderViewBaselayout,
										VIDEOS_NOCONTENT_MSG_NO_VIDEOS_IDS,
										NULL);
	return g_pFolderViewWidget->pNocontentsLayout;
}

static bool __mp_folder_item_view_arrange_video_list(int nItemSortType, MpListUpdateType eUpdateType, void *pVideosGenlist)
{
	if (!g_pFolderViewWidget || !g_pFolderViewHandle)
	{
		VideoLogError("No exist g_pSearchViewWidget.");
		return false;
	}

	int nVideoListSize = 0;
	bool bUnset = FALSE;
	if (eUpdateType == LIST_UPDATE_TYPE_ALL || eUpdateType == LIST_UPDATE_TYPE_DATA)
	{
		mp_util_svc_destory_video_item_list();
		if (g_pFolderViewHandle->pFolderUrl)
		{
			mp_util_svc_extract_video_list_from_folder(g_pFolderViewHandle->pFolderUrl, nItemSortType, MP_LIST_VIEW_ALL);
		}
	}
	nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	g_pFolderViewHandle->currentItemCount = nVideoListSize;

	elm_genlist_clear(pVideosGenlist);

	//__mp_folder_item_view_update_count_layout();

	if (nVideoListSize > 0)
	{
		__mp_folder_item_view_free_video_genlistItem();
		if (g_pFolderViewWidget->pNocontentsLayout) {
			__mp_folder_item_view_hide_no_layout();
			bUnset = TRUE;
		}
		//__mp_folder_item_view_append_video_items(pVideosGenlist);
		__mp_folder_item_view_append_grid_layout(pVideosGenlist);
		if (bUnset) {
			elm_object_part_content_set(g_pFolderViewWidget->pFolderViewBaselayout, SWALLOW_LISTVIEW_CONTENT, pVideosGenlist);
			evas_object_show(pVideosGenlist);
		}
	}
	else
	{
		__mp_folder_item_view_hide_genlist();
		__mp_folder_item_view_add_no_contents_layout();
		elm_object_part_content_set(g_pFolderViewWidget->pFolderViewBaselayout, SWALLOW_LISTVIEW_CONTENT, g_pFolderViewWidget->pNocontentsLayout);
		evas_object_show(g_pFolderViewWidget->pNocontentsLayout);
	}
	//__mp_folder_item_view_update_count_layout();

	return true;
}

void __mp_folder_item_view_show_search_view(void)
{
	VideoLogInfo("");

	mp_ft_ctrl_hide_more_popup();

	char *szFolderPath = NULL;
	szFolderPath = mp_util_svc_get_video_folder_url(g_pFolderViewHandle->nFolder_index);
	mp_search_view_push((void *)mp_mgr_get_library_naviframe(), szFolderPath);
	MP_FREE_STRING(szFolderPath);

	mp_search_view_register_update_list_func(__mp_folder_item_view_result_user_cb, __mp_folder_item_view_result_view_update_cb);
}

void __mp_folder_item_view_search_item_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");

	__mp_folder_item_view_show_search_view();
}

// Naviframe title button
Evas_Object *__mp_folder_item_view_create_title_btn(Evas_Object *pParent, Evas_Smart_Cb pFunc)
{
	VideoLogInfo("");
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s/%s/%s", path , "edje", VIDEO_PLAYER_IMAGE_NAME_EDJ);

	Evas_Object *search_ic = elm_image_add(pParent);
	elm_image_file_set(search_ic, edj_path, VIDEO_LIST_VIEW_ICON_SEARCH);
	elm_image_resizable_set(search_ic, EINA_TRUE, EINA_TRUE);
	evas_object_show(search_ic);

	Evas_Object *pTitleBtn = elm_button_add(pParent);
	elm_object_style_set(pTitleBtn, "naviframe/title_icon");
	elm_object_part_content_set(pTitleBtn, "icon", search_ic);
	evas_object_smart_callback_add(pTitleBtn, "clicked", pFunc, NULL);

	return pTitleBtn;
}

// Smart callback of Genlist.

/*static void __mp_folder_item_view_thumbnail_update_cb(int nError, const char *szPath, void *pUserData)
{
	int nVideoItemIndex = (int)pUserData;

	VideoLogInfo("Index : %d", nVideoItemIndex);

	if (!g_pFolderViewHandle) {
		VideoLogError("g_pFolderViewWidget is not existed.");
		return;
	}

	if (!g_pFolderViewWidget) {
		VideoLogError("g_pFolderViewWidget is not existed.");
		return;
	}

	mp_util_svc_update_thumbnail_info(nVideoItemIndex);

	if (g_pFolderViewWidget->pVideolistItemList) {
		Elm_Object_Item *pTmpVideoGenlistItem = (Elm_Object_Item*)eina_list_nth(g_pFolderViewWidget->pVideolistItemList, nVideoItemIndex);
		if (pTmpVideoGenlistItem) {
			elm_genlist_item_fields_update(pTmpVideoGenlistItem, "elm.icon.1", ELM_GENLIST_ITEM_FIELD_CONTENT);
		} else {
			VideoLogInfo("[ERR] Fail to update genlist filed thumbnail icon.");
		}
	} else {
		VideoLogInfo("[ERR] No exist g_pFolderViewWidget->pVideolistItemList.");
	}
	//	elm_genlist_realized_items_update(g_pMainViewHandle->pVideosGenlist);
}*/


static void __mp_folder_item_view_realize_genlist_item_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!pEventInfo) {
		VideoLogError("invalid pEveninfo");
		return;
	}
	if (elm_config_access_get()) {
		VideoLogWarning("Support TTS.");
		const Eina_List *org = elm_object_item_access_order_get(pEventInfo);
	    Evas_Object *content = elm_object_item_part_content_get(pEventInfo, "elm.icon");
	    if (content &&  !eina_list_data_find(org, content)) {
	    	Evas_Object *tmp;
	    	Eina_List *l;
	        Eina_List *items = NULL;
	        // Duplicate original access order
	        EINA_LIST_FOREACH((Eina_List *)org, l, tmp)
	        items = eina_list_append(items, tmp);
	        Evas_Object *ao = elm_access_object_register(content, pObject);
	        items = eina_list_append(items, ao);
	        elm_object_item_access_order_set(pEventInfo, items);
	    }
	}
}

static int __mp_folder_item_view_get_sort_type(void)
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

	VideoLogInfo("current sorttype = %d", nSortType);

	return nSortType;
}

static void __mp_folder_item_view_update_video_items(void *pVideosGenlist)
{
	if (!pVideosGenlist) {
		VideoLogInfo("[ERR] pVideosGenlist or g_pMainViewWidget are NULL.");
		return;
	}

	VideoLogInfo("");

	Elm_Object_Item *pNextGenlistItem = NULL;
	Elm_Object_Item *pCurrGenlistItem = elm_genlist_first_item_get(pVideosGenlist);

	while (pCurrGenlistItem)
	{

		pNextGenlistItem = elm_genlist_item_next_get(pCurrGenlistItem);
		elm_genlist_item_update(pCurrGenlistItem);
		pCurrGenlistItem = pNextGenlistItem;
	}
}

static void __mp_folder_item_view_db_result_data_update_cb(void)
{
	if (!g_pFolderViewHandle)
	{
		VideoLogInfo("g_pFolderViewHandle handles of list view are not existed.");
		return;
	}

	VideoLogInfo("");

	if (!g_pFolderViewHandle->pVideosGenlist)
	{
		VideoLogInfo("g_pMainViewHandle->pVideosGenlist is NULL.");
		return;
	}
	if (g_pFolderViewHandle->pFolderUrl)
	{
		mp_util_svc_destory_video_item_list();
		mp_util_svc_extract_video_list_from_folder(g_pFolderViewHandle->pFolderUrl, __mp_folder_item_view_get_sort_type(), MP_LIST_VIEW_ALL);
		//mp_util_svc_ctrl_sort_list(mp_sort_ctrl_get_sort_state());
	}
	else
	{
		VideoLogError("url is NULL.");
	}

	__mp_folder_item_view_update_video_items(g_pFolderViewHandle->pVideosGenlist);
}

static void __mp_folder_item_view_result_view_update_cb(void)
{
	if (!g_pFolderViewHandle)
	{
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}
	VideoLogInfo("");
	if (g_pFolderViewHandle->pFolderUrl)
	{
		mp_util_svc_destory_video_item_list();
		mp_util_svc_extract_video_list_from_folder(g_pFolderViewHandle->pFolderUrl, mp_sort_ctrl_get_sort_state(), MP_LIST_VIEW_ALL);
	}
	else
	{
		VideoLogError("url is NULL.");
	}
//	__mp_folder_item_view_check_search_button_enable();
	__mp_folder_item_view_update_video_list(LIST_UPDATE_TYPE_ALL);

	VideoLogInfo("");
}


static void __mp_folder_item_view_result_user_cb(MpListUpdateType eUpdateType)
{
	if (!g_pFolderViewHandle)
	{
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}

	VideoLogInfo("");
	if (__mp_folder_item_view_update_video_list(eUpdateType))
	{
		Elm_Object_Item *pItem = elm_genlist_first_item_get(g_pFolderViewHandle->pVideosGenlist);
		if (pItem)
		{
			elm_genlist_item_bring_in(pItem, ELM_GENLIST_ITEM_SCROLLTO_IN);
		}
	}
}


static void __mp_folder_item_view_delete_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pFolderViewHandle)
	{
		VideoLogError("g_pFolderViewHandle == NULL");
		return;
	}

	VideoLogInfo("");

	mp_ft_ctrl_hide_more_popup();
	char *szFolderPath = NULL;
	szFolderPath = mp_util_svc_get_video_folder_url(g_pFolderViewHandle->nFolder_index);

	mp_remove_view_push(g_pFolderViewHandle->pNaviFrameHandle, __mp_folder_item_view_result_user_cb, __mp_folder_item_view_result_view_update_cb, szFolderPath, LIST_TAB_TYPE_PERSONAL);
	MP_FREE_STRING(szFolderPath);

}

static void __mp_folder_item_view_sort_item_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");

	mp_ft_ctrl_hide_more_popup();

	mp_sort_ctrl_show(__mp_folder_item_view_db_result_data_update_cb);
}

void __mp_folder_item_view_rename_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pFolderViewHandle)
	{
		VideoLogError("g_pFolderViewHandle == NULL");
		return;
	}

	VideoLogInfo("");

	mp_ft_ctrl_hide_more_popup();
	char *szFolderPath = NULL;
	szFolderPath = mp_util_svc_get_video_folder_url(g_pFolderViewHandle->nFolder_index);
	mp_select_view_push((void *)mp_mgr_get_library_naviframe(), g_pFolderViewHandle->pNaviFrameItem,__mp_folder_item_view_result_user_cb, szFolderPath, SELECT_TYPE_RENAME, LIST_TAB_TYPE_PERSONAL);
	MP_FREE_STRING(szFolderPath);

}

/*void __mp_folder_item_view_details_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pFolderViewHandle)
	{
		VideoLogError("g_pFolderViewHandle == NULL");
		return;
	}

	VideoLogInfo("");

	mp_ft_ctrl_hide_more_popup();
	char *szFolderPath = NULL;
	szFolderPath = mp_util_svc_get_video_folder_url(g_pFolderViewHandle->nFolder_index);
	mp_select_view_push((void *)mp_mgr_get_library_naviframe(), g_pFolderViewHandle->pNaviFrameItem, NULL, szFolderPath, SELECT_TYPE_DETAIL, LIST_TAB_TYPE_PERSONAL);
	MP_FREE_STRING(szFolderPath);

}*/

static void __mp_folder_item_view_download_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pFolderViewHandle)
	{
		VideoLogError("g_pFolderViewHandle == NULL");
		return;
	}

	VideoLogInfo("");

	mp_ft_ctrl_hide_more_popup();
}

static void __mp_folder_item_view_share_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pFolderViewHandle)
	{
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}

	mp_ft_ctrl_hide_more_popup();

	char *szFolderPath = NULL;
	szFolderPath = mp_util_svc_get_video_folder_url(g_pFolderViewHandle->nFolder_index);

	mp_share_view_push(g_pFolderViewHandle->pNaviFrameHandle, __mp_folder_item_view_result_view_update_cb, szFolderPath, LIST_TAB_TYPE_PERSONAL);
	MP_FREE_STRING(szFolderPath);
}
#ifdef	ENABLE_PRIVATE_MODE
static void __mp_folder_item_view_add_personal_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pFolderViewHandle)
	{
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}

	mp_ft_ctrl_hide_more_popup();
	char *szFolderPath = NULL;
	szFolderPath = mp_util_svc_get_video_folder_url(g_pFolderViewHandle->nFolder_index);

	mp_personal_view_push(g_pFolderViewHandle->pNaviFrameHandle, __mp_folder_item_view_result_user_cb, szFolderPath, 0);
	MP_FREE_STRING(szFolderPath);
}

static void __mp_folder_item_view_remove_personal_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pFolderViewHandle)
	{
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}

	mp_ft_ctrl_hide_more_popup();
	char *szFolderPath = NULL;
	szFolderPath = mp_util_svc_get_video_folder_url(g_pFolderViewHandle->nFolder_index);
	mp_personal_view_push(g_pFolderViewHandle->pNaviFrameHandle, __mp_folder_item_view_result_user_cb, szFolderPath, 1);
	MP_FREE_STRING(szFolderPath);
}
#endif
#ifdef ENABLE_LONGPRESS
static void __mp_folder_item_view_share_via_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	mp_option_ctrl_delete_handle();

	if (!g_pFolderViewHandle)
	{
		VideoLogError("g_pFolderViewHandle of list view are not existed.");
		return;
	}

	int videoItemIndex = (int)pUserData;
	char *pVideo_url = mp_util_svc_get_video_url(videoItemIndex);
#ifdef ENABLE_DRM_FEATURE
	bool bDRMLock = FALSE;
	mp_util_check_drm_lock((const char*)pVideo_url, &bDRMLock);
	if (bDRMLock)
	{
		MP_FREE_STRING(pVideo_url);
		mp_util_notify_popup(NULL, VIDEOS_COMMON_HEADER_ERROR, VIDEOS_NOTIPOP_MSG_UNABLE_TO_SHARE_DRM_FILE, __mp_folder_item_view_warning_popup_cb);
		return;
	}
#endif
	mp_external_service_share_panel(pVideo_url);
	MP_FREE_STRING(pVideo_url);
	//to do share
}
#endif

void mp_folder_item_view_result_user_cb(void)
{
	VideoLogInfo("");
	__mp_folder_item_view_update_video_list(LIST_UPDATE_TYPE_ALL);
}

#ifdef ENABLE_LONGPRESS
static void __mp_folder_item_view_add_to_home_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	mp_option_ctrl_delete_handle();

	if (!g_pFolderViewHandle)
	{
		VideoLogInfo("g_pFolderViewHandle of list view are not existed.");
		return;
	}

	int nVideoItemIndex = (int)pUserData;

	VideoLogInfo("nVideoItemIndex : %d", nVideoItemIndex);


	char *pVideo_id = mp_util_svc_get_video_id_by_index(nVideoItemIndex);
	char *pVideo_thumbnail = mp_util_svc_get_video_thumbnail(nVideoItemIndex);
	if (pVideo_id)
	{
//		if (!mp_util_add_to_home(pVideo_id, pVideo_thumbnail))
		{
			mp_util_status_bar_noti_popup_show(MP_COM_OPT_UNABLE_ADDED);
		}
	}
	else
	{
		VideoLogError("[ERR] No exist file uri.");
	}
	MP_FREE_STRING(pVideo_id);
	MP_FREE_STRING(pVideo_thumbnail);

	//to do share
}
#endif

#if 0  //Unused function
bool __mp_folder_item_view_get_detail_info(int  nVideoItemIndex, stDetailInfo **pDetailInfo)
{
	char *szFilePath = NULL;
	char *szVideoTitle = NULL;

	char szTmpStr[FORMAT_LEN_MAX] = {0,};
	char szTmpFileDate[FORMAT_LEN_MAX] = {0,};
	char szTmpFileExtension[FORMAT_LEN_MAX] = {0,};
	char szTmpFileSize[FORMAT_LEN_MAX] = {0,};
	char *szTmpModifiedTime = NULL;

	//double dLongitude = 0.0;
	//double dLatitude = 0.0;

	int nWidth = 0;
	int nHeight = 0;
	stDetailInfo *pDetail = NULL;

	szFilePath = mp_util_svc_get_video_url(nVideoItemIndex);
	szVideoTitle = mp_util_svc_get_video_title(nVideoItemIndex);

	mp_info_ctrl_get_file_info(szFilePath, szTmpFileDate, FORMAT_LEN_MAX-1, szTmpFileExtension, FORMAT_LEN_MAX-1, szTmpFileSize, FORMAT_LEN_MAX-1);

	nWidth = mp_util_svc_get_video_item_width(nVideoItemIndex);
	nHeight = mp_util_svc_get_video_item_height(nVideoItemIndex);
	//dLongitude = mp_util_svc_get_video_item_longitude(nVideoItemIndex);
	//dLatitude = mp_util_svc_get_video_item_latitude(nVideoItemIndex);
	szTmpModifiedTime = mp_util_svc_get_video_item_modified_time(nVideoItemIndex);

	pDetail = calloc(1, sizeof(stDetailInfo));
	if (!pDetail) {
		MP_FREE_STRING(szFilePath);
		MP_FREE_STRING(szVideoTitle);
		VideoLogError("pDetailPopup alloc fail");
		return FALSE;
	}

	pDetail->szTitle = g_strdup(szVideoTitle);
	pDetail->szLocation = vp_util_convert_file_location(szFilePath);
	MP_FREE_STRING(szFilePath);

	snprintf(szTmpStr, FORMAT_LEN_MAX-1, "%s", szTmpFileExtension);
	pDetail->szFormat= g_strdup(szTmpStr);
	memset(szTmpStr, 0, FORMAT_LEN_MAX);

	/*snprintf(szTmpStr, FORMAT_LEN_MAX-1, "%s", szTmpFileDate);
	pDetail->szDate = g_strdup(szTmpStr);
	memset(szTmpStr, 0, FORMAT_LEN_MAX);*/

	snprintf(szTmpStr, FORMAT_LEN_MAX-1, "%s", szTmpFileSize);
	pDetail->szSize = g_strdup(szTmpStr);
	memset(szTmpStr, 0, FORMAT_LEN_MAX);

	pDetail->szLastModifiedTime = g_strdup(szTmpModifiedTime);
	MP_FREE_STRING(szTmpModifiedTime);

	snprintf(szTmpStr, FORMAT_LEN_MAX-1, "%d X %d", nWidth, nHeight);
	pDetail->szResolution = g_strdup(szTmpStr);
	memset(szTmpStr, 0, FORMAT_LEN_MAX);

	/*snprintf(szTmpStr, FORMAT_LEN_MAX-1, "%f", dLatitude);
	pDetail->szLatitude = g_strdup(szTmpStr);
	memset(szTmpStr, 0, FORMAT_LEN_MAX);

	snprintf(szTmpStr, FORMAT_LEN_MAX-1, "%f", dLongitude);
	pDetail->szLongitude = g_strdup(szTmpStr);
	memset(szTmpStr, 0, FORMAT_LEN_MAX);*/
	*pDetailInfo = pDetail;
	return TRUE;
}
#endif

#ifdef ENABLE_LONGPRESS
static void __mp_folder_item_view_normal_result_popup()
{
	if (!g_pFolderViewHandle)
	{
		VideoLogError("Invalid list and object");
		return;
	}
	evas_object_data_set(g_pFolderViewHandle->pVideosGenlist, "popup", NULL);
	elm_object_scroll_freeze_pop(g_pFolderViewHandle->pVideosGenlist);
}

static void __mp_folder_item_view_long_press_item_cb(void *data, Evas_Object *obj, void *pEventInfo)
{
	if (!g_pFolderViewHandle || !pEventInfo) {
		VideoLogError("Invalid list");
		return;
	}

	Elm_Object_Item *long_item = (Elm_Object_Item *)pEventInfo;

	//elm_genlist_item_selected_set(long_item, EINA_FALSE);
	int nIndexGenlist = elm_genlist_item_index_get(long_item);
	VideoLogDebug("nIndexGenlist : %d", nIndexGenlist);

	if (nIndexGenlist < 0 || nIndexGenlist >= mp_util_svc_get_video_list_size_for_checking_index())
	{
		VideoSecureLogError("nVideoItemIndex is invalid");
		return;
	}
	st_RegisteOptionCbFunc *pRegisteCbFunc = (st_RegisteOptionCbFunc*)calloc(1, sizeof(st_RegisteOptionCbFunc));
	if (pRegisteCbFunc) {
		pRegisteCbFunc->ShareViaItemCb= (void*)__mp_folder_item_view_share_via_cb;
		pRegisteCbFunc->AddToHomeItemCb= (void*)__mp_folder_item_view_add_to_home_cb;
		//pRegisteCbFunc->RenameItemCb = (void*)__mp_folder_item_view_rename_popup_cb;
		//pRegisteCbFunc->DetailsItemCb= (void*)__mp_folder_item_view_detail_cb;
		pRegisteCbFunc->DeleteItemCb = (void*)__mp_folder_item_view_delete_btn_cb;
	}

	char *title = mp_util_svc_get_video_title(nIndexGenlist);
	mp_option_ctrl_show(g_pFolderViewHandle->pNaviFrameHandle,
		(const char*)title, nIndexGenlist, pRegisteCbFunc, __mp_folder_item_view_normal_result_popup, NULL, 0);
	if (mp_option_ctrl_get_handle())
	{
		evas_object_data_set(g_pFolderViewHandle->pVideosGenlist, "popup", mp_option_ctrl_get_handle());
		elm_object_scroll_freeze_push(g_pFolderViewHandle->pVideosGenlist);
	}
	MP_FREE_STRING(pRegisteCbFunc);
	MP_FREE_STRING(title);

}
#endif

void __mp_folder_item_view_language_changed(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");
	if (!pObject) {
		VideoSecureLogError("error genlist handle");
	}
	elm_genlist_realized_items_update(pObject);
}


/*internal*/
static void __mp_folder_item_view_del_genlist_cb(void)
{
	if (!g_pFolderViewHandle || !g_pFolderViewHandle->pVideosGenlist) {
		VideoSecureLogError("g_pFolderViewHandle or pVideosGenlist is NULL");
		return;
	}
	VideoLogInfo("");
	evas_object_smart_callback_del(g_pFolderViewHandle->pVideosGenlist, "language,changed", __mp_folder_item_view_language_changed);
	evas_object_smart_callback_del(g_pFolderViewHandle->pVideosGenlist, "realized", __mp_folder_item_view_realize_genlist_item_cb);
#ifdef ENABLE_LONGPRESS
	evas_object_smart_callback_del(g_pFolderViewHandle->pVideosGenlist, "longpressed", __mp_folder_item_view_long_press_item_cb);
#endif
}

/*internal*/
static void __mp_folder_item_view_add_genlist_cb(void *pVideosGenlist)
{
	if (!g_pFolderViewHandle || !g_pFolderViewHandle->pVideosGenlist) {
		VideoSecureLogError("g_pFolderViewHandle is NULL");
		return;
	}
	VideoLogInfo("");

	evas_object_smart_callback_add(g_pFolderViewHandle->pVideosGenlist, "language,changed", __mp_folder_item_view_language_changed, NULL);
	evas_object_smart_callback_add(g_pFolderViewHandle->pVideosGenlist, "realized", __mp_folder_item_view_realize_genlist_item_cb, NULL);
#ifdef ENABLE_LONGPRESS
	evas_object_smart_callback_add(g_pFolderViewHandle->pVideosGenlist, "longpressed", __mp_folder_item_view_long_press_item_cb, NULL);
#endif
}

static bool __mp_folder_item_view_update_video_list(MpListUpdateType eUpdateType)
{
	if (!g_pFolderViewHandle || !g_pFolderViewWidget || !g_pFolderViewHandle->pVideosGenlist) {
		VideoLogError("handles of list view are not existed.");
		return false;
	}

	__mp_folder_item_view_del_genlist_cb();

	bool bRet = __mp_folder_item_view_arrange_video_list(__mp_folder_item_view_get_sort_type(), eUpdateType, g_pFolderViewHandle->pVideosGenlist);
	int nVideoListSize = 0;
	if (bRet) {
		nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();

		if (nVideoListSize > 0) {
			VideoLogInfo("listsize:%d", nVideoListSize);
			g_pFolderViewHandle->currentItemCount = nVideoListSize;
			__mp_folder_item_view_add_genlist_cb(g_pFolderViewHandle->pVideosGenlist);
		}
	}

	return bRet;
}

static void __mp_folder_item_view_db_changed_cb(void *pUserData)
{
	if (!g_pFolderViewHandle || !g_pFolderViewWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	if (g_pFolderViewHandle->pFolderUrl) {
		VideoSecureLogInfo("szFolderPath : %s", g_pFolderViewHandle->pFolderUrl);
		if (vp_file_exists(g_pFolderViewHandle->pFolderUrl))
		{
			//update video-list
			VideoLogInfo("update folder item");
			__mp_folder_item_view_result_user_cb(LIST_UPDATE_TYPE_ALL);
		}
		else
		{
			VideoLogInfo("pop folder item");
			elm_naviframe_item_pop(g_pFolderViewHandle->pNaviFrameHandle);
		}
	}
}

#ifdef ENABLE_LONGPRESS
static void __mp_folder_item_view_delete_video_items(int nVideoItemIndex)
{
	int nVideoListSize = g_pFolderViewHandle->currentItemCount;

	if (nVideoListSize == 0)
	{
		__mp_folder_item_view_update_video_list(LIST_UPDATE_TYPE_ALL);
		return;
	}

	if (!g_pFolderViewHandle || !g_pFolderViewWidget || !g_pFolderViewWidget->pVideolistItemList)
	{
		VideoLogWarning("[WARNING] g_pFolderViewWidget is not existed.");
		return;
	}

	Elm_Object_Item *pTmpVideoGenlistItem = (Elm_Object_Item*)eina_list_nth(g_pFolderViewWidget->pVideolistItemList, nVideoItemIndex);
	if (!pTmpVideoGenlistItem)
	{
		VideoLogWarning("[WARNING] pVideolistItemList is not existed.");
		return;
	}


	Elm_Object_Item *pNextGenlistItem = NULL;
	Elm_Object_Item *pCurrGenlistItem = elm_genlist_item_next_get(pTmpVideoGenlistItem);

	g_pFolderViewWidget->pVideolistItemList = eina_list_remove(g_pFolderViewWidget->pVideolistItemList, pTmpVideoGenlistItem);
	elm_object_item_del(pTmpVideoGenlistItem);

	if (!pCurrGenlistItem)
	{
		return;
	}

	while (pCurrGenlistItem)
	{
		pNextGenlistItem = elm_genlist_item_next_get(pCurrGenlistItem);
		g_pFolderViewWidget->pVideolistItemList = eina_list_remove(g_pFolderViewWidget->pVideolistItemList, pCurrGenlistItem);
		elm_object_item_del(pCurrGenlistItem);
		pCurrGenlistItem = pNextGenlistItem;
	}

	int nIndex = nVideoItemIndex;
	for (nIndex = nVideoItemIndex; nIndex < nVideoListSize; nIndex++)
	{
		pTmpVideoGenlistItem = elm_genlist_item_append(g_pFolderViewHandle->pVideosGenlist, &VideoFolder_Itc,
			(void*)nIndex, NULL, ELM_GENLIST_ITEM_NONE, __mp_folder_item_view_select_video_item_cb, NULL);
		g_pFolderViewWidget->pVideolistItemList = eina_list_append(g_pFolderViewWidget->pVideolistItemList, pTmpVideoGenlistItem);

		elm_object_item_del_cb_set(pTmpVideoGenlistItem, __mp_folder_item_view_video_item_del_cb);
	}
}
#endif

static void
__mp_folder_item_view_base_layout_del_cb(void *pUserData, Evas *e, Evas_Object *pObject, void *pEventInfo)
{
	//evas_object_event_callback_del(g_pListRemoveViewWidget->pRemoveViewBaselayout, EVAS_CALLBACK_DEL, __mp_remove_view_base_layout_del_cb);
	VideoLogInfo("");
	__mp_folder_item_view_reset();
}

static void __mp_folder_item_view_rotate_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!g_pFolderViewWidget || !g_pFolderViewHandle) {
		VideoLogError("__mp_folder_view_rotate_cb IS null");
		return;
	}
	bool bLandscape = mp_rotate_ctrl_check_landspace_by_win((Evas_Object*)mp_util_get_main_window_handle());
	if (bLandscape) {
		g_pFolderViewHandle->bLandscape = true;
	} else {
		g_pFolderViewHandle->bLandscape = false;
	}
	elm_genlist_clear(g_pFolderViewHandle->pVideosGenlist);
	__mp_folder_item_view_append_grid_layout(g_pFolderViewHandle->pVideosGenlist);
}

void  __mp_folder_soft_back_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	VideoLogInfo(" ");
	Evas_Object *pTopNaviFrame = NULL;
	pTopNaviFrame = elm_naviframe_item_pop(g_pFolderViewHandle->pNaviFrameHandle);
	evas_object_del(pTopNaviFrame);
}

static void __mp_folder_item_view_init(void *pParent)
{
	if (!g_pFolderViewHandle || !g_pFolderViewWidget)
	{
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	VideoLogInfo("");

	g_pFolderViewWidget->pFolderViewBox		= elm_box_add((Evas_Object*)pParent);
	elm_box_horizontal_set(g_pFolderViewWidget->pFolderViewBox	, EINA_FALSE);
	evas_object_show(g_pFolderViewWidget->pFolderViewBox);

	g_pFolderViewWidget->pFolderViewBaselayout = __mp_folder_item_view_init_base_layout(g_pFolderViewWidget->pFolderViewBox);
	evas_object_show(g_pFolderViewWidget->pFolderViewBaselayout);

	elm_box_pack_end(g_pFolderViewWidget->pFolderViewBox, g_pFolderViewWidget->pFolderViewBaselayout);

	evas_object_event_callback_add(g_pFolderViewWidget->pFolderViewBaselayout, EVAS_CALLBACK_DEL, __mp_folder_item_view_base_layout_del_cb, NULL);

	g_pFolderViewHandle->pVideosGenlist = elm_genlist_add(g_pFolderViewWidget->pFolderViewBaselayout);
	elm_scroller_bounce_set(g_pFolderViewHandle->pVideosGenlist, EINA_FALSE, EINA_TRUE);
	evas_object_size_hint_weight_set(g_pFolderViewHandle->pVideosGenlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pFolderViewHandle->pVideosGenlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_genlist_block_count_set(g_pFolderViewHandle->pVideosGenlist, VIDEO_GENLIST_BLOCK_COUNT);
	elm_scroller_single_direction_set(g_pFolderViewHandle->pVideosGenlist, ELM_SCROLLER_SINGLE_DIRECTION_HARD);
	elm_genlist_mode_set(g_pFolderViewHandle->pVideosGenlist, ELM_LIST_COMPRESS);
	elm_object_part_content_set(g_pFolderViewWidget->pFolderViewBaselayout, SWALLOW_LISTVIEW_CONTENT, g_pFolderViewHandle->pVideosGenlist);
	__mp_folder_item_view_update_video_list(LIST_UPDATE_TYPE_ALL);
	evas_object_show(g_pFolderViewHandle->pVideosGenlist);
	g_pFolderViewHandle->bLandscape = false;
	evas_object_smart_callback_add((Evas_Object*)mp_util_get_main_window_handle(), "wm,rotation,changed", __mp_folder_item_view_rotate_cb, NULL);

	Evas_Object * pLeftbtn = elm_button_add(g_pFolderViewHandle->pNaviFrameHandle);
	elm_object_style_set(pLeftbtn, "naviframe/end_btn/default");
	evas_object_smart_callback_add(pLeftbtn, "clicked", __mp_folder_soft_back_button_cb, NULL);

	char *folder_name = mp_util_svc_get_video_folder_name(g_pFolderViewHandle->nFolder_index);

	g_pFolderViewHandle->pNaviFrameItem = elm_naviframe_item_push(	g_pFolderViewHandle->pNaviFrameHandle,
																	(const char*)folder_name,
																	pLeftbtn,
																	NULL,
																	g_pFolderViewWidget->pFolderViewBox,
																	NULL	);

	elm_naviframe_item_pop_cb_set(g_pFolderViewHandle->pNaviFrameItem , __mp_folder_item_view_back_btn_cb, g_pFolderViewHandle);

	MP_FREE_STRING(folder_name);

	mp_folder_item_view_check_more_button_of_toolbar();

	elm_naviframe_item_title_enabled_set(g_pFolderViewHandle->pNaviFrameItem, TRUE, EINA_FALSE);

	mp_util_db_set_update_fun(g_pFolderViewHandle->euLev, __mp_folder_item_view_db_changed_cb);

	mp_util_hide_indicator();

}

bool mp_folder_item_view_is_current_view()
{
	return (g_pFolderViewHandle != NULL && !g_pFolderViewHandle->bExist);
}

void mp_folder_item_view_check_more_button_of_toolbar(void)
{
	if (!g_pFolderViewHandle) {
		VideoLogError("g_pFolderViewHandle is null");
		return;
	}

	st_RegisteCbFunc *pRegisteCbFunc = (st_RegisteCbFunc*)calloc(1, sizeof(st_RegisteCbFunc));
	if (!pRegisteCbFunc) {
		VideoLogError("failed to allocate memory");
		return;
	}

	pRegisteCbFunc->DownloadCb = (void*)__mp_folder_item_view_download_cb;
	pRegisteCbFunc->SearchCb = (void*)__mp_folder_item_view_search_item_cb;
	pRegisteCbFunc->ShareViaCb = (void*)__mp_folder_item_view_share_cb;
	pRegisteCbFunc->DeleteCb = (void*)__mp_folder_item_view_delete_cb;
	pRegisteCbFunc->SortItemCb = (void*)__mp_folder_item_view_sort_item_cb;
	pRegisteCbFunc->RenameCb = (void*)__mp_folder_item_view_rename_cb;
//	pRegisteCbFunc->DetailsCb= (void*)__mp_folder_item_view_details_cb;
#ifdef	ENABLE_PRIVATE_MODE
	pRegisteCbFunc->AddToPersonalCB = (void*)__mp_folder_item_view_add_personal_cb;
	pRegisteCbFunc->RemoveFromPersonalCB = (void*)__mp_folder_item_view_remove_personal_cb;
#endif
	mp_ft_ctrl_create(g_pFolderViewHandle->pNaviFrameHandle, g_pFolderViewHandle->pNaviFrameItem, FOOTER_TAB_TYPE_PERSONAL, pRegisteCbFunc);
	free(pRegisteCbFunc);

	elm_naviframe_item_title_enabled_set(g_pFolderViewHandle->pNaviFrameItem, EINA_TRUE, EINA_FALSE);
	mp_util_db_set_update_fun(g_pFolderViewHandle->euLev, __mp_folder_item_view_db_changed_cb);

	mp_util_hide_indicator();
}

char*
__mp_folder_item_view_text_get_video_number_cb(void *pUserData, Evas_Object *pObj, const char *part)
{
	VideoLogInfo("");

	if (!g_pFolderViewHandle)
	{
		VideoLogWarning("g_pFolderViewHandle is NULL.");
		return NULL;
	}

	if (!g_pFolderViewWidget)
	{
		VideoLogWarning("g_pFolderViewWidget is NULL.");
		return NULL;
	}

	if (!g_strcmp0(part, "elm.text.sub"))
	{
		int nCount = (int)pUserData;

		if (nCount == 1)
		{
			return g_strdup_printf(VIDEOS_LISTVIEW_BODY_ONE_VIDEO);
		}
		else
		{
			return g_strdup_printf(VIDEOS_LISTVIEW_BODY_PD_VIDEOS, nCount);
		}
	}

	return NULL;
}


/*static void __mp_folder_item_title_layout_del_cb(void *pUserData, Evas *e, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pFolderViewWidget)
	{
		VideoLogError("g_pMainViewWidget == NUL!!!");
		return;
	}

	g_pFolderViewWidget->pFolderViewCountlayout = NULL;
	g_pFolderViewWidget->pFolderViewGenlistTitle = NULL;
	g_pFolderViewWidget->pFolderViewGenlistItemForTitle = NULL;
}*/

/*static void __mp_folder_video_index_item_del_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!g_pFolderViewWidget)
	{
		VideoLogError("g_pMainViewWidget == NUL!!!");
		return;
	}

	g_pFolderViewWidget->pFolderViewGenlistItemForTitle = NULL;
}*/


/*static void
__mp_folder_item_view_update_count_layout()
{
	unsigned int 	nCount				= mp_util_svc_get_number_of_video_item_by_type();

	VideoLogInfo("nCount : [%d]", nCount);

	if (nCount == 0)
	{
		if (g_pFolderViewWidget->pFolderViewCountlayout != NULL)
		{
			elm_box_unpack(g_pFolderViewWidget->pFolderViewBox, g_pFolderViewWidget->pFolderViewCountlayout);
			evas_object_del(g_pFolderViewWidget->pFolderViewCountlayout);
			g_pFolderViewWidget->pFolderViewCountlayout	= NULL;
		}
	}
	else
	{
		if (g_pFolderViewWidget->pFolderViewGenlistItemForTitle != NULL)	{
			elm_object_item_del(g_pFolderViewWidget->pFolderViewGenlistItemForTitle);
		}

		if (g_pFolderViewWidget->pFolderViewGenlistTitleItc == NULL) {

			g_pFolderViewWidget->pFolderViewGenlistTitleItc = elm_genlist_item_class_new();
			g_pFolderViewWidget->pFolderViewGenlistTitleItc->item_style = "groupindex.sub";
			g_pFolderViewWidget->pFolderViewGenlistTitleItc->func.text_get = __mp_folder_item_view_text_get_video_number_cb;
			g_pFolderViewWidget->pFolderViewGenlistTitleItc->func.content_get = NULL;
			g_pFolderViewWidget->pFolderViewGenlistTitleItc->func.del = NULL;
		}

		if (g_pFolderViewWidget->pFolderViewGenlistTitle == NULL)	{
			g_pFolderViewWidget->pFolderViewGenlistTitle = elm_genlist_add(g_pFolderViewWidget->pFolderViewBox);
			elm_genlist_mode_set(g_pFolderViewWidget->pFolderViewGenlistTitle, ELM_LIST_COMPRESS);
			evas_object_show(g_pFolderViewWidget->pFolderViewGenlistTitle);
		}

		if (g_pFolderViewWidget->pFolderViewCountlayout == NULL)	{
			// create layout
			g_pFolderViewWidget->pFolderViewCountlayout = elm_layout_add(g_pFolderViewWidget->pFolderViewBox);
			evas_object_size_hint_weight_set(g_pFolderViewWidget->pFolderViewCountlayout, EVAS_HINT_EXPAND, 0);
			evas_object_size_hint_align_set(g_pFolderViewWidget->pFolderViewCountlayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
			elm_layout_file_set(g_pFolderViewWidget->pFolderViewCountlayout, VIDEO_CUSTOM_THEME, "count_info_groupindex");
			evas_object_event_callback_add(g_pFolderViewWidget->pFolderViewCountlayout, EVAS_CALLBACK_DEL, __mp_folder_item_title_layout_del_cb, NULL);

			evas_object_show(g_pFolderViewWidget->pFolderViewCountlayout);

			elm_layout_content_set(g_pFolderViewWidget->pFolderViewCountlayout, "elm.swallow.groupindex", g_pFolderViewWidget->pFolderViewGenlistTitle);

			elm_box_pack_start(g_pFolderViewWidget->pFolderViewBox, g_pFolderViewWidget->pFolderViewCountlayout);
		}

		g_pFolderViewWidget->pFolderViewGenlistItemForTitle = elm_genlist_item_append(g_pFolderViewWidget->pFolderViewGenlistTitle,
																			g_pFolderViewWidget->pFolderViewGenlistTitleItc,
																			(void *)nCount,
																			NULL,
																			ELM_GENLIST_ITEM_NONE,
																			NULL,
																			NULL);
		elm_object_item_del_cb_set(g_pFolderViewWidget->pFolderViewGenlistItemForTitle, __mp_folder_video_index_item_del_cb);
	}
}*/


void mp_folder_item_view_push(void *pNaviFrame, int folder_index, UpdateListItemsCbFunc pChangeViewCb)
{
	if (!pNaviFrame)
	{
		VideoLogError("[ERR] pNaviFrame");
		return;
	}

	VideoLogInfo("");
	__mp_folder_item_view_destroy();
	g_pFolderViewHandle = (st_VideoFolderViewHandle*)calloc(1, sizeof(st_VideoFolderViewHandle));
	g_pFolderViewWidget = (st_VideoFolderViewWidget*)calloc(1, sizeof(st_VideoFolderViewWidget));

	memset(g_pFolderViewHandle, 0, sizeof(st_VideoFolderViewHandle));
	memset(g_pFolderViewWidget, 0, sizeof(st_VideoFolderViewWidget));

	g_pFolderViewHandle->pNaviFrameHandle = pNaviFrame;
	g_pFolderViewHandle->pUpdateListUserCbFunc = pChangeViewCb;
	g_pFolderViewHandle->nFolder_index = folder_index;
	g_pFolderViewHandle->bExist = false;
	g_pFolderViewHandle->bLandscape = mp_rotate_ctrl_check_landspace_by_win((Evas_Object*)mp_util_get_main_window_handle());
	char *szPath = mp_util_svc_get_video_folder_url(folder_index);
	g_pFolderViewHandle->pFolderUrl = g_strdup(szPath);
	VideoSecureLogInfo("folder path = %s", szPath);
	MP_FREE_STRING(szPath);

	g_pFolderViewHandle->euLev = MP_DB_UPDATE_LEV_1;

	__mp_folder_item_view_init(g_pFolderViewHandle->pNaviFrameHandle);
	g_pFolderViewHandle->nPlayVideoIndex = -1;

}

void mp_folder_item_view_update_played_item(void)
{
	if (!g_pFolderViewHandle || !g_pFolderViewWidget) {
		VideoLogInfo("Folderitem View does not exist.");
		return;
	}
	VideoLogInfo("");

	if (g_pFolderViewHandle->pVideosGenlist) {
		int nGenItemIndex = g_pFolderViewHandle->nPlayVideoIndex;
		VideoLogInfo("%d", nGenItemIndex);

		int nGenlistIndex = nGenItemIndex;
		if (!g_pFolderViewHandle->bLandscape)
			nGenlistIndex = nGenlistIndex / VP_GENLIST_THUMB_PORTRAIT_COUNT;
		else
			nGenlistIndex = nGenlistIndex / VP_GENLIST_THUMB_LANDSPACE_COUNT;

		Elm_Object_Item *pTmpVideoGenlistItem = NULL;
		pTmpVideoGenlistItem = elm_genlist_nth_item_get((const Evas_Object *)g_pFolderViewHandle->pVideosGenlist, nGenlistIndex);
		mp_util_svc_destory_video_item_list();
		mp_util_svc_extract_video_list_from_folder(g_pFolderViewHandle->pFolderUrl, __mp_folder_item_view_get_sort_type(), MP_LIST_VIEW_ALL);
		mp_util_svc_update_thumbnail_info(g_pFolderViewHandle->nPlayVideoIndex);
		elm_genlist_item_update(pTmpVideoGenlistItem);
		if (pTmpVideoGenlistItem) {
			elm_genlist_clear(g_pFolderViewHandle->pVideosGenlist);
			__mp_folder_item_view_append_grid_layout(g_pFolderViewHandle->pVideosGenlist);
			//elm_genlist_item_update(pTmpVideoGenlistItem);
			//elm_genlist_item_fields_update(pTmpVideoGenlistItem, "elm.icon.1", ELM_GENLIST_ITEM_FIELD_CONTENT);
			elm_genlist_item_selected_set(pTmpVideoGenlistItem, EINA_FALSE);
			int nLastPlayedTime = mp_util_svc_get_video_last_played_pos(nGenItemIndex);
			int nDurationTime = mp_util_svc_get_video_duration_time(nGenItemIndex);
			Evas_Object *pIconContent = elm_object_item_part_content_get(pTmpVideoGenlistItem, "elm.icon.1");
			if (!pIconContent) {
				VideoLogError("");
				return;
			}
			Evas_Object *pProgressbar = elm_layout_content_get(pIconContent, "elm.progressbar.icon");

			if (nLastPlayedTime > 0 && nDurationTime > 0) {
				double nRatio = ((double)nLastPlayedTime) / ((double)nDurationTime);
				if (!pProgressbar) {
					elm_genlist_item_fields_update(pTmpVideoGenlistItem, "elm.icon.1", ELM_GENLIST_ITEM_FIELD_CONTENT);
				} else {
					elm_progressbar_value_set(pProgressbar, nRatio);
				}
			} else {
				if (pProgressbar) {
					elm_layout_content_unset(pIconContent, "elm.progressbar.icon");
					MP_DEL_OBJ(pProgressbar);
				}
			}

			elm_genlist_item_fields_update(pTmpVideoGenlistItem, "elm.text.sub.left.bottom", ELM_GENLIST_ITEM_FIELD_TEXT);
		}
	}
}

void mp_folder_item_view_change_language(void)
{
	if (!g_pFolderViewHandle || !g_pFolderViewWidget)
	{
		VideoLogError("No existed handle of Folder item view.");
		return;
	}

	VideoLogInfo("");

	//__mp_folder_item_view_update_count_layout();
}

/*external interface*/
void mp_folder_item_view_update_video_list(MpListUpdateType eUpdateType)
{
	if (!g_pFolderViewHandle || !g_pFolderViewWidget || !g_pFolderViewHandle->pVideosGenlist)
	{
		VideoLogError("handles of list view are not existed.");
		return;
	}
	if (eUpdateType != LIST_UPDATE_TYPE_NONE) {
		__mp_folder_item_view_update_video_list(eUpdateType);
	}
}


