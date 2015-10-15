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
#pragma once

#include <stdbool.h>
#include <Elementary.h>

typedef void *subtitle_handle;

typedef enum {
	VP_SUBTITLE_SMALL = 0x00,
	VP_SUBTITLE_MEDIUM,
	VP_SUBTITLE_LARGE,
}video_subtitle_size_t;

typedef enum {
	VP_SUBTITLE_COLOR_WHITE = 0x00,
	VP_SUBTITLE_COLOR_BLACK,
	VP_SUBTITLE_COLOR_RED,
	VP_SUBTITLE_COLOR_GREEN,
	VP_SUBTITLE_COLOR_BLUE,
	VP_SUBTITLE_COLOR_YELLOW,
	VP_SUBTITLE_COLOR_MAGENTA,
	VP_SUBTITLE_COLOR_CYAN,
	VP_SUBTITLE_COLOR_CUSTOM,
	VP_SUBTITLE_COLOR_NONE
}video_subtitle_color_t;

typedef enum {
	VP_SUBTITLE_ALIGNMENT_LEFT = 0x00,
	VP_SUBTITLE_ALIGNMENT_CENTER,
	VP_SUBTITLE_ALIGNMENT_RIGHT
}vp_subtitle_alignment_t;

typedef enum {
	VP_SUBTITLE_TYPE_NORMAL = 0x00,
	VP_SUBTITLE_TYPE_MULTI
}vp_subtitle_type_t;

typedef struct _SubtitleWidget
{
	Evas_Object		*pParent;
	Evas_Object		*pLayout;
	Evas_Object		*pTextBlock;
	char			*szFontName;
	char			*szText;
	video_subtitle_size_t	nSize;
	int				nEdge;
	float			fZoom;

#ifdef SUBTITLE_K_FEATURE
	vp_subtitle_alignment_t nAlignment;
#endif

#ifndef SUBTITLE_K_FEATURE
	video_subtitle_color_t	nFontColor;
	video_subtitle_color_t	nBGColor;
#else
	char *pFontColorHex;
	char *pBGColorHex;
	char *pCaptionWinColorHex;
#endif
	bool			bIsRealize;
	bool			bShowCaptionWin;
}Subtitle;

subtitle_handle vp_play_subtitle_create(Evas_Object *pParent, vp_subtitle_type_t nType);
void vp_play_subtitle_destroy(subtitle_handle pWidgetHandle);
bool vp_play_subtitle_realize(subtitle_handle pWidgetHandle);
bool vp_play_subtitle_unrealize(subtitle_handle pWidgetHandle);
bool vp_play_subtitle_is_realize(subtitle_handle pWidgetHandle, bool *bIsRealize);
Evas_Object *vp_play_subtitle_get_object(subtitle_handle pWidgetHandle);
bool vp_play_subtitle_set_font(subtitle_handle pWidgetHandle, char *szFontName);
bool vp_play_subtitle_get_font(subtitle_handle pWidgetHandle, char **szFontName);
bool vp_play_subtitle_set_size(subtitle_handle pWidgetHandle, video_subtitle_size_t nSize);
bool vp_play_subtitle_get_size(subtitle_handle pWidgetHandle, video_subtitle_size_t *nSize);
bool vp_play_subtitle_set_size_zoom(subtitle_handle pWidgetHandle, float fZoom);
bool vp_play_subtitle_get_size_zoom(subtitle_handle pWidgetHandle, float *fZoom);
void vp_play_subtitle_text_update(Subtitle *pSubtitle, const char *szTxt);
#ifndef SUBTITLE_K_FEATURE
bool vp_play_subtitle_set_color(subtitle_handle pWidgetHandle, video_subtitle_color_t nColor);
bool vp_play_subtitle_get_color(subtitle_handle pWidgetHandle, video_subtitle_color_t *nColor);
bool vp_play_subtitle_set_bg_color(subtitle_handle pWidgetHandle, video_subtitle_color_t nColor);
bool vp_play_subtitle_get_bg_color(subtitle_handle pWidgetHandle, video_subtitle_color_t *nColor);
#else
bool vp_play_subtitle_get_color(subtitle_handle pWidgetHandle, char **pColorHex);
bool vp_play_subtitle_set_color(subtitle_handle pWidgetHandle, char *pColorHex);
bool vp_play_subtitle_get_bg_color(subtitle_handle pWidgetHandle, char **pColorHex);
bool vp_play_subtitle_set_bg_color(subtitle_handle pWidgetHandle, char *pColorHex);
#endif
bool vp_play_subtitle_set_text(subtitle_handle pWidgetHandle, const char *szText);
bool vp_play_subtitle_get_text(subtitle_handle pWidgetHandle, char **szText);

#ifdef SUBTITLE_K_FEATURE
bool vp_play_subtitle_set_alignment(subtitle_handle pWidgetHandle, vp_subtitle_alignment_t nAlignment);
bool vp_play_subtitle_get_alignment(subtitle_handle pWidgetHandle, vp_subtitle_alignment_t *nAlignment);
bool vp_play_subtitle_set_caption_win_color(subtitle_handle pWidgetHandle, bool bShowCaption);
bool vp_play_subtitle_get_geometry(subtitle_handle pWidgetHandle, bool bLandscape, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
#endif

bool vp_play_subtitle_set_edge(subtitle_handle pWidgetHandle, int nEdge);
bool vp_play_subtitle_get_edge(subtitle_handle pWidgetHandle, int *nEdge);

bool vp_play_subtitle_get_size_formatted(subtitle_handle pWidgetHandle, Evas_Coord *ww, Evas_Coord *hh);
