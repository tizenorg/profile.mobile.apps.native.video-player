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
#include <Ecore_Evas.h>
#include <Elementary.h>

#include "mp-util.h"
#include "mp-video-log.h"
#include "video-player.h"
#include "mp-video-value-define.h"
#include "mp-video-string-define.h"
#include "mp-video-list-option-ctrl.h"
#include "mp-util-media-service.h"
#include "vp-util.h"
#include "mp-util-widget-ctrl.h"


typedef struct _MpVideoListOption {
	/*obj*/
	Evas_Object *pPopUpHandle;
	Evas_Object *pGenList;
	Evas_Object *pBox;

	/*data*/
	MpDownloadType stDownloadType;
	Elm_Genlist_Item_Class *stListOptionItc;
	st_RegisteOptionCbFunc st_OptionCbFun;
	MpListOptionCbFunc pMpListOptionCloseCb;
	int nListCount;
	void *pUserData;

} stMpVideoListOption;

static stMpVideoListOption *g_pOptionCtrlHandle = NULL;


/*static void __MpListOptionCtrlDismissedCb(void *data, Evas_Object *obj, void *event_info);*/
static void __mp_option_ctrl_realized(void *data, Evas_Object *obj,
				      void *event_info);
static void __mp_option_ctrl_cancel_cb(void *pUserData,
				       Evas_Object *pObject,
				       void *pEventInfo);
static void __mp_option_ctrl_mouse_up_cb(void *pUserData, Evas *pEvas,
		Evas_Object *pObject,
		void *pEventInfo);
static void __mp_option_ctrl_rotate_cb(void *data, Evas_Object *obj,
				       void *event_info);

void mp_option_ctrl_delete_handle(void)
{
	VideoLogInfo("");

	if (!g_pOptionCtrlHandle) {
		VideoLogError("");
		return;
	}

	if (g_pOptionCtrlHandle->pMpListOptionCloseCb) {
		g_pOptionCtrlHandle->pMpListOptionCloseCb(g_pOptionCtrlHandle->
				pUserData);
	}

	if (g_pOptionCtrlHandle->pPopUpHandle) {
		evas_object_event_callback_del(g_pOptionCtrlHandle->pPopUpHandle,
					       EVAS_CALLBACK_MOUSE_UP,
					       __mp_option_ctrl_mouse_up_cb);
		evas_object_smart_callback_del(g_pOptionCtrlHandle->pPopUpHandle,
					       "block,clicked",
					       __mp_option_ctrl_cancel_cb);
		eext_object_event_callback_del(g_pOptionCtrlHandle->pPopUpHandle,
					       EEXT_CALLBACK_BACK,
					       __mp_option_ctrl_cancel_cb);
		evas_object_smart_callback_del((Evas_Object *)
					       mp_util_get_main_window_handle(),
					       "rotation,changed",
					       __mp_option_ctrl_rotate_cb);
	}
	MP_DEL_OBJ(g_pOptionCtrlHandle->pGenList);
	MP_DEL_OBJ(g_pOptionCtrlHandle->pBox);
	MP_DEL_OBJ(g_pOptionCtrlHandle->pPopUpHandle);
	MP_DEL_ITC(g_pOptionCtrlHandle->stListOptionItc);
	MP_FREE_STRING(g_pOptionCtrlHandle);

}

/*static void __MpListOptionCtrlDismissedCb(void *data, Evas_Object *obj, void *event_info)
{
	VideoSecureLogDebug("__MpListOptionCtrlDismissedCb");

	mp_option_ctrl_delete_handle();
}*/

static void __mp_option_ctrl_register_cb_func(st_RegisteOptionCbFunc *
		pCallbackFunc)
{

	if (!pCallbackFunc || !g_pOptionCtrlHandle) {
		VideoLogError("pCallbackFunc == NULL");
		return;
	}

	memset(&g_pOptionCtrlHandle->st_OptionCbFun, 0,
	       sizeof(st_RegisteOptionCbFunc));

	if (pCallbackFunc->ShareViaItemCb) {
		g_pOptionCtrlHandle->st_OptionCbFun.ShareViaItemCb =
			pCallbackFunc->ShareViaItemCb;
	}

	if (pCallbackFunc->AddToHomeItemCb) {
		g_pOptionCtrlHandle->st_OptionCbFun.AddToHomeItemCb =
			pCallbackFunc->AddToHomeItemCb;
	}

	if (pCallbackFunc->DeleteItemCb) {
		g_pOptionCtrlHandle->st_OptionCbFun.DeleteItemCb =
			pCallbackFunc->DeleteItemCb;
	}

	if (pCallbackFunc->DownloadCb) {
		g_pOptionCtrlHandle->st_OptionCbFun.DownloadCb =
			pCallbackFunc->DownloadCb;
	}
}

static char *mp_option_ctrl_get_lable_of_gen_item_cb(const void
		*pUserData,
		Evas_Object *
		pObject,
		const char *pPart)
{

	int nIndex = (int) pUserData;

	/*      if (!strcmp(pPart, "elm.text"))*/
	if (!strcmp(pPart, "elm.text.main.left")) {
		switch (nIndex) {
		case MP_LIST_OPTION_SHARE_VIA_ITEM:
			return g_strdup(VIDEOS_CTXPOP_MENU_SHARE);
		case MP_LIST_OPTION_ADD_HOME_ITEM:
			return g_strdup(VIDEOS_CTXPOP_MENU_ADD_TO_HOME);
		case MP_LIST_OPTION_RENAME_ITEM:
			return g_strdup(VIDEOS_CTXPOP_MENU_RENAME);
		case MP_LIST_OPTION_DETAILS_ITEM:
			return g_strdup(VIDEOS_CTXPOP_MENU_DETAILS);
		case MP_LIST_OPTION_DELETE_ITEM:
			return g_strdup(VIDEOS_CTXPOP_MENU_DELETE);
		case MP_LIST_OPTION_DOWNLOAD_ITEM:
			return g_strdup(VIDEOS_CTXPOP_MENU_DOWNLOAD);
		default:
			break;
		}
	}

	return NULL;
}

static void __mp_option_ctrl_realized(void *data, Evas_Object *obj,
				      void *event_info)
{
	if (!event_info || !obj) {
		VideoLogError("event info is invalid");
		return;
	}
	Elm_Object_Item *pItem = elm_genlist_last_item_get(obj);

	if (pItem) {
		elm_object_item_signal_emit(pItem, "elm,state,bottomline,hide",
					    "");
	}
}

static Evas_Object *__mp_option_ctrl_init(void *pNaviHandle,
		const char *title,
		int nVideoIndex)
{
	VideoLogDebug("title=%s", title);
	if (!g_pOptionCtrlHandle) {
		VideoLogError("g_pOptionCtrlHandle == NULL");
		return NULL;
	}

	MP_DEL_ITC(g_pOptionCtrlHandle->stListOptionItc);
	g_pOptionCtrlHandle->stListOptionItc = elm_genlist_item_class_new();

	/*  Set item of Genlist.*/
	g_pOptionCtrlHandle->stListOptionItc->item_style = "1line";	/*"1text/popup";*/
	g_pOptionCtrlHandle->stListOptionItc->func.text_get =
		(void *) mp_option_ctrl_get_lable_of_gen_item_cb;
	g_pOptionCtrlHandle->stListOptionItc->func.content_get = NULL;
	g_pOptionCtrlHandle->stListOptionItc->func.state_get = NULL;
	g_pOptionCtrlHandle->stListOptionItc->func.del = NULL;

	/* Create genlist handle*/
	g_pOptionCtrlHandle->pGenList =
		elm_genlist_add(g_pOptionCtrlHandle->pPopUpHandle);
	evas_object_size_hint_weight_set(g_pOptionCtrlHandle->pGenList,
					 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pOptionCtrlHandle->pGenList,
					EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(g_pOptionCtrlHandle->pGenList,
				       "realized", __mp_option_ctrl_realized,
				       NULL);

	g_pOptionCtrlHandle->nListCount = 0;

	if (g_pOptionCtrlHandle->st_OptionCbFun.DeleteItemCb) {
		g_pOptionCtrlHandle->nListCount++;
		elm_genlist_item_append(g_pOptionCtrlHandle->pGenList,
					g_pOptionCtrlHandle->stListOptionItc,
					(void *) MP_LIST_OPTION_DELETE_ITEM, NULL,
					ELM_GENLIST_ITEM_NONE,
					g_pOptionCtrlHandle->st_OptionCbFun.
					DeleteItemCb, (void *) nVideoIndex);
	}

	if (g_pOptionCtrlHandle->st_OptionCbFun.ShareViaItemCb) {
		g_pOptionCtrlHandle->nListCount++;
		elm_genlist_item_append(g_pOptionCtrlHandle->pGenList,
					g_pOptionCtrlHandle->stListOptionItc,
					(void *) MP_LIST_OPTION_SHARE_VIA_ITEM,
					NULL, ELM_GENLIST_ITEM_NONE,
					g_pOptionCtrlHandle->st_OptionCbFun.
					ShareViaItemCb, (void *) nVideoIndex);
	}

	evas_object_show(g_pOptionCtrlHandle->pGenList);

	g_pOptionCtrlHandle->pBox =
		elm_box_add(g_pOptionCtrlHandle->pPopUpHandle);

	mp_widget_ctrl_set_popup_min_size(g_pOptionCtrlHandle->pBox,
					  g_pOptionCtrlHandle->nListCount,
					  VIDEOS_POPUP_114);
	evas_object_size_hint_weight_set(g_pOptionCtrlHandle->pBox,
					 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pOptionCtrlHandle->pBox,
					EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_box_pack_end(g_pOptionCtrlHandle->pBox,
			 g_pOptionCtrlHandle->pGenList);

	evas_object_show(g_pOptionCtrlHandle->pBox);

	return g_pOptionCtrlHandle->pBox;

}

static void __mp_option_ctrl_cancel_cb(void *pUserData,
				       Evas_Object *pObject,
				       void *pEventInfo)
{
	VideoLogInfo("");

	mp_option_ctrl_delete_handle();
}

static void __mp_option_ctrl_mouse_up_cb(void *pUserData, Evas *pEvas,
		Evas_Object *pObject,
		void *pEventInfo)
{
	Evas_Event_Mouse_Up *pEvent = pEventInfo;

	if (pEvent->button == 3) {
		mp_option_ctrl_delete_handle();
	}
}

static void __mp_option_ctrl_rotate_cb(void *data, Evas_Object *obj,
				       void *event_info)
{
	if (!g_pOptionCtrlHandle) {
		VideoLogError("pVideoListSortPopUpHandle IS null");
		return;
	}
	if (g_pOptionCtrlHandle->nListCount < VIDEO_POPUP_MIN_ITEMS) {
		return;
	}
	mp_widget_ctrl_set_popup_min_size(g_pOptionCtrlHandle->pBox,
					  g_pOptionCtrlHandle->nListCount,
					  VIDEOS_POPUP_114);

}


bool mp_option_ctrl_show(void *pNaviHandle, const char *title,
			 int nVideoIndex,
			 st_RegisteOptionCbFunc *pCallbackFunc,
			 MpListOptionCbFunc pCloseFun, void *userData,
			 MpDownloadType euType)
{
	if (!pNaviHandle) {
		VideoSecureLogError("invalid pNaviHandle");
		return false;
	}

	mp_option_ctrl_delete_handle();
	g_pOptionCtrlHandle =
		(stMpVideoListOption *) calloc(1, sizeof(stMpVideoListOption));
	if (!g_pOptionCtrlHandle) {
		VideoLogError("g_pOptionCtrlHandle is NULL");
		return FALSE;
	}

	g_pOptionCtrlHandle->pMpListOptionCloseCb = pCloseFun;
	__mp_option_ctrl_register_cb_func(pCallbackFunc);
	g_pOptionCtrlHandle->stDownloadType = euType;
	g_pOptionCtrlHandle->pUserData = userData;

	g_pOptionCtrlHandle->pPopUpHandle =
		elm_popup_add((Evas_Object *) pNaviHandle);

	/*elm_object_style_set(pListOptionPopUpHandle, "content_no_vhpad"); //menustyle*/
	elm_object_style_set(g_pOptionCtrlHandle->pPopUpHandle, "default");
	if (title) {
		char *szTitleUtf8 = elm_entry_utf8_to_markup(title);
		elm_object_part_text_set(g_pOptionCtrlHandle->pPopUpHandle,
					 "title,text", szTitleUtf8);
		MP_FREE_STRING(szTitleUtf8);
	}
	evas_object_size_hint_weight_set(g_pOptionCtrlHandle->pPopUpHandle,
					 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_weight_set(g_pOptionCtrlHandle->pPopUpHandle,
					 EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_event_callback_add(g_pOptionCtrlHandle->pPopUpHandle,
				       EVAS_CALLBACK_MOUSE_UP,
				       __mp_option_ctrl_mouse_up_cb, NULL);
	evas_object_smart_callback_add(g_pOptionCtrlHandle->pPopUpHandle,
				       "block,clicked",
				       __mp_option_ctrl_cancel_cb, NULL);
	eext_object_event_callback_add(g_pOptionCtrlHandle->pPopUpHandle,
				       EEXT_CALLBACK_BACK,
				       __mp_option_ctrl_cancel_cb, NULL);
	evas_object_smart_callback_add((Evas_Object *)
				       mp_util_get_main_window_handle(),
				       "rotation,changed",
				       __mp_option_ctrl_rotate_cb, NULL);

	__mp_option_ctrl_init(pNaviHandle, title, nVideoIndex);


	elm_object_content_set(g_pOptionCtrlHandle->pPopUpHandle,
			       g_pOptionCtrlHandle->pBox);
	evas_object_show(g_pOptionCtrlHandle->pPopUpHandle);

	return TRUE;
}

void mp_option_ctrl_change_language(void)
{
	if (!g_pOptionCtrlHandle) {
		VideoLogError("g_pOptionCtrlHandle is NULL");
		return;
	}

	if (!g_pOptionCtrlHandle->pGenList
			|| !g_pOptionCtrlHandle->pPopUpHandle) {
		VideoLogError("No existed genlist or popup handle.");
		return;
	}

	VideoLogInfo("");

	elm_genlist_realized_items_update(g_pOptionCtrlHandle->pGenList);

}

void *mp_option_ctrl_get_handle(void)
{
	VideoLogInfo("");
	if (!g_pOptionCtrlHandle) {
		VideoLogError("g_pOptionCtrlHandle is NULL");
		return NULL;
	}
	return (void *) g_pOptionCtrlHandle->pPopUpHandle;
}
