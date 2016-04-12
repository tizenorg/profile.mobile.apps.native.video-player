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

#include <sound_manager.h>
#include "vp-play-macro-define.h"
#include "vp-play-normal-view.h"
#include "vp-play-view.h"
#include "vp-sound.h"

/* check temp */
#include "vp-play-log.h"

/* callback functions */

/* internal functions */

/* external functions */

void vp_player_focus_callback(sound_stream_info_h stream_info, sound_stream_focus_change_reason_e reason_for_change,
					const char *additional_info, void *user_data)
{
	PlayView *pPlayView = (PlayView *)user_data;
	sound_stream_focus_state_e state_for_playback;
	sound_stream_focus_state_e state_for_recording;
	int ret = -1;
	ret = sound_manager_get_focus_state(pPlayView->stream_info, &state_for_playback,
										&state_for_recording);
	if(ret != SOUND_MANAGER_ERROR_NONE){
		VideoLogError("sound_manager_get_focus_state fail");
		return;
	}
	if (state_for_playback == SOUND_STREAM_FOCUS_STATE_RELEASED) {
		if (reason_for_change != SOUND_STREAM_FOCUS_CHANGED_BY_ALARM &&
				reason_for_change != SOUND_STREAM_FOCUS_CHANGED_BY_NOTIFICATION) {
			sound_manager_get_focus_reacquisition(pPlayView->stream_info, &pPlayView->reacquire_state);
			if (pPlayView->reacquire_state == EINA_TRUE) {
				sound_manager_set_focus_reacquisition(pPlayView->stream_info, EINA_FALSE);
			}
		}
	} else {
		vp_play_normal_view_play_player(pPlayView);
	}
}

bool vp_sound_init_session(play_view_handle pViewHandle)
{
	VideoLogInfo(">> Sound Session : Init");

        PlayView *pPlayView = (PlayView *)pViewHandle;
	int nRet = SOUND_MANAGER_ERROR_NONE;
	if (!pPlayView->stream_info) {
		nRet = sound_manager_create_stream_information(SOUND_STREAM_TYPE_MEDIA, vp_player_focus_callback, pPlayView, &pPlayView->stream_info);

		if (nRet != SOUND_MANAGER_ERROR_NONE) {
			VideoLogError("failed to create_stream_information %x", nRet);
			return FALSE;
		}
	}

	nRet = sound_manager_set_current_sound_type(SOUND_TYPE_MEDIA);
	if (nRet != SOUND_MANAGER_ERROR_NONE) {
		VideoLogError("sound_manager_set_current_sound_type fail");
		return FALSE;
	}
//      nRet = sound_manager_set_safety_volume(SOUND_SAFETY_VOL_TYPE_APP_VIDEO);
	if (nRet != SOUND_MANAGER_ERROR_NONE) {
		VideoLogError("sound_manager_set_safety_volume fail");
		return FALSE;
	}

	VideoLogInfo("<< Sound Session : Init");

	return TRUE;
}

bool vp_sound_deinit_session(play_view_handle pViewHandle)
{
	int nRet = SOUND_MANAGER_ERROR_NONE;
        PlayView *pPlayView = (PlayView *)pViewHandle;
//      nRet = sound_manager_unset_safety_volume();
//      if (nRet != SOUND_MANAGER_ERROR_NONE) {
//              VideoLogError("sound_manager_unset_safety_volume fail");
//              return FALSE;
//      }

	nRet = sound_manager_destroy_stream_information(pPlayView->stream_info);
	if (nRet != SOUND_MANAGER_ERROR_NONE) {
		VideoLogError("sound_manager_unset_current_sound_type fail");
		return FALSE;
	}
//      sound_manager_unset_available_route_changed_cb();
	sound_manager_unset_volume_changed_cb();
	/*
	   nRet = sound_manager_set_session_type(SOUND_SESSION_TYPE_SHARE);
	   if (nRet != SOUND_MANAGER_ERROR_NONE) {
	   VideoLogError("sound_manager_set_session_type fail");
	   return FALSE;
	   }
	 */
	return TRUE;
}

bool vp_sound_get_volume(int *nValue)
{
	int nRet = SOUND_MANAGER_ERROR_NONE;
	int nVal = 0;
	nRet = sound_manager_get_volume(SOUND_TYPE_MEDIA, &nVal);
	if (nRet != SOUND_MANAGER_ERROR_NONE) {
		VideoLogError("sound_manager_set_session_type fail");
		return FALSE;
	}

	*nValue = nVal;

	return TRUE;
}

bool vp_sound_set_volume(int nValue)
{
	int nRet = SOUND_MANAGER_ERROR_NONE;

	nRet = sound_manager_set_volume(SOUND_TYPE_MEDIA, nValue);
	if (nRet != SOUND_MANAGER_ERROR_NONE) {
		VideoLogError("sound_manager_set_volume fail");
		return FALSE;
	}
	return TRUE;
}

bool vp_sound_get_max_volume(int *nMaxValue)
{
	int nRet = SOUND_MANAGER_ERROR_NONE;
	int nVal = 0;
	nRet = sound_manager_get_max_volume(SOUND_TYPE_MEDIA, &nVal);
	if (nRet != SOUND_MANAGER_ERROR_NONE) {
		VideoLogError("sound_manager_set_session_type fail");
		return FALSE;
	}

	*nMaxValue = nVal;

	return TRUE;
}

bool vp_sound_get_active_device(video_sound_device_type_t *nDeviceType)
{
	sound_device_h device;
	sound_device_type_e type = SOUND_DEVICE_BUILTIN_SPEAKER;
	sound_device_list_h g_device_list = NULL;
	sound_device_mask_e g_device_mask =
	    SOUND_DEVICE_IO_DIRECTION_OUT_MASK;
//              WARN_TRACE("Enter sound_manager_get_active_device");
	int ret;
	if ((ret =
	            sound_manager_get_current_device_list(g_device_mask,
	                    &g_device_list)))
//                      ERROR_TRACE("sound_manager_get_active_device()... [0x%x]", ret);


		if (!
		        (ret =
		             sound_manager_get_next_device(g_device_list, &device))) {
//                   ERROR_TRACE("success to get next device\n");
			if ((ret = sound_manager_get_device_type(device, &type))) {
				return FALSE;
			}
		}


	switch (type) {
	case SOUND_DEVICE_BUILTIN_SPEAKER:
		*nDeviceType = VP_SOUND_DEVICE_SPEAKER;
		break;

	case SOUND_DEVICE_AUDIO_JACK:
		*nDeviceType = VP_SOUND_DEVICE_EARJACK;
		break;

	case SOUND_DEVICE_BLUETOOTH:
		*nDeviceType = VP_SOUND_DEVICE_BLUETOOTH;
		break;

	case SOUND_DEVICE_HDMI:
		*nDeviceType = VP_SOUND_DEVICE_HDMI;
		break;

	case SOUND_DEVICE_MIRRORING:
		*nDeviceType = VP_SOUND_DEVICE_MIRRORING;
		break;

	case SOUND_DEVICE_USB_AUDIO:
		*nDeviceType = VP_SOUND_DEVICE_USB_AUDIO;
		break;

	default:
		*nDeviceType = VP_SOUND_DEVICE_SPEAKER;
		break;
	}

	return TRUE;
}


bool vp_sound_is_sound_path_enable(bool *bIsSoundPath)
{
	bool bConnected = FALSE;

	int nPathCount = 1;		//speaker or headphone

	sound_device_h device;
	sound_device_type_e type;
	sound_device_list_h g_device_list = NULL;
	sound_device_mask_e g_device_mask =
	    SOUND_DEVICE_IO_DIRECTION_OUT_MASK;

	int ret;
	if ((ret =
	            sound_manager_get_current_device_list(g_device_mask,
	                    &g_device_list))) {
		return bConnected;
	}

	while (!(ret = sound_manager_get_next_device(g_device_list, &device))) {
		if ((ret = sound_manager_get_device_type(device, &type))) {
			nPathCount++;
			goto COUNT_CHECK;
		}
	}
//      sound_manager_is_route_available(NULL, NULL, &bRouteAvailable);
//      if (bRouteAvailable)
//      {
//              nPathCount ++;
//              goto COUNT_CHECK;
//      }
//      sound_manager_is_route_available(NULL, NULL, &bRouteAvailable);
//      if (bRouteAvailable)
//      {
//              nPathCount ++;
//              goto COUNT_CHECK;
//      }
//
//      sound_manager_is_route_available(NULL, NULL, &bRouteAvailable);
//      if (bRouteAvailable)
//      {
//              nPathCount ++;
//              goto COUNT_CHECK;
//      }
//      sound_manager_is_route_available(NULL, NULL, &bRouteAvailable);
//      if (bRouteAvailable)
//      {
//              nPathCount ++;
//              goto COUNT_CHECK;
//      }
//      sound_manager_is_route_available(NULL, NULL, &bRouteAvailable);
//      if (bRouteAvailable)
//      {
//              nPathCount ++;
//              goto COUNT_CHECK;
//      }

COUNT_CHECK:
	if (nPathCount > 1) {
		bConnected = TRUE;
	}

	*bIsSoundPath = bConnected;

	return bConnected;
}

sound_device_connected_cb vp_audio_jack_connected_cb(sound_device_h
                                device,
                                bool is_connected,
                                void *user_data)
{
	if (!user_data) {
		return NULL;
	}
	sound_device_type_e type;
	void *pPlayview = NULL;
	pPlayview = user_data;
	if (sound_manager_get_device_type(device, &type) ==
	        SOUND_MANAGER_ERROR_NONE) {
		if (type == SOUND_DEVICE_AUDIO_JACK) {
			vp_play_normal_view_show_volume_popup(pPlayview);
			if (!is_connected) {
				vp_play_normal_view_pause_player(pPlayview);
			}
		}
	}
	return NULL;
}

bool vp_sound_device_is_enable(video_sound_device_type_t nDeviceType,
                               void *pUserdata)
{
	if (!pUserdata) {
		return FALSE;
	}
	void *pPlayview = NULL;
	pPlayview = pUserdata;
	sound_device_h device;
	sound_device_type_e type;
	sound_device_list_h g_device_list = NULL;
	sound_device_mask_e g_device_mask =
	    SOUND_DEVICE_IO_DIRECTION_OUT_MASK;

	if (sound_manager_get_current_device_list
	        (g_device_mask, &g_device_list) != SOUND_MANAGER_ERROR_NONE) {
		return FALSE;
	}

	sound_manager_set_device_connected_cb
	(SOUND_DEVICE_ALL_MASK,
	 (sound_device_connected_cb) vp_audio_jack_connected_cb,
	 (void *) pPlayview);

	while (sound_manager_get_next_device(g_device_list, &device) ==
	        SOUND_MANAGER_ERROR_NONE) {
		if (sound_manager_get_device_type(device, &type) ==
		        SOUND_MANAGER_ERROR_NONE) {
			if (type == SOUND_DEVICE_AUDIO_JACK) {
				return TRUE;
			}
		}
	}

//      switch (nDeviceType)
//      {
//      case VP_SOUND_DEVICE_SPEAKER:
//              sound_manager_is_route_available(NULL, NULL, &bRouteAvailable); return bRouteAvailable;
//
//      case VP_SOUND_DEVICE_EARJACK:
//              sound_manager_is_route_available(NULL, NULL, &bRouteAvailable); return bRouteAvailable;
//
//      case VP_SOUND_DEVICE_BLUETOOTH:
//              sound_manager_is_route_available(NULL, NULL, &bRouteAvailable); return bRouteAvailable;
//
//      case VP_SOUND_DEVICE_HDMI:
//              sound_manager_is_route_available(NULL, NULL, &bRouteAvailable); return bRouteAvailable;
//
//      case VP_SOUND_DEVICE_MIRRORING:
//              sound_manager_is_route_available(NULL, NULL, &bRouteAvailable); return bRouteAvailable;
//
//      case VP_SOUND_DEVICE_USB_AUDIO:
//              sound_manager_is_route_available(NULL, NULL, &bRouteAvailable); return bRouteAvailable;
//
//      default:
//              sound_manager_is_route_available(NULL, NULL, &bRouteAvailable); return bRouteAvailable;
//      }

	return FALSE;

}

bool vp_sound_get_a2dp_name(char **szBTName)
{
//      int nRet = SOUND_MANAGER_ERROR_NONE;
	bool bIsConnect = FALSE;
//      nRet = sound_manager_get_a2dp_status (&bIsConnect, szBTName);
//      if (nRet != SOUND_MANAGER_ERROR_NONE) {
//              VideoLogError("sound_manager_get_a2dp_status fail");
//              VP_FREE(*szBTName);
//              return FALSE;
//      }

	if (bIsConnect == FALSE) {
		VideoLogError("Not connected BT");
		VP_FREE(*szBTName);
	}

	return FALSE;
//      return TRUE;
}

bool vp_sound_set_active_route(video_sound_device_type_t nDeviceType)
{
	//int nRet = SOUND_MANAGER_ERROR_NONE;

	if (nDeviceType == VP_SOUND_DEVICE_NONE) {
		VideoLogError("Invalid device type");
		return FALSE;
	}
//      else if (nDeviceType == VP_SOUND_DEVICE_SPEAKER) {
//              nRet = sound_manager_set_active_route(NULL, NULL);
//              if (nRet != SOUND_MANAGER_ERROR_NONE) {
//                      VideoLogError("sound_manager_set_active_route NULL fail");
//                      return FALSE;
//              }
//      }
//      else if (nDeviceType == VP_SOUND_DEVICE_EARJACK) {
//              nRet = sound_manager_set_active_route(NULL, NULL);
//              if (nRet != SOUND_MANAGER_ERROR_NONE) {
//                      VideoLogError("sound_manager_set_active_route SOUND_ROUTE_OUT_WIRED_ACCESSORY fail");
//                      return FALSE;
//              }
//      }
//      else if (nDeviceType == VP_SOUND_DEVICE_BLUETOOTH) {
//              nRet = sound_manager_set_active_route(NULL, NULL);
//              if (nRet != SOUND_MANAGER_ERROR_NONE) {
//                      VideoLogError("sound_manager_set_active_route NULL fail");
//                      return FALSE;
//              }
//      }
//
//      else if (nDeviceType == VP_SOUND_DEVICE_HDMI) {
//              nRet = sound_manager_set_active_route(NULL, NULL);
//              if (nRet != SOUND_MANAGER_ERROR_NONE) {
//                      VideoLogError("sound_manager_set_active_route NULL fail");
//                      return FALSE;
//              }
//      }
//      else if (nDeviceType == VP_SOUND_DEVICE_MIRRORING) {
//              nRet = sound_manager_set_active_route(NULL, NULL);
//              if (nRet != SOUND_MANAGER_ERROR_NONE) {
//                      VideoLogError("sound_manager_set_active_route SOUND_ROUTE_OUT_MIRRORING fail");
//                      return FALSE;
//              }
//      }
//      else if (nDeviceType == VP_SOUND_DEVICE_USB_AUDIO) {
//              nRet = sound_manager_set_active_route(NULL, NULL);
//              if (nRet != SOUND_MANAGER_ERROR_NONE) {
//                      VideoLogError("sound_manager_set_active_route SOUND_ROUTE_OUT_USB_AUDIO fail");
//                      return FALSE;
//              }
//      }

	return TRUE;
}

bool vp_sound_set_route_change_cb(vp_sound_route_change_cb func,
                                  void *pUserData)
{
	int nRet = SOUND_MANAGER_ERROR_NONE;

	//nRet = sound_manager_set_available_route_changed_cb((sound_available_route_changed_cb), pUserData);

	sound_device_mask_e g_device_mask =
	    SOUND_DEVICE_IO_DIRECTION_OUT_MASK;

	nRet =
	    sound_manager_set_device_information_changed_cb(g_device_mask,
	            NULL, pUserData);
	if (nRet != SOUND_MANAGER_ERROR_NONE) {
		VideoLogError
		("sound_manager_set_available_route_changed_cb fail");
		return FALSE;
	}
	return TRUE;
}

bool vp_sound_set_volume_change_cb(vp_sound_volume_change_cb func,
                                   void *pUserData)
{
	int nRet = SOUND_MANAGER_ERROR_NONE;

	nRet =
	    sound_manager_set_volume_changed_cb((sound_manager_volume_changed_cb) func, pUserData);
	if (nRet != SOUND_MANAGER_ERROR_NONE) {
		VideoLogError("sound_manager_set_volume_changed_cb fail");
		return FALSE;
	}
	return TRUE;
}
