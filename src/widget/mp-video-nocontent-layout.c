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


#include <Elementary.h>

#include "mp-video-nocontent-layout.h"
#include "mp-video-log.h"
#include "mp-video-value-define.h"

void
mp_object_part_translate_set(Evas_Object *pObject, const char *ID,
			     const char *pPart)
{
	if (!pObject || !ID || !pPart) {
		VideoLogError("pObject || ID || pPart == NULL!!!");
		return;
	}

	const char *domain;
	domain = NULL;
	elm_object_domain_translatable_part_text_set(pObject, pPart, domain,
			ID);
}

Evas_Object *mp_create_nocontent_layout(const Evas_Object *pParent,
					const char *pText,
					const char *pHelpText)
{
	Evas_Object *pLayout = NULL;

	if (pParent == NULL) {
		VideoLogError("pParent == NULL!!!");
		VideoLogAssert(0);
		return NULL;
	}

	if (pText == NULL) {
		VideoLogError("pTextIds == NULL!!!");
		VideoLogAssert(0);
		return NULL;
	}

	VideoSecureLogInfo("pText : [%s], pHelpText : [%s]", pText,
			   pHelpText);

	pLayout = elm_layout_add((Evas_Object *)pParent);
	elm_layout_theme_set(pLayout, "layout", "nocontents", "text");

	evas_object_size_hint_weight_set(pLayout, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pLayout, EVAS_HINT_FILL,
					EVAS_HINT_FILL);

	//mp_object_part_translate_set(pLayout, pText, "elm.text");
	elm_object_part_text_set(pLayout, "elm.text", pText);

	if (pHelpText != NULL) {
		mp_object_part_translate_set(pLayout, pHelpText, "elm.help.text");
	}
	elm_layout_signal_emit(pLayout, "text,disabled", "");
	elm_layout_signal_emit(pLayout, "align.center", "elm");

	return pLayout;
}
