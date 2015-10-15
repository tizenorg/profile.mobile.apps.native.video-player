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
#include "mp-video-sound-manager.h"
#include "mp-video-log.h"


static void
__mp_sound_mgr_error_print(int nError)
{
	switch (nError)
	{
	case SOUND_MANAGER_ERROR_NONE:				/**< Successful */
		{
			VideoLogWarning("SOUND_MANAGER_ERROR_NONE");
		}
		break;

	case SOUND_MANAGER_ERROR_OUT_OF_MEMORY:		/**< Out of memory */
		{
			VideoLogWarning("SOUND_MANAGER_ERROR_OUT_OF_MEMORY");
		}
		break;

	case SOUND_MANAGER_ERROR_INVALID_PARAMETER:	/**< Invalid parameter */
		{
			VideoLogWarning("SOUND_MANAGER_ERROR_INVALID_PARAMETER");
		}
		break;

	case SOUND_MANAGER_ERROR_INVALID_OPERATION:	/**< Invalid operation */
		{
			VideoLogWarning("SOUND_MANAGER_ERROR_INVALID_OPERATION");
		}
		break;

	case SOUND_MANAGER_ERROR_NO_PLAYING_SOUND:	/**< No playing sound */
		{
			VideoLogWarning("SOUND_MANAGER_ERROR_NO_PLAYING_SOUND");
		}
		break;

//	case SOUND_MANAGER_ERROR_NO_OPERATION:		/**< No operation */
//		{
//			VideoLogWarning("SOUND_MANAGER_ERROR_NO_OPERATION");
//		}
//		break;

	case SOUND_MANAGER_ERROR_INTERNAL:			/**< Internal error*/
		{
			VideoLogWarning("SOUND_MANAGER_ERROR_NO_OPERATION");
		}
		break;

	default:
		{
			VideoLogWarning("unknow error : [%d]", nError);
		}
		break;
	}
}


void
mp_sound_mgr_init()
{
	int	nResult	= 0;
//	nResult	= sound_manager_set_safety_volume(SOUND_SAFETY_VOL_TYPE_APP_VIDEO);
	if (nResult != SOUND_MANAGER_ERROR_NONE)
	{
		VideoLogError("sound_manager_set_safety_volume failed!!!");
		__mp_sound_mgr_error_print(nResult);

		return;
	}

	nResult	= sound_manager_set_current_sound_type(SOUND_TYPE_MEDIA);
	if (nResult != SOUND_MANAGER_ERROR_NONE)
	{
		VideoLogError("sound_manager_set_current_sound_type failed!!!");
		__mp_sound_mgr_error_print(nResult);

		return;
	}
}


void
mp_sound_mgr_deinit()
{
	int	nResult	= 0;
	//nResult	= sound_manager_unset_safety_volume();
	if (nResult != SOUND_MANAGER_ERROR_NONE)
	{
		VideoLogError("sound_manager_unset_safety_volume failed!!!");
		__mp_sound_mgr_error_print(nResult);

		return;
	}

	nResult	= sound_manager_unset_current_sound_type();
	if (nResult != SOUND_MANAGER_ERROR_NONE)
	{
		VideoLogError("sound_manager_unset_current_sound_type failed!!!");
		__mp_sound_mgr_error_print(nResult);

		return;
	}
}

