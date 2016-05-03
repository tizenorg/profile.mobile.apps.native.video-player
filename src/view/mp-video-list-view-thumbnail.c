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

#include "vp-play-log.h"
#include "vp-util.h"
#include "vp-file-util.h"
#include "vp-pinch-zoom.h"
#include "mp-util.h"
#ifdef ENABLE_DRM_FEATURE
#include "mp-drm-ctrl.h"
#endif
#include "mp-video-log.h"
#include "mp-util-media-service.h"
#include "mp-video-value-define.h"
#include "mp-video-string-define.h"


#include "mp-video-list-view-thumbnail.h"

#include "mp-footer-toolbar.h"
#include "mp-video-list-sort-ctrl.h"
#include "mp-video-view-popup-ctrl.h"
#include "mp-video-list-view-normal.h"
#include "mp-video-list-view-as-ctrl.h"
#include "mp-launching-video-displayer.h"
#include "mp-video-list-option-ctrl.h"
#include "mp-external-ug.h"
#include "videos-view-mgr.h"
#include "mp-rotate-ctrl.h"
#include "mp-video-rename-ctrl.h"
#include "mp-video-info-ctrl.h"
#include "mp-video-list-view-main.h"
#include "mp-video-detail-view.h"

#include "mp-util-preference.h"
#include "mp-util-config.h"
#include "mp-video-util-db-controller.h"
#include "mp-video-nocontent-layout.h"


#define VIDEO_WALL_UPDATE_TIMER		0.3
/*the max update waiting time is VIDEO_WALL_UPDATE_TIMER*VIDEO_WALL_UPDATE_TIMER_COUNTER*/
#define VIDEO_WALL_UPDATE_TIMER_COUNTER		6

typedef struct {
	UpdateListItemsCbFunc fViewCbFunc;
	st_VideoListViewMainViewHandle *pMainHandle;
	st_VideoListViewMainViewWidget *pMainWidget;

	Evas_Object *pVideosGenlist;

	Eina_List *pVideoItemList;
	Ecore_Idler *pPlayIdler;
	Ecore_Timer *pStartWallTimer;
	Ecore_Idler *pWallResolutionIdler;
	MpListUpdateType eUpdateType;

	/*wall*/
	Eina_List *pWallHandleList;
	Evas_Object *pNocontentlayout;

	Elm_Genlist_Item_Class *pGenGridItc;
	Elm_Genlist_Item_Class *pGenGridZoomItc;
	Elm_Genlist_Item_Class *pNoItemItc;
	Elm_Gengrid_Item_Class *pGenGridItc_l;
	Elm_Gengrid_Item_Class *pGenGridItc_r;

	bool bLandscape;
	int nSelectedVideoItem;
	int nUpdateTimerCounter;

	int zoom_level;
} stThumbnailView;


static stThumbnailView *g_pThumbnailView = NULL;


static void __mp_thumbnail_view_update_cb(int nError, const char *szPath,
		void *pUserData);
static void __mp_thumbnail_view_language_changed_genlist_cb(void
		*pUserData,
		Evas_Object *
		pObject,
		void
		*pEventInfo);
static void __mp_thumbnail_view_realize_genlist_item_cb(void *pUserData,
		Evas_Object *
		pObject,
		void *pEventInfo);
static void __mp_thumbnail_view_append_gengrid_items(Evas_Object *
		pGengrid,
		int nGenItemIndex);
static Evas_Object *__mp_thumbnail_view_create_gengrid(Evas_Object *
		parent, int nWidth,
		int nHeight);
static void __mp_thumbnail_view_reset(MpListUpdateType eUpdateType);

static int mp_thumbnail_get_portrait_count()
{
	if (g_pThumbnailView->zoom_level == VP_ZOOM_IN_MAX)
		return VP_GENLIST_THUMB_PORTRAIT_COUNT_ZOOM_IN;

	return VP_GENLIST_THUMB_PORTRAIT_COUNT;
}

static int mp_thumbnail_get_landscape_count()
{
	if (g_pThumbnailView->zoom_level == VP_ZOOM_IN_MAX)
		return VP_GENLIST_THUMB_LANDSPACE_COUNT_ZOOM_IN;

	return VP_GENLIST_THUMB_LANDSPACE_COUNT;
}

void mp_thumbnail_get_icon_dimensions(int *height, int *width)
{
	int winHeight = 0;
	int winWidth = 0;
	elm_win_screen_size_get(mp_util_get_main_window_handle(), NULL, NULL,
				&winWidth, &winHeight);

	bool bLandscape = mp_rotate_ctrl_check_landspace();
	if (bLandscape) {
		*width = winHeight / mp_thumbnail_get_landscape_count();
	} else {
		*width = winWidth / mp_thumbnail_get_portrait_count();
	}

	if (g_pThumbnailView->zoom_level == VP_ZOOM_IN_MAX)
		*height = VP_ALBUM_THUMB_ICON_HEIGHT_ZOOM_IN * MP_SCALE;
	else
		*height = VP_ALBUM_THUMB_ICON_HEIGHT * MP_SCALE;
}

void mp_thumbnail_view_cancel_thumbnail_update(int nIndexGenlist)
{
	if (!g_pThumbnailView) {
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
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	VideoLogInfo("realize is %d--%d--%d", nItemCount, nVideoItemIndex,
		     nVideoListSize);

	int nIndex = 0;
	for (; (nIndex < nItemCount) && (nVideoItemIndex < nVideoListSize);
			nIndex++) {
		nVideoItemIndex = nVideoItemIndex + nIndex;

		mp_util_svc_cancel_thumbnail_update(nVideoItemIndex);
	}

}

int mp_thumbnail_view_thumbnail_to_genlist_index(int nVideoItemIndex)
{
	/*nVideoItemIndex = 0-->max */
	if (nVideoItemIndex < 0) {
		VideoLogDebug("invalid nVideoItemIndex");
		return 0;
	}
	int nItemCount = mp_thumbnail_get_portrait_count();
	int nGenlistIndex = 0;
	if (g_pThumbnailView->bLandscape) {
		nItemCount = mp_thumbnail_get_landscape_count();
	}
	nGenlistIndex = nVideoItemIndex / nItemCount;
	return nGenlistIndex;
}

void __mp_thumbnail_view_warning_popup_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	VideoLogInfo("");

	mp_util_delete_popup_handle();
}

static Eina_Bool __mp_thumbnail_view_play_video_idler_cb(void *pUserData)
{
	if (!g_pThumbnailView) {
		VideoLogError("");
		return ECORE_CALLBACK_CANCEL;
	}

	g_pThumbnailView->pPlayIdler = NULL;

	int nVideoItemIndex = (int) pUserData;

	if (nVideoItemIndex < 0
			|| nVideoItemIndex >=
			mp_util_svc_get_video_list_size_for_checking_index()) {
		VideoLogError("[ERR] list item index.");
		return ECORE_CALLBACK_CANCEL;
	}

	VideoLogInfo("nVideoItemIndex %d", nVideoItemIndex);

	char *szMediaUrl = mp_util_svc_get_video_url(nVideoItemIndex);

	if (szMediaUrl) {
		if (!mp_util_check_local_file_exist(szMediaUrl)) {
			mp_util_notify_popup(NULL, VIDEOS_COMMON_HEADER_ERROR,
					     VIDEOS_NOTIPOP_MSG_FILE_NOT_EXIST,
					     __mp_thumbnail_view_warning_popup_cb);
			free(szMediaUrl);
			return ECORE_CALLBACK_CANCEL;
		}

		mp_launch_video_play(szMediaUrl, MP_PLAYER_TYPE_VIDEO, NULL);
	} else {
		VideoLogInfo("[Err] Fail to get video uri from media service.");
		return ECORE_CALLBACK_CANCEL;
	}

	MP_FREE_STRING(szMediaUrl);

	return ECORE_CALLBACK_CANCEL;
}

static Evas_Object *__mp_thumbnail_view_get_icon_of_grid_cb(const void
		*pUserData,
		Evas_Object *
		pObject,
		const char
		*pPart)
{
	if (!g_pThumbnailView || !pObject) {
		VideoLogError("g_pMainViewWidgetOfThumbnailList is NULL");
		return NULL;
	}

	int nGenItemIndex = (int) pUserData;
	int nHeight = 0;
	int nWidth = 0;
	mp_thumbnail_get_icon_dimensions(&nHeight, &nWidth);

	Evas_Object *pGengrid =
		__mp_thumbnail_view_create_gengrid(pObject, nWidth, nHeight);
	__mp_thumbnail_view_append_gengrid_items(pGengrid, nGenItemIndex);

	return pGengrid;
}

static void __mp_thumbnail_view_video_item_del_cb(void *data,
		Evas_Object *obj,
		void *event_info)
{
	if (!g_pThumbnailView || !event_info
			|| !g_pThumbnailView->pVideoItemList) {
		VideoLogError("g_pMainViewWidge is NULL");
		return;
	}
	int nIndexGenlist = 0;
	Elm_Object_Item *pCurrentItem = (Elm_Object_Item *) event_info;

	Eina_List *pCurList = NULL;
	Elm_Object_Item *pTmpVideoItem = NULL;
	EINA_LIST_FOREACH(g_pThumbnailView->pVideoItemList, pCurList,
			  pTmpVideoItem) {
		if (pCurrentItem == pTmpVideoItem) {
			nIndexGenlist = (int) elm_object_item_data_get(pCurrentItem);
			mp_thumbnail_view_cancel_thumbnail_update(nIndexGenlist);
			pTmpVideoItem = NULL;
			pCurList->data = NULL;
			break;
		}
	}
}

#ifdef ENABLE_LONGPRESS
static void __mp_thumbnail_view_free_video_wall_item(char *pVideoItemId)
{
	if (!pVideoItemId || !g_pThumbnailView
			|| !g_pThumbnailView->pWallHandleList) {
		VideoLogError("");
		return;
	}
}
#endif

static bool __mp_thumbnail_view_append_grid_layout(void *pGenlist)
{
	if (!pGenlist || !g_pThumbnailView) {
		VideoLogError("error handle.");
		return FALSE;
	}
	int nIndex = 0;
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	if (nVideoListSize <= 0) {
		VideoLogWarning("nVideoListSize = %d", nVideoListSize);
		return FALSE;
	}

	MP_DEL_ITC(g_pThumbnailView->pGenGridItc);
	g_pThumbnailView->pGenGridItc = elm_genlist_item_class_new();

	g_pThumbnailView->pGenGridItc->item_style = "video/1icon.thumbnail";
	g_pThumbnailView->pGenGridItc->func.text_get = NULL;
	g_pThumbnailView->pGenGridItc->func.content_get =
		(void *) __mp_thumbnail_view_get_icon_of_grid_cb;
	g_pThumbnailView->pGenGridItc->func.state_get = NULL;
	g_pThumbnailView->pGenGridItc->func.del = NULL;
	g_pThumbnailView->pGenGridItc->decorate_item_style = NULL;

	MP_DEL_ITC(g_pThumbnailView->pGenGridZoomItc);
	g_pThumbnailView->pGenGridZoomItc = elm_genlist_item_class_new();
	g_pThumbnailView->pGenGridZoomItc->item_style =
		"video/1icon.thumbnail.zoom";
	g_pThumbnailView->pGenGridZoomItc->func.text_get = NULL;
	g_pThumbnailView->pGenGridZoomItc->func.content_get =
		(void *) __mp_thumbnail_view_get_icon_of_grid_cb;
	g_pThumbnailView->pGenGridZoomItc->func.state_get = NULL;
	g_pThumbnailView->pGenGridZoomItc->func.del = NULL;
	g_pThumbnailView->pGenGridZoomItc->decorate_item_style = NULL;

	int nGenlistRow = 0;
	nGenlistRow =
		mp_thumbnail_view_thumbnail_to_genlist_index(nVideoListSize - 1);
	Elm_Object_Item *pTmpVideoGenlistItem = NULL;

	if (g_pThumbnailView->zoom_level == VP_ZOOM_IN_MAX) {
		for (nIndex = 0; nIndex <= nGenlistRow; nIndex++) {
			pTmpVideoGenlistItem =
				elm_genlist_item_append(pGenlist,
							g_pThumbnailView->pGenGridZoomItc,
							(void *) nIndex, NULL,
							ELM_GENLIST_ITEM_NONE, NULL,
							NULL);
			/*elm_genlist_item_select_mode_set(pTmpVideoGenlistItem, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);*/

			g_pThumbnailView->pVideoItemList =
				eina_list_append(g_pThumbnailView->pVideoItemList,
						 pTmpVideoGenlistItem);
			elm_object_item_del_cb_set(pTmpVideoGenlistItem,
						   __mp_thumbnail_view_video_item_del_cb);
		}
	} else {
		for (nIndex = 0; nIndex <= nGenlistRow; nIndex++) {
			pTmpVideoGenlistItem =
				elm_genlist_item_append(pGenlist,
							g_pThumbnailView->pGenGridItc,
							(void *) nIndex, NULL,
							ELM_GENLIST_ITEM_NONE, NULL,
							NULL);
			/*elm_genlist_item_select_mode_set(pTmpVideoGenlistItem, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);*/

			g_pThumbnailView->pVideoItemList =
				eina_list_append(g_pThumbnailView->pVideoItemList,
						 pTmpVideoGenlistItem);
			elm_object_item_del_cb_set(pTmpVideoGenlistItem,
						   __mp_thumbnail_view_video_item_del_cb);
		}
	}


	MP_DEL_TIMER(g_pThumbnailView->pStartWallTimer);
	g_pThumbnailView->nUpdateTimerCounter = 0;

	return TRUE;
}

Evas_Object *__mp_thumbnail_view_get_icon_of_no_item_cb(const void
		*pUserData,
		Evas_Object *
		pObject,
		const char *pPart)
{
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_PLAYER_NO_ITEM_EDJ);
	free(path);
	if (!strcmp(pPart, "elm.icon")) {
		int width = NO_ITEM_GENLIST_WIDTH * elm_config_scale_get();
		int height = NO_ITEM_GENLIST_HEIGHT * elm_config_scale_get();

		Evas_Object *pNoItemLayout = NULL;
		pNoItemLayout = elm_layout_add(pObject);
		elm_layout_file_set(pNoItemLayout, edj_path,
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


/*//////////////////////////////////////////////////*/
/* External APIs*/

static void __mp_thumbnail_view_rotate_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	if (!g_pThumbnailView) {
		VideoLogError("__mp_thumbnail_view_rotate_cb IS null");
		return;
	}
	bool bLandscape =
		mp_rotate_ctrl_check_landspace_by_win((Evas_Object *)
				mp_util_get_main_window_handle
				());
	if (bLandscape) {
		VideoLogInfo("landscape mode");
		g_pThumbnailView->bLandscape = true;
	} else {
		g_pThumbnailView->bLandscape = false;
	}
	elm_genlist_clear(g_pThumbnailView->pVideosGenlist);
	__mp_thumbnail_view_append_grid_layout(g_pThumbnailView->
					       pVideosGenlist);
}

int _grid_view_zoom_out(void *data, Evas_Object *view)
{
	st_VideoListViewMainViewWidget *ad =
		(st_VideoListViewMainViewWidget *) data;

	g_pThumbnailView->zoom_level = ad->zoom_level;
	elm_genlist_clear(g_pThumbnailView->pVideosGenlist);
	__mp_thumbnail_view_append_grid_layout(g_pThumbnailView->
					       pVideosGenlist);

	return 0;
}

int _grid_view_zoom_in(void *data, Evas_Object *view)
{
	st_VideoListViewMainViewWidget *ad =
		(st_VideoListViewMainViewWidget *) data;

	g_pThumbnailView->zoom_level = ad->zoom_level;
	elm_genlist_clear(g_pThumbnailView->pVideosGenlist);
	__mp_thumbnail_view_append_grid_layout(g_pThumbnailView->
					       pVideosGenlist);

	return 0;
}

bool mp_thumbnail_view_arrange_list_item(void *pMainViewHandle,
		void *pMainViewWidget,
		MpListUpdateType eUpdateType,
		UpdateListItemsCbFunc
		pUpdateVideoListUserCb)
{
	VideoLogInfo("");

	if (!pMainViewHandle || !pMainViewWidget || !pUpdateVideoListUserCb) {
		VideoLogInfo("Main handles of list view are not existed.");
		return FALSE;
	}

	if (eUpdateType == LIST_UPDATE_TYPE_NONE) {
		VideoLogInfo("don't need to update and create");
		return TRUE;
	}

	__mp_thumbnail_view_reset(eUpdateType);

	if (!g_pThumbnailView) {
		g_pThumbnailView =
			(stThumbnailView *) calloc(1, sizeof(stThumbnailView));
	}

	g_pThumbnailView->fViewCbFunc = pUpdateVideoListUserCb;
	g_pThumbnailView->pMainHandle = pMainViewHandle;
	g_pThumbnailView->pMainWidget = pMainViewWidget;
	g_pThumbnailView->eUpdateType = eUpdateType;
	g_pThumbnailView->zoom_level = VP_ZOOM_IN_DEFAULT;
	g_pThumbnailView->bLandscape =
		mp_rotate_ctrl_check_landspace_by_win((Evas_Object *)
				mp_util_get_main_window_handle
				());

	st_VideoListViewMainViewHandle *pMainHandle =
		g_pThumbnailView->pMainHandle;
	g_pThumbnailView->pVideosGenlist = pMainHandle->pVideosGenlist;

	int nVideoListSize = 0;

	if (g_pThumbnailView->eUpdateType == LIST_UPDATE_TYPE_ALL
			|| g_pThumbnailView->eUpdateType == LIST_UPDATE_TYPE_DATA) {
		mp_util_svc_destory_video_list();
		mp_util_svc_extract_video_list_by_item_type
		(mp_sort_ctrl_get_sort_state(), mp_view_ctrl_get_list_type());
	}

	nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();

	VideoLogInfo("nVideoListSize : %d", nVideoListSize);

	/*mp_list_view_update_count_layout();*/
	/*mp_list_view_update_base_layout();*/

	if (!g_pThumbnailView->pVideosGenlist) {
		g_pThumbnailView->pVideosGenlist =
			elm_genlist_add(g_pThumbnailView->pMainWidget->
					pListViewBaselayout);
		elm_scroller_bounce_set(g_pThumbnailView->pVideosGenlist,
					EINA_FALSE, EINA_TRUE);
		evas_object_size_hint_weight_set(g_pThumbnailView->pVideosGenlist,
						 EVAS_HINT_EXPAND,
						 EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(g_pThumbnailView->pVideosGenlist,
						EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_genlist_mode_set(g_pThumbnailView->pVideosGenlist,
				     ELM_LIST_COMPRESS);
		elm_scroller_single_direction_set(g_pThumbnailView->
						  pVideosGenlist,
						  ELM_SCROLLER_SINGLE_DIRECTION_HARD);
		evas_object_show(g_pThumbnailView->pMainWidget->
				 pListViewBaselayout);
	}

	elm_object_part_content_set(g_pThumbnailView->pMainWidget->
				    pListViewBaselayout,
				    SWALLOW_LISTVIEW_CONTENT,
				    g_pThumbnailView->pVideosGenlist);
	if (nVideoListSize > 0) {
		if (g_pThumbnailView->pNocontentlayout) {
			elm_object_part_content_unset(g_pThumbnailView->
						      pNocontentlayout,
						      SWALLOW_LISTVIEW_CONTENT);
			MP_DEL_OBJ(g_pThumbnailView->pNocontentlayout);
		}
		_vp_pinch_add_event(g_pThumbnailView->pMainWidget,
				    g_pThumbnailView->pMainWidget->
				    pListViewBaselayout);
		__mp_thumbnail_view_append_grid_layout(g_pThumbnailView->
						       pVideosGenlist);

	} else {
			g_pThumbnailView->pNocontentlayout =
				mp_create_nocontent_layout(g_pThumbnailView->pMainWidget->
							   pListViewBaselayout,
							   VIDEOS_NOCONTENT_MSG_NO_VIDEOS_IDS,
							   VIDEOS_NOCONTENT_MSG_NO_VIDEOS_DESC_IDS);
			elm_object_part_content_unset(g_pThumbnailView->pMainWidget->
						      pListViewBaselayout,
						      SWALLOW_LISTVIEW_CONTENT);
			if (g_pThumbnailView->pVideosGenlist) {
				evas_object_hide(g_pThumbnailView->pVideosGenlist);
			}
			evas_object_show(g_pThumbnailView->pMainWidget->
					 pListViewBaselayout);
			elm_object_part_content_set(g_pThumbnailView->pMainWidget->
						    pListViewBaselayout,
						    SWALLOW_LISTVIEW_CONTENT,
						    g_pThumbnailView->
						    pNocontentlayout);
	}

	evas_object_smart_callback_add((Evas_Object *)
				       mp_util_get_main_window_handle(),
				       "wm,rotation,changed",
				       __mp_thumbnail_view_rotate_cb, NULL);
	mp_list_common_set_callabcks(g_pThumbnailView->pMainHandle,
				     __mp_thumbnail_view_language_changed_genlist_cb,
				     __mp_thumbnail_view_realize_genlist_item_cb,
				     NULL, NULL);

	evas_object_show(g_pThumbnailView->pVideosGenlist);

	return TRUE;
}

int mp_thumbnail_view_get_selected_video_item_index(void)
{
	if (!g_pThumbnailView) {
		VideoLogError("");
		return 0;
	}

	VideoLogInfo("g_pThumbnailView->nSelectedVideoItem : %d",
		     g_pThumbnailView->nSelectedVideoItem);

	return g_pThumbnailView->nSelectedVideoItem;
}

static void __mp_thumbnail_view_update_video_items(int nVideoItemIndex)
{
	if (!g_pThumbnailView) {
		VideoLogError
		("g_pMainViewHandleOfThumbnailList are not existed.");
		return;
	}

	mp_util_svc_update_thumbnail_info(nVideoItemIndex);
}

void mp_thumbnail_view_update_genlist_item(int nVideoItemIndex)
{

	if (!g_pThumbnailView || !g_pThumbnailView->pVideosGenlist) {
		VideoSecureLogError("invalid handle.");
		return;
	}

	if (!g_pThumbnailView->pVideoItemList) {
		VideoSecureLogError
		("g_pThumbnailView->pVideosGenlist is not existed.");
		return;
	}

	int nGenistIndex =
		mp_thumbnail_view_thumbnail_to_genlist_index(nVideoItemIndex);
	Elm_Object_Item *pTmpVideoGenlistItem =
		(Elm_Object_Item *) eina_list_nth(g_pThumbnailView->
				pVideoItemList, nGenistIndex);
	if (pTmpVideoGenlistItem) {
		VideoLogWarning("Update index: %d", nVideoItemIndex);
		Evas_Object *gengrid =
			elm_object_item_part_content_get(pTmpVideoGenlistItem,
					"elm.icon");
		if (gengrid) {
			int nCount =
				(mp_rotate_ctrl_check_landspace() ? 
				 VP_GENLIST_THUMB_LANDSPACE_COUNT :
				 VP_GENLIST_THUMB_PORTRAIT_COUNT);
			int nIndex = nVideoItemIndex % nCount;
			Elm_Object_Item *updateItem =
				elm_gengrid_first_item_get(gengrid);
			while (nIndex > 0 && updateItem) {
				updateItem = elm_gengrid_item_next_get(updateItem);
				--nIndex;
			}

			VideoLogWarning("updateItem: %p", updateItem);
			if (updateItem)
				elm_gengrid_item_update(updateItem);
		}
	}
}

#ifdef ENABLE_LONGPRESS
static void __mp_thumbnail_view_share_via_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	mp_option_ctrl_delete_handle();
	if (!g_pThumbnailView) {
		VideoLogError("g_pThumbnailView of list view is not existed.");
		return;
	}

	int videoItemIndex = (int) pUserData;
	char *pVideo_url = mp_util_svc_get_video_url(videoItemIndex);
	bool bDRMLock = FALSE;
#ifdef ENABLE_DRM_FEATURE
	mp_util_check_drm_lock((const char *) pVideo_url, &bDRMLock);
#endif
	if (bDRMLock) {
		MP_FREE_STRING(pVideo_url);
		mp_util_notify_popup(NULL, VIDEOS_COMMON_HEADER_ERROR,
				     VIDEOS_NOTIPOP_MSG_UNABLE_TO_SHARE_DRM_FILE,
				     __mp_thumbnail_view_warning_popup_cb);
		return;
	}

	mp_external_service_share_panel(pVideo_url);
	MP_FREE_STRING(pVideo_url);
}
#endif

void __mp_thumbnail_view_result_user_cb(void)
{
	if (!g_pThumbnailView) {
		VideoLogError("");
		return;
	}
	if (g_pThumbnailView->fViewCbFunc) {
		g_pThumbnailView->fViewCbFunc(LIST_UPDATE_TYPE_ALL);
	}
}

#ifdef ENABLE_LONGPRESS
static void __mp_thumbnail_view_add_to_home_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	mp_option_ctrl_delete_handle();

	if (!g_pThumbnailView) {
		VideoLogInfo("g_pThumbnailView is not existed.");
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

}

static void __mp_thumbnail_view_detail_video_popup_cb(void *pUserData,
		Evas_Object *
		pObject,
		void *pEventInfo)
{
	VideoLogInfo("");

	mp_util_delete_popup_handle();
}

static void __mp_thumbnail_view_detail_mouse_back_popup_cb(void
		*pUserData,
		Evas *pEvas,
		Evas_Object *
		pObject,
		void
		*pEventInfo)
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

void __mp_thumbnail_view_detail_key_back_popup_cb(void *pUserData,
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

#if 0				/*Unused function*/
bool __mp_thumbnail_view_get_detail_info(int nVideoItemIndex,
		stDetailInfo **pDetailInfo)
{
	char *szFilePath = NULL;
	char *szVideoTitle = NULL;

	char szTmpStr[FORMAT_LEN_MAX] = { 0, };
	char szTmpFileDate[FORMAT_LEN_MAX] = { 0, };
	char szTmpFileExtension[FORMAT_LEN_MAX] = { 0, };
	char szTmpFileSize[FORMAT_LEN_MAX] = { 0, };

	/*double dLongitude = 0.0;*/
	/*double dLatitude = 0.0;*/

	int nWidth = 0;
	int nHeight = 0;
	stDetailInfo *pDetail = NULL;

	szFilePath = mp_util_svc_get_video_url(nVideoItemIndex);
	szVideoTitle = mp_util_svc_get_video_title(nVideoItemIndex);

	mp_info_ctrl_get_file_info(szFilePath, szTmpFileDate,
				   FORMAT_LEN_MAX - 1, szTmpFileExtension,
				   FORMAT_LEN_MAX - 1, szTmpFileSize,
				   FORMAT_LEN_MAX - 1);

	nWidth = mp_util_svc_get_video_item_width(nVideoItemIndex);
	nHeight = mp_util_svc_get_video_item_height(nVideoItemIndex);
	/*mp_info_ctrl_get_gps(szFilePath, &dLongitude, &dLatitude);*/

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

	snprintf(szTmpStr, FORMAT_LEN_MAX - 1, "%s", szTmpFileExtension);
	pDetail->szFormat = g_strdup(szTmpStr);
	memset(szTmpStr, 0, FORMAT_LEN_MAX);

	/*snprintf(szTmpStr, FORMAT_LEN_MAX-1, "%s", szTmpFileDate);
	   pDetail->szDate = g_strdup(szTmpStr);
	   memset(szTmpStr, 0, FORMAT_LEN_MAX); */

	snprintf(szTmpStr, FORMAT_LEN_MAX - 1, "%s", szTmpFileSize);
	pDetail->szSize = g_strdup(szTmpStr);
	memset(szTmpStr, 0, FORMAT_LEN_MAX);

	snprintf(szTmpStr, FORMAT_LEN_MAX - 1, "%d X %d", nWidth, nHeight);
	pDetail->szResolution = g_strdup(szTmpStr);
	memset(szTmpStr, 0, FORMAT_LEN_MAX);

	/*snprintf(szTmpStr, FORMAT_LEN_MAX-1, "%f", dLatitude);
	   pDetail->szLatitude = g_strdup(szTmpStr);
	   memset(szTmpStr, 0, FORMAT_LEN_MAX);

	   snprintf(szTmpStr, FORMAT_LEN_MAX-1, "%f", dLongitude);
	   pDetail->szLongitude = g_strdup(szTmpStr);
	   memset(szTmpStr, 0, FORMAT_LEN_MAX); */
	*pDetailInfo = pDetail;
	return TRUE;
}
#endif

#ifdef ENABLE_LONGPRESS
static void __mp_thumbnail_view_cloud_download_btn_cb(void *pUserData,
		Evas_Object *
		pObject,
		void *pEventInfo)
{
	VideoLogInfo("");

}

static void __mp_thumbnail_view_respones_delete_popup_cb(void *pUserData,
		Evas_Object *
		pObject,
		void *pEventInfo)
{
	if (!g_pThumbnailView) {
		VideoLogError("");
		return;
	}

	mp_util_delete_popup_handle();

	int nVideoItemIndex = (int) pUserData;

	VideoLogInfo("nVideoItemIndex : %d", nVideoItemIndex);

	if (nVideoItemIndex < 0
			|| nVideoItemIndex >=
			mp_util_svc_get_video_list_size_for_checking_index()) {
		VideoLogInfo("[ERR] list item index.");
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
		VideoLogInfo("Delete file url : %s", szFileUrl);
		vp_file_unlink(szFileUrl);
		free(szFileUrl);
		szFileUrl = NULL;
		char *pVideoId = NULL;
		pVideoId = mp_util_svc_get_video_id_by_index(nVideoItemIndex);
		__mp_thumbnail_view_free_video_wall_item(pVideoId);
		MP_FREE_STRING(pVideoId);
		mp_util_svc_remove_item_by_list_index(nVideoItemIndex);

		mp_list_view_update_count_layout();
		/*mp_list_view_update_base_layout();*/

		if (g_pThumbnailView->fViewCbFunc) {
			g_pThumbnailView->fViewCbFunc(LIST_UPDATE_TYPE_VIEW);
		}

	}

}

static void __mp_thumbnail_view_pop_up_delete_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	mp_option_ctrl_delete_handle();

	if (!g_pThumbnailView) {
		VideoLogError("g_pThumbnailView of list view are not existed.");
		return;
	}
	int nVideoItemIndex = (int) pUserData;
	mp_util_remove_question_popup(nVideoItemIndex,
				      (MpListViewAsType) 0,
				      __mp_thumbnail_view_respones_delete_popup_cb,
				      __mp_thumbnail_view_detail_video_popup_cb,
				      __mp_thumbnail_view_detail_mouse_back_popup_cb,
				      __mp_thumbnail_view_detail_key_back_popup_cb);

}

static void __mp_thumbnail_view_result_pop_up(void *pUserData)
{
	if (!pUserData) {
		VideoLogError("Invalid list and object");
		return;
	}
	Evas_Object *pGrid = (Evas_Object *) pUserData;
	evas_object_data_set(pGrid, "popup", NULL);
	elm_object_scroll_freeze_pop(pGrid);
}
#endif

static bool __mp_thumbnail_view_normal_is_top_view(void)
{
	VideoLogInfo("");

	if (!g_pThumbnailView) {
		VideoLogError("g_pThumbnailView is NULL");
		return FALSE;
	}
	if (!g_pThumbnailView->pMainHandle->pNaviFrameHandle) {
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

static void __mp_thumbnail_view_update_cb(int nError, const char *szPath,
		void *pUserData)
{
	int nVideoItemIndex = (int) pUserData;

	VideoLogInfo("nVideoItemIndex : %d", nVideoItemIndex);

	if (!g_pThumbnailView) {
		VideoLogError("g_pMainViewHandleOfThumbnailView is not existed.");
		return;
	}

	if (!g_pThumbnailView) {
		VideoLogError("g_pMainViewWidgetOfThumbnailView is not existed.");
		return;
	}

	if (!__mp_thumbnail_view_normal_is_top_view()) {
		VideoLogError("Main list view is not top view.");
		return;
	}

	if (nVideoItemIndex < 0
			|| nVideoItemIndex >=
			mp_util_svc_get_video_list_size_for_checking_index()) {
		VideoSecureLogError("nVideoItemIndex is invalid = %d",
				    nVideoItemIndex);
		return;
	}

	__mp_thumbnail_view_update_video_items(nVideoItemIndex);


	if (g_pThumbnailView->pVideosGenlist) {
		mp_thumbnail_view_update_genlist_item(nVideoItemIndex);
	} else {
		VideoLogError
		("[ERR] No exist g_pMainViewWidgetOfNormalList->pVideolistItemList.");
	}
}

static void __mp_thumbnail_view_update_empty_cb(int nError,
		const char *szPath,
		void *pUserData)
{
	int nVideoItemIndex = (int) pUserData;

	VideoLogInfo("nVideoItemIndex : %d", nVideoItemIndex);
}

static void __mp_thumbnail_view_realize_genlist_item_cb(void *pUserData,
		Evas_Object *
		pObject,
		void *pEventInfo)
{
	if (!pEventInfo || !g_pThumbnailView) {
		VideoLogError
		("invalid pEveninfo 0r g_pMainViewWidgetOfThumbnailList");
		return;
	}
}

static void __mp_thumbnail_view_language_changed_genlist_cb(void
		*pUserData,
		Evas_Object *
		pObject,
		void
		*pEventInfo)
{
	VideoLogInfo("");
	if (!pObject) {
		VideoLogError("pObject is NULL");
		return;
	}
	elm_genlist_realized_items_update(pObject);
}

void mp_thumbnail_view_change_language(void)
{
	if (!g_pThumbnailView) {
		VideoLogError("No existed handle of thumbnail view.");
		return;
	}

	VideoLogInfo("");
}

#ifdef ENABLE_LONGPRESS
static void __mp_thumbnail_view_gengrid_long_press_cb(void *pUserData,
		Evas_Object *
		pObject,
		void *pEventInfo)
{
	if (!g_pThumbnailView) {
		VideoLogError("");
		return;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is not existed.");
		return;
	}

	int nVideoItemIndex =
		(int) elm_object_item_data_get((Elm_Object_Item *) pEventInfo);
	VideoLogInfo("Select nVideoItemIndex: %d", nVideoItemIndex);
	if (nVideoItemIndex < 0
			|| nVideoItemIndex >=
			mp_util_svc_get_video_list_size_for_checking_index()) {
		VideoSecureLogError("nVideoItemIndex is invalid = %d",
				    nVideoItemIndex);
		return;
	}

	st_RegisteOptionCbFunc *pRegisteCbFunc =
		(st_RegisteOptionCbFunc *) calloc(1,
				sizeof(st_RegisteOptionCbFunc));
	if (pRegisteCbFunc) {
		pRegisteCbFunc->ShareViaItemCb =
			(void *) __mp_thumbnail_view_share_via_cb;
		pRegisteCbFunc->AddToHomeItemCb =
			(void *) __mp_thumbnail_view_add_to_home_cb;
		/*pRegisteCbFunc->RenameItemCb = (void*)__mp_thumbnail_view_rename_popup_cb;*/
		/*pRegisteCbFunc->DetailsItemCb= (void*)__mp_thumbnail_view_pop_up_detail_cb;*/
		pRegisteCbFunc->DeleteItemCb =
			(void *) __mp_thumbnail_view_pop_up_delete_cb;
		pRegisteCbFunc->DownloadCb =
			(void *) __mp_thumbnail_view_cloud_download_btn_cb;
	}

	char *title = mp_util_svc_get_video_title(nVideoItemIndex);

	mp_option_ctrl_show(g_pThumbnailView->pMainHandle->pNaviFrameHandle,
			    (const char *) title, nVideoItemIndex,
			    pRegisteCbFunc, __mp_thumbnail_view_result_pop_up,
			    g_pThumbnailView->pVideosGenlist,
			    MP_LIST_OPTION_CLOUD_DOWNLOAD_ITEM);
	if (mp_option_ctrl_get_handle()) {
		evas_object_data_set(g_pThumbnailView->pVideosGenlist, "popup",
				     mp_option_ctrl_get_handle());
		elm_object_scroll_freeze_push(g_pThumbnailView->pVideosGenlist);
	}
	MP_FREE_STRING(pRegisteCbFunc);
	MP_FREE_STRING(title);
}
#endif

static void __mp_thumbnail_view_gengrid_realize_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{

	if (!pEventInfo) {
		VideoLogError("pEventInfo is not existed.");
		return;
	}

	int nVideoItemIndex =
		(int) elm_object_item_data_get((Elm_Object_Item *) pEventInfo);
	VideoLogInfo("Select nVideoItemIndex: %d", nVideoItemIndex);
	if (nVideoItemIndex < 0
			|| nVideoItemIndex >=
			mp_util_svc_get_video_list_size_for_checking_index()) {
		VideoSecureLogError("nVideoItemIndex is invalid = %d",
				    nVideoItemIndex);
		return;
	}

	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	char *pThumbIconUri =
		mp_util_svc_get_video_thumbnail(nVideoItemIndex);
	VideoSecureLogInfo("pThumbIconUri = %s:%d", pThumbIconUri,
			   nVideoItemIndex);
	if (!pThumbIconUri) {
		if (nVideoItemIndex < nVideoListSize - 1) {
			mp_util_svc_register_thumbnail_update_func(nVideoItemIndex,
					__mp_thumbnail_view_update_empty_cb,
					(void *)
					nVideoItemIndex);
		} else {
			mp_util_svc_register_thumbnail_update_func(nVideoItemIndex,
					(void *)
					__mp_thumbnail_view_update_cb,
					(void *)
					nVideoItemIndex);
		}
	} else {
		free(pThumbIconUri);
		pThumbIconUri = NULL;
	}
}

static Evas_Object *__mp_thumbnail_view_create_gengrid(Evas_Object *
		parent, int nWidth,
		int nHeight)
{
	Evas_Object *pGengrid =
		mp_util_create_gengrid(parent, nWidth, nHeight, EINA_TRUE);
	if (!pGengrid) {
		VideoLogError("pVideoGrid is not existed.");
		return NULL;
	}
#ifdef ENABLE_LONGPRESS
	evas_object_smart_callback_add(pGengrid, "longpressed",
				       __mp_thumbnail_view_gengrid_long_press_cb,
				       NULL);
#endif
	evas_object_smart_callback_add(pGengrid, "realized",
				       __mp_thumbnail_view_gengrid_realize_cb,
				       NULL);

	return pGengrid;
}

static char *__mp_thumbnail_view_get_label_of_grid_item_cb(const void
		*pUserData,
		Evas_Object *
		pObject,
		const char
		*pPart)
{
	int nVideoItemIndex = (int) pUserData;

	if (!strcmp(pPart, "elm.text")) {
		char *szTitle;
		szTitle = mp_util_svc_get_video_title(nVideoItemIndex);
		char *szTitleUtf8 = elm_entry_utf8_to_markup(szTitle);

		if (szTitle) {
			free(szTitle);
			szTitle = NULL;
		}

		return szTitleUtf8;
	} else if (!strcmp(pPart, "elm.text.2")) {
		return mp_util_svc_get_duration_str_time(nVideoItemIndex);
	} else {
	}

	return NULL;
}

static Evas_Object *__mp_thumbnail_view_get_grid_icon_cb(const void
		*pUserData,
		Evas_Object *
		pObject,
		const char
		*pPart)
{
	int nVideoItemIndex = (int) pUserData;
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_PLAYER_IMAGE_NAME_EDJ);
	free(path);
	if (!strcmp(pPart, "elm.swallow.icon")) {
		char *pThumbIconUri =
			mp_util_svc_get_video_thumbnail(nVideoItemIndex);
		Evas_Object *thumb = NULL;
		int nWidth = 0;
		int nHeight = 0;

		thumb = elm_image_add(pObject);
		elm_image_preload_disabled_set(thumb, EINA_FALSE);
		elm_image_smooth_set(thumb, EINA_FALSE);
		elm_image_prescale_set(thumb, 50 * MP_SCALE);
		elm_image_file_set(thumb, pThumbIconUri, NULL);
		elm_image_object_size_get(thumb, &nWidth, &nHeight);
		if (nWidth > nHeight) {
			elm_image_aspect_fixed_set(thumb, EINA_FALSE);
		}
		if (thumb)
			evas_object_show(thumb);
		MP_FREE_STRING(pThumbIconUri);
		return thumb;
	} else if (!strcmp(pPart, "elm.progressbar.icon")) {
		int nLastPlayedTime =
			mp_util_svc_get_video_last_played_pos(nVideoItemIndex);
		int nDurationTime =
			mp_util_svc_get_video_duration_time(nVideoItemIndex);

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
		evas_object_size_hint_align_set(pProgressBar, EVAS_HINT_FILL,
						EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(pProgressBar, EVAS_HINT_EXPAND,
						 EVAS_HINT_EXPAND);
		elm_progressbar_value_set(pProgressBar, nRatio);
		evas_object_show(pProgressBar);

		return pProgressBar;
	} else if (!strcmp(pPart, "elm.lock.icon")) {
		char *pVideoFile = mp_util_svc_get_video_url(nVideoItemIndex);
		bool bRet =
			mp_util_check_video_personal_status((const char *)
					pVideoFile);
		MP_FREE_STRING(pVideoFile);
		if (bRet) {
			Evas_Object *pLockIcon = elm_image_add(pObject);
			elm_image_file_set(pLockIcon, edj_path,
					   VIDEO_LIST_VIEW_ICON_PERSONAL_LOCK);
			evas_object_size_hint_align_set(pLockIcon, EVAS_HINT_FILL,
							EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(pLockIcon, EVAS_HINT_EXPAND,
							 EVAS_HINT_EXPAND);
			evas_object_show(pLockIcon);
			return pLockIcon;
		} else {
			return NULL;
		}
	} else if (!strcmp(pPart, "elm.dropbox.icon")) {
		bool bRet = mp_util_svc_is_cloud_storage(nVideoItemIndex);
		if (bRet) {
			Evas_Object *pDropboxIcon = elm_image_add(pObject);
			elm_image_file_set(pDropboxIcon, edj_path,
					   VIDEO_LIST_VIEW_ICON_DROPBOX);
			evas_object_size_hint_align_set(pDropboxIcon, EVAS_HINT_FILL,
							EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(pDropboxIcon,
							 EVAS_HINT_EXPAND,
							 EVAS_HINT_EXPAND);
			evas_object_show(pDropboxIcon);
			return pDropboxIcon;
		} else {
			return NULL;
		}
	}

	return NULL;
}

static void __mp_thumbnail_view_select_grid_item_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	if (!g_pThumbnailView) {
		VideoLogError("");
		return;
	}

	if (!pObject) {
		VideoLogInfo("[ERR]No have pUserData");
		return;
	}
	Evas_Object *pPopup = evas_object_data_get(g_pThumbnailView->pVideosGenlist, "popup");	/* Get popup*/
	if (pPopup) {
		VideoLogInfo("long press is called");
		return;			/* If popup exists, do nothing*/
	}

	if (!mp_util_call_off()) {
		mp_util_ticker_toast_popup
		(VIDEOS_NOTIPOP_MSG_UNABLE_TO_PLAY_VIDEO_DURING_CALL, false,
		 false);
		return;
	}

	if (!mp_launch_video_allow_check()) {
		return;
	}

	int nVideoItemIndex = (int) pUserData;
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();

	VideoLogDebug("nVideoItemIndex = %d--%d", nVideoItemIndex,
		      nVideoListSize);

	if (nVideoItemIndex < 0 || nVideoItemIndex >= nVideoListSize) {
		VideoLogError("nVideoItemIndex < 0 ===>>> RETURN");
		return;
	}

	VideoLogInfo("nVideoItemIndex : %d", nVideoItemIndex);

	g_pThumbnailView->nSelectedVideoItem = nVideoItemIndex;

	MP_DEL_IDLER(g_pThumbnailView->pPlayIdler);
	g_pThumbnailView->pPlayIdler =
		ecore_idler_add(__mp_thumbnail_view_play_video_idler_cb,
				(void *) nVideoItemIndex);
}

static void __mp_thumbnail_view_grid_item_del_cb(void *pUserdata,
		Evas_Object *obj,
		void *event_info)
{
	if (!g_pThumbnailView || !event_info
			|| !g_pThumbnailView->pVideoItemList) {
		VideoLogError("g_pMainViewWidge is NULL");
		return;
	}
}

static void __mp_thumbnail_view_append_gengrid_items(Evas_Object *
		pGengrid,
		int nGenItemIndex)
{
	if (!g_pThumbnailView) {
		VideoLogError("");
		return;
	}

	MP_DEL_GRID_ITC(g_pThumbnailView->pGenGridItc_l);
	g_pThumbnailView->pGenGridItc_l = elm_gengrid_item_class_new();

	g_pThumbnailView->pGenGridItc_l->item_style = "video/album_grid";
	g_pThumbnailView->pGenGridItc_l->func.text_get =
		(void *) __mp_thumbnail_view_get_label_of_grid_item_cb;
	g_pThumbnailView->pGenGridItc_l->func.content_get =
		(void *) __mp_thumbnail_view_get_grid_icon_cb;
	g_pThumbnailView->pGenGridItc_l->func.state_get = NULL;
	g_pThumbnailView->pGenGridItc_l->func.del = NULL;


	int nCount =
		(mp_rotate_ctrl_check_landspace() ? 
		 mp_thumbnail_get_landscape_count() :
		 mp_thumbnail_get_portrait_count());
	int nIndex = nGenItemIndex * nCount;
	int nMaxIndex = nIndex + nCount;
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	Elm_Object_Item *gridItem = NULL;
	for (; (nIndex < nMaxIndex) && (nIndex < nVideoListSize); nIndex++) {
		gridItem =
			elm_gengrid_item_append(pGengrid,
						g_pThumbnailView->pGenGridItc_l,
						(void *) nIndex,
						__mp_thumbnail_view_select_grid_item_cb,
						(void *) nIndex);
		elm_gengrid_item_select_mode_set(gridItem,
						 ELM_OBJECT_SELECT_MODE_ALWAYS);
		elm_object_item_del_cb_set(gridItem,
					   __mp_thumbnail_view_grid_item_del_cb);
		elm_object_item_data_set(gridItem, (void *) nIndex);
	}
}

static void __mp_thumbnail_view_free_genlist_item_list(void)
{
	if (!g_pThumbnailView) {
		VideoLogError("");
		return;
	}

	if (g_pThumbnailView->pVideoItemList) {
		Elm_Object_Item *pTmpVideoGenlistItem = NULL;
		EINA_LIST_FREE(g_pThumbnailView->pVideoItemList,
			       pTmpVideoGenlistItem) {
			if (pTmpVideoGenlistItem) {
				int nIndex =
					(int) elm_object_item_data_get(pTmpVideoGenlistItem);
				mp_thumbnail_view_cancel_thumbnail_update(nIndex);
				elm_object_item_del(pTmpVideoGenlistItem);
			}

			pTmpVideoGenlistItem = NULL;
		}

		g_pThumbnailView->pVideoItemList = NULL;
	}

}

void mp_thumbnail_view_destroy(void)
{
	VideoLogInfo("");

	if (g_pThumbnailView) {
		__mp_thumbnail_view_free_genlist_item_list();

		MP_DEL_ITC(g_pThumbnailView->pGenGridItc);
		MP_DEL_ITC(g_pThumbnailView->pGenGridZoomItc);
		MP_DEL_IDLER(g_pThumbnailView->pPlayIdler);
		MP_DEL_TIMER(g_pThumbnailView->pStartWallTimer);
		MP_DEL_IDLER(g_pThumbnailView->pWallResolutionIdler);
		MP_DEL_GRID_ITC(g_pThumbnailView->pGenGridItc_l);
		MP_DEL_GRID_ITC(g_pThumbnailView->pGenGridItc_r);
		MP_DEL_OBJ(g_pThumbnailView->pVideosGenlist);

		free(g_pThumbnailView);
		g_pThumbnailView = NULL;
		evas_object_smart_callback_del((Evas_Object *)
					       mp_util_get_main_window_handle(),
					       "wm,rotation,changed",
					       __mp_thumbnail_view_rotate_cb);
	}
}

static void __mp_thumbnail_view_reset(MpListUpdateType eUpdateType)
{
	if (g_pThumbnailView) {
		__mp_thumbnail_view_free_genlist_item_list();

		MP_DEL_ITC(g_pThumbnailView->pGenGridItc);
		MP_DEL_ITC(g_pThumbnailView->pGenGridZoomItc);
		MP_DEL_IDLER(g_pThumbnailView->pPlayIdler);
		MP_DEL_TIMER(g_pThumbnailView->pStartWallTimer);
		MP_DEL_IDLER(g_pThumbnailView->pWallResolutionIdler);

		MP_DEL_GRID_ITC(g_pThumbnailView->pGenGridItc_l);
		MP_DEL_GRID_ITC(g_pThumbnailView->pGenGridItc_r);

		g_pThumbnailView->fViewCbFunc = NULL;
		g_pThumbnailView->pMainHandle = NULL;
		g_pThumbnailView->pMainWidget = NULL;
		g_pThumbnailView->nSelectedVideoItem = 0;
		g_pThumbnailView->nUpdateTimerCounter = 0;
		g_pThumbnailView->pVideosGenlist = NULL;
		g_pThumbnailView->eUpdateType = LIST_UPDATE_TYPE_NONE;
		if (g_pThumbnailView->pVideosGenlist) {
			elm_genlist_clear(g_pThumbnailView->pVideosGenlist);
		}
		evas_object_smart_callback_del((Evas_Object *)
					       mp_util_get_main_window_handle(),
					       "wm,rotation,changed",
					       __mp_thumbnail_view_rotate_cb);
	}

}

void mp_thumbnail_view_update_played_item(void)
{
	if (!g_pThumbnailView || (g_pThumbnailView->nSelectedVideoItem < 0)) {
		VideoLogInfo("No item to update.");
		return;
	}
	VideoLogInfo("%d", g_pThumbnailView->nSelectedVideoItem);

	if (g_pThumbnailView->pVideosGenlist) {
		int nGenItemIndex = g_pThumbnailView->nSelectedVideoItem;

		int nGenlistIndex = nGenItemIndex;
		g_pThumbnailView->bLandscape =
		mp_rotate_ctrl_check_landspace_by_win((Evas_Object *)
				mp_util_get_main_window_handle
				());
		if (g_pThumbnailView->bLandscape) {
			nGenlistIndex = nGenlistIndex / mp_thumbnail_get_landscape_count();
		} else {
			nGenlistIndex = nGenlistIndex / mp_thumbnail_get_portrait_count();
		}
		Elm_Object_Item *pTmpVideoGenlistItem = NULL;
		pTmpVideoGenlistItem =
			elm_genlist_nth_item_get((const Evas_Object *)
						 g_pThumbnailView->pVideosGenlist,
						 nGenlistIndex);
		mp_util_svc_update_thumbnail_info(g_pThumbnailView->
						  nSelectedVideoItem);

		Evas_Object *pGengrid = elm_object_item_part_content_get(pTmpVideoGenlistItem, "elm.icon");
		Elm_Object_Item *pTmpVideoGengridItem = NULL;
		pTmpVideoGengridItem = elm_gengrid_selected_item_get(pGengrid);
		elm_gengrid_item_update(pTmpVideoGengridItem);
	}
}

void mp_thumbnail_view_update_video_list(void)
{
	if (!g_pThumbnailView || !g_pThumbnailView->pVideosGenlist) {
		VideoLogWarning
		("[ERR] pVideosGenlist or g_pMainViewWidget are NULL.");
		return;
	}

	VideoLogInfo("");

	Elm_Object_Item *pNextGenlistItem = NULL;
	Elm_Object_Item *pCurrGenlistItem =
		elm_genlist_first_item_get(g_pThumbnailView->pVideosGenlist);

	while (pCurrGenlistItem) {
		elm_genlist_item_update(pCurrGenlistItem);
		pNextGenlistItem = elm_genlist_item_next_get(pCurrGenlistItem);
		pCurrGenlistItem = pNextGenlistItem;
	}
}
