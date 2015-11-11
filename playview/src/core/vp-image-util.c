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

#include <image_util.h>

#include "vp-play-type-define.h"
#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-macro-define.h"

/* check temp */
#include "vp-play-log.h"

#include "vp-image-util.h"
#include "vp-util.h"

bool vp_image_util_image_resize_save(const char *szFilePath,
                                     const unsigned char *pSrcBuf,
                                     int nSrcW, int nSrcH, int nDestW,
                                     int nDestH, int nQuality)
{
	if (szFilePath == NULL) {
		VideoLogError("szFilePath is NULL");
		return FALSE;
	}

	if (pSrcBuf == NULL) {
		VideoLogError("pSrcBuf is NULL");
		return FALSE;
	}

	int nRet = IMAGE_UTIL_ERROR_NONE;
	unsigned char *pSaveBuf = NULL;
	unsigned int nResizBufSize = 0;
	int nResizeW = nDestW + (nDestW) % 2;
	int nResizeH = nDestH + (nDestH) % 2;

	nRet =
	    image_util_calculate_buffer_size(nResizeW, nResizeH,
	                                     IMAGE_UTIL_COLORSPACE_RGB888,
	                                     &nResizBufSize);
	if (nRet != IMAGE_UTIL_ERROR_NONE) {
		VideoLogWarning
		("image_util_calculate_buffer_size is fail : [0x%x]", nRet);
	}

	pSaveBuf = calloc(1, sizeof(unsigned char) * nResizBufSize);
	if (pSaveBuf == NULL) {
		VideoLogWarning("pSaveBuf alloc fail");
		return FALSE;
	}

	nRet =
	    vp_util_image_resize(pSaveBuf, &nResizeW, &nResizeH, pSrcBuf,
	                         nSrcW, nSrcH, IMAGE_UTIL_COLORSPACE_RGB888);
	if (nRet != IMAGE_UTIL_ERROR_NONE) {
		VideoLogWarning("vp_util_image_resize is fail : [0x%x]", nRet);
		VP_FREE(pSaveBuf);
		return FALSE;
	}

	nRet =
	    image_util_encode_jpeg(pSaveBuf, nResizeW, nResizeH,
	                           IMAGE_UTIL_COLORSPACE_RGB888, nQuality,
	                           szFilePath);
	if (nRet != IMAGE_UTIL_ERROR_NONE) {
		VideoLogWarning("image_util_encode_jpeg is fail : [0x%x]", nRet);
		VP_FREE(pSaveBuf);
		return FALSE;
	}

	VP_FREE(pSaveBuf);

	return TRUE;
}


bool vp_image_util_image_save(const char *szFilePath,
                              const unsigned char *pSrcBuf, int nSrcW,
                              int nSrcH, int nQuality)
{
	if (szFilePath == NULL) {
		VideoLogError("szFilePath is NULL");
		return FALSE;
	}

	if (pSrcBuf == NULL) {
		VideoLogError("pSrcBuf is NULL");
		return FALSE;
	}

	int nRet = IMAGE_UTIL_ERROR_NONE;

	nRet =
	    image_util_encode_jpeg(pSrcBuf, nSrcW, nSrcH,
	                           IMAGE_UTIL_COLORSPACE_RGB888, nQuality,
	                           szFilePath);
	if (nRet != IMAGE_UTIL_ERROR_NONE) {
		VideoLogWarning("image_util_encode_jpeg is fail : [0x%x]", nRet);
		return FALSE;
	}
	return TRUE;
}
