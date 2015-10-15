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

#ifndef _VIDEO_INFO_CTRL_
#define  _VIDEO_INFO_CTRL_

bool	mp_info_ctrl_get_gps(char *szUriPath, double *dLongitude, double *dLatitude);
char*	mp_info_ctrl_get_file_extension(char *szPath);
bool	mp_info_ctrl_get_resolution(char *szWidthResolution, char *szHeightResolution, void *pUserData);
bool	mp_info_ctrl_get_file_info(char *szUriPath, char *szFileDate, int nFileDateSize, char *szFileExtension, int nFileExtensionSize, char *szFileSize, int nFilesizeSize);
char*	mp_info_ctrl_get_data_of_file(time_t mtime);

#endif
