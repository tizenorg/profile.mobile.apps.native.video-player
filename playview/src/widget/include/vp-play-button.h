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

Evas_Object *vp_button_create(Evas_Object *pParent,const char *pStyle, const char *pTxt, Evas_Smart_Cb pClickFunc, Evas_Smart_Cb pPressFunc, Evas_Smart_Cb pUnpressFunc, void *pUserData);
Evas_Object *vp_button_create_icon(Evas_Object *pParent, const char *pEdjPath, const char *pGroupName);
Evas_Object *vp_button_create_image(Evas_Object *pParent, const char *pEdjPath, const char *pGroupName);
Evas_Object *vp_navirame_button_create(Evas_Object *pParent,const char *pStyle, const char *pEdjPath, const char *pGroupName, Evas_Smart_Cb pClickFunc, void *pUserData);

