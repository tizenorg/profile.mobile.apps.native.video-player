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

#include <pthread.h>
#include <metadata_extractor.h>
#include <image_util.h>

#include "vp-feature-common.h"

#include "vp-feature-util.h"
#include "vp-file-util.h"
#include "vp-thumb-gen.h"
#include "vp-util.h"


static pthread_t g_thread_id;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;


#define	THUMB_GEN_FILE_NAME_PREFIX	"%s/thumb_%02d.jpg"
#define	THUMB_GEN_FILE_NAME_PREFIX_LEN	14


typedef struct _ThumbGen {
	metadata_extractor_h meta_h;

	char *szMediaURL;
	bool bIsRealize;

	int nSrcWidth;
	int nSrcHeight;
	int nDuration;
	bool bResize;

	int nRotate;

	int nStartPosition;
	int nEndPosition;
	int nStartIdx;
	int nDestWidth;
	int nDestHeight;
	int nGenCount;
	int nInterval;
	char *szSaveDir;
	bool bAcuurate;
	bool bScale;
	bool bIsStart;
	bool bIsCancle;
	void *pUserData;

	int nGenIdx;

	vp_thumb_gen_progress_cb progress_cb;
	vp_thumb_gen_complete_cb complete_cb;
	vp_thumb_gen_cancle_cb cancle_cb;

	Ecore_Pipe *pProgressPipe;
	Ecore_Pipe *pCompletePipe;
	Ecore_Pipe *pCanclePipe;
} ThumbGen;


static void _vp_thumb_gen_destroy_handle(ThumbGen *pThumbGen);


/* callback functions */


/* internal functions */
static void _vp_thumb_gen_lock()
{
	pthread_mutex_lock(&g_mutex);
}

static void _vp_thumb_gen_unlock()
{
	pthread_mutex_unlock(&g_mutex);
}

static void *_vp_thumb_gen_thread_loop(void *pUserData)
{
	if (pUserData == NULL) {
		return NULL;
	}

	ThumbGen *pThumbGen = (ThumbGen *) pUserData;

	while (1) {
		_vp_thumb_gen_lock();
		if (pThumbGen->bIsRealize == FALSE) {
			vp_dbgW(" == pThumbGen->bIsRealize is Fail ==");
			_vp_thumb_gen_unlock();
			break;
		}

		bool bRunStatus = FALSE;
		if (pThumbGen->bIsStart && pThumbGen->nGenCount > 0) {
			bRunStatus = TRUE;
		}
		_vp_thumb_gen_unlock();

		if (bRunStatus) {
			bool bSuccess = TRUE;
			char *szURL = NULL;
			void *pFrame = NULL;
			int nFrameSize = 0;
			unsigned long nPosition = 0;
			int nRet = METADATA_EXTRACTOR_ERROR_NONE;
			unsigned char *pSaveBuf = NULL;
			unsigned char *pRotBuf = NULL;
			_vp_thumb_gen_lock();

			vp_dbgW("idx : %d", pThumbGen->nGenIdx);
			int nSrcWidth = pThumbGen->nSrcWidth;
			int nSrcHeight = pThumbGen->nSrcHeight;
			int nSaveWidth = pThumbGen->nDestWidth;
			int nSaveHeight = pThumbGen->nDestHeight;
			int nFileNameSize =
			    strlen(pThumbGen->szSaveDir) +
			    THUMB_GEN_FILE_NAME_PREFIX_LEN;
			bool bResize = pThumbGen->bResize;

			szURL = calloc(1, sizeof(char) * nFileNameSize);

			if (szURL != NULL) {
				snprintf(szURL, nFileNameSize,
				         THUMB_GEN_FILE_NAME_PREFIX,
				         pThumbGen->szSaveDir,
				         pThumbGen->nGenIdx + pThumbGen->nStartIdx);
			}

			nPosition =
			    pThumbGen->nStartPosition +
			    (pThumbGen->nGenIdx * pThumbGen->nInterval);

			nRet =
			    metadata_extractor_get_frame_at_time(pThumbGen->meta_h,
			            nPosition,
			            pThumbGen->bAcuurate,
			            &pFrame,
			            &nFrameSize);
			if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
				vp_dbgE
				("metadata_extractor_get_frame_at_time is fail : [0x%x]",
				 nRet);
				bSuccess = FALSE;
			}
			_vp_thumb_gen_unlock();

			if (pFrame == NULL) {
				vp_dbgE("pFrame is NULL");
				bSuccess = FALSE;
			}

			if (bSuccess) {
				if (bResize) {
					unsigned int nResizBufSize = 0;

					nRet = image_util_calculate_buffer_size(nSaveWidth,
					                                        nSaveHeight,
					                                        IMAGE_UTIL_COLORSPACE_RGB888,
					                                        &nResizBufSize);
					if (nRet != IMAGE_UTIL_ERROR_NONE) {
						vp_dbgE
						("image_util_calculate_buffer_size is fail : [0x%x]",
						 nRet);
						bSuccess = FALSE;
					}
					vp_dbgW("ResizeBuf Size : %d", nResizBufSize);

					pSaveBuf =
					    calloc(1, sizeof(unsigned char) * nResizBufSize);
					if (pSaveBuf == NULL) {
						vp_dbgE("pSaveBuf alloc fail");
						bSuccess = FALSE;
					} else {
						nRet = vp_util_image_resize(pSaveBuf,
						                            &nSaveWidth,
						                            &nSaveHeight,
						                            (unsigned char *)
						                            pFrame, nSrcWidth,
						                            nSrcHeight,
						                            IMAGE_UTIL_COLORSPACE_RGB888);
						if (nRet != IMAGE_UTIL_ERROR_NONE) {
							vp_dbgE
							("vp_util_image_resize is fail : [0x%x]",
							 nRet);
							bSuccess = FALSE;
						}
					}
				} else {
					pSaveBuf = (unsigned char *) pFrame;
				}
			}

			if (pThumbGen->nRotate != 0) {
				image_util_rotation_e nRot = IMAGE_UTIL_ROTATION_NONE;
				if (pThumbGen->nRotate == 90) {
					nRot = IMAGE_UTIL_ROTATION_90;
				} else if (pThumbGen->nRotate == 180) {
					nRot = IMAGE_UTIL_ROTATION_180;
				} else if (pThumbGen->nRotate == 270) {
					nRot = IMAGE_UTIL_ROTATION_270;
				}

				if (nRot != IMAGE_UTIL_ROTATION_NONE) {
					unsigned int nResizBufSize = 0;
					int nSrcW = nSaveWidth;
					int nSrcH = nSaveHeight;
					nRet = image_util_calculate_buffer_size(nSaveWidth,
					                                        nSaveHeight,
					                                        IMAGE_UTIL_COLORSPACE_RGB888,
					                                        &nResizBufSize);
					if (nRet != IMAGE_UTIL_ERROR_NONE) {
						vp_dbgE
						("image_util_calculate_buffer_size is fail : [0x%x]",
						 nRet);
						bSuccess = FALSE;
					}

					pRotBuf =
					    calloc(1, sizeof(unsigned char) * nResizBufSize);
					if (pRotBuf == NULL) {
						vp_dbgE("pRotBuf alloc fail");
						bSuccess = FALSE;
					} else {
						vp_dbgW("ResizeBuf Size : %d", nResizBufSize);
						nRet = vp_util_image_rotate(pRotBuf,
						                            &nSaveWidth,
						                            &nSaveHeight,
						                            nRot, pSaveBuf,
						                            nSrcW, nSrcH,
						                            IMAGE_UTIL_COLORSPACE_RGB888);
						if (nRet != IMAGE_UTIL_ERROR_NONE) {
							vp_dbgE
							("vp_util_image_rotate is fail : [0x%x]",
							 nRet);
							bSuccess = FALSE;
						}
					}
				}
			}

			if (bSuccess) {
				vp_sdbg("w:%d, h:%d, URL : %s", nSaveWidth, nSaveHeight,
				        szURL);
				if (pRotBuf) {
					nRet = image_util_encode_jpeg(pRotBuf,
					                              nSaveWidth,
					                              nSaveHeight,
					                              IMAGE_UTIL_COLORSPACE_RGB888,
					                              100, szURL);
					if (nRet != IMAGE_UTIL_ERROR_NONE) {
						vp_dbgE("image_util_encode_jpeg is fail : [0x%x]",
						        nRet);
						bSuccess = FALSE;
					}
				} else {
					nRet = image_util_encode_jpeg(pSaveBuf,
					                              nSaveWidth,
					                              nSaveHeight,
					                              IMAGE_UTIL_COLORSPACE_RGB888,
					                              100, szURL);
					if (nRet != IMAGE_UTIL_ERROR_NONE) {
						vp_dbgE("image_util_encode_jpeg is fail : [0x%x]",
						        nRet);
						bSuccess = FALSE;
					}
				}
			}

			if (bResize) {
				VP_FREE(pSaveBuf);
			}
			VP_FREE(pRotBuf);
			VP_FREE(pFrame);

			_vp_thumb_gen_lock();
			if (pThumbGen->progress_cb) {
				vp_dbgW(" ==> %p, %p", pThumbGen, pThumbGen->progress_cb);
				pThumbGen->progress_cb(bSuccess, pThumbGen->nGenIdx,
				                       szURL, nPosition,
				                       pThumbGen->pUserData);
			}

			pThumbGen->nGenIdx++;
			VP_FREE(szURL);
			_vp_thumb_gen_unlock();

			if (pThumbGen->nGenIdx == pThumbGen->nGenCount) {
				/* call complete callback */
				pThumbGen->bIsStart = FALSE;

				vp_thumb_gen_complete_cb func = pThumbGen->complete_cb;

				if (func) {
					func(pThumbGen->nGenCount, pThumbGen->pUserData);
				}
				pthread_exit(0);
			}

		}

		_vp_thumb_gen_lock();
		if (pThumbGen->bIsCancle) {
			if (pThumbGen->cancle_cb) {
				vp_dbgW(" ==> %p, %p", pThumbGen, pThumbGen->cancle_cb);
				pThumbGen->cancle_cb(pThumbGen->pUserData);
			}

			pThumbGen->bIsStart = FALSE;
		}
		_vp_thumb_gen_unlock();

		usleep(1000);
	}
	vp_dbgW(" == Thread End ==");

	return NULL;
}


static void _vp_thumb_gen_destroy_handle(ThumbGen *pThumbGen)
{
	if (pThumbGen == NULL) {
		vp_dbgE("hThumbGen is NULL");
		return;
	}

	int nRet = METADATA_EXTRACTOR_ERROR_NONE;
	_vp_thumb_gen_lock();

	pThumbGen->progress_cb = NULL;
	pThumbGen->complete_cb = NULL;
	pThumbGen->cancle_cb = NULL;

	nRet = metadata_extractor_destroy(pThumbGen->meta_h);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		vp_dbgE("metadata_extractor_destroy is fail : [0x%x]", nRet);
	}

	VP_FREE(pThumbGen->szMediaURL);
	VP_FREE(pThumbGen->szSaveDir);

	VP_FREE(pThumbGen);

	_vp_thumb_gen_unlock();

}


/* external functions */
vp_thumb_gen_h vp_thumb_gen_create(const char *szMediaURL)
{
	if (szMediaURL == NULL) {
		vp_dbgE("szMediaURL is NULL");
		return NULL;
	}

	ThumbGen *pThumbGen = calloc(1, sizeof(ThumbGen));
	if (pThumbGen == NULL) {
		vp_dbgE("ThumbGen alloc is fail");
		return NULL;
	}


	VP_STRDUP(pThumbGen->szMediaURL, szMediaURL);

	int nRet = METADATA_EXTRACTOR_ERROR_NONE;
	nRet = metadata_extractor_create(&(pThumbGen->meta_h));
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		_vp_thumb_gen_destroy_handle(pThumbGen);
		vp_dbgE("metadata_extractor_create is fail : [0x%x]", nRet);
		return NULL;
	}

	nRet = metadata_extractor_set_path(pThumbGen->meta_h, szMediaURL);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		_vp_thumb_gen_destroy_handle(pThumbGen);
		vp_dbgE("metadata_extractor_set_path is fail : [0x%x]", nRet);
		return NULL;
	}

	char *szVal = NULL;

	nRet =
	    metadata_extractor_get_metadata(pThumbGen->meta_h,
	                                    METADATA_DURATION, &szVal);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		_vp_thumb_gen_destroy_handle(pThumbGen);
		vp_dbgE("metadata_extractor_get_metadata is fail : [0x%x]", nRet);
		return NULL;
	}
	pThumbGen->nDuration = atoi(szVal);
	VP_FREE(szVal);

	nRet =
	    metadata_extractor_get_metadata(pThumbGen->meta_h,
	                                    METADATA_ROTATE, &szVal);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		_vp_thumb_gen_destroy_handle(pThumbGen);
		vp_dbgE("metadata_extractor_get_metadata is fail : [0x%x]", nRet);
		return NULL;
	}

	if (szVal == NULL) {

		pThumbGen->nRotate = 0;
	} else {
		pThumbGen->nRotate = atoi(szVal);
		VP_FREE(szVal);
	}

	nRet =
	    metadata_extractor_get_metadata(pThumbGen->meta_h,
	                                    METADATA_VIDEO_WIDTH, &szVal);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		_vp_thumb_gen_destroy_handle(pThumbGen);
		vp_dbgE("metadata_extractor_get_metadata is fail : [0x%x]", nRet);
		return NULL;
	}
	if (szVal == NULL) {
		pThumbGen->nSrcWidth = 0;
	} else {
		pThumbGen->nSrcWidth = atoi(szVal);
		VP_FREE(szVal);
	}

	nRet =
	    metadata_extractor_get_metadata(pThumbGen->meta_h,
	                                    METADATA_VIDEO_HEIGHT, &szVal);
	if (nRet != METADATA_EXTRACTOR_ERROR_NONE) {
		_vp_thumb_gen_destroy_handle(pThumbGen);
		vp_dbgE("metadata_extractor_get_metadata is fail : [0x%x]", nRet);
		return NULL;
	}
	if (szVal == NULL) {
		pThumbGen->nSrcHeight = 0;
	} else {
		pThumbGen->nSrcHeight = atoi(szVal);
		VP_FREE(szVal);
	}

	pthread_mutex_init(&g_mutex, NULL);

	return (vp_thumb_gen_h) pThumbGen;
}

void vp_thumb_gen_destroy(vp_thumb_gen_h hThumbGen)
{
	if (hThumbGen == NULL) {
		vp_dbgE("hThumbGen is NULL");
		return;
	}
	vp_dbgW("");

	ThumbGen *pThumbGen = (ThumbGen *) hThumbGen;

	vp_thumb_gen_unrealize((vp_thumb_gen_h) pThumbGen);
	_vp_thumb_gen_destroy_handle(pThumbGen);

	pthread_mutex_destroy(&g_mutex);

}

bool vp_thumb_gen_realize(vp_thumb_gen_h hThumbGen)
{
	if (hThumbGen == NULL) {
		vp_dbgE("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *) hThumbGen;

	pThumbGen->bIsRealize = TRUE;

	if (pThumbGen->bScale) {
		int nResultW = 0;
		int nResultH = 0;
		if (!vp_feature_util_calc_aspect_size(pThumbGen->nSrcWidth,
		                                      pThumbGen->nSrcHeight,
		                                      pThumbGen->nDestWidth,
		                                      pThumbGen->nDestHeight,
		                                      0, 0,
		                                      &nResultW, &nResultH)) {
			vp_dbgE("vp_feature_util_calc_aspect_size is fail");

		} else {
			pThumbGen->nDestWidth = nResultW;
			pThumbGen->nDestHeight = nResultH;
		}
		if (pThumbGen->nDestWidth % 2 != 0) {
			pThumbGen->nDestWidth += 1;
		}
		if (pThumbGen->nDestHeight % 2 != 0) {
			pThumbGen->nDestHeight += 1;
		}

	}
#if 1
	if (pthread_create
	        (&g_thread_id, NULL, _vp_thumb_gen_thread_loop,
	         (void *) pThumbGen) != 0) {
		vp_dbgE("pthread_create fail");
		pThumbGen->bIsRealize = FALSE;
		return FALSE;
	}
#else
	int status = 0;
	pthread_attr_t thread_attr;

	status = pthread_attr_init(&thread_attr);
	if (0 == status) {
		status = pthread_attr_setdetachstate(&thread_attr,
		                                     PTHREAD_CREATE_DETACHED);
		if (0 == status) {
			if (pthread_create
			        (&g_thread_id, &thread_attr, _vp_thumb_gen_thread_loop,
			         (void *) pThumbGen) != 0) {
				vp_dbgE("pthread_create fail");
				pThumbGen->bIsRealize = FALSE;
				return FALSE;
			}
		} else {
			vp_dbgE("pthread_attr_setdetachstate fail");
			pThumbGen->bIsRealize = FALSE;
			return FALSE;
		}
	} else {
		vp_dbgE("pthread_attr_init fail");
		pThumbGen->bIsRealize = FALSE;
		return FALSE;
	}

	if (pthread_attr_destroy(&thread_attr) != 0) {
		vp_dbgE("Cannot destroy the attribute object");
		pThumbGen->bIsRealize = FALSE;
		return FALSE;
	}
#endif
	return TRUE;
}

bool vp_thumb_gen_unrealize(vp_thumb_gen_h hThumbGen)
{
	if (hThumbGen == NULL) {
		vp_dbgE("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *) hThumbGen;

	_vp_thumb_gen_lock();
	pThumbGen->bIsStart = FALSE;
	_vp_thumb_gen_unlock();

	if (pThumbGen->bIsRealize == TRUE) {
		_vp_thumb_gen_lock();
		pThumbGen->bIsRealize = FALSE;
		_vp_thumb_gen_unlock();

		int status = 0;
		pthread_join(g_thread_id, (void **) &status);

		//pthread_detach(g_thread_id);
	}

	return TRUE;
}

bool vp_thumb_gen_is_realize(vp_thumb_gen_h hThumbGen, bool *bIsRealize)
{
	if (hThumbGen == NULL) {
		vp_dbgE("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *) hThumbGen;

	*bIsRealize = pThumbGen->bIsRealize;

	return TRUE;
}

bool vp_thumb_gen_set_dest_size(vp_thumb_gen_h hThumbGen, int nWidth,
                                int nHeight)
{
	if (hThumbGen == NULL) {
		vp_dbgE("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *) hThumbGen;

	_vp_thumb_gen_lock();

	pThumbGen->nDestWidth = nWidth;
	pThumbGen->nDestHeight = nHeight;

	if (nWidth != pThumbGen->nSrcWidth
	        || nHeight != pThumbGen->nSrcHeight) {
		pThumbGen->bResize = TRUE;
	}

	_vp_thumb_gen_unlock();

	return TRUE;
}

bool vp_thumb_gen_set_save_directory(vp_thumb_gen_h hThumbGen,
                                     char *szSaveDir)
{
	if (hThumbGen == NULL) {
		vp_dbgE("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *) hThumbGen;

	_vp_thumb_gen_lock();

	if (!vp_file_exists(szSaveDir)) {
		if (!vp_mkdir(szSaveDir)) {
			vp_dbgE("Make directory Fail : %s", szSaveDir);
		}
	}

	VP_STRDUP(pThumbGen->szSaveDir, szSaveDir);

	_vp_thumb_gen_unlock();

	return TRUE;
}

bool vp_thumb_gen_set_start_position(vp_thumb_gen_h hThumbGen,
                                     int nPosition)
{
	if (hThumbGen == NULL) {
		vp_dbgE("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *) hThumbGen;

	_vp_thumb_gen_lock();

	pThumbGen->nStartPosition = nPosition;

	_vp_thumb_gen_unlock();

	return TRUE;
}

bool vp_thumb_gen_set_end_position(vp_thumb_gen_h hThumbGen,
                                   int nPosition)
{
	if (hThumbGen == NULL) {
		vp_dbgE("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *) hThumbGen;

	_vp_thumb_gen_lock();

	pThumbGen->nEndPosition = nPosition;

	_vp_thumb_gen_unlock();

	return TRUE;
}

bool vp_thumb_gen_set_start_index(vp_thumb_gen_h hThumbGen, int nIdx)
{
	if (hThumbGen == NULL) {
		vp_dbgE("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *) hThumbGen;

	_vp_thumb_gen_lock();

	pThumbGen->nStartIdx = nIdx;

	_vp_thumb_gen_unlock();

	return TRUE;
}

bool vp_thumb_gen_set_count(vp_thumb_gen_h hThumbGen, int nCount)
{
	if (hThumbGen == NULL) {
		vp_dbgE("hThumbGen is NULL");
		return FALSE;
	}

	if (nCount <= 0) {
		vp_dbgE("Invalid  nCount ");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *) hThumbGen;

	_vp_thumb_gen_lock();

	pThumbGen->nGenCount = nCount;
	if (pThumbGen->nEndPosition <= 0) {
		vp_dbgW("It doesn't set value of end position.");
		pThumbGen->nEndPosition = pThumbGen->nDuration;
	}
	pThumbGen->nInterval =
	    (pThumbGen->nEndPosition - pThumbGen->nStartPosition) / nCount;

	_vp_thumb_gen_unlock();

	return TRUE;
}


bool vp_thumb_gen_set_acuurate(vp_thumb_gen_h hThumbGen, bool bAcuurate)
{
	if (hThumbGen == NULL) {
		vp_dbgE("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *) hThumbGen;

	_vp_thumb_gen_lock();

	pThumbGen->bAcuurate = bAcuurate;

	_vp_thumb_gen_unlock();

	return TRUE;
}

bool vp_thumb_gen_set_scale(vp_thumb_gen_h hThumbGen, bool bSacle)
{
	if (hThumbGen == NULL) {
		vp_dbgE("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *) hThumbGen;

	_vp_thumb_gen_lock();

	pThumbGen->bScale = bSacle;

	_vp_thumb_gen_unlock();

	return TRUE;
}

bool vp_thumb_gen_start(vp_thumb_gen_h hThumbGen)
{
	if (hThumbGen == NULL) {
		vp_dbgE("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *) hThumbGen;

	if (pThumbGen->bIsRealize == FALSE) {
		vp_dbgE("Not yet realize state");
		return FALSE;
	}

	if (pThumbGen->bIsStart == TRUE) {
		vp_dbgE("Already thumb gen start");
		return FALSE;
	}

	_vp_thumb_gen_lock();

	pThumbGen->bIsStart = TRUE;

	_vp_thumb_gen_unlock();

	return TRUE;
}

bool vp_thumb_gen_cancle(vp_thumb_gen_h hThumbGen)
{
	if (hThumbGen == NULL) {
		vp_dbgE("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *) hThumbGen;

	if (pThumbGen->bIsRealize == FALSE) {
		vp_dbgE("Not yet realize state");
		return FALSE;
	}

	if (pThumbGen->bIsCancle == TRUE) {
		vp_dbgE("Already thumb gen cancles");
		return FALSE;
	}

	_vp_thumb_gen_lock();

	pThumbGen->bIsCancle = TRUE;

	_vp_thumb_gen_unlock();

	return TRUE;
}

bool vp_thumb_gen_set_user_data(vp_thumb_gen_h hThumbGen, void *pUserData)
{
	if (hThumbGen == NULL) {
		vp_dbgE("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *) hThumbGen;

	_vp_thumb_gen_lock();

	pThumbGen->pUserData = pUserData;

	_vp_thumb_gen_unlock();

	return TRUE;
}

bool vp_thumb_gen_set_progress_cb(vp_thumb_gen_h hThumbGen,
                                  vp_thumb_gen_progress_cb progress_cb)
{
	if (hThumbGen == NULL) {
		vp_dbgE("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *) hThumbGen;

	_vp_thumb_gen_lock();

	pThumbGen->progress_cb = progress_cb;

	_vp_thumb_gen_unlock();

	return TRUE;
}

bool vp_thumb_gen_set_complete_cb(vp_thumb_gen_h hThumbGen,
                                  vp_thumb_gen_complete_cb complete_cb)
{
	if (hThumbGen == NULL) {
		vp_dbgE("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *) hThumbGen;

	_vp_thumb_gen_lock();

	pThumbGen->complete_cb = complete_cb;

	_vp_thumb_gen_unlock();

	return TRUE;
}

bool vp_thumb_gen_set_cancle_cb(vp_thumb_gen_h hThumbGen,
                                vp_thumb_gen_cancle_cb cancle_cb)
{
	if (hThumbGen == NULL) {
		vp_dbgE("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *) hThumbGen;

	_vp_thumb_gen_lock();

	pThumbGen->cancle_cb = cancle_cb;

	_vp_thumb_gen_unlock();

	return TRUE;
}
