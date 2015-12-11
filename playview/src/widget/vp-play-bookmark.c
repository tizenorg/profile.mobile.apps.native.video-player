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

#include "vp-play-bookmark.h"

#include "vp-play-button.h"
#include "vp-play-util.h"

#include "vp-media-contents.h"


/* check temp */
#include "vp-play-log.h"


#define VP_BOOKMARK_ITEM_WIDTH		(152 * elm_config_scale_get())
#define VP_BOOKMARK_ITEM_HEIGHT		(132 * elm_config_scale_get())

#define VP_BOOKMARK_THUMB_WIDTH		(146 * elm_config_scale_get())
#define VP_BOOKMARK_THUMB_HEIGHT	(88 * elm_config_scale_get())


#define VP_BOOKMARK_GENGRID_DATA_KEY		"vp.bookmark.gengrid"
#define VP_BOOKMARK_GENGRID_ITEM_DATA_KEY	"vp.bookmark.gengrid.item"

#define VP_BOOKMARK_PLAY_TIME_LENGTH 	9
#define VP_BOOKMARK_PLAY_TIME_FORMAT 	"02u:%02u:%02u"
#define VP_BOOKMARK_PLAY_TIME_ARGS(t) \
	(t) / (60 * 60), \
	((t) / 60) % 60, \
	(t) % 60

typedef struct _BookmarkItem {
	Elm_Object_Item *pElmItem;
	char *szFilePath;
	int nPos;
} BookmarkItem;

typedef struct _BookmarkWidget {
	Evas_Object *pParent;
	Evas_Object *pLayout;
	Evas_Object *pAddButton;
	Evas_Object *pGengrid;

	Elm_Object_Item *pLastItem;
	Elm_Gengrid_Item_Class *pItemClass;

	char *szMediaURL;
	char *szMediaID;

	bool bIsRealize;
	bool bEditMode;
	bool bPressed;
	GList *pItemList;

	BookmarkCaptureStartCbFunc pCaptureCb;
	BookmarkItemSelectCbFunc pSelectCb;

	void *pUserData;
} BookmarkWidget;

static void _vp_play_bookmark_destory_handle(BookmarkWidget *
        pBookmarkWidget);
static void _vp_play_bookmark_gengrid_item_update(BookmarkWidget *
        pBookmarkWidget);
static void _vp_play_bookmark_gengrid_item_clear(BookmarkWidget *
        pBookmarkWidget);
/* callbackk functions */

static void __vp_play_bookmark_delete_btn_clicked_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEvent)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return;
	}
	VideoLogError("clicked");
}

static void __vp_play_bookmark_delete_btn_press_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEvent)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return;
	}
	VideoLogError("press");

	Evas_Object *pIcon = NULL;
	pIcon =
	    elm_object_part_content_get(pObj, VP_PLAY_SWALLOW_BUTTON_ICON);
	if (pIcon) {
		evas_object_del(pIcon);
		pIcon = NULL;
	}
}

static void __vp_play_bookmark_delete_btn_unpress_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEvent)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return;
	}
	VideoLogError("unpress");

	Evas_Object *pIcon = NULL;
	pIcon =
	    elm_object_part_content_get(pObj, VP_PLAY_SWALLOW_BUTTON_ICON);
	if (pIcon) {
		evas_object_del(pIcon);
		pIcon = NULL;
	}
}

static Evas_Object *__vp_play_bookmark_gengrid_icon_get_cb(const void
        *pUserData,
        Evas_Object *
        pObj,
        const char
        *pPart)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}

	if (pPart == NULL) {
		VideoLogError("pPart is NULL");
		return NULL;
	}

	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return NULL;
	}

	BookmarkItem *pBookmarkItem = (BookmarkItem *) pUserData;
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s/%s/%s", path , "edje", VP_PLAY_BOOKMARK_ITEM_EDJ_PATH);

	if (!strcmp(pPart, "elm.swallow.icon")) {
		Evas_Object *layout = NULL;
		Evas_Object *bg = NULL;
		char buf[VP_BOOKMARK_PLAY_TIME_LENGTH] = { 0, };

		layout = elm_layout_add(pObj);
		bg = elm_bg_add(pObj);

		if (bg == NULL || layout == NULL) {
			VideoLogError("evas object is NULL");
			return NULL;
		}
		elm_layout_file_set(layout, edj_path,
		                    VP_PLAY_EDJ_GROUP_BOOKMARK_ITEM);
		elm_bg_load_size_set(bg, VP_BOOKMARK_THUMB_WIDTH,
		                     VP_BOOKMARK_THUMB_HEIGHT);
		elm_bg_file_set(bg, pBookmarkItem->szFilePath, NULL);
		evas_object_size_hint_max_set(bg, VP_BOOKMARK_THUMB_WIDTH,
		                              VP_BOOKMARK_THUMB_HEIGHT);

		evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND,
		                                 EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(bg, EVAS_HINT_FILL,
		                                EVAS_HINT_FILL);

		elm_object_part_content_set(layout,
		                            VP_PLAY_SWALLOW_BOOKMARK_ITEM_ICON,
		                            bg);

		snprintf(buf, VP_BOOKMARK_PLAY_TIME_LENGTH,
		         "%" VP_BOOKMARK_PLAY_TIME_FORMAT,
		         VP_BOOKMARK_PLAY_TIME_ARGS(pBookmarkItem->nPos / 1000));

		elm_object_part_text_set(layout,
		                         VP_PLAY_SWALLOW_BOOKMARK_ITEM_TXT, buf);

		evas_object_show(bg);
		evas_object_show(layout);

		return layout;

	} else if (!strcmp(pPart, "elm.swallow.end")) {
		BookmarkWidget *pBookmarkWidget =
		    (BookmarkWidget *) evas_object_data_get(pObj,
		            VP_BOOKMARK_GENGRID_DATA_KEY);
		if (pBookmarkWidget == NULL) {
			VideoLogError("pBookmarkWidget is NULL");
			return NULL;
		}
		if (pBookmarkWidget->bEditMode == FALSE) {
			return NULL;
		}

		Evas_Object *layout = NULL;
		Evas_Object *pIcon = NULL;
		Evas_Object *pBtn = NULL;

		layout = elm_layout_add(pObj);
		elm_layout_file_set(layout, edj_path,
		                    VP_PLAY_EDJ_GROUP_BOOKMARK_ITEM_END);
		pBtn =
		    vp_button_create(layout, "playview/custom/icon_48_48", NULL,
		                     (Evas_Smart_Cb)
		                     __vp_play_bookmark_delete_btn_clicked_cb,
		                     (Evas_Smart_Cb)
		                     __vp_play_bookmark_delete_btn_press_cb,
		                     (Evas_Smart_Cb)
		                     __vp_play_bookmark_delete_btn_unpress_cb,
		                     (void *) pBookmarkItem);

		evas_object_data_set(pBtn, VP_BOOKMARK_GENGRID_ITEM_DATA_KEY,
		                     (void *) pBookmarkWidget);
		return layout;
	}
	return NULL;

}


static void __vp_play_bookmark_longpress_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEvent)
{
	VideoLogInfo("");

	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pUserData;

	if (pBookmarkWidget->bEditMode == EINA_TRUE) {
		pBookmarkWidget->bEditMode = EINA_FALSE;
		pBookmarkWidget->bPressed = FALSE;

	} else {
		pBookmarkWidget->bEditMode = EINA_TRUE;
	}

	_vp_play_bookmark_gengrid_item_update(pBookmarkWidget);

}

static void __vp_play_bookmark_scroll_cb(void *pUserData,
        Evas_Object *pObj, void *pEvent)
{
	return;
}

static void __vp_play_bookmark_change_cb(void *pUserData,
        Evas_Object *pObj, void *pEvent)
{
	if (pUserData == NULL) {
		VideoLogError("bookmark data is NULL");
		return;
	}
	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pUserData;

	if (pBookmarkWidget->pLastItem == NULL) {
		return;
	}

	elm_gengrid_item_bring_in(pBookmarkWidget->pLastItem,
	                          ELM_GENGRID_ITEM_SCROLLTO_MIDDLE);
	pBookmarkWidget->pLastItem = NULL;
}

static void __vp_play_bookmark_pressed_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEvent)
{
	if (pUserData == NULL) {
		VideoLogError("bookmark data is NULL");
		return;
	}
	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pUserData;

	pBookmarkWidget->bPressed = TRUE;

	if (pBookmarkWidget->pSelectCb) {
		pBookmarkWidget->pSelectCb(-1, NULL, pBookmarkWidget->pUserData);
	}
}


static void __vp_play_bookmark_released_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEvent)
{
	if (pUserData == NULL) {
		VideoLogError("bookmark data is NULL");
		return;
	}
	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pUserData;

	pBookmarkWidget->bPressed = FALSE;
}


static void __vp_play_bookmark_item_select_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEvent)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	if (pEvent == NULL) {
		VideoLogError("pEvent is NULL");
		return;
	}

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pUserData;
	BookmarkItem *pBookmarkItem = NULL;
	pBookmarkItem =
	    (BookmarkItem *) elm_object_item_data_get((Elm_Object_Item *)
	            pEvent);

	elm_gengrid_item_selected_set(pBookmarkItem->pElmItem, EINA_FALSE);

	if (pBookmarkWidget->bEditMode) {
		pBookmarkWidget->bEditMode = FALSE;
		pBookmarkWidget->bPressed = FALSE;
		_vp_play_bookmark_gengrid_item_update(pBookmarkWidget);
		VideoLogError("+++");
		return;
	} else {
		if (pBookmarkWidget->pSelectCb) {
			pBookmarkWidget->pSelectCb(pBookmarkItem->nPos,
			                           pBookmarkItem->szFilePath,
			                           pBookmarkWidget->pUserData);
		}
	}
}


static void __vp_play_bookmark_btn_clicked_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEvent)
{
	if (pUserData == NULL) {
		VideoLogError("bookmark data is NULL");
		return;
	}

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pUserData;

	if (pBookmarkWidget->bEditMode) {
		pBookmarkWidget->bEditMode = FALSE;
		pBookmarkWidget->bPressed = FALSE;
		_vp_play_bookmark_gengrid_item_update(pBookmarkWidget);
	}

	if (pBookmarkWidget->pAddButton == pObj) {
		if (pBookmarkWidget->pCaptureCb) {
			pBookmarkWidget->pCaptureCb(pBookmarkWidget->pUserData);
		}
	}
}

static void __vp_play_bookmark_btn_press_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}
	if (!pObj) {
		VideoLogError("pObj is NULL");
		return;
	}

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pUserData;
	if (pBookmarkWidget->pAddButton == pObj) {
		Evas_Object *pIcon = NULL;
		pIcon =
		    elm_object_part_content_get(pBookmarkWidget->pAddButton,
		                                VP_PLAY_SWALLOW_BUTTON_ICON);
		if (pIcon) {
			evas_object_del(pIcon);
			pIcon = NULL;
		}
	}
}

static void __vp_play_bookmark_btn_unpress_cb(void *pUserData,
        Evas_Object *pObj,
        void *pEvent)
{
	if (!pUserData) {
		VideoLogError("pUserData is NULL");
		return;
	}

	if (!pObj) {
		VideoLogError("pObj is NULL");
		return;
	}

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pUserData;
	if (pBookmarkWidget->pAddButton == pObj) {
		Evas_Object *pIcon = NULL;
		pIcon =
		    elm_object_part_content_get(pBookmarkWidget->pAddButton,
		                                VP_PLAY_SWALLOW_BUTTON_ICON);
		if (pIcon) {
			evas_object_del(pIcon);
			pIcon = NULL;
		}

	}
}


/* internal functions */

static void _vp_play_bookmark_destory_handle(BookmarkWidget *
        pBookmarkWidget)
{
	if (pBookmarkWidget == NULL) {
		VideoLogError("pBookmarkWidget is NULL");
		return;
	}

	_vp_play_bookmark_gengrid_item_clear(pBookmarkWidget);

	if (pBookmarkWidget->pItemClass) {
		elm_gengrid_item_class_free(pBookmarkWidget->pItemClass);
		pBookmarkWidget->pItemClass = NULL;
	}

	VP_EVAS_DEL(pBookmarkWidget->pGengrid);
	VP_EVAS_DEL(pBookmarkWidget->pAddButton);
	VP_EVAS_DEL(pBookmarkWidget->pLayout);

	VP_FREE(pBookmarkWidget->szMediaID);
	VP_FREE(pBookmarkWidget->szMediaURL);

	VP_FREE(pBookmarkWidget);
}

static Elm_Object_Item *_vp_play_bookmark_find_before_item(GList *pList,
        int nPosition,
        int *nPos)
{
	if (pList == NULL) {
		VideoLogError("pBookmarkItem list is NULL");
		return NULL;
	}

	int nCnt = 0;
	int idx = 0;

	nCnt = g_list_length(pList);
	for (idx = 0; idx < nCnt; idx++) {
		BookmarkItem *pBookmarkItem = NULL;
		pBookmarkItem = (BookmarkItem *) g_list_nth_data(pList, idx);
		if (pBookmarkItem) {
			if (pBookmarkItem->nPos > nPosition) {
				VideoLogError("Item : %d , nPosition : %d",
				              pBookmarkItem->nPos, nPosition);
				*nPos = idx + 1;
				return pBookmarkItem->pElmItem;
			}
		}
	}

	return NULL;
}


static gint __vp_play_bookmark_compare_cb(BookmarkItem *pItem1,
        BookmarkItem *pItem2)
{
	if (pItem1 == NULL) {
		VideoLogError("pItem1 is NULL");
		return 0;
	}

	if (pItem2 == NULL) {
		VideoLogError("pItem2 is NULL");
		return 0;
	}

	if (pItem1->nPos > pItem2->nPos) {
		return 1;
	} else {
		return 0;
	}
}


static void _vp_play_bookmark_gengrid_item_append(BookmarkWidget *
        pBookmarkWidget,
        BookmarkItem *pItem)
{
	if (pBookmarkWidget == NULL) {
		VideoLogError("pBookmarkWidget is NULL");
		return;
	}

	if (pItem == NULL) {
		VideoLogError("pBookmarkWidget is NULL");
		return;
	}
	if (pBookmarkWidget->pItemClass == NULL) {
		pBookmarkWidget->pItemClass = elm_gengrid_item_class_new();
		if (pBookmarkWidget->pItemClass == NULL) {
			VideoLogError("gengrid item class alloc fail");
			return;
		}
	}

	pBookmarkWidget->pItemClass->item_style = "bookmark";
	pBookmarkWidget->pItemClass->func.text_get = NULL;
	pBookmarkWidget->pItemClass->func.content_get =
	    (void *) __vp_play_bookmark_gengrid_icon_get_cb;
	pBookmarkWidget->pItemClass->func.state_get = NULL;
	pBookmarkWidget->pItemClass->func.del = NULL;

	Elm_Object_Item *pBeforeItem = NULL;
	int nPos = 0;
	pBeforeItem =
	    _vp_play_bookmark_find_before_item(pBookmarkWidget->pItemList,
	                                       pItem->nPos, &nPos);

	if (pBeforeItem) {
		pItem->pElmItem =
		    elm_gengrid_item_insert_before(pBookmarkWidget->pGengrid,
		                                   pBookmarkWidget->pItemClass,
		                                   (void *) pItem, pBeforeItem,
		                                   __vp_play_bookmark_item_select_cb,
		                                   (void *) pBookmarkWidget);
	} else {
		pItem->pElmItem =
		    elm_gengrid_item_append(pBookmarkWidget->pGengrid,
		                            pBookmarkWidget->pItemClass,
		                            (void *) pItem,
		                            __vp_play_bookmark_item_select_cb,
		                            (void *) pBookmarkWidget);
	}

	pBookmarkWidget->pLastItem = pItem->pElmItem;

	pBookmarkWidget->pItemList =
	    g_list_insert_sorted(pBookmarkWidget->pItemList, pItem,
	                         (GCompareFunc)
	                         __vp_play_bookmark_compare_cb);
}

static void _vp_play_bookmark_gengrid_item_update(BookmarkWidget *
        pBookmarkWidget)
{
	if (pBookmarkWidget == NULL) {
		VideoLogError("pBookmarkWidget is NULL");
		return;
	}

	if (pBookmarkWidget->pItemList == NULL) {
		VideoLogError("pItemList is NULL");
		return;
	}

	int nCount = 0;
	int i = 0;

	nCount = g_list_length(pBookmarkWidget->pItemList);
	for (i = 0; i < nCount; i++) {
		BookmarkItem *pBookmarkItem = NULL;
		pBookmarkItem =
		    (BookmarkItem *) g_list_nth_data(pBookmarkWidget->pItemList,
		                                     i);
		if (pBookmarkItem) {
			if (pBookmarkItem->pElmItem) {
				elm_gengrid_item_update(pBookmarkItem->pElmItem);
				elm_gengrid_item_selected_set(pBookmarkItem->pElmItem,
				                              EINA_FALSE);
			}
		}
	}
}

static void _vp_play_bookmark_gengrid_item_clear(BookmarkWidget *
        pBookmarkWidget)
{
	if (pBookmarkWidget == NULL) {
		VideoLogError("pBookmarkWidget is NULL");
		return;
	}

	if (pBookmarkWidget->pItemList == NULL) {
		VideoLogError("pItemList is NULL");
		return;
	}

	int nCount = 0;
	int i = 0;

	nCount = g_list_length(pBookmarkWidget->pItemList);
	for (i = 0; i < nCount; i++) {
		BookmarkItem *pBookmarkItem = NULL;

		pBookmarkItem =
		    (BookmarkItem *) g_list_nth_data(pBookmarkWidget->pItemList,
		                                     i);
		if (pBookmarkItem) {
			VP_FREE(pBookmarkItem->szFilePath);
			VP_FREE(pBookmarkItem);
		}
	}

	g_list_free(pBookmarkWidget->pItemList);
	pBookmarkWidget->pItemList = NULL;
}


static Evas_Object *_vp_play_bookmark_create_layout(Evas_Object *pParent)
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
	snprintf(edj_path, 1024, "%s/%s/%s", path , "edje", VP_PLAY_BOOKMARK_EDJ_PATH);

	bRet =
	    elm_layout_file_set(pObj, edj_path,
	                        VP_PLAY_EDJ_GROUP_BOOKMARK);
	if (bRet != EINA_TRUE) {
		VideoLogError("elm_layout_file_set fail");
		return NULL;
	}

	return pObj;
}


static Evas_Object *_vp_play_bookmark_create_gengrid(Evas_Object *
        pParent)
{
	if (!pParent) {
		VideoLogError("Parent is NULL");
		return NULL;
	}

	Evas_Object *pObj = NULL;

	pObj = elm_gengrid_add(pParent);
	if (pObj == NULL) {
		VideoLogError("elm_gengrid_add object is NULL");
		return NULL;
	}

	elm_gengrid_item_size_set(pObj, VP_BOOKMARK_ITEM_WIDTH,
	                          VP_BOOKMARK_ITEM_HEIGHT);

	elm_gengrid_align_set(pObj, 0.0, 0.0);
	elm_gengrid_horizontal_set(pObj, EINA_TRUE);
	elm_gengrid_multi_select_set(pObj, EINA_TRUE);
	evas_object_size_hint_weight_set(pObj, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	evas_object_show(pObj);

	return pObj;
}

static void _vp_play_bookmark_layout_del_cb(void *data, Evas *e,
        Evas_Object *obj,
        void *event_info)
{
	if (data == NULL) {
		VideoLogError("pBookmarkWidget is NULL");
		return;
	}

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) data;
	pBookmarkWidget->pLayout = NULL;
	pBookmarkWidget->pAddButton = NULL;
	pBookmarkWidget->pGengrid = NULL;
}

static bool _vp_play_bookmark_init_layout(BookmarkWidget *
        pBookmarkWidget)
{
	if (pBookmarkWidget == NULL) {
		VideoLogError("pBookmarkWidget is NULL");
		return FALSE;
	}

	Evas_Object *pParent = pBookmarkWidget->pParent;

	pBookmarkWidget->pLayout = _vp_play_bookmark_create_layout(pParent);
	if (pBookmarkWidget->pLayout == NULL) {
		VideoLogError("_vp_play_bookmark_create_layout is fail");
		return FALSE;
	}
	evas_object_event_callback_add(pBookmarkWidget->pLayout,
	                               EVAS_CALLBACK_DEL,
	                               _vp_play_bookmark_layout_del_cb,
	                               (void *) pBookmarkWidget);

	pBookmarkWidget->pGengrid =
	    _vp_play_bookmark_create_gengrid(pBookmarkWidget->pLayout);
	if (pBookmarkWidget->pGengrid == NULL) {
		VideoLogError("_vp_play_bookmark_create_gengrid is fail");
		return FALSE;
	}

	evas_object_data_set(pBookmarkWidget->pGengrid,
	                     VP_BOOKMARK_GENGRID_DATA_KEY,
	                     (void *) pBookmarkWidget);

	pBookmarkWidget->pAddButton =
	    vp_button_create(pParent, "playview/custom/icon_44_44", NULL,
	                     (Evas_Smart_Cb)
	                     __vp_play_bookmark_btn_clicked_cb,
	                     (Evas_Smart_Cb) __vp_play_bookmark_btn_press_cb,
	                     (Evas_Smart_Cb)
	                     __vp_play_bookmark_btn_unpress_cb,
	                     (void *) pBookmarkWidget);
	if (!pBookmarkWidget->pAddButton) {
		VideoLogError("elm_button_add is fail");
		return FALSE;
	}

	Evas_Object *pIcon = NULL;
	elm_object_part_content_set(pBookmarkWidget->pAddButton,
	                            VP_PLAY_SWALLOW_BUTTON_ICON, pIcon);

	elm_object_part_content_set(pBookmarkWidget->pLayout,
	                            VP_PLAY_SWALLOW_BOOKMARK_ADD_BUTTON,
	                            pBookmarkWidget->pAddButton);
	elm_object_part_content_set(pBookmarkWidget->pLayout,
	                            VP_PLAY_SWALLOW_BOOKMARK_LIST,
	                            pBookmarkWidget->pGengrid);

	evas_object_smart_callback_add(pBookmarkWidget->pGengrid,
	                               "longpressed",
	                               __vp_play_bookmark_longpress_cb,
	                               (void *) pBookmarkWidget);
	evas_object_smart_callback_add(pBookmarkWidget->pGengrid, "scroll",
	                               __vp_play_bookmark_scroll_cb,
	                               (void *) pBookmarkWidget);
	evas_object_smart_callback_add(pBookmarkWidget->pGengrid, "changed",
	                               __vp_play_bookmark_change_cb,
	                               (void *) pBookmarkWidget);
	evas_object_smart_callback_add(pBookmarkWidget->pGengrid, "pressed",
	                               __vp_play_bookmark_pressed_cb,
	                               (void *) pBookmarkWidget);
	evas_object_smart_callback_add(pBookmarkWidget->pGengrid, "released",
	                               __vp_play_bookmark_released_cb,
	                               (void *) pBookmarkWidget);

	return TRUE;
}


/* external functions */
bookmark_handle vp_play_bookmark_create(Evas_Object *pParent)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	BookmarkWidget *pBookmarkWidget = NULL;

	pBookmarkWidget = calloc(1, sizeof(BookmarkWidget));

	if (pBookmarkWidget == NULL) {
		VideoLogError("pBookmarkWidget alloc fail");
		return NULL;
	}

	pBookmarkWidget->pParent = pParent;
	pBookmarkWidget->bIsRealize = FALSE;
	pBookmarkWidget->bEditMode = FALSE;
	pBookmarkWidget->bPressed = FALSE;

	if (!_vp_play_bookmark_init_layout(pBookmarkWidget)) {
		VideoLogError("_vp_play_bookmark_init_layout is fail");
		VP_FREE(pBookmarkWidget);
		return NULL;
	}

	return (bookmark_handle) pBookmarkWidget;

}


void vp_play_bookmark_destroy(bookmark_handle pWidgetHandle)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return;
	}

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pWidgetHandle;

	_vp_play_bookmark_destory_handle(pBookmarkWidget);

}


bool vp_play_bookmark_realize(bookmark_handle pWidgetHandle)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pWidgetHandle;

	if (pBookmarkWidget->szMediaURL == NULL) {
		VideoLogError("szMediaURL is NULL");
		return FALSE;
	}

	if (pBookmarkWidget->szMediaID == NULL) {
		VideoLogError("szMediaID is NULL");
		return FALSE;
	}

	/* load gengrid items */
	GList *pList = NULL;
	int nCnt = 0;
	int idx = 0;
	if (!vp_media_contents_bookmark_list_get
	        (pBookmarkWidget->szMediaID, &pList)) {
		VideoLogError("vp_media_contents_bookmark_list_get is Fail");
		return FALSE;
	}

	nCnt = g_list_length(pList);
	for (idx = 0; idx < nCnt; idx++) {
		void *pItem = NULL;
		pItem = g_list_nth_data(pList, idx);
		VideoLogWarning("Item : %p", pItem);
		if (pItem) {
			char *szPath = NULL;
			int nPos = 0;
			if (!vp_media_contents_bookmark_get_item_info
			        (pItem, &szPath, &nPos)) {
				VideoLogWarning
				("vp_media_contents_bookmark_get_item_info is Fail");
				continue;
			}

			BookmarkItem *pBookmarkItem = NULL;
			pBookmarkItem = calloc(1, sizeof(BookmarkItem));
			if (pBookmarkItem == NULL) {
				VideoLogWarning("pBookmarkItem alloc fail");
				VP_FREE(szPath);
				continue;
			}
			pBookmarkItem->pElmItem = NULL;
			pBookmarkItem->nPos = nPos;
			VP_STRDUP(pBookmarkItem->szFilePath, szPath);
			VP_FREE(szPath);

			_vp_play_bookmark_gengrid_item_append(pBookmarkWidget,
			                                      pBookmarkItem);
		}
	}

	if (!vp_media_contents_bookmark_list_clear(pList)) {
		VideoLogWarning("vp_media_contents_bookmark_list_clear is fail");
	}

	pList = NULL;

	evas_object_show(pBookmarkWidget->pLayout);

	pBookmarkWidget->bIsRealize = TRUE;
	pBookmarkWidget->bEditMode = FALSE;
	pBookmarkWidget->bPressed = FALSE;

	return TRUE;
}

bool vp_play_bookmark_unrealize(bookmark_handle pWidgetHandle)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pWidgetHandle;

	_vp_play_bookmark_gengrid_item_clear(pBookmarkWidget);
	if (pBookmarkWidget->pGengrid) {
		elm_gengrid_clear(pBookmarkWidget->pGengrid);
	}

	evas_object_hide(pBookmarkWidget->pLayout);

	pBookmarkWidget->bIsRealize = FALSE;

	return TRUE;
}

bool vp_play_bookmark_is_realize(bookmark_handle pWidgetHandle,
                                 bool *bIsRealize)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pWidgetHandle;

	*bIsRealize = pBookmarkWidget->bIsRealize;

	return TRUE;
}

Evas_Object *vp_play_bookmark_get_object(bookmark_handle pWidgetHandle)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return NULL;
	}

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pWidgetHandle;

	return pBookmarkWidget->pLayout;

}

bool vp_play_bookmark_set_capture_callback(bookmark_handle pWidgetHandle,
        BookmarkCaptureStartCbFunc
        func)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pWidgetHandle;

	pBookmarkWidget->pCaptureCb = func;

	return TRUE;
}

bool vp_play_bookmark_set_item_select_callback(bookmark_handle
        pWidgetHandle,
        BookmarkItemSelectCbFunc
        func)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pWidgetHandle;

	pBookmarkWidget->pSelectCb = func;

	return TRUE;
}

bool vp_play_bookmark_set_user_param(bookmark_handle pWidgetHandle,
                                     void *pUserData)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pWidgetHandle;

	pBookmarkWidget->pUserData = pUserData;

	return TRUE;
}

bool vp_play_bookmark_set_media_url(bookmark_handle pWidgetHandle,
                                    const char *szMediaURL)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	if (szMediaURL == NULL) {
		VideoLogError("szMediaURL is NULL");
		return FALSE;
	}
	VideoLogError("IN");

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pWidgetHandle;

	VP_STRDUP(pBookmarkWidget->szMediaURL, szMediaURL);

	if (!vp_media_contents_get_video_id
	        (szMediaURL, &(pBookmarkWidget->szMediaID))) {
		VideoLogError("vp_media_contents_get_video_id is fail");
		return FALSE;
	}
	VideoLogError("OUT");

	return TRUE;

}

bool vp_play_bookmark_set_edit_mode(bookmark_handle pWidgetHandle,
                                    bool bEditMode)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pWidgetHandle;

	pBookmarkWidget->bEditMode = bEditMode;

	_vp_play_bookmark_gengrid_item_update(pBookmarkWidget);

	return TRUE;

}

bool vp_play_bookmark_get_edit_mode(bookmark_handle pWidgetHandle,
                                    bool *bEditMode)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pWidgetHandle;

	*bEditMode = pBookmarkWidget->bEditMode;

	return TRUE;
}

bool vp_play_bookmark_get_pressed_status(bookmark_handle pWidgetHandle,
        bool *bPressed)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pWidgetHandle;

	*bPressed = pBookmarkWidget->bPressed;

	return TRUE;

}

bool vp_play_bookmark_insert_item(bookmark_handle pWidgetHandle,
                                  const char *szBookmarkURL, int nPos)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pWidgetHandle;

	if (!vp_media_contents_bookmark_insert
	        (pBookmarkWidget->szMediaID, nPos, szBookmarkURL)) {
		VideoLogError("vp_media_contents_bookmark_insert is fail");
		return FALSE;
	}

	BookmarkItem *pBookmarkItem = NULL;
	pBookmarkItem = calloc(1, sizeof(BookmarkItem));
	if (pBookmarkItem == NULL) {
		VideoLogWarning("pBookmarkItem alloc fail");
		return FALSE;
	}
	pBookmarkItem->pElmItem = NULL;
	pBookmarkItem->nPos = nPos;
	VP_STRDUP(pBookmarkItem->szFilePath, szBookmarkURL);

	_vp_play_bookmark_gengrid_item_append(pBookmarkWidget, pBookmarkItem);

	return TRUE;
}

bool vp_play_bookmark_get_item_count(bookmark_handle pWidgetHandle,
                                     int *nCount)
{
	if (pWidgetHandle == NULL) {
		VideoLogError("pWidgetHandle is NULL");
		return FALSE;
	}

	BookmarkWidget *pBookmarkWidget = (BookmarkWidget *) pWidgetHandle;

	if (pBookmarkWidget->pItemList == NULL) {
		*nCount = 0;
	} else {
		*nCount = g_list_length(pBookmarkWidget->pItemList);
	}

	return TRUE;
}
