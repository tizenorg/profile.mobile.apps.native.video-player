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

#include "vp-play-view-priv.h"

typedef void *multi_view_handle;

multi_view_handle vp_play_multi_view_create(PlayView *pPlayView, video_play_launching_type_t nLaunchingType);
void vp_play_multi_view_destroy(multi_view_handle pViewHandle);
bool vp_play_multi_view_realize(multi_view_handle pViewHandle);
bool vp_play_multi_view_unrealize(multi_view_handle pViewHandle);
bool vp_play_multi_view_is_realize(multi_view_handle pViewHandle, bool *bIsRealize);
bool vp_play_multi_view_set_url(multi_view_handle pViewHandle, const char *szMediaURL);
bool vp_play_multi_view_pause(multi_view_handle pViewHandle);
bool vp_play_multi_view_resume(multi_view_handle pViewHandle);
bool vp_play_multi_view_resume_or_pause(multi_view_handle pViewHandle);
bool vp_play_multi_view_next_play(multi_view_handle pViewHandle);
bool vp_play_multi_view_prev_play(multi_view_handle pViewHandle);
bool vp_play_multi_view_set_start_position(multi_view_handle pViewHandle, int nStartPosition);
bool vp_play_multi_view_get_last_position(multi_view_handle pViewHandle, int *nCurPosition);
bool vp_play_multi_view_set_subtitle(multi_view_handle pViewHandle, char *szSubtitle);
bool vp_play_multi_view_get_subtitle(multi_view_handle pViewHandle, char **szSubtitle);
bool vp_play_multi_view_get_audio_track(multi_view_handle pViewHandle, int *nAudioTrack);
bool vp_play_multi_view_set_audio_track(multi_view_handle pViewHandle, int nAudioTrack);
bool vp_play_multi_view_set_manual_pause(multi_view_handle pViewHandle, bool bManualPause);
bool vp_play_multi_view_get_manual_pause(multi_view_handle pViewHandle, bool *bManualPause);
bool vp_play_multi_view_set_rotate(multi_view_handle pViewHandle, video_play_rotate_t nRotate);
bool vp_play_multi_view_set_launching_mode(multi_view_handle pViewHandle, video_play_launching_type_t nLaunchingType);
bool vp_play_multi_view_get_sound_filter(multi_view_handle pViewHandle, video_sound_alive_t *nSoundAlive);
bool vp_play_multi_view_set_sound_filter(multi_view_handle pViewHandle, video_sound_alive_t nSoundAlive);

bool vp_play_multi_view_update(multi_view_handle pViewHandle);
bool vp_play_multi_view_volume_increase(multi_view_handle pViewHandle);
bool vp_play_multi_view_volume_decrease(multi_view_handle pViewHandle);
void vp_play_multi_view_web_type_disconnect(multi_view_handle pViewHandle);
void vp_play_multi_view_web_type_reconnect(multi_view_handle pViewHandle);
void vp_play_multi_view_set_cancelkey_press(multi_view_handle pViewHandle, bool bCancelKeyPress);
void vp_play_multi_view_get_cancelkey_press(multi_view_handle pViewHandle, bool *bCancelKeyPress);
void vp_play_multi_view_destroy_exit_popup(multi_view_handle pViewHandle);

bool vp_play_multi_view_get_main_layout_show_state(multi_view_handle *pViewHandle);
bool vp_play_multi_view_get_main_layout_focus_state(multi_view_handle *pViewHandle);
void vp_play_multi_view_show_main_layout(multi_view_handle *pViewHandle);
void vp_play_multi_view_ff_rew_keyboard_longpress_event_cb(multi_view_handle pViewHandle, bool bRelease, bool bFFseek);

//Focus UI
void vp_play_multi_view_set_main_layout_focus_out(multi_view_handle *pViewHandle);
void vp_play_multi_view_set_main_layout_focus_in(multi_view_handle *pViewHandle);

bool vp_play_multi_view_change_to_normal_view(multi_view_handle pViewHandle);

