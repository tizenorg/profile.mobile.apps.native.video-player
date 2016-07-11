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
#include "mp-video-list-auto-play-ctrl.h"
#include "mp-util-widget-ctrl.h"
#include "vp-util.h"

#define PREF_MP_VIDEO_AUTO_PLAY_TYPE_KEY  "preference/org.tizen.videos/auto_play_type"

typedef struct _MpVideoAutoPlay{
	//obj
	Evas_Object *pPopUpHandle;
	Evas_Object *pGroupRadio;
	Evas_Object *pGenList;
	Evas_Object *pBox;

	//data
	Elm_Genlist_Item_Class *stAutoPlayItc;
	MpListAutoPlaySelect nAutoPlaySelect;
}stMpVideoAutoPlay;

stMpVideoAutoPlay *g_pAutoPlayHandle = NULL;

////////////////////////////////////////////////////////////
// Internal function
////////////////////////////////////////////////////////////
static void __mp_auto_play_ctrl_rotate_cb(void *data, Evas_Object *obj, void *event_info);
static void __mp_auto_play_ctrl_mouse_up_cb(void *pUserData, Evas *pEvas, Evas_Object *pObject, void *pEventInfo);
static void __mp_auto_play_ctrl_cancel_btn_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo);
static void __mp_auto_play_ctrl_select_item_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo);
static void  __mp_auto_play_ctrl_select_radio_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo);

/*void mp_auto_play_config_set_auto_play_status(bool bMultiPlay)
{
	int nErr = 0;

	nErr = preference_set_boolean(PREF_MP_VIDEO_AUTO_PLAY_TYPE_KEY, bMultiPlay);
	if (nErr != 0) {
		VideoLogError("failed to set MULTI PLAY FLAG [0x%x]", nErr);
		return FALSE;
	}

	return TRUE;
}*/

bool mp_auto_play_config_get_auto_play_status()
{
	VideoLogInfo("");
	int nErr = 0;
	bool nVal = 0;
	nErr = preference_get_boolean(PREF_MP_VIDEO_AUTO_PLAY_TYPE_KEY, &nVal);
	if (nErr != 0) {
		VideoLogError("failed to get MULTI PLAY FLAG [0x%x]", nErr);
		return FALSE;
	}
	if(nVal)
		return TRUE;
	else
		return FALSE;
}
void mp_auto_play_ctrl_delete_handle(void)
{
	VideoLogInfo("");
	if (!g_pAutoPlayHandle)
	{
		VideoLogError("-------g_pAutoPlayHandle is NULL--------");
		return;
	}
	if (g_pAutoPlayHandle->pPopUpHandle)
	{
		eext_object_event_callback_del(g_pAutoPlayHandle->pPopUpHandle, EEXT_CALLBACK_BACK, __mp_auto_play_ctrl_cancel_btn_cb);
		evas_object_event_callback_del(g_pAutoPlayHandle->pPopUpHandle, EVAS_CALLBACK_MOUSE_UP, __mp_auto_play_ctrl_mouse_up_cb);
		evas_object_smart_callback_del(g_pAutoPlayHandle->pPopUpHandle, "block,clicked", __mp_auto_play_ctrl_cancel_btn_cb);
		evas_object_smart_callback_del((Evas_Object*)mp_util_get_main_window_handle(), "rotation,changed", __mp_auto_play_ctrl_rotate_cb);
	}
	MP_DEL_OBJ(g_pAutoPlayHandle->pGroupRadio);
	MP_DEL_OBJ(g_pAutoPlayHandle->pGenList);
	MP_DEL_OBJ(g_pAutoPlayHandle->pBox);
	MP_DEL_OBJ(g_pAutoPlayHandle->pPopUpHandle);
	MP_DEL_ITC(g_pAutoPlayHandle->stAutoPlayItc);
	MP_FREE_STRING(g_pAutoPlayHandle);

}

void mp_auto_play_ctrl_set_type(MpListAutoPlaySelect nAutoPlaySelect)
{
	if (!g_pAutoPlayHandle)
	{
		VideoLogError("");
		return;
	}

	if (MP_LIST_AUTO_PLAY_NONE >= nAutoPlaySelect || nAutoPlaySelect >= MP_LIST_AUTO_PLAY_MAX)
	{
		VideoLogWarning("[WARNING] Value of Auto Play selection is not matched.");
		g_pAutoPlayHandle->nAutoPlaySelect = MP_LIST_AUTO_PLAY_OFF;
	}

	g_pAutoPlayHandle->nAutoPlaySelect = nAutoPlaySelect;



}

static char *__mp_auto_play_ctrl_get_label_of_genlsit_item_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{

	int nIndex = (int)pUserData;

	if (!strcmp(pPart, "elm.text") || !strcmp(pPart, "elm.text.main.left"))
	{
		switch (nIndex)
		{
			case MP_LIST_AUTO_PLAY_ON:	return strdup(VIDEOS_STRING_COM_ON);
			case MP_LIST_AUTO_PLAY_OFF:		return strdup(VIDEOS_STRING_COM_OFF);
		}
	}
	return NULL;
}

static Evas_Object *__mp_auto_play_ctrl_get_icon_of_genlsit_item_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{

	if (!g_pAutoPlayHandle)
	{
		VideoLogError("");
		return NULL;
	}

	int nIndex = (int)pUserData;
	Evas_Object *pTmpRadio = NULL;
	/*Evas_Object *pLayout	= NULL;*/

	if (!strcmp(pPart, "elm.swallow.end") || !strcmp(pPart, "elm.icon.2"))
	{
		//pLayout = elm_layout_add(pObject);
		//elm_layout_theme_set(pLayout, "layout", "list/C/type.2", "default");

		//pTmpRadio = elm_radio_add(pLayout);
		pTmpRadio = elm_radio_add(pObject);
		elm_radio_state_value_set(pTmpRadio, nIndex);
		elm_radio_group_add(pTmpRadio, g_pAutoPlayHandle->pGroupRadio);

		if (g_pAutoPlayHandle->nAutoPlaySelect == nIndex)
		{
			elm_radio_value_set(g_pAutoPlayHandle->pGroupRadio, g_pAutoPlayHandle->nAutoPlaySelect);
			mp_auto_play_ctrl_set_type(g_pAutoPlayHandle->nAutoPlaySelect);
		}

		evas_object_smart_callback_add(pTmpRadio, "changed", __mp_auto_play_ctrl_select_radio_cb, (void *)pUserData);
		//elm_layout_content_set(pLayout, "elm.swallow.content", pTmpRadio);

		evas_object_show(pTmpRadio);
		//evas_object_show(pLayout);

	}
	//return pLayout;
	return pTmpRadio;

}

static void __mp_auto_play_ctrl_cancel_btn_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");

	mp_auto_play_ctrl_hide();
}

static void __mp_auto_play_ctrl_select_item_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");

	if (!g_pAutoPlayHandle)
	{
		VideoLogError("g_pAutoPlayHandle is ?NULL");
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
	VideoLogError("====================nindex=%d",nIndex);
	if(nIndex == 1) {
		VideoLogError("====================vp_set_auto_play=%true");
		vp_set_auto_play(true);
	}
	else
	{
		VideoLogError("====================vp_set_auto_play=%false");
		vp_set_auto_play(false);
	}
	if(vp_is_auto_play_on())
	{
		VideoLogError("====================found vp_set_auto_play=%true");
	}
	if (nIndex == g_pAutoPlayHandle->nAutoPlaySelect) {
		VideoLogInfo("Current state");
		mp_auto_play_ctrl_hide();
	} else {
		g_pAutoPlayHandle->nAutoPlaySelect = nIndex;
		elm_radio_value_set(g_pAutoPlayHandle->pGroupRadio, g_pAutoPlayHandle->nAutoPlaySelect);

		mp_auto_play_ctrl_set_type(g_pAutoPlayHandle->nAutoPlaySelect);

		/*ListAutoPlayCtrlCbFunc fFunc = g_pAutoPlayHandle->ListAutoPlayCtrlUserCbFunc;*/
		mp_auto_play_ctrl_hide();
/*
		if (fFunc)
		{
			fFunc();
		}*/
	}
}

static void  __mp_auto_play_ctrl_select_radio_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{

	if (!g_pAutoPlayHandle)
	{
		VideoLogError("");
		return;
	}

	int nIndex = (int)pUserData;

	VideoLogError("====================nindex=%d",nIndex);
	if(nIndex == 1) {
		VideoLogError("====================vp_set_auto_play=true");
		vp_set_auto_play(true);
	}
	else
	{
		VideoLogError("====================vp_set_auto_play=false");
		vp_set_auto_play(false);
	}
	if(vp_is_auto_play_on())
	{
		VideoLogError("====================found vp_set_auto_play=true");
	}
	if (nIndex == g_pAutoPlayHandle->nAutoPlaySelect) {
		VideoLogInfo("Current state");
		mp_auto_play_ctrl_hide();
	} else {
		g_pAutoPlayHandle->nAutoPlaySelect = nIndex;
		elm_radio_value_set(g_pAutoPlayHandle->pGroupRadio, g_pAutoPlayHandle->nAutoPlaySelect);

		mp_auto_play_ctrl_set_type(g_pAutoPlayHandle->nAutoPlaySelect);

		/*ListAutoPlayCtrlCbFunc fFunc = g_pAutoPlayHandle->ListAutoPlayCtrlUserCbFunc;*/
		mp_auto_play_ctrl_hide();
	}

}

static void __mp_auto_play_ctrl_mouse_up_cb(void *pUserData, Evas *pEvas, Evas_Object *pObject, void *pEventInfo)
{
	Evas_Event_Mouse_Up *pEvent = pEventInfo;

	if (pEvent->button == 3)
	{
		mp_auto_play_ctrl_hide();
	}
}

static void __mp_auto_play_ctrl_realized(void *data, Evas_Object *obj, void *event_info)
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


static void __mp_auto_play_ctrl_rotate_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!g_pAutoPlayHandle)
	{
		VideoLogError("pVideoListSortPopUpHandle IS null");
		return;
	}
	/*if (g_pAutoPlayHandle->nListCount < VIDEO_POPUP_MIN_ITEMS)
	{
		return;
	}*/
	mp_widget_ctrl_set_popup_min_size(g_pAutoPlayHandle->pBox,2, VIDEOS_POPUP_114);

}

////////////////////////////////////////////////////////////
// External function
////////////////////////////////////////////////////////////

bool mp_auto_play_ctrl_show()
{
	VideoLogInfo("");



	mp_auto_play_ctrl_delete_handle();
	g_pAutoPlayHandle = (stMpVideoAutoPlay*)calloc(1, sizeof(stMpVideoAutoPlay));
	if (!g_pAutoPlayHandle)
	{
		VideoLogError("g_pAutoPlayHandle is NULL");
		return FALSE;
	}

	//g_pAutoPlayHandle->ListAutoPlayCtrlUserCbFunc = pListAutoPlayCtrlCb;
	g_pAutoPlayHandle->nAutoPlaySelect = MP_LIST_AUTO_PLAY_OFF;
	g_pAutoPlayHandle->nAutoPlaySelect = mp_auto_play_ctrl_get_type();

	g_pAutoPlayHandle->pPopUpHandle = elm_popup_add(mp_util_get_main_window_handle());


	//elm_object_style_set(pListAutoPlayPopUpHandle, "content_no_vhpad"); //menustyle
	elm_object_style_set(g_pAutoPlayHandle->pPopUpHandle, "default"); //menustyle
	elm_object_domain_translatable_part_text_set(g_pAutoPlayHandle->pPopUpHandle, "title,text", PACKAGE, VIDEOS_MENU_AUTO_PLAY_NEXT_POP_HEADER);
	evas_object_size_hint_weight_set(g_pAutoPlayHandle->pPopUpHandle, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	eext_object_event_callback_add(g_pAutoPlayHandle->pPopUpHandle, EEXT_CALLBACK_BACK, __mp_auto_play_ctrl_cancel_btn_cb, NULL);
	evas_object_event_callback_add(g_pAutoPlayHandle->pPopUpHandle, EVAS_CALLBACK_MOUSE_UP, __mp_auto_play_ctrl_mouse_up_cb, NULL);
	evas_object_smart_callback_add(g_pAutoPlayHandle->pPopUpHandle, "block,clicked", __mp_auto_play_ctrl_cancel_btn_cb, NULL);
	evas_object_smart_callback_add((Evas_Object*)mp_util_get_main_window_handle(), "rotation,changed", __mp_auto_play_ctrl_rotate_cb, NULL);

	//  Set item of Genlist.
	MP_DEL_ITC(g_pAutoPlayHandle->stAutoPlayItc);
	g_pAutoPlayHandle->stAutoPlayItc = elm_genlist_item_class_new();
	g_pAutoPlayHandle->stAutoPlayItc->item_style = "default";	//"1text.1icon.2/popup";
	g_pAutoPlayHandle->stAutoPlayItc->func.text_get = (void *)__mp_auto_play_ctrl_get_label_of_genlsit_item_cb;
	g_pAutoPlayHandle->stAutoPlayItc->func.content_get = (void *)__mp_auto_play_ctrl_get_icon_of_genlsit_item_cb;
	g_pAutoPlayHandle->stAutoPlayItc->func.state_get = NULL;
	g_pAutoPlayHandle->stAutoPlayItc->func.del = NULL;

	// Create genlist handle
	g_pAutoPlayHandle->pGenList = elm_genlist_add(g_pAutoPlayHandle->pPopUpHandle);
	evas_object_size_hint_weight_set(g_pAutoPlayHandle->pGenList, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pAutoPlayHandle->pGenList, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(g_pAutoPlayHandle->pGenList, "realized", __mp_auto_play_ctrl_realized, NULL);

	g_pAutoPlayHandle->pGroupRadio = elm_radio_add(g_pAutoPlayHandle->pGenList);
	bool pref;
	pref = mp_auto_play_config_get_auto_play_status();
		if(pref)
		{
			g_pAutoPlayHandle->nAutoPlaySelect = MP_LIST_AUTO_PLAY_ON;
		}
		else
		{
			g_pAutoPlayHandle->nAutoPlaySelect = MP_LIST_AUTO_PLAY_OFF;
		}
	elm_radio_value_set(g_pAutoPlayHandle->pGroupRadio, g_pAutoPlayHandle->nAutoPlaySelect);
	int nMax_view_list = 0;
/*#ifdef VS_FEATURE_LIST_VIEW
	elm_genlist_item_append(g_pAutoPlayHandle->pGenList, g_pAutoPlayHandle->stAutoPlayItc, (void *)MP_LIST_AUTO_PLAY_NORMAL_LIST, NULL, ELM_GENLIST_ITEM_NONE, __mp_auto_play_ctrl_select_item_cb, NULL);
	nMax_view_list++;
#endif
#ifdef VS_FEATURE_THUMBNAIL_VIEW
	elm_genlist_item_append(g_pAutoPlayHandle->pGenList, g_pAutoPlayHandle->stAutoPlayItc, (void *)MP_LIST_AUTO_PLAY_THUMBNAIL_LIST, NULL, ELM_GENLIST_ITEM_NONE, __mp_auto_play_ctrl_select_item_cb, NULL);
	nMax_view_list++;
#endif
#ifdef ENABLE_STORE_VIEWAS
	if (FOOTER_TAB_TYPE_PERSONAL == g_pAutoPlayHandle->nAutoPlaySelect)
	{
		elm_genlist_item_append(g_pAutoPlayHandle->pGenList, g_pAutoPlayHandle->stAutoPlayItc, (void *)MP_LIST_AUTO_PLAY_FOLDER_LIST, NULL, ELM_GENLIST_ITEM_NONE, __mp_auto_play_ctrl_select_item_cb, NULL);
		nMax_view_list++;
	}
#else
*/
	if(!vp_is_auto_play_on())
	vp_set_auto_play(false);
	elm_genlist_item_append(g_pAutoPlayHandle->pGenList, g_pAutoPlayHandle->stAutoPlayItc, (void *)MP_LIST_AUTO_PLAY_ON, NULL, ELM_GENLIST_ITEM_NONE, __mp_auto_play_ctrl_select_item_cb, NULL);
	elm_genlist_item_append(g_pAutoPlayHandle->pGenList, g_pAutoPlayHandle->stAutoPlayItc, (void *)MP_LIST_AUTO_PLAY_OFF, NULL, ELM_GENLIST_ITEM_NONE, __mp_auto_play_ctrl_select_item_cb, NULL);
	nMax_view_list++;




	g_pAutoPlayHandle->pBox = elm_box_add(g_pAutoPlayHandle->pPopUpHandle);

	mp_widget_ctrl_set_popup_min_size(g_pAutoPlayHandle->pBox, 2, VIDEOS_POPUP_114);
	evas_object_size_hint_weight_set(g_pAutoPlayHandle->pBox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pAutoPlayHandle->pBox, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_box_pack_end(g_pAutoPlayHandle->pBox, g_pAutoPlayHandle->pGenList);
	elm_object_content_set(g_pAutoPlayHandle->pPopUpHandle, g_pAutoPlayHandle->pBox);

	evas_object_show(g_pAutoPlayHandle->pGenList);
	evas_object_show(g_pAutoPlayHandle->pPopUpHandle);

	return TRUE;
}

void mp_auto_play_ctrl_hide(void)
{
	VideoLogInfo("");

	mp_auto_play_ctrl_delete_handle();
}

void mp_auto_play_ctrl_change_language(void)
{
	if (!g_pAutoPlayHandle)
	{
		VideoLogError("g_pAutoPlayHandle is NULL");
		return;
	}

	if (!g_pAutoPlayHandle->pGenList || !g_pAutoPlayHandle->pPopUpHandle)
	{
		VideoLogInfo("No existed genlist or popup handle.");
		return;
	}

	VideoLogInfo("");

	elm_object_domain_translatable_part_text_set(g_pAutoPlayHandle->pPopUpHandle, "title,text", PACKAGE, VIDEOS_MENU_AUTO_PLAY_NEXT_POP_HEADER);
	elm_genlist_realized_items_update(g_pAutoPlayHandle->pGenList);

}

int mp_auto_play_ctrl_get_type(void)
{
	//int nValue = MP_LIST_AUTO_PLAY_NONE;

	/*if (preference_get_int(PREF_MP_VIDEO_AUTO_PLAY_TYPE_KEY, &nValue) != 0) {
		VideoLogInfo("Setting normal list type because of defending wrong value.");
		mp_auto_play_ctrl_set_type(MP_LIST_AUTO_PLAY_THUMBNAIL_LIST);
		return MP_LIST_AUTO_PLAY_THUMBNAIL_LIST;
	}

	if (nValue <= MP_LIST_AUTO_PLAY_NONE || nValue >= MP_LIST_AUTO_PLAY_MAX) {
		mp_auto_play_ctrl_set_type(MP_LIST_AUTO_PLAY_THUMBNAIL_LIST);
		return MP_LIST_AUTO_PLAY_THUMBNAIL_LIST;
	}*/
	return g_pAutoPlayHandle->nAutoPlaySelect;
}


