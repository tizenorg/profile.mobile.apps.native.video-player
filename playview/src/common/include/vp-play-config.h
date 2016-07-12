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
#include <system_settings.h>
#include <runtime_info.h>
#include <network/wifi.h>


typedef void (*KeyChangedCbFunc)(void *pNode, void* pUserData);


bool vp_play_config_get_rotate_lock_status(bool *bLocked);
bool vp_play_config_get_subtitle_show_key(bool *bShow);
bool vp_play_config_set_subtitle_show_key(bool bShow);
bool vp_play_config_set_subtitle_font_name_key(const char *szFontName);
bool vp_play_config_get_subtitle_font_name_key(char **szFontName);
bool vp_play_config_get_subtitle_size_key(int *nSize);
bool vp_play_config_set_subtitle_size_key(int nSize);
bool vp_play_config_set_subtitle_edge_key(int nEdge);
bool vp_play_config_get_subtitle_edge_key(int *nEdge);

bool vp_play_config_get_subtitle_font_color_key(int *nColor);
bool vp_play_config_set_subtitle_font_color_key(int nColor);
bool vp_play_config_get_subtitle_bg_color_key(int *nColor);
bool vp_play_config_set_subtitle_bg_color_key(int nColor);
bool vp_play_config_set_subtitle_alignment_key(int nAlignment);
bool vp_play_config_get_subtitle_alignment_key(int *nAlignment);
bool vp_play_config_get_subtitle_font_color_hex_key(char **szColorHex);
bool vp_play_config_set_subtitle_font_color_hex_key(const char *szColorHex);
bool vp_play_config_set_subtitle_bg_color_hex_key(const char *szColorHex);
bool vp_play_config_get_subtitle_bg_color_hex_key(char **szColorHex);
bool vp_play_config_set_subtitle_caption_win_color_hex_key(const char *szColorHex);
bool vp_play_config_get_subtitle_caption_win_color_hex_key(char **szColorHex);

bool vp_play_config_get_repeat_mode_key(int *nMode);
bool vp_play_config_set_repeat_mode_key(int nMode);
bool vp_play_config_set_screen_mode_key(int nMode);
bool vp_play_config_get_screen_mode_key(int *nMode);
bool vp_play_config_get_capture_on_key(bool *bEnable);
bool vp_play_config_set_capture_on_key(bool bEnable);
bool vp_play_config_get_tag_active_key(bool *bEnable);
bool vp_play_config_set_tag_active_key(bool bEnable);
bool vp_play_config_get_tag_weather_key(bool *bEnable);
bool vp_play_config_set_tag_weather_key(bool bEnable);
bool vp_play_config_get_tag_location_key(bool *bEnable);
bool vp_play_config_set_tag_location_key(bool bEnable);
bool vp_play_config_get_tag_edit_weather_key(int *nWeather);
bool vp_play_config_set_tag_edit_weather_key(int nWeather);

bool vp_play_config_get_sort_type_key(int *nType);
bool vp_play_config_set_preview_url_videos(const char *szMediaURL);
bool vp_play_config_get_preview_url_videos(char **szMediaURL);
bool vp_play_config_set_preview_url_records(const char *szMediaURL);
bool vp_play_config_get_preview_url_records(char **szMediaURL);
bool vp_play_config_set_preview_audio_track(int nAudioTrack);
bool vp_play_config_get_preview_audio_track(int *nAudioTrack);

bool vp_play_telephony_initialize(void *pUserData);
bool vp_play_telephony_deinitialize(void);
bool vp_play_config_get_call_state(bool *bCallOn);
bool vp_play_config_get_battery_charge_state(bool *bCharge);
bool vp_play_config_get_wifi_direct_connect_status(bool *bConnect);
bool vp_play_config_get_wifi_direct_active_status(bool *bConnect);
bool vp_play_config_get_lcd_off_state(bool *bLCDOff);

bool vp_play_config_set_rotate_state_key_changed_cb(system_settings_changed_cb pFunc, void *pUserData);
bool vp_play_config_ignore_rotate_state_key_changed_cb(void);
bool vp_play_config_set_battery_charge_state_key_changed_cb(runtime_info_changed_cb pFunc, void *pUserData);
bool vp_play_config_ignore_battery_charge_state_key_changed_cb(void);
bool vp_play_wifi_initialize();
bool vp_play_config_set_wifi_state_key_changed_cb(wifi_connection_state_changed_cb pFunc, void *pUserData);
bool vp_play_config_ignore_wifi_state_key_changed_cb(void);
void vp_play_wifi_deinitialize();

bool vp_play_config_get_allow_dock_connect_key(bool *bEnable);
bool vp_play_config_get_extern_mode_key(bool *bEnable);
bool vp_play_config_set_allow_dock_connect_key(bool bEnable);
bool vp_play_config_set_extern_mode_key(bool bEnable);

bool vp_play_config_set_multi_play_status(bool bMultiPlay);
bool vp_play_config_get_multi_play_status(bool *bMultiPlay);
bool vp_play_config_set_mirroring_warning_status(bool bMultiPlay);
bool vp_play_config_get_mirroring_warning_status(bool *bMultiPlay);

bool vp_play_config_get_sound_alive_status(int *nStatus);
bool vp_play_config_set_sound_alive_status(int nStatus);
