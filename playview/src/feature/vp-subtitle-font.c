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
#include <fontconfig/fontconfig.h>

#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-type-define.h"
#include "vp-play-macro-define.h"

#include "vp-subtitle-font.h"

/* check temp */
#include "vp-play-log.h"

#define VP_SUBTITLE_FONT_GENLIST_DATA_KEY		"vp.subtitle.font.genlist"
#define VP_FONT_PRELOAD_FONT_PATH 			"/usr/share/fonts"
#define VP_FONT_DOWNLOADED_FONT_PATH 			"/opt/share/fonts"
#define VP_FONT_DEFAULT_FONT_NAME 			"Tizen"
#define VP_FONT_UDRGOTHIC_FONT_NAME		"UDRGothic"
#define VP_FONT_UDMINCHO_FONT_NAME		"UDMincho"
#define VP_FONT_POP_FONT_NAME 			"POP"
#define VP_FONT_CHOCOCOOKY_FONT_NAME		"Choco cooky"
#define VP_FONT_COOLJAZZ_FONT_NAME		"Cool jazz"
#define VP_FONT_ROSEMARY_FONT_NAME		"Rosemary"

typedef struct _SubtitleFontPopup {
	Evas_Object *pParent;
	Evas_Object *pPopup;
	Evas_Object *pRadio;
	Evas_Object *pButton;
	Evas_Object *pBox;
	Evas_Object *pGenList;
	Elm_Genlist_Item_Class *st_SubtitleFont_Itc;

	int nCurrentFontIdx;
	int nAppendCount;
	char *szCurrentFont;

	void *pUserData;
	SubtitleFontCbFunc pCloseCb;
	GList *pFontList;
	char *szSelectFont;
} SubtitleFontPopup;



static void _vp_subtitle_font_destroy_handle(SubtitleFontPopup *
        pSubtitleFont);
static void __vp_subtitle_font_genlist_item_selected_cb(void *pUserData,
        Evas_Object *
        pObject,
        void *pEventInfo);

static void __vp_subtitle_font_genlist_realized(void *data,
        Evas_Object *obj,
        void *event_info)
{
	VP_GENLIST_HIDE_BOTTOMLINE(data, obj, event_info);
}

static void __vp_subtitle_font_popup_rotate_cb(void *data,
        Evas_Object *obj,
        void *event_info)
{
	SubtitleFontPopup *pSubtitleFont = (SubtitleFontPopup *) data;
	if (!pSubtitleFont) {
		VideoLogError("pSubtitleFont IS null");
		return;
	}
	if (pSubtitleFont->nAppendCount < VP_POPUP_MIN_ITEMS) {
		return;
	}
	vp_popup_set_popup_min_size(pSubtitleFont->pParent,
	                            pSubtitleFont->pBox,
	                            pSubtitleFont->nAppendCount,
	                            VIDEO_POPUP_DEFAULT);

}

/* callback functions */
static char *__vp_subtitle_font_genlist_text_get_cb(const void *pUserData,
        Evas_Object *pObj,
        const char *pPart)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}

	char *szTxt = (char *)pUserData;

	if (!strcmp(pPart, "elm.text")) {
		char *szFormat = NULL;
		if (strcmp(szTxt, VP_PLAY_STRING_COM_DEFAULT) == 0) {
			szFormat =
			    g_strdup_printf("<font='%s'>%s</font>",
			                    VP_FONT_DEFAULT_FONT_NAME,
			                    VP_PLAY_STRING_FONT_DEFAULT);
		} else if (strcmp(szTxt, VP_FONT_UDRGOTHIC_FONT_NAME) == 0) {
			szFormat =
			    g_strdup_printf("<font='%s'>%s</font>",
			                    VP_FONT_UDRGOTHIC_FONT_NAME,
			                    VP_PLAY_STRING_FONT_UDRCOTHICM);
		} else if (strcmp(szTxt, VP_FONT_UDMINCHO_FONT_NAME) == 0) {
			szFormat =
			    g_strdup_printf("<font='%s'>%s</font>",
			                    VP_FONT_UDMINCHO_FONT_NAME,
			                    VP_PLAY_STRING_FONT_UDMINCHO);
		} else if (strcmp(szTxt, VP_FONT_POP_FONT_NAME) == 0) {
			szFormat =
			    g_strdup_printf("<font='%s'>%s</font>",
			                    VP_FONT_POP_FONT_NAME,
			                    VP_PLAY_STRING_FONT_POP);
		} else if (strcmp(szTxt, VP_FONT_CHOCOCOOKY_FONT_NAME) == 0) {
			szFormat =
			    g_strdup_printf("<font='%s'>%s</font>",
			                    VP_FONT_CHOCOCOOKY_FONT_NAME,
			                    VP_PLAY_STRING_FONT_CHOCO_COOKY);
		} else if (strcmp(szTxt, VP_FONT_COOLJAZZ_FONT_NAME) == 0) {
			szFormat =
			    g_strdup_printf("<font='%s'>%s</font>",
			                    VP_FONT_COOLJAZZ_FONT_NAME,
			                    VP_PLAY_STRING_FONT_COOL_JAZZ);
		} else if (strcmp(szTxt, VP_FONT_ROSEMARY_FONT_NAME) == 0) {
			szFormat =
			    g_strdup_printf("<font='%s'>%s</font>",
			                    VP_FONT_ROSEMARY_FONT_NAME,
			                    VP_PLAY_STRING_FONT_ROSEMARY);
		}

		return szFormat;
	}

	return NULL;
}

static Evas_Object *__vp_subtitle_font_genlist_content_get_cb(const void
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

	char *szTxt = (char *)pUserData;

	if (!strcmp(pPart, "elm.icon")) {
		Evas_Object *pRadioObj = NULL;

		SubtitleFontPopup *pSubtitleFont =
		    (SubtitleFontPopup *) evas_object_data_get(pObj,
		            VP_SUBTITLE_FONT_GENLIST_DATA_KEY);
		if (pSubtitleFont == NULL) {
			VideoLogWarning("evas_object_data_get is fail");
			return NULL;
		}

		int nCount = 0;
		int i = 0;
		int nIndex = 0;

		nCount = g_list_length(pSubtitleFont->pFontList);
		for (i = 0; i < nCount; i++) {
			char *szName = NULL;
			szName =
			    (char *)g_list_nth_data(pSubtitleFont->pFontList, i);
			if (szName == NULL) {
				continue;
			}
			if (strcmp(szName, szTxt) == 0) {
				nIndex = i + 1;
			}
		}

		/* get font list */
		pRadioObj = elm_radio_add(pObj);
		elm_radio_state_value_set(pRadioObj, nIndex);
		elm_radio_group_add(pRadioObj, pSubtitleFont->pRadio);
		elm_radio_value_set(pSubtitleFont->pRadio,
		                    pSubtitleFont->nCurrentFontIdx);
		evas_object_smart_callback_add(pRadioObj, "changed",
		                               __vp_subtitle_font_genlist_item_selected_cb,
		                               pSubtitleFont);
		evas_object_show(pRadioObj);

		return pRadioObj;
	}

	return NULL;
}

static void __vp_subtitle_font_genlist_item_selected_cb(void *pUserData,
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

	Elm_Object_Item *pItem = (Elm_Object_Item *) pEventInfo;
	Elm_Object_Item *pSelectedItem =
	    elm_genlist_selected_item_get(pObject);
	if (pSelectedItem) {
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}
	char *szTxt = (char *)elm_object_item_data_get(pItem);
	VideoLogError("szTxt = %s", szTxt);

	SubtitleFontPopup *pSubtitleFont = (SubtitleFontPopup *) pUserData;
	int nIdx = 0;
	char *szFontText = NULL;
	if (strcmp(szTxt, VP_PLAY_STRING_COM_DEFAULT) == 0) {
		szFontText = VP_FONT_DEFAULT_FONT_NAME;
		nIdx = 0;
	} else {
		szFontText = szTxt;

		int i = 0;
		int nCount = g_list_length(pSubtitleFont->pFontList);
		for (i = 0; i < nCount; i++) {
			char *szName = NULL;
			szName =
			    (char *)g_list_nth_data(pSubtitleFont->pFontList, i);
			if (szName == NULL) {
				continue;
			}
			if (strcmp(szName, szTxt) == 0) {
				nIdx = i + 1;
			}
		}
	}

	VP_FREE(pSubtitleFont->szSelectFont);
	VP_STRDUP(pSubtitleFont->szSelectFont, szFontText);
	pSubtitleFont->nCurrentFontIdx = nIdx;
	if (pSubtitleFont->pCloseCb) {
		pSubtitleFont->pCloseCb(szTxt, (void *) pSubtitleFont->pUserData);
	}
}

static void __vp_subtitle_font_popup_key_event_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleFontPopup *pSubtitleFont = (SubtitleFontPopup *) pUserData;

	if (pSubtitleFont->pCloseCb) {
		pSubtitleFont->pCloseCb(NULL, (void *) pSubtitleFont->pUserData);
	}
}

static void __vp_subtitle_font_popup_mouse_event_cb(void *pUserData,
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
		SubtitleFontPopup *pSubtitleFont =
		    (SubtitleFontPopup *) pUserData;

		if (pSubtitleFont->pCloseCb) {
			pSubtitleFont->pCloseCb(NULL,
			                        (void *) pSubtitleFont->pUserData);
		}
	}
}



/* internal functions */
static void _vp_subtitle_font_destroy_handle(SubtitleFontPopup *
        pSubtitleFont)
{
	if (pSubtitleFont == NULL) {
		VideoLogError("pSubtitleFont is NULL");
		return;
	}
	int nCount = 0;
	int i = 0;
	evas_object_smart_callback_del(pSubtitleFont->pParent,
	                               "rotation,changed",
	                               __vp_subtitle_font_popup_rotate_cb);
	evas_object_smart_callback_del(pSubtitleFont->pGenList, "realized",
	                               __vp_subtitle_font_genlist_realized);

	VP_EVAS_DEL(pSubtitleFont->pRadio);
	VP_EVAS_DEL(pSubtitleFont->pGenList);
	VP_EVAS_DEL(pSubtitleFont->pButton);
	VP_EVAS_DEL(pSubtitleFont->pBox);

	if (pSubtitleFont->st_SubtitleFont_Itc) {
		elm_genlist_item_class_free(pSubtitleFont->st_SubtitleFont_Itc);
		pSubtitleFont->st_SubtitleFont_Itc = NULL;
	}

	nCount = g_list_length(pSubtitleFont->pFontList);
	for (i = 0; i < nCount; i++) {
		char *szName = NULL;
		szName = (char *)g_list_nth_data(pSubtitleFont->pFontList, i);
		VP_FREE(szName);
	}
	g_list_free(pSubtitleFont->pFontList);
	pSubtitleFont->pFontList = NULL;

	VP_EVAS_DEL(pSubtitleFont->pPopup);
	VP_FREE(pSubtitleFont->szCurrentFont);
	VP_FREE(pSubtitleFont->szSelectFont);

	VP_FREE(pSubtitleFont);

}


static Evas_Object *_vp_subtitle_font_create_genlist(Evas_Object *
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

static bool _vp_subtitle_font_add_genlist_item(Evas_Object *pObj,
        void *pUserData)
{
	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return FALSE;
	}

	SubtitleFontPopup *pSubtitleFont = (SubtitleFontPopup *) pUserData;

	if (pSubtitleFont->st_SubtitleFont_Itc) {
		elm_genlist_item_class_free(pSubtitleFont->st_SubtitleFont_Itc);
		pSubtitleFont->st_SubtitleFont_Itc = NULL;
	}

	pSubtitleFont->st_SubtitleFont_Itc = elm_genlist_item_class_new();

	if (pSubtitleFont->st_SubtitleFont_Itc != NULL) {
		pSubtitleFont->st_SubtitleFont_Itc->version =
		    ELM_GENLIST_ITEM_CLASS_VERSION;
		pSubtitleFont->st_SubtitleFont_Itc->item_style =
		    "video.multiline/1text.1icon";
		pSubtitleFont->st_SubtitleFont_Itc->func.text_get =
		    (void *) __vp_subtitle_font_genlist_text_get_cb;
		pSubtitleFont->st_SubtitleFont_Itc->func.content_get =
		    (void *) __vp_subtitle_font_genlist_content_get_cb;
		pSubtitleFont->st_SubtitleFont_Itc->func.state_get = NULL;
		pSubtitleFont->st_SubtitleFont_Itc->func.del = NULL;

		elm_genlist_item_append(pObj, pSubtitleFont->st_SubtitleFont_Itc,
		                        (void *) VP_PLAY_STRING_COM_DEFAULT, NULL,
		                        ELM_GENLIST_ITEM_NONE,
		                        __vp_subtitle_font_genlist_item_selected_cb,
		                        pUserData);
		pSubtitleFont->nAppendCount++;

		int nCount = 0;
		int i = 0;

		nCount = g_list_length(pSubtitleFont->pFontList);
		for (i = 0; i < nCount; i++) {
			char *szName = NULL;
			szName = (char *)g_list_nth_data(pSubtitleFont->pFontList, i);
			if (szName == NULL) {
				continue;
			}
			elm_genlist_item_append(pObj, pSubtitleFont->st_SubtitleFont_Itc,
			                        (void *) szName, NULL,
			                        ELM_GENLIST_ITEM_NONE,
			                        __vp_subtitle_font_genlist_item_selected_cb,
			                        pUserData);
			pSubtitleFont->nAppendCount++;
		}
	}
	return TRUE;
}

static bool __vp_subtitle_font_load_font_list(SubtitleFontPopup *
        pSubtitleFont)
{
	if (pSubtitleFont == NULL) {
		VideoLogError("pSubtitleFont is NULL");
		return FALSE;
	}

	FcObjectSet *os = NULL;
	FcFontSet *fs = NULL;
	FcPattern *pat = NULL;
	FcConfig *font_config = NULL;

	font_config = FcInitLoadConfigAndFonts();
	pat = FcPatternCreate();
	os = FcObjectSetBuild(FC_FAMILY, FC_FILE, (char *)0);
	if (os) {
		fs = FcFontList(font_config, pat, os);
		FcObjectSetDestroy(os);
		os = NULL;
	}

	if (pat) {
		FcPatternDestroy(pat);
		pat = NULL;
	}

	int nPreloadPathLen = strlen(VP_FONT_PRELOAD_FONT_PATH);
	int nDownloadPathLen = strlen(VP_FONT_DOWNLOADED_FONT_PATH);

	if (fs) {
		int j;
		VideoLogError("fs->nfont = %d", fs->nfont);

		for (j = 0; j < fs->nfont; j++) {
			FcChar8 *szfamily = NULL;
			FcChar8 *szFile = NULL;

			if (FcPatternGetString(fs->fonts[j], FC_FILE, 0, &szFile) ==
			        FcResultMatch) {
				if (szFile == NULL) {
					continue;
				}
				if (strncmp
				        ((const char *) szFile, VP_FONT_PRELOAD_FONT_PATH,
				         nPreloadPathLen) == 0
				        || strncmp((const char *) szFile,
				                   VP_FONT_DOWNLOADED_FONT_PATH,
				                   nDownloadPathLen) == 0) {
					if (FcPatternGetString
					        (fs->fonts[j], FC_FAMILY, 0,
					         &szfamily) == FcResultMatch) {
						bool bIsExist = FALSE;

						if (szfamily == NULL) {
							continue;
						}

						if (pSubtitleFont->pFontList) {
							int nCount = 0;
							int nIdx = 0;

							nCount =
							    g_list_length(pSubtitleFont->pFontList);
							for (nIdx = 0; nIdx < nCount; nIdx++) {
								char *szListName = NULL;
								szListName =
								    (char *)
								    g_list_nth_data(pSubtitleFont->
								                    pFontList, nIdx);
								if (szListName) {
									if (strcmp
									        ((char *)szfamily,
									         szListName) == 0) {
										bIsExist = TRUE;
									}
								}
							}
						}
						if (bIsExist == FALSE) {
							if ((strcmp
							        ((char *)szfamily,
							         VP_FONT_UDRGOTHIC_FONT_NAME) == 0)
							        ||
							        (strcmp
							         ((char *)szfamily,
							          VP_FONT_UDMINCHO_FONT_NAME) == 0)
							        ||
							        (strcmp
							         ((char *)szfamily,
							          VP_FONT_POP_FONT_NAME) == 0)
							        ||
							        (strcmp
							         ((char *)szfamily,
							          VP_FONT_CHOCOCOOKY_FONT_NAME) == 0)
							        ||
							        (strcmp
							         ((char *)szfamily,
							          VP_FONT_COOLJAZZ_FONT_NAME) == 0)
							        ||
							        (strcmp
							         ((char *)szfamily,
							          VP_FONT_ROSEMARY_FONT_NAME) == 0)) {
								char *szFontName = NULL;
								VP_STRDUP(szFontName, (char *)szfamily);
								if (szFontName) {
									pSubtitleFont->pFontList =
									    g_list_append(pSubtitleFont->
									                  pFontList, szFontName);
									if (strcmp
									        (pSubtitleFont->szCurrentFont,
									         szFontName) == 0) {
										pSubtitleFont->nCurrentFontIdx =
										    g_list_length(pSubtitleFont->
										                  pFontList);
									}
								}
								VideoLogError("==> [%d] : %s ", j,
								              szfamily);
							}
						}
					}
				}
			}
		}
		FcFontSetDestroy(fs);
		fs = NULL;
	}
	FcConfigDestroy(font_config);
	font_config = NULL;

	return TRUE;
}


/* external functions */
subtitle_font_handle vp_subtitle_font_create(Evas_Object *pParent,
        SubtitleFontCbFunc pCloseCb,
        char *szCurrentFont)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	if (szCurrentFont == NULL) {
		VideoLogError("szCurrentFont is NULL");
		return NULL;
	}
	SubtitleFontPopup *pSubtitleFont = NULL;


	pSubtitleFont = calloc(1, sizeof(SubtitleFontPopup));

	if (pSubtitleFont == NULL) {
		VideoLogError("pSubtitleFont alloc fail");
		return NULL;
	}

	pSubtitleFont->pParent = pParent;
	pSubtitleFont->pCloseCb = pCloseCb;
	VP_STRDUP(pSubtitleFont->szCurrentFont, szCurrentFont);
	pSubtitleFont->nCurrentFontIdx = 0;

	pSubtitleFont->pPopup =
	    vp_popup_create(pParent, POPUP_STYLE_MENU_LIST_NO_CANCEL_BTN,
	                    VP_PLAY_STRING_SUBTITLE_FONT, NULL, 0.0, NULL,
	                    __vp_subtitle_font_popup_key_event_cb,
	                    __vp_subtitle_font_popup_mouse_event_cb,
	                    (void *) pSubtitleFont);
	if (pSubtitleFont->pPopup == NULL) {
		VideoLogError("vp_popup_create fail");
		_vp_subtitle_font_destroy_handle(pSubtitleFont);
		return NULL;
	}

	pSubtitleFont->pGenList =
	    _vp_subtitle_font_create_genlist(pSubtitleFont->pPopup);
	if (pSubtitleFont->pGenList == NULL) {
		VideoLogError("_vp_subtitle_font_create_genlist fail");
		_vp_subtitle_font_destroy_handle(pSubtitleFont);
		return NULL;
	}
	evas_object_data_set(pSubtitleFont->pGenList,
	                     VP_SUBTITLE_FONT_GENLIST_DATA_KEY,
	                     (void *) pSubtitleFont);
	evas_object_smart_callback_add(pSubtitleFont->pGenList, "realized",
	                               __vp_subtitle_font_genlist_realized,
	                               NULL);

	pSubtitleFont->pRadio = elm_radio_add(pSubtitleFont->pGenList);

	__vp_subtitle_font_load_font_list(pSubtitleFont);

	elm_genlist_mode_set(pSubtitleFont->pGenList, ELM_LIST_COMPRESS);

	if (!_vp_subtitle_font_add_genlist_item
	        (pSubtitleFont->pGenList, (void *) pSubtitleFont)) {
		VideoLogError("_vp_subtitle_font_add_genlist_item fail");
		return FALSE;
	}
	//elm_radio_value_set(pSubtitleFont->pRadio, pSubtitleFont->nCurrentFontIdx);
	evas_object_smart_callback_add(pSubtitleFont->pParent,
	                               "rotation,changed",
	                               __vp_subtitle_font_popup_rotate_cb,
	                               pSubtitleFont);

	pSubtitleFont->pBox = elm_box_add(pSubtitleFont->pPopup);
	vp_popup_set_popup_min_size(pSubtitleFont->pParent,
	                            pSubtitleFont->pBox,
	                            pSubtitleFont->nAppendCount,
	                            VIDEO_POPUP_DEFAULT);

	elm_box_pack_end(pSubtitleFont->pBox, pSubtitleFont->pGenList);
	elm_object_content_set(pSubtitleFont->pPopup, pSubtitleFont->pBox);

	return pSubtitleFont;
}

void vp_subtitle_font_destroy(subtitle_font_handle pSubtitleFontHandle)
{
	if (pSubtitleFontHandle == NULL) {
		VideoLogError("pSubtitleFontHandle is NULL");
		return;
	}

	SubtitleFontPopup *pSubtitleFont =
	    (SubtitleFontPopup *) pSubtitleFontHandle;

	_vp_subtitle_font_destroy_handle(pSubtitleFont);

}

bool vp_subtitle_font_realize(subtitle_font_handle pSubtitleFontHandle)
{
	if (pSubtitleFontHandle == NULL) {
		VideoLogError("pSubtitleFontHandle is NULL");
		return FALSE;
	}

	SubtitleFontPopup *pSubtitleFont =
	    (SubtitleFontPopup *) pSubtitleFontHandle;

	evas_object_show(pSubtitleFont->pPopup);

	return TRUE;
}

bool vp_subtitle_font_unrealize(subtitle_font_handle pSubtitleFontHandle)
{
	if (pSubtitleFontHandle == NULL) {
		VideoLogError("pSubtitleFontHandle is NULL");
		return FALSE;
	}

	SubtitleFontPopup *pSubtitleFont =
	    (SubtitleFontPopup *) pSubtitleFontHandle;

	evas_object_hide(pSubtitleFont->pPopup);

	return TRUE;
}

bool vp_subtitle_font_set_user_data(subtitle_font_handle
                                    pSubtitleFontHandle, void *pUserData)
{
	if (pSubtitleFontHandle == NULL) {
		VideoLogError("pSubtitleFontHandle is NULL");
		return FALSE;
	}

	SubtitleFontPopup *pSubtitleFont =
	    (SubtitleFontPopup *) pSubtitleFontHandle;

	pSubtitleFont->pUserData = pUserData;

	return TRUE;
}
