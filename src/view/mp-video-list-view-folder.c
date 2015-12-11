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
#include <media_content.h>

#include "mp-util.h"
#include "video-player.h"
#include "mp-video-log.h"
#include "mp-video-string-define.h"
#include "mp-video-value-define.h"

#include "mp-util-media-service.h"
#include "mp-video-list-view-folder.h"
#include "mp-video-view-popup-ctrl.h"
#include "mp-video-list-sort-ctrl.h"
#include "mp-footer-toolbar.h"
#include "mp-video-search-view.h"
#include "mp-external-ug.h"
#include "videos-view-mgr.h"
#include "mp-video-list-view-common.h"
#include "mp-video-list-view-item-of-folder.h"
#include "mp-video-list-option-ctrl.h"
#include "vp-util.h"
#include "vp-file-util.h"
#include "mp-video-list-view-main.h"
#include "mp-video-nocontent-layout.h"
#include "mp-video-list-share-via-view.h"
#include "mp-video-detail-view.h"
#include "mp-rotate-ctrl.h"

#define NO_CONTENTS_W					720
#define	NO_CONTENTS_H					897


typedef struct {
	st_VideoListViewMainViewHandle *pMainHandle;
	st_VideoListViewMainViewWidget *pMainWidget;
	UpdateListItemsCbFunc fUpdateListItemUserCb;

	Evas_Object *pVideosGenlist;
	Evas_Object *pNocontentlayout;
	Elm_Genlist_Item_Class *pFolderList_Itc;
	Elm_Genlist_Item_Class *pFolderListLandscape_Itc;
	Elm_Genlist_Item_Class *pGenGridItc;

	bool bItemSelected;

	/*start popup*/
	Ecore_Idler *pRemoveIdler;
	Evas_Object *pRemovePopUpHandle;
	Evas_Object *pProgressbar;
	Ecore_Timer *pStartDelTimer;
	Ecore_Timer *pFinishDelTimer;
	int nCompltedDeleteItem;
	/*end popup*/
#ifdef VS_FEATURE_THUMBNAIL_VIEW	/**/
	bool bLandscape;
#endif

} stFolderView;


static stFolderView *g_pFolderView = NULL;

static void __mp_folder_view_button_popup_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo);
static void __mp_folder_view_delete_popup_handle(void);

void __mp_folder_view_create_no_content()
{
	if (!g_pFolderView) {
		VideoLogError("");
		return;
	}

	if (!g_pFolderView->pNocontentlayout) {
		g_pFolderView->pNocontentlayout =
			mp_create_nocontent_layout(g_pFolderView->pMainWidget->
						   pListViewBaselayout,
						   VIDEOS_NOCONTENT_MSG_NO_VIDEOS_IDS,
						   VIDEOS_NOCONTENT_MSG_NO_VIDEOS_DESC_IDS);
		elm_object_part_content_unset(g_pFolderView->pMainWidget->
					      pListViewBaselayout,
					      SWALLOW_LISTVIEW_CONTENT);
		if (g_pFolderView->pVideosGenlist) {
			evas_object_hide(g_pFolderView->pVideosGenlist);
		}
		evas_object_show(g_pFolderView->pMainWidget->pListViewBaselayout);
		elm_object_part_content_set(g_pFolderView->pMainWidget->
					    pListViewBaselayout,
					    SWALLOW_LISTVIEW_CONTENT,
					    g_pFolderView->pNocontentlayout);
	}

}

bool mp_folder_view_is_top_view(void)
{
	VideoLogInfo("");


	if (!g_pFolderView) {
		VideoLogError("g_pFolderView is NULL");
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

void mp_folder_view_destroy(void)
{
	VideoLogInfo("");

	if (g_pFolderView) {
		__mp_folder_view_delete_popup_handle();

		MP_DEL_ITC(g_pFolderView->pFolderList_Itc);
		g_pFolderView->fUpdateListItemUserCb = NULL;
		g_pFolderView->pMainHandle = NULL;
		g_pFolderView->pMainWidget = NULL;
		MP_DEL_OBJ(g_pFolderView->pNocontentlayout);
		MP_DEL_OBJ(g_pFolderView->pVideosGenlist);
	}
	MP_FREE_STRING(g_pFolderView);

}

static void __mp_folder_view_reset(MpListUpdateType eUpdateType)
{
	if (g_pFolderView) {
		__mp_folder_view_delete_popup_handle();

		MP_DEL_ITC(g_pFolderView->pFolderList_Itc);
		g_pFolderView->fUpdateListItemUserCb = NULL;
		g_pFolderView->pMainHandle = NULL;
		g_pFolderView->pMainWidget = NULL;
		if (g_pFolderView->pVideosGenlist) {
			elm_genlist_clear(g_pFolderView->pVideosGenlist);
		}

	}

}

static void __mp_folder_view_delete_popup_handle(void)
{
	VideoLogInfo("");
	if (g_pFolderView->pRemovePopUpHandle) {
		eext_object_event_callback_del(g_pFolderView->pRemovePopUpHandle,
					       EEXT_CALLBACK_BACK,
					       __mp_folder_view_button_popup_cb);
	}
	MP_DEL_TIMER(g_pFolderView->pStartDelTimer);
	MP_DEL_TIMER(g_pFolderView->pFinishDelTimer);
	MP_DEL_IDLER(g_pFolderView->pRemoveIdler);
	MP_DEL_OBJ(g_pFolderView->pProgressbar);
	MP_DEL_OBJ(g_pFolderView->pRemovePopUpHandle);
	g_pFolderView->nCompltedDeleteItem = 0;
}

void mp_folder_view_warning_popup_cb(void *pUserData,
				     Evas_Object *pObject,
				     void *pEventInfo)
{
	VideoLogInfo("");

	mp_util_delete_popup_handle();
}


/*//////////////////////////////////////////////////*/
/* For Folder View*/
static int mp_folder_view_get_respective_video_no(int nFolderItemIndex)
{
	mp_util_svc_destory_video_item_list();
	char *szPath = mp_util_svc_get_video_folder_url(nFolderItemIndex);
	mp_util_svc_extract_video_list_from_folder(szPath,
			mp_sort_ctrl_get_sort_state
			(), MP_LIST_VIEW_ALL);
	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();

	MP_FREE_STRING(szPath);

	return nVideoListSize;
}

static char *mp_folder_view_get_label_of_folder_cb(const void *pUserData,
		Evas_Object *pObject,
		const char *pPart)
{

	int nFolderItemIndex = (int) pUserData;

	if (!g_strcmp0(pPart, "elm.text.main.left.top")) {
		char *szTitle = NULL;
		szTitle = mp_util_svc_get_video_folder_name(nFolderItemIndex);

		char *szTitleUtf8 = elm_entry_utf8_to_markup(szTitle);

		MP_FREE_STRING(szTitle);
		return szTitleUtf8;
	} else if (!g_strcmp0(pPart, "elm.text.sub.left.bottom")) {
		char *szFolderPath = NULL;
		szFolderPath = mp_util_svc_get_video_folder_url(nFolderItemIndex);
		char *pDes_path = mp_util_get_dir_by_path(szFolderPath);
		MP_FREE_STRING(szFolderPath);
		char *szFolderUtf8 = elm_entry_utf8_to_markup(pDes_path);
		MP_FREE_STRING(pDes_path);
		return szFolderUtf8;

	} else if (!strcmp(pPart, "elm.text")) {
		char *szTitle1 = NULL;
		szTitle1 = mp_util_svc_get_video_folder_name(nFolderItemIndex);
		char *szTitleUtf8 = elm_entry_utf8_to_markup(szTitle1);

		MP_FREE_STRING(szTitle1);

		return szTitleUtf8;
	}
	else if (!strcmp(pPart, "elm.text.number")) {
		char szTitle2[10] = {0, };
		int nVideoListSize =
			mp_folder_view_get_respective_video_no(nFolderItemIndex);
		snprintf(szTitle2, 10, "(%d)", nVideoListSize);
		char *szTitleUtf8 = elm_entry_utf8_to_markup(szTitle2);

		return szTitleUtf8;
	}
	/*else if (!strcmp(pPart, "elm.text.2")) {
	   char *szFolderPath = NULL;
	   szFolderPath = mp_util_svc_get_video_folder_url(nFolderItemIndex);
	   char *pDes_path = mp_util_get_dir_by_path(szFolderPath);
	   MP_FREE_STRING(szFolderPath);
	   char *szFolderUtf8 = elm_entry_utf8_to_markup(pDes_path);
	   MP_FREE_STRING(pDes_path);
	   return szFolderUtf8;
	   } */
	return NULL;
}

static Evas_Object *mp_folder_view_get_icon_of_folder_cb(const void
		*pUserData,
		Evas_Object *
		pObject,
		const char
		*pPart)
{
	int nFolderItemIndex = (int) pUserData;
	VideoLogInfo("Index : %d", nFolderItemIndex);
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_CUSTOM_THEME);
	free(path);
	if (!g_strcmp0(pPart, "elm.icon.1")) {
		VideoLogInfo("pPart: elm.icon.1 - thumbnail");

		Evas_Object *pLayout = NULL;
		Evas_Object *pBg = NULL;
		char *pThumbIconUri =
			mp_util_get_folder_thumbnail(nFolderItemIndex,
						     mp_sort_ctrl_get_sort_state());

		pLayout = elm_layout_add(pObject);
		elm_layout_file_set(pLayout, edj_path,
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

		if (!mp_util_create_folder_sub_icon(pLayout, nFolderItemIndex)) {
			VideoLogWarning
			("Create sub icon is failed. nFolderItemIndex: %d",
			 nFolderItemIndex);
		}

		MP_FREE_STRING(pThumbIconUri);

		evas_object_show(pLayout);

		return pLayout;
	} else if (!strcmp(pPart, "elm.swallow.icon")) {
		char *pThumbIconUri =
			mp_util_get_folder_thumbnail(nFolderItemIndex,
						     mp_sort_ctrl_get_sort_state());
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
	} else if (!strcmp(pPart, "elm.sdcard.icon")) {
		MpMediaStorageType StorageType = MP_MEDIA_TYPE_STORAGE_UNKNOWN;
		StorageType = mp_util_svc_get_folder_storage(nFolderItemIndex);
		if (StorageType == MP_MEDIA_TYPE_STORAGE_EXTERNAL) {
			Evas_Object *pSdcardIcon = elm_image_add(pObject);
			elm_image_file_set(pSdcardIcon, edj_path,
					   VIDEO_LIST_VIEW_ICON_FOLDER_SDCARD);
			evas_object_size_hint_align_set(pSdcardIcon, EVAS_HINT_FILL,
							EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(pSdcardIcon,
							 EVAS_HINT_EXPAND,
							 EVAS_HINT_EXPAND);
			evas_object_show(pSdcardIcon);
			return pSdcardIcon;
		} else {
			return NULL;
		}
	}

	return NULL;
}

Evas_Object *mp_folder_view_get_icon_of_no_folder_item_cb(const void
		*pUserData,
		Evas_Object *
		pObject,
		const char
		*pPart)
{
	VideoSecureLogInfo("");

	if (!strcmp(pPart, "elm.icon")) {
		int width = NO_ITEM_GENLIST_WIDTH * elm_config_scale_get();
		int height = NO_ITEM_GENLIST_HEIGHT * elm_config_scale_get();
		char edj_path[1024] = {0};

		char *path = app_get_resource_path();
		snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_PLAYER_NO_ITEM_EDJ);
		free(path);
		Evas_Object *pNoItemLayout = NULL;
		pNoItemLayout = elm_layout_add(pObject);
		elm_layout_file_set(pNoItemLayout, edj_path,
				    GENLIST_NOITEM_EDJ_GROUP);
		evas_object_size_hint_min_set(pNoItemLayout, width, height);

		elm_object_part_text_set(pNoItemLayout,
					 SWALLOW_LISTVIEW_NO_ITEM_TEXT,
					 VIDEOS_NOCONTENT_MSG_NO_VIDEOS_DESC);
		/*elm_object_domain_translatable_part_text_set(pNoItemLayout, SWALLOW_LISTVIEW_NO_ITEM_TEXT, PACKAGE, VIDEOS_NOCONTENT_MSG_NO_VIDEOS_DESC);*/
		evas_object_show(pNoItemLayout);
		return pNoItemLayout;
	}

	return NULL;
}

void mp_folder_view_set_is_item_view(bool isSelected)
{
	if (g_pFolderView)
		g_pFolderView->bItemSelected = isSelected;
}

bool mp_folder_view_is_item_view()
{
	if (g_pFolderView)
		return g_pFolderView->bItemSelected;
	else
		return false;
}

static void mp_folder_view_gen_select_video_folder_cb(void *pUserData,
		Evas_Object *
		pObject,
		void *pEventInfo)
{
	if (!g_pFolderView || !pObject) {
		VideoLogWarning
		("[WARNING] Main handles of list view are not existed.");
		return;
	}
	Elm_Object_Item *pSelectedItem =
		elm_genlist_selected_item_get(pObject);
	if (pSelectedItem) {
		VideoLogInfo("pSelectedItem is NULL");
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}
	Evas_Object *pPopup = evas_object_data_get(pObject, "popup");	/* Get popup*/
	if (pPopup) {
		VideoLogInfo("long press is called");
		return;			/* If popup exists, do nothing*/
	}

	if (!g_pFolderView->fUpdateListItemUserCb) {
		VideoLogWarning("[WARNING] No existed pMainViewWidget.");
		return;
	}

	if (!pObject) {
		VideoLogWarning("[WARNING] No have pObject");
		return;
	}

	VideoLogInfo("");

	int nFolderItemIndex = 0;

	Elm_Object_Item *pItem = (Elm_Object_Item *) pEventInfo;

	if (!pItem) {
		VideoLogInfo("[ERR]Error item number.");
		return;
	}
	nFolderItemIndex = (int) elm_object_item_data_get(pItem);
	g_pFolderView->bItemSelected = true;

	VideoLogInfo("nFolderItemIndex:%d", nFolderItemIndex);
	mp_folder_item_view_push((void *) mp_mgr_get_library_naviframe(),
				 nFolderItemIndex,
				 (UpdateListItemsCbFunc) g_pFolderView->
				 fUpdateListItemUserCb);
}

#ifdef VS_FEATURE_THUMBNAIL_VIEW
static void __mp_folder_view_append_gengrid_items(void *pObjVideosList,
		int nGenItemIndex)
{
	if (!g_pFolderView) {
		VideoLogInfo("invalid handle");
		return;
	}

	MP_DEL_GRID_ITC(g_pFolderView->pGenGridItc);
	g_pFolderView->pGenGridItc = elm_gengrid_item_class_new();
	g_pFolderView->pGenGridItc->item_style = "video/folder_grid";
	g_pFolderView->pGenGridItc->func.text_get =
		(void *) mp_folder_view_get_label_of_folder_cb;
	g_pFolderView->pGenGridItc->func.content_get =
		(void *) mp_folder_view_get_icon_of_folder_cb;
	g_pFolderView->pGenGridItc->func.state_get = NULL;
	g_pFolderView->pGenGridItc->func.del = NULL;

	int nCount =
		(mp_rotate_ctrl_check_landspace() ?
		 VP_GENLIST_THUMB_LANDSPACE_COUNT :
		 VP_GENLIST_THUMB_PORTRAIT_COUNT);
	int nIndex = nGenItemIndex * nCount;
	int nMaxIndex = nIndex + nCount;
	int nVideoListSize = mp_util_svc_get_video_folder_size();

	Elm_Object_Item *gridItem = NULL;
	for (; (nIndex < nMaxIndex) && (nIndex < nVideoListSize); nIndex++) {
		elm_gengrid_item_append(pObjVideosList,
					g_pFolderView->pGenGridItc,
					(void *) nIndex,
					mp_folder_view_gen_select_video_folder_cb,
					NULL);
		elm_object_item_data_set(gridItem, (void *) nIndex);
	}
}

static Evas_Object *__mp_folder_view_create_gengrid(Evas_Object *parent,
		int nWidth,
		int nHeight)
{
	Evas_Object *pGengrid;
	pGengrid = elm_gengrid_add(parent);
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
	elm_gengrid_horizontal_set(pGengrid, EINA_TRUE);

	return pGengrid;
}

static void mp_folder_view_get_icon_dimensions(int *height, int *width)
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

static Evas_Object *__mp_folder_view_get_icon_of_grid_cb(const void
		*pUserData,
		Evas_Object *
		pObject,
		const char
		*pPart)
{
	VideoLogInfo("in __mp_folder_view_get_icon_of_grid_cb");
	if (!g_pFolderView || !pObject) {
		VideoLogError("g_pMainViewWidgetOfFolderList is NULL");
		return NULL;
	}

	int nGenItemIndex = (int) pUserData;
	int nHeight = 0;
	int nWidth = 0;
	mp_folder_view_get_icon_dimensions(&nHeight, &nWidth);

	Evas_Object *pGengrid =
		__mp_folder_view_create_gengrid(pObject, nWidth, nHeight);
	__mp_folder_view_append_gengrid_items(pGengrid, nGenItemIndex);

	return pGengrid;
}

int mp_folder_view_thumbnail_to_genlist_index(int nVideoItemIndex)
{
	if (nVideoItemIndex < 0) {
		VideoLogDebug("invalid nVideoItemIndex");
		return 0;
	}
	int nItemCount = VP_GENLIST_THUMB_PORTRAIT_COUNT;
	int nGenlistIndex = 0;
	if (g_pFolderView->bLandscape) {
		nItemCount = VP_GENLIST_THUMB_LANDSPACE_COUNT;
	}
	nGenlistIndex = nVideoItemIndex / nItemCount;
	return nGenlistIndex;
}

static bool __mp_folder_view_append_grid_layout(void *pGenlist)
{
	if (!pGenlist || !g_pFolderView) {
		VideoLogError("error handle.");
		return FALSE;
	}
	int nIndex = 0;
	int nVideoListSize = mp_util_svc_get_video_folder_size();
	if (nVideoListSize <= 0) {
		VideoLogWarning("nVideoListSize = %d", nVideoListSize);
		return FALSE;
	}

	MP_DEL_ITC(g_pFolderView->pGenGridItc);
	g_pFolderView->pFolderList_Itc = elm_genlist_item_class_new();

	g_pFolderView->pFolderList_Itc->item_style = "video/1icon.thumbnail";
	g_pFolderView->pFolderList_Itc->func.text_get = NULL;
	g_pFolderView->pFolderList_Itc->func.content_get =
		(void *) __mp_folder_view_get_icon_of_grid_cb;
	g_pFolderView->pFolderList_Itc->func.state_get = NULL;
	g_pFolderView->pFolderList_Itc->func.del = NULL;
	g_pFolderView->pFolderList_Itc->decorate_item_style = NULL;

	MP_DEL_ITC(g_pFolderView->pFolderListLandscape_Itc);
	g_pFolderView->pFolderListLandscape_Itc =
		elm_genlist_item_class_new();
	g_pFolderView->pFolderListLandscape_Itc->item_style =
		"video/1icon.thumbnail.landscape";
	g_pFolderView->pFolderListLandscape_Itc->func.text_get = NULL;
	g_pFolderView->pFolderListLandscape_Itc->func.content_get =
		(void *) __mp_folder_view_get_icon_of_grid_cb;
	g_pFolderView->pFolderListLandscape_Itc->func.state_get = NULL;
	g_pFolderView->pFolderListLandscape_Itc->func.del = NULL;
	g_pFolderView->pFolderListLandscape_Itc->decorate_item_style = NULL;

	int nGenlistRow = 0;
	nGenlistRow =
		mp_folder_view_thumbnail_to_genlist_index(nVideoListSize - 1);

	if (mp_rotate_ctrl_check_landspace()) {
		for (nIndex = 0; nIndex <= nGenlistRow; nIndex++) {
			elm_genlist_item_append(pGenlist,
						g_pFolderView->
						pFolderListLandscape_Itc,
						(void *) nIndex, NULL,
						ELM_GENLIST_ITEM_NONE, NULL, NULL);
			/*elm_genlist_item_select_mode_set(pTmpVideoGenlistItem, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);*/
		}
	} else {
		for (nIndex = 0; nIndex <= nGenlistRow; nIndex++) {
			elm_genlist_item_append(pGenlist,
						g_pFolderView->pFolderList_Itc,
						(void *) nIndex, NULL,
						ELM_GENLIST_ITEM_NONE, NULL, NULL);
			/*elm_genlist_item_select_mode_set(pTmpVideoGenlistItem, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);*/
		}
	}

	return TRUE;
}
#endif

/*void mp_folder_view_append_folder_items(void *pVideosGenlist)
{
	int nIndex = 0;
	int nFolderListSize = 0;

	nFolderListSize = mp_util_svc_get_video_folder_size();
	VideoLogInfo("nVideoListSize : %d", nFolderListSize);

	if (nFolderListSize > 0)
	{
		MP_DEL_ITC(g_pFolderView->pFolderList_Itc);
		g_pFolderView->pFolderList_Itc = elm_genlist_item_class_new();
		g_pFolderView->pFolderList_Itc->item_style = "2line.top";
		g_pFolderView->pFolderList_Itc->func.text_get = (void*)mp_folder_view_get_label_of_folder_cb;
		g_pFolderView->pFolderList_Itc->func.content_get = (void*)mp_folder_view_get_icon_of_folder_cb;
		g_pFolderView->pFolderList_Itc->func.state_get = NULL;
		g_pFolderView->pFolderList_Itc->func.del = NULL;
		g_pFolderView->pFolderList_Itc->decorate_item_style = NULL;

		for (nIndex = 0; nIndex < nFolderListSize; nIndex++)
		{
			VideoLogInfo("nCounter : %d", nIndex);
			elm_genlist_item_append(pVideosGenlist, g_pFolderView->pFolderList_Itc, (void*)nIndex, NULL, ELM_GENLIST_ITEM_NONE, mp_folder_view_gen_select_video_folder_cb, NULL);
		}
	}
}*/

#ifdef ENABLE_LONGPRESS
static void __mp_folder_view_delete_popup_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	VideoLogInfo("");

	mp_util_delete_popup_handle();
}

static void __mp_folder_view_mouse_back_up_popup_cb(void *pUserData,
		Evas *pEvas,
		Evas_Object *pObject,
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

void __mp_folder_view_key_back_popup_cb(void *pUserData, Evas *pEvas,
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
static void __mp_folder_view_remove_video_item(int nVideoItemIndex)
{
	/*int nVideoTotal = mp_util_svc_get_video_list_size_for_checking_index();*/

	char *szFileUrl = mp_util_svc_get_video_url(nVideoItemIndex);
	if (szFileUrl) {
		VideoSecureLogInfo("Delete file url : %s", szFileUrl);
		mp_util_svc_remove_item_from_db_by_list_index(nVideoItemIndex);
		unlink(szFileUrl);
		free(szFileUrl);
		szFileUrl = NULL;

	}
}

static Eina_Bool __mp_folder_view_finish_del_timer_cb(void *pUserData)
{

	g_pFolderView->pFinishDelTimer = NULL;
	if (pUserData) {
		int nFolderItemIndex = (int) pUserData;
		char *szTmpFolderUrl =
			mp_util_svc_get_video_folder_url(nFolderItemIndex);
		if (szTmpFolderUrl) {
			if (vp_file_rmdir(szTmpFolderUrl) == EINA_FALSE) {
				VideoLogError("vp_file_rmdir failed!");
			}

			MP_FREE_STRING(szTmpFolderUrl);
		}
	}
	mp_util_svc_destory_video_item_list();
	__mp_folder_view_delete_popup_handle();

	if (g_pFolderView->fUpdateListItemUserCb) {
		g_pFolderView->fUpdateListItemUserCb(LIST_UPDATE_TYPE_ALL);
	}

	return FALSE;
}

static Eina_Bool __mp_folder_view_delete_item_idler_cb(void *pUserData)
{

	int nVideoTotal =
		mp_util_svc_get_video_list_size_for_checking_index();
	VideoLogInfo("nVideoTotal = %d", nVideoTotal);
	/*      int nVideoItemIndex = 0;*/
	double nDeleteRate = 0.0;

	__mp_folder_view_remove_video_item(g_pFolderView->
					   nCompltedDeleteItem);

	g_pFolderView->nCompltedDeleteItem++;

	if (g_pFolderView->pProgressbar) {
		VideoLogInfo("Set progress bar");
		nDeleteRate =
			((double) g_pFolderView->nCompltedDeleteItem) /
			((double) nVideoTotal);
		elm_progressbar_value_set(g_pFolderView->pProgressbar,
					  nDeleteRate);
	}

	if (g_pFolderView->nCompltedDeleteItem == nVideoTotal) {
		g_pFolderView->pRemoveIdler = NULL;
		MP_DEL_TIMER(g_pFolderView->pFinishDelTimer);
		g_pFolderView->pFinishDelTimer =
			ecore_timer_add(0.1, __mp_folder_view_finish_del_timer_cb,
					pUserData);

		return FALSE;
	}
	return TRUE;
}
#endif

static void __mp_folder_view_button_popup_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	VideoLogInfo("");
	__mp_folder_view_delete_popup_handle();

	if (g_pFolderView->fUpdateListItemUserCb) {
		g_pFolderView->fUpdateListItemUserCb(LIST_UPDATE_TYPE_ALL);
	}

}

#ifdef ENABLE_LONGPRESS
static Eina_Bool __mp_folder_view_start_del_timer_cb(void *pUserData)
{
	g_pFolderView->pStartDelTimer = NULL;
	g_pFolderView->pRemoveIdler =
		ecore_idler_add(__mp_folder_view_delete_item_idler_cb, pUserData);
	return FALSE;
}

static void __mp_folder_view_respones_delete_popup_cb(void *pUserData,
		Evas_Object *
		pObject,
		void *pEventInfo)
{
	mp_util_delete_popup_handle();

	int nFolderItemIndex = (int) pUserData;
	if (nFolderItemIndex < 0
			|| nFolderItemIndex >= mp_util_svc_get_video_folder_size()) {
		VideoLogError("nFolderItemIndex is invalid");
		return;
	}

	char *szTmpFolderUrl =
		mp_util_svc_get_video_folder_url(nFolderItemIndex);

	VideoSecureLogInfo("folder_index : %d, szTmpFolderUrl : %s",
			   nFolderItemIndex, szTmpFolderUrl);
	mp_util_svc_destory_video_item_list();

	if (szTmpFolderUrl) {
		mp_util_svc_extract_video_list_from_folder(szTmpFolderUrl,
				MP_MEDIA_SORT_BY_NAME,
				MP_LIST_VIEW_ALL);
	} else {
		VideoLogError("folder path is NULL");
		return;
	}
	MP_FREE_STRING(szTmpFolderUrl);
#ifdef _MULTI_WINDOW
	g_pFolderView->pRemovePopUpHandle =
		ea_center_popup_add(mp_util_get_main_window_handle());
#else
	g_pFolderView->pRemovePopUpHandle =
		elm_popup_add(mp_util_get_main_window_handle());
#endif
	eext_object_event_callback_add(g_pFolderView->pRemovePopUpHandle,
				       EEXT_CALLBACK_BACK,
				       __mp_folder_view_button_popup_cb,
				       NULL);
	Evas_Object *pLabel =
		elm_label_add(g_pFolderView->pRemovePopUpHandle);
	elm_object_style_set(pLabel, "popup/default");
	elm_label_line_wrap_set(pLabel, ELM_WRAP_MIXED);
	elm_object_domain_translatable_text_set(pLabel, MP_SYS_STRING,
						VIDEOS_DELETEPOP_MSG_DELETING_IDS);
	evas_object_size_hint_weight_set(pLabel, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(pLabel, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	evas_object_show(pLabel);
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_PLAYER_POPUP_PROGRESSBAR_EDJ);
	free(path);
	Evas_Object *pPopupLayout =
		elm_layout_add(g_pFolderView->pRemovePopUpHandle);
	elm_layout_file_set(pPopupLayout, edj_path,
			    "popup_center_text_progressview");
	evas_object_size_hint_weight_set(pPopupLayout, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);

	Evas_Object *pProgressbar =
		elm_progressbar_add(g_pFolderView->pRemovePopUpHandle);
	elm_object_style_set(pProgressbar, "list_progress");
	elm_progressbar_horizontal_set(pProgressbar, EINA_TRUE);
	evas_object_size_hint_align_set(pProgressbar, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(pProgressbar, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_progressbar_value_set(pProgressbar, 0.0);

	evas_object_show(pProgressbar);
	g_pFolderView->pProgressbar = pProgressbar;

	elm_object_part_content_set(pPopupLayout, "elm.swallow.content",
				    pLabel);
	elm_object_part_content_set(pPopupLayout, "elm.swallow.end",
				    pProgressbar);
	elm_object_part_text_set(pPopupLayout, "elm.text.subtext1", "");
	elm_object_part_text_set(pPopupLayout, "elm.text.subtext2", "");

	elm_object_content_set(g_pFolderView->pRemovePopUpHandle,
			       pPopupLayout);
	Evas_Object *pButton =
		elm_button_add(g_pFolderView->pRemovePopUpHandle);
	elm_object_style_set(pButton, "popup_button/default");
	elm_object_domain_translatable_text_set(pButton, VIDEOS_STRING,
						VIDEOS_COMMON_BUTTON_CANCEL_IDS);
	elm_object_part_content_set(g_pFolderView->pRemovePopUpHandle,
				    "button1", pButton);
	evas_object_smart_callback_add(pButton, "clicked",
				       __mp_folder_view_button_popup_cb,
				       pUserData);

	evas_object_show(g_pFolderView->pRemovePopUpHandle);
	g_pFolderView->nCompltedDeleteItem = 0;

	MP_DEL_TIMER(g_pFolderView->pStartDelTimer);
	g_pFolderView->pStartDelTimer =
		ecore_timer_add(0.3, __mp_folder_view_start_del_timer_cb,
				pUserData);

}

static void __mp_folder_view_popup_delete_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	mp_option_ctrl_delete_handle();

	if (!g_pFolderView) {
		VideoLogError("g_pFolderView of list view are not existed.");
		return;
	}

	int nFolderItemIndex = (int) pUserData;
	MpListViewAsType type = MP_LIST_VIEW_AS_FOLDER_LIST;
	mp_util_remove_question_popup(nFolderItemIndex, type,
				      __mp_folder_view_respones_delete_popup_cb,
				      __mp_folder_view_delete_popup_cb,
				      __mp_folder_view_mouse_back_up_popup_cb,
				      __mp_folder_view_key_back_popup_cb);

	VideoLogInfo("");
}

static void __mp_folder_view_share_via_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	mp_option_ctrl_delete_handle();

	if (!g_pFolderView) {
		VideoLogError("g_pFolderView of list view are not existed.");
		return;
	}

	if (!g_pFolderView->pMainHandle) {
		VideoLogError("pMainHandle is null.");
		return;
	}

	int nFolderItemIndex = (int) pUserData;
	char *szFolderPath = NULL;
	szFolderPath = mp_util_svc_get_video_folder_url(nFolderItemIndex);
	VideoSecureLogInfo("nFolderItemIndex=%d, szFolderPath=%s",
			   nFolderItemIndex, szFolderPath);
	mp_share_view_push(g_pFolderView->pMainHandle->pNaviFrameHandle,
			   (ChangeListShareViewCbFunc) g_pFolderView->
			   fUpdateListItemUserCb, szFolderPath,
			   LIST_TAB_TYPE_PERSONAL);

	Elm_Object_Item *pSelectedItem =
		elm_genlist_selected_item_get(pObject);
	if (pSelectedItem) {
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}
}

static void __mp_folder_view_normal_result_popup()
{
	if (!g_pFolderView) {
		VideoLogError("Invalid list and object");
		return;
	}
	evas_object_data_set(g_pFolderView->pVideosGenlist, "popup", NULL);
	elm_object_scroll_freeze_pop(g_pFolderView->pVideosGenlist);
}

static void __mp_folder_view_long_press_item_cb(void *pUserData,
		Evas_Object *obj,
		void *event_info)
{

	if (!g_pFolderView || !event_info || !obj) {
		VideoLogError("Invalid list and object");
		return;
	}

	Elm_Object_Item *long_item = (Elm_Object_Item *) event_info;
	/*elm_genlist_item_selected_set(long_item, EINA_FALSE);*/
	int nFolderItemIndex = elm_genlist_item_index_get(long_item);
	if (nFolderItemIndex < 0) {
		VideoLogError("nGenlistItemIndex is invalid");
		return;
	}

	st_RegisteOptionCbFunc *pRegisteCbFunc =
		(st_RegisteOptionCbFunc *) calloc(1,
				sizeof(st_RegisteOptionCbFunc));
	if (pRegisteCbFunc) {
		pRegisteCbFunc->ShareViaItemCb =
			(void *) __mp_folder_view_share_via_cb;
		pRegisteCbFunc->AddToHomeItemCb = NULL;
		/*pRegisteCbFunc->DetailsItemCb= NULL;*/
		pRegisteCbFunc->DeleteItemCb =
			(void *) __mp_folder_view_popup_delete_cb;
	}

	VideoLogDebug("nVideoItemIndex=%d", nFolderItemIndex);
	char *title = mp_util_svc_get_video_folder_name(nFolderItemIndex);
	mp_option_ctrl_show(g_pFolderView->pMainHandle->pNaviFrameHandle,
			    (const char *) title, nFolderItemIndex,
			    pRegisteCbFunc,
			    __mp_folder_view_normal_result_popup, NULL,
			    MP_LIST_OPTION_DOWNLOAD_NONE);
	if (mp_option_ctrl_get_handle()) {
		evas_object_data_set(g_pFolderView->pVideosGenlist, "popup",
				     mp_option_ctrl_get_handle());
		elm_object_scroll_freeze_push(g_pFolderView->pVideosGenlist);
	}
	MP_FREE_STRING(pRegisteCbFunc);
	MP_FREE_STRING(title);

}

static void __mp_folder_view_del_genlist_cb(Evas_Object *pVideosGenlist)
{
	if (!pVideosGenlist) {
		VideoSecureLogError("pVideosGenlist is NULL");
		return;
	}
	evas_object_smart_callback_del(pVideosGenlist, "longpressed",
				       __mp_folder_view_long_press_item_cb);
}

static void __mp_folder_view_add_genlist_cb(Evas_Object *pVideosGenlist)
{
	if (!pVideosGenlist) {
		VideoSecureLogError("pVideosGenlist is NULL");
		return;
	}

	evas_object_smart_callback_add(pVideosGenlist, "longpressed",
				       __mp_folder_view_long_press_item_cb,
				       NULL);

}
#endif

/* External APIs*/
static void __mp_folder_view_rotate_cb(void *data, Evas_Object *obj,
				       void *event_info)
{
	if (!g_pFolderView) {
		VideoLogError("__mp_folder_view_rotate_cb IS null");
		return;
	}
	if (!mp_folder_view_is_top_view())
		return;
	bool bLandscape =
		mp_rotate_ctrl_check_landspace_by_win((Evas_Object *)
				mp_util_get_main_window_handle
				());
	if (bLandscape) {
		VideoLogInfo("landscape mode");
		g_pFolderView->bLandscape = true;
	} else {
		g_pFolderView->bLandscape = false;
	}
	elm_genlist_clear(g_pFolderView->pVideosGenlist);
	__mp_folder_view_append_grid_layout(g_pFolderView->pVideosGenlist);
}

void mp_folder_view_arrange_folder_list(void *pMainViewHandle,
					void *pMainViewWidget,
					MpListUpdateType eUpdateType,
					UpdateListItemsCbFunc
					pUpdateVideoListUserCb)
{
	if (!pMainViewHandle || !pMainViewWidget || !pUpdateVideoListUserCb) {
		VideoLogInfo
		("pMainViewHandle or pMainViewHandle or pUpdateVideoListUserCb are not existed.");
		return;
	}
	if (eUpdateType == LIST_UPDATE_TYPE_NONE) {
		VideoLogInfo("don't need to update and create");
		return;
	}

	__mp_folder_view_reset(eUpdateType);

	if (!g_pFolderView) {
		g_pFolderView = (stFolderView *) calloc(1, sizeof(stFolderView));
	}

	g_pFolderView->bItemSelected = false;
	g_pFolderView->pMainHandle = pMainViewHandle;
	g_pFolderView->pMainWidget = pMainViewWidget;
	g_pFolderView->fUpdateListItemUserCb = pUpdateVideoListUserCb;
	g_pFolderView->bLandscape =
		mp_rotate_ctrl_check_landspace_by_win((Evas_Object *)
				mp_util_get_main_window_handle
				());

	elm_object_item_part_text_set(g_pFolderView->pMainHandle->
				      pNaviFrameItem, "elm.text.title",
				      VIDEOS_TITLE_OPT_FOLDER);

	VideoLogInfo("");

	int nFolderListSize = 0;

	if (eUpdateType == LIST_UPDATE_TYPE_ALL
			|| eUpdateType == LIST_UPDATE_TYPE_DATA) {
		mp_util_svc_destory_video_list();
		mp_util_svc_extract_video_list_by_item_type
		(mp_sort_ctrl_get_sort_state(), mp_view_ctrl_get_list_type());
		mp_util_svc_extract_video_folder_list_by_type
		(mp_sort_ctrl_get_sort_state(), mp_view_ctrl_get_list_type());
	}

	nFolderListSize = mp_util_svc_get_video_folder_size();
	VideoLogInfo("nVideoListSize : %d", nFolderListSize);
	/*mp_util_svc_set_update_db_cb_func(g_pFolderView->fUpdateListItemUserCb);*/

	if (nFolderListSize > 0) {
		if (g_pFolderView->pNocontentlayout) {
			elm_object_part_content_unset(g_pFolderView->pNocontentlayout,
						      SWALLOW_LISTVIEW_CONTENT);
			MP_DEL_OBJ(g_pFolderView->pNocontentlayout);
		}
		if (!g_pFolderView->pVideosGenlist) {
			g_pFolderView->pVideosGenlist =
				elm_genlist_add(g_pFolderView->pMainHandle->
						pNaviFrameHandle);
			elm_scroller_bounce_set(g_pFolderView->pVideosGenlist,
						EINA_FALSE, EINA_TRUE);
			evas_object_size_hint_weight_set(g_pFolderView->
							 pVideosGenlist,
							 EVAS_HINT_EXPAND,
							 EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(g_pFolderView->pVideosGenlist,
							EVAS_HINT_FILL,
							EVAS_HINT_FILL);
			elm_genlist_mode_set(g_pFolderView->pVideosGenlist,
					     ELM_LIST_COMPRESS);
			elm_genlist_homogeneous_set(g_pFolderView->pVideosGenlist,
						    EINA_TRUE);

			elm_genlist_block_count_set(g_pFolderView->pVideosGenlist,
						    VIDEO_MAIN_LIST_BLOCK_COUNT);
			elm_scroller_single_direction_set(g_pFolderView->
							  pVideosGenlist,
							  ELM_SCROLLER_SINGLE_DIRECTION_HARD);
#ifdef ENABLE_LONGPRESS
			__mp_folder_view_del_genlist_cb(g_pFolderView->
							pVideosGenlist);
			__mp_folder_view_add_genlist_cb(g_pFolderView->
							pVideosGenlist);
#endif
		}
		elm_object_part_content_set(g_pFolderView->pMainWidget->
					    pListViewBaselayout,
					    SWALLOW_LISTVIEW_CONTENT,
					    g_pFolderView->pVideosGenlist);
		/*mp_folder_view_append_folder_items(g_pFolderView->pVideosGenlist);*/
		__mp_folder_view_append_grid_layout(g_pFolderView->
						    pVideosGenlist);
		evas_object_smart_callback_add((Evas_Object *)
					       mp_util_get_main_window_handle(),
					       "wm,rotation,changed",
					       __mp_folder_view_rotate_cb, NULL);
		evas_object_show(g_pFolderView->pVideosGenlist);
	} else {
		__mp_folder_view_create_no_content();
	}

	mp_list_view_update_count_layout();

}

void mp_folder_view_update_video_items(void)
{
	if (!g_pFolderView->pVideosGenlist) {
		VideoLogWarning
		("[WARNING] g_pFolderView->pFolderItemList is not existed.");
		return;
	}

	VideoLogInfo("");

	int nFolderListSize = mp_util_svc_get_video_folder_size();
	Elm_Object_Item *pTmpVideoGenlistItem = NULL;
	if (nFolderListSize > 0) {

		pTmpVideoGenlistItem =
			elm_genlist_first_item_get(g_pFolderView->pVideosGenlist);
		while (pTmpVideoGenlistItem) {
			elm_genlist_item_update(pTmpVideoGenlistItem);
			pTmpVideoGenlistItem =
				elm_genlist_item_next_get(pTmpVideoGenlistItem);
		}
	}
}

void mp_folder_view_change_language(void)
{
	if (!g_pFolderView) {
		VideoLogInfo("No existed handle of Folder view.");
		return;
	}

	VideoLogInfo("");

	mp_folder_view_update_video_items();
}
