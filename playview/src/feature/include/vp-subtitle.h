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

#include <vp-play-popup.h>

typedef void *subtitle_popup_handle;

typedef enum {
	VP_SUBTITLE_MODE_NONE = 0x00,
	VP_SUBTITLE_MODE_ACTIVATION,
	VP_SUBTITLE_MODE_SYNC,
	VP_SUBTITLE_MODE_SELECT,
	VP_SUBTITLE_MODE_LANGUAGE,
#ifdef SUBTITLE_K_FEATURE
	VP_SUBTITLE_MODE_ALIGNMENT,
#endif
	VP_SUBTITLE_MODE_FONT,
	VP_SUBTITLE_MODE_SIZE,
	VP_SUBTITLE_MODE_TEXT,
	VP_SUBTITLE_MODE_BG_COLOR,
#ifdef SUBTITLE_K_FEATURE
	VP_SUBTITLE_MODE_CAPTION_WINDOW,
#endif
	VP_SUBTITLE_MODE_EDGE,
	VP_SUBTITLE_MODE_CLOSE
}vp_subtitle_mode_t;

typedef enum {
	VP_SUBTITLE_EDGE_NO_EDGE = 0x00,
	VP_SUBTITLE_EDGE_RAISED,
	VP_SUBTITLE_EDGE_DEPRESSED,
	VP_SUBTITLE_EDGE_UNIFORM,
	VP_SUBTITLE_EDGE_DROP_SHADOW
}vp_subtitle_edge_t;


typedef struct _SubtitleInfo{
	bool						bClosedCaption;
	bool						original_activation;
	float						fSync;
	char						*szURL;
	char						*szText;
	char						*szLanguage;
#ifdef SUBTITLE_K_FEATURE
	video_play_subtitle_alignment_t		eAlignment;
	video_play_subtitle_alignment_t		original_eAlignment;
#endif
	char						*szFontName;
	float						fZoom;
	int 						nFontSize;
	int						    original_nFontSize;
	int							nEdge;
#ifndef SUBTITLE_K_FEATURE
	int 						nTextColor;
	int 						nBGColor;
#else
	char						*pTextColorHex;
	char						*pBGColorHex;
	char						*pCaptionWinColorHex;
#endif
	bool						bSupportSelect;
}SubtitleInfo;


subtitle_popup_handle vp_subtitle_create(Evas_Object *pParent, SubtitleInfo *pSubtitleInfo, PopupCloseCbFunc pCloseCb);
void vp_subtitle_destroy(subtitle_popup_handle pSubtitleHandle);
bool vp_subtitle_realize(subtitle_popup_handle pSubtitleHandle);
bool vp_subtitle_unrealize(subtitle_popup_handle pSubtitleHandle);
bool vp_subtitle_update(subtitle_popup_handle pSubtitleHandle, SubtitleInfo *pSubtitleInfo);
bool vp_subtitle_set_user_data(subtitle_popup_handle pSubtitleHandle, void *pUserData);

