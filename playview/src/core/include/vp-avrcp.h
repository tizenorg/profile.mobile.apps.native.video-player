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
#include "vp-mm-player.h"


bool vp_avrcp_initialize();
bool vp_avrcp_deinitialize();
bool vp_avrcp_noti_track(const char *title, const char *artist, const char *album, const char *genre, unsigned int duration);
bool vp_avrcp_noti_track_position(unsigned int position);
bool vp_avrcp_noti_player_state(vp_mm_player_state_t nState);

