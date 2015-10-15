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
#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-type-define.h"
#include "vp-play-macro-define.h"
#include "vp-capture.h"

/* check temp */
#include "vp-play-log.h"

#define VP_CAPTURE_POPUP_GENLIST_DATA_KEY		"vp.capture.popup.genlist"

typedef struct _CapturePopup {
	Evas_Object *pParent;
	Evas_Object *pPopup;
	Evas_Object *pRadio;
	Evas_Object *pButton;
	Evas_Object *pBox;
	Evas_Object *pGenList;
	Elm_Genlist_Item_Class *st_Capture_Itc;
	int nListCount;

	void *pUserData;
	PopupCloseCbFunc pCloseCb;
	bool bCaptureOn;
} CapturePopup;



static void _vp_capture_popup_destroy_handle(CapturePopup *
		pCapturePopup);
static void __vp_capture_popup_genlist_item_selected_cb(void *pUserData,
		Evas_Object *
		pObject,
		void *pEventInfo);

static void __vp_capture_genlist_realized(void *data, Evas_Object *obj,
		void *event_info)
{
	VP_GENLIST_HIDE_BOTTOMLINE(data, obj, event_info);
}

static void __vp_capture_popup_rotate_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	CapturePopup *pCapture = (CapturePopup *) data;
	if (!pCapture) {
		VideoLogError("pCapture IS null");
		return;
	}
	if (pCapture->nListCount < VP_POPUP_MIN_ITEMS) {
		return;
	}
	vp_popup_set_popup_min_size(pCapture->pParent, pCapture->pBox,
				    pCapture->nListCount,
				    VIDEO_POPUP_DEFAULT);

}

/* callback functions */
static char *__vp_capture_popup_genlist_text_get_cb(const void *pUserData,
		Evas_Object *pObj,
		const char *pPart)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}

	char *szTxt = (char *) pUserData;

	if (!strcmp(pPart, "elm.text")) {
		return strdup(szTxt);
	}

	return NULL;
}

static Evas_Object *__vp_capture_popup_genlist_content_get_cb(const void
		*pUserData,
		Evas_Object
		* pObj,
		const char
		*pPart)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}

	char *szTxt = (char *) pUserData;

	if (!strcmp(pPart, "elm.icon")) {
		Evas_Object *pRadioObj = NULL;

		CapturePopup *pCapture =
			(CapturePopup *) evas_object_data_get(pObj,
					VP_CAPTURE_POPUP_GENLIST_DATA_KEY);
		if (pCapture == NULL) {
			VideoLogWarning("evas_object_data_get is fail");
			return NULL;
		}

		int nVal = 0;

		if (!strcmp(szTxt, VP_PLAY_STRING_COM_ON)) {
			nVal = 1;
		} else if (!strcmp(szTxt, VP_PLAY_STRING_COM_OFF)) {
			nVal = 0;
		}

		pRadioObj = elm_radio_add(pObj);
		elm_radio_state_value_set(pRadioObj, nVal);
		elm_radio_group_add(pRadioObj, pCapture->pRadio);
		elm_radio_value_set(pCapture->pRadio, (int) pCapture->bCaptureOn);
		evas_object_smart_callback_add(pRadioObj, "changed",
					       __vp_capture_popup_genlist_item_selected_cb,
					       pCapture);
		evas_object_show(pRadioObj);

		return pRadioObj;
	}

	return NULL;
}

static void __vp_capture_popup_genlist_item_selected_cb(void *pUserData,
		Evas_Object *
		pObject,
		void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}
	if (pEventInfo == NULL) {
		VideoLogError("pEventInfo is NULL");
		return;
	}

	Elm_Object_Item *pSelectedItem =
		elm_genlist_selected_item_get(pObject);
	if (pSelectedItem) {
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}
	Elm_Object_Item *pItem = (Elm_Object_Item *) pEventInfo;
	char *szTxt = (char *) elm_object_item_data_get(pItem);
	if (szTxt == NULL) {
		VideoLogError("szTxt is NULL");
		return;
	}

	int nVal = 0;
	bool bCapture = FALSE;
	if (!strcmp(szTxt, VP_PLAY_STRING_COM_ON)) {
		bCapture = TRUE;
		nVal = 1;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_COM_OFF)) {
		nVal = 0;
		bCapture = FALSE;
	}

	CapturePopup *pCapture = (CapturePopup *) pUserData;
	pCapture->bCaptureOn = bCapture;
	if (pCapture->pCloseCb) {
		pCapture->pCloseCb(nVal, FALSE, (void *) pCapture->pUserData);
	}
}

static void __vp_capture_popup_key_event_cb(void *pUserData,
		Evas_Object *pObj,
		void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	CapturePopup *pCapture = (CapturePopup *) pUserData;

	if (pCapture->pCloseCb) {
		pCapture->pCloseCb(-1, FALSE, (void *) pCapture->pUserData);
	}
}

static void __vp_capture_popup_mouse_event_cb(void *pUserData,
		Evas *pEvas,
		Evas_Object *pObj,
		void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	if (pEventInfo == NULL) {
		VideoLogError("pEventInfo is NULL");
		return;
	}

	Evas_Event_Mouse_Up *ev = pEventInfo;

	if (ev->button == 3) {
		CapturePopup *pCapture = (CapturePopup *) pUserData;

		if (pCapture->pCloseCb) {
			pCapture->pCloseCb(-1, FALSE, (void *) pCapture->pUserData);
		}
	}
}


/* internal functions */
static void _vp_capture_popup_destroy_handle(CapturePopup *pCapture)
{
	if (pCapture == NULL) {
		VideoLogError("pCapture is NULL");
		return;
	}

	evas_object_smart_callback_del(pCapture->pGenList, "realized",
				       __vp_capture_genlist_realized);
	evas_object_smart_callback_del(pCapture->pParent, "rotation,changed",
				       __vp_capture_popup_rotate_cb);

	VP_EVAS_DEL(pCapture->pRadio);
	VP_EVAS_DEL(pCapture->pGenList);
	VP_EVAS_DEL(pCapture->pButton);
	VP_EVAS_DEL(pCapture->pBox);

	if (pCapture->st_Capture_Itc) {
		elm_genlist_item_class_free(pCapture->st_Capture_Itc);
		pCapture->st_Capture_Itc = NULL;
	}

	VP_EVAS_DEL(pCapture->pPopup);

	VP_FREE(pCapture);
}

static Evas_Object *_vp_capture_popup_create_genlist(Evas_Object *
		pParent)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	Evas_Object *pObj = NULL;

	pObj = elm_genlist_add(pParent);
	evas_object_size_hint_weight_set(pObj, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pObj, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(pObj);
	return pObj;
}


static bool _vp_capture_popup_add_genlist_item(Evas_Object *pObj,
		void *pUserData)
{
	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return FALSE;
	}

	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return FALSE;
	}

	CapturePopup *pCapture = (CapturePopup *) pUserData;
	if (pCapture->st_Capture_Itc) {
		elm_genlist_item_class_free(pCapture->st_Capture_Itc);
		pCapture->st_Capture_Itc = NULL;
	}

	pCapture->st_Capture_Itc = elm_genlist_item_class_new();
	if (pCapture->st_Capture_Itc == NULL) {
		VideoLogError("failed to create genlist class");
		return FALSE;
	}

	pCapture->st_Capture_Itc->version = ELM_GENLIST_ITEM_CLASS_VERSION;
	pCapture->st_Capture_Itc->item_style = "1text.1icon.3/popup";
	pCapture->st_Capture_Itc->func.text_get =
		(void *) __vp_capture_popup_genlist_text_get_cb;
	pCapture->st_Capture_Itc->func.content_get =
		(void *) __vp_capture_popup_genlist_content_get_cb;
	pCapture->st_Capture_Itc->func.state_get = NULL;
	pCapture->st_Capture_Itc->func.del = NULL;
	pCapture->nListCount = 0;

	elm_genlist_item_append(pObj, pCapture->st_Capture_Itc,
				(void *) VP_PLAY_STRING_COM_ON, NULL,
				ELM_GENLIST_ITEM_NONE,
				__vp_capture_popup_genlist_item_selected_cb,
				pUserData);
	pCapture->nListCount++;
	elm_genlist_item_append(pObj, pCapture->st_Capture_Itc,
				(void *) VP_PLAY_STRING_COM_OFF, NULL,
				ELM_GENLIST_ITEM_NONE,
				__vp_capture_popup_genlist_item_selected_cb,
				pUserData);
	pCapture->nListCount++;

	return TRUE;
}

/* external functions */
capture_popup_handle vp_capture_popup_create(Evas_Object *pParent,
		bool bCaptureOn,
		PopupCloseCbFunc pCloseCb)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	CapturePopup *pCapture = NULL;

	pCapture = calloc(1, sizeof(CapturePopup));

	if (pCapture == NULL) {
		VideoLogError("pCapture alloc fail");
		return NULL;
	}

	pCapture->pParent = pParent;
	pCapture->pCloseCb = pCloseCb;

	pCapture->pPopup =
		vp_popup_create(pParent, POPUP_STYLE_MENU_LIST_NO_CANCEL_BTN,
				VP_PLAY_STRING_POPUP_CAPTURE, NULL, 0.0, NULL,
				__vp_capture_popup_key_event_cb,
				__vp_capture_popup_mouse_event_cb,
				(void *) pCapture);
	if (pCapture->pPopup == NULL) {
		VideoLogError("vp_popup_create fail");
		_vp_capture_popup_destroy_handle(pCapture);
		return NULL;
	}

	pCapture->pGenList =
		_vp_capture_popup_create_genlist(pCapture->pPopup);
	if (pCapture->pGenList == NULL) {
		VideoLogError("_vp_capture_popup_create_genlist fail");
		_vp_capture_popup_destroy_handle(pCapture);
		return NULL;
	}
	evas_object_data_set(pCapture->pGenList,
			     VP_CAPTURE_POPUP_GENLIST_DATA_KEY,
			     (void *) pCapture);
	evas_object_smart_callback_add(pCapture->pGenList, "realized",
				       __vp_capture_genlist_realized, NULL);

	pCapture->pRadio = elm_radio_add(pCapture->pGenList);

	if (!_vp_capture_popup_add_genlist_item
			(pCapture->pGenList, (void *) pCapture)) {
		VideoLogError("_vp_capture_popup_add_genlist_item fail");
		_vp_capture_popup_destroy_handle(pCapture);
		return NULL;
	}
	evas_object_smart_callback_add(pCapture->pParent, "rotation,changed",
				       __vp_capture_popup_rotate_cb,
				       pCapture);

	//elm_radio_value_set(pCapture->pRadio, (int)bCaptureOn);
	pCapture->bCaptureOn = bCaptureOn;

	pCapture->pBox = elm_box_add(pCapture->pPopup);
	vp_popup_set_popup_min_size(pCapture->pParent, pCapture->pBox,
				    pCapture->nListCount,
				    VIDEO_POPUP_DEFAULT);

	elm_box_pack_end(pCapture->pBox, pCapture->pGenList);

	elm_object_content_set(pCapture->pPopup, pCapture->pBox);

	return pCapture;
}

void vp_capture_popup_destroy(capture_popup_handle pCapturePopupHandle)
{
	if (pCapturePopupHandle == NULL) {
		VideoLogError("pCapturePopupHandle is NULL");
		return;
	}

	CapturePopup *pCapture = (CapturePopup *) pCapturePopupHandle;

	_vp_capture_popup_destroy_handle(pCapture);

}

bool vp_capture_popup_realize(capture_popup_handle pCapturePopupHandle)
{
	if (pCapturePopupHandle == NULL) {
		VideoLogError("pCapturePopupHandle is NULL");
		return FALSE;
	}

	CapturePopup *pCapture = (CapturePopup *) pCapturePopupHandle;

	evas_object_show(pCapture->pPopup);

	return TRUE;
}

bool vp_capture_popup_unrealize(capture_popup_handle pCapturePopupHandle)
{
	if (pCapturePopupHandle == NULL) {
		VideoLogError("pCapturePopupHandle is NULL");
		return FALSE;
	}

	CapturePopup *pCapture = (CapturePopup *) pCapturePopupHandle;

	evas_object_hide(pCapture->pPopup);

	return TRUE;
}

bool vp_capture_popup_set_user_data(capture_popup_handle
				    pCapturePopupHandle, void *pUserData)
{
	if (pCapturePopupHandle == NULL) {
		VideoLogError("pCapturePopupHandle is NULL");
		return FALSE;
	}

	CapturePopup *pCapture = (CapturePopup *) pCapturePopupHandle;


	pCapture->pUserData = pUserData;

	return TRUE;
}
