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
#include "mp-video-list-sort-ctrl.h"
#include "mp-video-list-share-via-view.h"
#include "mp-video-list-folder-share-via-view.h"
#include "mp-video-view-popup-ctrl.h"
#include "mp-video-list-view-as-ctrl.h"
#include "mp-util-widget-ctrl.h"
#include "mp-video-util-db-controller.h"
#include "vp-file-util.h"

typedef struct {
	void *pNaviFrameHandle;
	void *pNaviFrameItem;
	void *pGenlist;
	Elm_Genlist_Item_Class *pGenListItc;
	MpMediaSvcSortType nSortType;
	MpDbUpdateViewLev euLev;
	ChangeListFolderShareViewCbFunc pChangeViewUserCbFunc;
	MpVideoListTabType eTabType;
} st_FolderListShareViaViewHandle;

typedef struct {
	Evas_Object *pBox;
	Evas_Object *pBaselayout;
} st_FolderListShareViaViewWidget;

static st_FolderListShareViaViewHandle *g_pFolderShareHandle = NULL;
static st_FolderListShareViaViewWidget *g_pFolderShareWidget = NULL;


/* Pre define function*/
int mp_folder_share_view_get_sort_type(void);
bool __mp_folder_share_view_arrange_video_list(void *pGenlist,
		bool bNeedSvc);
void mp_folder_share_view_naviframe_transition_effect_cb(void *pUserData,
		Evas_Object *
		pObject,
		void
		*pEventInfo);
static void __mp_folder_share_view_base_layout_del_cb(void *pUserData,
		Evas *e,
		Evas_Object *
		pObject,
		void *pEventInfo);

void mp_folder_share_view_destroy(void)
{
	VideoLogInfo("");

	if (g_pFolderShareHandle) {
		evas_object_smart_callback_del(g_pFolderShareHandle->
					       pNaviFrameHandle,
					       "transition,finished",
					       mp_folder_share_view_naviframe_transition_effect_cb);

		MP_DEL_OBJ(g_pFolderShareHandle->pGenlist);
		MP_DEL_ITC(g_pFolderShareHandle->pGenListItc);
		MP_FREE_STRING(g_pFolderShareHandle);
	}

	if (g_pFolderShareWidget) {
		MP_DEL_OBJ(g_pFolderShareWidget->pBox);

		if (g_pFolderShareWidget->pBaselayout != NULL) {
			evas_object_event_callback_del(g_pFolderShareWidget->
						       pBaselayout, EVAS_CALLBACK_DEL,
						       __mp_folder_share_view_base_layout_del_cb);
			MP_DEL_OBJ(g_pFolderShareWidget->pBaselayout);
		}
		MP_FREE_STRING(g_pFolderShareWidget);
	}
}

static void __mp_folder_share_view_reset(void)
{
	VideoLogInfo("");

	if (g_pFolderShareHandle) {
		mp_util_db_set_update_fun(g_pFolderShareHandle->euLev, NULL);
		mp_util_db_set_backup_fun(g_pFolderShareHandle->euLev, NULL);
		evas_object_smart_callback_del(g_pFolderShareHandle->
					       pNaviFrameHandle,
					       "transition,finished",
					       mp_folder_share_view_naviframe_transition_effect_cb);
		g_pFolderShareHandle->pGenlist = NULL;
		MP_FREE_STRING(g_pFolderShareHandle);
	}

	if (g_pFolderShareWidget) {

		g_pFolderShareWidget->pBox = NULL;
		g_pFolderShareWidget->pBaselayout = NULL;

		MP_FREE_STRING(g_pFolderShareWidget);
	}
}

void mp_folder_share_view_reset_value(void)
{
	VideoLogInfo("");

	if (!g_pFolderShareHandle) {
		VideoLogError("[ERR] g_pFolderShareHandle is not existed.");
		return;
	}

	g_pFolderShareHandle->pNaviFrameHandle = NULL;
	g_pFolderShareHandle->pNaviFrameItem = NULL;
	g_pFolderShareHandle->pChangeViewUserCbFunc = NULL;
	g_pFolderShareHandle->nSortType = MP_MEDIA_SORT_BY_RECENTLY_VIEWED;
}

static void __mp_folder_share_view_trans_finished_cb(void *pUserData,
		Evas_Object *obj,
		void *event_info)
{
	if (!obj) {
		VideoLogError("[ERR] g_pFolderShareHandle is NULL.");
		return;
	}

	mp_widget_ctrl_enable_navi_handle_focus(obj);

	evas_object_smart_callback_del(obj, "transition,finished",
				       __mp_folder_share_view_trans_finished_cb);
}

static Eina_Bool mp_folder_share_view_back_btn_cb(void *pUserData,
		Elm_Object_Item *pItem)
{
	if (!g_pFolderShareHandle) {
		VideoLogInfo("[ERR] g_pFolderShareHandle is NULL.");
		return EINA_TRUE;
	}
	/* Register transition finished callback.*/
	mp_widget_ctrl_disable_navi_handle_focus_except_item
	(g_pFolderShareHandle->pNaviFrameHandle,
	 g_pFolderShareHandle->pNaviFrameItem);
	evas_object_smart_callback_add(g_pFolderShareHandle->pNaviFrameHandle,
				       "transition,finished",
				       __mp_folder_share_view_trans_finished_cb,
				       NULL);

	mp_folder_share_view_pop();

	return EINA_TRUE;
}

void mp_folder_share_view_naviframe_transition_effect_cb(void *pUserData,
		Evas_Object *
		pObject,
		void *pEventInfo)
{
	if (!g_pFolderShareHandle) {
		VideoLogError("[ERR] No exist g_pFolderShareHandle.");
		return;
	}

	VideoLogInfo("");

	Evas_Object *pTmpContents = NULL;
	Elm_Object_Item *pTmpItem = NULL;

	if (!g_pFolderShareHandle->pNaviFrameHandle) {
		VideoLogInfo("[ERR] No exist naviframe handle.");
		return;
	}

	pTmpItem =
		elm_naviframe_top_item_get(g_pFolderShareHandle->
					   pNaviFrameHandle);
	pTmpContents = elm_object_item_content_get(pTmpItem);
	if (pTmpContents) {
		if (pTmpContents == g_pFolderShareWidget->pBox) {
		} else {
			VideoLogInfo
			("It's not pListShareViaViewTabbarLayout in top screen.");
		}
	} else {
		VideoLogError("[ERR] No exist naviframe top item.");
	}
}

static void __mp_folder_share_view_result_view_update_cb(void)
{
	if (!g_pFolderShareHandle) {
		VideoLogInfo("g_pFolderShareHandle is null.");
		return;
	}

	mp_util_svc_destory_video_list();
	mp_util_svc_extract_video_list_by_item_type
	(mp_sort_ctrl_get_sort_state(), mp_view_ctrl_get_list_type());

	/*refresh folder view */
	mp_util_svc_extract_video_folder_list_by_type
	(mp_sort_ctrl_get_sort_state(), MP_LIST_VIEW_ALL);
	__mp_folder_share_view_arrange_video_list(g_pFolderShareHandle->
			pGenlist, FALSE);

	VideoLogInfo("");
}

void mp_folder_share_view_select_item_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	if (!g_pFolderShareHandle->pGenlist) {
		VideoLogError("No exist genlist object.");
		return;
	}

	int nFolderItemIndex = (int) pUserData;
	char *szFolderPath = NULL;
	szFolderPath = mp_util_svc_get_video_folder_url(nFolderItemIndex);
	VideoSecureLogInfo("nFolderItemIndex=%d, szFolderPath=%s",
			   nFolderItemIndex, szFolderPath);
	mp_share_view_push(g_pFolderShareHandle->pNaviFrameHandle,
			   __mp_folder_share_view_result_view_update_cb,
			   szFolderPath, LIST_TAB_TYPE_PERSONAL);

	Elm_Object_Item *pSelectedItem =
		elm_genlist_selected_item_get(pObject);
	if (pSelectedItem) {
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}
	MP_FREE_STRING(szFolderPath);
}

static char *__mp_folder_share_view_get_label_cb(const void *pUserData,
		Evas_Object *pObject,
		const char *pPart)
{
	int nFolderItemIndex = (int) pUserData;

	if (!strcmp(pPart, "elm.text.main.left.top")) {
		char *szTitle = NULL;
		szTitle = mp_util_svc_get_video_folder_name(nFolderItemIndex);

		char *szTitleUtf8 = elm_entry_utf8_to_markup(szTitle);

		MP_FREE_STRING(szTitle);
		return szTitleUtf8;
	} else if (!strcmp(pPart, "elm.text.sub.left.bottom")) {
		char *szFolderPath = NULL;
		szFolderPath = mp_util_svc_get_video_folder_url(nFolderItemIndex);
		char *pDes_path = mp_util_get_dir_by_path(szFolderPath);
		MP_FREE_STRING(szFolderPath);
		char *szFolderUtf8 = elm_entry_utf8_to_markup(pDes_path);
		MP_FREE_STRING(pDes_path);
		return szFolderUtf8;
	} else {
	}

	return NULL;
}

static Evas_Object *__mp_folder_share_view_get_icon_cb(const void
		*pUserData,
		Evas_Object *
		pObject,
		const char *pPart)
{
	int nFolderItemIndex = (int) pUserData;
	if (!g_strcmp0(pPart, "elm.icon.1")) {
		VideoLogInfo("pPart: elm.icon.1 - thumbnail");

		Evas_Object *pLayout = NULL;
		Evas_Object *pBg = NULL;
		char *pThumbIconUri =
			mp_util_get_folder_thumbnail(nFolderItemIndex,
						     mp_sort_ctrl_get_sort_state());

		pLayout = elm_layout_add(pObject);
		elm_layout_file_set(pLayout, VIDEO_CUSTOM_THEME,
				    "listview.thumbnail.layout");

		if (!pThumbIconUri || !vp_file_exists(pThumbIconUri)) {
			MP_FREE_STRING(pThumbIconUri);
		}

		pBg =
			mp_util_create_preload_image(pObject, pThumbIconUri,
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
	}

	return NULL;
}

int mp_folder_share_view_get_sort_type(void)
{

	switch (mp_sort_ctrl_get_sort_state()) {
	/*case MP_LIST_SORT_BY_RECENTLY_VIEWED:
	   g_pFolderShareHandle->nSortType = MP_MEDIA_SORT_BY_RECENTLY_VIEWED;
	   break; */

	case MP_LIST_SORT_BY_RECENTLY_ADDED:
		g_pFolderShareHandle->nSortType = MP_MEDIA_SORT_BY_RECENTLY_ADDED;
		break;

	case MP_LIST_SORT_BY_NAME:
		g_pFolderShareHandle->nSortType = MP_MEDIA_SORT_BY_NAME;
		break;

		/*case MP_LIST_SORT_BY_SIZE:
		   g_pFolderShareHandle->nSortType = MP_MEDIA_SORT_BY_SIZE;
		   break;

		   case MP_LIST_SORT_BY_TYPE:
		   g_pFolderShareHandle->nSortType = MP_MEDIA_SORT_BY_TYPE;
		   break; */
	}

	return g_pFolderShareHandle->nSortType;
}

static void __mp_folder_share_view_append_folder_items(void *pGenlist)
{
	VideoLogInfo("");
	if (!g_pFolderShareHandle) {
		VideoLogError("No exist g_pFolderShareHandle.");
		return;
	}

	int nIndex = 0;
	int nFolderListSize = mp_util_svc_get_video_folder_size();
	MP_DEL_ITC(g_pFolderShareHandle->pGenListItc);
	g_pFolderShareHandle->pGenListItc = elm_genlist_item_class_new();
	g_pFolderShareHandle->pGenListItc->item_style = "2line.top";
	g_pFolderShareHandle->pGenListItc->func.text_get =
		(void *) __mp_folder_share_view_get_label_cb;
	g_pFolderShareHandle->pGenListItc->func.content_get =
		(void *) __mp_folder_share_view_get_icon_cb;
	g_pFolderShareHandle->pGenListItc->func.state_get = NULL;
	g_pFolderShareHandle->pGenListItc->func.del = NULL;

	for (nIndex = 0; nIndex < nFolderListSize; nIndex++) {
		if (mp_util_svc_get_folder_storage(nIndex) !=
				MP_MEDIA_TYPE_STORAGE_DROPBOX) {
			elm_genlist_item_append(pGenlist,
						g_pFolderShareHandle->pGenListItc,
						(void *) nIndex, NULL,
						ELM_GENLIST_ITEM_NONE,
						mp_folder_share_view_select_item_cb,
						(void *) nIndex);
		}
	}
}

bool __mp_folder_share_view_arrange_video_list(void *pGenlist,
		bool bNeedSvc)
{
	if (!g_pFolderShareWidget || !g_pFolderShareHandle) {
		VideoLogError("No exist g_pShareViaListViewWidget.");
		return FALSE;
	}

	if (!pGenlist) {
		VideoLogError("No exist genlist object.");
		return FALSE;
	}

	int nVideoListSize = mp_util_svc_get_video_folder_size();
	mp_util_svc_destory_video_item_list();
	mp_util_svc_extract_video_list_by_item_type
	(mp_folder_share_view_get_sort_type(), MP_LIST_VIEW_PHONE);
#ifdef ENABLE_DRM_FEATURE
	mp_util_svc_filter_drm_video_item_list();
#endif
	VideoLogInfo("nVideoListSize : %d", nVideoListSize);
	if (nVideoListSize > 0) {
		elm_genlist_clear(pGenlist);
		__mp_folder_share_view_append_folder_items(pGenlist);
	} else {
		elm_naviframe_item_pop(g_pFolderShareHandle->pNaviFrameHandle);
		return FALSE;
	}

	evas_object_show(pGenlist);
	return TRUE;
}

Evas_Object *mp_folder_share_view_create_internal_layout(void *pParent)
{
	if (!pParent) {
		VideoLogInfo("[ERR]");
		return NULL;
	}

	VideoLogInfo("");

	if (g_pFolderShareWidget->pBaselayout) {
		evas_object_del(g_pFolderShareWidget->pBaselayout);
		g_pFolderShareWidget->pBaselayout = NULL;
	}

	g_pFolderShareWidget->pBaselayout = elm_layout_add(pParent);
	elm_layout_file_set(g_pFolderShareWidget->pBaselayout,
			    VIDEO_PLAYER_REMOVE_LIST_EDJ,
			    REMOVE_LIST_EDJ_GROUP);
	evas_object_size_hint_weight_set(g_pFolderShareWidget->pBaselayout,
					 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pFolderShareWidget->pBaselayout,
					EVAS_HINT_FILL, EVAS_HINT_FILL);

	return g_pFolderShareWidget->pBaselayout;
}

static void __mp_folder_share_view_db_changed_cb(void *pUserData)
{
	VideoLogWarning("Video DB has been changed.");
	if (!g_pFolderShareHandle || !g_pFolderShareWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	bool bNormalShow = TRUE;
	bNormalShow =
		__mp_folder_share_view_arrange_video_list(g_pFolderShareHandle->
				pGenlist, TRUE);
	if (!bNormalShow) {
		VideoLogError("invalid view");
		return;
	}

	Elm_Object_Item *pItem = NULL;
	pItem = elm_genlist_first_item_get(g_pFolderShareHandle->pGenlist);
	if (pItem) {
		elm_genlist_item_bring_in(pItem, ELM_GENLIST_ITEM_SCROLLTO_IN);
	}
}

void mp_folder_share_view_init(void *pParent)
{
	if (!g_pFolderShareHandle || !g_pFolderShareWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	VideoLogInfo("");
	g_pFolderShareWidget->pBaselayout =
		mp_folder_share_view_create_internal_layout(pParent);
	if (!g_pFolderShareWidget->pBaselayout) {
		VideoLogError("pBaselayout is not existed.");
		return;
	}
	evas_object_event_callback_add(g_pFolderShareWidget->pBaselayout,
				       EVAS_CALLBACK_DEL,
				       __mp_folder_share_view_base_layout_del_cb,
				       NULL);

	g_pFolderShareWidget->pBox =
		elm_box_add(g_pFolderShareWidget->pBaselayout);
	if (!g_pFolderShareWidget->pBox) {
		VideoLogError("pBox is not existed.");
		return;
	}

	g_pFolderShareHandle->pGenlist =
		elm_genlist_add(g_pFolderShareWidget->pBox);
	if (!g_pFolderShareHandle->pGenlist) {
		VideoLogError("pGenlist is not existed.");
		return;
	}
	evas_object_size_hint_weight_set(g_pFolderShareHandle->pGenlist,
					 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pFolderShareHandle->pGenlist,
					EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_genlist_block_count_set(g_pFolderShareHandle->pGenlist,
				    VIDEO_GENLIST_BLOCK_COUNT);
	elm_genlist_mode_set(g_pFolderShareHandle->pGenlist,
			     ELM_LIST_COMPRESS);
	elm_genlist_homogeneous_set(g_pFolderShareHandle->pGenlist,
				    EINA_TRUE);

	elm_box_pack_end(g_pFolderShareWidget->pBox,
			 g_pFolderShareHandle->pGenlist);
	elm_object_part_content_set(g_pFolderShareWidget->pBaselayout,
				    SWALLOW_LISTVIEW_CONTENT,
				    g_pFolderShareWidget->pBox);

	bool bNormalShow = TRUE;
	bNormalShow =
		__mp_folder_share_view_arrange_video_list(g_pFolderShareHandle->
				pGenlist, FALSE);
	if (!bNormalShow) {
		VideoLogError("invalid view");
		return;
	}

	Evas_Object *back_btn = elm_button_add(pParent);
	elm_object_style_set(back_btn, "naviframe/end_btn/default");
	evas_object_show(back_btn);
	g_pFolderShareHandle->pNaviFrameItem =
		elm_naviframe_item_push(g_pFolderShareHandle->pNaviFrameHandle,
					VIDEOS_SELECTVIEW_HEADER_SELECT, back_btn,
					NULL, g_pFolderShareWidget->pBaselayout,
					NULL);
	elm_object_item_domain_text_translatable_set(g_pFolderShareHandle->
			pNaviFrameItem,
			VIDEOS_STRING,
			EINA_TRUE);
	elm_naviframe_item_pop_cb_set(g_pFolderShareHandle->pNaviFrameItem,
				      mp_folder_share_view_back_btn_cb,
				      g_pFolderShareHandle);


	elm_naviframe_item_title_enabled_set(g_pFolderShareHandle->
					     pNaviFrameItem, EINA_TRUE,
					     EINA_FALSE);
	evas_object_smart_callback_add(g_pFolderShareHandle->pNaviFrameHandle,
				       "transition,finished",
				       mp_folder_share_view_naviframe_transition_effect_cb,
				       NULL);

	evas_object_show(g_pFolderShareHandle->pGenlist);
	evas_object_show(g_pFolderShareWidget->pBaselayout);

	mp_util_db_set_update_fun(g_pFolderShareHandle->euLev,
				  __mp_folder_share_view_db_changed_cb);

	mp_util_hide_indicator();
}

void mp_folder_share_view_push(void *pNaviFrame,
			       ChangeListFolderShareViewCbFunc
			       pChangeViewCb, MpVideoListTabType eTabType)
{
	VideoLogInfo("");
	if (g_pFolderShareHandle || g_pFolderShareWidget) {
		mp_folder_share_view_destroy();
	}

	g_pFolderShareHandle =
		(st_FolderListShareViaViewHandle *) calloc(1,
				sizeof
				(st_FolderListShareViaViewHandle));
	g_pFolderShareWidget =
		(st_FolderListShareViaViewWidget *) calloc(1,
				sizeof
				(st_FolderListShareViaViewWidget));

	memset(g_pFolderShareHandle, 0,
	       sizeof(st_FolderListShareViaViewHandle));
	memset(g_pFolderShareWidget, 0,
	       sizeof(st_FolderListShareViaViewWidget));
	mp_folder_share_view_reset_value();

	g_pFolderShareHandle->pNaviFrameHandle = pNaviFrame;
	g_pFolderShareHandle->pChangeViewUserCbFunc = pChangeViewCb;
	g_pFolderShareHandle->eTabType = eTabType;
	g_pFolderShareHandle->euLev = MP_DB_UPDATE_LEV_1;

	mp_folder_share_view_init(g_pFolderShareHandle->pNaviFrameHandle);
}

void mp_folder_share_view_pop(void)
{
	VideoLogInfo("");

	if (!g_pFolderShareHandle) {
		VideoLogError("No existed handle of share view.");
		return;
	}
	if (g_pFolderShareHandle->pChangeViewUserCbFunc) {
		g_pFolderShareHandle->pChangeViewUserCbFunc();
	}

	mp_util_hide_indicator();
}

void mp_folder_share_view_change_language(void)
{
	if (!g_pFolderShareHandle || !g_pFolderShareWidget) {
		VideoLogError("No existed handle of share view.");
		return;
	}

	VideoLogInfo("");

	elm_object_item_domain_translatable_part_text_set
	(g_pFolderShareHandle->pNaviFrameItem, "elm.text.title",
	 VIDEOS_STRING, VIDEOS_SELECTVIEW_HEADER_SELECT_IDS);
}

static void __mp_folder_share_view_base_layout_del_cb(void *pUserData,
		Evas *e,
		Evas_Object *
		pObject,
		void *pEventInfo)
{
	__mp_folder_share_view_reset();
}
