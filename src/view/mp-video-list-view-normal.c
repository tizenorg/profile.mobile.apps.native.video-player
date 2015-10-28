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
#include <unistd.h>
#include <stdbool.h>
#include <Ecore_Evas.h>
#include <Elementary.h>

#include "vp-util.h"
#include "vp-file-util.h"
#include "mp-util.h"
#ifdef ENABLE_DRM_FEATURE
#include "mp-drm-ctrl.h"
#endif
#include "mp-video-log.h"
#include "mp-util-media-service.h"
#include "mp-video-value-define.h"
#include "mp-video-string-define.h"

#include "mp-footer-toolbar.h"
#include "mp-video-list-sort-ctrl.h"
#include "mp-video-view-popup-ctrl.h"
#include "mp-video-list-view-common.h"
#include "mp-video-list-view-normal.h"
#include "mp-video-list-view-as-ctrl.h"
#include "mp-launching-video-displayer.h"
#include "mp-video-list-option-ctrl.h"
#include "mp-video-rename-ctrl.h"
#include "mp-external-ug.h"
#include "videos-view-mgr.h"
#include "mp-video-rename-ctrl.h"
#include "mp-video-info-ctrl.h"
#include "mp-video-list-view-main.h"
#include "mp-video-nocontent-layout.h"
#include "mp-video-list-personal-ctrl.h"
#include "mp-video-detail-view.h"
#include "mp-util-preference.h"
#include "mp-util-config.h"
#include "mp-util-widget-ctrl.h"

#ifdef VS_FEATURE_LIST_VIEW
typedef struct {
	st_VideoListViewMainViewHandle *pMainHandle;
	st_VideoListViewMainViewWidget *pMainWidget;
	UpdateListItemsCbFunc fUpdateListItemUserCb;

	Evas_Object *pVideosGenlist;
	Evas_Object *pNocontentlayout;
	Elm_Genlist_Item_Class *pVideoList_Itc;

	Eina_List *pVideoItemList;

	int nSelectedVideoItem;
	bool bRunLongPress;

} stNormalView;


static stNormalView *g_pNormalView = NULL;

void __mp_normal_view_create_no_content()
{
	if (!g_pNormalView) {
		VideoLogError("");
		return;
	}

	if (!g_pNormalView->pNocontentlayout) {
		g_pNormalView->pNocontentlayout =
			mp_create_nocontent_layout(g_pNormalView->pMainWidget->
						   pListViewBaselayout,
						   VIDEOS_NOCONTENT_MSG_NO_VIDEOS_IDS,
						   VIDEOS_NOCONTENT_MSG_NO_VIDEOS_DESC_IDS);
		elm_object_part_content_unset(g_pNormalView->pMainWidget->
					      pListViewBaselayout,
					      SWALLOW_LISTVIEW_CONTENT);
		if (g_pNormalView->pVideosGenlist) {
			evas_object_hide(g_pNormalView->pVideosGenlist);
		}
		evas_object_show(g_pNormalView->pMainWidget->pListViewBaselayout);
		elm_object_part_content_set(g_pNormalView->pMainWidget->
					    pListViewBaselayout,
					    SWALLOW_LISTVIEW_CONTENT,
					    g_pNormalView->pNocontentlayout);
	}

}

static bool __mp_normal_view_clear_genlist_item(void)
{
	if (!g_pNormalView) {
		VideoLogError("");
		return FALSE;
	}
	if (g_pNormalView->pVideoItemList) {
		Elm_Object_Item *pTmpVideoGenlistItem = NULL;
		EINA_LIST_FREE(g_pNormalView->pVideoItemList,
			       pTmpVideoGenlistItem) {
			if (pTmpVideoGenlistItem) {
				int nIndex =
					(int) elm_object_item_data_get(pTmpVideoGenlistItem);
				mp_util_svc_cancel_thumbnail_update(nIndex);
				elm_object_item_del(pTmpVideoGenlistItem);
			}

			pTmpVideoGenlistItem = NULL;
		}

		g_pNormalView->pVideoItemList = NULL;
	}

	return TRUE;
}

void mp_normal_view_destroy(void)
{
	VideoLogInfo("");

	if (g_pNormalView) {
		__mp_normal_view_clear_genlist_item();

		MP_DEL_ITC(g_pNormalView->pVideoList_Itc);
		g_pNormalView->bRunLongPress = FALSE;
		g_pNormalView->fUpdateListItemUserCb = NULL;
		g_pNormalView->pMainHandle = NULL;
		g_pNormalView->pMainWidget = NULL;
		g_pNormalView->nSelectedVideoItem = -1;
		MP_DEL_OBJ(g_pNormalView->pNocontentlayout);
		MP_DEL_OBJ(g_pNormalView->pVideosGenlist);
	}
	MP_FREE_STRING(g_pNormalView);

}

static void __mp_normal_view_reset(MpListUpdateType eUpdateType)
{
	if (g_pNormalView) {
		__mp_normal_view_clear_genlist_item();

		MP_DEL_ITC(g_pNormalView->pVideoList_Itc);
		g_pNormalView->bRunLongPress = FALSE;
		g_pNormalView->fUpdateListItemUserCb = NULL;
		g_pNormalView->pMainHandle = NULL;
		g_pNormalView->pMainWidget = NULL;
		if (g_pNormalView->pVideosGenlist) {
			elm_genlist_clear(g_pNormalView->pVideosGenlist);
		}

	}

}

void mp_normal_view_update_video_items(int nVideoItemIndex)
{
	if (!g_pNormalView) {
		VideoLogError("");
		return;
	}

	if (!g_pNormalView->pVideoItemList) {
		VideoLogWarning
		("[WARNING] g_pNormalView->pVideoItemList is not existed.");
		return;
	}

	Elm_Object_Item *pTmpVideoGenlistItem =
		(Elm_Object_Item *) eina_list_nth(g_pNormalView->pVideoItemList,
				nVideoItemIndex);
	if (pTmpVideoGenlistItem) {
		/*elm_genlist_item_update(pTmpVideoGenlistItem);*/
		/*elm_genlist_item_fields_update(pTmpVideoGenlistItem, "elm.icon.1", ELM_GENLIST_ITEM_FIELD_CONTENT);*/
		elm_genlist_item_selected_set(pTmpVideoGenlistItem, EINA_FALSE);
		int nLastPlayedTime =
			mp_util_svc_get_video_last_played_pos(nVideoItemIndex);
		int nDurationTime =
			mp_util_svc_get_video_duration_time(nVideoItemIndex);
		Evas_Object *pIconContent =
			elm_object_item_part_content_get(pTmpVideoGenlistItem,
					"elm.icon.1");
		if (!pIconContent) {
			VideoLogError("");
			return;
		}
		Evas_Object *pProgressbar =
			elm_layout_content_get(pIconContent, "elm.progressbar.icon");
		if (nLastPlayedTime > 0) {
			double nRatio =
				((double) nLastPlayedTime) / ((double) nDurationTime);
			if (nRatio < 0.05) {
				nRatio = 0.05;
			}

			if (!pProgressbar) {
				elm_genlist_item_fields_update(pTmpVideoGenlistItem,
							       "elm.icon.1",
							       ELM_GENLIST_ITEM_FIELD_CONTENT);
			} else {
				elm_progressbar_value_set(pProgressbar, nRatio);
			}

		} else {
			if (pProgressbar) {
				elm_layout_content_unset(pIconContent,
							 "elm.progressbar.icon");
				MP_DEL_OBJ(pProgressbar);
			}
		}

		elm_genlist_item_fields_update(pTmpVideoGenlistItem,
					       "elm.text.sub.left.bottom",
					       ELM_GENLIST_ITEM_FIELD_TEXT);
	}
}

bool mp_normal_view_is_top_view(void)
{
	VideoLogInfo("");


	if (!g_pNormalView || !g_pNormalView->pMainHandle) {
		VideoLogError("g_pNormalView->pMainHandle is NULL");
		return FALSE;
	}
	if (!g_pNormalView->pMainHandle->pNaviFrameHandle) {
		VideoLogInfo("[ERR] No exist naviframe handle.");
		return FALSE;
	}
	if (mp_mgr_get_library_naviframe()) {
		int count =
			eina_list_count(elm_naviframe_items_get
					(mp_mgr_get_library_naviframe()));
		return (count == 1);
	}
	return FALSE;
}

static void __mp_normal_view_warning_popup_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	VideoLogInfo("");

	mp_util_delete_popup_handle();
}

bool mp_normal_view_is_normal_list(void)
{
	if (!g_pNormalView || !g_pNormalView->pMainHandle) {
		VideoLogError("");
		return FALSE;
	}

	if (mp_view_as_ctrl_get_type() != MP_LIST_VIEW_AS_NORMAL_LIST) {
		VideoLogError("Invalid to call realize function.");
		return FALSE;
	}

	return TRUE;
}

/**/
/*////////////////////////////////////////////////////////////*/


/*////////////////////////////////////////////////////////////*/
/**/
void mp_normal_view_thumbnail_update_cb(int nError, const char *szPath,
					void *pUserData)
{
	if (!g_pNormalView) {
		VideoLogError("");
		return;
	}

	if (mp_normal_view_is_normal_list() == FALSE) {
		VideoLogError("Current list is not for normal video item.");
		return;
	}

	int nVideoItemIndex = (int) pUserData;

	VideoLogInfo("Index : %d", nVideoItemIndex);

	if (!g_pNormalView->pMainHandle) {
		VideoLogError("g_pNormalView->pMainHandle is not existed.");
		return;
	}

	if (!g_pNormalView->pMainWidget) {
		VideoLogError("g_pNormalView->pMainWidget is not existed.");
		return;
	}

	if (!mp_normal_view_is_top_view()) {
		VideoLogError("Main list view is not top view.");
		return;
	}

	mp_util_svc_update_thumbnail_info(nVideoItemIndex);

	if (g_pNormalView->pVideoItemList) {
		Elm_Object_Item *pTmpVideoGenlistItem =
			(Elm_Object_Item *) eina_list_nth(g_pNormalView->
					pVideoItemList,
					nVideoItemIndex);
		if (pTmpVideoGenlistItem) {
			elm_genlist_item_fields_update(pTmpVideoGenlistItem,
						       "elm.icon.1",
						       ELM_GENLIST_ITEM_FIELD_CONTENT);
		} else {
			VideoLogError
			("[ERR] Fail to update genlist filed thumbnail icon.");
		}
	} else {
		VideoLogError("[ERR] No exist g_pNormalView->pVideoItemList.");
	}

	/*      elm_genlist_realized_items_update(g_pNormalView->pVideosGenlist);*/
}

/**/
/*////////////////////////////////////////////////////////////*/

/*////////////////////////////////////////////////////////////*/
/* Smart callback of Genlist about normal.*/
static void __mp_normal_view_genlist_language_changed_cb(void *pUserData,
		Evas_Object *
		pObject,
		void *pEventInfo)
{
	if (mp_normal_view_is_normal_list() == FALSE) {
		VideoLogError("Current list is not for normal video item.");
		return;
	}

	VideoLogInfo("");

	elm_genlist_realized_items_update(pObject);
}

static void __mp_normal_view_realize_genlist_item_cb(void *pUserData,
		Evas_Object *
		pObject,
		void *pEventInfo)
{
	if (!g_pNormalView) {
		VideoLogError("");
		return;
	}

	if (!pEventInfo) {
		VideoLogError("Invalid pEveninfo.");
		return;
	}

	int nIndexGenlist = elm_genlist_item_index_get(pEventInfo);

	if (nIndexGenlist < 0) {
		VideoLogError("nIndexGenlist < 0 = RETURN");
		return;
	}

	if ((nIndexGenlist < 0)
			|| (nIndexGenlist >=
			    mp_util_svc_get_number_of_video_item_by_type())) {
		VideoLogError("nIndexGenlist < 0 = RETURN = %d", nIndexGenlist);
		return;
	}

	char *pThumbIconUri = mp_util_svc_get_video_thumbnail(nIndexGenlist);
	if (!pThumbIconUri) {
		VideoLogInfo("nIndexGenlist : %d", nIndexGenlist);
		mp_util_svc_register_thumbnail_update_func(nIndexGenlist,
				(void *)
				mp_normal_view_thumbnail_update_cb,
				(void *)
				nIndexGenlist);
	} else {
		free(pThumbIconUri);
		pThumbIconUri = NULL;
	}
}

/*////////////////////////////////////////////////////////////*/
/* Content/Text Callback of Elm_Genlist_Item_Class.*/
char *mp_normal_view_get_label_of_video_item_cb(const void *pUserData,
		Evas_Object *pObject,
		const char *pPart)
{
	int nVideoItemIndex = (int) pUserData;

	if (!strcmp(pPart, "elm.text.main.left.top")) {
		char *szTitle = mp_util_svc_get_video_title(nVideoItemIndex);
		char *szTitleUtf8 = elm_entry_utf8_to_markup(szTitle);
		if (szTitle) {
			free(szTitle);
			szTitle = NULL;
		}

		return szTitleUtf8;
	} else if (!strcmp(pPart, "elm.text.sub.left.bottom")) {
		return mp_util_svc_get_duration_str_time(nVideoItemIndex);
	}
	return NULL;
}

Evas_Object *mp_normal_view_get_icon_of_video_item_cb(const void
		*pUserData,
		Evas_Object *
		pObject,
		const char *pPart)
{
	Evas_Object *pLayout = NULL;

	int nVideoItemIndex = (int) pUserData;
	if (!pPart || !pObject) {
		return NULL;
	}

	if (!strcmp(pPart, "elm.icon.1")) {
		VideoLogInfo("pPart: elm.icon.1 - thumbnail");

		char *pThumbIconUri =
			mp_util_svc_get_video_thumbnail(nVideoItemIndex);
		Evas_Object *pBg = NULL;

		pLayout = elm_layout_add(pObject);
		elm_layout_file_set(pLayout, VIDEO_CUSTOM_THEME,
				    "listview.thumbnail.layout");

		if (!pThumbIconUri || !vp_file_exists(pThumbIconUri)) {
			MP_FREE_STRING(pThumbIconUri);
		}

		pBg =
			mp_util_create_preload_image(pLayout, pThumbIconUri,
						     VIDEO_ICON_WIDTH);
		elm_layout_content_set(pLayout, "elm.thumbnail.icon", pBg);

		if (pBg)
			evas_object_show(pBg);

		MP_FREE_STRING(pThumbIconUri);

		int nLastPlayedTime =
			mp_util_svc_get_video_last_played_pos(nVideoItemIndex);
		int nDurationTime =
			mp_util_svc_get_video_duration_time(nVideoItemIndex);

		if (nLastPlayedTime > 0) {
			double nRatio =
				((double) nLastPlayedTime) / ((double) nDurationTime);
			if (nRatio < 0.05) {
				nRatio = 0.05;
			}

			Evas_Object *pProgressBar =
				mp_widget_ctrl_create_progressbar(pLayout, nRatio);
			elm_layout_content_set(pLayout, "elm.progressbar.icon",
					       pProgressBar);

			if (pProgressBar) {
				evas_object_show(pProgressBar);
			}
		}

		if (!mp_util_create_video_sub_icon(pLayout, nVideoItemIndex)) {
			VideoLogWarning("Local File or Create sub icon is failed.",
					nVideoItemIndex);
		}

		if (pLayout) {
			evas_object_show(pLayout);
		}

		return pLayout;
	}

	MP_DEL_OBJ(pLayout);

	return NULL;
}

Evas_Object *mp_normal_view_get_icon_of_no_item_cb(const void *pUserData,
		Evas_Object *pObject,
		const char *pPart)
{

	if (!strcmp(pPart, "elm.icon")) {
		int width = NO_ITEM_GENLIST_WIDTH * elm_config_scale_get();
		int height = NO_ITEM_GENLIST_HEIGHT * elm_config_scale_get();

		Evas_Object *pNoItemLayout = NULL;
		pNoItemLayout = elm_layout_add(pObject);
		elm_layout_file_set(pNoItemLayout, VIDEO_PLAYER_NO_ITEM_EDJ,
				    GENLIST_NOITEM_EDJ_GROUP);
		evas_object_size_hint_min_set(pNoItemLayout, width, height);

		elm_object_part_text_set(pNoItemLayout,
					 SWALLOW_LISTVIEW_NO_ITEM_TEXT,
					 VIDEOS_NOCONTENT_MSG_NO_VIDEOS_DESC);
		evas_object_show(pNoItemLayout);
		return pNoItemLayout;
	}

	return NULL;
}


/*////////////////////////////////////////////////////////////*/
/**/
static void mp_normal_view_select_video_item_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	if (!pObject) {
		VideoLogError("[ERR]No have pUserData");
		return;
	}

	Elm_Object_Item *pSelectedItem =
		elm_genlist_selected_item_get(pObject);
	if (!pSelectedItem) {
		VideoLogInfo("pSelectedItem is NULL");
		return;
	} else {
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}

	if (g_pNormalView->bRunLongPress) {
		VideoLogInfo("run long pressed event");
		g_pNormalView->bRunLongPress = false;

		return;
	}

	if (!mp_launch_video_allow_check()) {
		return;
	}
	Evas_Object *pPopup = evas_object_data_get(pObject, "popup");	/* Get popup*/
	if (pPopup) {
		VideoLogError("long press is called");
		return;			/* If popup exists, do nothing*/
	}

	int nTotal = elm_genlist_items_count(pObject);

	/*int nVideoItemIndex = elm_genlist_item_index_get(pSelectedItem);*/
	int nVideoItemIndex = (int) elm_object_item_data_get(pSelectedItem);

	if (nVideoItemIndex < 0) {
		VideoLogError("nGenlistItemIndex is invalid");
		return;
	}

	VideoLogInfo("nVideoIndex:ngenIndex : %d:%d", nVideoItemIndex,
		     nTotal);


	if (nVideoItemIndex < 0) {
		VideoLogWarning("nVideoItemIndex < 0 ===>>> RETURN");
		return;
	}

	if (!mp_util_call_off()) {
		mp_util_ticker_toast_popup
		(VIDEOS_NOTIPOP_MSG_UNABLE_TO_PLAY_VIDEO_DURING_CALL, false,
		 false);
		return;
	}

	g_pNormalView->nSelectedVideoItem = nVideoItemIndex;

	char *szMediaUrl = mp_util_svc_get_video_url(nVideoItemIndex);

	if (szMediaUrl) {
		if (!mp_util_check_local_file_exist(szMediaUrl)) {
			mp_util_notify_popup(NULL, VIDEOS_COMMON_HEADER_ERROR,
					     VIDEOS_NOTIPOP_MSG_FILE_NOT_EXIST,
					     __mp_normal_view_warning_popup_cb);
			free(szMediaUrl);
			return;
		}
		mp_launch_video_play(szMediaUrl, MP_PLAYER_TYPE_VIDEO, NULL);
	} else {
		VideoLogInfo("[Err] Fail to get video uri from media service.");
		return;
	}

	if (szMediaUrl) {
		free(szMediaUrl);
	}
}

/**/
/*////////////////////////////////////////////////////////////*/

/*////////////////////////////////////////////////////////////*/
/* Append item into genlist.*/

static void __mp_normal_view_video_item_del_cb(void *data,
		Evas_Object *obj,
		void *event_info)
{
	if (!g_pNormalView) {
		VideoLogError("");
		return;
	}

	if (!g_pNormalView->pVideoItemList || !event_info) {
		VideoLogError("g_pMainViewWidge is NULL");
		return;
	}

	int nIndexGenlist = 0;
	Elm_Object_Item *pCurrentItem = (Elm_Object_Item *) event_info;

	Eina_List *pCurList = NULL;
	Elm_Object_Item *pTmpVideoItem = NULL;
	EINA_LIST_FOREACH(g_pNormalView->pVideoItemList, pCurList,
			  pTmpVideoItem) {
		if (pCurrentItem == pTmpVideoItem) {
			mp_util_svc_cancel_thumbnail_update(nIndexGenlist);
			VideoLogInfo("testestesteste=%p", pCurrentItem);
			pTmpVideoItem = NULL;
			pCurList->data = NULL;
			break;
		}
		nIndexGenlist++;
	}
}


void mp_normal_view_append_video_items(void *pVideosGenlist)
{
	VideoLogInfo("");
	if (!g_pNormalView) {
		VideoLogError("");
		return;
	}

	int nIndex = 0;
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	Elm_Object_Item *pTmpVideoGenlistItem = NULL;

	MP_DEL_ITC(g_pNormalView->pVideoList_Itc);
	g_pNormalView->pVideoList_Itc = elm_genlist_item_class_new();
	g_pNormalView->pVideoList_Itc->item_style = "2line.top";
	g_pNormalView->pVideoList_Itc->func.text_get =
		(void *) mp_normal_view_get_label_of_video_item_cb;
	g_pNormalView->pVideoList_Itc->func.content_get =
		(void *) mp_normal_view_get_icon_of_video_item_cb;
	g_pNormalView->pVideoList_Itc->func.state_get = NULL;
	g_pNormalView->pVideoList_Itc->func.del = NULL;
	g_pNormalView->pVideoList_Itc->decorate_item_style = NULL;

	for (nIndex = 0; nIndex < nVideoListSize; nIndex++) {
		pTmpVideoGenlistItem =
			elm_genlist_item_append(pVideosGenlist,
						g_pNormalView->pVideoList_Itc,
						(void *) nIndex, NULL,
						ELM_GENLIST_ITEM_NONE,
						mp_normal_view_select_video_item_cb,
						NULL);
		g_pNormalView->pVideoItemList =
			eina_list_append(g_pNormalView->pVideoItemList,
					 pTmpVideoGenlistItem);
		elm_object_item_data_set(pTmpVideoGenlistItem, (void *) nIndex);
		elm_object_item_del_cb_set(pTmpVideoGenlistItem,
					   __mp_normal_view_video_item_del_cb);
	}
}

#ifdef ENABLE_LONGPRESS
static void __mp_normal_view_share_via_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	mp_option_ctrl_delete_handle();

	if (!g_pNormalView->pMainHandle) {
		VideoLogInfo
		("g_pNormalView->pMainHandle of list view are not existed.");
		return;
	}
	VideoLogInfo("");

	int videoItemIndex = (int) pUserData;
	char *pVideo_url = mp_util_svc_get_video_url(videoItemIndex);
#ifdef ENABLE_DRM_FEATURE
	bool bDRMLock = FALSE;
	mp_util_check_drm_lock((const char *) pVideo_url, &bDRMLock);
	if (bDRMLock) {
		MP_FREE_STRING(pVideo_url);
		mp_util_notify_popup(NULL, VIDEOS_COMMON_HEADER_ERROR,
				     VIDEOS_NOTIPOP_MSG_UNABLE_TO_SHARE_DRM_FILE,
				     __mp_normal_view_warning_popup_cb);
		return;
	}
#endif
	if (g_pNormalView->pMainHandle) {
		mp_external_service_share_panel(pVideo_url);	/*,  g_pNormalView->pMainHandle->pNaviFrameHandle);*/
	}
	MP_FREE_STRING(pVideo_url);
}

static void __mp_normal_view_add_to_home_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	mp_option_ctrl_delete_handle();

	if (!g_pNormalView->pMainHandle) {
		VideoLogError
		("g_pNormalView->pMainHandle of list view are not existed.");
		return;
	}

	int nVideoItemIndex = (int) pUserData;

	VideoLogInfo("nVideoItemIndex : %d", nVideoItemIndex);


	char *pVideo_id = mp_util_svc_get_video_id_by_index(nVideoItemIndex);
	char *pVideo_thumbnail =
		mp_util_svc_get_video_thumbnail(nVideoItemIndex);

	if (pVideo_id) {
		/*              if (!mp_util_add_to_home(pVideo_id, pVideo_thumbnail))*/
		{
			mp_util_status_bar_noti_popup_show(MP_COM_OPT_UNABLE_ADDED);
		}
	} else {
		VideoLogError("[ERR] No exist file uri.");
	}
	MP_FREE_STRING(pVideo_id);
	MP_FREE_STRING(pVideo_thumbnail);

	/*to do share*/
}
#endif

void mp_normal_view_result_user_cb(void)
{
	VideoLogInfo("");
	if (!g_pNormalView) {
		VideoLogError("");
		return;
	}
	if (g_pNormalView->fUpdateListItemUserCb) {
		g_pNormalView->fUpdateListItemUserCb(LIST_UPDATE_TYPE_ALL);
	}
}

#ifdef ENABLE_LONGPRESS
static void mp_normal_view_detail_video_popup_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	mp_util_delete_popup_handle();
}

static void mp_normal_view_detail_mouse_back_popup_cb(void *pUserData,
		Evas *pEvas,
		Evas_Object *
		pObject,
		void *pEventInfo)
{
	Evas_Event_Mouse_Up *pEvent = pEventInfo;
	if (!pEvent) {
		return;
	}

	if (pEvent->button == 3) {
		mp_util_delete_popup_handle();
	}
}
#endif

void mp_normal_view_detail_key_back_popup_cb(void *pUserData,
		Evas *pEvas,
		Evas_Object *pObject,
		void *pEventInfo)
{
	Evas_Event_Key_Down *pEvent = pEventInfo;
	if (!pEvent) {
		return;
	}

	if (!g_strcmp0(pEvent->keyname, "XF86Back")) {
		mp_util_delete_popup_handle();
	}
}

#ifdef ENABLE_LONGPRESS
static void __mp_normal_view_delete_video_items(int nVideoItemIndex)
{
	if (!g_pNormalView->pVideoItemList || !g_pNormalView->pMainHandle) {
		VideoLogWarning
		("[WARNING] g_pNormalView->pVideoItemList is not existed.");
		return;
	}

	Elm_Object_Item *pTmpVideoGenlistItem =
		(Elm_Object_Item *) eina_list_nth(g_pNormalView->pVideoItemList,
				nVideoItemIndex);
	if (!pTmpVideoGenlistItem) {
		VideoLogWarning
		("[WARNING] g_pNormalView->pVideoItemList is not existed.");
		return;
	}


	Elm_Object_Item *pNextGenlistItem = NULL;
	Elm_Object_Item *pCurrGenlistItem =
		elm_genlist_item_next_get(pTmpVideoGenlistItem);

	g_pNormalView->pVideoItemList =
		eina_list_remove(g_pNormalView->pVideoItemList,
				 pTmpVideoGenlistItem);
	elm_object_item_del(pTmpVideoGenlistItem);

	if (!pCurrGenlistItem) {
		return;
	}

	while (pCurrGenlistItem) {
		pNextGenlistItem = elm_genlist_item_next_get(pCurrGenlistItem);
		g_pNormalView->pVideoItemList =
			eina_list_remove(g_pNormalView->pVideoItemList,
					 pCurrGenlistItem);
		elm_object_item_del(pCurrGenlistItem);
		pCurrGenlistItem = pNextGenlistItem;
	}

	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	int nIndex = nVideoItemIndex;
	for (nIndex = nVideoItemIndex; nIndex < nVideoListSize; nIndex++) {
		pTmpVideoGenlistItem =
			elm_genlist_item_append(g_pNormalView->pVideosGenlist,
						g_pNormalView->pVideoList_Itc,
						(void *) nIndex, NULL,
						ELM_GENLIST_ITEM_NONE,
						mp_normal_view_select_video_item_cb,
						NULL);
		g_pNormalView->pVideoItemList =
			eina_list_append(g_pNormalView->pVideoItemList,
					 pTmpVideoGenlistItem);
		elm_object_item_del_cb_set(pTmpVideoGenlistItem,
					   __mp_normal_view_video_item_del_cb);
	}

	if (nVideoListSize == 0) {
		if (g_pNormalView->fUpdateListItemUserCb) {
			g_pNormalView->fUpdateListItemUserCb(LIST_UPDATE_TYPE_VIEW);
		}
	}


}


static void mp_normal_view_respones_delete_popup_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	mp_util_delete_popup_handle();

	int nVideoItemIndex = (int) pUserData;

	VideoLogInfo("nVideoItemIndex : %d", nVideoItemIndex);

	if (nVideoItemIndex < 0
			|| nVideoItemIndex >=
			mp_util_svc_get_video_list_size_for_checking_index()) {
		VideoLogError("[ERR] list item index.");
		return;
	}

	char *szFileUrl = mp_util_svc_get_video_url(nVideoItemIndex);
	if (szFileUrl) {

		char *szMediaUrlForPreview = mp_util_config_get_preview();
		if (szMediaUrlForPreview) {
			if (!g_strcmp0(szMediaUrlForPreview, szFileUrl)) {
				VideoLogInfo("Erase preview url.");
				mp_util_config_set_preview(NULL);
			}
		}
		MP_FREE_STRING(szMediaUrlForPreview);
		VideoSecureLogInfo("Delete file url : %s", szFileUrl);

		vp_file_unlink(szFileUrl);
		free(szFileUrl);
		szFileUrl = NULL;
		mp_util_svc_remove_item_by_list_index(nVideoItemIndex);

		__mp_normal_view_delete_video_items(nVideoItemIndex);

		mp_list_view_update_count_layout();
		int nCount = mp_util_svc_get_number_of_video_item_by_type();
		if (nCount < 1) {
			__mp_normal_view_create_no_content();
		}
	}
}

static void __mp_normal_view_delete_cb(void *pUserData,
				       Evas_Object *pObject,
				       void *pEventInfo)
{
	mp_option_ctrl_delete_handle();

	if (!g_pNormalView->pMainHandle) {
		VideoLogError
		("g_pNormalView->pMainHandle of list view are not existed.");
		return;
	}

	int nVideoItemIndex = (int) pUserData;
	MpListViewAsType type = MP_LIST_VIEW_AS_NORMAL_LIST;
	mp_util_remove_question_popup(nVideoItemIndex, type,
				      mp_normal_view_respones_delete_popup_cb,
				      mp_normal_view_detail_video_popup_cb,
				      mp_normal_view_detail_mouse_back_popup_cb,
				      mp_normal_view_detail_mouse_back_popup_cb);

}

static void __mp_normal_view_result_pop_up()
{
	if (!g_pNormalView) {
		VideoLogError("");
		return;
	}

	if (!g_pNormalView->pMainHandle) {
		VideoLogError("Invalid list and object");
		return;
	}
	evas_object_data_set(g_pNormalView->pVideosGenlist, "popup", NULL);
	elm_object_scroll_freeze_pop(g_pNormalView->pVideosGenlist);
}

/**/
/*////////////////////////////////////////////////////////////*/

static void __mp_normal_view_long_pressed_item_cb(void *data,
		Evas_Object *obj,
		void *event_info)
{
	if (!g_pNormalView || !event_info || !obj) {
		VideoLogError("Invalid list and object");
		return;
	}
	g_pNormalView->bRunLongPress = true;

	Elm_Object_Item *long_item = (Elm_Object_Item *) event_info;
	/*elm_genlist_item_selected_set(long_item, EINA_FALSE);*/
	int nVideoItemIndex = elm_genlist_item_index_get(long_item);
	if (nVideoItemIndex < 0) {
		VideoLogError("nGenlistItemIndex is invalid");
		return;
	}

	if (nVideoItemIndex < 0
			|| nVideoItemIndex >=
			mp_util_svc_get_video_list_size_for_checking_index()) {
		VideoSecureLogError("nVideoItemIndex is invalid");
		return;
	}

	st_RegisteOptionCbFunc *pRegisteCbFunc =
		(st_RegisteOptionCbFunc *) calloc(1,
				sizeof(st_RegisteOptionCbFunc));
	if (pRegisteCbFunc) {
		pRegisteCbFunc->ShareViaItemCb =
			(void *) __mp_normal_view_share_via_cb;
		pRegisteCbFunc->AddToHomeItemCb =
			(void *) __mp_normal_view_add_to_home_cb;
		pRegisteCbFunc->DeleteItemCb =
			(void *) __mp_normal_view_delete_cb;
	}

	VideoLogDebug("nVideoItemIndex=:%d", nVideoItemIndex);

	char *szTitle = mp_util_svc_get_video_title(nVideoItemIndex);
	mp_option_ctrl_show(g_pNormalView->pMainHandle->pNaviFrameHandle,
			    (const char *) szTitle, nVideoItemIndex,
			    pRegisteCbFunc, __mp_normal_view_result_pop_up,
			    NULL, 0);
	if (mp_option_ctrl_get_handle()) {
		evas_object_data_set(g_pNormalView->pVideosGenlist, "popup",
				     mp_option_ctrl_get_handle());
		elm_object_scroll_freeze_push(g_pNormalView->pVideosGenlist);
	}

	MP_FREE_STRING(pRegisteCbFunc);
	MP_FREE_STRING(szTitle);
}
#endif

static void __mp_normal_view_pressed_item_cb(void *data,
		Evas_Object *obj,
		void *event_info)
{
	if (!g_pNormalView) {
		VideoLogError("Invalid list");
		return;
	}
	VideoLogInfo("");
	g_pNormalView->bRunLongPress = false;
}

static void __mp_normal_view_del_genlist_cb(Evas_Object *pVideosGenlist)
{
	if (!pVideosGenlist) {
		VideoSecureLogError("pVideosGenlist is NULL");
		return;
	}
	evas_object_smart_callback_del(pVideosGenlist, "language,changed",
				       __mp_normal_view_genlist_language_changed_cb);
	evas_object_smart_callback_del(pVideosGenlist, "realized",
				       __mp_normal_view_realize_genlist_item_cb);
#ifdef ENABLE_LONGPRESS
	evas_object_smart_callback_del(pVideosGenlist, "longpressed",
				       __mp_normal_view_long_pressed_item_cb);
#endif
	evas_object_smart_callback_del(pVideosGenlist, "pressed",
				       __mp_normal_view_pressed_item_cb);
}

static void __mp_normal_view_add_genlist_cb(Evas_Object *pVideosGenlist)
{
	if (!pVideosGenlist) {
		VideoSecureLogError("pVideosGenlist is NULL");
		return;
	}

	evas_object_smart_callback_add(pVideosGenlist, "language,changed",
				       __mp_normal_view_genlist_language_changed_cb,
				       NULL);
	evas_object_smart_callback_add(pVideosGenlist, "realized",
				       __mp_normal_view_realize_genlist_item_cb,
				       NULL);
#ifdef ENABLE_LONGPRESS
	evas_object_smart_callback_add(pVideosGenlist, "longpressed",
				       __mp_normal_view_long_pressed_item_cb,
				       NULL);
#endif
	evas_object_smart_callback_add(pVideosGenlist, "pressed",
				       __mp_normal_view_pressed_item_cb,
				       NULL);

}

/*//////////////////////////////////////////////////*/
/* External APIs*/
bool mp_normal_view_arrange_list_item(void *pMainViewHandle,
				      void *pMainViewWidget,
				      MpListUpdateType eUpdateType,
				      UpdateListItemsCbFunc
				      pUpdateVideoListUserCb)
{
	if (!pMainViewHandle || !pMainViewWidget || !pUpdateVideoListUserCb) {
		VideoLogError
		("pMainViewHandle or pMainViewHandle or pUpdateVideoListUserCb are not existed.");
		return FALSE;
	}

	if (eUpdateType == LIST_UPDATE_TYPE_NONE) {
		VideoLogInfo("don't need to update and create");
		return TRUE;
	}

	__mp_normal_view_reset(eUpdateType);

	if (!g_pNormalView) {
		g_pNormalView = (stNormalView *) calloc(1, sizeof(stNormalView));
	}

	g_pNormalView->pMainHandle = pMainViewHandle;
	g_pNormalView->pMainWidget = pMainViewWidget;
	g_pNormalView->fUpdateListItemUserCb = pUpdateVideoListUserCb;

	int nVideoListSize = 0;

	if (eUpdateType == LIST_UPDATE_TYPE_ALL
			|| eUpdateType == LIST_UPDATE_TYPE_DATA) {
		mp_util_svc_destory_video_list();
		mp_util_svc_extract_video_list_by_item_type
		(mp_sort_ctrl_get_sort_state(), mp_view_ctrl_get_list_type());
	}

	nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	VideoLogInfo("nVideoListSize : %d", nVideoListSize);
	/*mp_util_svc_set_update_db_cb_func(g_pNormalView->fUpdateListItemUserCb);*/
	mp_list_view_update_count_layout();

	if (nVideoListSize > 0) {
		if (g_pNormalView->pNocontentlayout) {
			elm_object_part_content_unset(g_pNormalView->pNocontentlayout,
						      SWALLOW_LISTVIEW_CONTENT);
			MP_DEL_OBJ(g_pNormalView->pNocontentlayout);
		}
		if (!g_pNormalView->pVideosGenlist) {
			g_pNormalView->pVideosGenlist =
				elm_genlist_add(g_pNormalView->pMainWidget->
						pListViewBaselayout);
			elm_scroller_bounce_set(g_pNormalView->pVideosGenlist,
						EINA_FALSE, EINA_TRUE);
			evas_object_size_hint_weight_set(g_pNormalView->
							 pVideosGenlist,
							 EVAS_HINT_EXPAND,
							 EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(g_pNormalView->pVideosGenlist,
							EVAS_HINT_FILL,
							EVAS_HINT_FILL);
			elm_genlist_mode_set(g_pNormalView->pVideosGenlist,
					     ELM_LIST_COMPRESS);
			elm_genlist_homogeneous_set(g_pNormalView->pVideosGenlist,
						    EINA_TRUE);

			elm_genlist_block_count_set(g_pNormalView->pVideosGenlist,
						    VIDEO_MAIN_LIST_BLOCK_COUNT);
			elm_scroller_single_direction_set(g_pNormalView->
							  pVideosGenlist,
							  ELM_SCROLLER_SINGLE_DIRECTION_HARD);
			evas_object_show(g_pNormalView->pMainWidget->
					 pListViewBaselayout);
			__mp_normal_view_del_genlist_cb(g_pNormalView->
							pVideosGenlist);
			__mp_normal_view_add_genlist_cb(g_pNormalView->
							pVideosGenlist);
		}
		elm_object_part_content_set(g_pNormalView->pMainWidget->
					    pListViewBaselayout,
					    SWALLOW_LISTVIEW_CONTENT,
					    g_pNormalView->pVideosGenlist);

		mp_normal_view_append_video_items(g_pNormalView->pVideosGenlist);

		evas_object_show(g_pNormalView->pVideosGenlist);
	} else {
		__mp_normal_view_create_no_content();
	}

	return TRUE;
}

int mp_normal_view_get_selected_video_item_index(void)
{

	return g_pNormalView->nSelectedVideoItem;
}

void mp_normal_view_change_language(void)
{
	if (!mp_normal_view_is_top_view()) {
		VideoLogError("Main list view is not top view.");
		return;
	}

	VideoLogInfo("");
}

void mp_normal_view_update_video_list(void)
{
	if (!g_pNormalView || !g_pNormalView->pVideosGenlist) {
		VideoLogWarning
		("[ERR] pVideosGenlist or g_pMainViewWidget are NULL.");
		return;
	}

	VideoLogInfo("");

	Elm_Object_Item *pNextGenlistItem = NULL;
	Elm_Object_Item *pCurrGenlistItem =
		elm_genlist_first_item_get(g_pNormalView->pVideosGenlist);

	while (pCurrGenlistItem) {
		elm_genlist_item_update(pCurrGenlistItem);
		pNextGenlistItem = elm_genlist_item_next_get(pCurrGenlistItem);
		pCurrGenlistItem = pNextGenlistItem;
	}
}
#endif
