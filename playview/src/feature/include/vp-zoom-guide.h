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

typedef void *zoom_guide_handle;

zoom_guide_handle vp_zoom_guide_create(Evas_Object *pParent);
void vp_zoom_guide_destroy(zoom_guide_handle pWidgetHandle);
bool vp_zoom_guide_realize(zoom_guide_handle pWidgetHandle);
bool vp_zoom_guide_unrealize(zoom_guide_handle pWidgetHandle);
Evas_Object *vp_zoom_guide_get_object(zoom_guide_handle pWidgetHandle);
bool vp_zoom_guide_set_zoom_value(zoom_guide_handle pWidgetHandle, double fZoomVal);
bool vp_zoom_guide_get_zoom_value(zoom_guide_handle pWidgetHandle, double *fZoomVal);
bool vp_zoom_guide_set_move_position(zoom_guide_handle pWidgetHandle, double fPosX, double fPosY);
bool vp_zoom_guide_get_move_position(zoom_guide_handle pWidgetHandle, double *fPosX, double *fPosY);
bool vp_zoom_guide_get_real_position(zoom_guide_handle pWidgetHandle, double *fPosX, double *fPosY);
bool vp_zoom_guide_update(zoom_guide_handle pWidgetHandle);


