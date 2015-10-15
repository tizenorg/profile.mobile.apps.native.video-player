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
#include <Ecore_Evas.h>
#include <Elementary.h>
#include <app_preference.h>

#include "mp-util.h"
#include "mp-video-log.h"
#include "video-player.h"
#include "mp-video-value-define.h"
#include "mp-video-string-define.h"
#include "mp-footer-toolbar.h"
#include "mp-video-list-view-as-ctrl.h"
#include "mp-util-widget-ctrl.h"
#include "vp-util.h"

#define PREF_MP_VIDEO_VIEW_AS_TYPE_KEY  "preference/org.tizen.videos/view_as_type"

typedef struct _MpVideoViewAs{
	//obj
	Evas_Object *pPopUpHandle;
	Evas_Object *pGroupRadio;
	Evas_Object *pGenList;
	Evas_Object *pBox;

	//data
	Elm_Genlist_Item_Class *stViewAsItc;
	ListViewAsCtrlCbFunc ListViewAsCtrlUserCbFunc;
	MpListViewAsType nCurrViewAsType;
	int nListCount;
	int nCurrTabType;

}stMpVideoViewAs;

stMpVideoViewAs *g_pViewAsHandle = NULL;

////////////////////////////////////////////////////////////
// Internal function
////////////////////////////////////////////////////////////
static void __mp_view_as_ctrl_rotate_cb(void *data, Evas_Object *obj, void *event_info);
static void __mp_view_as_ctrl_mouse_up_cb(void *pUserData, Evas *pEvas, Evas_Object *pObject, void *pEventInfo);
static void __mp_view_as_ctrl_cancel_btn_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo);
static void __mp_view_as_ctrl_select_item_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo);
static void  __mp_view_as_ctrl_select_radio_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo);

void mp_view_as_ctrl_delete_handle(void)
{
	VideoLogInfo("");
	if (!g_pViewAsHandle)
	{
		VideoLogError("-------g_pViewAsHandle is NULL--------");
		return;
	}
	if (g_pViewAsHandle->pPopUpHandle)
	{
		eext_object_event_callback_del(g_pViewAsHandle->pPopUpHandle, EEXT_CALLBACK_BACK, __mp_view_as_ctrl_cancel_btn_cb);
		evas_object_event_callback_del(g_pViewAsHandle->pPopUpHandle, EVAS_CALLBACK_MOUSE_UP, __mp_view_as_ctrl_mouse_up_cb);
		evas_object_smart_callback_del(g_pViewAsHandle->pPopUpHandle, "block,clicked", __mp_view_as_ctrl_cancel_btn_cb);
		evas_object_smart_callback_del((Evas_Object*)mp_util_get_main_window_handle(), "rotation,changed", __mp_view_as_ctrl_rotate_cb);
	}
	MP_DEL_OBJ(g_pViewAsHandle->pGroupRadio);
	MP_DEL_OBJ(g_pViewAsHandle->pGenList);
	MP_DEL_OBJ(g_pViewAsHandle->pBox);
	MP_DEL_OBJ(g_pViewAsHandle->pPopUpHandle);
	MP_DEL_ITC(g_pViewAsHandle->stViewAsItc);
	MP_FREE_STRING(g_pViewAsHandle);

}

void mp_view_as_ctrl_set_type(MpListViewAsType nViewAsType)
{
	if (!g_pViewAsHandle)
	{
		VideoLogError("");
		return;
	}

	if (MP_LIST_VIEW_AS_NONE >= nViewAsType || nViewAsType >= MP_LIST_VIEW_AS_MAX)
	{
		VideoLogWarning("[WARNING] Value of View as type is not matched.");
		nViewAsType = MP_LIST_VIEW_AS_THUMBNAIL_LIST;
	}

	g_pViewAsHandle->nCurrViewAsType = nViewAsType;

	int nRet = preference_set_int(PREF_MP_VIDEO_VIEW_AS_TYPE_KEY, g_pViewAsHandle->nCurrViewAsType);
	if (nRet)
	{
		VideoLogWarning("[WARNING] Fail to set pref value about list view as type.");
	}
}

static char *__mp_view_as_ctrl_get_label_of_genlsit_item_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{

	int nIndex = (int)pUserData;

	if (!strcmp(pPart, "elm.text") || !strcmp(pPart, "elm.text.main.left"))
	{
		switch (nIndex)
		{
#ifdef VS_FEATURE_LIST_VIEW
			case MP_LIST_VIEW_AS_NORMAL_LIST:		return strdup(VIDEOS_VIEWASOPT_OPT_LIST);
#endif
#ifdef VS_FEATURE_THUMBNAIL_VIEW
			case MP_LIST_VIEW_AS_THUMBNAIL_LIST:	return strdup(VIDEOS_VIEWASOPT_OPT_THUMBNAIL);
#endif
			case MP_LIST_VIEW_AS_FOLDER_LIST:		return strdup(VIDEOS_VIEWASOPT_OPT_FOLDER);
		}
	}
	return NULL;
}

static Evas_Object *__mp_view_as_ctrl_get_icon_of_genlsit_item_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{

	if (!g_pViewAsHandle)
	{
		VideoLogError("");
		return NULL;
	}

	int nIndex = (int)pUserData;
	Evas_Object *pTmpRadio = NULL;
	Evas_Object *pLayout	= NULL;

	if (!strcmp(pPart, "elm.swallow.end") || !strcmp(pPart, "elm.icon.2"))
	{
		pLayout = elm_layout_add(pObject);
		elm_layout_theme_set(pLayout, "layout", "list/C/type.2", "default");

		pTmpRadio = elm_radio_add(pLayout);
		elm_radio_state_value_set(pTmpRadio, nIndex);
		elm_radio_group_add(pTmpRadio, g_pViewAsHandle->pGroupRadio);

		if (g_pViewAsHandle->nCurrViewAsType == nIndex)
		{
			elm_radio_value_set(g_pViewAsHandle->pGroupRadio, g_pViewAsHandle->nCurrViewAsType);
			mp_view_as_ctrl_set_type(g_pViewAsHandle->nCurrViewAsType);
		}

		evas_object_smart_callback_add(pTmpRadio, "changed", __mp_view_as_ctrl_select_radio_cb, (void *)pUserData);
		elm_layout_content_set(pLayout, "elm.swallow.content", pTmpRadio);

		evas_object_show(pTmpRadio);
		evas_object_show(pLayout);

	}
	return pLayout;

}

static void __mp_view_as_ctrl_cancel_btn_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");

	mp_view_as_ctrl_hide();
}

static void __mp_view_as_ctrl_select_item_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");

	if (!g_pViewAsHandle)
	{
		VideoLogError("g_pViewAsHandle is ?NULL");
		return;
	}

	if (!pEventInfo) {
		VideoLogError("pEventInfo is NULL");
		return;
	}

	int nIndex = 0;
	Elm_Object_Item *pItem = (Elm_Object_Item *)pEventInfo;
	Elm_Object_Item *pSelectedItem = elm_genlist_selected_item_get(pObject);
	if (pSelectedItem)
	{
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}

	nIndex = (int)elm_object_item_data_get(pItem);
	if (nIndex == g_pViewAsHandle->nCurrViewAsType) {
		VideoLogInfo("Current state");
		mp_view_as_ctrl_hide();
	} else {
		g_pViewAsHandle->nCurrViewAsType = nIndex;
		elm_radio_value_set(g_pViewAsHandle->pGroupRadio, g_pViewAsHandle->nCurrViewAsType);

		mp_view_as_ctrl_set_type(g_pViewAsHandle->nCurrViewAsType);

		ListViewAsCtrlCbFunc fFunc = g_pViewAsHandle->ListViewAsCtrlUserCbFunc;
		mp_view_as_ctrl_hide();

		if (fFunc)
		{
			fFunc();
		}
	}
}

static void  __mp_view_as_ctrl_select_radio_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{

	if (!g_pViewAsHandle)
	{
		VideoLogError("");
		return;
	}

	int nIndex = (int)pUserData;

	g_pViewAsHandle->nCurrViewAsType = nIndex;
	mp_view_as_ctrl_set_type(g_pViewAsHandle->nCurrViewAsType);

	ListViewAsCtrlCbFunc fFunc = g_pViewAsHandle->ListViewAsCtrlUserCbFunc;
	mp_view_as_ctrl_hide();

	if (fFunc)
	{
		fFunc();
	}
}

static void __mp_view_as_ctrl_mouse_up_cb(void *pUserData, Evas *pEvas, Evas_Object *pObject, void *pEventInfo)
{
	Evas_Event_Mouse_Up *pEvent = pEventInfo;

	if (pEvent->button == 3)
	{
		mp_view_as_ctrl_hide();
	}
}

static void __mp_view_as_ctrl_realized(void *data, Evas_Object *obj, void *event_info)
{
	if (!event_info || !obj)
	{
		VideoLogError("event info is invalid");
		return;
	}
	Elm_Object_Item *pItem = elm_genlist_last_item_get(obj);

	if (pItem)
	{
		elm_object_item_signal_emit(pItem, "elm,state,bottomline,hide", "");
	}
}


static void __mp_view_as_ctrl_rotate_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!g_pViewAsHandle)
	{
		VideoLogError("pVideoListSortPopUpHandle IS null");
		return;
	}
	if (g_pViewAsHandle->nListCount < VIDEO_POPUP_MIN_ITEMS)
	{
		return;
	}
	mp_widget_ctrl_set_popup_min_size(g_pViewAsHandle->pBox, g_pViewAsHandle->nListCount, VIDEOS_POPUP_114);

}

////////////////////////////////////////////////////////////
// External function
////////////////////////////////////////////////////////////

bool mp_view_as_ctrl_show(int tabType, ListViewAsCtrlCbFunc pListViewAsCtrlCb)
{
	VideoLogInfo("");

	if (pListViewAsCtrlCb == NULL)
	{
		VideoLogError("No have pListViewAsCtrlCb handle.");
		return FALSE;
	}

	mp_view_as_ctrl_delete_handle();
	g_pViewAsHandle = (stMpVideoViewAs*)calloc(1, sizeof(stMpVideoViewAs));
	if (!g_pViewAsHandle)
	{
		VideoLogError("g_pViewAsHandle is NULL");
		return FALSE;
	}

	g_pViewAsHandle->ListViewAsCtrlUserCbFunc = pListViewAsCtrlCb;
	g_pViewAsHandle->nCurrTabType = tabType;
	g_pViewAsHandle->nCurrViewAsType = mp_view_as_ctrl_get_type();

	g_pViewAsHandle->pPopUpHandle = elm_popup_add(mp_util_get_main_window_handle());


	//elm_object_style_set(pListViewAsPopUpHandle, "content_no_vhpad"); //menustyle
	elm_object_style_set(g_pViewAsHandle->pPopUpHandle, "default"); //menustyle
	elm_object_domain_translatable_part_text_set(g_pViewAsHandle->pPopUpHandle, "title,text", PACKAGE, VIDEOS_VIEWASPOP_HEADER_VIEW_AS);
	evas_object_size_hint_weight_set(g_pViewAsHandle->pPopUpHandle, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	eext_object_event_callback_add(g_pViewAsHandle->pPopUpHandle, EEXT_CALLBACK_BACK, __mp_view_as_ctrl_cancel_btn_cb, NULL);
	evas_object_event_callback_add(g_pViewAsHandle->pPopUpHandle, EVAS_CALLBACK_MOUSE_UP, __mp_view_as_ctrl_mouse_up_cb, NULL);
	evas_object_smart_callback_add(g_pViewAsHandle->pPopUpHandle, "block,clicked", __mp_view_as_ctrl_cancel_btn_cb, NULL);
	evas_object_smart_callback_add((Evas_Object*)mp_util_get_main_window_handle(), "rotation,changed", __mp_view_as_ctrl_rotate_cb, NULL);

	//  Set item of Genlist.
	MP_DEL_ITC(g_pViewAsHandle->stViewAsItc);
	g_pViewAsHandle->stViewAsItc = elm_genlist_item_class_new();
	g_pViewAsHandle->stViewAsItc->item_style = "default";	//"1text.1icon.2/popup";
	g_pViewAsHandle->stViewAsItc->func.text_get = (void *)__mp_view_as_ctrl_get_label_of_genlsit_item_cb;
	g_pViewAsHandle->stViewAsItc->func.content_get = (void *)__mp_view_as_ctrl_get_icon_of_genlsit_item_cb;
	g_pViewAsHandle->stViewAsItc->func.state_get = NULL;
	g_pViewAsHandle->stViewAsItc->func.del = NULL;

	// Create genlist handle
	g_pViewAsHandle->pGenList = elm_genlist_add(g_pViewAsHandle->pPopUpHandle);
	evas_object_size_hint_weight_set(g_pViewAsHandle->pGenList, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pViewAsHandle->pGenList, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(g_pViewAsHandle->pGenList, "realized", __mp_view_as_ctrl_realized, NULL);

	g_pViewAsHandle->pGroupRadio = elm_radio_add(g_pViewAsHandle->pGenList);

	int nMax_view_list = 0;
#ifdef VS_FEATURE_LIST_VIEW
	elm_genlist_item_append(g_pViewAsHandle->pGenList, g_pViewAsHandle->stViewAsItc, (void *)MP_LIST_VIEW_AS_NORMAL_LIST, NULL, ELM_GENLIST_ITEM_NONE, __mp_view_as_ctrl_select_item_cb, NULL);
	nMax_view_list++;
#endif
#ifdef VS_FEATURE_THUMBNAIL_VIEW
	elm_genlist_item_append(g_pViewAsHandle->pGenList, g_pViewAsHandle->stViewAsItc, (void *)MP_LIST_VIEW_AS_THUMBNAIL_LIST, NULL, ELM_GENLIST_ITEM_NONE, __mp_view_as_ctrl_select_item_cb, NULL);
	nMax_view_list++;
#endif
#ifdef ENABLE_STORE_VIEWAS
	if (FOOTER_TAB_TYPE_PERSONAL == g_pViewAsHandle->nCurrTabType)
	{
		elm_genlist_item_append(g_pViewAsHandle->pGenList, g_pViewAsHandle->stViewAsItc, (void *)MP_LIST_VIEW_AS_FOLDER_LIST, NULL, ELM_GENLIST_ITEM_NONE, __mp_view_as_ctrl_select_item_cb, NULL);
		nMax_view_list++;
	}
#else
	elm_genlist_item_append(g_pViewAsHandle->pGenList, g_pViewAsHandle->stViewAsItc, (void *)MP_LIST_VIEW_AS_FOLDER_LIST, NULL, ELM_GENLIST_ITEM_NONE, __mp_view_as_ctrl_select_item_cb, NULL);
	nMax_view_list++;
#endif

	g_pViewAsHandle->nListCount = nMax_view_list;

	g_pViewAsHandle->pBox = elm_box_add(g_pViewAsHandle->pPopUpHandle);

	mp_widget_ctrl_set_popup_min_size(g_pViewAsHandle->pBox, g_pViewAsHandle->nListCount, VIDEOS_POPUP_114);
	evas_object_size_hint_weight_set(g_pViewAsHandle->pBox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pViewAsHandle->pBox, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_box_pack_end(g_pViewAsHandle->pBox, g_pViewAsHandle->pGenList);
	elm_object_content_set(g_pViewAsHandle->pPopUpHandle, g_pViewAsHandle->pBox);

	evas_object_show(g_pViewAsHandle->pGenList);
	evas_object_show(g_pViewAsHandle->pPopUpHandle);

	return TRUE;
}

void mp_view_as_ctrl_hide(void)
{
	VideoLogInfo("");

	mp_view_as_ctrl_delete_handle();
}

void mp_view_as_ctrl_change_language(void)
{
	if (!g_pViewAsHandle)
	{
		VideoLogError("g_pViewAsHandle is NULL");
		return;
	}

	if (!g_pViewAsHandle->pGenList || !g_pViewAsHandle->pPopUpHandle)
	{
		VideoLogInfo("No existed genlist or popup handle.");
		return;
	}

	VideoLogInfo("");

	elm_object_domain_translatable_part_text_set(g_pViewAsHandle->pPopUpHandle, "title,text", PACKAGE, VIDEOS_VIEWASPOP_HEADER_VIEW_AS);
	elm_genlist_realized_items_update(g_pViewAsHandle->pGenList);

}

int mp_view_as_ctrl_get_type(void)
{
	int nValue = MP_LIST_VIEW_AS_NONE;

	if (preference_get_int(PREF_MP_VIDEO_VIEW_AS_TYPE_KEY, &nValue) != 0) {
		VideoLogInfo("Setting normal list type because of defending wrong value.");
		mp_view_as_ctrl_set_type(MP_LIST_VIEW_AS_THUMBNAIL_LIST);
		return MP_LIST_VIEW_AS_THUMBNAIL_LIST;
	}

	if (nValue <= MP_LIST_VIEW_AS_NONE || nValue >= MP_LIST_VIEW_AS_MAX) {
		mp_view_as_ctrl_set_type(MP_LIST_VIEW_AS_THUMBNAIL_LIST);
		return MP_LIST_VIEW_AS_THUMBNAIL_LIST;
	}
	return nValue;
}


