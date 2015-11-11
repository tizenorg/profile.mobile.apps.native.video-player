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
#include "vp-play-macro-define.h"

#include "vp-share.h"


/* check temp */
#include "vp-play-log.h"


typedef struct _SharePopup {
	Evas_Object *pParent;
	Evas_Object *pPopup;
	Evas_Object *pButton;
	Evas_Object *pBox;
	Evas_Object *pGenList;

	char *pMediUrl;
	int nListCount;

	void *pUserData;
	PopupCloseCbFunc pCloseCb;

} SharePopup;

static void __vp_share_popup_rotate_cb(void *data, Evas_Object *obj,
                                       void *event_info)
{
	SharePopup *pShare = (SharePopup *) data;
	if (!pShare) {
		VideoLogError("pShare IS null");
		return;
	}
	if (pShare->nListCount < VP_POPUP_MIN_ITEMS) {
		return;
	}
	vp_popup_set_popup_min_size(pShare->pParent, pShare->pBox,
	                            pShare->nListCount, VIDEO_POPUP_DEFAULT);

}

static void _vp_share_destroy_handle(SharePopup *pShare)
{
	if (pShare == NULL) {
		VideoLogError("pShare is NULL");
		return;
	}
	evas_object_smart_callback_del(pShare->pParent, "rotation,changed",
	                               __vp_share_popup_rotate_cb);

	VP_EVAS_DEL(pShare->pGenList);
	VP_EVAS_DEL(pShare->pButton);
	VP_EVAS_DEL(pShare->pBox);

	VP_EVAS_DEL(pShare->pPopup);

	VP_FREE(pShare->pMediUrl);

	VP_FREE(pShare);
}

/* callback functions */
static char *__vp_share_genlist_text_get_cb(const void *pUserData,
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

static void __vp_share_genlist_item_selected_cb(void *pUserData,
        Evas_Object *pObject,
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

	Elm_Object_Item *pItem = (Elm_Object_Item *) pEventInfo;
	char *szTxt = (char *) elm_object_item_data_get(pItem);
	SharePopup *pShare = (SharePopup *) pUserData;

	if (szTxt == NULL) {
		VideoLogError("szTxt is NULL");
		return;
	}

	video_share_type_t nType = VP_SHARE_NONE;

	if (!strcmp(szTxt, VP_PLAY_STRING_SHARE_MESSAGE)) {
		VideoLogWarning
		("SELECT SHARE ITEM (VP_PLAY_STRING_SHARE_MESSAGE)");
		nType = VP_SHARE_MESSAGE;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_SHARE_EMAIL)) {
		VideoLogWarning("SELECT SHARE ITEM (VP_PLAY_STRING_SHARE_EMAIL)");
		nType = VP_SHARE_EMAIL;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_SHARE_BLUETOOTH)) {
		VideoLogWarning
		("SELECT SHARE ITEM (VP_PLAY_STRING_SHARE_BLUETOOTH)");
		nType = VP_SHARE_BLUETOOTH;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_SHARE_FTM)) {
		VideoLogWarning("SELECT SHARE ITEM (VP_PLAY_STRING_SHARE_FTM)");
		nType = VP_SHARE_FTM;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_SHARE_FACEBOOK)) {
		VideoLogWarning
		("SELECT SHARE ITEM (VP_PLAY_STRING_SHARE_FACEBOOK)");
		nType = VP_SHARE_FACEBOOK;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_SHARE_YOUTUBE)) {
		VideoLogWarning
		("SELECT SHARE ITEM (VP_PLAY_STRING_SHARE_YOUTUBE)");
		nType = VP_SHARE_YOUTUBE;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_SHARE_NFC)) {
		VideoLogWarning("SELECT SHARE ITEM (VP_PLAY_STRING_SHARE_NFC)");
		nType = VP_SHARE_NFC;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_SHARE_FILE_TRANSFER)) {
		VideoLogWarning
		("SELECT SHARE ITEM (VP_PLAY_STRING_SHARE_FILE_TRANSFER)");
		nType = VP_SHARE_FILE_TRANSFER;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_SHARE_DROPBOX)) {
		VideoLogWarning
		("SELECT SHARE ITEM (VP_PLAY_STRING_SHARE_DROPBOX)");
		nType = VP_SHARE_DROP_BOX;
	}

	if (pShare->pCloseCb) {
		pShare->pCloseCb((int) nType, TRUE, (void *) pShare->pUserData);
	}
}


static void __vp_share_close_btn_cb(void *pUserData, Evas_Object *pObj,
                                    void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pShareHandle is NULL");
		return;
	}

	SharePopup *pShare = (SharePopup *) pUserData;

	if (pShare->pCloseCb) {
		pShare->pCloseCb(VP_SHARE_NONE, FALSE,
		                 (void *) pShare->pUserData);
	}
}

static void __vp_share_popup_key_event_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SharePopup *pShare = (SharePopup *) pUserData;

	if (pShare->pCloseCb) {
		pShare->pCloseCb(VP_SHARE_NONE, FALSE,
		                 (void *) pShare->pUserData);
	}
}

static void __vp_share_popup_mouse_event_cb(void *pUserData, Evas *pEvas,
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
		SharePopup *pShare = (SharePopup *) pUserData;

		if (pShare->pCloseCb) {
			pShare->pCloseCb(VP_SHARE_NONE, FALSE,
			                 (void *) pShare->pUserData);
		}
	}
}


/* internal functions */
static Evas_Object *_vp_share_create_button(Evas_Object *pParent,
        void *pUserData)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	Evas_Object *pObj = NULL;
	pObj = elm_button_add(pParent);
	elm_object_style_set(pObj, "popup_button/default");
	elm_object_domain_translatable_text_set(pObj, VP_SYS_STR_PREFIX,
	                                        VP_PLAY_STRING_COM_CLOSE_IDS);
	elm_object_part_content_set(pParent, "button1", pObj);
	evas_object_smart_callback_add(pObj, "clicked",
	                               __vp_share_close_btn_cb, pUserData);
	evas_object_show(pObj);

	return pObj;
}

static Evas_Object *_vp_share_create_genlist(Evas_Object *pParent)
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


static bool _vp_share_add_genlist_item(Evas_Object *pObj,
                                       void *pUserData)
{
	if (!pObj || !pUserData) {
		VideoLogError("pObj is NULL");
		return FALSE;
	}
	SharePopup *pShare = (SharePopup *) pUserData;

	Elm_Genlist_Item_Class *st_Share_Itc = NULL;

	st_Share_Itc = elm_genlist_item_class_new();

	if (st_Share_Itc != NULL) {
		st_Share_Itc->version = ELM_GENLIST_ITEM_CLASS_VERSION;
		st_Share_Itc->item_style = "1text/popup";
		st_Share_Itc->func.text_get = (void *) __vp_share_genlist_text_get_cb;
		st_Share_Itc->func.content_get = NULL;
		st_Share_Itc->func.state_get = NULL;
		st_Share_Itc->func.del = NULL;
		pShare->nListCount = 0;

		elm_genlist_item_append(pObj, st_Share_Itc,
		                        (void *) VP_PLAY_STRING_SHARE_MESSAGE, NULL,
		                        ELM_GENLIST_ITEM_NONE,
		                        __vp_share_genlist_item_selected_cb,
		                        pUserData);
		pShare->nListCount++;
		elm_genlist_item_append(pObj, st_Share_Itc,
		                        (void *) VP_PLAY_STRING_SHARE_EMAIL, NULL,
		                        ELM_GENLIST_ITEM_NONE,
		                        __vp_share_genlist_item_selected_cb,
		                        pUserData);
		pShare->nListCount++;
		elm_genlist_item_append(pObj, st_Share_Itc,
		                        (void *) VP_PLAY_STRING_SHARE_FTM, NULL,
		                        ELM_GENLIST_ITEM_NONE,
		                        __vp_share_genlist_item_selected_cb,
		                        pUserData);
		pShare->nListCount++;
		elm_genlist_item_append(pObj, st_Share_Itc,
		                        (void *) VP_PLAY_STRING_SHARE_BLUETOOTH, NULL,
		                        ELM_GENLIST_ITEM_NONE,
		                        __vp_share_genlist_item_selected_cb,
		                        pUserData);
		pShare->nListCount++;
		elm_genlist_item_append(pObj, st_Share_Itc,
		                        (void *) VP_PLAY_STRING_SHARE_FACEBOOK, NULL,
		                        ELM_GENLIST_ITEM_NONE,
		                        __vp_share_genlist_item_selected_cb,
		                        pUserData);
		elm_genlist_item_class_free(st_Share_Itc);
	}

	return TRUE;
}

/* external functions */
share_handle vp_share_create(Evas_Object *pParent, char *pMediaUrl,
                             PopupCloseCbFunc pCloseCb)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	if (pMediaUrl == NULL) {
		VideoLogError("pMediaUrl is NULL");
		return NULL;
	}

	SharePopup *pShare = NULL;

	pShare = calloc(1, sizeof(SharePopup));

	if (pShare == NULL) {
		VideoLogError("pShare alloc fail");
		return NULL;
	}

	pShare->pParent = pParent;
	pShare->pCloseCb = pCloseCb;

	VP_STRDUP(pShare->pMediUrl, pMediaUrl);

	pShare->pPopup =
	    vp_popup_create(pParent, POPUP_STYLE_MENU_LIST_WITH_CANCEL_BTN,
	                    VP_PLAY_STRING_POPUP_SHARE, NULL, 0.0, NULL,
	                    __vp_share_popup_key_event_cb,
	                    __vp_share_popup_mouse_event_cb, (void *) pShare);
	if (pShare->pPopup == NULL) {
		VideoLogError("_vp_share_create_popup fail");
		_vp_share_destroy_handle(pShare);
		return NULL;
	}

	pShare->pButton =
	    _vp_share_create_button(pShare->pPopup, (void *) pShare);
	if (pShare->pButton == NULL) {
		VideoLogError("_vp_share_create_button fail");
		_vp_share_destroy_handle(pShare);
		return NULL;
	}

	pShare->pGenList = _vp_share_create_genlist(pShare->pPopup);
	if (pShare->pGenList == NULL) {
		VideoLogError("_vp_share_create_genlist fail");
		_vp_share_destroy_handle(pShare);
		return NULL;
	}
	if (!_vp_share_add_genlist_item(pShare->pGenList, (void *) pShare)) {
		VideoLogError("_vp_share_add_genlist_item fail");
		_vp_share_destroy_handle(pShare);
		return NULL;
	}
	evas_object_smart_callback_add(pShare->pParent, "rotation,changed",
	                               __vp_share_popup_rotate_cb, pShare);

	pShare->pBox = elm_box_add(pShare->pPopup);

	vp_popup_set_popup_min_size(pShare->pParent, pShare->pBox,
	                            pShare->nListCount, VIDEO_POPUP_DEFAULT);

	elm_box_pack_end(pShare->pBox, pShare->pGenList);

	elm_object_content_set(pShare->pPopup, pShare->pBox);

	return pShare;
}

void vp_share_destroy(share_handle pShareHandle)
{
	if (pShareHandle == NULL) {
		VideoLogError("pShareHandle is NULL");
		return;
	}

	SharePopup *pShare = (SharePopup *) pShareHandle;

	_vp_share_destroy_handle(pShare);

}

bool vp_share_realize(share_handle pShareHandle)
{
	if (pShareHandle == NULL) {
		VideoLogError("pShareHandle is NULL");
		return FALSE;
	}

	SharePopup *pShare = (SharePopup *) pShareHandle;

	evas_object_show(pShare->pPopup);

	return TRUE;
}

bool vp_share_unrealize(share_handle pShareHandle)
{
	if (pShareHandle == NULL) {
		VideoLogError("pShareHandle is NULL");
		return FALSE;
	}

	SharePopup *pShare = (SharePopup *) pShareHandle;

	evas_object_hide(pShare->pPopup);

	return TRUE;
}

bool vp_share_set_user_data(share_handle pShareHandle, void *pUserData)
{
	if (pShareHandle == NULL) {
		VideoLogError("pShareHandle is NULL");
		return FALSE;
	}

	SharePopup *pShare = (SharePopup *) pShareHandle;

	pShare->pUserData = pUserData;

	return TRUE;
}
