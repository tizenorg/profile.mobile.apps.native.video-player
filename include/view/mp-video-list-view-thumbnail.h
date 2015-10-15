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


#ifndef _VIDEO_LIST_VIEW_THUMBNAIL_
#define  _VIDEO_LIST_VIEW_THUMBNAIL_

#include "mp-video-list-view-common.h"

bool mp_thumbnail_view_arrange_list_item(void* pMainViewHandle, void* pMainViewWidget, MpListUpdateType eUpdateType, UpdateListItemsCbFunc pUpdateVideoListUserCb);

int mp_thumbnail_view_get_selected_video_item_index(void);
void mp_thumbnail_view_update_genlist_item(int nVideoItemIndex);
void mp_thumbnail_view_update_played_item(void);
void mp_thumbnail_view_update_video_list(void);
int mp_thumbnail_view_thumbnail_to_genlist_index(int nVideoItemIndex);

int _grid_view_zoom_in(void *data, Evas_Object *view);
int _grid_view_zoom_out(void *data, Evas_Object *view);

void mp_thumbnail_view_change_language(void);
void mp_thumbnail_view_start_to_render(char *pVideoId);
void mp_thumbnail_view_free_wall_handle_list(void);
void mp_thumbnail_view_start_video_wall(bool *bSuccess);
void mp_thumbnail_view_pause_video_wall(void);
void mp_thumbnail_view_destroy(void);

#endif
