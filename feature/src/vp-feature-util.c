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
#include "vp-feature-common.h"
#include "vp-feature-util.h"


bool vp_feature_util_calc_aspect_size(int nSrcW, int nSrcH, int nDestW,
				      int nDestH, int nMinW, int nMinH,
				      int *nResultW, int *nResultH)
{
	if (nSrcH == 0 || nSrcW == 0 || nDestW == 0 || nDestW == 0) {
		vp_dbgE("invalid param : (src %d x %d) , (dest %d x %d)", nSrcW,
			nSrcH, nDestW, nDestH);
		return FALSE;
	}

	double src_ratio = 0;
	double dst_ratio = 0;

	int nReturnW = 0;
	int nReturnH = 0;

	src_ratio = (double) nSrcW / nSrcH;
	dst_ratio = (double) nDestW / nDestH;

	if (src_ratio > dst_ratio) {
		nReturnW = nDestW;
		nReturnH = nDestW / src_ratio;
	} else if (src_ratio < dst_ratio) {
		nReturnW = nDestH * src_ratio;
		nReturnH = nDestH;
	} else {
		nReturnW = nDestW;
		nReturnH = nDestH;
	}

	if (nReturnH < nMinH) {
		int tmpHeight = nReturnH;

		src_ratio = (double) nMinH / tmpHeight;

		nReturnW = src_ratio * nReturnW;
		nReturnH = nMinH;
	}

	if (nReturnW < nMinW) {
		int tmpWidth = nReturnW;

		src_ratio = (double) nMinW / tmpWidth;

		nReturnW = nMinW;
		nReturnH = src_ratio * nReturnH;
	}

	*nResultW = nReturnW;
	*nResultH = nReturnH;

	return TRUE;
}
