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

#include "vp-play-button.h"

/* check temp */
#include "vp-play-log.h"

Evas_Object *vp_button_create(Evas_Object *pParent, const char *pStyle,
                              const char *pTxt, Evas_Smart_Cb pClickFunc,
                              Evas_Smart_Cb pPressFunc,
                              Evas_Smart_Cb pUnpressFunc, void *pUserData)
{
	if (!pParent) {
		VideoLogError("pParent is NULL");
		return NULL;
	}
	Evas_Object *pObj = NULL;

	pObj = elm_button_add(pParent);

	if (!pObj) {
		VideoLogError("pObj is NULL");
		return NULL;
	}

	if (pStyle) {
		elm_object_style_set(pObj, pStyle);
	}

	if (pTxt) {
		elm_object_text_set(pObj, pTxt);
	}

	evas_object_size_hint_weight_set(pObj, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pObj, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_smart_callback_add(pObj, "clicked", pClickFunc,
	                               pUserData);
	evas_object_smart_callback_add(pObj, "pressed", pPressFunc,
	                               pUserData);
	evas_object_smart_callback_add(pObj, "unpressed", pUnpressFunc,
	                               pUserData);

	return pObj;
}

Evas_Object *vp_button_create_icon(Evas_Object *pParent,
                                   const char *pEdjPath,
                                   const char *pGroupName)
{
	if (!pParent) {
		VideoLogError("pParent is NULL");
		return NULL;
	}
	Evas_Object *pObj = NULL;

	pObj = elm_icon_add(pParent);
	if (!elm_image_file_set(pObj, pEdjPath, pGroupName)) {
		VideoLogError("elm_image_file_set is fail");
		return NULL;
	}
	evas_object_size_hint_weight_set(pObj, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pObj, EVAS_HINT_FILL, EVAS_HINT_FILL);

	return pObj;
}


Evas_Object *vp_button_create_image(Evas_Object *pParent,
                                    const char *pEdjPath,
                                    const char *pGroupName)
{
	if (!pParent) {
		VideoLogError("pParent is NULL");
		return NULL;
	}
	Evas_Object *pObj = NULL;

	pObj = elm_image_add(pParent);
	if (!elm_image_file_set(pObj, pEdjPath, pGroupName)) {
		VideoLogError("elm_image_file_set is fail");
		return NULL;
	}

	return pObj;
}

Evas_Object *vp_navirame_button_create(Evas_Object *pParent,
                                       const char *pStyle,
                                       const char *pEdjPath,
                                       const char *pGroupName,
                                       Evas_Smart_Cb pClickFunc,
                                       void *pUserData)
{
	if (!pParent) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	Evas_Object *pImage = NULL;
	Evas_Object *btn = elm_button_add(pParent);
	if (!btn) {
		return NULL;
	}

	if (pStyle) {
		elm_object_style_set(btn, pStyle);
	} else {
		elm_object_style_set(btn, "naviframe/title_icon");
	}

	pImage = elm_image_add(pParent);
	elm_image_file_set(pImage, pEdjPath, pGroupName);
	evas_object_size_hint_aspect_set(pImage, EVAS_ASPECT_CONTROL_BOTH, 1,
	                                 1);
	elm_image_resizable_set(pImage, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", pImage);

	evas_object_smart_callback_add(btn, "clicked", pClickFunc, pUserData);
	evas_object_show(btn);
	return btn;
}
