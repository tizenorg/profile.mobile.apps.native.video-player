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
#include <media_content.h>


#include "mp-util.h"
#include "vp-util.h"
#include "mp-video-log.h"
#include "mp-external-ug.h"
#include "mp-video-value-define.h"
#include "mp-video-string-define.h"
#include "mp-util-media-service.h"

#include "mp-footer-toolbar.h"
#include "mp-video-list-view-main.h"
#include "mp-video-list-sort-ctrl.h"
#include "mp-video-view-popup-ctrl.h"
#include "mp-video-list-remove-ctrl.h"
#include "mp-video-list-remove-view.h"
#include "mp-video-search-view.h"
#include "mp-video-list-view-as-ctrl.h"
#include "mp-launching-video-displayer.h"
#include "mp-video-list-view-normal.h"
#include "mp-video-info-ctrl.h"
#include "mp-rotate-ctrl.h"
#include "mp-video-list-view-thumbnail.h"
#include "mp-video-list-view-item-of-folder.h"
#include "mp-util-widget-ctrl.h"

#include "videos-view-mgr.h"
#include "mp-video-list-view-folder.h"
#include "mp-video-list-share-via-view.h"
#include "mp-video-list-view-common.h"
#include "mp-video-list-option-ctrl.h"
#include "mp-video-list-personal-view.h"
#include "mp-video-nocontent-layout.h"

#include "mp-video-rename-ctrl.h"
#include "mp-video-util-db-controller.h"
#include "mp-video-list-view-select.h"
#include "mp-video-list-folder-share-via-view.h"

#include "mp-util-preference.h"
#include "mp-util-config.h"


static st_VideoListViewMainViewHandle *g_pMainViewHandle = NULL;
static st_VideoListViewMainViewWidget *g_pMainViewWidget = NULL;


/*///////////////////////////////////////////////////*/
/* Pre-define function for building.*/
static void __mp_list_view_update_list(int nListTabType,
				       MpListUpdateType eUpdateType);
/*static void mp_list_view_check_center_button_of_toolbar(void);*/
static void mp_list_view_check_more_button_of_toolbar(void);

static bool __mp_list_view_is_top_view(void);

/*use used for poping view situation,update data list-->reset DB callback*/
static void __mp_list_view_result_view_update_cb(void);

/*use used for popup situation,update data list-->update videos items*/
static void __mp_list_view_result_data_update_cb(void);

/*use used for whole update,  clear videos items -->update data list-->append video items*/
static void __mp_list_view_result_user_cb(MpListUpdateType eUpdateType);

/*use used for personal storage style -->update data list-->update video items*/
static void __mp_list_view_result_media_data_update_cb(void);

static char *__mp_list_view_count_layout_tts_cb(void *data,
		Evas_Object *obj);


static void __mp_list_view_recently_change_cb(const char *key, void *data)
{
	VideoLogInfo("");
	char *pszVideo = mp_util_config_get_preview();
	if (!pszVideo) {
		return;
	}
	MP_FREE_STRING(pszVideo);

	mp_list_view_update_widget();
}

static void __mp_list_view_register_recently_config()
{
	mp_util_config_ignore_preview_key_changed_cb();
	mp_util_config_preview_key_changed_cb(__mp_list_view_recently_change_cb, NULL);
}

void mp_list_view_free_main_view_widget(void)
{
	VideoLogInfo("");

	if (g_pMainViewWidget) {
		if (g_pMainViewWidget->pToolBar) {
			evas_object_del(g_pMainViewWidget->pToolBar);
			g_pMainViewWidget->pToolBar = NULL;
		}

		if (g_pMainViewWidget->pListViewBaselayout) {
			evas_object_del(g_pMainViewWidget->pListViewBaselayout);
			g_pMainViewWidget->pListViewBaselayout = NULL;
		}

		if (g_pMainViewWidget->pListViewTitlelayout) {
			evas_object_del(g_pMainViewWidget->pListViewTitlelayout);
			g_pMainViewWidget->pListViewTitlelayout = NULL;
		}

		if (g_pMainViewWidget->pBox) {
			evas_object_del(g_pMainViewWidget->pBox);
			g_pMainViewWidget->pBox = NULL;
		}
		/*
				if (g_pMainViewWidget->pSearchBtn)
				{
					evas_object_del(g_pMainViewWidget->pSearchBtn);
					g_pMainViewWidget->pSearchBtn = NULL;
				}
		*/
		free(g_pMainViewWidget);
		g_pMainViewWidget = NULL;
	}
}


/*//////////////////////////////////////////////////*/
/* Callback function of more of footer widget.*/

void mp_list_view_delete_cb(void *pUserData, Evas_Object *pObject,
			    void *pEventInfo)
{
	if (!g_pMainViewHandle) {
		VideoLogWarning
		("[WARNING] Main handles of list view are not existed.");
		return;
	}

	mp_ft_ctrl_hide_more_popup();

	MpFooterLibraryMainListTabType footerTabType =
		(MpFooterLibraryMainListTabType) pUserData;
	if (FOOTER_TAB_TYPE_PERSONAL == footerTabType) {
		mp_remove_view_push((void *) mp_mgr_get_library_naviframe(),
				    (void *) __mp_list_view_result_user_cb,
				    (void *) __mp_list_view_result_view_update_cb,
				    NULL, LIST_TAB_TYPE_PERSONAL);
	} else {
		VideoLogError("invalid footer type");
	}
}

void mp_list_view_sort_item_cb(void *pUserData, Evas_Object *pObject,
			       void *pEventInfo)
{

	mp_ft_ctrl_hide_more_popup();

	MpFooterLibraryMainListTabType footerTabType =
		(MpFooterLibraryMainListTabType) pUserData;
	if (FOOTER_TAB_TYPE_PERSONAL == footerTabType) {
		mp_sort_ctrl_show((void *) __mp_list_view_result_data_update_cb);
	} else {
		VideoLogError("invalid footer type");
	}
}

static void __mp_list_view_view_as_result_user_cb(void)
{
	if (!g_pMainViewHandle) {
		VideoLogWarning("g_pMainViewHandle == NULL");
		return;
	}

	int nPreViewType = g_pMainViewHandle->nCurrentViewType;
	g_pMainViewHandle->nCurrentViewType = mp_view_as_ctrl_get_type();

	if (g_pMainViewHandle->nCurrentViewType != nPreViewType) {

		elm_object_part_content_unset(g_pMainViewWidget->
					      pListViewBaselayout,
					      SWALLOW_LISTVIEW_CONTENT);
#ifdef VS_FEATURE_LIST_VIEW
		if (nPreViewType == MP_LIST_VIEW_AS_NORMAL_LIST) {
			mp_normal_view_destroy();
		} else
#endif
			if (nPreViewType == MP_LIST_VIEW_AS_FOLDER_LIST) {
				mp_folder_view_destroy();
			} else if (nPreViewType == MP_LIST_VIEW_AS_THUMBNAIL_LIST) {
				mp_thumbnail_view_destroy();
			} else {
				VideoLogError("invalid type");
#ifdef VS_FEATURE_LIST_VIEW
				mp_normal_view_destroy();
#endif
				mp_folder_view_destroy();
				mp_thumbnail_view_destroy();
			}
	}
	__mp_list_view_result_user_cb(MP_LIST_VIEW_ALL);
}

void mp_list_view_view_as_cb(void *pUserData, Evas_Object *pObject,
			     void *pEventInfo)
{

	mp_ft_ctrl_hide_more_popup();

	MpFooterLibraryMainListTabType footerTabType =
		(MpFooterLibraryMainListTabType) pUserData;
	if (FOOTER_TAB_TYPE_PERSONAL == footerTabType) {
		mp_view_as_ctrl_show(footerTabType,
				     (void *)
				     __mp_list_view_view_as_result_user_cb);
	} else {
		VideoLogError("invalid footer type");
	}
}

void mp_list_view_rename_cb(void *pUserData, Evas_Object *pObject,
			    void *pEventInfo)
{
	if (!g_pMainViewHandle) {
		VideoLogWarning
		("[WARNING] Main handles of list view are not existed.");
		return;
	}

	mp_ft_ctrl_hide_more_popup();
	mp_select_view_push((void *) mp_mgr_get_library_naviframe(),
			    g_pMainViewHandle->pNaviFrameItem,
			    __mp_list_view_result_user_cb, NULL,
			    SELECT_TYPE_RENAME, LIST_TAB_TYPE_PERSONAL);
}

/*void mp_list_view_details_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pMainViewHandle)
	{
		VideoLogWarning("[WARNING] Main handles of list view are not existed.");
		return;
	}

	mp_ft_ctrl_hide_more_popup();
	mp_select_view_push((void *)mp_mgr_get_library_naviframe(), g_pMainViewHandle->pNaviFrameItem, NULL, NULL, SELECT_TYPE_DETAIL, LIST_TAB_TYPE_PERSONAL);
}*/

void mp_list_view_content_to_display_cb(void *pUserData,
					Evas_Object *pObject,
					void *pEventInfo)
{
	mp_ft_ctrl_hide_more_popup();

	mp_view_ctrl_show((void *) __mp_list_view_result_user_cb);
}

void mp_list_view_sign_in_cb(void *pUserData, Evas_Object *pObject,
			     void *pEventInfo)
{

	mp_ft_ctrl_hide_more_popup();

	/*mp_sign_ctrl_request_everglades_sign_in(NULL, NULL);*/
}

void mp_list_view_share_via_cb(void *pUserData, Evas_Object *pObject,
			       void *pEventInfo)
{
	if (!g_pMainViewHandle) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	mp_ft_ctrl_hide_more_popup();

	if (mp_view_as_ctrl_get_type() == MP_LIST_VIEW_AS_FOLDER_LIST) {
		VideoLogWarning("Select folder for sharing.");
		mp_folder_share_view_push((void *) mp_mgr_get_library_naviframe(),
					  (void *)
					  __mp_list_view_result_view_update_cb,
					  LIST_TAB_TYPE_PERSONAL);
	} else {
		mp_share_view_push((void *) mp_mgr_get_library_naviframe(),
				   (void *) __mp_list_view_result_view_update_cb,
				   NULL, LIST_TAB_TYPE_PERSONAL);
	}
}

void mp_list_view_add_personal_cb(void *pUserData, Evas_Object *pObject,
				  void *pEventInfo)
{
	if (!g_pMainViewHandle) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	mp_ft_ctrl_hide_more_popup();

	mp_personal_view_push((void *) mp_mgr_get_library_naviframe(),
			      (void *)
			      __mp_list_view_result_media_data_update_cb,
			      NULL, 0);
}

void mp_list_view_remove_personal_cb(void *pUserData,
				     Evas_Object *pObject,
				     void *pEventInfo)
{
	if (!g_pMainViewHandle) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}

	mp_ft_ctrl_hide_more_popup();

	mp_personal_view_push((void *) mp_mgr_get_library_naviframe(),
			      (void *)
			      __mp_list_view_result_media_data_update_cb,
			      NULL, 1);
}


/**/
/*//////////////////////////////////////////////////*/

/*//////////////////////////////////////////////////*/
/* Footer widgets*/

void mp_list_view_check_more_button_of_toolbar(void)
{
	if (!g_pMainViewHandle) {
		VideoLogInfo("g_pMainViewHandle == NULL");
		return;
	}

	VideoLogInfo("nListTabType=%d", g_pMainViewHandle->nListTabType);

	if (g_pMainViewHandle->nListTabType == LIST_TAB_TYPE_PERSONAL) {
		if (mp_folder_item_view_is_current_view()) {
			VideoLogInfo("Update more button of folder item view");
			mp_folder_item_view_check_more_button_of_toolbar();
		} else {
			VideoLogInfo("Update more button of main list view");
			st_RegisteCbFunc *pRegisteCbFunc =
				(st_RegisteCbFunc *) calloc(1, sizeof(st_RegisteCbFunc));
			if (!pRegisteCbFunc) {
				VideoLogError("failed to allocate memory");
				return;
			}

			pRegisteCbFunc->SearchCb =
				(void *) mp_list_view_search_item_cb;
			pRegisteCbFunc->DeleteCb = (void *) mp_list_view_delete_cb;
			pRegisteCbFunc->ViewAsCb = (void *) mp_list_view_view_as_cb;
			if (mp_view_as_ctrl_get_type() != MP_LIST_VIEW_AS_FOLDER_LIST) {
				pRegisteCbFunc->SortItemCb =
					(void *) mp_list_view_sort_item_cb;
				pRegisteCbFunc->RenameCb =
					(void *) mp_list_view_rename_cb;
				/*pRegisteCbFunc->DetailsCb= (void*)mp_list_view_details_cb;*/
				pRegisteCbFunc->ShareViaCb =
					(void *) mp_list_view_share_via_cb;
			}
			pRegisteCbFunc->SignInCb = (void *) mp_list_view_sign_in_cb;

#ifdef	ENABLE_PRIVATE_MODE
			pRegisteCbFunc->AddToPersonalCB =
				(void *) mp_list_view_add_personal_cb;
			pRegisteCbFunc->RemoveFromPersonalCB =
				(void *) mp_list_view_remove_personal_cb;
#endif
			bool bShowFT = mp_ft_ctrl_is_show();
			if (bShowFT == FALSE) {
				mp_ft_ctrl_create(g_pMainViewHandle->pNaviFrameHandle,
						  g_pMainViewHandle->pNaviFrameItem,
						  g_pMainViewHandle->nListTabType,
						  pRegisteCbFunc);
			}
			free(pRegisteCbFunc);
		}
	} else {
		mp_ft_ctrl_disable();
	}
}

/**/
/*//////////////////////////////////////////////////*/

/*//////////////////////////////////////////////////*/
/* Search view*/
void mp_list_view_show_search_view(void)
{
	if (!g_pMainViewHandle) {
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}

	VideoLogInfo("");

	mp_ft_ctrl_hide_more_popup();

	mp_search_view_push((void *) mp_mgr_get_library_naviframe(), NULL);

	mp_search_view_register_update_list_func((void *)
			__mp_list_view_result_user_cb,
			(void *)
			__mp_list_view_result_view_update_cb);
}

/**/
/*//////////////////////////////////////////////////*/

/*//////////////////////////////////////////////////*/
/* Callback function of base widget in main list view.*/
void mp_list_view_naviframe_transition_effect_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	if (!g_pMainViewHandle) {
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}

	VideoLogInfo("");

	Evas_Object *pTmpContents = NULL;
	Elm_Object_Item *pTmpItem = NULL;

	if (!g_pMainViewHandle->pNaviFrameHandle) {
		VideoLogInfo("[ERR] No exist naviframe handle.");
		return;
	}

	pTmpItem =
		elm_naviframe_top_item_get(g_pMainViewHandle->pNaviFrameHandle);
	pTmpContents = elm_object_item_content_get(pTmpItem);
	if (pTmpContents) {
	} else {
		VideoLogInfo("[ERR] No exist naviframe top item.");
	}
}

void mp_list_view_double_tap_sensor_cb(void)
{
	if (!g_pMainViewHandle) {
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}

	VideoLogInfo("");

	if (!__mp_list_view_is_top_view()) {
		VideoLogInfo("Main list view is not top view.");
		return;
	}

}

static void __mp_list_view_result_media_data_update_cb(void)
{
	if (!g_pMainViewHandle || !g_pMainViewWidget) {
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}

	VideoLogInfo("");

	mp_util_svc_destory_video_list();
	mp_util_svc_extract_video_list_by_item_type
	(mp_sort_ctrl_get_sort_state(), mp_view_ctrl_get_list_type());

	int nViewType = mp_view_as_ctrl_get_type();
#ifdef VS_FEATURE_LIST_VIEW
	if (nViewType == MP_LIST_VIEW_AS_NORMAL_LIST) {
		mp_normal_view_update_video_list();
	} else
#endif
		if (nViewType == MP_LIST_VIEW_AS_FOLDER_LIST) {
			mp_folder_view_update_video_items();
			mp_util_svc_extract_video_folder_list_by_type
			(mp_sort_ctrl_get_sort_state(), MP_LIST_VIEW_ALL);
		} else {
			VideoLogError("");
		}

	mp_list_view_check_more_button_of_toolbar();
	/*mp_list_view_check_center_button_of_toolbar();*/
}

static void __mp_list_view_result_user_cb(MpListUpdateType eUpdateType)
{
	if (!g_pMainViewHandle || !g_pMainViewWidget) {
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}

	VideoLogInfo("");
	elm_object_item_part_text_set(g_pMainViewHandle->pNaviFrameItem,
				      "elm.text.title", VIDEOS_TITLE_VIDEO);

	__mp_list_view_update_list(g_pMainViewHandle->nListTabType,
				   eUpdateType);

	mp_list_view_check_more_button_of_toolbar();
	/*mp_list_view_check_center_button_of_toolbar();*/
}

static void __mp_list_view_result_data_update_cb(void)
{
	if (!g_pMainViewHandle || !g_pMainViewWidget) {
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}

	VideoLogInfo("");

	/* mp_util_svc_ctrl_sort_list(mp_sort_ctrl_get_sort_state());*/
	int nViewType = mp_view_as_ctrl_get_type();
#ifdef VS_FEATURE_LIST_VIEW
	if (nViewType == MP_LIST_VIEW_AS_NORMAL_LIST) {
		mp_util_svc_destory_video_item_list();
		mp_util_svc_extract_video_list_by_item_type
		(mp_sort_ctrl_get_sort_state(), mp_view_ctrl_get_list_type());
		mp_normal_view_update_video_list();
	} else
#endif
		if (nViewType == MP_LIST_VIEW_AS_FOLDER_LIST) {
			mp_util_svc_destory_video_folder_list();
			mp_util_svc_extract_video_folder_list_by_type
			(mp_sort_ctrl_get_sort_state(), MP_LIST_VIEW_ALL);
			mp_folder_view_update_video_items();
		} else if (nViewType == MP_LIST_VIEW_AS_THUMBNAIL_LIST) {
			mp_util_svc_destory_video_item_list();
			mp_util_svc_extract_video_list_by_item_type
			(mp_sort_ctrl_get_sort_state(), mp_view_ctrl_get_list_type());
			mp_thumbnail_view_update_video_list();
		} else {

			VideoLogError("");
		}

	mp_list_view_check_more_button_of_toolbar();
	/*mp_list_view_check_center_button_of_toolbar();*/
}

static void __mp_list_view_result_view_update_cb(void)
{
	if (!g_pMainViewHandle || !g_pMainViewWidget) {
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}
	VideoLogInfo("");
	int nViewType = mp_view_as_ctrl_get_type();
	mp_util_svc_destory_video_list();
	mp_util_svc_extract_video_list_by_item_type
	(mp_sort_ctrl_get_sort_state(), mp_view_ctrl_get_list_type());

	if (nViewType == MP_LIST_VIEW_AS_FOLDER_LIST) {
		/*refresh folder view */
		mp_util_svc_extract_video_folder_list_by_type
		(mp_sort_ctrl_get_sort_state(), MP_LIST_VIEW_ALL);
		if (!mp_folder_view_is_item_view() && !mp_search_view_get_active_view())
			mp_folder_view_arrange_folder_list(g_pMainViewHandle,
						   g_pMainViewWidget,
						   LIST_UPDATE_TYPE_VIEW,
						   (void *)
						   __mp_list_view_result_user_cb);
	}
	if (nViewType == MP_LIST_VIEW_AS_THUMBNAIL_LIST) {
		/*refresh thumbnail view */
		mp_thumbnail_view_update_video_list();
	}
	mp_list_view_check_more_button_of_toolbar();

	/*mp_list_view_check_center_button_of_toolbar();*/
}

static void __mp_list_view_db_update(int nUpdateStyle)
{

	if (!g_pMainViewHandle || !g_pMainViewWidget) {
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}

	mp_util_db_run_backup_fun_except_lev0(NULL, NULL, NULL, NULL);
	__mp_list_view_update_list(g_pMainViewHandle->nListTabType,
				   LIST_UPDATE_TYPE_ALL);

	mp_util_db_run_update_fun_except_lev0(NULL, NULL, NULL, NULL);

	g_pMainViewHandle->pDbUpdateTimer = NULL;
	VideoLogInfo("db change ----update");
}

/*pUserData = update style,  insert, delete, update or -1*/
static Eina_Bool __mp_list_view_db_update_timer_cb(void *pUserData)
{

	if (!g_pMainViewHandle || !g_pMainViewWidget) {
		VideoLogInfo("Main handles of list view are not existed.");
		return ECORE_CALLBACK_CANCEL;
	}
	if (!g_pMainViewHandle->pDbUpdateTimer) {
		return ECORE_CALLBACK_CANCEL;
	}

	int nUpdateStyle = (int) pUserData;

	__mp_list_view_db_update(nUpdateStyle);

	return ECORE_CALLBACK_CANCEL;
}

static void __mp_list_view_db_change_cb(void *pUserData, float fTime,
					int nUpdateStyle)
{
	if (!g_pMainViewHandle || !g_pMainViewWidget) {
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}

	VideoLogInfo("db change----");

	MP_DEL_TIMER(g_pMainViewHandle->pDbUpdateTimer);
	g_pMainViewHandle->pDbUpdateTimer =
		ecore_timer_add(fTime, __mp_list_view_db_update_timer_cb,
				(void *) nUpdateStyle);

}

void mp_list_view_search_item_cb(void *pUserData, Evas_Object *pObject,
				 void *pEventInfo)
{
	VideoLogInfo("");

	if (!g_pMainViewHandle || !g_pMainViewWidget) {
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}

	mp_list_view_show_search_view();
}

Eina_Bool mp_list_view_back_btn_cb(void *pUserData,
				   Elm_Object_Item *pItem)
{
	VideoLogInfo("");

	if (mp_util_get_main_window_handle()) {
		elm_win_lower(mp_util_get_main_window_handle());
	}

	return EINA_FALSE;
}

void mp_list_view_launching_video_player_cb(void *pUserData,
		void *pBuffer,
		unsigned int nBufferSize)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

}


/*//////////////////////////////////////////////////*/
/**/
void __mp_list_view_update_list(int nListTabType,
				MpListUpdateType eUpdateType)
{
	VideoLogInfo("");

	mp_option_ctrl_delete_handle();
	mp_util_delete_popup_handle();
	//mp_rename_ctrl_delete_handle();

	int nViewType = mp_view_as_ctrl_get_type();
#ifdef VS_FEATURE_LIST_VIEW
	if (nViewType == MP_LIST_VIEW_AS_NORMAL_LIST) {
		mp_normal_view_arrange_list_item(g_pMainViewHandle,
						 g_pMainViewWidget, eUpdateType,
						 (void *)
						 __mp_list_view_result_user_cb);
	} else
#endif
		if (nViewType == MP_LIST_VIEW_AS_FOLDER_LIST) {
			mp_folder_view_arrange_folder_list(g_pMainViewHandle,
							   g_pMainViewWidget, eUpdateType,
							   (void *)
							   __mp_list_view_result_user_cb);
		} else if (nViewType == MP_LIST_VIEW_AS_THUMBNAIL_LIST) {
			mp_thumbnail_view_arrange_list_item(g_pMainViewHandle,
							    g_pMainViewWidget,
							    eUpdateType,
							    (void *)
							    __mp_list_view_result_user_cb);
		}

	mp_list_view_check_more_button_of_toolbar();

	/*mp_list_view_check_center_button_of_toolbar();*/

}

char *__mp_list_view_text_get_video_number_cb(void *pUserData,
		Evas_Object *pObj,
		const char *part)
{
	VideoLogInfo("");

	if (g_pMainViewHandle == NULL) {
		VideoLogError("g_pMainViewHandle == NUL!!!");
		VideoLogAssert(0);
		return NULL;
	}

	if (g_pMainViewWidget == NULL) {
		VideoLogError("g_pMainViewWidget == NUL!!!");
		VideoLogAssert(0);
		return NULL;
	}

	if (!strcmp(part, "elm.text.sub")) {
		unsigned int nCount = 0;
		int nViewType = mp_view_as_ctrl_get_type();

		if (nViewType == MP_LIST_VIEW_AS_FOLDER_LIST) {
			nCount = mp_util_svc_get_video_folder_size();
		} else {
			nCount = mp_util_svc_get_number_of_video_item_by_type();
		}

		VideoLogInfo("nViewType : [%d], nCount : [%d]", nViewType,
			     nCount);

		if (nCount == 1) {
			if (nViewType == MP_LIST_VIEW_AS_FOLDER_LIST) {
				return g_strdup_printf("%s",
						       VIDEOS_FOLDERVIEW_BODY_ONE_FOLDER);
			} else {
				return g_strdup_printf("%s",
						       VIDEOS_LISTVIEW_BODY_ONE_VIDEO);
			}
		} else {
			if (nViewType == MP_LIST_VIEW_AS_FOLDER_LIST) {
				return g_strdup_printf(VIDEOS_FOLDERVIEW_BODY_PD_FOLDERS,
						       nCount);
			} else {
				return g_strdup_printf(VIDEOS_LISTVIEW_BODY_PD_VIDEOS,
						       nCount);
			}
		}
	}

	return NULL;
}

/**/
/*//////////////////////////////////////////////////*/

void mp_list_view_update_whole_genlist(MpListUpdateType eUpdateType)
{
	if (!g_pMainViewHandle) {
		VideoLogInfo("[ERR] g_pMainViewHandle is NULL.");
		return;
	}

	VideoLogInfo("");

	if (g_pMainViewHandle->nListTabType == LIST_TAB_TYPE_PERSONAL) {
		int nViewType = mp_view_as_ctrl_get_type();
#ifdef VS_FEATURE_LIST_VIEW
		if (nViewType == MP_LIST_VIEW_AS_NORMAL_LIST) {
			mp_normal_view_arrange_list_item(g_pMainViewHandle,
							 g_pMainViewWidget,
							 eUpdateType,
							 (void *)
							 __mp_list_view_result_user_cb);
		} else
#endif
			if (nViewType == MP_LIST_VIEW_AS_FOLDER_LIST) {
				mp_folder_view_arrange_folder_list(g_pMainViewHandle,
								   g_pMainViewWidget,
								   eUpdateType,
								   (void *)
								   __mp_list_view_result_user_cb);
				mp_folder_item_view_update_video_list(LIST_UPDATE_TYPE_NONE);	/*folder item view, update NONE*/
			}
		if (mp_personal_view_is_top_view()) {
			mp_personal_view_update();
		}
	} else {
		VideoLogInfo("It's not relevant list arrange.");
	}

	mp_ft_ctrl_rotate_update();
	/*mp_list_view_check_center_button_of_toolbar();*/
}

void mp_list_view_rotate_view(MpListUpdateType eUpdateType)
{
	if (!g_pMainViewHandle) {
		VideoLogInfo("[ERR] g_pMainViewHandle is NULL.");
		return;
	}

	VideoLogInfo("");

	mp_ft_ctrl_rotate_update();
}

static void __mp_main_title_layout_del_cb(void *pUserData, Evas *e,
		Evas_Object *pObject,
		void *pEventInfo)
{
	if (!g_pMainViewWidget) {
		VideoLogError("g_pMainViewWidget == NUL!!!");
		return;
	}

	g_pMainViewWidget->pListViewTitlelayout = NULL;
	g_pMainViewWidget->pVideoGenlistTitle = NULL;
	g_pMainViewWidget->pVideoGenlistItemForTitle = NULL;
}

static void __mp_main_video_index_item_del_cb(void *data,
		Evas_Object *obj,
		void *event_info)
{
	if (!g_pMainViewWidget) {
		VideoLogError("g_pMainViewWidget == NUL!!!");
		return;
	}
	g_pMainViewWidget->pVideoGenlistItemForTitle = NULL;
}


void mp_list_view_update_count_layout()
{
	unsigned int nCount = 0;
	int nViewType = mp_view_as_ctrl_get_type();

	if (g_pMainViewHandle == NULL) {
		VideoLogError("g_pMainViewHandle == NUL!!!");
		VideoLogAssert(0);

		return;
	}

	if (g_pMainViewWidget == NULL) {
		VideoLogError("g_pMainViewWidget == NUL!!!");
		VideoLogAssert(0);

		return;
	}

	if (nViewType == MP_LIST_VIEW_AS_FOLDER_LIST) {
		nCount = mp_util_svc_get_video_folder_size();
	} else {
		nCount = mp_util_svc_get_number_of_video_item_by_type();
	}

	VideoLogInfo("nViewType : [%d], nCount : [%d]", nViewType, nCount);

	if (nCount == 0) {
		if (g_pMainViewWidget->pListViewTitlelayout != NULL) {
			elm_box_unpack(g_pMainViewWidget->pBox,
				       g_pMainViewWidget->pListViewTitlelayout);
			evas_object_del(g_pMainViewWidget->pListViewTitlelayout);
			g_pMainViewWidget->pListViewTitlelayout = NULL;
		}
	} else {
		/* genlist - groupindex*/
		if (g_pMainViewWidget->pVideoGenlistItemForTitle != NULL) {
			elm_object_item_del(g_pMainViewWidget->
					    pVideoGenlistItemForTitle);
		}

		if (g_pMainViewWidget->pVideoGenlistTitleItc == NULL) {
			g_pMainViewWidget->pVideoGenlistTitleItc =
				elm_genlist_item_class_new();
			g_pMainViewWidget->pVideoGenlistTitleItc->item_style =
				"groupindex.sub";
			g_pMainViewWidget->pVideoGenlistTitleItc->func.text_get =
				__mp_list_view_text_get_video_number_cb;
			g_pMainViewWidget->pVideoGenlistTitleItc->func.content_get =
				NULL;
			g_pMainViewWidget->pVideoGenlistTitleItc->func.del = NULL;
		}

		if (g_pMainViewWidget->pVideoGenlistTitle == NULL) {
			g_pMainViewWidget->pVideoGenlistTitle =
				elm_genlist_add(g_pMainViewWidget->pBox);
			elm_genlist_mode_set(g_pMainViewWidget->pVideoGenlistTitle,
					     ELM_LIST_COMPRESS);
			evas_object_show(g_pMainViewWidget->pVideoGenlistTitle);
		}

		if (g_pMainViewWidget->pListViewTitlelayout == NULL) {
			Evas_Object *pTTSObject = NULL;
			char edj_path[1024] = {0};

			char *path = app_get_resource_path();
			snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_CUSTOM_THEME);

			/* create layout*/
			g_pMainViewWidget->pListViewTitlelayout =
				elm_layout_add(g_pMainViewWidget->pBox);
			evas_object_size_hint_weight_set(g_pMainViewWidget->
							 pListViewTitlelayout,
							 EVAS_HINT_EXPAND, 0);
			evas_object_size_hint_align_set(g_pMainViewWidget->
							pListViewTitlelayout,
							EVAS_HINT_FILL,
							EVAS_HINT_FILL);
			elm_layout_file_set(g_pMainViewWidget->pListViewTitlelayout,
					    edj_path,
					    "count_info_groupindex");
			evas_object_event_callback_add(g_pMainViewWidget->
						       pListViewTitlelayout,
						       EVAS_CALLBACK_DEL,
						       __mp_main_title_layout_del_cb,
						       NULL);

			evas_object_show(g_pMainViewWidget->pListViewTitlelayout);

			pTTSObject =
				elm_access_object_register(g_pMainViewWidget->
							   pListViewTitlelayout,
							   g_pMainViewWidget->pBox);
			elm_access_info_cb_set(pTTSObject, ELM_ACCESS_TYPE,
					       __mp_list_view_count_layout_tts_cb,
					       NULL);

			elm_layout_content_set(g_pMainViewWidget->
					       pListViewTitlelayout,
					       "elm.swallow.groupindex",
					       g_pMainViewWidget->pVideoGenlistTitle);

			elm_box_pack_start(g_pMainViewWidget->pBox,
					   g_pMainViewWidget->pListViewTitlelayout);
		}

		g_pMainViewWidget->pVideoGenlistItemForTitle =
			elm_genlist_item_append(g_pMainViewWidget->pVideoGenlistTitle,
						g_pMainViewWidget->
						pVideoGenlistTitleItc, NULL, NULL,
						ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_object_item_del_cb_set(g_pMainViewWidget->
					   pVideoGenlistItemForTitle,
					   __mp_main_video_index_item_del_cb);
	}
}

Evas_Object *mp_list_view_create_base_layout(void *pParent)
{
	if (!g_pMainViewWidget) {
		VideoLogInfo("Main handles of list view are not existed.");
		return NULL;
	}

	if (!pParent) {
		VideoLogInfo("[ERR]");
		return NULL;
	}

	VideoLogInfo("");

	if (g_pMainViewWidget->pListViewBaselayout) {
		evas_object_del(g_pMainViewWidget->pListViewBaselayout);
		g_pMainViewWidget->pListViewBaselayout = NULL;
	}
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_PLAYER_LISTVIEW_EDJ);

	g_pMainViewWidget->pListViewBaselayout = elm_layout_add(pParent);
	elm_layout_file_set(g_pMainViewWidget->pListViewBaselayout,
			    edj_path, LISTVIEW_EDJ_GROUP);
	evas_object_size_hint_weight_set(g_pMainViewWidget->
					 pListViewBaselayout,
					 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pMainViewWidget->
					pListViewBaselayout, EVAS_HINT_FILL,
					EVAS_HINT_FILL);

	return g_pMainViewWidget->pListViewBaselayout;
}

static void __mp_list_personal_view_display(void)
{
	if (g_pMainViewHandle == NULL) {
		VideoLogError("g_pMainViewHandle == NULL!!!");
		VideoLogAssert(0);
		return;
	}

	if (g_pMainViewWidget == NULL) {
		VideoLogError("g_pMainViewWidget == NULL!!!");
		VideoLogAssert(0);
		return;
	}
	/*      if (g_pMainViewWidget->pListViewBaselayout == NULL)*/
	/*      {*/
	/*              g_pMainViewWidget->pListViewBaselayout = mp_list_view_create_base_layout(g_pMainViewWidget->pBox);*/
	/*      }*/

	evas_object_show(g_pMainViewWidget->pListViewBaselayout);
	/*elm_box_pack_end(g_pMainViewWidget->pBox, g_pMainViewWidget->pListViewBaselayout);*/
	/*elm_box_pack_end(g_pMainViewWidget->pBox, g_pMainViewWidget->pListViewBaselayout);*/
	mp_list_view_check_more_button_of_toolbar();
	__mp_list_view_update_list(g_pMainViewHandle->nListTabType,
				   LIST_UPDATE_TYPE_ALL);
}

void mp_list_view_init_widget(void *pNaviFrameHandle)
{
	if (!g_pMainViewHandle || !g_pMainViewWidget || !pNaviFrameHandle) {
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}

	g_pMainViewHandle->nListTabType = mp_util_get_main_tab_type();

	VideoLogInfo("g_pMainViewHandle->nListTabType : [%d]",
		     g_pMainViewHandle->nListTabType);

	if (g_pMainViewHandle->nListTabType != LIST_TAB_TYPE_PERSONAL
			&& g_pMainViewHandle->nListTabType != LIST_TAB_TYPE_DEVICES) {
		VideoLogError("ListTabType error : [%d]!!!",
			      g_pMainViewHandle->nListTabType);
		g_pMainViewHandle->nListTabType = LIST_TAB_TYPE_PERSONAL;
		mp_util_set_main_tab_type(g_pMainViewHandle->nListTabType);
	}

	g_pMainViewHandle->pNaviFrameHandle = pNaviFrameHandle;

	/*      g_pMainViewWidget->pBox = elm_box_add((Evas_Object*)pNaviFrameHandle);*/
	/*      elm_box_horizontal_set(g_pMainViewWidget->pBox, EINA_FALSE);*/
	/*      evas_object_show(g_pMainViewWidget->pBox);*/


	if (g_pMainViewWidget->pListViewBaselayout == NULL) {
		g_pMainViewWidget->pListViewBaselayout =
			mp_list_view_create_base_layout(g_pMainViewHandle->
							pNaviFrameHandle);
	}

	g_pMainViewHandle->pNaviFrameItem =
		elm_naviframe_item_push(g_pMainViewHandle->pNaviFrameHandle,
					VIDEOS_TITLE_VIDEO, NULL, NULL,
					g_pMainViewWidget->pListViewBaselayout,
					NULL);

	elm_naviframe_item_pop_cb_set(g_pMainViewHandle->pNaviFrameItem,
				      mp_list_view_back_btn_cb,
				      g_pMainViewHandle);
	elm_naviframe_item_title_enabled_set(g_pMainViewHandle->
					     pNaviFrameItem, EINA_TRUE,
					     EINA_FALSE);
	evas_object_smart_callback_add(g_pMainViewHandle->pNaviFrameHandle,
				       "transition,finished",
				       mp_list_view_naviframe_transition_effect_cb,
				       NULL);

	__mp_list_view_register_recently_config();
	/*      mp_util_cloud_change_callback_register((void*)__mp_list_view_result_user_cb);*/
	mp_util_svc_set_update_db_cb_func((UpdateDatabaseCbFunc)
					  __mp_list_view_db_change_cb);
	mp_util_hide_indicator();

	g_pMainViewHandle->nListTabType = mp_util_get_main_tab_type();

	if (g_pMainViewHandle->nListTabType == LIST_TAB_TYPE_PERSONAL) {
		__mp_list_personal_view_display();
	}
}

void mp_list_view_init_handles(void)
{
	VideoLogInfo("");

	if (g_pMainViewHandle) {
		free(g_pMainViewHandle);
		g_pMainViewHandle = NULL;
	}

	if (g_pMainViewWidget) {
		free(g_pMainViewWidget);
		g_pMainViewWidget = NULL;
	}

	g_pMainViewHandle =
		(st_VideoListViewMainViewHandle *) calloc(1,
				sizeof
				(st_VideoListViewMainViewHandle));
	g_pMainViewWidget =
		(st_VideoListViewMainViewWidget *) calloc(1,
				sizeof
				(st_VideoListViewMainViewWidget));
}


/*///////////////////////////////////////////////////////////////*/
/* External APIs*/
void mp_list_view_push(void *pNaviFrame)
{
	VideoLogInfo("");

	mp_list_view_init_handles();
	mp_list_view_init_widget(pNaviFrame);
}

void mp_list_view_destroy(void)
{
	if (!g_pMainViewHandle) {
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}
#ifdef VS_FEATURE_LIST_VIEW
	mp_normal_view_destroy();
#endif
	mp_folder_view_destroy();
	mp_thumbnail_view_destroy();
	VideoLogInfo("");

	mp_util_db_reset_all_update_fun();
	mp_util_db_reset_all_backup_fun();

	mp_ft_ctrl_destroy();
	mp_ft_ctrl_delete_center_toolbar();

	mp_list_view_free_main_view_widget();
	mp_util_svc_unset_update_db_cb_func();
	mp_util_config_ignore_preview_key_changed_cb();

	if (g_pMainViewHandle) {
		g_pMainViewHandle->pNaviFrameHandle = NULL;
		g_pMainViewHandle->pNaviFrameItem = NULL;
		g_pMainViewHandle->nListTabType = LIST_TAB_TYPE_PERSONAL;
		MP_DEL_TIMER(g_pMainViewHandle->pDbUpdateTimer);
		MP_DEL_ITC(g_pMainViewWidget->pVideoGenlistTitleItc);

		free(g_pMainViewHandle);
		g_pMainViewHandle = NULL;
	}
}

static bool __mp_list_view_is_top_view(void)
{

	if (!g_pMainViewHandle || !g_pMainViewWidget) {
		VideoLogInfo("Main handles of list view are not existed.");
		return FALSE;
	}

	if (!g_pMainViewHandle->pNaviFrameHandle) {
		VideoLogInfo("[ERR] No exist naviframe handle.");
		return FALSE;
	}
	if (mp_mgr_get_library_naviframe()) {
		int count =
			eina_list_count(elm_naviframe_items_get
					(mp_mgr_get_library_naviframe()));
		VideoLogInfo("eina_list_count = %d", count);
		return (count == 1);
	}
	return FALSE;

}

static char *__mp_list_view_count_layout_tts_cb(void *data,
		Evas_Object *obj)
{
	Elm_Object_Item *pItem = NULL;
	const char *pText = NULL;

	if (g_pMainViewHandle == NULL) {
		VideoLogError("g_pMainViewHandle == NULL!!!");
		return NULL;
	}

	if (g_pMainViewWidget == NULL) {
		VideoLogError("g_pMainViewWidget == NULL!!!");
		return NULL;
	}

	if (g_pMainViewWidget->pListViewTitlelayout == NULL) {
		VideoLogError
		("g_pMainViewWidget->pListViewTitlelayout == NULL!!!");
		return NULL;
	}

	pItem =
		elm_genlist_first_item_get(g_pMainViewWidget->pVideoGenlistTitle);
	pText = elm_object_item_part_text_get(pItem, "elm.text.sub");
	if (pText != NULL) {
		VideoLogInfo("pText : [%s]", pText);
		return g_strdup(pText);
	} else {
		VideoLogError("pText == NULL!!!");
	}

	return NULL;
}

/*   update---------------------------------------------- situation   *
  *  1,When it is folder view or download view,                                                       *
  *     only update recently played part;                                                                   *
  *  2,When it is normal view or thumbnail view,                                                      *
  *     update both recently played part and genlist item.                                        *
  *  Consider different situation for clicking recently played part and genlist item,  *
  *  First to get just played url,Get genlist index by this url,                                    *
  *  If url is NULL, get genlist index by related view.                                                */

void mp_list_view_update_widget(void)
{
	if (!g_pMainViewHandle || !g_pMainViewWidget) {
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}

	int nViewType = mp_view_as_ctrl_get_type();
	int nVideoCount = mp_util_svc_get_number_of_video_item_by_type();
	if (nViewType == MP_LIST_VIEW_AS_FOLDER_LIST) {
		nVideoCount = mp_util_svc_get_video_folder_size();
	}

	if (nVideoCount < 1) {
		VideoLogWarning("No video item");
		return;
	}

	mp_ft_ctrl_hide_more_popup();

	if (g_pMainViewHandle->nListTabType == LIST_TAB_TYPE_PERSONAL) {
		int nSortType = mp_sort_ctrl_get_sort_state();
		if (MP_LIST_SORT_BY_RECENTLY_ADDED == nSortType) {
			char *pFirstPath = mp_util_svc_get_video_url(0);
			char *pVideoPath = mp_util_config_get_preview();
			if (g_strcmp0(pFirstPath, pVideoPath) != 0) {
				MP_FREE_STRING(pVideoPath);
				MP_FREE_STRING(pFirstPath);
				__mp_list_view_db_update(-1);
				return;
			}
			MP_FREE_STRING(pVideoPath);
			MP_FREE_STRING(pFirstPath);
		}
		/*if (MP_LIST_SORT_BY_RECENTLY_VIEWED == nSortType)
		   {
		   mp_search_view_update_played_item(0);
		   } */
		else {
			mp_search_view_update_played_item(-1);
		}
#ifdef VS_FEATURE_LIST_VIEW
		if (nViewType == MP_LIST_VIEW_AS_NORMAL_LIST) {
			int nSelectedVideoItemIndex =
				mp_normal_view_get_selected_video_item_index();
			if (MP_LIST_SORT_BY_RECENTLY_ADDED == nSortType) {
				nSelectedVideoItemIndex = 0;
			}
			mp_util_svc_update_thumbnail_info(nSelectedVideoItemIndex);
			mp_normal_view_update_video_items(nSelectedVideoItemIndex);
		} else
#endif
			if (nViewType == MP_LIST_VIEW_AS_FOLDER_LIST) {
				VideoLogInfo("MP_LIST_VIEW_AS_FOLDER_LIST");
				mp_folder_item_view_update_played_item();
				mp_search_view_update_genlist();
			} else if (nViewType == MP_LIST_VIEW_AS_THUMBNAIL_LIST) {
				VideoLogInfo("MP_LIST_VIEW_AS_THUMBNAIL_LIST");
				mp_thumbnail_view_update_played_item();
			} else {
				VideoLogInfo("Don't need to update video item");
			}

	}

}

void mp_list_view_update_widget_resume(void)
{
	if (!g_pMainViewHandle || !g_pMainViewWidget) {
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}

	__mp_list_view_result_view_update_cb();
	int nViewType = mp_view_as_ctrl_get_type();
	mp_search_view_update_played_item(-1);
	if (nViewType == MP_LIST_VIEW_AS_FOLDER_LIST) {
		VideoLogInfo("MP_LIST_VIEW_AS_FOLDER_LIST");
		mp_folder_item_view_update_played_item();
		mp_search_view_update_genlist();
		} else if (nViewType == MP_LIST_VIEW_AS_THUMBNAIL_LIST) {
			VideoLogInfo("MP_LIST_VIEW_AS_THUMBNAIL_LIST");
			mp_thumbnail_view_update_played_item();
		} else {
				VideoLogInfo("Don't need to update video item");
		}
}
void mp_list_view_change_language(void)
{
	if (!g_pMainViewHandle || !g_pMainViewWidget) {
		VideoLogInfo("Main handles of list view are not existed.");
		return;
	}

	VideoLogInfo("");

	elm_object_item_domain_translatable_part_text_set(g_pMainViewHandle->
			pNaviFrameItem,
			"elm.text.title",
			PACKAGE,
			VIDEOS_TITLE_VIDEO);
#ifdef VS_FEATURE_LIST_VIEW
	mp_normal_view_change_language();
#endif
	mp_folder_item_view_change_language();
	mp_remove_view_change_language();
	mp_sort_ctrl_change_language();
	mp_folder_view_change_language();
	mp_view_ctrl_change_language();
	mp_search_view_change_language();
	mp_share_view_change_language();
	mp_select_view_change_language();
	mp_view_as_ctrl_change_language();
	mp_option_ctrl_change_language();
	mp_list_view_update_count_layout();
}

void mp_list_view_change_color(void)
{
	if (!g_pMainViewHandle || !g_pMainViewWidget) {
		VideoLogError("Main handles of list view are not existed.");
		return;
	}
}

void mp_list_view_destroy_player(void)
{
	mp_launch_video_stop();
}
