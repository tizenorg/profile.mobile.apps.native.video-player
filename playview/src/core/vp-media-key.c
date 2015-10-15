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
#include <media_key.h>
#include <efl_extension.h>

#include "vp-play-macro-define.h"
#include "vp-media-key.h"
#include "vp-play-log.h"


typedef struct _MediaKey {
	Evas_Object *pWin;

	void *pUserData;
	MediaKeyEventCbFunc pEventCb;

} MediaKey;

static void _vp_media_key_destroy_handle(MediaKey *pMediaKey);


/* callback functions */
static void __vp_media_key_event_cb(media_key_e nKey,
				    media_key_event_e nEvent,
				    void *pUserData)
{
	if (!pUserData) {
		VideoLogError("User data is NULL.");
		return;
	}

	bool bReleased = false;
	if (nEvent == MEDIA_KEY_STATUS_RELEASED) {
		bReleased = true;
	}

	if (nEvent == MEDIA_KEY_STATUS_UNKNOWN) {
		VideoLogWarning("unknown key status");
		return;
	}

	MediaKey *pMediaKey = (MediaKey *) pUserData;

	if (pMediaKey->pEventCb == NULL) {
		VideoLogWarning("pMediaKey->pEventCb is NULL");
		return;
	}

	switch (nKey) {
	case MEDIA_KEY_PLAY:
		pMediaKey->pEventCb(VP_MEDIA_KEY_PLAY, bReleased,
				    pMediaKey->pUserData);
		break;
	case MEDIA_KEY_PAUSE:
		pMediaKey->pEventCb(VP_MEDIA_KEY_PAUSE, bReleased,
				    pMediaKey->pUserData);
		break;

	case MEDIA_KEY_PLAYPAUSE:
		pMediaKey->pEventCb(VP_MEDIA_KEY_PLAYPAUSE, bReleased,
				    pMediaKey->pUserData);
		break;
	case MEDIA_KEY_PREVIOUS:
		pMediaKey->pEventCb(VP_MEDIA_KEY_PREVIOUS, bReleased,
				    pMediaKey->pUserData);
		break;
	case MEDIA_KEY_REWIND:
		pMediaKey->pEventCb(VP_MEDIA_KEY_REWIND, bReleased,
				    pMediaKey->pUserData);
		break;
	case MEDIA_KEY_NEXT:
		pMediaKey->pEventCb(VP_MEDIA_KEY_NEXT, bReleased,
				    pMediaKey->pUserData);
		break;
	case MEDIA_KEY_FASTFORWARD:
		pMediaKey->pEventCb(VP_MEDIA_KEY_FASTFORWARD, bReleased,
				    pMediaKey->pUserData);
		break;
	case MEDIA_KEY_STOP:
		pMediaKey->pEventCb(VP_MEDIA_KEY_STOP, bReleased,
				    pMediaKey->pUserData);
		break;
	default:
		VideoLogWarning("Undefined key : %d", nKey);
		break;
	}

}

/* internal functions */
static void _vp_media_key_destroy_handle(MediaKey *pMediaKey)
{
	if (pMediaKey == NULL) {
		VideoLogError("pMediaKey is NULL");
		return;
	}

	VP_FREE(pMediaKey);
}



/* external functions */
media_key_handle vp_media_key_create(Evas_Object *pWin,
				     MediaKeyEventCbFunc pEventCb)
{
	if (pWin == NULL) {
		VideoLogError("pWin is NULL");
		return NULL;
	}

	MediaKey *pMediaKey = NULL;

	pMediaKey = calloc(1, sizeof(MediaKey));

	if (pMediaKey == NULL) {
		VideoLogError("pMediaKey alloc fail");
		return NULL;
	}

	pMediaKey->pEventCb = pEventCb;

	return (media_key_handle) pMediaKey;
}


void vp_media_key_destroy(media_key_handle pMediaKeyHandle)
{
	if (pMediaKeyHandle == NULL) {
		VideoLogError("pMediaKeyHandle is NULL");
		return;
	}

	MediaKey *pMediaKey = (MediaKey *) pMediaKeyHandle;

	vp_media_key_unrealize(pMediaKeyHandle);

	_vp_media_key_destroy_handle(pMediaKey);
}

bool vp_media_key_realize(media_key_handle pMediaKeyHandle)
{
	if (pMediaKeyHandle == NULL) {
		VideoLogError("pMediaKeyHandle is NULL");
		return FALSE;
	}

	MediaKey *pMediaKey = (MediaKey *)pMediaKeyHandle;
	//Eina_Bool nret = eext_win_keygrab_set(pMediaKey->pWin, "XF86AudioMedia");
	//VideoLogError("XF86AudioMedia key returns value: %d", nret);
	int nErr = MEDIA_KEY_ERROR_NONE;
	nErr = media_key_reserve(__vp_media_key_event_cb, (void *)pMediaKey);
	if (nErr != MEDIA_KEY_ERROR_NONE) {
		VideoLogError("media_key_reserve().. %d", nErr);
		return FALSE;
	}
	return TRUE;
}

bool vp_media_key_unrealize(media_key_handle pMediaKeyHandle)
{
	if (pMediaKeyHandle == NULL) {
		VideoLogError("pMediaKeyHandle is NULL");
		return FALSE;
	}

	media_key_release();
	//MediaKey *pMediaKey = (MediaKey *) pMediaKeyHandle;

	//Eina_Bool nret = eext_win_keygrab_unset(pMediaKey->pWin, "XF86AudioMedia");
	//VideoLogError("XF86AudioMedia key returns value: %d", nret);

	return TRUE;
}

bool vp_media_key_set_user_data(media_key_handle pMediaKeyHandle,
				void *pUserData)
{
	if (pMediaKeyHandle == NULL) {
		VideoLogError("pMediaKeyHandle is NULL");
		return FALSE;
	}

	MediaKey *pMediaKey = (MediaKey *) pMediaKeyHandle;

	pMediaKey->pUserData = pUserData;

	return TRUE;
}
