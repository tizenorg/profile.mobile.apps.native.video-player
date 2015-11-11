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

#include "vp-play-macro-define.h"

#include "vp-play-loading-ani.h"

/* check temp */
#include "vp-play-log.h"


Evas_Object *vp_play_loading_ani_create(Evas_Object *pParent,
                                        video_loading_size_t nSize)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	Evas_Object *progressbar = NULL;
	progressbar = elm_progressbar_add(pParent);
	if (nSize == VIDEO_LOADING_SIZE_SMALL) {
		elm_object_style_set(progressbar, "process_small");
	} else if (nSize == VIDEO_LOADING_SIZE_MIDUM) {
		elm_object_style_set(progressbar, "process_medium");
	} else if (nSize == VIDEO_LOADING_SIZE_LARGE) {
		elm_object_style_set(progressbar, "process_large");
	} else {
		elm_object_style_set(progressbar, "process_Xlarge");
	}
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	evas_object_show(progressbar);
	elm_progressbar_pulse(progressbar, EINA_TRUE);

	return progressbar;

}

void vp_play_loading_ani_destroy(Evas_Object *pProgress)
{
	if (pProgress == NULL) {
		VideoLogError("pProgress is NULL");
		return;
	}

	VP_EVAS_DEL(pProgress);
}
