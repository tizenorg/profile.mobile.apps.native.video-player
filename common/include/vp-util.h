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

#pragma once

#include <stdbool.h>
#include <Elementary.h>
#include <image_util_type.h>

char* vp_util_convert_file_location(const char *szFileLocation);
int vp_util_image_rotate(unsigned char *dest, int *dest_width, int *dest_height, const image_util_rotation_e dest_rotation,
		const unsigned char *src, const int src_w, const int src_h, const image_util_colorspace_e colorspace);
int vp_util_image_resize(unsigned char *dest, const int *dest_width , const int *dest_height, const unsigned char *src,
		const int src_w, const int src_h, const image_util_colorspace_e colorspace);
void vp_util_lock_cpu();
void vp_util_release_cpu();

