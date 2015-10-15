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
#include "mp-video-value-define.h"
#include "mp-video-string-define.h"
#include "mp-video-list-sort-ctrl.h"
#include "mp-util-widget-ctrl.h"
#include "vp-util.h"

#define PREF_VP_VIDEOS_SORT_TYPE  "preference/org.tizen.videos/sort_type"

typedef struct _MpVideoListSort{
	//obj
	Evas_Object *pPopUpHandle;
	Evas_Object *pGroupRadio;
	Evas_Object *pGenList;
	Evas_Object *pBox;

	//data
	Elm_Genlist_Item_Class *stListSortItc;
	ListSortCtrlCbFunc ListSortCtrlUserCbFunc;
	MpListSortType nCurrListSortType;
	int nListCount;

}stMpVideoListSort;

stMpVideoListSort *g_pSortCtrlHandle = NULL;


static void __mp_sort_ctrl_realized(void *data, Evas_Object *obj, void *event_info);
static void __mp_sort_ctrl_rotate_cb(void *data, Evas_Object *obj, void *event_info);
static void __mp_sort_ctrl_btn_asc_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo);
static void __mp_sort_ctrl_mouse_up_cb(void *pUserData, Evas *pEvas, Evas_Object *pObject, void *pEventInfo);
static void  __mp_sort_ctrl_select_radio_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo);

////////////////////////////////////////////////////////////
// Internal function
////////////////////////////////////////////////////////////


void mp_sort_ctrl_delete_handle(void)
{
	VideoLogInfo("");

	if (!g_pSortCtrlHandle)
	{
		VideoLogError("");
		return;
	}
	if (g_pSortCtrlHandle->pPopUpHandle)
	{
		eext_object_event_callback_del(g_pSortCtrlHandle->pPopUpHandle, EEXT_CALLBACK_BACK, __mp_sort_ctrl_btn_asc_cb);
		//elm_object_style_set(pVideoListSortPopUpHandle, "content_no_vhpad");
		evas_object_smart_callback_del(g_pSortCtrlHandle->pPopUpHandle, "block,clicked", __mp_sort_ctrl_btn_asc_cb);
		evas_object_smart_callback_del((Evas_Object*)mp_util_get_main_window_handle(), "rotation,changed", __mp_sort_ctrl_rotate_cb);
		evas_object_event_callback_del(g_pSortCtrlHandle->pPopUpHandle, EVAS_CALLBACK_MOUSE_UP, __mp_sort_ctrl_mouse_up_cb);
	}
	MP_DEL_OBJ(g_pSortCtrlHandle->pGroupRadio);
	MP_DEL_OBJ(g_pSortCtrlHandle->pGenList);
	MP_DEL_OBJ(g_pSortCtrlHandle->pBox);
	MP_DEL_OBJ(g_pSortCtrlHandle->pPopUpHandle);
	MP_DEL_ITC(g_pSortCtrlHandle->stListSortItc);
	MP_FREE_STRING(g_pSortCtrlHandle);
}

void mp_sort_ctrl_set_sort_state(MpListSortType nListSortType)
{
	if (!g_pSortCtrlHandle)
	{
		VideoLogError("");
		return;
	}

	if (MP_LIST_SORT_BY_NONE >= nListSortType || nListSortType >= MP_LIST_SORT_BY_MAX)
	{
		VideoLogWarning("[WARNING] Value of sort type is not matched.");
		nListSortType = MP_LIST_SORT_BY_RECENTLY_ADDED;
	}

	g_pSortCtrlHandle->nCurrListSortType = nListSortType;

	int nRet = preference_set_int(PREF_VP_VIDEOS_SORT_TYPE, g_pSortCtrlHandle->nCurrListSortType);
	if (nRet)
	{
		VideoLogWarning("[WARNING] Fail to set value about sort type.");
	}
}

static char *mp_sort_ctrl_get_label_of_genlist_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{

	int nIndex = (int)pUserData;

	if (!strcmp(pPart, "elm.text") || !strcmp(pPart, "elm.text.main.left"))
	{
		switch (nIndex)
		{
			//case MP_LIST_SORT_BY_RECENTLY_VIEWED:	return strdup(VIDEOS_SORTBYOPT_OPT_RECENTLY_VIEWED);
			case MP_LIST_SORT_BY_RECENTLY_ADDED:	return strdup(VIDEOS_SORTBYOPT_OPT_DATE_MOST_RECENT);
			case MP_LIST_SORT_BY_NAME:			return strdup(VIDEOS_SORTBYOPT_OPT_NAME);
			//case MP_LIST_SORT_BY_SIZE:			return strdup(VIDEOS_SORTBYOPT_OPT_SIZE);
			//case MP_LIST_SORT_BY_TYPE:			return strdup(VIDEOS_SORTBYOPT_OPT_TYPE);
		}
	}

	return NULL;
}

static Evas_Object *mp_sort_ctrl_get_icon_of_genlist_cb(const void *pUserData, Evas_Object *pObject, const char *pPart)
{
	int nIndex = (int)pUserData;
	Evas_Object *pLayout = NULL;
	Evas_Object *pTmpRadio = NULL;
	if (!g_pSortCtrlHandle)
	{
		VideoLogError("");
		return NULL;
	}

	if (!strcmp(pPart, "elm.swallow.end") || !strcmp(pPart, "elm.icon.2"))
	{
		pLayout = elm_layout_add(pObject);
		pTmpRadio = elm_radio_add(pLayout);

		elm_layout_theme_set(pLayout, "layout", "list/C/type.2", "default");
		elm_radio_state_value_set(pTmpRadio, nIndex);
		elm_radio_group_add(pTmpRadio, g_pSortCtrlHandle->pGroupRadio);

		if (g_pSortCtrlHandle->nCurrListSortType == nIndex)
		{
			elm_radio_value_set(g_pSortCtrlHandle->pGroupRadio, g_pSortCtrlHandle->nCurrListSortType);
			mp_sort_ctrl_set_sort_state(g_pSortCtrlHandle->nCurrListSortType);
		}
//		elm_object_signal_callback_add(pTmpRadio, "elm,action,show,finished", "elm", __mp_sort_ctrl_select_radio_cb, (void *)pUserData);
		evas_object_smart_callback_add(pTmpRadio, "changed", __mp_sort_ctrl_select_radio_cb, (void *)pUserData);
		elm_object_part_content_set(pLayout, "elm.swallow.content", pTmpRadio);

		evas_object_show(pTmpRadio);
		evas_object_show(pLayout);
	}

	return pLayout;
}

static void __mp_sort_ctrl_btn_asc_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");

	mp_sort_ctrl_hide();
}

static void mp_sort_ctrl_select_item_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	VideoLogInfo("");
	if (!g_pSortCtrlHandle)
	{
		VideoLogError("");
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
	if (nIndex == g_pSortCtrlHandle->nCurrListSortType) {
		VideoLogInfo("Do nothing");
		mp_sort_ctrl_hide();
	} else {
		g_pSortCtrlHandle->nCurrListSortType = nIndex;
		elm_radio_value_set(g_pSortCtrlHandle->pGroupRadio, g_pSortCtrlHandle->nCurrListSortType);

		mp_sort_ctrl_set_sort_state(g_pSortCtrlHandle->nCurrListSortType);

		ListSortCtrlCbFunc fFunc = g_pSortCtrlHandle->ListSortCtrlUserCbFunc;
		mp_sort_ctrl_hide();

		if (fFunc)
		{
			fFunc();
		}
	}
}

static void  __mp_sort_ctrl_select_radio_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (!g_pSortCtrlHandle)
	{
		VideoLogError("");
		return;
	}
	int nIndex = (int)pUserData;

	g_pSortCtrlHandle->nCurrListSortType = nIndex;
	mp_sort_ctrl_set_sort_state(g_pSortCtrlHandle->nCurrListSortType);

	ListSortCtrlCbFunc fFunc = g_pSortCtrlHandle->ListSortCtrlUserCbFunc;
	mp_sort_ctrl_hide();

	if (fFunc)
	{
		fFunc();
	}
}

static void __mp_sort_ctrl_mouse_up_cb(void *pUserData, Evas *pEvas, Evas_Object *pObject, void *pEventInfo)
{
	Evas_Event_Mouse_Up *pEvent = pEventInfo;

	if (pEvent->button == 3)
	{
		mp_sort_ctrl_hide();
	}
}

////////////////////////////////////////////////////////////
// External function
////////////////////////////////////////////////////////////
void mp_sort_ctrl_genlist_change_language(void *data, Evas_Object *obj, void *event_info)
{
   elm_genlist_realized_items_update(obj);
}

static void __mp_sort_ctrl_realized(void *data, Evas_Object *obj, void *event_info)
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

static void __mp_sort_ctrl_rotate_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!g_pSortCtrlHandle)
	{
		VideoLogError("pVideoListSortPopUpHandle IS null");
		return;
	}
	if (g_pSortCtrlHandle->nListCount < VIDEO_POPUP_MIN_ITEMS)
	{
		return;
	}
	mp_widget_ctrl_set_popup_min_size(g_pSortCtrlHandle->pBox, g_pSortCtrlHandle->nListCount, VIDEOS_POPUP_114);

}

bool mp_sort_ctrl_show(ListSortCtrlCbFunc pListSortCtrlCb)
{
	VideoLogInfo("");

	mp_sort_ctrl_delete_handle();
	g_pSortCtrlHandle = (stMpVideoListSort*)calloc(1, sizeof(stMpVideoListSort));
	if (!g_pSortCtrlHandle)
	{
		VideoLogError("g_pSortCtrlHandle is NULL");
		return FALSE;
	}

	g_pSortCtrlHandle->ListSortCtrlUserCbFunc = pListSortCtrlCb;
	g_pSortCtrlHandle->nCurrListSortType = mp_sort_ctrl_get_sort_state();

	g_pSortCtrlHandle->pPopUpHandle = elm_popup_add(mp_util_get_main_window_handle());

	elm_object_style_set(g_pSortCtrlHandle->pPopUpHandle, "default");
	eext_object_event_callback_add(g_pSortCtrlHandle->pPopUpHandle, EEXT_CALLBACK_BACK, __mp_sort_ctrl_btn_asc_cb, NULL);
	evas_object_smart_callback_add(g_pSortCtrlHandle->pPopUpHandle, "block,clicked", __mp_sort_ctrl_btn_asc_cb, NULL);
	evas_object_smart_callback_add((Evas_Object*)mp_util_get_main_window_handle(), "rotation,changed", __mp_sort_ctrl_rotate_cb, NULL);

	elm_object_domain_translatable_part_text_set(g_pSortCtrlHandle->pPopUpHandle, "title,text", VIDEOS_STRING, VIDEOS_SORTBYPOP_HEADER_SORT_BY_IDS);
	evas_object_size_hint_weight_set(g_pSortCtrlHandle->pPopUpHandle, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	evas_object_event_callback_add(g_pSortCtrlHandle->pPopUpHandle, EVAS_CALLBACK_MOUSE_UP, __mp_sort_ctrl_mouse_up_cb, NULL);
	//evas_object_event_callback_add(pVideoListSortPopUpHandle, EVAS_CALLBACK_KEY_DOWN, MpVideoListSortKeyDownCb, NULL);

	MP_DEL_ITC(g_pSortCtrlHandle->stListSortItc);
	g_pSortCtrlHandle->stListSortItc = elm_genlist_item_class_new();
	//  Set item of Genlist.
	g_pSortCtrlHandle->stListSortItc->item_style = "default";//"1text.1icon.2/popup";
	g_pSortCtrlHandle->stListSortItc->func.text_get = (void *)mp_sort_ctrl_get_label_of_genlist_cb;
	g_pSortCtrlHandle->stListSortItc->func.content_get = (void *)mp_sort_ctrl_get_icon_of_genlist_cb;
	g_pSortCtrlHandle->stListSortItc->func.state_get = NULL;
	g_pSortCtrlHandle->stListSortItc->func.del = NULL;

	// Create genlist handle
	g_pSortCtrlHandle->pGenList = elm_genlist_add(g_pSortCtrlHandle->pPopUpHandle);
	evas_object_size_hint_weight_set(g_pSortCtrlHandle->pGenList, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pSortCtrlHandle->pGenList, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_smart_callback_add(g_pSortCtrlHandle->pGenList, "language,changed", mp_sort_ctrl_genlist_change_language, NULL);
	evas_object_smart_callback_add(g_pSortCtrlHandle->pGenList, "realized", __mp_sort_ctrl_realized, NULL);

	g_pSortCtrlHandle->pGroupRadio = elm_radio_add(g_pSortCtrlHandle->pGenList);

	mp_sort_ctrl_get_sort_state();

	int nListCount = 0;
	//elm_genlist_item_append(g_pSortCtrlHandle->pGenList, g_pSortCtrlHandle->stListSortItc, (void *)MP_LIST_SORT_BY_RECENTLY_VIEWED, NULL, ELM_GENLIST_ITEM_NONE, mp_sort_ctrl_select_item_cb, NULL);
	//nListCount++;
	elm_genlist_item_append(g_pSortCtrlHandle->pGenList, g_pSortCtrlHandle->stListSortItc, (void *)MP_LIST_SORT_BY_RECENTLY_ADDED, NULL, ELM_GENLIST_ITEM_NONE, mp_sort_ctrl_select_item_cb, NULL);
	nListCount++;
	elm_genlist_item_append(g_pSortCtrlHandle->pGenList, g_pSortCtrlHandle->stListSortItc, (void *)MP_LIST_SORT_BY_NAME, NULL, ELM_GENLIST_ITEM_NONE, mp_sort_ctrl_select_item_cb, NULL);
	nListCount++;
	//elm_genlist_item_append(g_pSortCtrlHandle->pGenList, g_pSortCtrlHandle->stListSortItc, (void *)MP_LIST_SORT_BY_SIZE, NULL, ELM_GENLIST_ITEM_NONE, mp_sort_ctrl_select_item_cb, NULL);
	//nListCount++;
	//elm_genlist_item_append(g_pSortCtrlHandle->pGenList, g_pSortCtrlHandle->stListSortItc, (void *)MP_LIST_SORT_BY_TYPE, NULL, ELM_GENLIST_ITEM_NONE, mp_sort_ctrl_select_item_cb, NULL);
	//nListCount++;
	g_pSortCtrlHandle->nListCount = nListCount;

	//elm_radio_value_set(g_pSortCtrlHandle->pGroupRadio, g_pSortCtrlHandle->nCurrListSortType);

	g_pSortCtrlHandle->pBox = elm_box_add(g_pSortCtrlHandle->pPopUpHandle);

	mp_widget_ctrl_set_popup_min_size(g_pSortCtrlHandle->pBox, g_pSortCtrlHandle->nListCount, VIDEOS_POPUP_114);
	evas_object_size_hint_weight_set(g_pSortCtrlHandle->pBox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pSortCtrlHandle->pBox, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_box_pack_end(g_pSortCtrlHandle->pBox, g_pSortCtrlHandle->pGenList);
	elm_object_content_set(g_pSortCtrlHandle->pPopUpHandle, g_pSortCtrlHandle->pBox);

	evas_object_show(g_pSortCtrlHandle->pBox);
	evas_object_show(g_pSortCtrlHandle->pGenList);
	evas_object_show(g_pSortCtrlHandle->pPopUpHandle);

	return TRUE;
}

void mp_sort_ctrl_hide(void)
{
	VideoLogInfo("");

	mp_sort_ctrl_delete_handle();
}

void mp_sort_ctrl_change_language(void)
{
	VideoLogInfo("");
	if (!g_pSortCtrlHandle)
	{
		VideoLogError("");
		return;
	}

	if (!g_pSortCtrlHandle->pGenList || !g_pSortCtrlHandle->pPopUpHandle)
	{
		VideoLogError("No existed genlist or popup handle.");
		return;
	}

	elm_genlist_realized_items_update(g_pSortCtrlHandle->pGenList);
}


int mp_sort_ctrl_get_sort_state(void)
{
	int nValue = MP_LIST_SORT_BY_NONE;

	if (0 != preference_get_int(PREF_VP_VIDEOS_SORT_TYPE, &nValue))
	{
		VideoLogInfo("Setting normal list type because of defending wrong value.:%d", nValue);
		mp_sort_ctrl_set_sort_state(MP_LIST_SORT_BY_RECENTLY_ADDED);
		return MP_LIST_SORT_BY_RECENTLY_ADDED;
	}

	if (nValue <= MP_LIST_SORT_BY_NONE || nValue >= MP_LIST_SORT_BY_MAX)
	{
		mp_sort_ctrl_set_sort_state(MP_LIST_SORT_BY_RECENTLY_ADDED);
		return MP_LIST_SORT_BY_RECENTLY_ADDED;
	}

	VideoLogInfo("sortstate start:%d", nValue);

	return (int)nValue;
}

