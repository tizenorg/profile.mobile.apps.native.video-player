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

#include "mp-util.h"
#include "mp-video-log.h"
#include "mp-external-ug.h"
#include "mp-video-string-define.h"
#include "mp-video-value-define.h"
#include "mp-util-media-service.h"
#include "mp-video-list-view-select.h"
#include "mp-video-list-sort-ctrl.h"
#include "mp-video-view-popup-ctrl.h"
#include "mp-video-list-view-as-ctrl.h"
#include "mp-util-widget-ctrl.h"
#include "mp-video-util-db-controller.h"
#include "mp-video-rename-ctrl.h"
#include "mp-video-detail-view.h"
#include "vp-file-util.h"
#include "mp-rotate-ctrl.h"
#include "videos-view-mgr.h"

typedef struct
{
	void *pNaviFrameHandle;
	void *pNaviFrameItem;
	void *pPreNaviItem;
	void *pGenlist;
	Elm_Genlist_Item_Class *pGenListItc;
	Elm_Genlist_Item_Class *pGenListLandscapeItc;
	Elm_Gengrid_Item_Class *pGenGridItc_1;
	Elm_Gengrid_Item_Class *pGenGridItc_2;
	MpMediaSvcSortType nSortType;
	MpDbUpdateViewLev euLev;
	ChangeSelectViewCbFunc pChangeViewUserCbFunc;
	char *pFolderUrl;
	MpVideoListTabType eTabType;
	SelectType eSelType;
	int nViewType;
	bool bRename;
#ifdef VS_FEATURE_THUMBNAIL_VIEW	//
	bool bLandscape;
#endif
}st_SelectViewHandle;

typedef struct
{
	Evas_Object *pBox;
	Evas_Object *pBaselayout;
}st_SelectViewWidget;

static st_SelectViewHandle *g_pSelectHandle = NULL;
static st_SelectViewWidget *g_pSelectWidget = NULL;

static Evas_Object *g_pRenameLayout = NULL;

// Pre define function
int mp_select_view_get_sort_type(void);
bool mp_select_view_arrange_video_list(void *pGenlist, bool bNeedSvc);
void mp_select_view_naviframe_transition_effect_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo);
static void __mp_select_view_base_layout_del_cb(void *pUserData, Evas *e, Evas_Object *pObject, void *pEventInfo);

void mp_select_view_destroy(void)
{
	VideoLogInfo("");

	if (g_pSelectHandle)
	{
		evas_object_smart_callback_del(g_pSelectHandle->pNaviFrameHandle, "transition,finished", mp_select_view_naviframe_transition_effect_cb);

		MP_DEL_OBJ(g_pSelectHandle->pGenlist);
		MP_DEL_ITC(g_pSelectHandle->pGenListItc);
		MP_FREE_STRING(g_pSelectHandle->pFolderUrl);
		MP_FREE_STRING(g_pSelectHandle);
	}

	if (g_pSelectWidget)
	{
		MP_DEL_OBJ(g_pSelectWidget->pBox);

		if (g_pSelectWidget->pBaselayout != NULL)
		{
			evas_object_event_callback_del(g_pSelectWidget->pBaselayout, EVAS_CALLBACK_DEL, __mp_select_view_base_layout_del_cb);
			MP_DEL_OBJ(g_pSelectWidget->pBaselayout);
		}
		MP_FREE_STRING(g_pSelectWidget);
	}
}

static void __mp_select_view_reset(void)
{
	VideoLogInfo("");

	if (g_pSelectHandle)
	{
		MP_FREE_STRING(g_pSelectHandle->pFolderUrl);
		mp_util_db_set_update_fun(g_pSelectHandle->euLev, NULL);
		mp_util_db_set_backup_fun(g_pSelectHandle->euLev, NULL);
		evas_object_smart_callback_del(g_pSelectHandle->pNaviFrameHandle, "transition,finished", mp_select_view_naviframe_transition_effect_cb);
		g_pSelectHandle->pGenlist = NULL;
		MP_FREE_STRING(g_pSelectHandle);
	}

	if (g_pSelectWidget)
	{

		g_pSelectWidget->pBox = NULL;
		g_pSelectWidget->pBaselayout = NULL;

		MP_FREE_STRING(g_pSelectWidget);
	}
}

void mp_select_view_reset_value(void)
{
	VideoLogInfo("");

	if (!g_pSelectHandle)
	{
		VideoLogError("[ERR] g_pSelectHandle is not existed.");
		return;
	}

	g_pSelectHandle->pNaviFrameHandle = NULL;
	g_pSelectHandle->pNaviFrameItem = NULL;
	g_pSelectHandle->pChangeViewUserCbFunc = NULL;
	g_pSelectHandle->nSortType = MP_MEDIA_SORT_BY_RECENTLY_VIEWED;
}

static void __mp_select_view_trans_finished_cb(void *pUserData, Evas_Object *obj, void *event_info)
{
	if (!obj)
	{
		VideoLogError("[ERR] g_pSelectHandle is NULL.");
		return;
	}

	mp_widget_ctrl_enable_navi_handle_focus(obj);

	evas_object_smart_callback_del(obj, "transition,finished", __mp_select_view_trans_finished_cb);
}

static Eina_Bool  mp_select_view_back_btn_cb(void *pUserData, Elm_Object_Item *pItem)
{
	if (!g_pSelectHandle)
	{
		VideoLogInfo("[ERR] g_pSelectHandle is NULL.");
		return EINA_TRUE;
	}

	// Register transition finished callback.
	mp_widget_ctrl_disable_navi_handle_focus_except_item(g_pSelectHandle->pNaviFrameHandle, g_pSelectHandle->pNaviFrameItem);
	evas_object_smart_callback_add(g_pSelectHandle->pNaviFrameHandle, "transition,finished", __mp_select_view_trans_finished_cb, NULL);

	mp_select_view_pop();

	return EINA_TRUE;
}

void mp_select_view_naviframe_transition_effect_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pSelectHandle)
	{
		VideoLogError("[ERR] No exist g_pSelectHandle.");
		return;
	}

	VideoLogInfo("");

	Evas_Object *pTmpContents = NULL;
	Elm_Object_Item *pTmpItem = NULL;

	if (!g_pSelectHandle->pNaviFrameHandle)
	{
		VideoLogInfo("[ERR] No exist naviframe handle.");
		return;
	}

	pTmpItem = elm_naviframe_top_item_get(g_pSelectHandle->pNaviFrameHandle);
	pTmpContents = elm_object_item_content_get(pTmpItem);

	if (g_pRenameLayout) {
		mp_rename_ctrl_set_focus();
	}

	if (pTmpContents)
	{
		if (pTmpContents == g_pSelectWidget->pBox)
		{
		}
		else
		{
			VideoLogInfo("It's not pListShareViaViewTabbarLayout in top screen.");
		}
	}
	else
	{
		VideoLogError("[ERR] No exist naviframe top item.");
	}
}

static char *__mp_select_view_get_label_of_folder_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{

	int nFolderItemIndex = (int)pUserData;

	if (!g_strcmp0(pPart, "elm.text.main.left.top"))
	{
		char *szTitle = NULL;
		szTitle = mp_util_svc_get_video_folder_name(nFolderItemIndex);

		char *szTitleUtf8 = elm_entry_utf8_to_markup(szTitle);

		MP_FREE_STRING(szTitle);
		return szTitleUtf8;
	}
	else if (!g_strcmp0(pPart, "elm.text.sub.left.bottom"))
	{
		char *szFolderPath = NULL;
		szFolderPath = mp_util_svc_get_video_folder_url(nFolderItemIndex);
		char *pDes_path = mp_util_get_dir_by_path(szFolderPath);
		MP_FREE_STRING(szFolderPath);
		char *szFolderUtf8 = elm_entry_utf8_to_markup(pDes_path);
		MP_FREE_STRING(pDes_path);
		return szFolderUtf8;

	}

	return NULL;
}

static Evas_Object *__mp_select_view_get_icon_of_folder_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{
	int nFolderItemIndex = (int)pUserData;
	VideoLogInfo("Index : %d", nFolderItemIndex);

	if (!g_strcmp0(pPart, "elm.icon.1"))
	{
		VideoLogInfo("pPart: elm.icon.1 - thumbnail");

		Evas_Object *pLayout = NULL;
		Evas_Object *pBg = NULL;
		char *pThumbIconUri = mp_util_get_folder_thumbnail(nFolderItemIndex, mp_sort_ctrl_get_sort_state());

		pLayout = elm_layout_add(pObject);
		elm_layout_file_set(pLayout, VIDEO_CUSTOM_THEME, "listview.thumbnail.layout");

		if (!pThumbIconUri || !vp_file_exists(pThumbIconUri))
		{
			MP_FREE_STRING(pThumbIconUri);
		}

		pBg = mp_util_create_preload_image(pLayout, pThumbIconUri, VIDEO_ICON_WIDTH);
		elm_layout_content_set(pLayout, "elm.thumbnail.icon", pBg);
		if (pBg)
			evas_object_show(pBg);

		if (!mp_util_create_folder_sub_icon(pLayout, nFolderItemIndex))	{
			VideoLogWarning("Create sub icon is failed. nFolderItemIndex: %d", nFolderItemIndex);
		}

		MP_FREE_STRING(pThumbIconUri);

		evas_object_show(pLayout);

		return pLayout;
	}

	return NULL;
}

int mp_select_view_get_sort_type(void)
{

	switch (mp_sort_ctrl_get_sort_state())
	{
	/*case MP_LIST_SORT_BY_RECENTLY_VIEWED:
		g_pSelectHandle->nSortType = MP_MEDIA_SORT_BY_RECENTLY_VIEWED;
		break;*/

	case MP_LIST_SORT_BY_RECENTLY_ADDED:
		g_pSelectHandle->nSortType = MP_MEDIA_SORT_BY_RECENTLY_ADDED;
		break;

	case MP_LIST_SORT_BY_NAME:
		g_pSelectHandle->nSortType = MP_MEDIA_SORT_BY_NAME;
		break;

	/*case MP_LIST_SORT_BY_SIZE:
		g_pSelectHandle->nSortType = MP_MEDIA_SORT_BY_SIZE;
		break;

	case MP_LIST_SORT_BY_TYPE:
		g_pSelectHandle->nSortType = MP_MEDIA_SORT_BY_TYPE;
		break;*/
	}

	return g_pSelectHandle->nSortType;
}

#ifdef VS_FEATURE_LIST_VIEW
static char *__mp_select_view_get_label_of_video_item_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{
	int nVideoItemIndex = (int)pUserData;

	if (!strcmp(pPart, "elm.text.main.left.top"))
	{
		char *szTitle = mp_util_svc_get_video_title(nVideoItemIndex);
		char *szTitleUtf8 = elm_entry_utf8_to_markup(szTitle);
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

static Evas_Object *__mp_select_view_get_icon_of_video_item_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{
	Evas_Object *pLayout = NULL;

	int nVideoItemIndex = (int)pUserData;
	if (!pPart || !pObject)
	{
		return NULL;
	}

	if (!strcmp(pPart, "elm.icon.1"))
	{
		VideoLogInfo("pPart: elm.icon.1 - thumbnail");

		char *pThumbIconUri = mp_util_svc_get_video_thumbnail(nVideoItemIndex);
		Evas_Object *pBg = NULL;

		pLayout = elm_layout_add(pObject);
		elm_layout_file_set(pLayout, VIDEO_CUSTOM_THEME, "listview.thumbnail.layout");

		if (!pThumbIconUri || !vp_file_exists(pThumbIconUri))
		{
			MP_FREE_STRING(pThumbIconUri);
		}

		pBg = mp_util_create_preload_image(pLayout, pThumbIconUri, VIDEO_ICON_WIDTH);
		elm_layout_content_set(pLayout, "elm.thumbnail.icon", pBg);

		if (pBg)
			evas_object_show(pBg);

		MP_FREE_STRING(pThumbIconUri);

		int nLastPlayedTime = mp_util_svc_get_video_last_played_pos(nVideoItemIndex);
		int nDurationTime = mp_util_svc_get_video_duration_time(nVideoItemIndex);

		if (nLastPlayedTime > 0)
		{
			double nRatio = ((double)nLastPlayedTime) / ((double)nDurationTime);
			if (nRatio < 0.05) {
				nRatio = 0.05;
			}

			Evas_Object *pProgressBar = elm_progressbar_add(pLayout);
			elm_object_style_set(pProgressBar, "elm/progressbar/horizontal/default");
			elm_progressbar_horizontal_set(pProgressBar, EINA_TRUE);
			elm_progressbar_span_size_set(pProgressBar, VIDEO_LIST_PROGRESSBAR_LENGTH);
			evas_object_size_hint_align_set(pProgressBar, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(pProgressBar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_progressbar_value_set(pProgressBar, nRatio);
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
	}

	MP_DEL_OBJ(pLayout);

	return NULL;
}
#endif

/*if other view need update select view, need register this update callback*/
static void __mp_select_view_result_view_update_cb(void)
{
	if (!g_pSelectHandle)
	{
		VideoLogInfo("g_pSelectHandle is null.");
		return;
	}
	if (!g_pSelectHandle->bRename)
		g_pSelectHandle->bRename = TRUE;


	bool bNormalShow = mp_select_view_arrange_video_list(g_pSelectHandle->pGenlist, TRUE);
	if (bNormalShow)
	{
		Elm_Object_Item *pItem = NULL;
		pItem = elm_genlist_first_item_get(g_pSelectHandle->pGenlist);
		if (pItem)
		{
			elm_genlist_item_bring_in(pItem, ELM_GENLIST_ITEM_SCROLLTO_IN);
		}
	}
	else
	{
		elm_naviframe_item_pop(g_pSelectHandle->pNaviFrameHandle);
	}
	g_pSelectHandle->pChangeViewUserCbFunc(MP_LIST_VIEW_ALL);
	elm_naviframe_item_pop_to(g_pSelectHandle->pPreNaviItem);
}

static void __mp_select_view_show_detail_view(int nIndex, bool IsFolder)
{

	if (!g_pSelectHandle)
	{
		VideoLogInfo("g_pSelectHandle of list view are not existed.");
		return;
	}

	VideoLogInfo("nVideoItemIndex : %d", nIndex);

	stDetailInfo *pDetail = NULL;
	char *pMediaId = NULL;
	if (IsFolder) {
		mp_detail_view_get_folder_detail_info(nIndex, &pDetail);
		pMediaId = mp_util_svc_get_video_folder_id(nIndex);
	}
	else
	{
		mp_detail_view_get_video_detail_info(nIndex, &pDetail);
		pMediaId = mp_util_svc_get_video_id_by_index(nIndex);
	}

	mp_detail_view_push((void *)g_pSelectHandle->pNaviFrameHandle, IsFolder, pMediaId, pDetail);
	mp_detail_view_free_detail_info(pDetail);
	MP_FREE_STRING(pMediaId);
}

static void __mp_select_view_genlist_selected_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pSelectHandle || !pObject)
	{
		VideoLogWarning("[WARNING] Main handles of list view are not existed.");
		return;
	}
#ifdef VS_FEATURE_THUMBNAIL_VIEW
	Elm_Object_Item *pSelectedItem = elm_gengrid_selected_item_get(pObject);
	if (pSelectedItem) {
		elm_gengrid_item_selected_set(pSelectedItem, EINA_FALSE);
	}
#else
	Elm_Object_Item *pSelectedItem = elm_genlist_selected_item_get(pObject);
	if (pSelectedItem) {
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}
#endif
	int nSelectedIndex = (int)pUserData;
	VideoLogInfo("selected index is %d", nSelectedIndex);

	if (SELECT_TYPE_DETAIL == g_pSelectHandle->eSelType) {
		VideoLogInfo("Show detail view");
		/*Todo: show detail view*/
		if (MP_LIST_VIEW_AS_THUMBNAIL_LIST == g_pSelectHandle->nViewType) {
			__mp_select_view_show_detail_view(nSelectedIndex, FALSE);
		} else if (MP_LIST_VIEW_AS_FOLDER_LIST == g_pSelectHandle->nViewType) {
			if (g_pSelectHandle->pFolderUrl) {
				VideoLogWarning("Folder item view");
				__mp_select_view_show_detail_view(nSelectedIndex, FALSE);
			} else {
				VideoLogWarning("Folder view");
				__mp_select_view_show_detail_view(nSelectedIndex, TRUE);
			}
		}
	}
	else if (SELECT_TYPE_RENAME == g_pSelectHandle->eSelType)
	{
		VideoLogInfo("Show rename popup");

		char *szTitle = NULL;
		szTitle = g_strdup(VIDEOS_SELECTVIEW_HEADER_SELECT_VIDEO);
		g_pRenameLayout = mp_rename_ctrl_show(g_pSelectHandle->pNaviFrameHandle, NULL, (void*)__mp_select_view_result_view_update_cb, nSelectedIndex);
		Evas_Object * pLeftbtn = elm_button_add(g_pSelectHandle->pNaviFrameHandle);
		elm_object_style_set(pLeftbtn, "naviframe/end_btn/default");
		elm_naviframe_item_push(g_pSelectHandle->pNaviFrameHandle, szTitle, pLeftbtn, NULL, g_pRenameLayout, NULL);
	}
}

static void __mp_select_view_append_folder_items(void *pFolderGenlist)
{
	VideoLogInfo("");
	int nIndex = 0;
	int nFolderListSize = 0;

	nFolderListSize = mp_util_svc_get_video_folder_size();
	VideoLogInfo("nVideoListSize : %d", nFolderListSize);

	if (nFolderListSize > 0)
	{
		MP_DEL_ITC(g_pSelectHandle->pGenListItc);
		g_pSelectHandle->pGenListItc = elm_genlist_item_class_new();
		g_pSelectHandle->pGenListItc->item_style = "2line.top";
		g_pSelectHandle->pGenListItc->func.text_get = (void*)__mp_select_view_get_label_of_folder_cb;
		g_pSelectHandle->pGenListItc->func.content_get = (void*)__mp_select_view_get_icon_of_folder_cb;
		g_pSelectHandle->pGenListItc->func.state_get = NULL;
		g_pSelectHandle->pGenListItc->func.del = NULL;
		g_pSelectHandle->pGenListItc->decorate_item_style = NULL;

		for (nIndex = 0; nIndex < nFolderListSize; nIndex++)
		{
			VideoLogInfo("nCounter : %d", nIndex);
			elm_genlist_item_append(pFolderGenlist, g_pSelectHandle->pGenListItc, (void*)nIndex, NULL, ELM_GENLIST_ITEM_NONE, __mp_select_view_genlist_selected_cb, (void *)nIndex);
		}
	}
}

#ifdef VS_FEATURE_THUMBNAIL_VIEW
static Evas_Object *__mp_select_view_create_gengrid(Evas_Object *parent, int nWidth, int nHeight)
{
	Evas_Object *pGengrid;
	pGengrid = elm_gengrid_add(parent);
	if (!pGengrid) {
		VideoLogError("pVideoGrid does not exist.");
		return NULL;
	}
	evas_object_size_hint_weight_set(pGengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pGengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_gengrid_item_size_set(pGengrid, nWidth, nHeight);
	elm_gengrid_align_set(pGengrid, 0.0, 0.0);
	elm_gengrid_horizontal_set(pGengrid, EINA_TRUE);

	return pGengrid;
}

static char *__mp_select_view_get_label_of_grid_item_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{
	int nVideoItemIndex = (int)pUserData;

	if (!strcmp(pPart, "elm.text")) {
		char *szTitle;
		szTitle = mp_util_svc_get_video_title(nVideoItemIndex);
		char *szTitleUtf8 = elm_entry_utf8_to_markup(szTitle);

		if (szTitle) {
			free(szTitle);
			szTitle = NULL;
		}

		return szTitleUtf8;
	} else if (!strcmp(pPart, "elm.text.2"))
	{
		return mp_util_svc_get_duration_str_time(nVideoItemIndex);
	}

	return NULL;
}

static Evas_Object *__mp_select_view_get_grid_icon_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{
	int nVideoItemIndex = (int)pUserData;

	if (!strcmp(pPart, "elm.swallow.icon")) {
		char *pThumbIconUri = mp_util_svc_get_video_thumbnail(nVideoItemIndex);
		Evas_Object *pBg = NULL;

		if (!pThumbIconUri || !vp_file_exists(pThumbIconUri)) {
			pBg = elm_image_add(pObject);
			MP_FREE_STRING(pThumbIconUri);
			evas_object_size_hint_aspect_set(pBg, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
			elm_image_file_set(pBg, pThumbIconUri, NULL);
		} else {
			pBg = mp_util_create_preload_image(pObject, pThumbIconUri, GRID_VIDEO_ICON_WIDTH);
		}

		MP_FREE_STRING(pThumbIconUri);

		return pBg;
	} else if (!strcmp(pPart, "elm.progressbar.icon"))
	{
		int nLastPlayedTime = mp_util_svc_get_video_last_played_pos(nVideoItemIndex);
		int nDurationTime = mp_util_svc_get_video_duration_time(nVideoItemIndex);

		if (nLastPlayedTime <= 0) {
			return NULL;
		}

		double nRatio = 0.0;
		if (nDurationTime > 0) {
			nRatio = ((double)nLastPlayedTime) / ((double)nDurationTime);
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
	} else if (!strcmp(pPart, "elm.lock.icon"))
	{
		char *pVideoFile = mp_util_svc_get_video_url(nVideoItemIndex);
		bool bRet = mp_util_check_video_personal_status((const char*)pVideoFile);
		MP_FREE_STRING(pVideoFile);
		if (bRet) {
			Evas_Object *pLockIcon = elm_image_add(pObject);
			elm_image_file_set(pLockIcon, VIDEO_PLAYER_IMAGE_NAME_EDJ, VIDEO_LIST_VIEW_ICON_PERSONAL_LOCK);
			evas_object_size_hint_align_set(pLockIcon, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(pLockIcon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_show(pLockIcon);
			return pLockIcon;
		} else {
			return NULL;
		}
	} else if (!strcmp(pPart, "elm.dropbox.icon"))
	{
		bool bRet = mp_util_svc_is_cloud_storage(nVideoItemIndex);
		if (bRet) {
			Evas_Object *pDropboxIcon = elm_image_add(pObject);
			elm_image_file_set(pDropboxIcon, VIDEO_PLAYER_IMAGE_NAME_EDJ, VIDEO_LIST_VIEW_ICON_DROPBOX);
			evas_object_size_hint_align_set(pDropboxIcon, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(pDropboxIcon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_show(pDropboxIcon);
			return pDropboxIcon;
		} else {
			return NULL;
		}
	} else if (!strcmp(pPart, "elm.swallow.end"))
	{
	}
	return NULL;
}

static void __mp_select_view_append_grid_items(void *pObjVideosList, int nGenItemIndex)
{
	if (!g_pSelectHandle) {
		VideoLogInfo("invalid handle");
		return;
	}

	MP_DEL_GRID_ITC(g_pSelectHandle->pGenGridItc_1);
	MP_DEL_GRID_ITC(g_pSelectHandle->pGenGridItc_2);
	g_pSelectHandle->pGenGridItc_1 = elm_gengrid_item_class_new();
	g_pSelectHandle->pGenGridItc_1->item_style = "video/album_grid";
	g_pSelectHandle->pGenGridItc_1->func.text_get = (void*)__mp_select_view_get_label_of_grid_item_cb;
	g_pSelectHandle->pGenGridItc_1->func.content_get = (void*)__mp_select_view_get_grid_icon_cb;
	g_pSelectHandle->pGenGridItc_1->func.state_get = NULL;
	g_pSelectHandle->pGenGridItc_1->func.del = NULL;

	g_pSelectHandle->pGenGridItc_2 = elm_gengrid_item_class_new();
	g_pSelectHandle->pGenGridItc_2->item_style = "video/album_grid";
	g_pSelectHandle->pGenGridItc_2->func.text_get = (void*)__mp_select_view_get_label_of_grid_item_cb;
	g_pSelectHandle->pGenGridItc_2->func.content_get = (void*)__mp_select_view_get_grid_icon_cb;
	g_pSelectHandle->pGenGridItc_2->func.state_get = NULL;
	g_pSelectHandle->pGenGridItc_2->func.del = NULL;

	int nCount = (mp_rotate_ctrl_check_landspace() ? VP_GENLIST_THUMB_LANDSPACE_COUNT : VP_GENLIST_THUMB_PORTRAIT_COUNT);
	int nIndex = nGenItemIndex*nCount;
	int nMaxIndex = nIndex + nCount;
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();

	Elm_Object_Item *gridItem = NULL;
	for (; (nIndex < nMaxIndex) && (nIndex < nVideoListSize); nIndex++) {
		if (nIndex%2 == 0) {
			elm_gengrid_item_append(pObjVideosList, g_pSelectHandle->pGenGridItc_1, (void *)nIndex, __mp_select_view_genlist_selected_cb, (void *)nIndex);
		} else {
			elm_gengrid_item_append(pObjVideosList, g_pSelectHandle->pGenGridItc_2, (void *)nIndex, __mp_select_view_genlist_selected_cb, (void *)nIndex);
		}
		elm_object_item_data_set(gridItem, (void *)nIndex);
	}
}

static void mp_select_view_get_icon_dimensions(int *height, int *width)
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

static Evas_Object *__mp_select_view_get_icon_of_grid_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{
	VideoLogInfo("in __mp_select_view_get_icon_of_grid_cb");
	if (!g_pSelectHandle || !pObject) {
		VideoLogError("g_pMainViewWidgetOfSelectList is NULL");
		return NULL;
	}

	int nGenItemIndex = (int)pUserData;
	int nHeight = 0;
	int nWidth = 0;
	mp_select_view_get_icon_dimensions(&nHeight, &nWidth);

	Evas_Object *pGengrid = __mp_select_view_create_gengrid(pObject, nWidth, nHeight);
	__mp_select_view_append_grid_items(pGengrid, nGenItemIndex);

	return pGengrid;
}

int mp_select_view_thumbnail_to_genlist_index(int nVideoItemIndex)
{
	/*nVideoItemIndex = 0-->max*/
	if (nVideoItemIndex < 0) {
		VideoLogDebug("invalid nVideoItemIndex");
		return 0;
	}
	g_pSelectHandle->bLandscape = mp_rotate_ctrl_check_landspace_by_win((Evas_Object*)mp_util_get_main_window_handle());
	int nItemCount = VP_GENLIST_THUMB_PORTRAIT_COUNT;
	int nGenlistIndex = 0;
	if (g_pSelectHandle->bLandscape) {
		nItemCount = VP_GENLIST_THUMB_LANDSPACE_COUNT;
	}
	nGenlistIndex = nVideoItemIndex/nItemCount;
	VideoLogWarning("in mp_select_view_thumbnail_to_genlist_index %d",nGenlistIndex);
	return nGenlistIndex;
}

static bool __mp_select_view_append_grid_layout(void *pGenlist)
{
	VideoLogInfo("in append grid layout");
	if (!pGenlist || !g_pSelectHandle) {
		VideoLogError("error handle.");
		return FALSE;
	}
	int nIndex = 0;
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	if (nVideoListSize <= 0) {
		VideoLogWarning("nVideoListSize = %d", nVideoListSize);
		return FALSE;
	} else
		VideoLogWarning("nVideoListSize = %d", nVideoListSize);

	MP_DEL_ITC(g_pSelectHandle->pGenListItc);
	g_pSelectHandle->pGenListItc = elm_genlist_item_class_new();

	g_pSelectHandle->pGenListItc->item_style = "video/1icon.thumbnail";
	g_pSelectHandle->pGenListItc->func.text_get = NULL;
	g_pSelectHandle->pGenListItc->func.content_get = (void *)__mp_select_view_get_icon_of_grid_cb;
	g_pSelectHandle->pGenListItc->func.state_get = NULL;
	g_pSelectHandle->pGenListItc->func.del = NULL;
	g_pSelectHandle->pGenListItc->decorate_item_style = NULL;

	MP_DEL_ITC(g_pSelectHandle->pGenListLandscapeItc);
	g_pSelectHandle->pGenListLandscapeItc = elm_genlist_item_class_new();
	g_pSelectHandle->pGenListLandscapeItc->item_style = "video/1icon.thumbnail.landscape";
	g_pSelectHandle->pGenListLandscapeItc->func.text_get = NULL;
	g_pSelectHandle->pGenListLandscapeItc->func.content_get = (void *)__mp_select_view_get_icon_of_grid_cb;
	g_pSelectHandle->pGenListLandscapeItc->func.state_get = NULL;
	g_pSelectHandle->pGenListLandscapeItc->func.del = NULL;
	g_pSelectHandle->pGenListLandscapeItc->decorate_item_style = NULL;

	int nGenlistRow = 0;
	nGenlistRow = mp_select_view_thumbnail_to_genlist_index(nVideoListSize - 1);

	if (g_pSelectHandle->bLandscape) {
		for (nIndex = 0; nIndex <= nGenlistRow; nIndex++) {
			 elm_genlist_item_append(pGenlist, g_pSelectHandle->pGenListLandscapeItc, (void *)nIndex, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		}
	} else {
		for (nIndex = 0; nIndex <= nGenlistRow; nIndex++) {
			 elm_genlist_item_append(pGenlist, g_pSelectHandle->pGenListItc, (void *)nIndex, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		}
	}

	return TRUE;
}
#endif

#ifdef VS_FEATURE_LIST_VIEW
static void __mp_select_view_append_video_items(void *pVideosGenlist)
{
	VideoLogInfo("");
	if (!g_pSelectHandle)
	{
		VideoLogError("");
		return;
	}

	int nIndex = 0;
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();

	MP_DEL_ITC(g_pSelectHandle->pGenListItc);
	g_pSelectHandle->pGenListItc = elm_genlist_item_class_new();
	g_pSelectHandle->pGenListItc->item_style = "2line.top";
	g_pSelectHandle->pGenListItc->func.text_get = (void*)__mp_select_view_get_label_of_video_item_cb;
	g_pSelectHandle->pGenListItc->func.content_get = (void*)__mp_select_view_get_icon_of_video_item_cb;
	g_pSelectHandle->pGenListItc->func.state_get = NULL;
	g_pSelectHandle->pGenListItc->func.del = NULL;
	g_pSelectHandle->pGenListItc->decorate_item_style = NULL;

	for (nIndex = 0; nIndex < nVideoListSize; nIndex++)
	{
		elm_genlist_item_append(pVideosGenlist, g_pSelectHandle->pGenListItc, (void *)nIndex, NULL, ELM_GENLIST_ITEM_NONE, __mp_select_view_genlist_selected_cb, (void *)nIndex);
	}
}
#endif

bool mp_select_view_arrange_video_list(void *pGenlist, bool bNeedSvc)
{
	if (!g_pSelectWidget || !g_pSelectHandle) {
		VideoLogError("No exist g_pSelectListViewWidget.");
		return FALSE;
	}

	if (!pGenlist) {
		VideoLogError("No exist genlist object.");
		return FALSE;
	}

	int nVideoListSize = 0;
#ifdef VS_FEATURE_LIST_VIEW
	if (g_pSelectHandle->nViewType == MP_LIST_VIEW_AS_NORMAL_LIST) {
		/*update video list data*/
		mp_util_svc_destory_video_list();
		mp_util_svc_extract_video_list_by_item_type(mp_sort_ctrl_get_sort_state(), mp_view_ctrl_get_list_type());

		nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
		if (nVideoListSize <= 0)
		{
			VideoLogWarning("nVideoListSize=%d", nVideoListSize);
			//elm_naviframe_item_pop(g_pSelectHandle->pNaviFrameHandle);
			return FALSE;
		}

		elm_genlist_clear(pGenlist);
		__mp_select_view_append_video_items(pGenlist);
	} else
#endif
	  if (g_pSelectHandle->nViewType == MP_LIST_VIEW_AS_FOLDER_LIST) {
		if (g_pSelectHandle->pFolderUrl) {
			VideoLogWarning("Folder item view");
			/*update folder item list data*/
			mp_util_svc_destory_video_item_list();
			mp_util_svc_extract_video_list_from_folder(g_pSelectHandle->pFolderUrl, mp_sort_ctrl_get_sort_state(), MP_LIST_VIEW_ALL);

			nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
			if (nVideoListSize <= 0) {
				VideoLogWarning("nVideoListSize=%d", nVideoListSize);
				//elm_naviframe_item_pop(g_pSelectHandle->pNaviFrameHandle);
				return FALSE;
			}

			elm_genlist_clear(pGenlist);
			//__mp_select_view_append_video_items(pGenlist);
			__mp_select_view_append_grid_layout(pGenlist);
		} else {
			VideoLogWarning("Folder view");
			/*update folder list data*/
			mp_util_svc_destory_video_list();
			mp_util_svc_extract_video_list_by_item_type(mp_sort_ctrl_get_sort_state(), mp_view_ctrl_get_list_type());
			mp_util_svc_extract_video_folder_list_by_type(mp_sort_ctrl_get_sort_state(), mp_view_ctrl_get_list_type());

			nVideoListSize = mp_util_svc_get_video_folder_size();
			if (nVideoListSize <= 0) {
				VideoLogWarning("nVideoListSize=%d", nVideoListSize);
				//elm_naviframe_item_pop(g_pSelectHandle->pNaviFrameHandle);
				return FALSE;
			}

			elm_genlist_clear(pGenlist);
			__mp_select_view_append_folder_items(pGenlist);
		}
	} else if (g_pSelectHandle->nViewType == MP_LIST_VIEW_AS_THUMBNAIL_LIST) {
		/*update video list data*/
		mp_util_svc_destory_video_list();
		mp_util_svc_extract_video_list_by_item_type(mp_sort_ctrl_get_sort_state(), mp_view_ctrl_get_list_type());

		nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
		if (nVideoListSize <= 0) {
			VideoLogWarning("nVideoListSize=%d", nVideoListSize);
			//elm_naviframe_item_pop(g_pSelectHandle->pNaviFrameHandle);
			return FALSE;
		}

		elm_genlist_clear(pGenlist);
		__mp_select_view_append_grid_layout(pGenlist);
	}

	evas_object_show(pGenlist);
	return TRUE;
}

Evas_Object *mp_select_view_create_internal_layout(void *pParent)
{
	if (!pParent)
	{
		VideoLogInfo("[ERR]");
		return NULL;
	}

	VideoLogInfo("");

	if (g_pSelectWidget->pBaselayout)
	{
		evas_object_del(g_pSelectWidget->pBaselayout);
		g_pSelectWidget->pBaselayout = NULL;
	}

	g_pSelectWidget->pBaselayout = elm_layout_add(pParent);
	elm_layout_file_set(g_pSelectWidget->pBaselayout, VIDEO_PLAYER_REMOVE_LIST_EDJ, REMOVE_LIST_EDJ_GROUP);
	evas_object_size_hint_weight_set(g_pSelectWidget->pBaselayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pSelectWidget->pBaselayout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	return g_pSelectWidget->pBaselayout;
}

static void __mp_select_view_db_changed_cb(void *pUserData)
{
	VideoLogWarning("Video DB has been changed.");
	if (!g_pSelectHandle || !g_pSelectWidget)
	{
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	bool bNormalShow = TRUE;

	bNormalShow = mp_select_view_arrange_video_list(g_pSelectHandle->pGenlist, TRUE);
	if (bNormalShow)
	{
		Elm_Object_Item *pItem = NULL;
		pItem = elm_genlist_first_item_get(g_pSelectHandle->pGenlist);
		if (pItem)
		{
			elm_genlist_item_bring_in(pItem, ELM_GENLIST_ITEM_SCROLLTO_IN);
		}
		if (SELECT_TYPE_DETAIL == g_pSelectHandle->eSelType) {
			mp_detail_view_update();
		} else if (SELECT_TYPE_RENAME == g_pSelectHandle->eSelType) {
			//if needed, todo;
		}
	}
	mp_rename_ctrl_focus_out_cb();
	g_pSelectHandle->pChangeViewUserCbFunc(MP_LIST_VIEW_ALL);
	elm_naviframe_item_pop_to(g_pSelectHandle->pPreNaviItem);
}

void __mp_select_soft_back_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *pTopNaviFrame = NULL;
	pTopNaviFrame = elm_naviframe_item_pop(g_pSelectHandle->pNaviFrameHandle);
	evas_object_del(pTopNaviFrame);
}

void mp_select_view_create_genlist()
{
	if (!g_pSelectHandle || !g_pSelectWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	VideoLogInfo("");

	g_pSelectHandle->pGenlist = elm_genlist_add(g_pSelectWidget->pBox);
	if (!g_pSelectHandle->pGenlist) {
		VideoLogError("pGenlist is not existed.");
		return;
	}
	elm_scroller_bounce_set(g_pSelectHandle->pGenlist, EINA_FALSE, EINA_TRUE);
	evas_object_size_hint_weight_set(g_pSelectHandle->pGenlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pSelectHandle->pGenlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_genlist_block_count_set(g_pSelectHandle->pGenlist, VIDEO_GENLIST_BLOCK_COUNT);
	elm_genlist_homogeneous_set(g_pSelectHandle->pGenlist, EINA_TRUE);
	elm_genlist_mode_set(g_pSelectHandle->pGenlist, ELM_LIST_COMPRESS);

	elm_box_pack_end(g_pSelectWidget->pBox, g_pSelectHandle->pGenlist);
	elm_object_part_content_set(g_pSelectWidget->pBaselayout, SWALLOW_LISTVIEW_CONTENT, g_pSelectWidget->pBox);

	bool bNormalShow = TRUE;
	bNormalShow = mp_select_view_arrange_video_list(g_pSelectHandle->pGenlist, FALSE);
	if (!bNormalShow) {
		VideoLogError("invalid view");
		mp_select_view_destroy();
		return;
	}
	evas_object_show(g_pSelectHandle->pGenlist);
}

void mp_select_view_init(void *pParent)
{
	if (!g_pSelectHandle || !g_pSelectWidget)
	{
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	VideoLogInfo("");
	g_pSelectWidget->pBaselayout = mp_select_view_create_internal_layout(pParent);
	if (!g_pSelectWidget->pBaselayout) {
		VideoLogError("pBaselayout is not existed.");
		return;
	}
	evas_object_event_callback_add(g_pSelectWidget->pBaselayout, EVAS_CALLBACK_DEL, __mp_select_view_base_layout_del_cb, NULL);

	g_pSelectWidget->pBox = elm_box_add(g_pSelectWidget->pBaselayout);
	if (!g_pSelectWidget->pBox) {
		VideoLogError("pBox is not existed.");
		return;
	}

	mp_select_view_create_genlist();

	Evas_Object *pLeftbtn = elm_button_add(pParent);
	elm_object_style_set(pLeftbtn, "naviframe/end_btn/default");
	evas_object_smart_callback_add(pLeftbtn, "clicked", __mp_select_soft_back_button_cb, NULL);

	char *szTitle = NULL;
	szTitle = g_strdup(VIDEOS_SELECTVIEW_HEADER_SELECT_VIDEO);

	g_pSelectHandle->pNaviFrameItem = elm_naviframe_item_push(g_pSelectHandle->pNaviFrameHandle,
									szTitle,
									pLeftbtn,
									NULL,
									g_pSelectWidget->pBaselayout,
									NULL);

	elm_object_item_domain_text_translatable_set(g_pSelectHandle->pNaviFrameItem, VIDEOS_STRING, EINA_TRUE);
	elm_naviframe_item_pop_cb_set(g_pSelectHandle->pNaviFrameItem, mp_select_view_back_btn_cb, g_pSelectHandle);
	g_free(szTitle);

	elm_naviframe_item_title_enabled_set(g_pSelectHandle->pNaviFrameItem, EINA_TRUE, EINA_FALSE);
	evas_object_smart_callback_add(g_pSelectHandle->pNaviFrameHandle, "transition,finished", mp_select_view_naviframe_transition_effect_cb, NULL);

	evas_object_show(g_pSelectWidget->pBaselayout);

	mp_util_db_set_update_fun(g_pSelectHandle->euLev, __mp_select_view_db_changed_cb);

	mp_util_hide_indicator();
}

static void __mp_select_view_rotate_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!g_pSelectHandle) {
		VideoLogError("__mp_select_view_rotate_cb IS null");
		return;
	}
	bool bLandscape =  mp_rotate_ctrl_check_landspace_by_win((Evas_Object*)mp_util_get_main_window_handle());
	if (bLandscape) {
		g_pSelectHandle->bLandscape = true;
	} else {
		g_pSelectHandle->bLandscape = false;
	}

	mp_select_view_arrange_video_list(g_pSelectHandle->pGenlist, TRUE);
}

void mp_select_view_push(void *pNaviFrame, void *pNaviItem, ChangeSelectViewCbFunc pChangeViewCb, char *pFolderUrl, SelectType eSelType, MpVideoListTabType eTabType)
{
	VideoLogInfo("");
	if (g_pSelectHandle || g_pSelectWidget) {
		mp_select_view_destroy();
	}

	g_pSelectHandle = (st_SelectViewHandle*)calloc(1, sizeof(st_SelectViewHandle));
	g_pSelectWidget = (st_SelectViewWidget*)calloc(1, sizeof(st_SelectViewWidget));

	memset(g_pSelectHandle, 0, sizeof(st_SelectViewHandle));
	memset(g_pSelectWidget, 0, sizeof(st_SelectViewWidget));
	mp_select_view_reset_value();

	g_pSelectHandle->pNaviFrameHandle = pNaviFrame;
	g_pSelectHandle->pPreNaviItem = pNaviItem;
	g_pSelectHandle->pChangeViewUserCbFunc = pChangeViewCb;
	g_pSelectHandle->pFolderUrl = g_strdup(pFolderUrl);
	g_pSelectHandle->eSelType = eSelType;
	g_pSelectHandle->eTabType = eTabType;
	g_pSelectHandle->euLev = pFolderUrl ? MP_DB_UPDATE_LEV_2 : MP_DB_UPDATE_LEV_1;
	g_pSelectHandle->nViewType = mp_view_as_ctrl_get_type();
	g_pSelectHandle->bLandscape = mp_rotate_ctrl_check_landspace_by_win((Evas_Object*)mp_util_get_main_window_handle());

	mp_select_view_init(g_pSelectHandle->pNaviFrameHandle);
	evas_object_smart_callback_add((Evas_Object*)mp_util_get_main_window_handle(), "wm,rotation,changed", __mp_select_view_rotate_cb, (void*)g_pSelectHandle);

}

void mp_select_view_pop(void)
{
	VideoLogInfo("");

	if (!g_pSelectHandle)
	{
		VideoLogError("No existed handle of select view.");
		return;
	}
	if (g_pSelectHandle->bRename && g_pSelectHandle->pChangeViewUserCbFunc)
	{
		g_pSelectHandle->pChangeViewUserCbFunc(MP_LIST_VIEW_ALL);
	}

	mp_util_hide_indicator();
}

void mp_select_view_change_language(void)
{
	if (!g_pSelectHandle || !g_pSelectWidget)
	{
		VideoLogError("No existed handle of select view.");
		return;
	}

	VideoLogInfo("");
	char *szTitle = NULL;
	if (SELECT_TYPE_DETAIL == g_pSelectHandle->eSelType) {
		szTitle = g_strdup(VIDEOS_DETAILVIEW_HEADER_DETAILS_IDS);
	} else if (SELECT_TYPE_RENAME == g_pSelectHandle->eSelType) {
		szTitle = g_strdup(VIDEOS_RENAMEVIEW_HEADER_RENAME_IDS);
	}
	elm_object_item_domain_translatable_part_text_set(g_pSelectHandle->pNaviFrameItem, "elm.text.title", VIDEOS_STRING, szTitle);
	g_free(szTitle);
}

static void __mp_select_view_base_layout_del_cb(void *pUserData, Evas *e ,Evas_Object *pObject, void *pEventInfo)
{
	__mp_select_view_reset();
}


