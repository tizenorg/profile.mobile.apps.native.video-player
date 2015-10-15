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
#include <glib.h>


bool vp_multi_path_add_item(GList **pList, const char *szURL, char *szTitle, char *szSubTitle, int nPosition, int nDuration, bool bIsSameAP);
bool vp_multi_path_clear_item(GList *pList);
bool vp_multi_path_set_item_subtitle(const char *szMediaURL, char *szSubtitleURL, GList *pList);
bool vp_multi_path_set_item_position(const char *szMediaURL, int nPosition, GList *pList);
bool vp_multi_path_get_current_item(const char *szMediaURL, char **szURL, char **szTitle, char **szSubtitle, int *nPosition, int *nDuration, bool *bIsSameAP, GList *pList);
bool vp_multi_path_get_next_item(const char *szMediaURL, char **szURL, char **szSubtitle, int *nPosition, int *nDuration, bool bLoop, GList *pList);
bool vp_multi_path_get_prev_item(const char *szMediaURL, char **szURL, char **szSubtitle, int *nPosition, int *nDuration, bool bLoop, GList *pList);

