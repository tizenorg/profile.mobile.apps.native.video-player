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
#include <app_preference.h>

#include "vp-play-preference.h"

/* check temp */
#include "vp-play-log.h"
#include "vp-play-type-define.h"


#define VP_PREFERENCE_KEY_GENERAL_BACKGROUND_PLAYING  		"preference/org.tizen.videos/backgroud_playing"
#define VP_PREFERENCE_KEY_SUBTILE_SHOW  				"preference/org.tizen.videos/show_subtitle"
#define VP_PREFERENCE_KEY_SUBTILE_SIZE  				"preference/org.tizen.videos/subtitle_size"
#define VP_PREFERENCE_KEY_SUBTILE_EDGE 				"preference/org.tizen.videos/subtitle_edge"

#define VP_PREFERENCE_KEY_SUBTILE_FONT  				"preference/org.tizen.videos/subtitle_font"
#define VP_PREFERENCE_KEY_SUBTILE_FONT_COLOR  			"preference/org.tizen.videos/subtitle_font_color"
#define VP_PREFERENCE_KEY_SUBTILE_FONT_COLOR_HEX				"preference/org.tizen.videos/subtitle_font_color_hex"
#define VP_PREFERENCE_KEY_SUBTILE_BG_COLOR  				"preference/org.tizen.videos/subtitle_bg_color"
#define VP_PREFERENCE_KEY_SUBTILE_BG_COLOR_HEX				"preference/org.tizen.videos/subtitle_bg_color_hex"
#define VP_PREFERENCE_KEY_SUBTILE_CAPTION_WIN_COLOR_HEX				"preference/org.tizen.videos/subtitle_caption_win_color_hex"
#define VP_PREFERENCE_KEY_SUBTILE_ALIGNMENT  			"preference/org.tizen.videos/subtitle_alignment"

//#define VP_PREFERENCE_KEY_DISPLAY_SHOW_INDICATOR                      "preference/org.tizen.videos/show_indicator"
//#define VP_PREFERENCE_KEY_DISPLAY_SHOW_BORDER                         "preference/org.tizen.videos/show_border"
#define VP_PREFERENCE_KEY_REPEAT_MODE		  		"preference/org.tizen.videos/repeat_mode"
#define VP_PREFERENCE_KEY_SCREEN_MODE		  		"preference/org.tizen.videos/screen_mode"
//#define VP_PREFERENCE_KEY_ZOOM_ENABLE                         "preference/setting/gallery/inline_video_state"
#define VP_PREFERENCE_KEY_PREVIEW_URL_VIDEOS    			"preference/org.tizen.videos/preview_url_videos"
#define VP_PREFERENCE_KEY_PREVIEW_AUDIO_TRACK			"preference/org.tizen.videos/preview_audio_track"

#define VP_PREFERENCE_KEY_SENSOR_ASKED 	  			"preference/org.tizen.videos/motion_asked"
#define VP_PREFERENCE_KEY_VIDEOS_SORT_TYPE  				"preference/org.tizen.videos/sort_type"
#define VP_PREFERENCE_KEY_CAPTURE_MODE_ON  				"preference/org.tizen.videos/capture_on"

#define VP_PREFERENCE_KEY_TAG_ACTIVE 				"preference/org.tizen.videos/tag_active"
#define VP_PREFERENCE_KEY_TAG_WEATHER  				"preference/org.tizen.videos/tag_weather"
#define VP_PREFERENCE_KEY_TAG_LOCATION  				"preference/org.tizen.videos/tag_location"
#define VP_PREFERENCE_KEY_TAG_EDIT_WEATHER  				"preference/org.tizen.videos/tag_edit_weather"

#define VP_PREFERENCE_KEY_ALLOW_DOCK_CONNECT 			"preference/org.tizen.videos/allow_dock_connect"
#define VP_PREFERENCE_KEY_EXTERN_MODE	 			"preference/org.tizen.videos/extern_mode"
#define VP_PREFERENCE_KEY_MULTI_PLAY_FLAG   	"preference/org.tizen.videos/multi_play"
#define VP_PREFERENCE_KEY_SCREEN_MIRRORING_WARNING 			"preference/org.tizen.videos/mirroring_warning"


/*for saving sound alive value*/
#define VP_PREFERENCE_KEY_SOUND_ALIVE				"preference/org.tizen.videos/sound_alive"

void vp_play_preference_init()
{
	VideoLogInfo("");

	int nErr = PREFERENCE_ERROR_NONE;
	bool exist;

	nErr =
		preference_is_existing
		(VP_PREFERENCE_KEY_GENERAL_BACKGROUND_PLAYING, &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_int
				(VP_PREFERENCE_KEY_GENERAL_BACKGROUND_PLAYING, 0);
			if (nErr != 0) {
				VideoLogError
				("VP_PREFERENCE_KEY_GENERAL_BACKGROUND_PLAYING is fail [%d]",
				 nErr);
			}
		}
	}

	nErr = preference_is_existing(VP_PREFERENCE_KEY_SUBTILE_SHOW, &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_boolean(VP_PREFERENCE_KEY_SUBTILE_SHOW, 1);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_SUBTILE_SHOW is fail [%d]",
				 nErr);
			}
		}
	}

	nErr = preference_is_existing(VP_PREFERENCE_KEY_SUBTILE_SIZE, &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr = preference_set_int(VP_PREFERENCE_KEY_SUBTILE_SIZE, 1);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_SUBTILE_SIZE is fail [%d]",
				 nErr);
			}
		}
	}

	nErr = preference_is_existing(VP_PREFERENCE_KEY_SUBTILE_EDGE, &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr = preference_set_int(VP_PREFERENCE_KEY_SUBTILE_EDGE, 1);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_SUBTILE_EDGE is fail [%d]",
				 nErr);
			}
		}
	}

	nErr = preference_is_existing(VP_PREFERENCE_KEY_SUBTILE_FONT, &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_string(VP_PREFERENCE_KEY_SUBTILE_FONT,
						      "Tizen");
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_SUBTILE_FONT is fail [%d]",
				 nErr);
			}
		}
	}

	nErr =
		preference_is_existing(VP_PREFERENCE_KEY_SUBTILE_FONT_COLOR,
				       &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_int(VP_PREFERENCE_KEY_SUBTILE_FONT_COLOR,
						   0);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_SUBTILE_FONT_COLOR is fail [%d]",
				 nErr);
			}
		}
	}

	nErr =
		preference_is_existing(VP_PREFERENCE_KEY_SUBTILE_FONT_COLOR_HEX,
				       &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_string
				(VP_PREFERENCE_KEY_SUBTILE_FONT_COLOR_HEX, "#ffffffff");
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_SUBTILE_FONT_COLOR_HEX is fail [%d]",
				 nErr);
			}
		}
	}

	nErr =
		preference_is_existing(VP_PREFERENCE_KEY_SUBTILE_BG_COLOR,
				       &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_int(VP_PREFERENCE_KEY_SUBTILE_BG_COLOR, 1);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_SUBTILE_BG_COLOR is fail [%d]",
				 nErr);
			}
		}
	}

	nErr =
		preference_is_existing(VP_PREFERENCE_KEY_SUBTILE_BG_COLOR_HEX,
				       &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_string
				(VP_PREFERENCE_KEY_SUBTILE_BG_COLOR_HEX, "#00000000");
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_SUBTILE_BG_COLOR_HEX is fail [%d]",
				 nErr);
			}
		}
	}

	nErr =
		preference_is_existing
		(VP_PREFERENCE_KEY_SUBTILE_CAPTION_WIN_COLOR_HEX, &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_string
				(VP_PREFERENCE_KEY_SUBTILE_CAPTION_WIN_COLOR_HEX,
				 "#00000000");
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_SUBTILE_CAPTION_WIN_COLOR_HEX is fail [%d]",
				 nErr);
			}
		}
	}

	nErr =
		preference_is_existing(VP_PREFERENCE_KEY_SUBTILE_ALIGNMENT,
				       &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_int(VP_PREFERENCE_KEY_SUBTILE_ALIGNMENT,
						   1);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_SUBTILE_ALIGNMENT is fail [%d]",
				 nErr);
			}
		}
	}

	nErr = preference_is_existing(VP_PREFERENCE_KEY_REPEAT_MODE, &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr = preference_set_int(VP_PREFERENCE_KEY_REPEAT_MODE, 1);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_REPEAT_MODE is fail [%d]",
				 nErr);
			}
		}
	}

	nErr = preference_is_existing(VP_PREFERENCE_KEY_SCREEN_MODE, &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr = preference_set_int(VP_PREFERENCE_KEY_SCREEN_MODE, 0);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_SCREEN_MODE is fail [%d]",
				 nErr);
			}
		}
	}

	nErr =
		preference_is_existing(VP_PREFERENCE_KEY_PREVIEW_URL_VIDEOS,
				       &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_string
				(VP_PREFERENCE_KEY_PREVIEW_URL_VIDEOS, "");
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_PREVIEW_URL_VIDEOS is fail [%d]",
				 nErr);
			}
		}
	}

	nErr =
		preference_is_existing(VP_PREFERENCE_KEY_PREVIEW_AUDIO_TRACK,
				       &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_int(VP_PREFERENCE_KEY_PREVIEW_AUDIO_TRACK,
						   0);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_PREVIEW_AUDIO_TRACK is fail [%d]",
				 nErr);
			}
		}
	}

	nErr = preference_is_existing(VP_PREFERENCE_KEY_SENSOR_ASKED, &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_boolean(VP_PREFERENCE_KEY_SENSOR_ASKED, 0);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_SENSOR_ASKED is fail [%d]",
				 nErr);
			}
		}
	}

	nErr =
		preference_is_existing(VP_PREFERENCE_KEY_VIDEOS_SORT_TYPE,
				       &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_int(VP_PREFERENCE_KEY_VIDEOS_SORT_TYPE, 0);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_VIDEOS_SORT_TYPE is fail [%d]",
				 nErr);
			}
		}
	}

	nErr =
		preference_is_existing(VP_PREFERENCE_KEY_CAPTURE_MODE_ON, &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_boolean(VP_PREFERENCE_KEY_CAPTURE_MODE_ON,
						       0);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_CAPTURE_MODE_ON is fail [%d]",
				 nErr);
			}
		}
	}

	nErr = preference_is_existing(VP_PREFERENCE_KEY_TAG_ACTIVE, &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_boolean(VP_PREFERENCE_KEY_TAG_ACTIVE, 1);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_TAG_ACTIVE is fail [%d]",
				 nErr);
			}
		}
	}

	nErr = preference_is_existing(VP_PREFERENCE_KEY_TAG_WEATHER, &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_boolean(VP_PREFERENCE_KEY_TAG_WEATHER, 1);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_TAG_WEATHER is fail [%d]",
				 nErr);
			}
		}
	}

	nErr = preference_is_existing(VP_PREFERENCE_KEY_TAG_LOCATION, &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_boolean(VP_PREFERENCE_KEY_TAG_LOCATION, 1);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_TAG_LOCATION is fail [%d]",
				 nErr);
			}
		}
	}

	nErr =
		preference_is_existing(VP_PREFERENCE_KEY_TAG_EDIT_WEATHER,
				       &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_int(VP_PREFERENCE_KEY_TAG_EDIT_WEATHER, 0);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_TAG_EDIT_WEATHER is fail [%d]",
				 nErr);
			}
		}
	}

	nErr =
		preference_is_existing(VP_PREFERENCE_KEY_ALLOW_DOCK_CONNECT,
				       &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_boolean
				(VP_PREFERENCE_KEY_ALLOW_DOCK_CONNECT, 1);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_ALLOW_DOCK_CONNECT is fail [%d]",
				 nErr);
			}
		}
	}

	nErr = preference_is_existing(VP_PREFERENCE_KEY_EXTERN_MODE, &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_boolean(VP_PREFERENCE_KEY_EXTERN_MODE, 0);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_EXTERN_MODE is fail [%d]",
				 nErr);
			}
		}
	}

	nErr =
		preference_is_existing(VP_PREFERENCE_KEY_MULTI_PLAY_FLAG, &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_boolean(VP_PREFERENCE_KEY_MULTI_PLAY_FLAG,
						       0);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_MULTI_PLAY_FLAG is fail [%d]",
				 nErr);
			}
		}
	}

	nErr =
		preference_is_existing(VP_PREFERENCE_KEY_SCREEN_MIRRORING_WARNING,
				       &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr =
				preference_set_boolean
				(VP_PREFERENCE_KEY_SCREEN_MIRRORING_WARNING, 1);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_SCREEN_MIRRORING_WARNING is fail [%d]",
				 nErr);
			}
		}
	}

	nErr = preference_is_existing(VP_PREFERENCE_KEY_SOUND_ALIVE, &exist);
	if (nErr != 0) {
		VideoLogError("preference_is_existing is fail [%d]", nErr);
	} else {
		if (!exist) {
			nErr = preference_set_int(VP_PREFERENCE_KEY_SOUND_ALIVE, 1);
			if (nErr != 0) {
				VideoLogError
				("SET VP_PREFERENCE_KEY_SOUND_ALIVE is fail [%d]",
				 nErr);
			}
		}
	}
}

bool vp_play_preference_set_multi_play_status(bool bMultiPlay)
{
	int nErr = 0;

	nErr =
		preference_set_boolean(VP_PREFERENCE_KEY_MULTI_PLAY_FLAG,
				       bMultiPlay);
	if (nErr != 0) {
		VideoLogError
		("SET VP_PREFERENCE_KEY_MULTI_PLAY_FLAG is fail [%d]", nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_get_multi_play_status(bool *bMultiPlay)
{
	int nErr = 0;
	bool nVal = 0;

	nErr =
		preference_get_boolean(VP_PREFERENCE_KEY_MULTI_PLAY_FLAG, &nVal);
	if (nErr != 0) {
		VideoLogError("GET preference_get_boolean is fail [%d]", nErr);
		return FALSE;
	}

	*bMultiPlay = (nVal == 0) ? FALSE : TRUE;

	return TRUE;
}

bool vp_play_preference_set_mirroring_warning_status(bool bMultiPlay)
{
	int nErr = 0;

	nErr =
		preference_set_boolean(VP_PREFERENCE_KEY_SCREEN_MIRRORING_WARNING,
				       bMultiPlay);
	if (nErr != 0) {
		VideoLogError
		("SET VP_PREFERENCE_KEY_SCREEN_MIRRORING_WARNING is fail [%d]",
		 nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_get_mirroring_warning_status(bool *bWarning)
{
	int nErr = 0;
	bool nVal = 0;

	nErr =
		preference_get_boolean(VP_PREFERENCE_KEY_SCREEN_MIRRORING_WARNING,
				       &nVal);
	if (nErr != 0) {
		VideoLogError
		("GET VP_PREFERENCE_KEY_SCREEN_MIRRORING_WARNING is fail [%d]",
		 nErr);
		return FALSE;
	}

	*bWarning = (nVal == 0) ? FALSE : TRUE;

	return TRUE;
}

bool vp_play_preference_get_subtitle_show_key(bool *bShow)
{
	int nErr = 0;
	bool nVal = 0;

	nErr = preference_get_boolean(VP_PREFERENCE_KEY_SUBTILE_SHOW, &nVal);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SUBTILE_SHOW is fail [%d]",
			      nErr);
		return FALSE;
	}

	*bShow = (nVal == 0) ? FALSE : TRUE;

	return TRUE;
}

bool vp_play_preference_set_subtitle_show_key(bool bShow)
{
	int nErr = 0;

	nErr = preference_set_boolean(VP_PREFERENCE_KEY_SUBTILE_SHOW, bShow);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SUBTILE_SHOW is fail [%d]",
			      nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_set_subtitle_font_name_key(const char *szFontName)
{
	int nErr = 0;

	nErr =
		preference_set_string(VP_PREFERENCE_KEY_SUBTILE_FONT, szFontName);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SUBTILE_FONT is fail [%d]",
			      nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_get_subtitle_font_name_key(char **szFontName)
{
	int nErr = 0;
	nErr =
		preference_get_string(VP_PREFERENCE_KEY_SUBTILE_FONT, szFontName);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SUBTILE_FONT is fail [%d]",
			      nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_get_subtitle_size_key(int *nSize)
{
	int nErr = 0;
	int nVal = 0;

	nErr = preference_get_int(VP_PREFERENCE_KEY_SUBTILE_SIZE, &nVal);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SUBTILE_SIZE is fail [%d]",
			      nErr);
		return FALSE;
	}

	*nSize = nVal;

	return TRUE;

}

bool vp_play_preference_set_subtitle_size_key(int nSize)
{
	int nErr = 0;

	nErr = preference_set_int(VP_PREFERENCE_KEY_SUBTILE_SIZE, nSize);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SUBTILE_SIZE is fail [%d]",
			      nErr);
		return FALSE;
	}

	return TRUE;
}


bool vp_play_preference_get_subtitle_edge_key(int *nEdge)
{
	int nErr = 0;
	int nVal = 0;

	nErr = preference_get_int(VP_PREFERENCE_KEY_SUBTILE_EDGE, &nVal);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SUBTILE_EDGE is fail [%d]",
			      nErr);
		return FALSE;
	}

	*nEdge = nVal;

	return TRUE;

}

bool vp_play_preference_set_subtitle_edge_key(int nEdge)
{
	int nErr = 0;

	nErr = preference_set_int(VP_PREFERENCE_KEY_SUBTILE_EDGE, nEdge);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SUBTILE_EDGE is fail [%d]",
			      nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_set_subtitle_alignment_key(int nAlignment)
{
	int nErr = 0;

	nErr =
		preference_set_int(VP_PREFERENCE_KEY_SUBTILE_ALIGNMENT,
				   nAlignment);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SUBTILE_SIZE is fail [%d]",
			      nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_get_subtitle_alignment_key(int *nAlignment)
{
	int nErr = 0;
	int nVal = 0;

	nErr = preference_get_int(VP_PREFERENCE_KEY_SUBTILE_ALIGNMENT, &nVal);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SUBTILE_SIZE is fail [%d]",
			      nErr);
		return FALSE;
	}

	*nAlignment = nVal;

	return TRUE;

}

bool vp_play_preference_set_subtitle_font_color_key(int nColor)
{
	int nErr = 0;

	nErr =
		preference_set_int(VP_PREFERENCE_KEY_SUBTILE_FONT_COLOR, nColor);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SUBTILE_SIZE is fail [%d]",
			      nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_get_subtitle_font_color_key(int *nColor)
{
	int nErr = 0;
	int nVal = 0;

	nErr =
		preference_get_int(VP_PREFERENCE_KEY_SUBTILE_FONT_COLOR, &nVal);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SUBTILE_SIZE is fail [%d]",
			      nErr);
		return FALSE;
	}

	*nColor = nVal;

	return TRUE;

}

bool vp_play_preference_set_subtitle_font_color_hex_key(const char
		*szColorHex)
{
	int nErr = 0;

	nErr =
		preference_set_string(VP_PREFERENCE_KEY_SUBTILE_FONT_COLOR_HEX,
				      szColorHex);
	if (nErr != 0) {
		VideoLogError
		("VP_PREFERENCE_KEY_SUBTILE_FONT_COLOR_HEX is fail [%d]",
		 nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_get_subtitle_font_color_hex_key(char **szColorHex)
{
	int nErr = 0;
	nErr =
		preference_get_string(VP_PREFERENCE_KEY_SUBTILE_FONT_COLOR_HEX,
				      szColorHex);
	if (nErr != 0) {
		VideoLogError
		("VP_PREFERENCE_KEY_SUBTILE_FONT_COLOR_HEX is fail [%d]",
		 nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_set_subtitle_bg_color_hex_key(const char
		*szColorHex)
{
	int nErr = 0;

	nErr =
		preference_set_string(VP_PREFERENCE_KEY_SUBTILE_BG_COLOR_HEX,
				      szColorHex);
	if (nErr != 0) {
		VideoLogError
		("VP_PREFERENCE_KEY_SUBTILE_BG_COLOR_HEX is fail [%d]", nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_get_subtitle_bg_color_hex_key(char **szColorHex)
{
	int nErr = 0;
	nErr =
		preference_get_string(VP_PREFERENCE_KEY_SUBTILE_BG_COLOR_HEX,
				      szColorHex);
	if (nErr != 0) {
		VideoLogError
		("VP_PREFERENCE_KEY_SUBTILE_BG_COLOR_HEX is fail [%d]", nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_set_subtitle_caption_win_color_hex_key(const char
		*szColorHex)
{
	int nErr = 0;

	nErr =
		preference_set_string
		(VP_PREFERENCE_KEY_SUBTILE_CAPTION_WIN_COLOR_HEX, szColorHex);
	if (nErr != 0) {
		VideoLogError
		("VP_PREFERENCE_KEY_SUBTILE_CAPTION_WIN_COLOR_HEX is fail [%d]",
		 nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_get_subtitle_caption_win_color_hex_key(char
		**szColorHex)
{
	int nErr = 0;
	nErr =
		preference_get_string
		(VP_PREFERENCE_KEY_SUBTILE_CAPTION_WIN_COLOR_HEX, szColorHex);
	if (nErr != 0) {
		VideoLogError
		("VP_PREFERENCE_KEY_SUBTILE_CAPTION_WIN_COLOR_HEX is fail [%d]",
		 nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_get_subtitle_bg_color_key(int *nColor)
{
	int nErr = 0;
	int nVal = 0;

	nErr = preference_get_int(VP_PREFERENCE_KEY_SUBTILE_BG_COLOR, &nVal);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SUBTILE_BG_COLOR is fail [%d]",
			      nErr);
		return FALSE;
	}

	*nColor = nVal;

	return TRUE;

}

bool vp_play_preference_set_subtitle_bg_color_key(int nColor)
{
	int nErr = 0;

	nErr = preference_set_int(VP_PREFERENCE_KEY_SUBTILE_BG_COLOR, nColor);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SUBTILE_BG_COLOR is fail [%d]",
			      nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_get_repeat_mode_key(int *nMode)
{
	int nErr = 0;
	int nVal = 0;

	nErr = preference_get_int(VP_PREFERENCE_KEY_REPEAT_MODE, &nVal);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_REPEAT_MODE is fail [%d]", nErr);
		return FALSE;
	}

	*nMode = nVal;

	return TRUE;

}

bool vp_play_preference_set_repeat_mode_key(int nMode)
{
	int nErr = 0;

	nErr = preference_set_int(VP_PREFERENCE_KEY_REPEAT_MODE, nMode);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_REPEAT_MODE is fail [%d]", nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_get_screen_mode_key(int *nMode)
{
	int nErr = 0;
	int nVal = 0;

	nErr = preference_get_int(VP_PREFERENCE_KEY_SCREEN_MODE, &nVal);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SCREEN_MODE is fail [%d]", nErr);
		return FALSE;
	}

	*nMode = nVal;

	return TRUE;

}

bool vp_play_preference_set_screen_mode_key(int nMode)
{
	int nErr = 0;

	nErr = preference_set_int(VP_PREFERENCE_KEY_SCREEN_MODE, nMode);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SCREEN_MODE is fail [%d]", nErr);
		return FALSE;
	}

	VideoLogInfo("== SCRENN MODE SET : %d", nMode);

	return TRUE;
}

bool vp_play_preference_get_capture_on_key(bool *bEnable)
{
	int nErr = 0;
	bool nVal = 0;

	nErr =
		preference_get_boolean(VP_PREFERENCE_KEY_CAPTURE_MODE_ON, &nVal);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_CAPTURE_MODE_ON is fail [%d]",
			      nErr);
		return FALSE;
	}

	*bEnable = (nVal == 0) ? FALSE : TRUE;

	return TRUE;
}

bool vp_play_preference_set_capture_on_key(bool bEnable)
{
	int nErr = 0;

	nErr =
		preference_set_boolean(VP_PREFERENCE_KEY_CAPTURE_MODE_ON,
				       bEnable);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_CAPTURE_MODE_ON is fail [%d]",
			      nErr);
		return FALSE;
	}

	return TRUE;
}


bool vp_play_preference_get_tag_active_key(bool *bEnable)
{
	int nErr = 0;
	bool nVal = 0;

	nErr = preference_get_boolean(VP_PREFERENCE_KEY_TAG_ACTIVE, &nVal);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_TAG_ACTIVE is fail [%d]", nErr);
		return FALSE;
	}

	*bEnable = (nVal == 0) ? FALSE : TRUE;

	return TRUE;
}

bool vp_play_preference_set_tag_active_key(bool bEnable)
{
	int nErr = 0;

	nErr = preference_set_boolean(VP_PREFERENCE_KEY_TAG_ACTIVE, bEnable);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_TAG_ACTIVE is fail [%d]", nErr);
		return FALSE;
	}

	return TRUE;
}


bool vp_play_preference_get_tag_weather_key(bool *bEnable)
{
	int nErr = 0;
	bool nVal = 0;

	nErr = preference_get_boolean(VP_PREFERENCE_KEY_TAG_WEATHER, &nVal);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_TAG_WEATHER is fail [%d]", nErr);
		return FALSE;
	}

	*bEnable = (nVal == 0) ? FALSE : TRUE;

	return TRUE;
}

bool vp_play_preference_set_tag_weather_key(bool bEnable)
{
	int nErr = 0;

	nErr = preference_set_boolean(VP_PREFERENCE_KEY_TAG_WEATHER, bEnable);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_TAG_WEATHER is fail [%d]", nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_get_tag_location_key(bool *bEnable)
{
	int nErr = 0;
	bool nVal = 0;

	nErr = preference_get_boolean(VP_PREFERENCE_KEY_TAG_LOCATION, &nVal);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_TAG_LOCATION is fail [%d]",
			      nErr);
		return FALSE;
	}

	*bEnable = (nVal == 0) ? FALSE : TRUE;

	return TRUE;
}

bool vp_play_preference_set_tag_location_key(bool bEnable)
{
	int nErr = 0;

	nErr =
		preference_set_boolean(VP_PREFERENCE_KEY_TAG_LOCATION, bEnable);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_TAG_LOCATION is fail [%d]",
			      nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_get_tag_edit_weather_key(int *nWeather)
{

	int nErr = 0;
	int nVal = 0;

	nErr = preference_get_int(VP_PREFERENCE_KEY_TAG_EDIT_WEATHER, &nVal);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_TAG_EDIT_WEATHER is fail [%d]",
			      nErr);
		return FALSE;
	}

	*nWeather = nVal;

	return TRUE;
}

bool vp_play_preference_set_tag_edit_weather_key(int nWeather)
{
	int nErr = 0;

	nErr =
		preference_set_int(VP_PREFERENCE_KEY_TAG_EDIT_WEATHER, nWeather);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_TAG_EDIT_WEATHER is fail [%d]",
			      nErr);
		return FALSE;
	}

	return TRUE;
}


bool vp_play_preference_get_sort_type_key(int *nType)
{

	int nErr = 0;
	int nVal = 0;

	nErr = preference_get_int(VP_PREFERENCE_KEY_VIDEOS_SORT_TYPE, &nVal);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_VIDEOS_SORT_TYPE is fail [%d]",
			      nErr);
		return FALSE;
	}

	*nType = nVal;

	return TRUE;
}

bool vp_play_preference_set_preview_url_videos(const char *szMediaURL)
{
	if (!szMediaURL) {
		VideoLogError("szMediaURL is null");
		return FALSE;
	}

	int nErr = 0;
	const char *pStorePath = ".video-store";

	if (strstr(szMediaURL, pStorePath) == NULL) {
		nErr =
			preference_set_string(VP_PREFERENCE_KEY_PREVIEW_URL_VIDEOS,
					      szMediaURL);
		if (nErr != 0) {
			VideoLogError
			("VP_PREFERENCE_KEY_PREVIEW_URL_VIDEOS is fail [%d]",
			 nErr);
			return FALSE;
		}
	}

	return TRUE;
}

bool vp_play_preference_get_preview_url_videos(char **szMediaURL)
{
	int nErr = 0;

	nErr =
		preference_get_string(VP_PREFERENCE_KEY_PREVIEW_URL_VIDEOS,
				      szMediaURL);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_PREVIEW_URL_VIDEOS is fail [%d]",
			      nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_set_preview_audio_track(int nAudioTrack)
{
	int nErr = 0;

	nErr =
		preference_set_int(VP_PREFERENCE_KEY_PREVIEW_AUDIO_TRACK,
				   nAudioTrack);
	if (nErr != 0) {
		VideoLogError
		("VP_PREFERENCE_KEY_PREVIEW_AUDIO_TRACK is fail [%d]", nErr);
		return FALSE;
	}
	return TRUE;
}


bool vp_play_preference_get_preview_audio_track(int *nAudioTrack)
{
	int nErr = 0;
	int nVal = 0;

	nErr =
		preference_get_int(VP_PREFERENCE_KEY_PREVIEW_AUDIO_TRACK, &nVal);
	if (nErr != 0) {
		VideoLogError
		("VP_PREFERENCE_KEY_PREVIEW_AUDIO_TRACK is fail [%d]", nErr);
		return FALSE;
	}

	*nAudioTrack = nVal;

	return TRUE;
}

bool vp_play_preference_get_sensor_asked_state(bool *bAsked)
{
	int nErr = 0;
	bool nVal = 0;

	nErr = preference_get_boolean(VP_PREFERENCE_KEY_SENSOR_ASKED, &nVal);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SENSOR_ASKED is fail [%d]",
			      nErr);
		return FALSE;
	}

	*bAsked = (nVal == 0) ? FALSE : TRUE;

	return TRUE;
}

bool vp_play_preference_set_sensor_asked_state(bool bAsked)
{
	int nErr = 0;

	nErr = preference_set_boolean(VP_PREFERENCE_KEY_SENSOR_ASKED, bAsked);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SENSOR_ASKED is fail [%d]",
			      nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_get_allow_dock_connect_key(bool *bEnable)
{
	int nErr = 0;
	bool nVal = 0;

	nErr =
		preference_get_boolean(VP_PREFERENCE_KEY_ALLOW_DOCK_CONNECT,
				       &nVal);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_ALLOW_DOCK_CONNECT is fail [%d]",
			      nErr);
		return FALSE;
	}

	*bEnable = (nVal == 0) ? FALSE : TRUE;

	return TRUE;
}

bool vp_play_preference_set_allow_dock_connect_key(bool bEnable)
{
	int nErr = 0;

	nErr =
		preference_set_boolean(VP_PREFERENCE_KEY_ALLOW_DOCK_CONNECT,
				       bEnable);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_ALLOW_DOCK_CONNECT is fail [%d]",
			      nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_get_extern_mode_key(bool *bEnable)
{
	int nErr = 0;
	bool nVal = 0;

	nErr = preference_get_boolean(VP_PREFERENCE_KEY_EXTERN_MODE, &nVal);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_EXTERN_MODE is fail [%d]", nErr);
		return FALSE;
	}

	*bEnable = (nVal == 0) ? FALSE : TRUE;

	return TRUE;
}

bool vp_play_preference_set_extern_mode_key(bool bEnable)
{
	int nErr = 0;

	nErr = preference_set_boolean(VP_PREFERENCE_KEY_EXTERN_MODE, bEnable);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_EXTERN_MODE is fail [%d]", nErr);
		return FALSE;
	}

	return TRUE;
}

bool vp_play_preference_get_sound_alive_status(int *nStatus)
{
	int nErr = 0;
	int nVal = 0;

	nErr = preference_get_int(VP_PREFERENCE_KEY_SOUND_ALIVE, &nVal);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SOUND_ALIVE is fail [%d]", nErr);
		return FALSE;
	}

	*nStatus = nVal;

	return TRUE;

}

bool vp_play_preference_set_sound_alive_status(int nStatus)
{
	int nErr = 0;

	nErr = preference_set_int(VP_PREFERENCE_KEY_SOUND_ALIVE, nStatus);
	if (nErr != 0) {
		VideoLogError("VP_PREFERENCE_KEY_SOUND_ALIVE is fail [%d]", nErr);
		return FALSE;
	}

	return TRUE;
}
