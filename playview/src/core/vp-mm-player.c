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
#include <glib.h>
#include <Ecore.h>
#include <player.h>
#include "vp-mm-player.h"
#include "vp-play-macro-define.h"

/* check temp */
#include "vp-play-log.h"

static void _vp_mm_player_print_err(int nErr)
{
	switch (nErr) {
	case PLAYER_ERROR_NONE:
		VideoLogError("PLAYER_ERROR_NONE");
		break;
	case PLAYER_ERROR_OUT_OF_MEMORY:
		VideoLogError("PLAYER_ERROR_OUT_OF_MEMORY");
		break;
	case PLAYER_ERROR_INVALID_PARAMETER:
		VideoLogError("PLAYER_ERROR_INVALID_PARAMETER");
		break;
	case PLAYER_ERROR_NO_SUCH_FILE:
		VideoLogError("PLAYER_ERROR_NO_SUCH_FILE");
		break;
	case PLAYER_ERROR_INVALID_OPERATION:
		VideoLogError("PLAYER_ERROR_INVALID_OPERATION");
		break;
	case PLAYER_ERROR_FILE_NO_SPACE_ON_DEVICE:
		VideoLogError("PLAYER_ERROR_FILE_NO_SPACE_ON_DEVICE");
		break;
	case PLAYER_ERROR_SEEK_FAILED:
		VideoLogError("PLAYER_ERROR_SEEK_FAILED");
		break;
	case PLAYER_ERROR_INVALID_STATE:
		VideoLogError("PLAYER_ERROR_INVALID_STATE");
		break;
	case PLAYER_ERROR_NOT_SUPPORTED_FILE:
		VideoLogError("PLAYER_ERROR_NOT_SUPPORTED_FILE");
		break;
	case PLAYER_ERROR_INVALID_URI:
		VideoLogError("PLAYER_ERROR_INVALID_URI");
		break;
	case PLAYER_ERROR_SOUND_POLICY:
		VideoLogError("PLAYER_ERROR_SOUND_POLICY");
		break;
	case PLAYER_ERROR_CONNECTION_FAILED:
		VideoLogError("PLAYER_ERROR_CONNECTION_FAILED");
		break;
	case PLAYER_ERROR_VIDEO_CAPTURE_FAILED:
		VideoLogError("PLAYER_ERROR_VIDEO_CAPTURE_FAILED");
		break;
	case PLAYER_ERROR_DRM_EXPIRED:
		VideoLogError("PLAYER_ERROR_DRM_EXPIRED");
		break;
	case PLAYER_ERROR_DRM_NO_LICENSE:
		VideoLogError("PLAYER_ERROR_DRM_NO_LICENSE");
		break;
	case PLAYER_ERROR_DRM_FUTURE_USE:
		VideoLogError("PLAYER_ERROR_DRM_FUTURE_USE");
		break;
	}
}

//static int _vp_mm_player_priv_convert_suond_filter(int nCode, bool bAppToFW)
//{
//
///*    if (bAppToFW) {
//              switch (nCode)
//              {
//      //      case AUDIO_EFFECT_PRESET_NONE:          return VP_MM_PLAYER_FILTER_NONE;
////            case AUDIO_EFFECT_PRESET_VOICE:         return VP_MM_PLAYER_FILTER_VOICE;
//      //      case AUDIO_EFFECT_PRESET_MOVIE:         return VP_MM_PLAYER_FILTER_MOVIE;
//              //case AUDIO_EFFECT_PRESET_VIRTUAL_5_1:         return VP_MM_PLAYER_FILTER_VITUAL_71;
//              }
//      }
//      else {
//              switch (nCode)
//              {
//      //      case VP_MM_PLAYER_FILTER_NONE:          return AUDIO_EFFECT_PRESET_NONE;
//      //      case VP_MM_PLAYER_FILTER_VOICE:         return AUDIO_EFFECT_PRESET_VOICE;
////            case VP_MM_PLAYER_FILTER_MOVIE:         return AUDIO_EFFECT_PRESET_MOVIE;
//      //      case VP_MM_PLAYER_FILTER_VITUAL_71:     return AUDIO_EFFECT_PRESET_VIRTUAL_5_1;
//              }
//      }*/
//      return 0;
//}


static int _vp_mm_player_priv_convert_video_sink(int nCode, bool bAppToFW)
{

	if (bAppToFW) {
		switch (nCode) {
		case PLAYER_DISPLAY_TYPE_OVERLAY:
			return VP_MM_PLAYER_VIDEO_TYPE_X11;
		case PLAYER_DISPLAY_TYPE_EVAS:
			return VP_MM_PLAYER_VIDEO_TYPE_EVAS;
		}
	} else {
		switch (nCode) {
		case VP_MM_PLAYER_VIDEO_TYPE_X11:
			return PLAYER_DISPLAY_TYPE_OVERLAY;
		case VP_MM_PLAYER_VIDEO_TYPE_EVAS:
			return PLAYER_DISPLAY_TYPE_EVAS;
		}
	}
	return 0;
}

#ifdef _NATIVE_BUFFER_SYNC
#else
static int _vp_mm_player_priv_convert_rotate(int nCode, bool bAppToFW)
{

	if (bAppToFW) {
		switch (nCode) {
		case PLAYER_DISPLAY_ROTATION_NONE:
			return VP_MM_PLAYER_ROTATE_0;
		case PLAYER_DISPLAY_ROTATION_90:
			return VP_MM_PLAYER_ROTATE_90;
		case PLAYER_DISPLAY_ROTATION_180:
			return VP_MM_PLAYER_ROTATE_180;
		case PLAYER_DISPLAY_ROTATION_270:
			return VP_MM_PLAYER_ROTATE_270;
		}
	} else {
		switch (nCode) {
		case VP_MM_PLAYER_ROTATE_0:
			return PLAYER_DISPLAY_ROTATION_NONE;
		case VP_MM_PLAYER_ROTATE_90:
			return PLAYER_DISPLAY_ROTATION_90;
		case VP_MM_PLAYER_ROTATE_180:
			return PLAYER_DISPLAY_ROTATION_180;
		case VP_MM_PLAYER_ROTATE_270:
			return PLAYER_DISPLAY_ROTATION_270;
		}
	}
	return 0;
}
#endif

static int _vp_mm_player_priv_convert_error(int nCode, bool bAppToFW)
{

	if (bAppToFW) {
		switch (nCode) {
		case PLAYER_ERROR_NONE:
			return VP_MM_PLYAER_ERR_NONE;
		case PLAYER_ERROR_OUT_OF_MEMORY:
			return VP_MM_PLAYER_ERR_OUT_OF_MEMORY;
		case PLAYER_ERROR_INVALID_PARAMETER:
			return VP_MM_PLAYER_ERR_INVALID_PARAMETER;
		case PLAYER_ERROR_NO_SUCH_FILE:
			return VP_MM_PLAYER_ERR_NO_SUCH_FILE;
		case PLAYER_ERROR_INVALID_OPERATION:
			return VP_MM_PLAYER_ERR_INVALID_OPERATION;
		case PLAYER_ERROR_FILE_NO_SPACE_ON_DEVICE:
			return VP_MM_PLAYER_ERR_FILE_NO_SPACE_ON_DEVICE;
		case PLAYER_ERROR_SEEK_FAILED:
			return VP_MM_PLAYER_ERR_SEEK_FAILED;
		case PLAYER_ERROR_INVALID_STATE:
			return VP_MM_PLAYER_ERR_INVALID_STATE;
		case PLAYER_ERROR_NOT_SUPPORTED_FILE:
			return VP_MM_PLAYER_ERR_NOT_SUPPORTED_FILE;
		case PLAYER_ERROR_INVALID_URI:
			return VP_MM_PLAYER_ERR_INVALID_URI;
		case PLAYER_ERROR_SOUND_POLICY:
			return VP_MM_PLAYER_ERR_SOUND_POLICY;
		case PLAYER_ERROR_CONNECTION_FAILED:
			return VP_MM_PLAYER_ERR_CONNECTION_FAILED;
		case PLAYER_ERROR_VIDEO_CAPTURE_FAILED:
			return VP_MM_PLAYER_ERR_VIDEO_CAPTUR_FAILED;
		case PLAYER_ERROR_DRM_EXPIRED:
			return VP_MM_PLAYER_ERR_DRM_EXPIRED;
		case PLAYER_ERROR_DRM_NO_LICENSE:
			return VP_MM_PLAYER_ERR_DRM_NO_LICENSE;
		case PLAYER_ERROR_DRM_FUTURE_USE:
			return VP_MM_PLAYER_ERR_DRM_FUTURE_USE;
		}
	} else {
		switch (nCode) {
		case VP_MM_PLYAER_ERR_NONE:
			return PLAYER_ERROR_NONE;
		case VP_MM_PLAYER_ERR_OUT_OF_MEMORY:
			return PLAYER_ERROR_OUT_OF_MEMORY;
		case VP_MM_PLAYER_ERR_INVALID_PARAMETER:
			return PLAYER_ERROR_INVALID_PARAMETER;
		case VP_MM_PLAYER_ERR_NO_SUCH_FILE:
			return PLAYER_ERROR_NO_SUCH_FILE;
		case VP_MM_PLAYER_ERR_INVALID_OPERATION:
			return PLAYER_ERROR_INVALID_OPERATION;
		case VP_MM_PLAYER_ERR_FILE_NO_SPACE_ON_DEVICE:
			return PLAYER_ERROR_FILE_NO_SPACE_ON_DEVICE;
		case VP_MM_PLAYER_ERR_SEEK_FAILED:
			return PLAYER_ERROR_SEEK_FAILED;
		case VP_MM_PLAYER_ERR_INVALID_STATE:
			return PLAYER_ERROR_INVALID_STATE;
		case VP_MM_PLAYER_ERR_NOT_SUPPORTED_FILE:
			return PLAYER_ERROR_NOT_SUPPORTED_FILE;
		case VP_MM_PLAYER_ERR_INVALID_URI:
			return PLAYER_ERROR_INVALID_URI;
		case VP_MM_PLAYER_ERR_SOUND_POLICY:
			return PLAYER_ERROR_SOUND_POLICY;
		case VP_MM_PLAYER_ERR_CONNECTION_FAILED:
			return PLAYER_ERROR_CONNECTION_FAILED;
		case VP_MM_PLAYER_ERR_VIDEO_CAPTUR_FAILED:
			return PLAYER_ERROR_VIDEO_CAPTURE_FAILED;
		case VP_MM_PLAYER_ERR_DRM_EXPIRED:
			return PLAYER_ERROR_DRM_EXPIRED;
		case VP_MM_PLAYER_ERR_DRM_NO_LICENSE:
			return PLAYER_ERROR_DRM_NO_LICENSE;
		case VP_MM_PLAYER_ERR_DRM_FUTURE_USE:
			return PLAYER_ERROR_DRM_FUTURE_USE;
		}
	}
	return 0;
}

static int _vp_mm_player_priv_convert_interrupt(int nCode, bool bAppToFW)
{

	if (bAppToFW) {
		switch (nCode) {
                case PLAYER_INTERRUPTED_BY_RESOURCE_CONFLICT:
			return VP_MM_PLAYER_INTERRUPTED_BY_RESOURCE_CONFLICT;
		}
	} else {
		switch (nCode) {
		case VP_MM_PLAYER_INTERRUPTED_COMPLETED:
			return PLAYER_INTERRUPTED_COMPLETED;
		case VP_MM_PLAYER_INTERRUPTED_BY_MEDIA:
			return PLAYER_INTERRUPTED_BY_MEDIA;
		case VP_MM_PLAYER_INTERRUPTED_BY_CALL:
			return PLAYER_INTERRUPTED_BY_CALL;
		case VP_MM_PLAYER_INTERRUPTED_BY_EARJACK_UNPLUG:
			return PLAYER_INTERRUPTED_BY_EARJACK_UNPLUG;
		case VP_MM_PLAYER_INTERRUPTED_BY_RESOURCE_CONFLICT:
			return PLAYER_INTERRUPTED_BY_RESOURCE_CONFLICT;
		case VP_MM_PLAYER_INTERRUPTED_BY_ALARM:
			return PLAYER_INTERRUPTED_BY_ALARM;
		case VP_MM_PLAYER_INTERRUPTED_BY_EMERGENCY:
			return PLAYER_INTERRUPTED_BY_EMERGENCY;
		//case VP_MM_PLAYER_INTERRUPTED_RESUMABLE_MEDIA:
		//	return PLAYER_INTERRUPTED_BY_RESUMABLE_MEDIA;
		case VP_MM_PLAYER_INTERRUPTED_NOTIFICATION:
			return PLAYER_INTERRUPTED_BY_NOTIFICATION;
		}
	}
	return 0;
}

static int _vp_mm_player_priv_convert_player_state(int nCode,
        bool bAppToFW)
{

	if (bAppToFW) {
		switch (nCode) {
		case PLAYER_STATE_NONE:
			return VP_MM_PLAYER_STATE_NONE;
		case PLAYER_STATE_IDLE:
			return VP_MM_PLAYER_STATE_IDLE;
		case PLAYER_STATE_READY:
			return VP_MM_PLAYER_STATE_READY;
		case PLAYER_STATE_PLAYING:
			return VP_MM_PLAYER_STATE_PLAYING;
		case PLAYER_STATE_PAUSED:
			return VP_MM_PLAYER_STATE_PAUSED;
		}
	} else {
		switch (nCode) {
		case VP_MM_PLAYER_STATE_NONE:
			return PLAYER_STATE_NONE;
		case VP_MM_PLAYER_STATE_IDLE:
			return PLAYER_STATE_IDLE;
		case VP_MM_PLAYER_STATE_READY:
			return PLAYER_STATE_READY;
		case VP_MM_PLAYER_STATE_PLAYING:
			return PLAYER_STATE_PLAYING;
		case VP_MM_PLAYER_STATE_PAUSED:
			return PLAYER_STATE_PAUSED;
		}
	}
	return 0;
}


static int _vp_mm_player_priv_convert_display_mode(int nCode,
        bool bAppToFW)
{
	if (bAppToFW) {
		switch (nCode) {
		case PLAYER_DISPLAY_MODE_LETTER_BOX:
			return VP_MM_PLAYER_DISPLAY_LETTER_BOX;
		case PLAYER_DISPLAY_MODE_ORIGIN_SIZE:
			return VP_MM_PLAYER_DISPLAY_ORIGIN_SIZE;
		case PLAYER_DISPLAY_MODE_FULL_SCREEN:
			return VP_MM_PLAYER_DISPLAY_FULL_SCREEN;
		case PLAYER_DISPLAY_MODE_CROPPED_FULL:
			return VP_MM_PLAYER_DISPLAY_CROPPED_FULL;
		case PLAYER_DISPLAY_MODE_ORIGIN_OR_LETTER:
			return VP_MM_PLAYER_DISPLAY_ORIGIN_OR_LETTER;
		}
	} else {
		switch (nCode) {
		case VP_MM_PLAYER_DISPLAY_LETTER_BOX:
			return PLAYER_DISPLAY_MODE_LETTER_BOX;
		case VP_MM_PLAYER_DISPLAY_ORIGIN_SIZE:
			return PLAYER_DISPLAY_MODE_ORIGIN_SIZE;
		case VP_MM_PLAYER_DISPLAY_FULL_SCREEN:
			return PLAYER_DISPLAY_MODE_FULL_SCREEN;
		case VP_MM_PLAYER_DISPLAY_CROPPED_FULL:
			return PLAYER_DISPLAY_MODE_CROPPED_FULL;
		case VP_MM_PLAYER_DISPLAY_ORIGIN_OR_LETTER:
			return PLAYER_DISPLAY_MODE_ORIGIN_OR_LETTER;
		}
	}
	return 0;
}


//static int _vp_mm_player_priv_convert_missed_plugin_type(int nCode, bool bAppToFW)
//{
///*    if (bAppToFW) {
//              switch (nCode)
//              {
////            case PLAYER_MISSED_PLUGIN_UNKNOWN:              return VP_MM_PLAYER_MISSED_PLUGIN_UNKNOWN;
//      //      case PLAYER_MISSED_PLUGIN_VIDEO_DECODER:        return VP_MM_PLAYER_MISSED_PLUGIN_VIDEO;
//      //      case PLAYER_MISSED_PLUGIN_AUDIO_DECODER:        return VP_MM_PLAYER_MISSED_PLUGIN_AUDIO;
//              }
//      }
//      else {
//              switch (nCode)
//              {
//      //      case VP_MM_PLAYER_MISSED_PLUGIN_UNKNOWN:        return PLAYER_MISSED_PLUGIN_UNKNOWN;
//      //      case VP_MM_PLAYER_MISSED_PLUGIN_VIDEO:          return PLAYER_MISSED_PLUGIN_VIDEO_DECODER;
////            case VP_MM_PLAYER_MISSED_PLUGIN_AUDIO:          return PLAYER_MISSED_PLUGIN_AUDIO_DECODER;
//              }
//      }*/
//      return 0;
//}



/* callback functions */
static void __vp_mm_player_pipe_prepare_cb(void *pUserData, void *pBuff,
		        unsigned int nByte)
{
	if (pUserData == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return;
	}

	MMPlayer *pMMPlayer = (MMPlayer *) pUserData;

	pMMPlayer->bIsRealize = TRUE;

	if (pMMPlayer->pCbFunc->pPrepareCb) {
		pMMPlayer->pCbFunc->pPrepareCb(pMMPlayer->pParam);
	}

	return;
}

static void __vp_mm_player_pipe_end_of_stream_cb(void *pUserData,
        		void *pBuff,
        		unsigned int nByte)
{
	if (pUserData == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return;
	}

	MMPlayer *pMMPlayer = (MMPlayer *) pUserData;

	if (pMMPlayer->pCbFunc->pEndOfStreamCb) {
		pMMPlayer->pCbFunc->pEndOfStreamCb(pMMPlayer->pParam);
	}


	return;
}

static void __vp_mm_player_pipe_seek_complete_cb(void *pUserData,
        		void *pBuff,
        		unsigned int nByte)
{
	if (pUserData == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return;
	}

	MMPlayer *pMMPlayer = (MMPlayer *) pUserData;

	if (pMMPlayer->pCbFunc->pSeekCompleteCb) {
		pMMPlayer->pCbFunc->pSeekCompleteCb(pMMPlayer->pParam);
	}

	return;
}

static void __vp_mm_player_pipe_interrupted_cb(void *pUserData,
                        void *pBuff,
                        unsigned int nByte)
{
	if (pUserData == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return;
	}

	MMPlayer *pMMPlayer = (MMPlayer *) pUserData;

	if (pMMPlayer->pCbFunc->pInterruptedCb) {
		int nInterruptCode = 0;
		if (pMMPlayer->pInterruptParam) {
			nInterruptCode =
			    _vp_mm_player_priv_convert_interrupt(pMMPlayer->
			            pInterruptParam->
			            nInterrupt, TRUE);
		}

		pMMPlayer->pCbFunc->pInterruptedCb(nInterruptCode,
		                                   pMMPlayer->pParam);
	}

	return;
}

static void __vp_mm_player_pipe_error_cb(void *pUserData, void *pBuff,
                        unsigned int nByte)
{
	if (pUserData == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return;
	}

	MMPlayer *pMMPlayer = (MMPlayer *) pUserData;
	if (pMMPlayer->pCbFunc->pErrorCb) {
		int nErr = 0;
		if (pMMPlayer->pErrorParam) {
			nErr =
			    _vp_mm_player_priv_convert_error(pMMPlayer->pErrorParam->
			                                     nError, TRUE);
		}

		pMMPlayer->pCbFunc->pErrorCb(nErr, pMMPlayer->pParam);
	}
	return;
}

static void __vp_mm_player_pipe_buffering_cb(void *pUserData, void *pBuff,
                        unsigned int nByte)
{
	if (pUserData == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return;
	}

	MMPlayer *pMMPlayer = (MMPlayer *) pUserData;
	if (pMMPlayer->pCbFunc->pBufferingCb) {
		int nPercent = 0;
		if (pMMPlayer->pBufferingParam) {
			nPercent = pMMPlayer->pBufferingParam->nPercent;
		}

		pMMPlayer->pCbFunc->pBufferingCb(nPercent, pMMPlayer->pParam);
	}
	return;
}

static void __vp_mm_player_pipe_subtitle_update_cb(void *pUserData,
                        void *pBuff,
                        unsigned int nByte)
{
	if (pUserData == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return;
	}

	MMPlayer *pMMPlayer = (MMPlayer *) pUserData;

	if (pMMPlayer->pCbFunc->pSubtitleUpdateCb) {
		char *szSubtitleText = NULL;
		unsigned long nDuration = 0;
		if (pMMPlayer->pSubtitleParam) {
			if (pMMPlayer->pSubtitleParam->txt) {
				szSubtitleText = strdup(pMMPlayer->pSubtitleParam->txt);
				free(pMMPlayer->pSubtitleParam->txt);
				pMMPlayer->pSubtitleParam->txt = NULL;
			}
			nDuration = pMMPlayer->pSubtitleParam->nDuration;
		}

		if (szSubtitleText && strlen(szSubtitleText) > 0)
			pMMPlayer->pCbFunc->pSubtitleUpdateCb(nDuration,
			                                      szSubtitleText,
			                                      pMMPlayer->pParam);
		VP_FREE(szSubtitleText);
	}

	return;

}

static void __vp_mm_player_pipe_pd_message_cb(void *pUserData,
                        void *pBuff,
                        unsigned int nByte)
{
	if (pUserData == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return;
	}

	MMPlayer *pMMPlayer = (MMPlayer *) pUserData;

	if (pMMPlayer->pCbFunc->pPDMessageCb) {
		int nMsg = 0;
		if (pMMPlayer->pPDParam) {
			nMsg = pMMPlayer->pPDParam->nMessage;
		}

		pMMPlayer->pCbFunc->pPDMessageCb(nMsg, pMMPlayer->pParam);
	}
	return;

}

static void __vp_mm_player_pipe_captured_cb(void *pUserData, void *pBuff,
                        unsigned int nByte)
{
	if (pUserData == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return;
	}

	MMPlayer *pMMPlayer = (MMPlayer *) pUserData;

	if (pMMPlayer->pCbFunc->pCapturedCb) {
		unsigned char *pFrame = NULL;
		int nWidth = 0;
		int nHeight = 0;
		unsigned int nSize = 0;

		if (pMMPlayer->pCaptureParam) {
			pFrame = pMMPlayer->pCaptureParam->pFrame;
			nWidth = pMMPlayer->pCaptureParam->nWidth;
			nHeight = pMMPlayer->pCaptureParam->nHeight;
			nSize = pMMPlayer->pCaptureParam->nSize;
		}

		pMMPlayer->pCbFunc->pCapturedCb(pFrame, nWidth, nHeight, nSize,
		                                pMMPlayer->pParam);
	}

	return;
}

static void __vp_mm_player_pipe_missed_plugin_cb(void *pUserData,
                        void *pBuff,
                        unsigned int nByte)
{
	if (pUserData == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return;
	}

	MMPlayer *pMMPlayer = (MMPlayer *) pUserData;

	if (pMMPlayer->pCbFunc->pMissedPluginCb) {

		vp_mm_player_missed_plugin_t nMissedType =
		    VP_MM_PLAYER_MISSED_PLUGIN_UNKNOWN;

		if (pMMPlayer->pMissedParam) {
			nMissedType = pMMPlayer->pMissedParam->nMissedType;
		}

		pMMPlayer->pCbFunc->pMissedPluginCb(nMissedType,
		                                    pMMPlayer->pParam);
	}

	return;
}



static void __vp_mm_player_pipe_image_buffer_cb(void *pUserData,
                        void *pBuff,
                        unsigned int nByte)
{
	if (pUserData == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return;
	}

	MMPlayer *pMMPlayer = (MMPlayer *) pUserData;

	if (pMMPlayer->pCbFunc->pImageBufferCb) {
		char *pBuffer = NULL;
		int nSize = 0;

		if (pMMPlayer->pImageBufferParam) {
			pBuffer = pMMPlayer->pImageBufferParam->pBuffer;
			nSize = pMMPlayer->pImageBufferParam->nSize;
		}

		pMMPlayer->pCbFunc->pImageBufferCb(pBuffer, nSize,
		                                   pMMPlayer->pParam);
	}

	return;
}



/* player api local cb functions */
static void __vp_mm_player_prepare_cb(void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}
	VideoLogWarning("");

	MMPlayer *pMMPlayer = (MMPlayer *) pUserData;

	if (pMMPlayer->pCbPipe->pPreparePipe) {
		Eina_Bool bRet = EINA_FALSE;
		bRet =
		    ecore_pipe_write(pMMPlayer->pCbPipe->pPreparePipe,
		                     (void *) pMMPlayer, sizeof(MMPlayer));
		if (bRet != EINA_TRUE) {
			VideoLogError(" : ecore pipe write fail");
		}
	}

	return;
}

static void __vp_mm_player_completed_cb(void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MMPlayer *pMMPlayer = (MMPlayer *) pUserData;

	if (pMMPlayer->pCbPipe->pEndOfStreamPipe) {
		Eina_Bool bRet = EINA_FALSE;
		bRet =
		    ecore_pipe_write(pMMPlayer->pCbPipe->pEndOfStreamPipe,
		                     (void *) pMMPlayer, sizeof(MMPlayer));
		if (bRet != EINA_TRUE) {
			VideoLogError(" : ecore pipe write fail");
		}
	}

	return;

}

static void __vp_mm_player_seek_completed_cb(void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MMPlayer *pMMPlayer = (MMPlayer *) pUserData;
	if (pMMPlayer->pCbPipe->pSeekCompletePipe) {
		Eina_Bool bRet = EINA_FALSE;
		bRet =
		    ecore_pipe_write(pMMPlayer->pCbPipe->pSeekCompletePipe,
		                     (void *) pMMPlayer, sizeof(MMPlayer));
		if (bRet != EINA_TRUE) {
			VideoLogError(" : ecore pipe write fail");
		}
	}

	return;

}

static void __vp_mm_player_interrupted_cb(player_interrupted_code_e nCode,
        void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MMPlayer *pMMPlayer = (MMPlayer *) pUserData;
	if (pMMPlayer->pCbPipe->pInterruptedPipe) {
		Eina_Bool bRet = EINA_FALSE;

		if (pMMPlayer->pInterruptParam) {
			pMMPlayer->pInterruptParam->nInterrupt = nCode;
			pMMPlayer->pInterruptParam->pParam = pMMPlayer->pParam;
		}

		bRet =
		    ecore_pipe_write(pMMPlayer->pCbPipe->pInterruptedPipe,
		                     (void *) pMMPlayer, sizeof(MMPlayer));
		if (bRet != EINA_TRUE) {
			VideoLogError(" : ecore pipe write fail");
		}
	}

	return;

}

static void __vp_mm_player_error_cb(int nError, void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MMPlayer *pMMPlayer = (MMPlayer *) pUserData;
	if (pMMPlayer->pCbPipe->pErrorPipe) {
		Eina_Bool bRet = EINA_FALSE;

		if (pMMPlayer->pErrorParam) {
			pMMPlayer->pErrorParam->nError = nError;
			pMMPlayer->pErrorParam->pParam = pMMPlayer->pParam;
		}

		bRet =
		    ecore_pipe_write(pMMPlayer->pCbPipe->pErrorPipe,
		                     (void *) pMMPlayer, sizeof(MMPlayer));
		if (bRet != EINA_TRUE) {
			VideoLogError(" : ecore pipe write fail");
		}
	}

	return;

}

static void __vp_mm_player_buffering_cb(int nPercent, void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MMPlayer *pMMPlayer = (MMPlayer *) pUserData;
	if (pMMPlayer->pCbPipe->pBufferingPipe) {
		Eina_Bool bRet = EINA_FALSE;

		if (pMMPlayer->pBufferingParam) {
			pMMPlayer->pBufferingParam->nPercent = nPercent;
			pMMPlayer->pBufferingParam->pParam = pMMPlayer->pParam;
		}

		bRet =
		    ecore_pipe_write(pMMPlayer->pCbPipe->pBufferingPipe,
		                     (void *) pMMPlayer, sizeof(MMPlayer));
		if (bRet != EINA_TRUE) {
			VideoLogError(" : ecore pipe write fail");
		}
	}

	return;

}

static void __vp_mm_player_subtitle_updated_cb(unsigned long nDuration,
                        char *text,
                        void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MMPlayer *pMMPlayer = (MMPlayer *) pUserData;

	if (pMMPlayer->bDeactivateSubtitle) {
		return;
	}

	if (pMMPlayer->pCbPipe->pSubtitleUpdatePipe) {
		if (pMMPlayer->pSubtitleParam) {
			pMMPlayer->pSubtitleParam->nDuration = nDuration;

			if (text) {
				if (pMMPlayer->pSubtitleParam->txt) {
					free(pMMPlayer->pSubtitleParam->txt);
					pMMPlayer->pSubtitleParam->txt = NULL;
				}
				pMMPlayer->pSubtitleParam->txt = strdup(text);
			}

			pMMPlayer->pSubtitleParam->pParam = pMMPlayer->pParam;
		}

		if (pMMPlayer->pSubtitleParam && pMMPlayer->pSubtitleParam->txt
		        && strlen(pMMPlayer->pSubtitleParam->txt) > 0) {
			Eina_Bool bRet = EINA_FALSE;
			bRet =
			    ecore_pipe_write(pMMPlayer->pCbPipe->pSubtitleUpdatePipe,
			                     (void *) pMMPlayer, sizeof(MMPlayer));
			if (bRet != EINA_TRUE) {
				VideoLogError(" : ecore pipe write fail");
			}
		}
	}
	return;

}

static void __vp_mm_player_video_captured_cb(unsigned char *pFrame,
                        int nWidth, int nHeight,
                        unsigned int nSize,
                        void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	MMPlayer *pMMPlayer = (MMPlayer *) pUserData;
	if (pMMPlayer->pCbPipe->pCapturedPipe) {
		Eina_Bool bRet = EINA_FALSE;

		if (pMMPlayer->pCaptureParam) {

			if (pMMPlayer->pCaptureParam->pFrame) {
				free(pMMPlayer->pCaptureParam->pFrame);
				pMMPlayer->pCaptureParam->pFrame = NULL;
			}

			pMMPlayer->pCaptureParam->pFrame = calloc(1, nSize);
			if (pMMPlayer->pCaptureParam->pFrame == NULL) {
				return;
			}

			memcpy(pMMPlayer->pCaptureParam->pFrame, pFrame, nSize);

			pMMPlayer->pCaptureParam->nWidth = nWidth;
			pMMPlayer->pCaptureParam->nHeight = nHeight;
			pMMPlayer->pCaptureParam->nSize = nSize;
			pMMPlayer->pCaptureParam->pParam = pMMPlayer->pParam;
		}
		bRet =
		    ecore_pipe_write(pMMPlayer->pCbPipe->pCapturedPipe,
		                     (void *) pMMPlayer, sizeof(MMPlayer));
		if (bRet != EINA_TRUE) {
			VideoLogError(" : ecore pipe write fail");
		}
	}

	return;
}


//static void __vp_mm_player_missed_plugin_cb(player_missed_plugin_type_e type, char *message, void *pUserData)
//{
//      if (pUserData == NULL) {
//              VideoLogError("pUserData is NULL");
//              return;
//      }
//
//      MMPlayer *pMMPlayer = (MMPlayer *)pUserData;
//      if (pMMPlayer->pCbPipe->pMissedPluginPipe) {
//
//              if (pMMPlayer->pMissedParam) {
//
//                      pMMPlayer->pMissedParam->nMissedType = 0;//_vp_mm_player_priv_convert_missed_plugin_type(type, TRUE);
//                      pMMPlayer->pMissedParam->pParam = pMMPlayer->pParam;
//              }
//
//              Eina_Bool bRet = EINA_FALSE;
//              bRet = ecore_pipe_write(pMMPlayer->pCbPipe->pMissedPluginPipe, (void *)pMMPlayer, sizeof(MMPlayer));
//              if (bRet != EINA_TRUE) {
//                      VideoLogError(" : ecore pipe write fail");
//              }
//      }
//
//      return;
//
//}

//static void __vp_mm_player_image_buffer_cb(char *pBuffer, int nSize, void *pUserData)
//{
//      if (pUserData == NULL) {
//              VideoLogError("pUserData is NULL");
//              return;
//      }
//
//      if (pBuffer == NULL) {
//              VideoLogError("pBuffer is NULL");
//              return;
//      }
//      MMPlayer *pMMPlayer = (MMPlayer *)pUserData;
//      if (pMMPlayer->pCbPipe->pImageBufferPipe) {
//              if (pMMPlayer->pImageBufferParam) {
//                      if (pMMPlayer->pImageBufferParam->pBuffer) {
//                              free(pMMPlayer->pImageBufferParam->pBuffer);
//                              pMMPlayer->pImageBufferParam->pBuffer = NULL;
//                      }
//
//                      pMMPlayer->pImageBufferParam->pBuffer = calloc(1, nSize);
//                      if (pMMPlayer->pImageBufferParam->pBuffer == NULL) {
//                              VideoLogError("Alloc Fail");
//                              return;
//                      }
//
//                      memcpy(pMMPlayer->pImageBufferParam->pBuffer, pBuffer, nSize);
//
//                      pMMPlayer->pImageBufferParam->nSize = nSize;
//                      pMMPlayer->pImageBufferParam->pParam = pMMPlayer->pParam;
//              }
//
//              Eina_Bool bRet = EINA_FALSE;
//              bRet = ecore_pipe_write(pMMPlayer->pCbPipe->pImageBufferPipe, (void *)pMMPlayer, sizeof(MMPlayer));
//              if (bRet != EINA_TRUE) {
//                      VideoLogError(" : ecore pipe write fail");
//              }
//
//      }
//
//      return;
//
//}


/* internal functions */



static void _vp_mm_callback_param_free(vp_mm_player_callback_t nType,
                                       void *pParam)
{
	if (pParam == NULL) {
		VideoLogError("Param data is NULL");
		return;
	}

	if (nType == VP_MM_PLAYER_CAPTURE_VIDEO_CB) {
		MMPlayerCaptureParam *pCaptureParam =
		    (MMPlayerCaptureParam *) pParam;
		if (pCaptureParam->pFrame) {
			free(pCaptureParam->pFrame);
			pCaptureParam->pFrame = NULL;
		}

		free(pCaptureParam);
		pCaptureParam = NULL;
	} else if (nType == VP_MM_PLAYER_SUBTITLE_UPDATE_CB) {
		MMPlayerSubtitleParam *pSubtitleParam =
		    (MMPlayerSubtitleParam *) pParam;
		if (pSubtitleParam->txt) {
			free(pSubtitleParam->txt);
			pSubtitleParam->txt = NULL;
		}
		free(pSubtitleParam);
		pSubtitleParam = NULL;
	} else if (nType == VP_MM_PLAYER_IMAGE_BUFFER_CB) {
		MMPlayerImageBufferParam *pImageBufferParam =
		    (MMPlayerImageBufferParam *) pParam;
		if (pImageBufferParam->pBuffer) {
			free(pImageBufferParam->pBuffer);
			pImageBufferParam->pBuffer = NULL;
		}
		free(pImageBufferParam);
		pImageBufferParam = NULL;
	}

	return;
}

static void _vp_mm_ecore_pipe_create(MMPlayer *pMMPlayer)
{
	if (pMMPlayer == NULL) {
		VideoLogError("pMMPlayer is NULL");
		return;
	}

	MMPlayerCbPipe *pCbPipe = pMMPlayer->pCbPipe;
	if (pCbPipe == NULL) {
		VideoLogError("pCbPipe is NULL");
		return;
	}

	pCbPipe->pPreparePipe =
	    ecore_pipe_add((Ecore_Pipe_Cb) __vp_mm_player_pipe_prepare_cb,
	                   (void *) pMMPlayer);
	if (pCbPipe->pPreparePipe == NULL) {
		VideoLogWarning("pPreparePipe create fail");
	}

	pCbPipe->pEndOfStreamPipe =
	    ecore_pipe_add((Ecore_Pipe_Cb)
	                   __vp_mm_player_pipe_end_of_stream_cb,
	                   (void *) pMMPlayer);
	if (pCbPipe->pEndOfStreamPipe == NULL) {
		VideoLogWarning("pEndOfStreamPipe create fail");
	}

	pCbPipe->pSeekCompletePipe =
	    ecore_pipe_add((Ecore_Pipe_Cb)
	                   __vp_mm_player_pipe_seek_complete_cb,
	                   (void *) pMMPlayer);
	if (pCbPipe->pSeekCompletePipe == NULL) {
		VideoLogWarning("pSeekCompletePipe create fail");
	}

	pCbPipe->pInterruptedPipe =
	    ecore_pipe_add((Ecore_Pipe_Cb) __vp_mm_player_pipe_interrupted_cb,
	                   (void *) pMMPlayer);
	if (pCbPipe->pInterruptedPipe == NULL) {
		VideoLogWarning("pInterruptedPipe create fail");
	}

	pCbPipe->pErrorPipe =
	    ecore_pipe_add((Ecore_Pipe_Cb) __vp_mm_player_pipe_error_cb,
	                   (void *) pMMPlayer);
	if (pCbPipe->pErrorPipe == NULL) {
		VideoLogWarning("pErrorPipe create fail");
	}

	pCbPipe->pBufferingPipe =
	    ecore_pipe_add((Ecore_Pipe_Cb) __vp_mm_player_pipe_buffering_cb,
	                   (void *) pMMPlayer);
	if (pCbPipe->pBufferingPipe == NULL) {
		VideoLogWarning("pBufferingPipe create fail");
	}

	pCbPipe->pSubtitleUpdatePipe =
	    ecore_pipe_add((Ecore_Pipe_Cb)
	                   __vp_mm_player_pipe_subtitle_update_cb,
	                   (void *) pMMPlayer);
	if (pCbPipe->pSubtitleUpdatePipe == NULL) {
		VideoLogWarning("pSubtitleUpdatePipe create fail");
	}

	pCbPipe->pPDMessagePipe =
	    ecore_pipe_add((Ecore_Pipe_Cb) __vp_mm_player_pipe_pd_message_cb,
	                   (void *) pMMPlayer);
	if (pCbPipe->pPDMessagePipe == NULL) {
		VideoLogWarning("pPDMessagePipe create fail");
	}

	pCbPipe->pCapturedPipe =
	    ecore_pipe_add((Ecore_Pipe_Cb) __vp_mm_player_pipe_captured_cb,
	                   (void *) pMMPlayer);
	if (pCbPipe->pCapturedPipe == NULL) {
		VideoLogWarning("pCapturedPipe create fail");
	}

	pCbPipe->pMissedPluginPipe =
	    ecore_pipe_add((Ecore_Pipe_Cb)
	                   __vp_mm_player_pipe_missed_plugin_cb,
	                   (void *) pMMPlayer);
	if (pCbPipe->pMissedPluginPipe == NULL) {
		VideoLogWarning("pMissedPluginPipe create fail");
	}

	pCbPipe->pImageBufferPipe =
	    ecore_pipe_add((Ecore_Pipe_Cb)
	                   __vp_mm_player_pipe_image_buffer_cb,
	                   (void *) pMMPlayer);
	if (pCbPipe->pImageBufferPipe == NULL) {
		VideoLogWarning("pImageBufferPipe create fail");
	}

	return;
}

static void _vp_mm_ecore_pipe_del(Ecore_Pipe *pPipe)
{
	if (pPipe == NULL) {
		VideoLogError("pPipe is NULL");
		return;
	}

	ecore_pipe_del(pPipe);
	pPipe = NULL;

	return;
}

static void _vp_mm_register_default_callback(MMPlayer *pMMPlayer)
{
	if (pMMPlayer == NULL) {
		VideoLogError("pMMPlayer is NULL");
		return;
	}

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return;
	}
	int nRet = PLAYER_ERROR_NONE;
	nRet =
	    player_set_completed_cb(pMMPlayer->pPlayer,
	                            __vp_mm_player_completed_cb,
	                            (void *) pMMPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_completed_cb fail : %d", nRet);
	}

	nRet =
	    player_set_interrupted_cb(pMMPlayer->pPlayer,
	                              __vp_mm_player_interrupted_cb,
	                              (void *) pMMPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_interrupted_cb fail : %d", nRet);
	}

	nRet =
	    player_set_error_cb(pMMPlayer->pPlayer, __vp_mm_player_error_cb,
	                        (void *) pMMPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_error_cb fail : %d", nRet);
	}

	nRet =
	    player_set_buffering_cb(pMMPlayer->pPlayer,
	                            __vp_mm_player_buffering_cb,
	                            (void *) pMMPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_buffering_cb fail : %d", nRet);
	}

	nRet =
	    player_set_subtitle_updated_cb(pMMPlayer->pPlayer,
	                                   __vp_mm_player_subtitle_updated_cb,
	                                   (void *) pMMPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_subtitle_updated_cb fail : %d", nRet);
	}
//      nRet = player_set_missed_plugin_info_cb(pMMPlayer->pPlayer, __vp_mm_player_missed_plugin_cb, (void *)pMMPlayer);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_set_missed_plugin_info_cb fail : %d", nRet);
//      }

//      nRet = player_set_image_buffer_updated_cb(pMMPlayer->pPlayer, __vp_mm_player_image_buffer_cb, (void *)pMMPlayer);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_set_image_buffer_updated_cb fail : %d", nRet);
//      }


	return;

}

static void _vp_mm_unregister_default_callback(MMPlayer *pMMPlayer)
{
	if (pMMPlayer == NULL) {
		VideoLogError("pMMPlayer is NULL");
		return;
	}
	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return;
	}
	int nRet = PLAYER_ERROR_NONE;
	nRet = player_unset_completed_cb(pMMPlayer->pPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_unset_completed_cb fail : %d", nRet);
	}
	nRet = player_unset_interrupted_cb(pMMPlayer->pPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_unset_interrupted_cb fail : %d", nRet);
	}
	nRet = player_unset_error_cb(pMMPlayer->pPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_unset_error_cb fail : %d", nRet);
	}
	nRet = player_unset_buffering_cb(pMMPlayer->pPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_unset_buffering_cb fail : %d", nRet);
	}
	nRet = player_unset_subtitle_updated_cb(pMMPlayer->pPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_unset_subtitle_updated_cb fail : %d", nRet);
	}
//      nRet = player_unset_image_buffer_updated_cb(pMMPlayer->pPlayer);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_unset_image_buffer_updated_cb fail : %d", nRet);
//      }

	return;
}



/* external functions */
mm_player_handle vp_mm_player_create()
{
	MMPlayer *pMMPlayer = NULL;
	pMMPlayer = calloc(1, sizeof(MMPlayer));
	if (pMMPlayer == NULL) {
		VideoLogError("MMPlayer alloc fail");
		return NULL;
	}

	VideoLogInfo(">> player create");

	int nRet = 0;
	nRet = player_create(&pMMPlayer->pPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		vp_mm_player_destroy((mm_player_handle) pMMPlayer);
		return NULL;
	}

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("player handle is NULL");
		vp_mm_player_destroy((mm_player_handle) pMMPlayer);
		return NULL;
	}
//      nRet = player_set_rich_audio(pMMPlayer->pPlayer);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//      }

	pMMPlayer->pCbFunc = calloc(1, sizeof(MMPlayerCbFunc));
	if (pMMPlayer->pCbFunc == NULL) {
		VideoLogError("MMPlayer callback func alloc fail");
		vp_mm_player_destroy((mm_player_handle) pMMPlayer);
		return NULL;
	}

	pMMPlayer->pCbPipe = calloc(1, sizeof(MMPlayerCbPipe));
	if (pMMPlayer->pCbPipe == NULL) {
		VideoLogError("MMPlayer Pipe alloc fail");
		vp_mm_player_destroy((mm_player_handle) pMMPlayer);
		return NULL;
	}

	pMMPlayer->pCaptureParam = calloc(1, sizeof(MMPlayerCaptureParam));
	if (pMMPlayer->pCaptureParam == NULL) {
		VideoLogError("MMPlayer pCaptureParam alloc fail");
	}

	pMMPlayer->pSubtitleParam = calloc(1, sizeof(MMPlayerSubtitleParam));
	if (pMMPlayer->pSubtitleParam == NULL) {
		VideoLogError("MMPlayer pSubtitleParam alloc fail");
	}

	pMMPlayer->pInterruptParam =
	    calloc(1, sizeof(MMPlayerInterruptParam));
	if (pMMPlayer->pInterruptParam == NULL) {
		VideoLogError("MMPlayer pInterruptParam alloc fail");
	}

	pMMPlayer->pErrorParam = calloc(1, sizeof(MMPlayerErrorParam));
	if (pMMPlayer->pErrorParam == NULL) {
		VideoLogError("MMPlayer pErrorParam alloc fail");
	}
	pMMPlayer->pBufferingParam =
	    calloc(1, sizeof(MMPlayerBufferingParam));
	if (pMMPlayer->pBufferingParam == NULL) {
		VideoLogError("MMPlayer pBufferingParam alloc fail");
	}

	pMMPlayer->pPDParam = calloc(1, sizeof(MMPlayerPDParam));
	if (pMMPlayer->pPDParam == NULL) {
		VideoLogError("MMPlayer pPDParam alloc fail");
	}

	pMMPlayer->pMissedParam =
	    calloc(1, sizeof(MMPlayerMissedPluginParam));
	if (pMMPlayer->pMissedParam == NULL) {
		VideoLogError("MMPlayer pMissedParam alloc fail");
	}

	pMMPlayer->pImageBufferParam =
	    calloc(1, sizeof(MMPlayerImageBufferParam));
	if (pMMPlayer->pImageBufferParam == NULL) {
		VideoLogError("MMPlayer pImageBufferParam alloc fail");
	}

	_vp_mm_ecore_pipe_create(pMMPlayer);

	return (mm_player_handle) pMMPlayer;
}

void vp_mm_player_destroy(mm_player_handle pPlayerHandle)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;
	int nRet = 0;

	if (!vp_mm_player_unrealize(pPlayerHandle)) {
		VideoLogWarning("vp_mm_player_unrealize fail");
	}


	nRet = player_destroy(pMMPlayer->pPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
	}
	pMMPlayer->pPlayer = NULL;

	pMMPlayer->fLeftVol = 0.0;
	pMMPlayer->fRightVol = 0.0;

	pMMPlayer->nSoundFilter = VP_MM_PLAYER_FILTER_NONE;
	pMMPlayer->nVideoSink = VP_MM_PLAYER_VIDEO_TYPE_X11;
	pMMPlayer->nRotate = VP_MM_PLAYER_ROTATE_0;
	pMMPlayer->bIsScaling = FALSE;

	_vp_mm_callback_param_free(VP_MM_PLAYER_CAPTURE_VIDEO_CB,
	                           (void *) pMMPlayer->pCaptureParam);
	_vp_mm_callback_param_free(VP_MM_PLAYER_SUBTITLE_UPDATE_CB,
	                           (void *) pMMPlayer->pSubtitleParam);
	_vp_mm_callback_param_free(VP_MM_PLAYER_IMAGE_BUFFER_CB,
	                           (void *) pMMPlayer->pImageBufferParam);

	if (pMMPlayer->pInterruptParam) {
		free(pMMPlayer->pInterruptParam);
		pMMPlayer->pInterruptParam = NULL;
	}

	if (pMMPlayer->pErrorParam) {
		free(pMMPlayer->pErrorParam);
		pMMPlayer->pErrorParam = NULL;
	}

	if (pMMPlayer->pBufferingParam) {
		free(pMMPlayer->pBufferingParam);
		pMMPlayer->pBufferingParam = NULL;
	}

	if (pMMPlayer->pPDParam) {
		free(pMMPlayer->pPDParam);
		pMMPlayer->pPDParam = NULL;
	}

	if (pMMPlayer->pMissedParam) {
		free(pMMPlayer->pMissedParam);
		pMMPlayer->pMissedParam = NULL;
	}

	if (pMMPlayer->pCbFunc) {
		pMMPlayer->pCbFunc->pBufferingCb = NULL;
		pMMPlayer->pCbFunc->pPrepareCb = NULL;
		pMMPlayer->pCbFunc->pEndOfStreamCb = NULL;
		pMMPlayer->pCbFunc->pSeekCompleteCb = NULL;
		pMMPlayer->pCbFunc->pInterruptedCb = NULL;
		pMMPlayer->pCbFunc->pErrorCb = NULL;
		pMMPlayer->pCbFunc->pBufferingCb = NULL;
		pMMPlayer->pCbFunc->pSubtitleUpdateCb = NULL;
		pMMPlayer->pCbFunc->pPDMessageCb = NULL;
		pMMPlayer->pCbFunc->pCapturedCb = NULL;
		pMMPlayer->pCbFunc->pMissedPluginCb = NULL;
		pMMPlayer->pCbFunc->pImageBufferCb = NULL;

		free(pMMPlayer->pCbFunc);
		pMMPlayer->pCbFunc = NULL;
	}

	if (pMMPlayer->pCbPipe) {
		_vp_mm_ecore_pipe_del(pMMPlayer->pCbPipe->pPreparePipe);
		_vp_mm_ecore_pipe_del(pMMPlayer->pCbPipe->pEndOfStreamPipe);
		_vp_mm_ecore_pipe_del(pMMPlayer->pCbPipe->pSeekCompletePipe);
		_vp_mm_ecore_pipe_del(pMMPlayer->pCbPipe->pInterruptedPipe);
		_vp_mm_ecore_pipe_del(pMMPlayer->pCbPipe->pErrorPipe);
		_vp_mm_ecore_pipe_del(pMMPlayer->pCbPipe->pBufferingPipe);
		_vp_mm_ecore_pipe_del(pMMPlayer->pCbPipe->pSubtitleUpdatePipe);
		_vp_mm_ecore_pipe_del(pMMPlayer->pCbPipe->pPDMessagePipe);
		_vp_mm_ecore_pipe_del(pMMPlayer->pCbPipe->pCapturedPipe);
		_vp_mm_ecore_pipe_del(pMMPlayer->pCbPipe->pMissedPluginPipe);
		_vp_mm_ecore_pipe_del(pMMPlayer->pCbPipe->pImageBufferPipe);

		free(pMMPlayer->pCbPipe);
		pMMPlayer->pCbPipe = NULL;
	}

	pMMPlayer->pParam = NULL;
	pMMPlayer->pVideoSink = NULL;

	free(pMMPlayer);
	pMMPlayer = NULL;
}

bool vp_mm_player_realize(mm_player_handle pPlayerHandle, char *szPath)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	if (szPath == NULL) {
		VideoLogError("szPath is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->bIsRealize) {
		VideoLogError("Already realized");
		return FALSE;
	}


	int nRet = player_set_uri(pMMPlayer->pPlayer, szPath);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_uri is fail : %d", nRet);
		return FALSE;
	}
//      nRet = player_set_safety_volume(pMMPlayer->pPlayer);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_set_safety_volume is fail : %d", nRet);
//      }

	_vp_mm_register_default_callback(pMMPlayer);

//      nRet = player_enable_sync_handler(pMMPlayer->pPlayer, TRUE);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_enable_sync_handler is fail : %d", nRet);
//              return FALSE;
//      }

	nRet = player_prepare(pMMPlayer->pPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_prepare is fail : %d", nRet);
		return FALSE;
	}

	pMMPlayer->bIsRealize = TRUE;

	return TRUE;
}

bool vp_mm_player_realize_async(mm_player_handle pPlayerHandle,
                                char *szPath)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->bIsRealize) {
		VideoLogError("Already realized");
		return FALSE;
	}

	int nRet = player_set_uri(pMMPlayer->pPlayer, szPath);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_uri is fail : %d", nRet);
		return FALSE;
	}
//      nRet = player_set_safety_volume(pMMPlayer->pPlayer);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_set_safety_volume is fail : %d", nRet);
//      }

	_vp_mm_register_default_callback(pMMPlayer);

//      nRet = player_enable_sync_handler(pMMPlayer->pPlayer, TRUE);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_enable_sync_handler is fail : %d", nRet);
//              return FALSE;
//      }

	nRet =
	    player_prepare_async(pMMPlayer->pPlayer,
	                         __vp_mm_player_prepare_cb,
	                         (void *) pMMPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_prepare is fail : %d", nRet);
		return FALSE;
	}

	return TRUE;
}

bool vp_mm_player_unrealize(mm_player_handle pPlayerHandle)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}
	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	if (pMMPlayer->bIsRealize == FALSE) {
		VideoLogError("Not realized");
		return FALSE;
	}

	_vp_mm_unregister_default_callback(pMMPlayer);

	int nRet = player_unprepare(pMMPlayer->pPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		VideoLogError("player_unprepare fail : %d", nRet);
		return FALSE;
	}

	if (pMMPlayer->szURL) {
		free(pMMPlayer->szURL);
		pMMPlayer->szURL = NULL;
	}

	if (pMMPlayer->szSubtitleURL) {
		free(pMMPlayer->szSubtitleURL);
		pMMPlayer->szSubtitleURL = NULL;
	}

	if (pMMPlayer->szCookie) {
		free(pMMPlayer->szCookie);
		pMMPlayer->szCookie = NULL;
	}

	if (pMMPlayer->szProxy) {
		free(pMMPlayer->szProxy);
		pMMPlayer->szProxy = NULL;
	}

	pMMPlayer->bIsRealize = FALSE;
	pMMPlayer->bDeactivateSubtitle = FALSE;
	pMMPlayer->bIsMute = FALSE;
	pMMPlayer->bIsClosedCaption = FALSE;
	pMMPlayer->bIsCapturing = FALSE;

	pMMPlayer->nState = VP_MM_PLAYER_STATE_NONE;
	pMMPlayer->nDisplayMode = VP_MM_PLAYER_DISPLAY_LETTER_BOX;

	pMMPlayer->nDuration = 0;
	pMMPlayer->nVideoWidth = 0;
	pMMPlayer->nVidehHeight = 0;

	return TRUE;
}

bool vp_mm_player_is_realize(mm_player_handle pPlayerHandle)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	return pMMPlayer->bIsRealize;

}

bool vp_mm_player_set_callback(mm_player_handle pPlayerHandle,
                               vp_mm_player_callback_t nCallbackType,
                               void *pCallback)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	if (pCallback == NULL) {
		VideoLogError("pCallback is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	switch (nCallbackType) {
	case VP_MM_PLAYER_REALIZE_ASYNC_CB:
		pMMPlayer->pCbFunc->pPrepareCb =
		    (vp_mm_player_prepare_cb) pCallback;
		break;
	case VP_MM_PLAYER_SEEK_COMPLETE_CB:
		pMMPlayer->pCbFunc->pSeekCompleteCb =
		    (vp_mm_player_seek_completed_cb) pCallback;
		break;
	case VP_MM_PLAYER_END_OF_STREAM_CB:
		pMMPlayer->pCbFunc->pEndOfStreamCb =
		    (vp_mm_player_completed_cb) pCallback;
		break;
	case VP_MM_PLAYER_INTERRUPT_CB:
		pMMPlayer->pCbFunc->pInterruptedCb =
		    (vp_mm_player_interrupted_cb) pCallback;
		break;
	case VP_MM_PLAYER_ERROR_CB:
		pMMPlayer->pCbFunc->pErrorCb = (vp_mm_player_error_cb) pCallback;
		break;
	case VP_MM_PLAYER_BUFFERING_CB:
		pMMPlayer->pCbFunc->pBufferingCb =
		    (vp_mm_player_buffering_cb) pCallback;
		break;
	case VP_MM_PLAYER_SUBTITLE_UPDATE_CB:
		pMMPlayer->pCbFunc->pSubtitleUpdateCb =
		    (vp_mm_player_subtitle_updated_cb) pCallback;
		break;
	case VP_MM_PLAYER_PD_MESSAGE_CB:
		pMMPlayer->pCbFunc->pPDMessageCb =
		    (vp_mm_player_pd_message_cb) pCallback;
		break;
	case VP_MM_PLAYER_CAPTURE_VIDEO_CB:
		pMMPlayer->pCbFunc->pCapturedCb =
		    (vp_mm_player_video_captured_cb) pCallback;
		break;
	case VP_MM_PLAYER_MISSED_PLUGIN_CB:
		pMMPlayer->pCbFunc->pMissedPluginCb =
		    (vp_mm_player_missed_plugin_cb) pCallback;
		break;
	case VP_MM_PLAYER_IMAGE_BUFFER_CB:
		pMMPlayer->pCbFunc->pImageBufferCb =
		    (vp_mm_player_image_buffer_updated_cb) pCallback;
		break;
	default:
		VideoLogError("Unknown Callback type : %d", nCallbackType);
		return FALSE;

	}
	return TRUE;
}

bool vp_mm_player_set_user_param(mm_player_handle pPlayerHandle,
                                 void *pParam)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	if (pParam == NULL) {
		VideoLogError("pCallback is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	pMMPlayer->pParam = pParam;

	return TRUE;
}

bool vp_mm_player_set_stream_info(mm_player_handle pPlayerHandle,
                                 void *stream_info)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	if (stream_info == NULL) {
		VideoLogError("pCallback is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	pMMPlayer->stream_info = stream_info;

	return TRUE;
}


bool vp_mm_player_play(mm_player_handle pPlayerHandle)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	VideoLogWarning("");

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	if (pMMPlayer->bIsRealize == FALSE) {
		VideoLogError("Not realized");
		return FALSE;
	}

	VideoLogWarning("[player_start start]");
	int nRet = player_start(pMMPlayer->pPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_start fail : %d", nRet);
		return FALSE;
	}
        int error = SOUND_MANAGER_ERROR_NONE;
	error = sound_manager_acquire_focus(pMMPlayer->stream_info, SOUND_STREAM_FOCUS_FOR_PLAYBACK, NULL);
	if (error != SOUND_MANAGER_ERROR_NONE) {
		VideoLogError("failed to acquire focus [%x]", error);
	}
	VideoLogWarning("[player_start end]");

	return TRUE;

}

bool vp_mm_player_stop(mm_player_handle pPlayerHandle)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	if (pMMPlayer->bIsRealize == FALSE) {
		VideoLogError("Not realized");
		return FALSE;
	}

	VideoLogWarning("[player_stop start]");
	int nRet = player_stop(pMMPlayer->pPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_stop fail : %d", nRet);
		return FALSE;
	}
	VideoLogWarning("[player_stop end]");

	return TRUE;

}

bool vp_mm_player_pause(mm_player_handle pPlayerHandle)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	if (pMMPlayer->bIsRealize == FALSE) {
		VideoLogError("Not realized");
		return FALSE;
	}

	VideoLogWarning("[player_pause start]");
	int nRet = player_pause(pMMPlayer->pPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_pause fail : %d", nRet);
		return FALSE;
	}
        int error = SOUND_MANAGER_ERROR_NONE;
	error = sound_manager_acquire_focus(pMMPlayer->stream_info, SOUND_STREAM_FOCUS_FOR_PLAYBACK, NULL);
	if (error != SOUND_MANAGER_ERROR_NONE) {
		VideoLogError("failed to acquire focus [%x]", error);
	}
	VideoLogWarning("[player_pause end]");

	return TRUE;

}

bool vp_mm_player_get_state(mm_player_handle pPlayerHandle,
                            vp_mm_player_state_t *nState)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;
	player_state_e nPlayerState = PLAYER_STATE_NONE;

	int nRet = player_get_state(pMMPlayer->pPlayer, &nPlayerState);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("PLAYER_ERROR_NONE fail : %d", nRet);
		return FALSE;
	}

	*nState = _vp_mm_player_priv_convert_player_state(nPlayerState, TRUE);

	return TRUE;
}

bool vp_mm_player_get_position(mm_player_handle pPlayerHandle,
                               int *nPosition)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	if (pMMPlayer->bIsRealize == FALSE) {
		VideoLogError("Not realized");
		return FALSE;
	}

	int nPos = 0;

	int nRet = player_get_play_position(pMMPlayer->pPlayer, &nPos);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_get_play_position fail : %d", nRet);
		return FALSE;
	}
	*nPosition = nPos;

	return TRUE;

}

bool vp_mm_player_set_position(mm_player_handle pPlayerHandle,
                               int nPosition)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	if (pMMPlayer->bIsRealize == FALSE) {
		VideoLogError("Not realized");
		return FALSE;
	}

	if (nPosition < 0) {
		VideoLogError("Invalid position value");
		return FALSE;
	}
#if 1				//#ifdef USE_I_FRAMESEKK        //fix. because of changing API
	int nRet =
	    player_set_play_position(pMMPlayer->pPlayer, nPosition, TRUE,
	                             __vp_mm_player_seek_completed_cb,
	                             (void *) pMMPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_position fail : %d", nRet);
		return FALSE;
	}
#else
	int nRet =
	    player_set_position(pMMPlayer->pPlayer, nPosition,
	                        __vp_mm_player_seek_completed_cb,
	                        (void *) pMMPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_position fail : %d", nRet);
		return FALSE;
	}
#endif
	return TRUE;

}

bool vp_mm_player_set_position_by_key_frame(mm_player_handle
        pPlayerHandle, int nPosition)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	if (pMMPlayer->bIsRealize == FALSE) {
		VideoLogError("Not realized");
		return FALSE;
	}

	if (nPosition < 0) {
		VideoLogError("Invalid position value");
		return FALSE;
	}
#if 1				//#ifdef USE_I_FRAMESEKK        //fix. because of changing API
	int nRet =
	    player_set_play_position(pMMPlayer->pPlayer, nPosition, FALSE,
	                             __vp_mm_player_seek_completed_cb,
	                             (void *) pMMPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_position fail : %d", nRet);
		return FALSE;
	}
#else
	int nRet =
	    player_set_position(pMMPlayer->pPlayer, nPosition,
	                        __vp_mm_player_seek_completed_cb,
	                        (void *) pMMPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_position fail : %d", nRet);
		return FALSE;
	}
#endif
	return TRUE;
}


bool vp_mm_player_get_buffering_position(mm_player_handle pPlayerHandle,
        int *nPosition)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	if (pMMPlayer->bIsRealize == FALSE) {
		VideoLogError("Not realized");
		return FALSE;
	}
	int nStart = 0;
	int nCurrent = 0;
	int nRet =
	    player_get_streaming_download_progress(pMMPlayer->pPlayer,
	            &nStart, &nCurrent);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_get_streaming_download_progress fail : %d",
		              nRet);
		return FALSE;
	}

	*nPosition = nCurrent;

	return TRUE;

}

bool vp_mm_player_set_scaling(mm_player_handle pPlayerHandle, bool bScale)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}
	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}
//      int nRet = player_enable_evas_display_scaling(pMMPlayer->pPlayer, bScale);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_enable_evas_display_scaling fail : %d", nRet);
//              return FALSE;
//      }

	pMMPlayer->bIsScaling = bScale;
	return TRUE;

}

bool vp_mm_player_set_loop(mm_player_handle pPlayerHandle, bool bLoop)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}
	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	int nRet = player_set_looping(pMMPlayer->pPlayer, bLoop);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_looping fail : %d", nRet);
		return FALSE;
	}

	pMMPlayer->bIsLooping = bLoop;

	return TRUE;
}

bool vp_mm_player_set_mute(mm_player_handle pPlayerHandle, bool bMute)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	if (pMMPlayer->bIsRealize == FALSE) {
		VideoLogError("Not realized");
		return FALSE;
	}

	int nRet = player_set_mute(pMMPlayer->pPlayer, bMute);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_mute fail : %d", nRet);
		return FALSE;
	}

	pMMPlayer->bIsMute = bMute;

	return TRUE;

}

bool vp_mm_player_get_mute(mm_player_handle pPlayerHandle, bool *bMute)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	if (pMMPlayer->bIsRealize == FALSE) {
		VideoLogError("Not realized");
		return FALSE;
	}
	bool bIsMute = FALSE;
	int nRet = player_is_muted(pMMPlayer->pPlayer, &bIsMute);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_is_muted fail : %d", nRet);
		return FALSE;
	}

	pMMPlayer->bIsMute = bIsMute;
	*bMute = bIsMute;

	return TRUE;
}

bool vp_mm_player_get_duration(mm_player_handle pPlayerHandle,
                               int *nDuration)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	if (pMMPlayer->bIsRealize == FALSE) {
		VideoLogError("Not realized");
		return FALSE;
	}

	int nDur = 0;
	int nRet = player_get_duration(pMMPlayer->pPlayer, &nDur);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_get_duration fail : %d", nRet);
		return FALSE;
	}

	*nDuration = nDur;

	return TRUE;

}

bool vp_mm_player_get_codecinfo(mm_player_handle pPlayerHandle,
                                char **audio_codec, char **video_codec)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	if (pMMPlayer->bIsRealize == FALSE) {
		VideoLogError("Not realized");
		return FALSE;
	}

	int nRet =
	    player_get_codec_info(pMMPlayer->pPlayer, audio_codec,
	                          video_codec);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_get_codec_info fail : %d", nRet);
		return FALSE;
	}

	return TRUE;
}


bool vp_mm_player_set_video_sink(mm_player_handle pPlayerHandle,
                                 int nType, void *pSink)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	int nSinkType = _vp_mm_player_priv_convert_video_sink(nType, FALSE);
	int nRet = player_set_display(pMMPlayer->pPlayer, nSinkType, pSink);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_display fail : %d", nRet);
		return FALSE;
	}

	pMMPlayer->nVideoSink = nType;
	pMMPlayer->pVideoSink = pSink;

	return TRUE;
}

bool vp_mm_player_set_video_rotate(mm_player_handle pPlayerHandle,
                                   int nRotate)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}
#ifdef _NATIVE_BUFFER_SYNC
	int nRot = PLAYER_DISPLAY_ROTATION_NONE;
#else
	int nRot = _vp_mm_player_priv_convert_rotate(nRotate, FALSE);
#endif

	int nRet = player_set_display_rotation(pMMPlayer->pPlayer, nRot);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_display_rotation fail : %d", nRet);
		return FALSE;
	}

	pMMPlayer->nRotate = nRotate;

	return TRUE;
}

bool vp_mm_player_set_display_mode(mm_player_handle pPlayerHandle,
                                   vp_mm_player_display_mode_t
                                   nDisplayMode)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}
	int nMode =
	    _vp_mm_player_priv_convert_display_mode(nDisplayMode, FALSE);
	int nRet = player_set_display_mode(pMMPlayer->pPlayer, nMode);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_display_mode fail : %d", nRet);
		return FALSE;
	}

	pMMPlayer->nDisplayMode = nDisplayMode;

	return TRUE;
}

bool vp_mm_player_get_resolution(mm_player_handle pPlayerHandle,
                                 int *nWidth, int *nHeight)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->bIsRealize == FALSE) {
		VideoLogError("Not realized");
		return FALSE;
	}

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	int nRet = player_get_video_size(pMMPlayer->pPlayer, nWidth, nHeight);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_subtitle_path fail : %d", nRet);
		return FALSE;
	}

	return TRUE;
}

bool vp_mm_player_set_subtitle_url(mm_player_handle pPlayerHandle,
                                   char *szSubtitlePath)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	if (szSubtitlePath == NULL) {
		VideoLogError("Invalid subtitle path info");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	if (pMMPlayer->szSubtitleURL) {
		free(pMMPlayer->szSubtitleURL);
		pMMPlayer->szSubtitleURL = NULL;
	}

	int nRet =
	    player_set_subtitle_path(pMMPlayer->pPlayer, szSubtitlePath);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_subtitle_path fail : %d", nRet);
		return FALSE;
	}

	pMMPlayer->szSubtitleURL = strdup(szSubtitlePath);

	return TRUE;
}

bool vp_mm_player_get_subtitle_url(mm_player_handle pPlayerHandle,
                                   char **szSubtitlePath)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->szSubtitleURL == NULL) {
		VideoLogError("pMMPlayer->szSubtitleURL is NULL");
		return FALSE;
	}

	*szSubtitlePath = strdup(pMMPlayer->szSubtitleURL);

	return TRUE;
}

bool vp_mm_player_set_subtitle_font(mm_player_handle pPlayerHandle,
                                    char *szFamilyName, char *szStyle,
                                    int nSize)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (szFamilyName == NULL) {
		VideoLogError("Invalid szFamilyName info");
		return FALSE;
	}

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}
//      char *szFontConfig = NULL;
//
//      if (szStyle) {
//              szFontConfig = g_strdup_printf("%s %s %d", szFamilyName, szStyle, nSize);
//      }
//      else {
//              szFontConfig = g_strdup_printf("%s %d", szFamilyName, nSize);
//      }
//#if 1


	//int nRet = PLAYER_ERROR_NONE;

//      nRet = player_set_external_subtitle_font(pMMPlayer->pPlayer, szFontConfig);
//      if (szFontConfig) {
//              free(szFontConfig);
//              szFontConfig = NULL;
//      }
	/*
	   //
	   //   if (nRet != PLAYER_ERROR_NONE) {
	   //           _vp_mm_player_print_err(nRet);
	   //           VideoLogError("player_set_external_subtitle_font fail : %d", nRet);
	   //           return FALSE;
	   //   }
	 */
//#endif
	return TRUE;
}

bool vp_mm_player_set_subtitle_font_color(mm_player_handle pPlayerHandle,
        unsigned int nForegroundColor,
        unsigned int nBGColor)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}
#if 1
//      int nRet = PLAYER_ERROR_NONE;

//      nRet = player_set_external_subtitle_font_color(pMMPlayer->pPlayer, nForegroundColor, nBGColor);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_set_external_subtitle_font_color fail : %d", nRet);
//              return FALSE;
//      }
#endif
	return TRUE;

}

bool vp_mm_player_subtitle_ignore_markup_tags(mm_player_handle
        pPlayerHandle,
        bool bIgnoreMarkup)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}
	return TRUE;

}

bool vp_mm_player_subtitle_set_alignment_in_line(mm_player_handle
        pPlayerHandle,
        vp_mm_player_subtitle_halign_t
        type)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}
	return TRUE;

}

bool vp_mm_player_subtitle_set_alignment_horizontal(mm_player_handle
        pPlayerHandle,
        vp_mm_player_subtitle_halign_t
        type)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}
	return TRUE;

}

bool vp_mm_player_subtitle_apply_alignment_right_away(mm_player_handle
        pPlayerHandle)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}
	return TRUE;

}

bool vp_mm_player_set_subtitle_position(mm_player_handle pPlayerHandle,
                                        int nPosition)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	if (pMMPlayer->bIsRealize == FALSE) {
		VideoLogError("Not realized");
		return FALSE;
	}

	int nRet =
	    player_set_subtitle_position_offset(pMMPlayer->pPlayer,
	                                        nPosition);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_subtitle_position fail : %d", nRet);
		return FALSE;
	}

	return TRUE;

}

bool vp_mm_player_set_deactivate_subtitle(mm_player_handle pPlayerHandle,
        bool bDeactivate)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	pMMPlayer->bDeactivateSubtitle = bDeactivate;

	return TRUE;
}



bool vp_mm_player_set_volume(mm_player_handle pPlayerHandle, float fLeft,
                             float fRight)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	if (pMMPlayer->bIsRealize == FALSE) {
		VideoLogError("Not realized");
		return FALSE;
	}

	int nRet = player_set_volume(pMMPlayer->pPlayer, fLeft, fRight);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_volume fail : %d", nRet);
		return FALSE;
	}

	return TRUE;
}

bool vp_mm_player_get_volume(mm_player_handle pPlayerHandle, float *fLeft,
                             float *fRight)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	if (pMMPlayer->bIsRealize == FALSE) {
		VideoLogError("Not realized");
		return FALSE;
	}

	float fLeftVal = 0.0;
	float fRightVal = 0.0;

	int nRet =
	    player_get_volume(pMMPlayer->pPlayer, &fLeftVal, &fRightVal);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_get_volume fail : %d", nRet);
		return FALSE;
	}

	*fLeft = fLeftVal;
	*fRight = fRightVal;

	return TRUE;
}

bool vp_mm_player_set_sound_filter(mm_player_handle pPlayerHandle,
                                   vp_mm_player_sound_filter_t nFilter)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}
//      MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;
//      audio_effect_preset_e nPreset = AUDIO_EFFECT_PRESET_NONE;
//      audio_effect_preset_e nPreset = NULL;
//      bool bIsAvailable = FALSE;

//      nPreset = _vp_mm_player_priv_convert_suond_filter(nFilter, FALSE);
//
//      int nRet =player_audio_effect_preset_is_available (pMMPlayer->pPlayer, nPreset, &bIsAvailable);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_audio_effect_preset_is_available fail : %d", nRet);
//              return FALSE;
//      }
//
//      if (bIsAvailable == false) {
//              VideoLogError("unavailable to [%d] effect.", nPreset);
//              return FALSE;
//      }
//
//      nRet =player_audio_effect_set_preset (pMMPlayer->pPlayer, nPreset);
//       if (nRet != PLAYER_ERROR_NONE) {
//             _vp_mm_player_print_err(nRet);
//             VideoLogError("player_audio_effect_set_preset fail : %d", nRet);
//             return FALSE;
//       }
//      pMMPlayer->nSoundFilter = nFilter;

	return FALSE;
}

bool vp_mm_player_get_sound_filter(mm_player_handle pPlayerHandle,
                                   vp_mm_player_sound_filter_t *nFilter)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	*nFilter = pMMPlayer->nSoundFilter;

	return TRUE;
}


bool vp_mm_player_capture_start(mm_player_handle pPlayerHandle)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	if (pMMPlayer->bIsRealize == FALSE) {
		VideoLogError("Not realized");
		return FALSE;
	}


	int nRet =
	    player_capture_video(pMMPlayer->pPlayer,
	                         __vp_mm_player_video_captured_cb,
	                         (void *) pMMPlayer);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_capture_video fail : %d", nRet);
		return FALSE;
	}

	return TRUE;
}

bool vp_mm_player_is_closed_caption_exist(mm_player_handle pPlayerHandle)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (pMMPlayer->pPlayer == NULL) {
		VideoLogError("Player Handle is NULL");
		return FALSE;
	}

	if (pMMPlayer->bIsRealize == FALSE) {
		VideoLogError("Not realized");
		return FALSE;
	}

//      int nRet = player_has_closed_caption(pMMPlayer->pPlayer, &bExist);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_has_closed_caption fail : %d", nRet);
//              return FALSE;
//      }

	return FALSE;
}

bool vp_mm_player_get_audio_track_count(mm_player_handle pPlayerHandle,
                                        int *nCount)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}
//      MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	int nCnt = 0;
	//int nRet = player_get_track_count(pMMPlayer->pPlayer, PLAYER_TRACK_TYPE_AUDIO, &nCnt);
//      int nRet = player_get_track_count(pMMPlayer->pPlayer, NULL, &nCnt);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_get_track_count fail : %d", nRet);
//              return FALSE;
//      }

	*nCount = nCnt;

	return TRUE;
}


bool vp_mm_player_get_audio_track_language_code(mm_player_handle
        pPlayerHandle, int nIndex,
        char **szCode)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}
	//MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

//      char *szVal = NULL;
//      //int nRet = player_get_track_language_code(pMMPlayer->pPlayer, PLAYER_TRACK_TYPE_AUDIO, nIndex, &szVal);
//      int nRet = player_get_track_language_code(pMMPlayer->pPlayer,NULL, nIndex, &szVal);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_get_track_language_code fail : %d", nRet);
//              return FALSE;
//      }

//      VP_STRDUP(*szCode, szVal);
//      VP_FREE(szVal);

	return TRUE;
}


bool vp_mm_player_set_audio_track(mm_player_handle pPlayerHandle,
                                  int nIndex)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}
	//MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	//int nRet = player_select_track(pMMPlayer->pPlayer, PLAYER_TRACK_TYPE_AUDIO, nIndex);
//      int nRet = player_select_track(pMMPlayer->pPlayer, NULL, nIndex);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_select_track fail : %d", nRet);
//              return FALSE;
//      }

	return TRUE;
}

bool vp_mm_player_get_subtitle_track_count(mm_player_handle pPlayerHandle,
        int *nCount)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}
//      MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;
//
//      int nCnt = 0;
	//int nRet = player_get_track_count(pMMPlayer->pPlayer, PLAYER_TRACK_TYPE_TEXT, &nCnt);

//      int nRet = player_get_track_count(pMMPlayer->pPlayer, NULL, &nCnt);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_get_track_count fail : %d", nRet);
//              return FALSE;
//      }

	*nCount = 0;

	return TRUE;
}


bool vp_mm_player_get_subtitle_track_language_code(mm_player_handle
        pPlayerHandle,
        int nIndex,
        char **szCode)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}
	//MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

//      char *szVal = NULL;
//      //int nRet = player_get_track_language_code(pMMPlayer->pPlayer, PLAYER_TRACK_TYPE_TEXT, nIndex, &szVal);
//
//      int nRet = player_get_track_language_code(pMMPlayer->pPlayer, NULL, nIndex, &szVal);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_get_track_language_code fail : %d", nRet);
//              return FALSE;
//      }
//
//      VP_STRDUP(*szCode, szVal);
//      VP_FREE(szVal);

	return TRUE;
}


bool vp_mm_player_set_subtitle_track(mm_player_handle pPlayerHandle,
                                     int nIndex)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}
//      MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;
//
//      //int nRet = player_select_track(pMMPlayer->pPlayer, PLAYER_TRACK_TYPE_TEXT, nIndex);
//      int nRet = player_select_track(pMMPlayer->pPlayer, NULL, nIndex);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_select_track fail : %d", nRet);
//              return FALSE;
//      }

	return TRUE;
}

#ifdef _SUBTITLE_MULTI_LANGUAGE
bool vp_mm_player_add_subtitle_language(mm_player_handle pPlayerHandle,
                                        int nIndex)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}
//      MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

//      int nRet = player_track_add_subtitle_language(pMMPlayer->pPlayer, nIndex);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_track_add_subtitle_language fail : %d", nRet);
//              return FALSE;
//      }

	return TRUE;
}

bool vp_mm_player_remove_subtitle_language(mm_player_handle pPlayerHandle,
        int nIndex)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}
//      MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

//      int nRet = player_track_remove_subtitle_language(pMMPlayer->pPlayer, nIndex);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_track_remove_subtitle_language fail : %d", nRet);
//              return FALSE;
//      }
//
	return TRUE;
}

bool
vp_mm_player_track_foreach_selected_subtitle_language(mm_player_handle
        pPlayerHandle,
        void *pCallback,
        void *pUserData)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}
//      MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

//      int nRet = player_track_foreach_selected_subtitle_language(pMMPlayer->pPlayer, (player_track_selected_subtitle_language_cb)pCallback, pUserData);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_track_foreach_selected_subtitle_language fail : %d", nRet);
	return FALSE;
//      }

	return TRUE;
}
#endif
bool vp_mm_player_set_cookie(mm_player_handle pPlayerHandle,
                             const char *szCookie)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	if (szCookie == NULL) {
		VideoLogError("szCookie is NULL");
		return FALSE;
	}


	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	int nRet =
	    player_set_streaming_cookie(pMMPlayer->pPlayer, szCookie,
	                                strlen(szCookie));
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_select_track fail : %d", nRet);
		return FALSE;
	}

	return TRUE;
}


bool vp_mm_player_set_proxy(mm_player_handle pPlayerHandle,
                            const char *szProxy)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	if (szProxy == NULL) {
		VideoLogError("szCookie is NULL");
		return FALSE;
	}
	/*
	   MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;
	 */
	return TRUE;
}


bool vp_mm_player_set_hw_decoder(bool bEnable)
{
	if (bEnable) {
		setenv("MM_PLAYER_HW_CODEC_DISABLE", "no", 1);
	} else {
		setenv("MM_PLAYER_HW_CODEC_DISABLE", "yes", 1);
	}

	return TRUE;
}


bool vp_mm_player_set_rate(mm_player_handle pPlayerHandle, float fRate)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	if (fRate < 0.5 || fRate > 1.5) {
		VideoLogError("Invalid Param : %f", fRate);
		return FALSE;
	}

	VideoLogDebug("== SET RATE : %f", fRate);

	int nRet = player_set_playback_rate(pMMPlayer->pPlayer, fRate);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_playback_rate_ex fail : %d", nRet);
		return FALSE;
	}

	return TRUE;
}

bool vp_mm_player_set_zoom(mm_player_handle pPlayerHandle, float fZoom,
                           int nPosX, int nPosY)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}
//      MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

//      int nRet = player_set_x11_display_zoom (pMMPlayer->pPlayer, fZoom, nPosX, nPosY);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_set_x11_display_zoom fail : %d", nRet);
	return FALSE;
//      }
//      return TRUE;
}

bool vp_mm_player_get_zoom_start_position(mm_player_handle pPlayerHandle,
        int *nPosX, int *nPosY)
{
	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	*nPosX = pMMPlayer->nZoomPosX;
	*nPosY = pMMPlayer->nZoomPosY;
	return TRUE;
}


bool vp_mm_player_set_visible(mm_player_handle pPlayerHandle,
                              bool bVisible)
{

	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}

	MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

	int nRet = player_set_display_visible(pMMPlayer->pPlayer, bVisible);
	if (nRet != PLAYER_ERROR_NONE) {
		_vp_mm_player_print_err(nRet);
		VideoLogError("player_set_display_visible fail : %d", nRet);
		return FALSE;
	}
	return TRUE;

}



bool vp_mm_player_set_hub_download_mode(mm_player_handle pPlayerHandle,
                                        bool bDownload)
{

	if (pPlayerHandle == NULL) {
		VideoLogError("pPlayerHandle is NULL");
		return FALSE;
	}
	//MMPlayer *pMMPlayer = (MMPlayer *)pPlayerHandle;

//      int nRet = player_set_video_hub_donwload_mode (pMMPlayer->pPlayer, bDownload);
//      if (nRet != PLAYER_ERROR_NONE) {
//              _vp_mm_player_print_err(nRet);
//              VideoLogError("player_set_video_hub_donwload_mode fail : %d", nRet);
	return FALSE;
//      }
//
//      return TRUE;
}
