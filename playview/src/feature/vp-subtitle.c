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
#include <efl_extension.h>

#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-macro-define.h"
#include "vp-play-type-define.h"

#include "vp-subtitle.h"
#include "vp-play-preference.h"
#include "vp-util.h"
#include "vp-play-util.h"

/* check temp */
#include "vp-play-log.h"


/** @li font_weight - Overrides the weight defined in "font". E.g: "font_weight=Bold" is the same as "font=:style=Bold". Supported weights: "normal", "thin", "ultralight",
	"light", "book", "medium", "semibold", "bold", "ultrabold", "black", and "extrablack".
* @li font_style - Overrides the style defined in "font". E.g: "font_style=Italic" is the same as "font=:style=Italic". Supported styles: "normal", "oblique", and "italic".
* @li font_width - Overrides the width defined in "font". E.g: "font_width=Condensed" is the same as "font=:style=Condensed". Supported widths: "normal",
	"ultracondensed", "extracondensed", "condensed", "semicondensed", "semiexpanded", "expanded", "extraexpanded", and "ultraexpanded".
*/

#define VP_SUBTITLE_GENLIST_DATA_KEY		"vp.subtitle.genlist"
#define VP_SUBTITLE_TXT_FORMAT "<font_size= %d><align=center><style=%s><font='%s' font_weight=%s color=%s backing_color=%s backing=%s align=%s>%s</font></style></font_size>"
#define	VP_SUBTITLE_COLOR_LEN	7
#define VP_SUBTITLE_SET_CAPTION_WINDOW_PREVIEW_COLOR_ID		1

typedef struct _SubtitlePopup {
	Evas_Object		*pParent;
	Evas_Object		*pPopup;
	Evas_Object		*pPopupTitle;
	Evas_Object		*pCaptionLayout;
	Evas_Object		*pBox;
	Evas_Object		*pContentBox;
	Evas_Object		*pGenList;

	Evas_Object		*pActivationCheck;
	Elm_Object_Item 	*pSelectItem;

	Elm_Genlist_Item_Class 	*st_Subtitle_Itc_1;
	Elm_Genlist_Item_Class 	*st_Subtitle_Itc_2;
	Elm_Genlist_Item_Class 	*st_Subtitle_Itc_1icon;
	Elm_Genlist_Item_Class 	*st_Subtitle_Itc_groupindex;
#ifdef SUBTITLE_K_FEATURE
	Elm_Genlist_Item_Class 	*st_Subtitle_Itc_3;
#endif
	SubtitleInfo		*pSubtitleInfo;

	void 			*pUserData;
	PopupCloseCbFunc	pCloseCb;

}SubtitlePopup;

static void _vp_subtitle_destroy_handle(SubtitlePopup *pSubtitle);
char *_vp_subtitle_get_opacity(char *colorHex);
Elm_Object_Item *_vp_subtitle_list_item_next_get(Elm_Object_Item *gl_item);
static void _vp_subtitle_set_activation(SubtitlePopup *pSubtitle, bool bOn);
static void _vp_subtitle_destroy_subtitle_info(SubtitlePopup *pSubtitle);
static Evas_Object * _vp_subtitle_create_caption_layout(SubtitlePopup *pSubtitle, Evas_Object *pParent);
static void _vp_subtitle_cancel_button_cb(void *data, Evas_Object *obj, void *Event_info);

static void __vp_subtitle_genlist_realized(void *data, Evas_Object *obj, void *event_info)
{
	if (event_info == NULL) {
		VideoLogError("pEventInfo is NULL");
		return;
	}

	if (obj == NULL) {
		VideoLogError("obj is NULL");
		return;
	}

	Elm_Object_Item *pItem = (Elm_Object_Item *)event_info;

	int nIndexGenlist = elm_genlist_item_index_get(pItem);
	VideoLogInfo("nIndexGenlist=%d", nIndexGenlist);

	if (nIndexGenlist == 2) {
		elm_object_item_signal_emit(pItem, "elm,state,bottomline,hide", "");
	}

	VP_GENLIST_HIDE_BOTTOMLINE(data, obj, event_info);
}

static void __vp_subtitle_popup_size_set(SubtitlePopup *pSubtitle)
{
	if (!pSubtitle) {
		VideoLogError("pSubtitle is null");
		return;
	}

	if (!pSubtitle->pParent) {
		VideoLogError("pParent is null");
		return;
	}

	if (!pSubtitle->pBox) {
		VideoLogError("pBox is null");
		return;
	}

	if (!pSubtitle->pContentBox) {
		VideoLogError("pContentBox is null");
		return;
	}

	int nWidth= 0, nHeight = 0;
	if (vp_popup_check_landspace_by_win(pSubtitle->pParent)) {
		nWidth = 480;
		nHeight = VP_POPUP_MAX_HEIGHT_L;
	} else {
		nWidth = 480;
		nHeight = VP_POPUP_MAX_HEIGHT;
	}
	VideoLogInfo("nWidth=%d, nHeight=%d", nWidth, nHeight);
	evas_object_size_hint_min_set(pSubtitle->pBox, nWidth,  (nHeight) * VP_SCALE);
	//evas_object_size_hint_min_set(pSubtitle->pPopupTitle, nWidth * VP_SCALE,  (VP_POPUP_TITLE_HEIGHT) * VP_SCALE);
	evas_object_size_hint_min_set(pSubtitle->pContentBox, nWidth, (nHeight-VP_POPUP_TITLE_HEIGHT) * VP_SCALE);
}

static void __vp_subtitle_popup_rotate_cb(void *data, Evas_Object *obj, void *event_info)
{
	SubtitlePopup *pSubtitle = (SubtitlePopup *)data;
	if (!pSubtitle)
	{
		VideoLogError("pSubtitle IS null");
		return;
	}
	__vp_subtitle_popup_size_set(pSubtitle);

	evas_object_show(pSubtitle->pBox);
	evas_object_show(pSubtitle->pPopup);
}


/* callback functions */
static void __vp_subtitle_check_change_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitlePopup *pSubtitle = (SubtitlePopup *)pUserData;

	if (pObj == pSubtitle->pActivationCheck) {
		bool bOn = FALSE;
		vp_play_preference_get_subtitle_show_key(&bOn);
		vp_play_preference_set_subtitle_show_key(!bOn);
		//elm_check_state_set(pObj, !bOn);

		vp_subtitle_mode_t nType =  VP_SUBTITLE_MODE_NONE;
		nType = VP_SUBTITLE_MODE_ACTIVATION;
		_vp_subtitle_set_activation(pSubtitle, !bOn);

		if (pSubtitle->pCloseCb) {
			pSubtitle->pCloseCb((int)nType, FALSE, (void *)pSubtitle->pUserData);
		}
	}
}

static char *_vp_play_subtitle_text_get(SubtitleInfo *pSubtitleInfo, bool bActivation)
{
	if (!pSubtitleInfo) {
		VideoLogError("pSubtitle is NULL");
		return NULL;
	}
	if (!pSubtitleInfo->szText || (strlen(pSubtitleInfo->szText) < 1)) {
		VideoLogError("pSubtitleInfo->szText is NULL");
		return NULL;
	}

	char *szMarkup = elm_entry_markup_to_utf8(pSubtitleInfo->szText);
	char *szOutput = elm_entry_utf8_to_markup(szMarkup);
	char *szTxtFormat = NULL;
	char *szColor = NULL;
	char *szBGColor = NULL;
	char *szBGColorOn= NULL;
	char *szFontEdge = NULL;
	char *szAlignment = NULL;
	char *szFontWeight = NULL;

	int nSize = 0;

	if (pSubtitleInfo->nFontSize == VIDEO_SUBTITLE_SIZE_SMALL) {
		nSize = VP_SUBTITLE_SIZE_SMALL_VALUE * pSubtitleInfo->fZoom;
	}
	else if (pSubtitleInfo->nFontSize == VIDEO_SUBTITLE_SIZE_LARGE) {
		nSize = VP_SUBTITLE_SIZE_LARGE_VALUE * pSubtitleInfo->fZoom;
	}
	else {
		nSize = VP_SUBTITLE_SIZE_MEDIUM_VALUE * pSubtitleInfo->fZoom;
	}

	if (nSize < VP_SUBTITLE_SIZE_SMALL_VALUE) {
		nSize = VP_SUBTITLE_SIZE_SMALL_VALUE;
	}

	if (nSize > VP_SUBTITLE_SIZE_LARGE_VALUE) {
		nSize = VP_SUBTITLE_SIZE_LARGE_VALUE;
	}

#ifndef SUBTITLE_K_FEATURE
	if (pSubtitleInfo->nTextColor == VIDEO_SUBTITLE_COLOR_BLACK) {
		szColor = g_strdup_printf("#000000");
	}
	else if (pSubtitleInfo->nTextColor == VIDEO_SUBTITLE_COLOR_BLUE) {
		szColor = g_strdup_printf("#0000FF");
	}
	else if (pSubtitleInfo->nTextColor == VIDEO_SUBTITLE_COLOR_GREEN) {
		szColor = g_strdup_printf("#00FF00");
	}
	else if (pSubtitleInfo->nTextColor == VIDEO_SUBTITLE_COLOR_WHITE) {
		szColor = g_strdup_printf("#FFFFFF");
	}

	if (pSubtitleInfo->nBGColor == VIDEO_SUBTITLE_COLOR_NONE) {
		szBGColorOn = g_strdup_printf("off");
		szBGColor = g_strdup_printf("#000000");
	}
	else if (pSubtitleInfo->nBGColor == VIDEO_SUBTITLE_COLOR_BLACK) {
		szBGColorOn = g_strdup_printf("on");
		szBGColor = g_strdup_printf("#000000");
	}
	else if (pSubtitleInfo->nBGColor == VIDEO_SUBTITLE_COLOR_WHITE) {
		szBGColorOn = g_strdup_printf("on");
		szBGColor = g_strdup_printf("#FFFFFF");
	}
#else

	if (pSubtitleInfo->pTextColorHex && bActivation) {
		szColor = g_strdup_printf("#000000ff");//strdup(pSubtitleInfo->pTextColorHex);
	} else {
		szColor = g_strdup_printf("#00000000");
	}

	if (pSubtitleInfo->pBGColorHex && bActivation) {
		szBGColorOn = g_strdup_printf("on");
		szBGColor = strdup(pSubtitleInfo->pBGColorHex);
	} else {
		szBGColorOn = g_strdup_printf("off");
		szBGColor = g_strdup_printf("#00000000");
	}

	if (pSubtitleInfo->eAlignment == VIDEO_SUBTITLE_ALIGNMENT_LEFT) {
		szAlignment = g_strdup_printf("left");
	}
	else if (pSubtitleInfo->eAlignment == VIDEO_SUBTITLE_ALIGNMENT_CENTER) {
		szAlignment = g_strdup_printf("center");
	}
	else if (pSubtitleInfo->eAlignment == VIDEO_SUBTITLE_ALIGNMENT_RIGHT) {
		szAlignment = g_strdup_printf("right");
	}

#endif
	szTxtFormat = g_strdup_printf(VP_SUBTITLE_TXT_FORMAT, nSize, szFontEdge, pSubtitleInfo->szFontName, szFontWeight, szColor, szBGColor, szBGColorOn, szAlignment, szOutput);

	VP_FREE(szBGColor);
	VP_FREE(szBGColorOn);
	VP_FREE(szColor);
	VP_FREE(szOutput);
	VP_FREE(szMarkup);
	VP_FREE(szFontEdge);
	VP_FREE(szFontWeight);
	VP_FREE(szAlignment);

	return szTxtFormat;
}

static char *__vp_subtitle_get_color_and_opacity(char *szColorHex)
{
	if (szColorHex == NULL) {
		VideoLogError("szColorHex is NULL");
		return NULL;
	}

	char *szColorAndOpacity = NULL;
	char *szOpacity = _vp_subtitle_get_opacity(szColorHex);
	VideoSecureLogInfo("========pSubtitleInfo->nCaptionWinColorHex=%s, szOpacity=%s", szColorHex, szOpacity);

	if (!strncasecmp(szColorHex, "#000000", VP_SUBTITLE_COLOR_LEN)) {
		szColorAndOpacity = g_strdup_printf("%s,%s", VP_PLAY_STRING_SUBTITLE_COLOR_BLACK, szOpacity);
	}
	else if (!strncasecmp(szColorHex, "#0000ff", VP_SUBTITLE_COLOR_LEN)) {
		szColorAndOpacity = g_strdup_printf("%s,%s", VP_PLAY_STRING_SUBTITLE_COLOR_BLUE, szOpacity);
	}
	else if (!strncasecmp(szColorHex, "#00ff00", VP_SUBTITLE_COLOR_LEN)) {
		szColorAndOpacity = g_strdup_printf("%s,%s", VP_PLAY_STRING_SUBTITLE_COLOR_GREEN, szOpacity);
	}
	else if (!strncasecmp(szColorHex, "#ffffff", VP_SUBTITLE_COLOR_LEN)) {
		szColorAndOpacity = g_strdup_printf("%s,%s", VP_PLAY_STRING_SUBTITLE_COLOR_WHITE, szOpacity);
	}
	else if (!strncasecmp(szColorHex, "#ff0000", VP_SUBTITLE_COLOR_LEN)) {
		szColorAndOpacity = g_strdup_printf("%s,%s", VP_PLAY_STRING_SUBTITLE_COLOR_RED, szOpacity);
	}
	else if (!strncasecmp(szColorHex, "#ffff00", VP_SUBTITLE_COLOR_LEN)) {
		szColorAndOpacity = g_strdup_printf("%s,%s", VP_PLAY_STRING_SUBTITLE_COLOR_YELLOW, szOpacity);
	}
	else if (!strncasecmp(szColorHex, "#ff00ff", VP_SUBTITLE_COLOR_LEN)) {
		szColorAndOpacity = g_strdup_printf("%s,%s", VP_PLAY_STRING_SUBTITLE_COLOR_MAGENTA, szOpacity);
	}
	else if (!strncasecmp(szColorHex, "#00ffff", VP_SUBTITLE_COLOR_LEN)) {
		szColorAndOpacity = g_strdup_printf("%s,%s", VP_PLAY_STRING_SUBTITLE_COLOR_CYAN, szOpacity);
	}
	else {
		szColorAndOpacity = g_strdup_printf("%s,%s", VP_PLAY_STRING_SUBTITLE_COLOR_CUSTOM, szOpacity);
	}

	VP_FREE(szOpacity);
	return szColorAndOpacity;
}

static char *__vp_subtitle_genlist_text_get_cb(const void *pUserData, Evas_Object *pObj, const char *pPart)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}

	char *szTxt= (char *)pUserData;

	if (!strcmp(pPart, "elm.text.main.left")) {
		if (!g_strcmp0(VP_PLAY_STRING_ACTIVATION, szTxt))
			return strdup(szTxt);
	}

	if (!g_strcmp0(VP_PLAY_STRING_SETTINGS, szTxt)) {
		if (!strcmp(pPart, "elm.text.main")) {
			return strdup(szTxt);
		}
	}

	if (!g_strcmp0(VP_PLAY_STRING_SUBTITLE_SELECT_SUBTITLES, szTxt)
		|| !g_strcmp0(VP_PLAY_STRING_SUBTITLE_ALIGNMENT, szTxt) || !g_strcmp0(VP_PLAY_STRING_SUBTITLE_SIZE, szTxt)) {
		if (!strcmp(pPart, "elm.text.main.left.top")) {
			return strdup(szTxt);
		} else if (!strcmp(pPart, "elm.text.sub.left.bottom")) {
			SubtitlePopup *pSubtitle = (SubtitlePopup *)evas_object_data_get(pObj , VP_SUBTITLE_GENLIST_DATA_KEY);
			if (pSubtitle == NULL) {
				VideoLogWarning("evas_object_data_get is fail");
				return NULL;
			}

			SubtitleInfo *pSubtitleInfo = pSubtitle->pSubtitleInfo;
			if (pSubtitleInfo == NULL) {
				VideoLogWarning("pSubtitleInfo is NULL");
				return NULL;
			}

			//return g_strdup(pSubtitleInfo->szURL);

			if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_SELECT_SUBTITLES)) {
				if (pSubtitleInfo->szURL) {
					return vp_util_convert_file_location(pSubtitleInfo->szURL);
				}
			}
			else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_ALIGNMENT)) {
				switch (pSubtitleInfo->eAlignment)
				{
					case VIDEO_SUBTITLE_ALIGNMENT_LEFT:
						{
							return strdup(VP_PLAY_STRING_ALIGNMENT_LEFT);
						}
					case VIDEO_SUBTITLE_ALIGNMENT_CENTER:
						{
							return strdup(VP_PLAY_STRING_ALIGNMENT_CENTER);
						}
					case VIDEO_SUBTITLE_ALIGNMENT_RIGHT:
						{
							return strdup(VP_PLAY_STRING_ALIGNMENT_RIGHT);
						}
					default: break;
				}
			}
			else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_SIZE)) {
				if (pSubtitleInfo->nFontSize == VIDEO_SUBTITLE_SIZE_LARGE) {
					return strdup(VP_PLAY_STRING_SUBTITLE_SIZE_LARGE);
				}
				else if (pSubtitleInfo->nFontSize == VIDEO_SUBTITLE_SIZE_MEDIUM) {
					return strdup(VP_PLAY_STRING_SUBTITLE_SIZE_MEDIUM);
				}
				else if (pSubtitleInfo->nFontSize == VIDEO_SUBTITLE_SIZE_SMALL) {
					return strdup(VP_PLAY_STRING_SUBTITLE_SIZE_SMALL);
				}
			}
		}
	}
#if 1
	if (!strcmp(pPart, "elm.text")) {
		return strdup(szTxt);
	}
	else if (!strcmp(pPart, "elm.text.1")) {
		return strdup(szTxt);
	}
	else if (!strcmp(pPart, "elm.text.2")) {

		SubtitlePopup *pSubtitle = (SubtitlePopup *)evas_object_data_get(pObj , VP_SUBTITLE_GENLIST_DATA_KEY);
		if (pSubtitle == NULL) {
			VideoLogWarning("evas_object_data_get is fail");
			return NULL;
		}

		SubtitleInfo *pSubtitleInfo = pSubtitle->pSubtitleInfo;
		if (pSubtitleInfo == NULL) {
			VideoLogWarning("pSubtitleInfo is NULL");
			return NULL;
		}

		if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_SYNC)) {
			char	szTemp[10]	= {0,};
			snprintf(szTemp, sizeof(szTemp), "%0.1f", pSubtitleInfo->fSync);

			return g_strdup_printf(VP_PLAY_STRING_SUBTITLE_SECS, szTemp);
		}
		else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_LANGUAGE)) {
			if (pSubtitleInfo->szLanguage) {
				return strdup(pSubtitleInfo->szLanguage);
			}
		}
#ifdef SUBTITLE_K_FEATURE
		else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_ALIGNMENT)) {
			switch (pSubtitleInfo->eAlignment)
			{
				case VIDEO_SUBTITLE_ALIGNMENT_LEFT:
					{
						return strdup(VP_PLAY_STRING_ALIGNMENT_LEFT);
					}
				case VIDEO_SUBTITLE_ALIGNMENT_CENTER:
					{
						return strdup(VP_PLAY_STRING_ALIGNMENT_CENTER);
					}
				case VIDEO_SUBTITLE_ALIGNMENT_RIGHT:
					{
						return strdup(VP_PLAY_STRING_ALIGNMENT_RIGHT);
					}
				default: break;
			}
		}
#endif
		else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_SELECT_SUBTITLES)) {
			if (pSubtitleInfo->szURL) {
				return vp_util_convert_file_location(pSubtitleInfo->szURL);
			}
		}
		else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_FONT)) {
			if (pSubtitleInfo->szFontName) {
				return strdup(pSubtitleInfo->szFontName);
			}
		}
		else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_SIZE)) {
			if (pSubtitleInfo->nFontSize == VIDEO_SUBTITLE_SIZE_LARGE) {
				return strdup(VP_PLAY_STRING_SUBTITLE_SIZE_LARGE);
			}
			else if (pSubtitleInfo->nFontSize == VIDEO_SUBTITLE_SIZE_MEDIUM) {
				return strdup(VP_PLAY_STRING_SUBTITLE_SIZE_MEDIUM);
			}
			else if (pSubtitleInfo->nFontSize == VIDEO_SUBTITLE_SIZE_SMALL) {
				return strdup(VP_PLAY_STRING_SUBTITLE_SIZE_SMALL);
			}
		}
//#ifdef SUBTITLE_K_FEATURE
		else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_EDGE)) {
			switch (pSubtitleInfo->nEdge)
			{
				case VP_SUBTITLE_EDGE_NO_EDGE:
					return strdup(VP_PLAY_STRING_EDGE_NO_EDGE);
				case VP_SUBTITLE_EDGE_RAISED:
					return strdup(VP_PLAY_STRING_EDGE_RAISED);
				case VP_SUBTITLE_EDGE_DEPRESSED:
					return strdup(VP_PLAY_STRING_EDGE_DEPRESSED);
				case VP_SUBTITLE_EDGE_UNIFORM:
					return strdup(VP_PLAY_STRING_EDGE_UNIFORM);
				case VP_SUBTITLE_EDGE_DROP_SHADOW:
					return strdup(VP_PLAY_STRING_EDGE_DROP_SHADOW);
				default:
					return strdup(VP_PLAY_STRING_EDGE_NO_EDGE);
			}
		}
//#endif
		else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_TEXT)) {
#ifndef SUBTITLE_K_FEATURE
			if (pSubtitleInfo->nTextColor == VIDEO_SUBTITLE_COLOR_BLACK) {
				return strdup(VP_PLAY_STRING_SUBTITLE_COLOR_BLACK);
			}
			else if (pSubtitleInfo->nTextColor == VIDEO_SUBTITLE_COLOR_BLUE) {
				return strdup(VP_PLAY_STRING_SUBTITLE_COLOR_BLUE);
			}
			else if (pSubtitleInfo->nTextColor == VIDEO_SUBTITLE_COLOR_GREEN) {
				return strdup(VP_PLAY_STRING_SUBTITLE_COLOR_GREEN);
			}
			else if (pSubtitleInfo->nTextColor == VIDEO_SUBTITLE_COLOR_WHITE) {
				return strdup(VP_PLAY_STRING_SUBTITLE_COLOR_WHITE);
			}
#else
			return __vp_subtitle_get_color_and_opacity(pSubtitleInfo->pTextColorHex);
#endif
		}
		else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_BG_COLOR)) {
#ifndef SUBTITLE_K_FEATURE
			if (pSubtitleInfo->nBGColor == VIDEO_SUBTITLE_COLOR_BLACK) {
				return strdup(VP_PLAY_STRING_SUBTITLE_COLOR_BLACK);
			}
			else if (pSubtitleInfo->nBGColor == VIDEO_SUBTITLE_COLOR_WHITE) {
				return strdup(VP_PLAY_STRING_SUBTITLE_COLOR_WHITE);
			}
			else if (pSubtitleInfo->nBGColor == VIDEO_SUBTITLE_COLOR_NONE) {
				return strdup(VP_PLAY_STRING_SUBTITLE_COLOR_NONE);
			}
#else
			return __vp_subtitle_get_color_and_opacity(pSubtitleInfo->pBGColorHex);
#endif
		}
#ifdef SUBTITLE_K_FEATURE
		else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_CAPTION_WINDOW)) {
			return __vp_subtitle_get_color_and_opacity(pSubtitleInfo->pCaptionWinColorHex);
		}
#endif
	}
#endif
	return NULL;
}

static Evas_Object *__vp_subtitle_genlist_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	SubtitlePopup *pSubtitle = (SubtitlePopup *)evas_object_data_get(obj , VP_SUBTITLE_GENLIST_DATA_KEY);
	if (pSubtitle == NULL) {
		VideoLogWarning("evas_object_data_get is fail");
		return NULL;
	}
	VideoLogWarning("part = %s", part);

	if (!strcmp(part, "elm.icon")) {
		if (!g_strcmp0(VP_PLAY_STRING_PREVIEW_SUBTITLE_CC, (char *)data)) {
			Evas_Object *pCaption = _vp_subtitle_create_caption_layout(pSubtitle, obj);
			evas_object_size_hint_align_set(pCaption, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(pCaption, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			return pCaption;
		}
	}

	if (!strcmp(part, "elm.icon.right")) {
		if (!g_strcmp0(VP_PLAY_STRING_ACTIVATION, (char *)data)) {
			Evas_Object *pCheck = NULL;
			pCheck	= elm_check_add(obj);
			bool bShow = FALSE;
			vp_play_preference_get_subtitle_show_key(&bShow);
			evas_object_size_hint_align_set(pCheck, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(pCheck, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_check_state_set(pCheck, bShow);
			elm_object_style_set(pCheck, "on&off");
			evas_object_propagate_events_set(pCheck, EINA_FALSE);
			evas_object_smart_callback_add(pCheck, "changed", __vp_subtitle_check_change_cb, (void *)pSubtitle);
			pSubtitle->pActivationCheck = pCheck;
			evas_object_show(pCheck);
			return pCheck;
		}
	}

	return NULL;
}


static void __vp_subtitle_genlist_item_selected_cb(void *pUserData,Evas_Object *pObject,void *pEventInfo)
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
	char *szTxt = (char *)elm_object_item_data_get(pItem);
	elm_genlist_item_selected_set(pItem, EINA_FALSE);
	SubtitlePopup *pSubtitle = (SubtitlePopup *)pUserData;
	pSubtitle->pSelectItem = pItem;
	if (szTxt == NULL) {
		VideoLogError("szTxt is NULL");
		return;
	}
	vp_subtitle_mode_t nType =  VP_SUBTITLE_MODE_NONE;

	if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_SYNC)) {
		VideoLogWarning("SELECT SUBTITLE ITEM (VP_PLAY_STRING_SUBTITLE_SYNC)");
		nType = VP_SUBTITLE_MODE_SYNC;
		if (pSubtitle->pCloseCb) {
			pSubtitle->pCloseCb((int)nType, TRUE, (void *)pSubtitle->pUserData);
		}
	}
	else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_SELECT_SUBTITLES)) {
		VideoLogWarning("SELECT SUBTITLE ITEM (VP_PLAY_STRING_SUBTITLE_SELECT_SUBTITLES)");
		nType = VP_SUBTITLE_MODE_SELECT;
		if (pSubtitle->pCloseCb) {
			pSubtitle->pCloseCb((int)nType, TRUE, (void *)pSubtitle->pUserData);
		}
	}
	else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_LANGUAGE)) {
		VideoLogWarning("SELECT SUBTITLE ITEM (VP_PLAY_STRING_SUBTITLE_LANGUAGE)");
		nType = VP_SUBTITLE_MODE_LANGUAGE;
		if (pSubtitle->pCloseCb) {
			pSubtitle->pCloseCb((int)nType, TRUE, (void *)pSubtitle->pUserData);
		}
	}
#ifdef SUBTITLE_K_FEATURE
	else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_ALIGNMENT)) {
		VideoLogWarning("SELECT SUBTITLE ITEM (VP_PLAY_STRING_SUBTITLE_ALIGNMENT)");
		nType = VP_SUBTITLE_MODE_ALIGNMENT;
		if (pSubtitle->pCloseCb) {
			pSubtitle->pCloseCb((int)nType, TRUE, (void *)pSubtitle->pUserData);
		}
	}
#endif
	else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_FONT)) {
		VideoLogWarning("SELECT SUBTITLE ITEM (VP_PLAY_STRING_SUBTITLE_FONT)");
		nType = VP_SUBTITLE_MODE_FONT;
		if (pSubtitle->pCloseCb) {
			pSubtitle->pCloseCb((int)nType, TRUE, (void *)pSubtitle->pUserData);
		}
	}
	else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_SIZE)) {
		VideoLogWarning("SELECT SUBTITLE ITEM (VP_PLAY_STRING_SUBTITLE_SIZE)");
		nType = VP_SUBTITLE_MODE_SIZE;
		if (pSubtitle->pCloseCb) {
			pSubtitle->pCloseCb((int)nType, TRUE, (void *)pSubtitle->pUserData);
		}
	}
//#ifdef SUBTITLE_K_FEATURE
	else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_EDGE)) {
		VideoLogWarning("SELECT SUBTITLE ITEM (VP_PLAY_STRING_SUBTITLE_EDGE)");
		nType = VP_SUBTITLE_MODE_EDGE;
		if (pSubtitle->pCloseCb) {
			pSubtitle->pCloseCb((int)nType, TRUE, (void *)pSubtitle->pUserData);
		}
	}
	else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_TEXT)) {
		VideoLogWarning("SELECT SUBTITLE ITEM (VP_PLAY_STRING_SUBTITLE_TEXT)");
		nType = VP_SUBTITLE_MODE_TEXT;
		if (pSubtitle->pCloseCb) {
			pSubtitle->pCloseCb((int)nType, TRUE, (void *)pSubtitle->pUserData);
		}
	}
	else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_BG_COLOR)) {
		VideoLogWarning("SELECT SUBTITLE ITEM (VP_PLAY_STRING_SUBTITLE_BG_COLOR)");
		nType = VP_SUBTITLE_MODE_BG_COLOR;
		if (pSubtitle->pCloseCb) {
			pSubtitle->pCloseCb((int)nType, TRUE, (void *)pSubtitle->pUserData);
		}
	}
#ifdef SUBTITLE_K_FEATURE
	else if (!strcmp(szTxt, VP_PLAY_STRING_SUBTITLE_CAPTION_WINDOW)) {
		VideoLogWarning("SELECT SUBTITLE ITEM (VP_PLAY_STRING_SUBTITLE_CAPTION_WINDOW)");
		nType = VP_SUBTITLE_MODE_CAPTION_WINDOW;
		if (pSubtitle->pCloseCb) {
			pSubtitle->pCloseCb((int)nType, TRUE, (void *)pSubtitle->pUserData);
		}
	}
#endif

}

static void __vp_subtitle_popup_key_event_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitlePopup *pSubtitle = (SubtitlePopup *)pUserData;
	_vp_subtitle_cancel_button_cb(pSubtitle, NULL, NULL);
}
static void __vp_subtitle_popup_mouse_event_cb(void *pUserData, Evas *pEvas, Evas_Object *pObj, void *pEventInfo)
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
		SubtitlePopup *pSubtitle = (SubtitlePopup *)pUserData;
		_vp_subtitle_cancel_button_cb(pSubtitle, NULL, NULL);
	}
}


/* internal functions */
static void _vp_subtitle_destroy_subtitle_info(SubtitlePopup *pSubtitle)
{
	if (pSubtitle == NULL) {
		VideoLogError("pSubtitle is NULL");
		return;
	}

	if (pSubtitle->pSubtitleInfo)
	{
		VP_FREE(pSubtitle->pSubtitleInfo->szFontName);
		VP_FREE(pSubtitle->pSubtitleInfo->szLanguage);
		VP_FREE(pSubtitle->pSubtitleInfo->szText);
#ifdef SUBTITLE_K_FEATURE
		VP_FREE(pSubtitle->pSubtitleInfo->pTextColorHex);
		VP_FREE(pSubtitle->pSubtitleInfo->pBGColorHex);
		VP_FREE(pSubtitle->pSubtitleInfo->pCaptionWinColorHex);
#endif
	}
}

static void _vp_subtitle_destroy_handle(SubtitlePopup *pSubtitle)
{
	if (pSubtitle == NULL) {
		VideoLogError("pSubtitle is NULL");
		return;
	}
	evas_object_smart_callback_del(pSubtitle->pParent, "rotation,changed", __vp_subtitle_popup_rotate_cb);
	evas_object_smart_callback_del(pSubtitle->pGenList, "realized", __vp_subtitle_genlist_realized);

	if (pSubtitle->st_Subtitle_Itc_1) {
		elm_genlist_item_class_free(pSubtitle->st_Subtitle_Itc_1);
		pSubtitle->st_Subtitle_Itc_1 = NULL;
	}

	if (pSubtitle->st_Subtitle_Itc_2) {
		elm_genlist_item_class_free(pSubtitle->st_Subtitle_Itc_2);
		pSubtitle->st_Subtitle_Itc_2 = NULL;
	}

	if (pSubtitle->st_Subtitle_Itc_1icon) {
		elm_genlist_item_class_free(pSubtitle->st_Subtitle_Itc_1icon);
		pSubtitle->st_Subtitle_Itc_1icon = NULL;
	}

#ifdef SUBTITLE_K_FEATURE
	if (pSubtitle->st_Subtitle_Itc_3) {
		elm_genlist_item_class_free(pSubtitle->st_Subtitle_Itc_3);
		pSubtitle->st_Subtitle_Itc_3 = NULL;
	}
#endif

	if (pSubtitle->st_Subtitle_Itc_groupindex) {
		elm_genlist_item_class_free(pSubtitle->st_Subtitle_Itc_groupindex);
		pSubtitle->st_Subtitle_Itc_groupindex = NULL;
	}

	pSubtitle->pActivationCheck = NULL;
	VP_EVAS_DEL(pSubtitle->pCaptionLayout);
	VP_EVAS_DEL(pSubtitle->pGenList);
	VP_EVAS_DEL(pSubtitle->pContentBox);
	VP_EVAS_DEL(pSubtitle->pBox);

	VP_EVAS_DEL(pSubtitle->pPopup);
	_vp_subtitle_destroy_subtitle_info(pSubtitle);
	VP_FREE(pSubtitle->pSubtitleInfo);

	VP_FREE(pSubtitle);
}

static Evas_Object *_vp_subtitle_create_genlist(Evas_Object *pParent)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	Evas_Object *pObj = NULL;

	pObj = elm_genlist_add(pParent);
	evas_object_size_hint_weight_set(pObj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pObj, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(pObj);
	return pObj;
}

static void  _vp_subtitle_genlist_disabled_set(Evas_Object *pGenList, bool bOn)
{
	if (pGenList == NULL) {
		VideoLogError("pGenList is NULL");
		return;
	}

	Elm_Object_Item *gl_item = elm_genlist_first_item_get(pGenList);
	if (gl_item) {
		gl_item = _vp_subtitle_list_item_next_get(gl_item);
		for (; gl_item; gl_item = _vp_subtitle_list_item_next_get(gl_item)) {
			elm_object_item_disabled_set(gl_item, !bOn);
		}
	}
}

static bool _vp_subtitle_add_genlist_item(Evas_Object *pObj, void *pUserData)
{
	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return FALSE;
	}

	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return FALSE;
	}

	SubtitlePopup *pSubtitle = (SubtitlePopup *)pUserData;

	if (pSubtitle->pSubtitleInfo == NULL) {
		VideoLogError("pSubtitleInfo is NULL");
		return FALSE;
	}

	if (pSubtitle->st_Subtitle_Itc_1) {
		elm_genlist_item_class_free(pSubtitle->st_Subtitle_Itc_1);
		pSubtitle->st_Subtitle_Itc_1 = NULL;
	}

	if (pSubtitle->st_Subtitle_Itc_2) {
		elm_genlist_item_class_free(pSubtitle->st_Subtitle_Itc_2);
		pSubtitle->st_Subtitle_Itc_2 = NULL;
	}

	if (pSubtitle->st_Subtitle_Itc_1icon) {
		elm_genlist_item_class_free(pSubtitle->st_Subtitle_Itc_1icon);
		pSubtitle->st_Subtitle_Itc_1icon = NULL;
	}

#ifdef SUBTITLE_K_FEATURE
	if (pSubtitle->st_Subtitle_Itc_3) {
		elm_genlist_item_class_free(pSubtitle->st_Subtitle_Itc_3);
		pSubtitle->st_Subtitle_Itc_3 = NULL;
	}
#endif

	if (pSubtitle->st_Subtitle_Itc_groupindex) {
		elm_genlist_item_class_free(pSubtitle->st_Subtitle_Itc_groupindex);
		pSubtitle->st_Subtitle_Itc_groupindex = NULL;
	}

	pSubtitle->st_Subtitle_Itc_1 = elm_genlist_item_class_new();
	
	if (pSubtitle->st_Subtitle_Itc_1 != NULL) {
		pSubtitle->st_Subtitle_Itc_1->version = ELM_GENLIST_ITEM_CLASS_VERSION;
		pSubtitle->st_Subtitle_Itc_1->item_style = "1line";
		pSubtitle->st_Subtitle_Itc_1->func.text_get = (void *)__vp_subtitle_genlist_text_get_cb;
		pSubtitle->st_Subtitle_Itc_1->func.content_get = (void *)__vp_subtitle_genlist_content_get_cb;
		pSubtitle->st_Subtitle_Itc_1->func.state_get = NULL;
		pSubtitle->st_Subtitle_Itc_1->func.del = NULL;
	}

	pSubtitle->st_Subtitle_Itc_2 = elm_genlist_item_class_new();
	
	if (pSubtitle->st_Subtitle_Itc_2 != NULL) {
		pSubtitle->st_Subtitle_Itc_2->version = ELM_GENLIST_ITEM_CLASS_VERSION;
		pSubtitle->st_Subtitle_Itc_2->item_style = "2line.top";
		pSubtitle->st_Subtitle_Itc_2->func.text_get = (void *)__vp_subtitle_genlist_text_get_cb;
		pSubtitle->st_Subtitle_Itc_2->func.content_get = NULL;
		pSubtitle->st_Subtitle_Itc_2->func.state_get = NULL;
		pSubtitle->st_Subtitle_Itc_2->func.del = NULL;
	}

	pSubtitle->st_Subtitle_Itc_1icon = elm_genlist_item_class_new();

	if (pSubtitle->st_Subtitle_Itc_1icon != NULL) {
		pSubtitle->st_Subtitle_Itc_1icon->version = ELM_GENLIST_ITEM_CLASS_VERSION;
		pSubtitle->st_Subtitle_Itc_1icon->item_style = "1icon";
		pSubtitle->st_Subtitle_Itc_1icon->func.text_get = NULL;
		pSubtitle->st_Subtitle_Itc_1icon->func.content_get = (void *)__vp_subtitle_genlist_content_get_cb;
		pSubtitle->st_Subtitle_Itc_1icon->func.state_get = NULL;
		pSubtitle->st_Subtitle_Itc_1icon->func.del = NULL;
	}
	
	if (pSubtitle->st_Subtitle_Itc_groupindex != NULL) {
		pSubtitle->st_Subtitle_Itc_groupindex = elm_genlist_item_class_new();
		pSubtitle->st_Subtitle_Itc_groupindex->version = ELM_GENLIST_ITEM_CLASS_VERSION;
		pSubtitle->st_Subtitle_Itc_groupindex->item_style = "groupindex";
		pSubtitle->st_Subtitle_Itc_groupindex->func.text_get = (void *)__vp_subtitle_genlist_text_get_cb;
		pSubtitle->st_Subtitle_Itc_groupindex->func.content_get = NULL;
		pSubtitle->st_Subtitle_Itc_groupindex->func.state_get = NULL;
		pSubtitle->st_Subtitle_Itc_groupindex->func.del = NULL;
	}

	Elm_Object_Item *pItem = NULL;
	VideoLogInfo("subtitle text pre is %s", pSubtitle->pSubtitleInfo->szText);
	pItem = elm_genlist_item_append(pObj, pSubtitle->st_Subtitle_Itc_1, (void *)VP_PLAY_STRING_ACTIVATION, NULL, ELM_GENLIST_ITEM_NONE, __vp_subtitle_genlist_item_selected_cb, pUserData);
	elm_genlist_item_select_mode_set(pItem , ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	//elm_genlist_item_append(pObj, pSubtitle->st_Subtitle_Itc_2, (void *)VP_PLAY_STRING_SUBTITLE_SELECT_SUBTITLES, NULL, ELM_GENLIST_ITEM_NONE, __vp_subtitle_genlist_item_selected_cb, pUserData);

	pItem = elm_genlist_item_append(pObj, pSubtitle->st_Subtitle_Itc_groupindex, (void *)VP_PLAY_STRING_SETTINGS, NULL, ELM_GENLIST_ITEM_NONE, NULL, pUserData);
	elm_genlist_item_select_mode_set(pItem , ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	pItem = elm_genlist_item_append(pObj, pSubtitle->st_Subtitle_Itc_1icon, (void *)VP_PLAY_STRING_PREVIEW_SUBTITLE_CC, NULL, ELM_GENLIST_ITEM_NONE, NULL, pUserData);
	elm_genlist_item_select_mode_set(pItem , ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	elm_genlist_item_append(pObj, pSubtitle->st_Subtitle_Itc_2, (void *)VP_PLAY_STRING_SUBTITLE_ALIGNMENT, NULL, ELM_GENLIST_ITEM_NONE, __vp_subtitle_genlist_item_selected_cb, pUserData);
	elm_genlist_item_append(pObj, pSubtitle->st_Subtitle_Itc_2, (void *)VP_PLAY_STRING_SUBTITLE_SIZE, NULL, ELM_GENLIST_ITEM_NONE, __vp_subtitle_genlist_item_selected_cb, pUserData);

	bool bOn = FALSE;
	vp_play_preference_get_subtitle_show_key(&bOn);
	_vp_subtitle_genlist_disabled_set(pSubtitle->pGenList, bOn);

	return TRUE;
}

static void _vp_subtitle_done_button_cb(void *data, Evas_Object *obj, void *Event_info)
{
	SubtitlePopup *pSubtitle = (SubtitlePopup *)data;

	if (pSubtitle->pCloseCb) {
		pSubtitle->pCloseCb(VP_SUBTITLE_MODE_CLOSE, FALSE, (void *)pSubtitle->pUserData);
	}
}

static void _vp_subtitle_cancel_button_cb(void *data, Evas_Object *obj, void *Event_info)
{
	SubtitlePopup *pSubtitle = (SubtitlePopup *)data;
	pSubtitle->pSubtitleInfo->eAlignment = pSubtitle->pSubtitleInfo->original_eAlignment;
	pSubtitle->pSubtitleInfo->nFontSize = pSubtitle->pSubtitleInfo->original_nFontSize;
	vp_play_preference_set_subtitle_alignment_key(pSubtitle->pSubtitleInfo->original_eAlignment);
	vp_play_preference_set_subtitle_size_key(pSubtitle->pSubtitleInfo->original_nFontSize);
	bool bOn = FALSE;
	vp_play_preference_get_subtitle_show_key(&bOn);
	if (bOn != pSubtitle->pSubtitleInfo->original_activation) {
		bOn = pSubtitle->pSubtitleInfo->original_activation;
		vp_play_preference_set_subtitle_show_key(bOn);
		_vp_subtitle_set_activation(pSubtitle, bOn);
	}
	if (pSubtitle->pCloseCb) {
		pSubtitle->pCloseCb(VP_SUBTITLE_MODE_CLOSE, FALSE, (void *)pSubtitle->pUserData);
	}
}

static void _vp_subtitle_create_title(SubtitlePopup *pSubtitle)
{
	if (pSubtitle == NULL) {
		VideoLogError("pSubtitle is NULL");
		return;
	}

	if (pSubtitle->pBox == NULL) {
		VideoLogError("pBox is NULL");
		return;
	}

	pSubtitle->pPopupTitle = elm_layout_add(pSubtitle->pBox);
	//evas_object_size_hint_weight_set(pSubtitle->pPopupTitle, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pSubtitle->pPopupTitle, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_layout_file_set(pSubtitle->pPopupTitle, VP_PLAY_POPUP_EDJ_PATH, VP_PLAY_EDJ_GROUP_TITLE_POPUP);

	/*set title*/
	elm_object_part_text_set(pSubtitle->pPopupTitle, "elm.text.title", VP_PLAY_STRING_POPUP_SUBTITLE);

	/* Title Cancel Button */
	Evas_Object *btn = NULL;
	btn = elm_button_add(pSubtitle->pPopupTitle);
	elm_object_style_set(btn, "naviframe/title_left");
	evas_object_smart_callback_add(btn, "clicked", (Evas_Smart_Cb)_vp_subtitle_cancel_button_cb, pSubtitle);
	elm_object_part_content_set(pSubtitle->pPopupTitle, "elm.swallow.title.left.icon", btn);
	elm_object_text_set(btn, VP_PLAY_STRING_SUBTITLE_CANCEL);

	/* Title Done Button */
	btn = elm_button_add(pSubtitle->pPopupTitle);
	elm_object_style_set(btn, "naviframe/title_right");
	evas_object_smart_callback_add(btn, "clicked", (Evas_Smart_Cb)_vp_subtitle_done_button_cb, pSubtitle);
	elm_object_part_content_set(pSubtitle->pPopupTitle, "elm.swallow.title.right.icon", btn);
	elm_object_text_set(btn,VP_PLAY_STRING_SUBTITLE_DONE);

	elm_box_pack_end(pSubtitle->pBox, pSubtitle->pPopupTitle);
	evas_object_show(pSubtitle->pPopupTitle);
}

static Evas_Object * _vp_subtitle_create_label(Evas_Object *parent)
{
	Evas_Object *pLabel = elm_label_add(parent);
	evas_object_size_hint_weight_set(pLabel, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pLabel, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_label_ellipsis_set(pLabel, EINA_TRUE);
	elm_object_style_set(pLabel, "popup/default");
	evas_object_size_hint_min_set(pLabel, EVAS_HINT_EXPAND, VP_POPUP_LIST_CLOSED_CAPTION_HEIGHT * VP_SCALE);

	evas_object_show(pLabel);

	return pLabel;
}

static Evas_Object * _vp_subtitle_create_caption_layout(SubtitlePopup *pSubtitle, Evas_Object *pParent)
{
	if (pSubtitle == NULL) {
		VideoLogError("pSubtitle is NULL");
		return NULL;
	}

	if (pSubtitle->pBox == NULL) {
		VideoLogError("pBox is NULL");
		return NULL;
	}

	if (pSubtitle->pSubtitleInfo == NULL) {
		VideoLogError("pSubtitleInfo is NULL");
		return NULL;
	}

	pSubtitle->pCaptionLayout = elm_layout_add(pParent);
	evas_object_size_hint_weight_set(pSubtitle->pCaptionLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pSubtitle->pCaptionLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_layout_file_set(pSubtitle->pCaptionLayout, VP_PLAY_POPUP_EDJ_PATH, VP_PLAY_EDJ_GROUP_CAPTION_POPUP);

	char *szText = NULL;
	Evas_Object *pLab = NULL;
	pLab = _vp_subtitle_create_label(pSubtitle->pCaptionLayout);

	bool bOn = FALSE;
	vp_play_preference_get_subtitle_show_key(&bOn);
	szText = _vp_play_subtitle_text_get(pSubtitle->pSubtitleInfo, bOn);
	VideoSecureLogInfo("preview title =%S", szText);
	elm_object_text_set(pLab, szText);
	VP_FREE(szText);
	elm_object_part_content_set(pSubtitle->pCaptionLayout, "elm.swallow.content", pLab);
	evas_object_show(pLab);
	evas_object_show(pSubtitle->pCaptionLayout);

	return pSubtitle->pCaptionLayout;
}

static bool _vp_subtitle_update_caption_layout(SubtitlePopup *pSubtitle, bool bActivation)
{
	if (pSubtitle == NULL) {
		VideoLogError("pSubtitle is NULL");
		return FALSE;
	}

	if (pSubtitle->pBox == NULL) {
		VideoLogError("pBox is NULL");
		return FALSE;
	}

	if (pSubtitle->pSubtitleInfo == NULL) {
		VideoLogError("pSubtitleInfo is NULL");
		return FALSE;
	}

	if (pSubtitle->pCaptionLayout == NULL) {
		VideoLogError("pCaptionLayout is NULL");
		return FALSE;
	}

	char *szText = NULL;
	Evas_Object *pLab = NULL;
	pLab = elm_object_part_content_get(pSubtitle->pCaptionLayout, "elm.swallow.content");
	szText = _vp_play_subtitle_text_get(pSubtitle->pSubtitleInfo, bActivation);
	VideoSecureLogInfo("preview title =%s", szText);
	elm_object_text_set(pLab, szText);
	VP_FREE(szText);
	return TRUE;
}

static Evas_Object *_vp_subtitle_popup_create(Evas_Object *pParent, Eext_Event_Cb pCancelKeyCb, Evas_Object_Event_Cb pCancelMouseCb, void *pUserData)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}

	Evas_Object *pObj = NULL;

	pObj = vp_popup_create(pParent, 0 ,
						NULL,
						NULL, 0.0, NULL,
						pCancelKeyCb,
						pCancelMouseCb,
						(void *)pUserData);

	return pObj;
}

static void _vp_subtitle_get_subtitle_info(SubtitlePopup *pSubtitle, SubtitleInfo *pSubtitleInfo)
{
	if (pSubtitle == NULL) {
		VideoLogError("pSubtitle is NULL");
		return;
	}

	if (pSubtitleInfo == NULL) {
		VideoLogError("pSubtitleInfo is NULL");
		return;
	}

	if (pSubtitle->pSubtitleInfo == NULL) {
		VideoLogError("pSubtitleInfo is NULL");
		return;
	}

	_vp_subtitle_destroy_subtitle_info(pSubtitle);

	pSubtitle->pSubtitleInfo->bClosedCaption = pSubtitleInfo->bClosedCaption;
	pSubtitle->pSubtitleInfo->fSync = pSubtitleInfo->fSync;
	VP_STRDUP(pSubtitle->pSubtitleInfo->szLanguage, pSubtitleInfo->szLanguage);
	VP_STRDUP(pSubtitle->pSubtitleInfo->szFontName, pSubtitleInfo->szFontName);
	VP_STRDUP(pSubtitle->pSubtitleInfo->szURL, pSubtitleInfo->szURL);
	VP_STRDUP(pSubtitle->pSubtitleInfo->szText, VP_PLAY_STRING_PREVIEW_SUBTITLE_CC);

	pSubtitle->pSubtitleInfo->nFontSize = pSubtitleInfo->nFontSize;
	pSubtitle->pSubtitleInfo->nEdge = pSubtitleInfo->nEdge;

#ifndef SUBTITLE_K_FEATURE
	pSubtitle->pSubtitleInfo->nTextColor = pSubtitleInfo->nTextColor;
	pSubtitle->pSubtitleInfo->nBGColor = pSubtitleInfo->nBGColor;
#else
	VP_STRDUP(pSubtitle->pSubtitleInfo->pTextColorHex, pSubtitleInfo->pTextColorHex);
	VP_STRDUP(pSubtitle->pSubtitleInfo->pBGColorHex, pSubtitleInfo->pBGColorHex);
	VP_STRDUP(pSubtitle->pSubtitleInfo->pCaptionWinColorHex, pSubtitleInfo->pCaptionWinColorHex);
	pSubtitle->pSubtitleInfo->eAlignment = pSubtitleInfo->eAlignment;
#endif
	pSubtitle->pSubtitleInfo->fZoom = pSubtitleInfo->fZoom;
	pSubtitle->pSubtitleInfo->bSupportSelect = pSubtitleInfo->bSupportSelect;
}

Elm_Object_Item*
_vp_subtitle_list_item_next_get(Elm_Object_Item *gl_item)
{
	if (!gl_item) {
		VideoLogError("gl_item is null!");
		return NULL;
	}

	Elm_Object_Item *next = gl_item;
	do {
		next = elm_genlist_item_next_get(next);
	} while (next && elm_genlist_item_expanded_depth_get(next) > 0);

	return next;
}

static void _vp_subtitle_set_activation(SubtitlePopup *pSubtitle, bool bOn)
{
	if (pSubtitle == NULL) {
		VideoLogError("pSubtitle is NULL");
		return;
	}
	VideoLogInfo("");

	_vp_subtitle_update_caption_layout(pSubtitle, bOn);
	//_vp_subtitle_update_preview_text_item(pSubtitle);
	_vp_subtitle_genlist_disabled_set(pSubtitle->pGenList, bOn);
}

/* external functions */
subtitle_popup_handle vp_subtitle_create(Evas_Object *pParent, SubtitleInfo *pSubtitleInfo, PopupCloseCbFunc pCloseCb)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	if (pSubtitleInfo == NULL) {
		VideoLogError("pSubtitleInfo is NULL");
		return NULL;
	}

	SubtitlePopup *pSubtitle = NULL;

	pSubtitle = calloc(1, sizeof(SubtitlePopup));

	if (pSubtitle == NULL) {
		VideoLogError("pSubtitle alloc fail");
		return NULL;
	}

	pSubtitle->pParent = pParent;
	pSubtitle->pCloseCb = pCloseCb;

	pSubtitle->pSubtitleInfo = calloc(1, sizeof(SubtitleInfo));
	if (pSubtitle->pSubtitleInfo == NULL) {
		VideoLogError("pSubtitleInfo alloc fail");
		_vp_subtitle_destroy_handle(pSubtitle);
		return NULL;
	}
	_vp_subtitle_get_subtitle_info(pSubtitle, pSubtitleInfo);
	pSubtitle->pSubtitleInfo->original_eAlignment = pSubtitleInfo->eAlignment;
	pSubtitle->pSubtitleInfo->original_nFontSize = pSubtitleInfo->nFontSize;
	bool bOn = FALSE;
	vp_play_preference_get_subtitle_show_key(&bOn);
	pSubtitle->pSubtitleInfo->original_activation = bOn;

	pSubtitle->pPopup = _vp_subtitle_popup_create(pParent, __vp_subtitle_popup_key_event_cb, __vp_subtitle_popup_mouse_event_cb, (void *)pSubtitle);
	if (pSubtitle->pPopup == NULL) {
		VideoLogError("_vp_subtitle_create_popup fail");
		_vp_subtitle_destroy_handle(pSubtitle);
		return NULL;
	}

	pSubtitle->pGenList = _vp_subtitle_create_genlist(pSubtitle->pPopup);
	if (pSubtitle->pGenList == NULL) {
		VideoLogError("_vp_subtitle_create_genlist fail");
		_vp_subtitle_destroy_handle(pSubtitle);
		return NULL;
	}

	evas_object_smart_callback_add(pSubtitle->pParent, "rotation,changed", __vp_subtitle_popup_rotate_cb, pSubtitle);
	evas_object_smart_callback_add(pSubtitle->pGenList, "realized", __vp_subtitle_genlist_realized, NULL);
	evas_object_data_set(pSubtitle->pGenList , VP_SUBTITLE_GENLIST_DATA_KEY, (void *)pSubtitle);
	elm_genlist_mode_set(pSubtitle->pGenList, ELM_LIST_COMPRESS);

	if (!_vp_subtitle_add_genlist_item(pSubtitle->pGenList, (void *)pSubtitle)) {
		VideoLogError("_vp_subtitle_add_genlist_item fail");
		_vp_subtitle_destroy_handle(pSubtitle);
		return NULL;
	}

	pSubtitle->pBox = elm_box_add(pSubtitle->pPopup);
	evas_object_size_hint_weight_set(pSubtitle->pBox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pSubtitle->pBox, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_homogeneous_set(pSubtitle->pBox, FALSE);

	pSubtitle->pContentBox = elm_box_add(pSubtitle->pBox);
	evas_object_size_hint_weight_set(pSubtitle->pContentBox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pSubtitle->pContentBox, EVAS_HINT_FILL, EVAS_HINT_FILL);

	__vp_subtitle_popup_size_set(pSubtitle);
	elm_box_pack_end(pSubtitle->pContentBox, pSubtitle->pGenList);

	_vp_subtitle_create_title(pSubtitle);
	//_vp_subtitle_create_caption_layout(pSubtitle);

	evas_object_show(pSubtitle->pContentBox);

	elm_box_pack_end(pSubtitle->pBox, pSubtitle->pContentBox);

	evas_object_show(pSubtitle->pGenList);
	elm_object_content_set(pSubtitle->pPopup, pSubtitle->pBox);

	return pSubtitle;
}

void vp_subtitle_destroy(subtitle_popup_handle pSubtitleHandle)
{
	if (pSubtitleHandle == NULL) {
		VideoLogError("pSubtitleHandle is NULL");
		return;
	}

	SubtitlePopup *pSubtitle = (SubtitlePopup *)pSubtitleHandle;

	_vp_subtitle_destroy_handle(pSubtitle);

}

bool vp_subtitle_realize(subtitle_popup_handle pSubtitleHandle)
{
	if (pSubtitleHandle == NULL) {
		VideoLogError("pSubtitleHandle is NULL");
		return FALSE;
	}

	SubtitlePopup *pSubtitle = (SubtitlePopup *)pSubtitleHandle;

	evas_object_show(pSubtitle->pPopup);

	return TRUE;
}

bool vp_subtitle_unrealize(subtitle_popup_handle pSubtitleHandle)
{
	if (pSubtitleHandle == NULL) {
		VideoLogError("pSubtitleHandle is NULL");
		return FALSE;
	}

	SubtitlePopup *pSubtitle = (SubtitlePopup *)pSubtitleHandle;

	evas_object_hide(pSubtitle->pPopup);

	return TRUE;
}

bool vp_subtitle_update(subtitle_popup_handle pSubtitleHandle, SubtitleInfo *pSubtitleInfo)
{
	if (pSubtitleHandle == NULL) {
		VideoLogError("pSubtitleHandle is NULL");
		return FALSE;
	}

	if (pSubtitleInfo== NULL) {
		VideoLogError("pSubtitleInfo is NULL");
		return FALSE;
	}

	SubtitlePopup *pSubtitle = (SubtitlePopup *)pSubtitleHandle;
	_vp_subtitle_get_subtitle_info(pSubtitle, pSubtitleInfo);
	//_vp_subtitle_update_preview_text_item(pSubtitle);

	if (!_vp_subtitle_update_caption_layout(pSubtitle, TRUE)) {
		VideoLogError("_vp_subtitle_update_caption_layout failed");
		return FALSE;
	}

	if (pSubtitle->pSelectItem) {
		VideoLogInfo("pSubtitle->pSelectItem=%p", pSubtitle->pSelectItem);
		elm_genlist_item_update(pSubtitle->pSelectItem);
	}

	return TRUE;
}

bool vp_subtitle_set_user_data(subtitle_popup_handle pSubtitleHandle, void *pUserData)
{
	if (pSubtitleHandle == NULL) {
		VideoLogError("pSubtitleHandle is NULL");
		return FALSE;
	}

	SubtitlePopup *pSubtitle = (SubtitlePopup *)pSubtitleHandle;

	pSubtitle->pUserData = pUserData;

	return TRUE;
}

char *_vp_subtitle_get_opacity(char *colorHex)
{
	if (colorHex == NULL) {
		VideoLogError("colorHex is NULL");
		return NULL;
	}

	char *szOpacity = g_strndup(colorHex+7, 2);

	float fOpacity = strtol(szOpacity, NULL, 16);

	VideoLogInfo("szOpacity=%s, fOpacity=%f", szOpacity, fOpacity);

	VP_FREE(szOpacity);

	return g_strdup_printf("%d%%", (int)((fOpacity*100)/255.0 + 0.5));
}

