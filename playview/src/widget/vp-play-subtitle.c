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
#include <app.h>

#include "vp-play-type-define.h"
#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-macro-define.h"

#include "vp-play-preference.h"
#include "vp-play-subtitle.h"
#include "vp-play-util.h"

/* check temp */
#include "vp-play-log.h"

#ifdef SUBTITLE_K_FEATURE
#define VP_SUBTITLE_STRING_TXT_FORMAT "<font_size= %d><style=%s><font='%s' font_weight='%s' color=%s backing_color=%s backing=%s align=%s>%s</font></style></font_size>"
#else
#define VP_SUBTITLE_STRING_TXT_FORMAT "<font_size= %d><style=%s><font='%s' font_weight=%s color=%s backing_color=%s backing=%s>%s</font></style></font_size>"
#endif

#define VP_SUBTITLE_SET_CAPTION_WINDOW_COLOR_ID		3
#define VP_SUBTITLE_PROTRAIT_W               		720
#define VP_SUBTITLE_PROTRAIT_H               		990
#define VP_SUBTITLE_LANDSCAPE_W               		1280
#define VP_SUBTITLE_LANDSCAPE_H               		530

#define VP_NORMAL_SUTITLE_TEXT_STYLE \
	"DEFAULT='font=Tizen:style=Roman align=center valign=bottom wrap=word'"\
	"br='\n'" \
	"ps='ps'" \
	"hilight='+ font=Tizen:style=Roman'" \
	"b='+ font=Tizen:style=Roman'" \
	"tab='\t'"

void _vp_play_subtitle_destroy_handle(Subtitle *pSubtitle);
/* callback functions */

/* internal functions */
void _vp_play_subtitle_destroy_handle(Subtitle *pSubtitle)
{
	if (pSubtitle == NULL) {
		VideoLogError("pSubtitle is NULL");
		return;
	}

	VP_EVAS_DEL(pSubtitle->pLayout);
	VP_FREE(pSubtitle->szFontName);
	VP_FREE(pSubtitle->szText);
	VP_EVAS_DEL(pSubtitle->pTextBlock);
#ifdef SUBTITLE_K_FEATURE
	VP_FREE(pSubtitle->pFontColorHex);
	VP_FREE(pSubtitle->pBGColorHex);
	VP_FREE(pSubtitle->pCaptionWinColorHex);
#endif
	VP_FREE(pSubtitle);
}

static Evas_Object *_vp_play_subtitle_create_layout(Evas_Object *pParent,
        vp_subtitle_type_t
        nType)
{
	if (!pParent) {
		VideoLogError("Parent is NULL");
		return NULL;
	}

	Evas_Object *pObj = NULL;
	Eina_Bool bRet = EINA_FALSE;

	pObj = elm_layout_add(pParent);
	if (pObj == NULL) {
		VideoLogError("elm_layout_add object is NULL");
		return NULL;
	}
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s/%s/%s", path , "edje", VP_PLAY_SUBTITLE_EDJ_PATH);

	if (nType == VP_SUBTITLE_TYPE_NORMAL) {
		bRet =
		    elm_layout_file_set(pObj, edj_path,
		                        VP_PLAY_EDJ_GROUP_SUBTITLE_MULTI);
	} else if (nType == VP_SUBTITLE_TYPE_MULTI) {
		bRet =
		    elm_layout_file_set(pObj, edj_path,
		                        VP_PLAY_EDJ_GROUP_SUBTITLE_MULTI);
	}

	if (bRet != EINA_TRUE) {
		VideoLogError("elm_layout_file_set fail");
		return NULL;
	}

	return pObj;
}

static void _vp_play_subtitle_layout_del_cb(void *pUserData, Evas *e,
        Evas_Object *pObject,
        void *pEventInfo)
{
	if (!pUserData) {
		VideoLogError("pSubtitle is NULL");
		return;
	}
	Subtitle *pSubtitle = (Subtitle *) pUserData;
	pSubtitle->pLayout = NULL;

}

static bool _vp_play_subtitle_init_layout(Subtitle *pSubtitle,
        vp_subtitle_type_t nType)
{
	if (pSubtitle == NULL) {
		VideoLogError("pSubtitle is NULL");
		return FALSE;
	}

	Evas_Object *pParent = pSubtitle->pParent;

	pSubtitle->pLayout = _vp_play_subtitle_create_layout(pParent, nType);
	if (pSubtitle->pLayout == NULL) {
		VideoLogError("_vp_play_subtitle_create_layout is fail");
		return FALSE;
	}
	pSubtitle->pTextBlock = NULL;

	evas_object_event_callback_add(pSubtitle->pLayout, EVAS_CALLBACK_DEL,
	                               _vp_play_subtitle_layout_del_cb,
	                               (void *) pSubtitle);

	return TRUE;
}

void vp_play_subtitle_text_update(Subtitle *pSubtitle, const char *szTxt)
{
	if (!pSubtitle || !pSubtitle->pLayout) {
		VideoLogError("pSubtitle is NULL");
		return;
	}

	if (!szTxt || strlen(szTxt) == 0) {
		VideoLogWarning
		("Text is null or empty, need hide caption window.");
		vp_play_subtitle_set_caption_win_color(pSubtitle, FALSE);
	} else if (!pSubtitle->bShowCaptionWin) {
		VideoLogWarning("Caption window is hidden, need show it again!");
		vp_play_subtitle_set_caption_win_color(pSubtitle, TRUE);
	}

	char *szMarkup = elm_entry_markup_to_utf8(szTxt);
	char *szOutput = elm_entry_utf8_to_markup(szMarkup);
	char *szTxtFormat = NULL;
	char *szColor = NULL;
	char *szBGColor = NULL;
	char *szBGColorOn = NULL;
#ifdef SUBTITLE_K_FEATURE
	char *szAlignment = NULL;
#endif
	char *szFontEdge = NULL;
	char *szFontWeight = NULL;

	int nSize = 0;
	vp_play_preference_get_subtitle_size_key(&nSize);

	if (nSize == VP_SUBTITLE_SMALL) {
		nSize = VP_SUBTITLE_SIZE_SMALL_VALUE * pSubtitle->fZoom;
	} else if (nSize == VP_SUBTITLE_LARGE) {
		nSize = VP_SUBTITLE_SIZE_LARGE_VALUE * pSubtitle->fZoom;
	} else {
		nSize = VP_SUBTITLE_SIZE_MEDIUM_VALUE * pSubtitle->fZoom;
	}

	if (nSize < VP_SUBTITLE_SIZE_SMALL_VALUE) {
		nSize = VP_SUBTITLE_SIZE_SMALL_VALUE;
	}

	if (nSize > VP_SUBTITLE_SIZE_LARGE_VALUE) {
		nSize = VP_SUBTITLE_SIZE_LARGE_VALUE;
	}
#ifndef SUBTITLE_K_FEATURE
	if (pSubtitle->nFontColor == VP_SUBTITLE_COLOR_BLACK) {
		szColor = g_strdup_printf("#000000");
	} else if (pSubtitle->nFontColor == VP_SUBTITLE_COLOR_BLUE) {
		szColor = g_strdup_printf("#0000FF");
	} else if (pSubtitle->nFontColor == VP_SUBTITLE_COLOR_GREEN) {
		szColor = g_strdup_printf("#00FF00");
	} else if (pSubtitle->nFontColor == VP_SUBTITLE_COLOR_WHITE) {
		szColor = g_strdup_printf("#FFFFFF");
	}

	if (pSubtitle->nBGColor == VP_SUBTITLE_COLOR_NONE) {
		szBGColorOn = g_strdup_printf("off");
		szBGColor = g_strdup_printf("#000000");
	} else if (pSubtitle->nBGColor == VP_SUBTITLE_COLOR_BLACK) {
		szBGColorOn = g_strdup_printf("on");
		szBGColor = g_strdup_printf("#000000");
	} else if (pSubtitle->nBGColor == VP_SUBTITLE_COLOR_WHITE) {
		szBGColorOn = g_strdup_printf("on");
		szBGColor = g_strdup_printf("#FFFFFF");
	}
#else

	VideoLogInfo
	("pSubtitle->pFontColorHex %s, pSubtitle->pBGColorHex is %s",
	 pSubtitle->pFontColorHex, pSubtitle->pBGColorHex);
	if (pSubtitle->pFontColorHex) {
		szColor = strdup(pSubtitle->pFontColorHex);
	} else {
		szColor = g_strdup_printf("#ffffffff");
	}

	if (pSubtitle->pBGColorHex) {
		szBGColorOn = g_strdup_printf("on");
		szBGColor = strdup(pSubtitle->pBGColorHex);
	} else {
		szBGColorOn = g_strdup_printf("off");
		szBGColor = g_strdup_printf("#00000000");
	}

	int nAlignment = 0;
	vp_play_preference_get_subtitle_alignment_key(&nAlignment);
	if (nAlignment == VP_SUBTITLE_ALIGNMENT_LEFT) {
		szAlignment = g_strdup_printf("left");
	} else if (nAlignment == VP_SUBTITLE_ALIGNMENT_CENTER) {
		szAlignment = g_strdup_printf("center");
	} else if (nAlignment == VP_SUBTITLE_ALIGNMENT_RIGHT) {
		szAlignment = g_strdup_printf("right");
	}
#endif

#ifdef SUBTITLE_K_FEATURE
	szTxtFormat =
	    g_strdup_printf(VP_SUBTITLE_STRING_TXT_FORMAT, nSize, szFontEdge,
	                    pSubtitle->szFontName, szFontWeight, szColor,
	                    szBGColor, szBGColorOn, szAlignment, szOutput);
#else
	szTxtFormat =
	    g_strdup_printf(VP_SUBTITLE_STRING_TXT_FORMAT, nSize, szFontEdge,
	                    pSubtitle->szFontName, szFontWeight, szColor,
	                    szBGColor, szBGColorOn, szOutput);
#endif

	if (pSubtitle->pTextBlock) {
		evas_object_textblock_text_markup_set(pSubtitle->pTextBlock,
		                                      szTxtFormat);
	} else {
		elm_object_part_text_set(pSubtitle->pLayout,
		                         VP_PLAY_PART_SUBTITLE_TEXT, szTxtFormat);
	}

	VP_FREE(szFontEdge);
	VP_FREE(szFontWeight);

	VP_FREE(szBGColor);
	VP_FREE(szBGColorOn);
	VP_FREE(szColor);
	VP_FREE(szTxtFormat);
	VP_FREE(szOutput);
	VP_FREE(szMarkup);
#ifdef SUBTITLE_K_FEATURE
	VP_FREE(szAlignment);
#endif
}


/* external functions */
subtitle_handle vp_play_subtitle_create(Evas_Object *pParent,
                                        vp_subtitle_type_t nType)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	Subtitle *pSubtitle = NULL;

	pSubtitle = calloc(1, sizeof(Subtitle));
	if (pSubtitle == NULL) {
		VideoLogError("pSubtitle alloc fail");
		return NULL;
	}

	pSubtitle->pParent = pParent;
	pSubtitle->fZoom = 1.0;
	pSubtitle->szFontName = strdup("Tizen");

	if (!_vp_play_subtitle_init_layout(pSubtitle, nType)) {
		VideoLogError("_vp_play_subtitle_init_layout is fail");
		_vp_play_subtitle_destroy_handle(pSubtitle);
		return NULL;
	}

	return pSubtitle;
}

void vp_play_subtitle_destroy(subtitle_handle pWidgetHandle)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	vp_play_subtitle_unrealize((subtitle_handle) pSubtitle);

	_vp_play_subtitle_destroy_handle(pSubtitle);
}

bool vp_play_subtitle_realize(subtitle_handle pWidgetHandle)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	pSubtitle->bIsRealize = TRUE;

	if (pSubtitle->pLayout) {
		evas_object_show(pSubtitle->pLayout);
	}

	vp_play_subtitle_text_update(pSubtitle, pSubtitle->szText);

	return TRUE;
}

bool vp_play_subtitle_unrealize(subtitle_handle pWidgetHandle)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	pSubtitle->bIsRealize = FALSE;

	if (pSubtitle->pLayout) {
		evas_object_hide(pSubtitle->pLayout);
	}

	vp_play_subtitle_text_update(pSubtitle, NULL);

	return TRUE;
}

bool vp_play_subtitle_is_realize(subtitle_handle pWidgetHandle,
                                 bool *bIsRealize)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	*bIsRealize = pSubtitle->bIsRealize;

	return TRUE;

}

Evas_Object *vp_play_subtitle_get_object(subtitle_handle pWidgetHandle)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return NULL;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	return pSubtitle->pLayout;
}

bool vp_play_subtitle_set_font(subtitle_handle pWidgetHandle,
                               char *szFontName)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	if (szFontName == NULL) {
		VideoLogError("szFontName is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	VP_FREE(pSubtitle->szFontName);
	VP_STRDUP(pSubtitle->szFontName, szFontName);

	if (pSubtitle->bIsRealize) {
		vp_play_subtitle_text_update(pSubtitle, pSubtitle->szText);
	}

	return TRUE;

}

bool vp_play_subtitle_get_font(subtitle_handle pWidgetHandle,
                               char **szFontName)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	if (pSubtitle->szFontName == NULL) {
		VideoLogError("szFontName is NULL");
		return FALSE;
	}

	VP_STRDUP(*szFontName, pSubtitle->szFontName);

	return TRUE;
}

bool vp_play_subtitle_set_edge(subtitle_handle pWidgetHandle, int nEdge)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	pSubtitle->nEdge = nEdge;

	if (pSubtitle->bIsRealize) {
		vp_play_subtitle_text_update(pSubtitle, pSubtitle->szText);
	}

	return TRUE;
}

bool vp_play_subtitle_get_edge(subtitle_handle pWidgetHandle, int *nEdge)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	*nEdge = pSubtitle->nEdge;

	return TRUE;
}

bool vp_play_subtitle_set_size(subtitle_handle pWidgetHandle,
                               video_subtitle_size_t nSize)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	pSubtitle->nSize = nSize;

	if (pSubtitle->bIsRealize) {
		vp_play_subtitle_text_update(pSubtitle, pSubtitle->szText);
		vp_play_subtitle_set_caption_win_color(pSubtitle, TRUE);
	}

	return TRUE;
}

bool vp_play_subtitle_get_size(subtitle_handle pWidgetHandle,
                               video_subtitle_size_t *nSize)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	*nSize = pSubtitle->nSize;

	return TRUE;
}

bool vp_play_subtitle_set_size_zoom(subtitle_handle pWidgetHandle,
                                    float fZoom)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	if (fZoom > 1.0) {
		fZoom = 1.0;
	}
	pSubtitle->fZoom = fZoom;

	if (pSubtitle->bIsRealize) {
		vp_play_subtitle_text_update(pSubtitle, pSubtitle->szText);
	}

	return TRUE;
}

bool vp_play_subtitle_get_size_zoom(subtitle_handle pWidgetHandle,
                                    float *fZoom)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	*fZoom = pSubtitle->fZoom;

	return TRUE;
}

#ifndef SUBTITLE_K_FEATURE
bool vp_play_subtitle_set_color(subtitle_handle pWidgetHandle,
                                video_subtitle_color_t nColor)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	pSubtitle->nFontColor = nColor;

	if (pSubtitle->bIsRealize) {
		vp_play_subtitle_text_update(pSubtitle, pSubtitle->szText);
	}

	return TRUE;
}

bool vp_play_subtitle_get_color(subtitle_handle pWidgetHandle,
                                video_subtitle_color_t *nColor)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	*nColor = pSubtitle->nFontColor;

	return TRUE;
}

bool vp_play_subtitle_set_bg_color(subtitle_handle pWidgetHandle,
                                   video_subtitle_color_t nColor)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	pSubtitle->nBGColor = nColor;

	if (pSubtitle->bIsRealize) {
		vp_play_subtitle_text_update(pSubtitle, pSubtitle->szText);
	}

	return TRUE;

}

bool vp_play_subtitle_get_bg_color(subtitle_handle pWidgetHandle,
                                   video_subtitle_color_t *nColor)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	*nColor = pSubtitle->nBGColor;

	return TRUE;
}
#else
bool vp_play_subtitle_get_color(subtitle_handle pWidgetHandle,
                                char **pColorHex)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	if (pSubtitle->pFontColorHex == NULL) {
		VideoLogError("pFontColorHex is NULL");
		return FALSE;
	}

	VP_STRDUP(*pColorHex, pSubtitle->pFontColorHex);

	return TRUE;
}

bool vp_play_subtitle_set_color(subtitle_handle pWidgetHandle,
                                char *pColorHex)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	VP_FREE(pSubtitle->pFontColorHex);
	pSubtitle->pFontColorHex = g_strdup(pColorHex);

	if (pSubtitle->bIsRealize) {
		vp_play_subtitle_text_update(pSubtitle, pSubtitle->szText);
	}

	return TRUE;
}

bool vp_play_subtitle_get_bg_color(subtitle_handle pWidgetHandle,
                                   char **pColorHex)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	if (pSubtitle->pBGColorHex == NULL) {
		VideoLogError("pBGColorHex is NULL");
		return FALSE;
	}

	VP_STRDUP(*pColorHex, pSubtitle->pBGColorHex);

	return TRUE;
}

bool vp_play_subtitle_set_bg_color(subtitle_handle pWidgetHandle,
                                   char *pColorHex)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	VP_FREE(pSubtitle->pBGColorHex);
	pSubtitle->pBGColorHex = g_strdup(pColorHex);

	if (pSubtitle->bIsRealize) {
		vp_play_subtitle_text_update(pSubtitle, pSubtitle->szText);
	}

	return TRUE;
}
#endif

bool vp_play_subtitle_set_text(subtitle_handle pWidgetHandle,
                               const char *szText)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	VP_FREE(pSubtitle->szText);
	VP_STRDUP(pSubtitle->szText, szText);

	if (pSubtitle->bIsRealize) {
		vp_play_subtitle_text_update(pSubtitle, pSubtitle->szText);
	}

	return TRUE;
}

bool vp_play_subtitle_get_text(subtitle_handle pWidgetHandle,
                               char **szText)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;
	VP_STRDUP(*szText, pSubtitle->szText);

	return TRUE;
}

#ifdef SUBTITLE_K_FEATURE
bool vp_play_subtitle_set_alignment(subtitle_handle pWidgetHandle,
                                    vp_subtitle_alignment_t nAlignment)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	pSubtitle->nAlignment = nAlignment;

	if (pSubtitle->bIsRealize) {
		vp_play_subtitle_text_update(pSubtitle, pSubtitle->szText);
		vp_play_subtitle_set_caption_win_color(pSubtitle, TRUE);
	}

	return TRUE;

}

bool vp_play_subtitle_get_alignment(subtitle_handle pWidgetHandle,
                                    vp_subtitle_alignment_t *nAlignment)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;

	*nAlignment = pSubtitle->nAlignment;

	return TRUE;
}

bool vp_play_subtitle_set_caption_win_color(subtitle_handle pWidgetHandle,
        bool bShowCaption)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;
	pSubtitle->bShowCaptionWin = bShowCaption;
	if (!bShowCaption) {
		VideoLogWarning("Hide caption window!");
		vp_play_util_set_object_color(_EDJ(pSubtitle->pLayout), 0, 0, 0,
		                              0,
		                              VP_SUBTITLE_SET_CAPTION_WINDOW_COLOR_ID);
		return TRUE;
	}

	char *pText = NULL;
	vp_play_subtitle_get_text(pSubtitle, &pText);

	if (pText == NULL || strlen(pText) == 0) {
		VideoLogWarning
		("pText is NULL or length is zero, need hide caption window!");
		pSubtitle->bShowCaptionWin = FALSE;
		vp_play_util_set_object_color(_EDJ(pSubtitle->pLayout), 0, 0, 0,
		                              0,
		                              VP_SUBTITLE_SET_CAPTION_WINDOW_COLOR_ID);
		if (pText) {
			VP_FREE(pText);
		}
		return FALSE;
	}

	char *pCaptionWinColorHex = NULL;
	if (!vp_play_preference_get_subtitle_caption_win_color_hex_key
	        (&pCaptionWinColorHex)) {
		VideoLogWarning
		("vp_play_preference_get_subtitle_caption_win_color_hex_key failed");
	}

	int nAlignment = 0;
	vp_play_preference_get_subtitle_alignment_key(&nAlignment);
	if (pCaptionWinColorHex) {
		int r = 0;
		int g = 0;
		int b = 0;
		int a = 0;
		vp_play_util_convert_hex_to_rgba(pCaptionWinColorHex, &r, &g, &b,
		                                 &a);
		vp_play_util_set_object_color(elm_layout_edje_get
		                              (pSubtitle->pLayout), r, g, b, a,
		                              VP_SUBTITLE_SET_CAPTION_WINDOW_COLOR_ID);
	}
	VP_FREE(pText);
	return TRUE;
}

bool vp_play_subtitle_get_geometry(subtitle_handle pWidgetHandle,
                                   bool bLandscape, Evas_Coord *x,
                                   Evas_Coord *y, Evas_Coord *w,
                                   Evas_Coord *h)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;
	if (pSubtitle->pLayout == NULL) {
		VideoLogError("pLayout is NULL");
		return FALSE;
	}

	evas_object_geometry_get(pSubtitle->pLayout, x, y, w, h);
	VideoLogInfo("%d, %d, %d, %d", x, y, w, h);
	int ww = VP_SUBTITLE_PROTRAIT_W;
	int hh = VP_SUBTITLE_PROTRAIT_H;
	if (bLandscape) {
		ww = VP_SUBTITLE_LANDSCAPE_W;
		hh = VP_SUBTITLE_LANDSCAPE_H;
	}
	*x = *x - ww / 2;
	*w = ww;
	*y = *y - hh / 2;
	*h = hh;

	return TRUE;
}
#endif

bool vp_play_subtitle_get_size_formatted(subtitle_handle pWidgetHandle,
        Evas_Coord *ww, Evas_Coord *hh)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	Subtitle *pSubtitle = (Subtitle *) pWidgetHandle;
	if (pSubtitle->pTextBlock == NULL) {
		VideoLogError("pTextBlock is NULL");
		return FALSE;
	}

	evas_object_textblock_size_formatted_get(pSubtitle->pTextBlock, ww,
	        hh);

	return TRUE;
}
