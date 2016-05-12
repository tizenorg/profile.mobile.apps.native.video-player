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
#include <string.h>
#include <libintl.h>
#include <device/power.h>
#include <storage/storage.h>

#include "vp-util.h"

#define CLOUD_FOLDER			"/opt/usr/media/.cloud"

#define OTG_FOLDER			"/opt/storage/usb"
#define PERSONAL_PAGE_FOLDER		"/opt/storage/PersonalStorage"
#define PKGNAME 			"videos"

#define VP_UTIL_LABEL_DEVICE_MEMORY	__vp_util_get_string("IDS_VIDEO_SBODY_DEVICE_STORAGE")
#define VP_UTIL_LABEL_SD_CARD		__vp_util_get_string("IDS_ST_BODY_SD_CARD")
#define VP_UTIL_LABEL_OTG		__vp_util_get_string("IDS_VPL_TAB_EXTERNAL_STORAGE")
#define VP_UTIL_LABEL_PERSONAL_PAGE	__vp_util_get_string("IDS_GALLERY_BODY_PERSONAL_STORAGE")
#define VP_UTIL_LABEL_UNKNOWN		__vp_util_get_string("VIDEOS_VIEW_DROPBOX_NAME")
#define VP_UTIL_LABEL_CLOUD			__vp_util_get_string("IDS_CLOUD_BODY_DROPBOX_ABB")


#define DIR_PATH_LEN_MAX		4096

typedef enum __Vp_Storage Vp_Storage;
enum __Vp_Storage {
	VP_STORAGE_NONE,
	VP_STORAGE_PHONE,
	VP_STORAGE_CLOUD,
	VP_STORAGE_MMC,
	VP_STORAGE_OTG,
	VP_STORAGE_PERSONAL_PAGE,
	VP_STORAGE_MAX
};

static const int RGB_BPP = 3;
static const int RGBA_BPP = 4;
typedef unsigned char uchar;
typedef struct _rgb888 {
	uchar r;
	uchar g;
	uchar b;
} rgb888;

typedef struct _rgba8888 {
	uchar r;
	uchar g;
	uchar b;
	uchar a;
} rgba8888;

typedef struct _transform {
	int index;
	float coef;
} transform;

static char *__vp_util_get_string(const char *ID)
{
	char *str;
	if (strstr(ID, "IDS_COM") || strstr(ID, "IDS_IDLE")) {
		str = dgettext("sys_string", ID);
	} else {
		str = dgettext("video-player", ID);
	}
	return str;
}

static Vp_Storage __vp_util_get_storage_type(const char *filepath)
{
	if (!filepath) {
		return VP_STORAGE_NONE;
	}

	char* PHONE_FOLDER = NULL;
	char* MEMORY_FOLDER = NULL;
	Vp_Storage store_type = VP_STORAGE_NONE;

	storage_get_root_directory(STORAGE_TYPE_INTERNAL, &PHONE_FOLDER);
	storage_get_root_directory(STORAGE_TYPE_EXTERNAL, &MEMORY_FOLDER);

	if (g_str_has_prefix(filepath, CLOUD_FOLDER)) {
		store_type = VP_STORAGE_CLOUD;/*store in cloud server*/
	} else if (g_str_has_prefix(filepath, PHONE_FOLDER)) {
		store_type = VP_STORAGE_PHONE;/*store in phone*/
	} else if (g_str_has_prefix(filepath, MEMORY_FOLDER)) {
		store_type = VP_STORAGE_MMC;	/*store in MMC*/
	} else if (g_str_has_prefix(filepath, OTG_FOLDER)) {
		store_type = VP_STORAGE_OTG; /*store in OTG*/
	} else if (g_str_has_prefix(filepath, PERSONAL_PAGE_FOLDER)) {
		store_type = VP_STORAGE_PERSONAL_PAGE; /*store in persona page*/
	} else {
		store_type = VP_STORAGE_NONE;
	}

	free(PHONE_FOLDER);
	free(MEMORY_FOLDER);

	return store_type;
}

static char *__vp_util_get_logic_path(const char *full_path)
{
	if (!full_path) {
		return NULL;
	}

	char* PHONE_FOLDER = NULL;
	char* MEMORY_FOLDER = NULL;
	Vp_Storage store_type = VP_STORAGE_NONE;
	int root_len = 0;

	storage_get_root_directory(STORAGE_TYPE_INTERNAL, &PHONE_FOLDER);
	storage_get_root_directory(STORAGE_TYPE_EXTERNAL, &MEMORY_FOLDER);

	store_type = __vp_util_get_storage_type(full_path);

	switch (store_type) {
	case VP_STORAGE_PHONE:
		root_len = strlen(PHONE_FOLDER);
		break;
	case VP_STORAGE_MMC:
		root_len = strlen(MEMORY_FOLDER);
		break;
	case VP_STORAGE_OTG:
		root_len = strlen(OTG_FOLDER);
		break;
	case VP_STORAGE_PERSONAL_PAGE:
		root_len = strlen(PERSONAL_PAGE_FOLDER);
		break;
	default:
		free(PHONE_FOLDER);
		free(MEMORY_FOLDER);
		return NULL;
	}

	free(PHONE_FOLDER);
	free(MEMORY_FOLDER);

	/*size of path is DIR_PATH_LEN_MAX+1*/
	char *logic_path = NULL;
	logic_path = (char *)malloc(DIR_PATH_LEN_MAX + 1);
	if (logic_path == NULL) {
		return NULL;
	}

	memset(logic_path, 0, DIR_PATH_LEN_MAX + 1);

	g_strlcpy(logic_path, full_path + root_len, DIR_PATH_LEN_MAX);

	if (strlen(logic_path) == 0) {
		g_strlcpy(logic_path, "/", DIR_PATH_LEN_MAX);
	}

	return logic_path;
}


char *vp_util_convert_file_location(const char *szFileLocation)
{
	if (!szFileLocation) {
		return NULL;
	}

	Vp_Storage store_type = VP_STORAGE_NONE;
	char *logic_path = NULL;
	char *file_location = NULL;

	store_type = __vp_util_get_storage_type(szFileLocation);

	if (store_type == VP_STORAGE_CLOUD) {
		file_location = g_strdup(VP_UTIL_LABEL_CLOUD);
		return file_location;
	}


	logic_path = __vp_util_get_logic_path(szFileLocation);

	if (!logic_path) {
		return NULL;
	}

	if (store_type == VP_STORAGE_MMC) {
		file_location = g_strconcat(VP_UTIL_LABEL_SD_CARD, logic_path, NULL);
	} else if (store_type == VP_STORAGE_PHONE) {
		file_location = g_strconcat(VP_UTIL_LABEL_DEVICE_MEMORY, logic_path, NULL);
	} else if (store_type == VP_STORAGE_OTG) {
		file_location = g_strconcat(VP_UTIL_LABEL_OTG, logic_path, NULL);
	} else if (store_type == VP_STORAGE_PERSONAL_PAGE) {
		file_location = g_strconcat(VP_UTIL_LABEL_PERSONAL_PAGE, logic_path, NULL);
	} else {
		file_location = g_strdup(VP_UTIL_LABEL_UNKNOWN);
	}

	g_free(logic_path);
	logic_path = NULL;

	return file_location;
}

int vp_util_image_rotate(unsigned char *dest, int *dest_width, int *dest_height, const image_util_rotation_e dest_rotation,
                         const unsigned char *src, const int src_w, const int src_h, const image_util_colorspace_e colorspace)
{
	if (!dest || !dest_width || !dest_height || !src) {
		return IMAGE_UTIL_ERROR_INVALID_PARAMETER;
	}

	if (IMAGE_UTIL_COLORSPACE_RGB888 != colorspace || src_w <= 0 || src_h <= 0
	        || dest_rotation <= IMAGE_UTIL_ROTATION_NONE || dest_rotation > IMAGE_UTIL_ROTATION_FLIP_VERT) {
		return IMAGE_UTIL_ERROR_INVALID_PARAMETER;
	}

	const int dest_w = (IMAGE_UTIL_ROTATION_180 == dest_rotation ? src_w : src_h);
	const int dest_h = (IMAGE_UTIL_ROTATION_180 == dest_rotation ? src_h : src_w);
	const unsigned int src_stride = src_w * sizeof(rgb888);
	const unsigned int dest_stride = dest_w * sizeof(rgb888);
	int x = 0, y = 0;

	switch (dest_rotation) {
	case IMAGE_UTIL_ROTATION_90: {
		const rgb888 * const src_col_0 = (rgb888*)(src + (src_h - 1) * src_stride);
		for (y = 0; y < dest_h; y++) {
			rgb888 * const dest_row = (rgb888*)(dest + y * dest_stride);
			const rgb888 * const src_col = (src_col_0 + y);
			for (x = 0; x < dest_w; x++) {
				dest_row[x] = *(rgb888*)((uchar*)src_col - x * src_stride);
			}
		}
	}
	break;

	case IMAGE_UTIL_ROTATION_180: {
		const rgb888 * const src_row_rev_0 = (rgb888*)(src + (src_h - 1) * src_stride
		                                     + (src_w - 1) * sizeof(rgb888));
		for (y = 0; y < dest_h; y++) {
			rgb888 * const dest_row = (rgb888*)(dest + y * dest_stride);
			const rgb888 * const src_row_rev = (rgb888*)((uchar*)src_row_rev_0 - y * src_stride);
			for (x = 0; x < dest_w; x++) {
				dest_row[x] = *(src_row_rev - x);
			}
		}
	}
	break;

	case IMAGE_UTIL_ROTATION_270: {
		const rgb888 * const src_col_rev_0 = (rgb888*)(src + (src_w - 1) * sizeof(rgb888));
		for (y = 0; y < dest_h; y++) {
			rgb888 * const dest_row = (rgb888*)(dest + y * dest_stride);
			const rgb888 * const src_col_rev = (src_col_rev_0 - y);
			for (x = 0; x < dest_w; x++) {
				dest_row[x] = *(rgb888*)((uchar*)src_col_rev + x * src_stride);
			}
		}
	}
	break;

	default:
		return IMAGE_UTIL_ERROR_INVALID_PARAMETER;
	}

	*dest_width = dest_w;
	*dest_height = dest_h;
	return IMAGE_UTIL_ERROR_NONE;
}

void vp_util_lock_cpu()
{
	int ret = device_power_request_lock(POWER_LOCK_CPU, 0);
	if (ret) {
		return;
	}
}

void vp_util_release_cpu()
{
	int ret = device_power_release_lock(POWER_LOCK_CPU);
	if (ret) {
		return;
	}
}

int vp_util_image_resize(unsigned char *dest, const int *dest_width , const int *dest_height, const unsigned char *src,
                         const int src_w, const int src_h, const image_util_colorspace_e colorspace)
{
	if (!dest || !dest_width || !dest_height || !src) {
		return IMAGE_UTIL_ERROR_INVALID_PARAMETER;
	}

	int dest_w = *dest_width;
	int dest_h = *dest_height;

	if ((IMAGE_UTIL_COLORSPACE_RGB888 != colorspace && IMAGE_UTIL_COLORSPACE_RGBA8888 != colorspace)
	        || src_w <= 0 || src_h <= 0 || dest_w <= 0 || dest_h <= 0) {
		return IMAGE_UTIL_ERROR_INVALID_PARAMETER;
	}

	const unsigned int bpp = (IMAGE_UTIL_COLORSPACE_RGBA8888 == colorspace ? RGBA_BPP : RGB_BPP);
	const unsigned int src_stride = bpp * src_w;
	const unsigned int dest_stride = bpp * dest_w;
	float coef = 0.0f;
	float c1, c2, c3, c4;
	c1 = c2 = c3 = c4 = 0.0f;
	u_int32_t red, green, blue, alpha;
	red = green = blue = alpha = 0;
	int x = 0, y = 0;
	const float coef_x = (float)(src_w) / (float)(dest_w);
	const float coef_y = (float)(src_h) / (float)(dest_h);
	const float add_x = 0.5f * coef_x - 0.5f;
	const float add_y = 0.5f * coef_y - 0.5f;
	transform *transform_x = NULL, *transform_y = NULL;
	transform_x = calloc(dest_w, sizeof(transform));
	if (!transform_x) {
		return IMAGE_UTIL_ERROR_OUT_OF_MEMORY;
	}
	transform_y = calloc(dest_h, sizeof(transform));
	if (!transform_y) {
		free(transform_x);
		return IMAGE_UTIL_ERROR_OUT_OF_MEMORY;
	}

	for (x = 0; x < dest_w; ++x) {
		coef = x * coef_x + add_x;
		transform_x[x].index = (int)coef;
		transform_x[x].coef = 1 - coef + transform_x[x].index;
	}
	if (transform_x[0].index < 0) {
		transform_x[0].index = 0;
		transform_x[0].coef = 1.0f;
	}
	if (transform_x[dest_w - 1].index >= src_w - 2) {
		transform_x[dest_w - 1].index = src_w - 2;
		transform_x[dest_w - 1].coef = 0.0f;
	}

	for (y = 0; y < dest_h; ++y) {
		coef = y * coef_y + add_y;
		transform_y[y].index = (int)coef;
		transform_y[y].coef = 1 - coef + transform_y[y].index;
	}
	if (transform_y[0].index < 0) {
		transform_y[0].index = 0;
		transform_y[0].coef = 1.0f;
	}
	if (transform_y[dest_h - 1].index >= src_h - 2) {
		transform_y[dest_h - 1].index = src_h - 2;
		transform_y[dest_h - 1].coef = 0.0f;
	}

	if (colorspace == IMAGE_UTIL_COLORSPACE_RGBA8888) {
		for (y = 0; y < dest_h; ++y) {
			const transform t_y = transform_y[y];
			rgba8888 * const dest_row = (rgba8888*)(dest + y * dest_stride);
			const rgba8888 * const src_row_1 = (rgba8888*)(src + t_y.index * src_stride);
			const rgba8888 * const src_row_2 = (rgba8888*)(src + (t_y.index + 1) * src_stride);

			for (x = 0; x < dest_w; ++x) {
				const transform t_x = transform_x[x];
				const rgba8888 pixel1 = src_row_1[t_x.index];
				const rgba8888 pixel2 = src_row_1[t_x.index + 1];
				const rgba8888 pixel3 = src_row_2[t_x.index];
				const rgba8888 pixel4 = src_row_2[t_x.index + 1];
				c1 = t_x.coef * t_y.coef;
				c2 = (1 - t_x.coef) * t_y.coef;
				c3 = t_x.coef * (1 - t_y.coef);
				c4 = (1 - t_x.coef) * (1 - t_y.coef);
				red = pixel1.r * c1 + pixel2.r * c2 + pixel3.r * c3
				      + pixel4.r * c4;
				green = pixel1.g * c1 + pixel2.g * c2 + pixel3.g * c3
				        + pixel4.g * c4;
				blue = pixel1.b * c1 + pixel2.b * c2 + pixel3.b * c3
				       + pixel4.b * c4;
				alpha = pixel1.a * c1 + pixel2.a * c2 + pixel3.a * c3
				        + pixel4.a * c4;
				dest_row[x].r = red;
				dest_row[x].g = green;
				dest_row[x].b = blue;
				dest_row[x].a = alpha;
			}
		}
	} else {
		for (y = 0; y < dest_h; ++y) {
			const transform t_y = transform_y[y];
			rgb888 * const dest_row = (rgb888*)(dest + y * dest_stride);
			const rgb888 * const src_row_1 = (rgb888*)(src + t_y.index * src_stride);
			const rgb888 * const src_row_2 = (rgb888*)(src + (t_y.index + 1) * src_stride);

			for (x = 0; x < dest_w; ++x) {
				const transform t_x = transform_x[x];
				const rgb888 pixel1 = src_row_1[t_x.index];
				const rgb888 pixel2 = src_row_1[t_x.index + 1];
				const rgb888 pixel3 = src_row_2[t_x.index];
				const rgb888 pixel4 = src_row_2[t_x.index + 1];
				c1 = t_x.coef * t_y.coef;
				c2 = (1 - t_x.coef) * t_y.coef;
				c3 = t_x.coef * (1 - t_y.coef);
				c4 = (1 - t_x.coef) * (1 - t_y.coef);
				red = pixel1.r * c1 + pixel2.r * c2 + pixel3.r * c3
				      + pixel4.r * c4;
				green = pixel1.g * c1 + pixel2.g * c2 + pixel3.g * c3
				        + pixel4.g * c4;
				blue = pixel1.b * c1 + pixel2.b * c2 + pixel3.b * c3
				       + pixel4.b * c4;

				dest_row[x].r = red;
				dest_row[x].g = green;
				dest_row[x].b = blue;
			}
		}
	}

	free(transform_x);
	free(transform_y);

	return IMAGE_UTIL_ERROR_NONE;
}
