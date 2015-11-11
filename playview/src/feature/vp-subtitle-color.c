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

#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-type-define.h"
#include "vp-play-macro-define.h"

#include "vp-subtitle-color.h"
#include "vp-play-button.h"
#include "vp-play-util.h"

/* check temp */
#include "vp-play-log.h"

#define VP_SUBTITLE_COLOR_GENLIST_DATA_KEY		"vp.subtitle.color.genlist"
#define VP_PLAY_OPACITY_MAX_VALUE		100
#define VP_PLAY_OPACITY_MIN_VALUE		0

typedef struct _SubtitleColorplane_Data {
	Evas_Object *layout;
	//Evas_Object *rect;
	Evas_Object *colorselector;
	Elm_Object_Item *it_last;
	Elm_Object_Item *sel_it;
	Eina_Bool changed;
	int r, g, b, a;
} SubtitleColorplaneData;

typedef struct _SubtitleColorPopup {
	Evas_Object *pParent;
	Evas_Object *pPopup;
#ifndef SUBTITLE_K_FEATURE
	Evas_Object *pRadio;
#else
	Evas_Object *pLayout;
	Evas_Object *pLayoutOut;
	Evas_Object *pScroller;
	Evas_Object *pOkButton;
	Evas_Object *pCancelButton;
	Evas_Object *pDelButton;
	Evas_Object *pAddButton;
	Evas_Object *pSlider;
	int nOpacityVal;
	SubtitleColorUpdateCbFunc pUpdateCb;
#endif
	Evas_Object *pButton;
	Evas_Object *pBox;
	Evas_Object *pGenList;
	Elm_Genlist_Item_Class *st_SubtitleFontColor_Itc;

	int nDefaultIndex;
	int nListCount;

	void *pUserData;
	SubtitlePopupCloseCbFunc pCloseCb;
	video_play_subtitle_color_t currentFontColor;
	SubtitleColorplaneData *pColorplaneData;
	char *pCurrentColorHex;
} SubtitleColorPopup;


static void _vp_subtitle_color_destroy_handle(SubtitleColorPopup *
        pSubtitleColor);

#ifndef SUBTITLE_K_FEATURE
static void __vp_subtitle_color_genlist_item_selected_cb(void *pUserData,
                        Evas_Object *pObject,
                        void *pEventInfo);

static void __vp_subtitle_color_genlist_realized(void *data,
                        Evas_Object *obj,
                        void *event_info)
{
	VP_GENLIST_HIDE_BOTTOMLINE(data, obj, event_info);
}
#endif
static void __vp_subtitle_color_popup_rotate_cb(void *data,
                        Evas_Object *obj,
                        void *event_info)
{
	SubtitleColorPopup *pSubtitleColor = (SubtitleColorPopup *) data;
	if (!pSubtitleColor) {
		VideoLogError("pSubtitleColor IS null");
		return;
	}
#ifndef SUBTITLE_K_FEATURE
	if (pSubtitleColor->nListCount < VP_POPUP_MIN_ITEMS) {
		return;
	}
	vp_popup_set_popup_min_size(pSubtitleColor->pParent,
	                            pSubtitleColor->pBox,
	                            pSubtitleColor->nListCount,
	                            VIDEO_POPUP_DEFAULT);
#else
	int nRotation = elm_win_rotation_get(pSubtitleColor->pParent);
	if (nRotation == 90 || nRotation == 270)
		elm_layout_file_set(pSubtitleColor->pLayoutOut,
		                    VP_PLAY_SUBTITLE_COLOR_POPUP_EDJ,
		                    "pv.subtitle.scroller.layout.landscape");
	else
		elm_layout_file_set(pSubtitleColor->pLayoutOut,
		                    VP_PLAY_SUBTITLE_COLOR_POPUP_EDJ,
		                    "pv.subtitle.scroller.layout");
#endif
}

#ifndef SUBTITLE_K_FEATURE

/* callback functions */
static char *__vp_subtitle_color_genlist_text_get_cb(const void
                        *pUserData,
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

static Evas_Object *__vp_subtitle_color_genlist_content_get_cb(const void
                        *pUserData,
                        Evas_Object *pObj,
                        const char *pPart)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}

	char *szTxt = (char *) pUserData;

	if (!strcmp(pPart, "elm.icon")) {
		Evas_Object *pRadioObj = NULL;

		SubtitleColorPopup *pSubtitleColor =
		    (SubtitleColorPopup *) evas_object_data_get(pObj,
		            VP_SUBTITLE_COLOR_GENLIST_DATA_KEY);
		if (pSubtitleColor == NULL) {
			VideoLogWarning("evas_object_data_get is fail");
			return NULL;
		}
		video_play_subtitle_color_t nColor = VIDEO_SUBTITLE_COLOR_BLACK;
		if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_COLOR_BLACK)) {
			nColor = VIDEO_SUBTITLE_COLOR_BLACK;
		} else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_COLOR_BLUE)) {
			nColor = VIDEO_SUBTITLE_COLOR_BLUE;
		} else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_COLOR_GREEN)) {
			nColor = VIDEO_SUBTITLE_COLOR_GREEN;
		} else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_COLOR_WHITE)) {
			nColor = VIDEO_SUBTITLE_COLOR_WHITE;
		}

		pRadioObj = elm_radio_add(pObj);
		elm_radio_state_value_set(pRadioObj, nColor);
		elm_radio_group_add(pRadioObj, pSubtitleColor->pRadio);
		elm_radio_value_set(pSubtitleColor->pRadio,
		                    pSubtitleColor->currentFontColor);
		evas_object_smart_callback_add(pRadioObj, "changed",
		                               __vp_subtitle_color_genlist_item_selected_cb,
		                               pSubtitleColor);
		evas_object_show(pRadioObj);

		return pRadioObj;
	}

	return NULL;
}

static void __vp_subtitle_color_genlist_item_selected_cb(void *pUserData,
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
	Elm_Object_Item *pSelectedItem =
	    elm_genlist_selected_item_get(pObject);
	if (pSelectedItem) {
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}
	char *szTxt = (char *) elm_object_item_data_get(pItem);

	SubtitleColorPopup *pSubtitleColor = (SubtitleColorPopup *) pUserData;

	if (szTxt == NULL) {
		VideoLogError("szTxt is NULL");
		return;
	}

	video_play_subtitle_color_t nColor = VIDEO_SUBTITLE_COLOR_BLACK;
	if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_COLOR_BLACK)) {
		nColor = VIDEO_SUBTITLE_COLOR_BLACK;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_COLOR_BLUE)) {
		nColor = VIDEO_SUBTITLE_COLOR_BLUE;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_COLOR_GREEN)) {
		nColor = VIDEO_SUBTITLE_COLOR_GREEN;
	} else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_COLOR_WHITE)) {
		nColor = VIDEO_SUBTITLE_COLOR_WHITE;
	}

	pSubtitleColor->currentFontColor = nColor;
	if (pSubtitleColor->pCloseCb) {
		pSubtitleColor->pCloseCb(nColor, FALSE,
		                         (void *) pSubtitleColor->pUserData);
	}
}

#endif

static void __vp_subtitle_color_popup_key_event_cb(void *pUserData,
                        Evas_Object *pObj,
                        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleColorPopup *pSubtitleColor = (SubtitleColorPopup *) pUserData;

	if (pSubtitleColor->pCloseCb) {
#ifndef SUBTITLE_K_FEATURE
		pSubtitleColor->pCloseCb(-1, FALSE,
		                         (void *) pSubtitleColor->pUserData);
#else
		pSubtitleColor->pCloseCb(pSubtitleColor->pCurrentColorHex, FALSE,
		                         (void *) pSubtitleColor->pUserData);
#endif
	}
}

static void __vp_subtitle_color_popup_mouse_event_cb(void *pUserData,
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
		SubtitleColorPopup *pSubtitleColor =
		    (SubtitleColorPopup *) pUserData;

#ifndef SUBTITLE_K_FEATURE
		pSubtitleColor->pCloseCb(-1, FALSE,
		                         (void *) pSubtitleColor->pUserData);
#else
		pSubtitleColor->pCloseCb(pSubtitleColor->pCurrentColorHex, FALSE,
		                         (void *) pSubtitleColor->pUserData);
#endif
	}
}



/* internal functions */
int _vp_subtitle_color_get_opacity(char *colorHex)
{
	if (colorHex == NULL) {
		VideoLogError("colorHex is NULL");
		return 0;
	}

	char *szOpacity = g_strndup(colorHex + 7, 2);

	float fOpacity = strtol(szOpacity, NULL, 16);

	VP_FREE(szOpacity);

	int nOpacity = (int)((fOpacity * 100) / 255.0 + 0.5);

	VideoLogInfo("nOpacity", nOpacity);

	return nOpacity;
}

int _vp_subtitle_color_get_platte_index(char *colorHex)
{
	if (colorHex == NULL) {
		VideoLogError("colorHex is NULL");
		return 0;
	}
	int nIndex = 0;
	char *pColorHex = g_strndup(colorHex, 7);
	VideoLogInfo("pColorHex %s", pColorHex);

	if (!strcmp(pColorHex, "#ffffff")) {
		nIndex = 0;
	} else if (!strcmp(pColorHex, "#000000")) {
		nIndex = 1;
	} else if (!strcmp(pColorHex, "#ff0000")) {
		nIndex = 2;
	} else if (!strcmp(pColorHex, "#00ff00")) {
		nIndex = 3;
	} else if (!strcmp(pColorHex, "#0000ff")) {
		nIndex = 4;
	} else if (!strcmp(pColorHex, "#ffff00")) {
		nIndex = 5;
	} else if (!strcmp(pColorHex, "#ff00ff")) {
		nIndex = 6;
	} else if (!strcmp(pColorHex, "#00ffff")) {
		nIndex = 7;
	} else {
		nIndex = 8;
	}

	VP_FREE(pColorHex);
	return nIndex;
}

char *_vp_subtitle_color_get_select_color(char *pColor, int nOpacity)
{
	if (pColor == NULL) {
		VideoLogError("colorHex is NULL");
		return 0;
	}

	int nOpacitytmp = (int)(((float)(nOpacity * 255) / 100.0) + 0.5);

	char *pOpacityHex = g_strdup_printf("%02x", nOpacitytmp);

	char *polorHex = g_strndup(pColor, 7);

	char *pColorHex = g_strdup_printf("%s%s", polorHex, pOpacityHex);

	VP_FREE(pOpacityHex);
	VP_FREE(polorHex);

	return pColorHex;
}

static void _vp_subtitle_color_destroy_handle(SubtitleColorPopup *
                                pSubtitleColor)
{
	if (pSubtitleColor == NULL) {
		VideoLogError("pSubtitleColor is NULL");
		return;
	}

	evas_object_smart_callback_del(pSubtitleColor->pParent,
	                               "rotation,changed",
	                               __vp_subtitle_color_popup_rotate_cb);
#ifndef SUBTITLE_K_FEATURE
	evas_object_smart_callback_del(pSubtitleColor->pGenList, "realized",
	                               __vp_subtitle_color_genlist_realized);
#endif


#ifndef SUBTITLE_K_FEATURE
	VP_EVAS_DEL(pSubtitleColor->pRadio);
	VP_EVAS_DEL(pSubtitleColor->pGenList);
	VP_EVAS_DEL(pSubtitleColor->pButton);
	VP_EVAS_DEL(pSubtitleColor->pBox);
#else
	VP_FREE(pSubtitleColor->pColorplaneData);
	pSubtitleColor->pColorplaneData = NULL;

	VP_FREE(pSubtitleColor->pCurrentColorHex);
	pSubtitleColor->pCurrentColorHex = NULL;

	VP_EVAS_DEL(pSubtitleColor->pOkButton);
	VP_EVAS_DEL(pSubtitleColor->pCancelButton);
#endif
	if (pSubtitleColor->st_SubtitleFontColor_Itc) {
		elm_genlist_item_class_free(pSubtitleColor->
		                            st_SubtitleFontColor_Itc);
		pSubtitleColor->st_SubtitleFontColor_Itc = NULL;
	}

	VP_EVAS_DEL(pSubtitleColor->pPopup);

	VP_FREE(pSubtitleColor);
}

#ifndef SUBTITLE_K_FEATURE
static Evas_Object *_vp_subtitle_color_create_genlist(Evas_Object *
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


static bool _vp_subtitle_color_add_genlist_item(Evas_Object *pObj,
                                void *pUserData)
{
	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return FALSE;
	}

	SubtitleColorPopup *pSubtitleColor = (SubtitleColorPopup *) pUserData;

	if (pSubtitleColor->st_SubtitleFontColor_Itc) {
		elm_genlist_item_class_free(pSubtitleColor->
		                            st_SubtitleFontColor_Itc);
		pSubtitleColor->st_SubtitleFontColor_Itc = NULL;
	}

	pSubtitleColor->st_SubtitleFontColor_Itc =
	    elm_genlist_item_class_new();
	pSubtitleColor->st_SubtitleFontColor_Itc->version =
	    ELM_GENLIST_ITEM_CLASS_VERSION;
	pSubtitleColor->st_SubtitleFontColor_Itc->item_style =
	    "1text.1icon.3/popup";
	pSubtitleColor->st_SubtitleFontColor_Itc->func.text_get =
	    (void *) __vp_subtitle_color_genlist_text_get_cb;
	pSubtitleColor->st_SubtitleFontColor_Itc->func.content_get =
	    (void *) __vp_subtitle_color_genlist_content_get_cb;
	pSubtitleColor->st_SubtitleFontColor_Itc->func.state_get = NULL;
	pSubtitleColor->st_SubtitleFontColor_Itc->func.del = NULL;
	pSubtitleColor->nListCount = 0;

	elm_genlist_item_append(pObj,
	                        pSubtitleColor->st_SubtitleFontColor_Itc,
	                        (void *) VP_PLAY_STRING_SUBTITLE_COLOR_WHITE,
	                        NULL, ELM_GENLIST_ITEM_NONE,
	                        __vp_subtitle_color_genlist_item_selected_cb,
	                        pUserData);
	pSubtitleColor->nListCount++;
	elm_genlist_item_append(pObj,
	                        pSubtitleColor->st_SubtitleFontColor_Itc,
	                        (void *) VP_PLAY_STRING_SUBTITLE_COLOR_BLUE,
	                        NULL, ELM_GENLIST_ITEM_NONE,
	                        __vp_subtitle_color_genlist_item_selected_cb,
	                        pUserData);
	pSubtitleColor->nListCount++;
	elm_genlist_item_append(pObj,
	                        pSubtitleColor->st_SubtitleFontColor_Itc,
	                        (void *) VP_PLAY_STRING_SUBTITLE_COLOR_GREEN,
	                        NULL, ELM_GENLIST_ITEM_NONE,
	                        __vp_subtitle_color_genlist_item_selected_cb,
	                        pUserData);
	pSubtitleColor->nListCount++;
	elm_genlist_item_append(pObj,
	                        pSubtitleColor->st_SubtitleFontColor_Itc,
	                        (void *) VP_PLAY_STRING_SUBTITLE_COLOR_BLACK,
	                        NULL, ELM_GENLIST_ITEM_NONE,
	                        __vp_subtitle_color_genlist_item_selected_cb,
	                        pUserData);
	pSubtitleColor->nListCount++;

	return TRUE;
}
#else

/* callback functions */

static void __vp_play_subtitle_color_popup_ok_btn_cb(void *pUserData,
                        Evas_Object *pObj,
                        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleColorPopup *pSubtitleColor = (SubtitleColorPopup *) pUserData;
	char *pColorHex =
	    _vp_subtitle_color_get_select_color(pSubtitleColor->
	                                        pCurrentColorHex,
	                                        pSubtitleColor->nOpacityVal);

	if (pSubtitleColor->pCloseCb) {
		pSubtitleColor->pCloseCb(pColorHex, TRUE,
		                         (void *) pSubtitleColor->pUserData);
	}
	VP_FREE(pColorHex);
}

static void __vp_play_subtitle_color_popup_cancel_btn_cb(void *pUserData,
        Evas_Object *
        pObj,
        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleColorPopup *pSubtitleColor = (SubtitleColorPopup *) pUserData;

	if (pSubtitleColor->pCloseCb) {
		pSubtitleColor->pCloseCb(pSubtitleColor->pCurrentColorHex, FALSE,
		                         (void *) pSubtitleColor->pUserData);
	}
}

static Evas_Object
*_vp_play_subtitle_color_popup_create_ok_button(Evas_Object *pParent,
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
	                                        VP_PLAY_STRING_COM_OK_IDS);
	elm_object_part_content_set(pParent, "button2", pObj);
	evas_object_smart_callback_add(pObj, "clicked",
	                               __vp_play_subtitle_color_popup_ok_btn_cb,
	                               pUserData);
	evas_object_show(pObj);

	return pObj;
}

static Evas_Object
*_vp_play_subtitle_color_popup_create_cancel_button(Evas_Object *
                        pParent,
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
	                                        VP_PLAY_STRING_COM_CANCEL_IDS);
	elm_object_part_content_set(pParent, "button1", pObj);
	evas_object_smart_callback_add(pObj, "clicked",
	                               __vp_play_subtitle_color_popup_cancel_btn_cb,
	                               pUserData);
	evas_object_show(pObj);

	return pObj;
}

static void _vp_play_subtitle_color_popup_colorpalette_cb(void *data,
                Evas_Object *obj,
                void *event_info)
{
	if (data == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}
	VideoLogInfo("");

	SubtitleColorPopup *pSubtitleColor = (SubtitleColorPopup *) data;

	int r = 255, g = 255, b = 255, a = 255;
	SubtitleColorplaneData *cp = pSubtitleColor->pColorplaneData;
	Elm_Object_Item *color_it = (Elm_Object_Item *) event_info;
	cp->sel_it = color_it;
	elm_colorselector_palette_item_color_get(color_it, &r, &g, &b, &a);
	elm_colorselector_color_set(cp->colorselector, r, g, b, a);

	VP_FREE(pSubtitleColor->pCurrentColorHex);
	pSubtitleColor->pCurrentColorHex =
	    vp_play_util_convert_rgba_to_hex(r, g, b, a);
	//Need Free;
	VideoLogInfo("p = %s", pSubtitleColor->pCurrentColorHex);
}

static void _vp_play_subtitle_color_popup_colorplane_cb(void *data,
                Evas_Object *obj,
                void *event_info)
{
	if (data == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}
	VideoLogInfo("");

	SubtitleColorPopup *pSubtitleColor = (SubtitleColorPopup *) data;

	int r = 255, g = 255, b = 255, a = 255;
	SubtitleColorplaneData *cp = pSubtitleColor->pColorplaneData;
	if (!cp->changed) {
		elm_object_item_signal_emit(cp->it_last, "elm,state,custom,hide",
		                            "");
		cp->changed = EINA_TRUE;
	}
	elm_colorselector_color_get(cp->colorselector, &r, &g, &b, &a);
	cp->r = r;
	cp->g = g;
	cp->b = b;
	cp->a = a;
	elm_colorselector_palette_item_color_set(cp->it_last, r, g, b, a);
	if (cp->sel_it != cp->it_last) {
		elm_object_item_signal_emit(cp->it_last, "elm,state,selected",
		                            "elm");
	}

	VP_FREE(pSubtitleColor->pCurrentColorHex);
	pSubtitleColor->pCurrentColorHex =
	    vp_play_util_convert_rgba_to_hex(r, g, b, a);

}

static void _vp_play_subtitle_color_popup_create_colorpalette(Evas_Object *layout,
                SubtitleColorPopup *pSubtitleColorPopup)
{
	if (pSubtitleColorPopup == NULL) {
		VideoLogError("pSubtitleColorPopup is NULL");
		return;
	}

	if (pSubtitleColorPopup->pColorplaneData == NULL) {
		VideoLogError("pSubtitleColorPopup is NULL");
		return;
	}
	VideoLogInfo("");

	SubtitleColorplaneData *cp = pSubtitleColorPopup->pColorplaneData;

	int nIndex =
	    _vp_subtitle_color_get_platte_index(pSubtitleColorPopup->
	                                        pCurrentColorHex);

	/* add color palette widget */
	Eina_List *last_list;
	const Eina_List *color_list;

	cp->layout = layout;
	cp->colorselector = elm_colorselector_add(layout);
	elm_object_style_set(cp->colorselector, "colorplane");
	//elm_colorselector_mode_set(cp->colorselector, ELM_COLORSELECTOR_PALETTE_PLANE);
	evas_object_size_hint_fill_set(cp->colorselector, EVAS_HINT_FILL,
	                               EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(cp->colorselector, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	elm_object_part_content_set(layout,
	                            "pv.subtitle-color-popup.colorpalette.swallow",
	                            cp->colorselector);
	evas_object_smart_callback_add(cp->colorselector,
	                               "color,item,selected",
	                               _vp_play_subtitle_color_popup_colorpalette_cb,
	                               pSubtitleColorPopup);

	edje_object_message_signal_process(elm_layout_edje_get(layout));
	elm_colorselector_palette_color_add(cp->colorselector, 255, 255, 255, 255);	//white
	elm_colorselector_palette_color_add(cp->colorselector, 0, 0, 0, 255);	//black
	elm_colorselector_palette_color_add(cp->colorselector, 255, 0, 0, 255);	//red
	elm_colorselector_palette_color_add(cp->colorselector, 0, 255, 0, 255);	//green
	elm_colorselector_palette_color_add(cp->colorselector, 0, 0, 255, 255);	//blue
	elm_colorselector_palette_color_add(cp->colorselector, 255, 255, 0, 255);	//yellow
	elm_colorselector_palette_color_add(cp->colorselector, 255, 0, 255, 255);	//purple
	elm_colorselector_palette_color_add(cp->colorselector, 0, 255, 255, 255);	//cyan
	elm_colorselector_palette_color_add(cp->colorselector, 0, 0, 0, 255);	//auto

	color_list = elm_colorselector_palette_items_get(cp->colorselector);
	last_list = eina_list_last(color_list);
	cp->it_last = eina_list_data_get(last_list);
	elm_object_item_signal_emit(cp->it_last, "elm,state,custom,show", "");
	cp->changed = EINA_FALSE;
	evas_object_smart_callback_add(cp->colorselector, "changed",
	                               _vp_play_subtitle_color_popup_colorplane_cb,
	                               pSubtitleColorPopup);
	cp->sel_it = eina_list_nth(color_list, nIndex);
	elm_object_item_signal_emit(cp->sel_it, "elm,state,selected", "elm");

	if (nIndex == 8) {
		int r, g, b, a;
		vp_play_util_convert_hex_to_rgba(pSubtitleColorPopup->
		                                 pCurrentColorHex, &r, &g, &b,
		                                 &a);
		elm_colorselector_palette_item_color_set(cp->it_last, r, g, b,
		        255);
	}
}

static Evas_Object
*_vp_play_subtitle_color_popup_create_layout(Evas_Object *pParent)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	Evas_Object *pLayout = elm_layout_add(pParent);
	if (pLayout == NULL) {
		VideoLogError("pLayout object is NULL");
		return NULL;
	}

	elm_layout_file_set(pLayout, VP_PLAY_SUBTITLE_COLOR_POPUP_EDJ,
	                    VP_PLAY_EDJ_GROUP_PLAY_SUBTITLE_COLOR_POPUP);

	evas_object_size_hint_weight_set(pLayout, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pLayout, EVAS_HINT_FILL,
	                                EVAS_HINT_FILL);

	elm_object_part_text_set(pLayout, "pv.subtitle-color-popup.subject",
	                         VP_PLAY_STRING_SUBTITLE_TEXT);
	elm_object_part_text_set(pLayout,
	                         "pv.subtitle-color-popup.opacity.text",
	                         VP_PLAY_STRING_SUBTITLE_OPACITY);

	evas_object_show(pLayout);

	return pLayout;

}

static void _vp_play_subtitle_color_update_text(SubtitleColorPopup *
                        pSubtitleColorPopup)
{
	if (pSubtitleColorPopup == NULL) {
		VideoLogError("pSubtitleColorPopup is NULL");
		return;
	}

	char szValue[6] = { 0, };

	snprintf(szValue, 6, "%2d %%",
	         (int) pSubtitleColorPopup->nOpacityVal);
	elm_object_part_text_set(pSubtitleColorPopup->pLayout,
	                         "pv.subtitle-color-popup.opacity.percent",
	                         szValue);

}

static void __vp_play_subtitle_color_btn_clicked_cb(void *pUserData,
                        Evas_Object *pObj,
                        void *pEvent)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleColorPopup *pSubtitleColorPopup =
	    (SubtitleColorPopup *) pUserData;

	int nValue = elm_slider_value_get(pSubtitleColorPopup->pSlider);
	VideoLogDebug("== %d ==", nValue);

	if (pObj == pSubtitleColorPopup->pAddButton) {
		nValue += 1;
		if (nValue > (int) VP_PLAY_OPACITY_MAX_VALUE) {
			nValue = (int) VP_PLAY_OPACITY_MAX_VALUE;
		}
		elm_slider_value_set(pSubtitleColorPopup->pSlider, nValue);

		if (nValue != pSubtitleColorPopup->nOpacityVal) {
			//if (pSubtitleColorPopup->pUpdateCb) {
			pSubtitleColorPopup->nOpacityVal = nValue;
			//pSubtitleColorPopup->pUpdateCb(pSubtitleColorPopup->fOpacityVal, (void *)pSubtitleColorPopup->pUserData);
			_vp_play_subtitle_color_update_text(pSubtitleColorPopup);
			//}
		}

	} else if (pObj == pSubtitleColorPopup->pDelButton) {
		nValue -= 1;
		if (nValue < (double) VP_PLAY_OPACITY_MIN_VALUE) {
			nValue = (double) VP_PLAY_OPACITY_MIN_VALUE;
		}

		elm_slider_value_set(pSubtitleColorPopup->pSlider, nValue);
		if (nValue != pSubtitleColorPopup->nOpacityVal) {
			//if (pSubtitleColorPopup->pUpdateCb) {
			pSubtitleColorPopup->nOpacityVal = nValue;
			//pSubtitleColorPopup->pUpdateCb(pSubtitleColorPopup->fOpacityVal, (void *)pSubtitleColorPopup->pUserData);
			_vp_play_subtitle_color_update_text(pSubtitleColorPopup);
			//}
		}
	}
}

static bool _vp_play_subtitle_color_popup_create_button(Evas_Object *
                        pParent,
                        void *pUserData)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return FALSE;
	}

	SubtitleColorPopup *pSubtitleColorPopup =
	    (SubtitleColorPopup *) pUserData;

	pSubtitleColorPopup->pDelButton =
	    vp_button_create(pParent, "playview/custom/flat_63_78/default",
	                     NULL,
	                     (Evas_Smart_Cb)
	                     __vp_play_subtitle_color_btn_clicked_cb,
	                     (Evas_Smart_Cb) NULL, (Evas_Smart_Cb) NULL,
	                     (void *) pSubtitleColorPopup);

	pSubtitleColorPopup->pAddButton =
	    vp_button_create(pParent, "playview/custom/flat_63_78/default",
	                     NULL,
	                     (Evas_Smart_Cb)
	                     __vp_play_subtitle_color_btn_clicked_cb,
	                     (Evas_Smart_Cb) NULL, (Evas_Smart_Cb) NULL,
	                     (void *) pSubtitleColorPopup);

	Evas_Object *pIcon = NULL;

	pIcon =
	    vp_button_create_icon(pSubtitleColorPopup->pDelButton,
	                          VP_PLAY_RESROUCE_EDJ_PATH,
	                          VP_PLAY_RES_BUTTON_DEL);
	elm_object_part_content_set(pSubtitleColorPopup->pDelButton,
	                            VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pSubtitleColorPopup->pDelButton,
	                          VP_PLAY_RESROUCE_EDJ_PATH,
	                          VP_PLAY_RES_BUTTON_DEL_PRESS);
	elm_object_part_content_set(pSubtitleColorPopup->pDelButton,
	                            VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pSubtitleColorPopup->pDelButton,
	                          VP_PLAY_RESROUCE_EDJ_PATH,
	                          VP_PLAY_RES_BUTTON_DEL);
	elm_object_part_content_set(pSubtitleColorPopup->pDelButton,
	                            VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pSubtitleColorPopup->pAddButton,
	                          VP_PLAY_RESROUCE_EDJ_PATH,
	                          VP_PLAY_RES_BUTTON_ADD);
	elm_object_part_content_set(pSubtitleColorPopup->pAddButton,
	                            VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pSubtitleColorPopup->pAddButton,
	                          VP_PLAY_RESROUCE_EDJ_PATH,
	                          VP_PLAY_RES_BUTTON_ADD_PRESS);
	elm_object_part_content_set(pSubtitleColorPopup->pAddButton,
	                            VP_PLAY_SWALLOW_BUTTON_PRESS_ICON, pIcon);

	pIcon =
	    vp_button_create_icon(pSubtitleColorPopup->pAddButton,
	                          VP_PLAY_RESROUCE_EDJ_PATH,
	                          VP_PLAY_RES_BUTTON_ADD);
	elm_object_part_content_set(pSubtitleColorPopup->pAddButton,
	                            VP_PLAY_SWALLOW_BUTTON_DIM_ICON, pIcon);

	elm_object_part_content_set(pParent, "elm.swallow.icon.1",
	                            pSubtitleColorPopup->pDelButton);
	elm_object_part_content_set(pParent, "elm.swallow.icon.2",
	                            pSubtitleColorPopup->pAddButton);

	return TRUE;
}

static void __vp_play_subtitle_color_popup_slider_change_cb(void
                        *pUserData,
                        Evas_Object *pObj,
                        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}
	SubtitleColorPopup *pSubtitleColor = (SubtitleColorPopup *) pUserData;

	int nValue = elm_slider_value_get(pSubtitleColor->pSlider);
	VideoLogDebug("== %d ==", nValue);

	if (nValue != pSubtitleColor->nOpacityVal) {
		//if (pSubtitleColor->pUpdateCb) {
		pSubtitleColor->nOpacityVal = nValue;
		//pSubtitleColor->pUpdateCb(pSubtitleColor->fOpacityVal, (void *)pSubtitleColor->pUserData);
		_vp_play_subtitle_color_update_text(pSubtitleColor);
		//}
	}

	char szValue[6] = { 0, };
	snprintf(szValue, 6, "%2d %%", (int) pSubtitleColor->nOpacityVal);
	int realValue = atoi(szValue);

	if (pSubtitleColor->nOpacityVal != realValue) {
		elm_slider_value_set(pSubtitleColor->pSlider, (int) realValue);
		pSubtitleColor->nOpacityVal = realValue;
	}

}

static Evas_Object
*_vp_play_subtitle_color_popup_create_slider(Evas_Object *pParent,
        void *pUserData)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}
	Evas_Object *pObj = NULL;

	pObj = elm_slider_add(pParent);
	elm_slider_indicator_show_set(pObj, EINA_FALSE);
	evas_object_size_hint_weight_set(pObj, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(pObj, EVAS_HINT_FILL, 0.5);

	elm_slider_min_max_set(pObj, 0, 100);
	evas_object_smart_callback_add(pObj, "changed",
	                               __vp_play_subtitle_color_popup_slider_change_cb,
	                               pUserData);

	return pObj;
}

#endif

/* external functions */
#ifndef SUBTITLE_K_FEATURE
subtitle_color_handle vp_subtitle_color_create(Evas_Object *pParent,
        SubtitlePopupCloseCbFunc
        pCloseCb,
        int nDefaultColor)
#else
subtitle_color_handle vp_subtitle_color_hex_create(Evas_Object *pParent,
        SubtitlePopupCloseCbFunc
        pCloseCb,
        char *pDefaultColor)
#endif
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	SubtitleColorPopup *pSubtitleColor = NULL;

	pSubtitleColor = calloc(1, sizeof(SubtitleColorPopup));

	if (pSubtitleColor == NULL) {
		VideoLogError("pSubtitleColor alloc fail");
		return NULL;
	}

	pSubtitleColor->pParent = pParent;
	pSubtitleColor->pCloseCb = pCloseCb;

#ifndef SUBTITLE_K_FEATURE
	pSubtitleColor->pPopup =
	    vp_popup_create(pParent, POPUP_STYLE_MENU_LIST_NO_CANCEL_BTN,
	                    VP_PLAY_STRING_SUBTITLE_TEXT, NULL, 0.0, NULL,
	                    __vp_subtitle_color_popup_key_event_cb,
	                    __vp_subtitle_color_popup_mouse_event_cb,
	                    (void *) pSubtitleColor);
#else
	pSubtitleColor->pPopup =
	    vp_popup_create(pParent, POPUP_STYLE_EXPAND_WITH_CANCEL_BTN,
	                    VP_PLAY_STRING_SUBTITLE_COLOR_AND_OPACITY, NULL,
	                    0.0, NULL, __vp_subtitle_color_popup_key_event_cb,
	                    __vp_subtitle_color_popup_mouse_event_cb,
	                    (void *) pSubtitleColor);
#endif
	if (pSubtitleColor->pPopup == NULL) {
		VideoLogError("vp_popup_create fail");
		_vp_subtitle_color_destroy_handle(pSubtitleColor);
		return NULL;
	}
#ifndef SUBTITLE_K_FEATURE
	pSubtitleColor->pGenList =
	    _vp_subtitle_color_create_genlist(pSubtitleColor->pPopup);
	if (pSubtitleColor->pGenList == NULL) {
		VideoLogError("_vp_subtitle_color_create_genlist fail");
		_vp_subtitle_color_destroy_handle(pSubtitleColor);
		return NULL;
	}
	evas_object_data_set(pSubtitleColor->pGenList,
	                     VP_SUBTITLE_COLOR_GENLIST_DATA_KEY,
	                     (void *) pSubtitleColor);
	evas_object_smart_callback_add(pSubtitleColor->pGenList, "realized",
	                               __vp_subtitle_color_genlist_realized,
	                               NULL);

	pSubtitleColor->nDefaultIndex = nDefaultColor;
	pSubtitleColor->pRadio = elm_radio_add(pSubtitleColor->pGenList);

	if (!_vp_subtitle_color_add_genlist_item
	        (pSubtitleColor->pGenList, (void *) pSubtitleColor)) {
		VideoLogError("_vp_subtitle_color_add_genlist_item fail");
		return FALSE;
	}
	//elm_radio_value_set(pSubtitleColor->pRadio, nDefaultColor);
	pSubtitleColor->currentFontColor = nDefaultColor;
	evas_object_smart_callback_add(pSubtitleColor->pParent,
	                               "rotation,changed",
	                               __vp_subtitle_color_popup_rotate_cb,
	                               pSubtitleColor);

	pSubtitleColor->pBox = elm_box_add(pSubtitleColor->pPopup);
	vp_popup_set_popup_min_size(pSubtitleColor->pParent,
	                            pSubtitleColor->pBox,
	                            pSubtitleColor->nListCount,
	                            VIDEO_POPUP_DEFAULT);
	elm_box_pack_end(pSubtitleColor->pBox, pSubtitleColor->pGenList);
	elm_object_content_set(pSubtitleColor->pPopup, pSubtitleColor->pBox);
#else
	pSubtitleColor->pCurrentColorHex = strdup(pDefaultColor);

	pSubtitleColor->pCancelButton =
	    _vp_play_subtitle_color_popup_create_cancel_button
	    (pSubtitleColor->pPopup, (void *) pSubtitleColor);
	if (pSubtitleColor->pCancelButton == NULL) {
		VideoLogError("_vp_play_speed_popup_create_cancel_button fail");
		_vp_subtitle_color_destroy_handle(pSubtitleColor);
		return NULL;
	}

	pSubtitleColor->pOkButton =
	    _vp_play_subtitle_color_popup_create_ok_button(pSubtitleColor->
	            pPopup,
	            (void *)
	            pSubtitleColor);
	if (pSubtitleColor->pOkButton == NULL) {
		VideoLogError("_vp_play_speed_popup_create_ok_button fail");
		_vp_subtitle_color_destroy_handle(pSubtitleColor);
		return NULL;
	}

	pSubtitleColor->pLayout =
	    _vp_play_subtitle_color_popup_create_layout(pSubtitleColor->
	            pPopup);
	if (pSubtitleColor->pLayout == NULL) {
		VideoLogError("_vp_play_speed_popup_create_layout fail");
		_vp_subtitle_color_destroy_handle(pSubtitleColor);
		return NULL;
	}

	pSubtitleColor->pColorplaneData =
	    (SubtitleColorplaneData *) malloc(sizeof(SubtitleColorplaneData));
	if (!pSubtitleColor->pColorplaneData) {
		return NULL;
	}
	memset(pSubtitleColor->pColorplaneData, 0,
	       sizeof(SubtitleColorplaneData));

	_vp_play_subtitle_color_popup_create_colorpalette(pSubtitleColor->
	        pLayout,
	        pSubtitleColor);

	pSubtitleColor->pSlider =
	    _vp_play_subtitle_color_popup_create_slider(pSubtitleColor->
	            pLayout,
	            pSubtitleColor);
	if (pSubtitleColor->pSlider == NULL) {
		VideoLogError("_vp_play_subtitle_color_popup_create_slider fail");
		_vp_subtitle_color_destroy_handle(pSubtitleColor);
		return NULL;
	}
	_vp_play_subtitle_color_popup_create_button(pSubtitleColor->pLayout,
	        pSubtitleColor);
	pSubtitleColor->nOpacityVal =
	    _vp_subtitle_color_get_opacity(pSubtitleColor->pCurrentColorHex);
	elm_slider_value_set(pSubtitleColor->pSlider,
	                     (int) pSubtitleColor->nOpacityVal);
	_vp_play_subtitle_color_update_text(pSubtitleColor);

	elm_object_part_content_set(pSubtitleColor->pLayout,
	                            "elm.swallow.content",
	                            pSubtitleColor->pSlider);

	//Used for scroller layout:
	//popup->LayoutOut->Scroller->pLayout
	pSubtitleColor->pLayoutOut = elm_layout_add(pSubtitleColor->pPopup);
	int nRotation = elm_win_rotation_get(pSubtitleColor->pParent);
	if (nRotation == 90 || nRotation == 270)
		elm_layout_file_set(pSubtitleColor->pLayoutOut,
		                    VP_PLAY_SUBTITLE_COLOR_POPUP_EDJ,
		                    "pv.subtitle.scroller.layout.landscape");
	else
		elm_layout_file_set(pSubtitleColor->pLayoutOut,
		                    VP_PLAY_SUBTITLE_COLOR_POPUP_EDJ,
		                    "pv.subtitle.scroller.layout");
	evas_object_size_hint_weight_set(pSubtitleColor->pLayoutOut,
	                                 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	pSubtitleColor->pScroller =
	    elm_scroller_add(pSubtitleColor->pLayoutOut);
	elm_scroller_bounce_set(pSubtitleColor->pScroller, EINA_TRUE,
	                        EINA_TRUE);
	elm_scroller_policy_set(pSubtitleColor->pScroller,
	                        ELM_SCROLLER_POLICY_OFF,
	                        ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(pSubtitleColor->pScroller);
	elm_object_content_set(pSubtitleColor->pScroller,
	                       pSubtitleColor->pLayout);

	elm_object_part_content_set(pSubtitleColor->pLayoutOut,
	                            "elm.swallow.content",
	                            pSubtitleColor->pScroller);
	elm_object_content_set(pSubtitleColor->pPopup,
	                       pSubtitleColor->pLayoutOut);

	evas_object_smart_callback_add(pSubtitleColor->pParent,
	                               "rotation,changed",
	                               __vp_subtitle_color_popup_rotate_cb,
	                               pSubtitleColor);
	evas_object_show(pSubtitleColor->pLayout);

#endif
	return pSubtitleColor;
}

void vp_subtitle_color_destroy(subtitle_color_handle pSubtitleColorHandle)
{
	if (pSubtitleColorHandle == NULL) {
		VideoLogError("pSubtitleColorHandle is NULL");
		return;
	}

	SubtitleColorPopup *pSubtitleColor =
	    (SubtitleColorPopup *) pSubtitleColorHandle;

	_vp_subtitle_color_destroy_handle(pSubtitleColor);

}

bool vp_subtitle_color_realize(subtitle_color_handle pSubtitleColorHandle)
{
	if (pSubtitleColorHandle == NULL) {
		VideoLogError("pSubtitleColorHandle is NULL");
		return FALSE;
	}

	SubtitleColorPopup *pSubtitleColor =
	    (SubtitleColorPopup *) pSubtitleColorHandle;

	evas_object_show(pSubtitleColor->pPopup);

	return TRUE;
}

bool vp_subtitle_color_unrealize(subtitle_color_handle
                                 pSubtitleColorHandle)
{
	if (pSubtitleColorHandle == NULL) {
		VideoLogError("pSubtitleColorHandle is NULL");
		return FALSE;
	}

	SubtitleColorPopup *pSubtitleColor =
	    (SubtitleColorPopup *) pSubtitleColorHandle;

	evas_object_hide(pSubtitleColor->pPopup);

	return TRUE;
}

bool vp_subtitle_color_set_user_data(subtitle_color_handle
                                     pSubtitleColorHandle,
                                     void *pUserData)
{
	if (pSubtitleColorHandle == NULL) {
		VideoLogError("pSubtitleColorHandle is NULL");
		return FALSE;
	}

	SubtitleColorPopup *pSubtitleColor =
	    (SubtitleColorPopup *) pSubtitleColorHandle;

	pSubtitleColor->pUserData = pUserData;

	return TRUE;
}
