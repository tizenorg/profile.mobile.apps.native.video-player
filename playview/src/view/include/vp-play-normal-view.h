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


#include "vp-play-view-priv.h"

typedef enum
{
	VP_PLAY_NORMAL_VIEW_POPUP_NONE = 0,
	VP_PLAY_NORMAL_VIEW_CTX_POPUP,
	VP_PLAY_NORMAL_VIEW_OTHER_POPUP,
}vp_normalview_popup_style_e;

typedef void *normal_view_handle;

typedef void (*vp_normal_view_state_cb) (void *pUserData, int nState);

normal_view_handle vp_play_normal_view_create(PlayView *pPlayView, video_play_launching_type_t nLaunchingType);
void vp_play_normal_view_destroy(normal_view_handle pViewHandle);
bool vp_play_normal_view_realize(normal_view_handle pViewHandle);
bool vp_play_normal_view_unrealize(normal_view_handle pViewHandle);
bool vp_play_normal_view_is_realize(normal_view_handle pViewHandle, bool *bIsRealize);
bool vp_play_normal_view_pause(normal_view_handle pViewHandle);
bool vp_play_normal_view_resume(normal_view_handle pViewHandle);
bool vp_play_normal_view_resume_or_pause(normal_view_handle pViewHandle);
bool vp_play_normal_view_next_play(normal_view_handle pViewHandle);
bool vp_play_normal_view_prev_play(normal_view_handle pViewHandle);
bool vp_play_normal_view_set_url(normal_view_handle pViewHandle, const char *szMediaURL);
bool vp_play_normal_view_set_video_only(normal_view_handle pViewHandle, bool bVideoOnly);
bool vp_play_normal_view_get_video_only(normal_view_handle pViewHandle, bool *bVideoOnly);
bool vp_play_normal_view_set_device_unplug_interrupt(normal_view_handle pViewHandle, bool bDeviceUnplugInterrupt);
bool vp_play_normal_view_set_rotate(normal_view_handle pViewHandle, video_play_rotate_t nRotate, bool bLayoutHide);
bool vp_play_normal_view_set_launching_mode(normal_view_handle pViewHandle, video_play_launching_type_t nLaunchingType);
bool vp_play_normal_view_set_start_position(normal_view_handle pViewHandle, int nStartPosition);
bool vp_play_normal_view_get_last_position(normal_view_handle pViewHandle, int *nCurPos);
bool vp_play_normal_view_set_update_last_position(normal_view_handle pViewHandle);
bool vp_play_normal_view_set_subtitle(normal_view_handle pViewHandle, char *szSubtitle);
bool vp_play_normal_view_get_subtitle(normal_view_handle pViewHandle, char **szSubtitle);
bool vp_play_normal_view_get_audio_track(normal_view_handle pViewHandle, int *nAudioTrack);
bool vp_play_normal_view_get_video_duration(normal_view_handle pViewHandle, int *nDuration);
bool vp_play_normal_view_set_audio_track(normal_view_handle pViewHandle, int nAudioTrack);
bool vp_play_normal_view_set_manual_pause(normal_view_handle pViewHandle, bool bStartPause);
bool vp_play_normal_view_get_manual_pause(normal_view_handle pViewHandle, bool *bManualPause);
bool vp_play_normal_view_get_pause_status(normal_view_handle pViewHandle, bool *bPause);
bool vp_play_normal_view_get_sound_filter(normal_view_handle pViewHandle, video_sound_alive_t *nSoundAlive);
bool vp_play_normal_view_set_sound_filter(normal_view_handle pViewHandle, video_sound_alive_t nSoundAlive);
bool vp_play_normal_view_update(normal_view_handle pViewHandle);
bool vp_play_normal_view_rotate_update(normal_view_handle pViewHandle);
bool vp_play_normal_view_set_share_panel_state(normal_view_handle pViewHandle);
bool vp_play_normal_view_on_more_menu(normal_view_handle pViewHandle);
bool vp_play_normal_view_volume_increase(normal_view_handle pViewHandle);
bool vp_play_normal_view_volume_decrease(normal_view_handle pViewHandle);
bool vp_play_normal_view_volume_mute(normal_view_handle pViewHandle);
bool vp_play_normal_view_volume_update(normal_view_handle pViewHandle);
bool vp_play_normal_view_set_lock_screen(normal_view_handle pViewHandle, bool bLock);
bool vp_play_normal_view_get_lock_screen(normal_view_handle pViewHandle, bool *bLock);
void vp_play_normal_view_hide_sound_popup(normal_view_handle pViewHandle);
void vp_play_normal_view_web_type_disconnect(normal_view_handle pViewHandle);
void vp_play_normal_view_web_type_reconnect(normal_view_handle pViewHandle);
void vp_play_normal_view_ff_rew_keyboard_longpress_event_cb(normal_view_handle pViewHandle, bool bRelease, bool bFFseek);
bool vp_play_normal_view_get_volume_state(normal_view_handle *pViewHandle);
bool vp_play_normal_view_get_main_layout_show_state(normal_view_handle *pViewHandle);
vp_normalview_popup_style_e vp_play_normal_view_get_popup_show_state(normal_view_handle *pViewHandle);
bool vp_play_normal_view_set_tagbuddy_on(normal_view_handle pViewHandle, bool bTagBuddyOn);
void vp_play_normal_view_show_volume_popup(void *pPlayview);
void vp_play_normal_view_pause_player(void *pPlayview);
void vp_play_normal_view_key_create(PlayView *pPlayView, normal_view_handle pViewHandle);
bool vp_play_normal_view_player_state_changed_get(normal_view_handle pViewHandle);
bool vp_play_normal_view_stop_player(normal_view_handle pViewHandle);
bool vp_play_normal_view_play_start(normal_view_handle pViewHandle);
//Focus UI
void vp_play_normal_view_set_main_layout_focus_out(normal_view_handle *pViewHandle);
void vp_play_normal_view_set_main_layout_focus_in(normal_view_handle *pViewHandle);
bool vp_play_normal_view_get_main_layout_focus_state(normal_view_handle *pViewHandle);
void vp_play_normal_view_show_main_layout(normal_view_handle *pViewHandle);
void vp_play_normal_view_set_resume_or_pause(normal_view_handle *pViewHandle);
void vp_play_normal_view_create_main_layout_hide_timer(normal_view_handle *pViewHandle);


//for minicontroller
bool vp_controller_play_normal_view_pause(normal_view_handle pViewHandle);
bool vp_controller_play_normal_view_resume(normal_view_handle pViewHandle);
bool vp_controller_play_normal_view_next_play(normal_view_handle pViewHandle);
bool vp_controller_play_normal_view_prev_play(normal_view_handle pViewHandle);
char *vp_controller_normal_view_get_thumbnail(normal_view_handle pViewHandle);
char *vp_controller_normal_view_get_title(normal_view_handle pViewHandle);
int vp_controller_normal_view_get_duration(normal_view_handle pViewHandle);
bool vp_controller_normal_view_pop_view(normal_view_handle pViewHandle);
int vp_controller_normal_view_get_played_state(normal_view_handle pViewHandle);
bool vp_controller_normal_view_add_played_state_cb(normal_view_handle pViewHandle, vp_normal_view_state_cb fState_cb);

//db
void vp_play_normal_view_db_change(normal_view_handle pViewHandle);

