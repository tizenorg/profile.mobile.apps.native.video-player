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


#include <Elementary.h>

#include "mp-util.h"
#include "mp-video-log.h"
#include "mp-rotate-ctrl.h"
#include "mp-video-list-view-main.h"


static VideosRotateStatus_t nCurrentRotateStatus = VIDEOS_ROTATE_NONE;

void mp_rotate_ctrl_changed_cb(void *pUserData, Evas_Object *pObject,
			       void *pEventInfo)
{
	VideoLogInfo("Rotate state before changing : %d",
		     nCurrentRotateStatus);

	switch (elm_win_rotation_get((Evas_Object *) pObject)) {
	case 0:
		nCurrentRotateStatus = VIDEOS_ROTATE_NONE;
		break;
	case 90:
		nCurrentRotateStatus = VIDEOS_ROTATE_90;
		break;
	case 180:
		nCurrentRotateStatus = VIDEOS_ROTATE_180;
		break;
	case 270:
		nCurrentRotateStatus = VIDEOS_ROTATE_270;
		break;
	default:
		nCurrentRotateStatus = VIDEOS_ROTATE_NONE;
	}

	/* If check top view, when back to main view from item view.
	   preview could not be updated. So we could not keep new preview item.
	 */
	mp_list_view_rotate_view(LIST_UPDATE_TYPE_VIEW);

	/*if (MpVideoListViewIsTopView())
	   {
	   VideoLogInfo("top view is main list view");
	   mp_list_view_rotate_view(LIST_UPDATE_TYPE_VIEW);
	   } */

}


/*///////////////////////////////////////////////////////////////*/
/* External APIs*/

void mp_rotate_ctrl_init(void)
{
	VideoLogInfo("");

	void *pWindowHandle = mp_util_get_main_window_handle();

	if (pWindowHandle) {
		if (elm_win_wm_rotation_supported_get(pWindowHandle)) {
			/*int nRotateType[4] = { 0, 90, 180, 270 };*/
			const int nRotateType[4] = { 0, 90, 180, 270 };
			elm_win_wm_rotation_available_rotations_set(pWindowHandle,
					nRotateType, 4);
		}

		nCurrentRotateStatus = VIDEOS_ROTATE_NONE;

		/*evas_object_smart_callback_add(pWindowHandle, "wm,rotation,changed", mp_rotate_ctrl_changed_cb, NULL);*/
	}
}

void mp_rotate_ctrl_destroy(void)
{
	VideoLogInfo("");
	nCurrentRotateStatus = VIDEOS_ROTATE_NONE;
	/*evas_object_smart_callback_del(mp_util_get_main_window_handle(), "wm,rotation,changed", mp_rotate_ctrl_changed_cb);*/
}

VideosRotateStatus_t mp_rotate_ctrl_get_current_state(void)
{

	bool bValid = true;

#ifdef ENABLE_LANDSCAPE
	bValid = true;
#endif
	if (!bValid) {
		VideoLogInfo("don't support rotated operation.");
		return 0;
	}

	VideoLogInfo("Current rotate status : %d", nCurrentRotateStatus);

	return nCurrentRotateStatus;
}

bool mp_rotate_ctrl_check_landspace()
{
	bool bLandSpace = FALSE;
	if (nCurrentRotateStatus == VIDEOS_ROTATE_90
			|| nCurrentRotateStatus == VIDEOS_ROTATE_270) {
		bLandSpace = TRUE;
	}
	return bLandSpace;
}

bool mp_rotate_ctrl_check_landspace_by_win(Evas_Object *pWin)
{
	if (!pWin) {
		VideoLogError("win is NULL.");
		return FALSE;
	}
	bool bLandSpace = FALSE;
	int bROtation = elm_win_rotation_get(pWin);
	if (bROtation == 90 || bROtation == 270) {
		bLandSpace = TRUE;
	}
	return bLandSpace;
}

void mp_rotate_ctrl_set_state(int nAngle)
{
	switch (nAngle) {
	case 0:
		nCurrentRotateStatus = VIDEOS_ROTATE_NONE;
		break;
	case 90:
		nCurrentRotateStatus = VIDEOS_ROTATE_90;
		break;
	case 180:
		nCurrentRotateStatus = VIDEOS_ROTATE_180;
		break;
	case 270:
		nCurrentRotateStatus = VIDEOS_ROTATE_270;
		break;
	default:
		nCurrentRotateStatus = VIDEOS_ROTATE_NONE;
	}

	VideoLogInfo("CHANGE ROTATE : %d", nCurrentRotateStatus);
}
