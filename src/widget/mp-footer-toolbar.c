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

#include "mp-util.h"
#include "mp-video-log.h"
#include "mp-footer-toolbar.h"
#include "mp-util-media-service.h"
#include "mp-video-string-define.h"
#include "mp-video-value-define.h"
#include "mp-external-ug.h"

#include "mp-rotate-ctrl.h"
#include "mp-video-view-popup-ctrl.h"
#include "vp-util.h"

typedef struct {
	void *pNaviFrameHandle;
	void *pNaviFrameItem;

	MpFooterLibraryMainListTabType nFooterListTabType;

	Evas_Object *pSortToolbarButton;
	Evas_Object *pSearchToolbarButton;
	Evas_Object *pSettingToolbarButton;
	Evas_Object *pMoreToolbarButton;
	Evas_Object *pMoreCtxPopup;
	Evas_Object *pDeleteButton;
	Evas_Object *pDeleteCancelButton;
	Evas_Object *pViewButton;

	st_RegisteCbFunc ToolbarCbFunc;
} st_ToolbarWidget;


st_ToolbarWidget *g_pToolbarWidget = NULL;


void mp_ft_ctrl_free_toolbar_btn(void)
{
	VideoLogInfo("");

	if (g_pToolbarWidget) {
		if (g_pToolbarWidget->pNaviFrameItem) {
			/*elm_object_item_part_content_unset(g_pToolbarWidget->pNaviFrameItem, "toolbar_more_btn");*/
			/*elm_object_item_part_content_unset(g_pToolbarWidget->pNaviFrameItem, "toolbar_button1");*/
			elm_object_item_part_content_unset(g_pToolbarWidget->
							   pNaviFrameItem,
							   "toolbar_button2");

			g_pToolbarWidget->pNaviFrameItem = NULL;
		}
		g_pToolbarWidget->pNaviFrameHandle = NULL;

		if (g_pToolbarWidget->pViewButton) {
			evas_object_del(g_pToolbarWidget->pViewButton);
			g_pToolbarWidget->pViewButton = NULL;
		}

		if (g_pToolbarWidget->pMoreToolbarButton) {
			evas_object_del(g_pToolbarWidget->pMoreToolbarButton);
			g_pToolbarWidget->pMoreToolbarButton = NULL;
		}

		if (g_pToolbarWidget->pSortToolbarButton) {
			evas_object_del(g_pToolbarWidget->pSortToolbarButton);
			g_pToolbarWidget->pSortToolbarButton = NULL;
		}

		if (g_pToolbarWidget->pSearchToolbarButton) {
			evas_object_del(g_pToolbarWidget->pSearchToolbarButton);
			g_pToolbarWidget->pSearchToolbarButton = NULL;
		}

		if (g_pToolbarWidget->pSettingToolbarButton) {
			evas_object_del(g_pToolbarWidget->pSettingToolbarButton);
			g_pToolbarWidget->pSettingToolbarButton = NULL;
		}

		if (g_pToolbarWidget->pMoreCtxPopup) {
			evas_object_del(g_pToolbarWidget->pMoreCtxPopup);
			g_pToolbarWidget->pMoreCtxPopup = NULL;
		}

		if (g_pToolbarWidget->pDeleteButton) {
			evas_object_del(g_pToolbarWidget->pDeleteButton);
			g_pToolbarWidget->pDeleteButton = NULL;
		}

		if (g_pToolbarWidget->pDeleteCancelButton) {
			evas_object_del(g_pToolbarWidget->pDeleteCancelButton);
			g_pToolbarWidget->pDeleteCancelButton = NULL;
		}

		g_pToolbarWidget->nFooterListTabType = FOOTER_TAB_TYPE_PERSONAL;
		g_pToolbarWidget->pNaviFrameItem = NULL;

		memset((void *) &g_pToolbarWidget->ToolbarCbFunc, 0,
		       sizeof(st_RegisteCbFunc));

		free(g_pToolbarWidget);
		g_pToolbarWidget = NULL;
	}
}

void *mp_ft_ctrl_create_more_style_btn(void *pParent, Evas_Smart_Cb pFunc,
				       void *pUserData)
{
	if (!pParent) {
		VideoLogError("No exist parent.");
		return NULL;
	}

	Evas_Object *pTmpBtn = elm_button_add(pParent);
	elm_access_info_set(pTmpBtn, ELM_ACCESS_INFO, "more");
	elm_object_style_set(pTmpBtn, "naviframe/more/default");
	evas_object_smart_callback_add(pTmpBtn, "clicked", pFunc, pUserData);

	return pTmpBtn;
}

void *mp_ft_ctrl_create_toolbar_btn(void *pParent, const char *pText,
				    Evas_Smart_Cb pFunc, void *pUserData)
{
	if (!pParent) {
		VideoLogError("No exist parent.");
		return NULL;
	}

	Evas_Object *pTmpBtn = elm_button_add(pParent);
	elm_object_style_set(pTmpBtn, "naviframe/toolbar/default");
	elm_object_text_set(pTmpBtn, pText);
	evas_object_smart_callback_add(pTmpBtn, "clicked", pFunc, pUserData);

	return pTmpBtn;
}

void mp_ft_ctrl_more_popup_dismissed_cb(void *pUserData,
					Evas_Object *pObject,
					void *pEventInfo)
{
	VideoLogInfo("");

	mp_ft_ctrl_hide_more_popup();
}

void mp_ft_ctrl_move_more_popup(Evas_Object *pCtxPopup,
				Evas_Object *pTargetObject)
{
	Evas_Coord_Rectangle rect = { 0, };
	int nRotate = elm_win_rotation_get(mp_util_get_main_window_handle());

	elm_win_screen_size_get(mp_util_get_main_window_handle(), &rect.x,
				&rect.y, &rect.w, &rect.h);

	VideoLogInfo("nRotate : [%d], rect.w : [%d], rect.h : [%d]", nRotate,
		     rect.w, rect.h);

	int nW = 0;
	int nH = 0;

	switch (nRotate) {
	case 0:
	case 180:
	{
		nW = rect.w / 2;
		nH = rect.h;
	}
	break;

	case 90:
	{
		nW = rect.h / 2;
		nH = rect.w;
	}
	break;

	case 270:
	{
		nW = rect.h / 2;
		nH = rect.w;
	}
	break;

	default:
	{
		VideoLogError("not handled nRotate : [%d]!!!", nRotate);
	}
	break;
	}

	evas_object_move(g_pToolbarWidget->pMoreCtxPopup, nW, nH);

}

static void __mp_ft_ctrl_more_popup_del_cb(void *data, Evas *e,
		Evas_Object *obj,
		void *event_info)
{
	VideoLogInfo("");
	evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL,
				       __mp_ft_ctrl_more_popup_del_cb);
	if (g_pToolbarWidget)
		g_pToolbarWidget->pMoreCtxPopup = NULL;
}

void mp_ft_ctrl_create_more_btn_of_personal(void *pUserData,
		Evas_Object *pTargetObject)
{
	VideoLogInfo("");

	Elm_Object_Item *item = NULL;
	int nItemCount = 0;

	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();

	g_pToolbarWidget->pMoreCtxPopup =
		elm_ctxpopup_add((Evas_Object *)
				 mp_util_get_main_window_handle());


	if (g_pToolbarWidget->pMoreCtxPopup == NULL) {
		VideoLogError("g_pToolbarWidget->pMoreCtxPopup == NULL!!!");
		return;
	}

	evas_object_event_callback_add(g_pToolbarWidget->pMoreCtxPopup,
				       EVAS_CALLBACK_DEL,
				       __mp_ft_ctrl_more_popup_del_cb, NULL);
	eext_object_event_callback_add(g_pToolbarWidget->pMoreCtxPopup,
				       EEXT_CALLBACK_BACK,
				       eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(g_pToolbarWidget->pMoreCtxPopup,
				       EEXT_CALLBACK_MORE,
				       eext_ctxpopup_back_cb, NULL);

	elm_object_style_set(g_pToolbarWidget->pMoreCtxPopup, "more/default");
	elm_ctxpopup_auto_hide_disabled_set(g_pToolbarWidget->pMoreCtxPopup,
					    EINA_TRUE);

	evas_object_smart_callback_add(g_pToolbarWidget->pMoreCtxPopup,
				       "dismissed",
				       mp_ft_ctrl_more_popup_dismissed_cb,
				       NULL);

	if (nVideoListSize > 0 && g_pToolbarWidget->ToolbarCbFunc.SearchCb) {
		item =
			elm_ctxpopup_item_append(g_pToolbarWidget->pMoreCtxPopup,
						 VIDEOS_CTXPOP_MENU_SEARCH_IDS, NULL,
						 g_pToolbarWidget->ToolbarCbFunc.
						 SearchCb,
						 (void *) FOOTER_TAB_TYPE_PERSONAL);
		elm_object_item_domain_text_translatable_set(item, VIDEOS_STRING,
				EINA_TRUE);
		nItemCount++;
	}
	if (g_pToolbarWidget->ToolbarCbFunc.ShareViaCb) {
		item =
			elm_ctxpopup_item_append(g_pToolbarWidget->pMoreCtxPopup,
						 VIDEOS_CTXPOP_MENU_SHARE_IDS, NULL,
						 g_pToolbarWidget->ToolbarCbFunc.
						 ShareViaCb, NULL);
		elm_object_item_domain_text_translatable_set(item, VIDEOS_STRING,
				EINA_TRUE);
		nItemCount++;
	}
	if (nVideoListSize > 0 && g_pToolbarWidget->ToolbarCbFunc.DeleteCb) {
		item =
			elm_ctxpopup_item_append(g_pToolbarWidget->pMoreCtxPopup,
						 VIDEOS_CTXPOP_MENU_DELETE_IDS, NULL,
						 g_pToolbarWidget->ToolbarCbFunc.
						 DeleteCb,
						 (void *) FOOTER_TAB_TYPE_PERSONAL);
		elm_object_item_domain_text_translatable_set(item, VIDEOS_STRING,
				EINA_TRUE);
		nItemCount++;
	}
	if (nVideoListSize > 0 && g_pToolbarWidget->ToolbarCbFunc.ViewAsCb) {
		item =
			elm_ctxpopup_item_append(g_pToolbarWidget->pMoreCtxPopup,
						 VIDEOS_CTXPOP_MENU_VIEW_AS_IDS, NULL,
						 g_pToolbarWidget->ToolbarCbFunc.
						 ViewAsCb,
						 (void *) FOOTER_TAB_TYPE_PERSONAL);
		elm_object_item_domain_text_translatable_set(item, VIDEOS_STRING,
				EINA_TRUE);
		nItemCount++;
	}
	if (nVideoListSize > 0 && g_pToolbarWidget->ToolbarCbFunc.AutoPlayCb) {
			item =
				elm_ctxpopup_item_append(g_pToolbarWidget->pMoreCtxPopup,
						VIDEOS_CTXPOP_MENU_AUTO_PLAY_NEXT_IDS, NULL,
							 g_pToolbarWidget->ToolbarCbFunc.
							 AutoPlayCb,
							 (void *) FOOTER_TAB_TYPE_PERSONAL);
			elm_object_item_domain_text_translatable_set(item, VIDEOS_STRING,
					EINA_TRUE);
			nItemCount++;
	}
	if (nVideoListSize > 0 && g_pToolbarWidget->ToolbarCbFunc.SortItemCb) {
		item =
			elm_ctxpopup_item_append(g_pToolbarWidget->pMoreCtxPopup,
						 VIDEOS_CTXPOP_MENU_SORT_BY_IDS, NULL,
						 g_pToolbarWidget->ToolbarCbFunc.
						 SortItemCb,
						 (void *) FOOTER_TAB_TYPE_PERSONAL);
		elm_object_item_domain_text_translatable_set(item, VIDEOS_STRING,
				EINA_TRUE);
		nItemCount++;
	}

	if (nVideoListSize > 0 && g_pToolbarWidget->ToolbarCbFunc.RenameCb) {
		item =
			elm_ctxpopup_item_append(g_pToolbarWidget->pMoreCtxPopup,
						 VIDEOS_CTXPOP_MENU_RENAME_IDS, NULL,
						 g_pToolbarWidget->ToolbarCbFunc.
						 RenameCb,
						 (void *) FOOTER_TAB_TYPE_PERSONAL);
		elm_object_item_domain_text_translatable_set(item, VIDEOS_STRING,
				EINA_TRUE);
		nItemCount++;
	}
	if (nVideoListSize > 0 && g_pToolbarWidget->ToolbarCbFunc.DetailsCb) {
		item =
			elm_ctxpopup_item_append(g_pToolbarWidget->pMoreCtxPopup,
						 VIDEOS_CTXPOP_MENU_DETAILS_IDS, NULL,
						 g_pToolbarWidget->ToolbarCbFunc.
						 DetailsCb,
						 (void *) FOOTER_TAB_TYPE_PERSONAL);
		elm_object_item_domain_text_translatable_set(item, VIDEOS_STRING,
				EINA_TRUE);
		nItemCount++;
	}

	if (nItemCount > 0) {
		mp_ft_ctrl_move_more_popup(g_pToolbarWidget->pMoreCtxPopup,
					   pTargetObject);
		evas_object_show(g_pToolbarWidget->pMoreCtxPopup);
	} else {
		mp_ft_ctrl_free_toolbar_btn();
	}
}


void mp_ft_ctrl_more_btn_cb(void *pUserData, Evas_Object *pObject,
			    void *pEventInfo)
{
	if (!pObject) {
		VideoLogInfo("[ERR]");
		return;
	}

	VideoLogInfo("");

	if (g_pToolbarWidget->pMoreCtxPopup) {
		evas_object_del(g_pToolbarWidget->pMoreCtxPopup);
		g_pToolbarWidget->pMoreCtxPopup = NULL;
	} else {
		int nVideoListSize =
			mp_util_svc_get_number_of_video_item_by_type();
		VideoLogInfo("g_pToolbarWidget->nFooterListTabType=%d",
			     g_pToolbarWidget->nFooterListTabType);
		if (nVideoListSize > 0) {
			if (g_pToolbarWidget->nFooterListTabType ==
					FOOTER_TAB_TYPE_PERSONAL) {
				mp_ft_ctrl_create_more_btn_of_personal(pUserData,
								       pObject);
			} else {
				VideoLogError("invalid footer type");
			}
		} else {
			VideoLogError("Empty video list");
		}
	}
}

void mp_ft_ctrl_create_center_button(void *pUserData,
				     void *pToolbarCenterButtonCb)
{
	if (!g_pToolbarWidget) {
		VideoLogError("g_pToolbarWidget is NULL.");
		return;
	}

	if (g_pToolbarWidget->pViewButton) {
		evas_object_del(g_pToolbarWidget->pViewButton);
		g_pToolbarWidget->pViewButton = NULL;
	}

	g_pToolbarWidget->pViewButton =
		mp_ft_ctrl_create_toolbar_btn(g_pToolbarWidget->pNaviFrameHandle,
					      MP_VPL_LIST_TAB_VIEW,
					      pToolbarCenterButtonCb, pUserData);
	elm_object_item_part_content_set(g_pToolbarWidget->pNaviFrameItem,
					 "toolbar_button1",
					 g_pToolbarWidget->pViewButton);
}

void mp_ft_ctrl_create_center_toolbar(void *pParent,
				      void *pToolbarCenterButtonCb)
{
	if (!g_pToolbarWidget || !pParent) {
		VideoLogError("g_pToolbarWidget or pParent is NULL.");
		return;
	}

	Evas_Object *toolbar = elm_toolbar_add(pParent);
	if (!toolbar)
		return;
	elm_object_style_set(toolbar, "default");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);
	elm_toolbar_item_append(toolbar, NULL, MP_VPL_LIST_TAB_VIEW,
				pToolbarCenterButtonCb, NULL);

	if (g_pToolbarWidget->pNaviFrameItem) {
		elm_object_item_part_content_set(g_pToolbarWidget->pNaviFrameItem,
						 "toolbar", toolbar);
	}
}


void mp_ft_ctrl_delete_center_toolbar()
{

	Evas_Object *toolbar = NULL;
	if (!g_pToolbarWidget)
		return;

	if (g_pToolbarWidget->pNaviFrameItem) {
		toolbar =
			elm_object_item_part_content_unset(g_pToolbarWidget->
					pNaviFrameItem, "toolbar");
		MP_DEL_OBJ(toolbar);
	}
}

void mp_ft_ctrl_delete_center_button(void)
{
	if (!g_pToolbarWidget) {
		VideoLogError("g_pToolbarWidget is NULL.");
		return;
	}

	if (g_pToolbarWidget->pNaviFrameItem) {
		elm_object_item_part_content_unset(g_pToolbarWidget->
						   pNaviFrameItem,
						   "toolbar_button1");
	}

	if (g_pToolbarWidget->pViewButton) {
		evas_object_del(g_pToolbarWidget->pViewButton);
		g_pToolbarWidget->pViewButton = NULL;
	}
}

void mp_ft_ctrl_disable(void)
{
	if (!g_pToolbarWidget) {
		VideoLogError("g_pToolbarWidget is NULL.");
		return;
	}

	if (g_pToolbarWidget->pNaviFrameItem) {
		elm_object_item_part_content_unset(g_pToolbarWidget->
						   pNaviFrameItem,
						   "toolbar_more_btn");
	}

	if (g_pToolbarWidget->pMoreToolbarButton) {
		evas_object_del(g_pToolbarWidget->pMoreToolbarButton);
		g_pToolbarWidget->pMoreToolbarButton = NULL;
	}
}

void mp_ft_ctrl_registe_cb_func(st_RegisteCbFunc *pCallbackFunc)
{
	if (!g_pToolbarWidget) {
		VideoLogError("g_pToolbarWidget == NULL");
		return;
	}

	if (!pCallbackFunc) {
		VideoLogError("pCallbackFunc == NULL");
		return;
	}

	VideoLogInfo("");

	memset((void *) &g_pToolbarWidget->ToolbarCbFunc, 0,
	       sizeof(st_RegisteCbFunc));

	if (pCallbackFunc->DownloadCb) {
		g_pToolbarWidget->ToolbarCbFunc.DownloadCb =
			pCallbackFunc->DownloadCb;
	}

	if (pCallbackFunc->SearchCb) {
		g_pToolbarWidget->ToolbarCbFunc.SearchCb =
			pCallbackFunc->SearchCb;
	}

	if (pCallbackFunc->DeleteCb) {
		g_pToolbarWidget->ToolbarCbFunc.DeleteCb =
			pCallbackFunc->DeleteCb;
	}

	if (pCallbackFunc->SortItemCb) {
		g_pToolbarWidget->ToolbarCbFunc.SortItemCb =
			pCallbackFunc->SortItemCb;
	}

	if (pCallbackFunc->ViewAsCb) {
		g_pToolbarWidget->ToolbarCbFunc.ViewAsCb =
			pCallbackFunc->ViewAsCb;
	}

	if (pCallbackFunc->AutoPlayCb) {
			g_pToolbarWidget->ToolbarCbFunc.AutoPlayCb =
				pCallbackFunc->AutoPlayCb;
	}

	if (pCallbackFunc->SignInCb) {
		g_pToolbarWidget->ToolbarCbFunc.SignInCb =
			pCallbackFunc->SignInCb;
	}

	if (pCallbackFunc->ShareViaCb) {
		g_pToolbarWidget->ToolbarCbFunc.ShareViaCb =
			pCallbackFunc->ShareViaCb;
	}

	if (pCallbackFunc->RenameCb) {
		g_pToolbarWidget->ToolbarCbFunc.RenameCb =
			pCallbackFunc->RenameCb;
	}

	if (pCallbackFunc->DetailsCb) {
		g_pToolbarWidget->ToolbarCbFunc.DetailsCb =
			pCallbackFunc->DetailsCb;
	}
#ifdef	ENABLE_PRIVATE_MODE
	if (pCallbackFunc->AddToPersonalCB) {
		g_pToolbarWidget->ToolbarCbFunc.AddToPersonalCB =
			pCallbackFunc->AddToPersonalCB;
	}

	if (pCallbackFunc->RemoveFromPersonalCB) {
		g_pToolbarWidget->ToolbarCbFunc.RemoveFromPersonalCB =
			pCallbackFunc->RemoveFromPersonalCB;
	}
#endif
}


void mp_ft_ctrl_create(void *pNaviFrameHandle, void *pNaviFrameItem,
		       MpFooterLibraryMainListTabType nFooterListTabType,
		       st_RegisteCbFunc *pRegisteCbFunc)
{
	if (!pNaviFrameHandle) {
		VideoLogError("g_pToolbarWidget is NULL.");
		return;
	}

	mp_ft_ctrl_free_toolbar_btn();

	g_pToolbarWidget =
		(st_ToolbarWidget *) calloc(1, sizeof(st_ToolbarWidget));
	memset(g_pToolbarWidget, 0, sizeof(st_ToolbarWidget));

	g_pToolbarWidget->pNaviFrameHandle = pNaviFrameHandle;
	g_pToolbarWidget->pNaviFrameItem = pNaviFrameItem;
	g_pToolbarWidget->nFooterListTabType = nFooterListTabType;

	if (pRegisteCbFunc) {
		mp_ft_ctrl_registe_cb_func(pRegisteCbFunc);
	}

	g_pToolbarWidget->pMoreToolbarButton =
		mp_ft_ctrl_create_more_style_btn(g_pToolbarWidget->
				pNaviFrameHandle,
				mp_ft_ctrl_more_btn_cb, NULL);
	elm_object_item_part_content_set(g_pToolbarWidget->pNaviFrameItem,
					 "toolbar_more_btn",
					 g_pToolbarWidget->
					 pMoreToolbarButton);
}

void mp_ft_ctrl_destroy(void)
{
	mp_ft_ctrl_free_toolbar_btn();
}

void mp_ft_ctrl_hide_more_popup(void)
{
	if (!g_pToolbarWidget) {
		VideoLogError("g_pToolbarWidget == NULL");
		return;
	}

	if (g_pToolbarWidget->pMoreCtxPopup) {
		evas_object_smart_callback_del(g_pToolbarWidget->pMoreCtxPopup,
					       "dismissed",
					       mp_ft_ctrl_more_popup_dismissed_cb);
		/*evas_object_hide(g_pToolbarWidget->pMoreCtxPopup);*/
		evas_object_del(g_pToolbarWidget->pMoreCtxPopup);
		g_pToolbarWidget->pMoreCtxPopup = NULL;
	}
}

void mp_ft_ctrl_set_list_tab_type(MpFooterLibraryMainListTabType
				  nFooterListTabType)
{
	if (!g_pToolbarWidget) {
		VideoLogError("g_pToolbarWidget == NULL");
		return;
	}

	g_pToolbarWidget->nFooterListTabType = nFooterListTabType;

	if (g_pToolbarWidget->nFooterListTabType == FOOTER_TAB_TYPE_PERSONAL) {
		VideoLogInfo("FOOTER_TAB_TYPE_PERSONAL");
	} else if (g_pToolbarWidget->nFooterListTabType ==
			FOOTER_TAB_TYPE_DOWNLOAD) {
		VideoLogInfo("FOOTER_TAB_TYPE_DOWNLOAD");
	} else {
		g_pToolbarWidget->nFooterListTabType = FOOTER_TAB_TYPE_PERSONAL;
		VideoLogInfo("FOOTER_TAB_TYPE_PERSONAL");
	}
}

void mp_ft_ctrl_rotate_update()
{
	VideoLogInfo("");
	if (g_pToolbarWidget != NULL
			&& g_pToolbarWidget->pMoreCtxPopup != NULL) {
		mp_ft_ctrl_move_more_popup(g_pToolbarWidget->pMoreCtxPopup, NULL);
		evas_object_show(g_pToolbarWidget->pMoreCtxPopup);
	}
}

bool mp_ft_ctrl_is_show(void)
{
	if (g_pToolbarWidget && g_pToolbarWidget->pMoreCtxPopup) {
		VideoLogInfo("footbar is show");
		return TRUE;
	}
	VideoLogInfo("footbar not show");
	return FALSE;
}
