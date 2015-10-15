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

#include <stdio.h>
#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <ftw.h>
#include <Ecore_Evas.h>
#include <sys/stat.h>
#include <sys/statfs.h>


#include "mp-util.h"
#include "mp-video-log.h"
#include "mp-util-media-service.h"
#include "mp-video-value-define.h"
#include "mp-video-string-define.h"
#include "mp-util-move.h"
#include "vp-file-util.h"

#define MSG_REPORT_PERIOD (1)
#define DEF_ALLLOC_SIZE 16384	/*((4)*(1024))*/
#define DIR_MODE_BIT (01777)
#define FILE_MODE_BIT (S_IRWXU | S_IRWXG | S_IRWXO)
#define PER_HANDLE_MAX_SIZE (10*1024*1204)
#define PER_HANDLE_MIN_SIZE (1024*1024)
#define MP_VISUAL_FOLDER_SIZE 16

typedef struct __MpUtilDirListInfo MpUtilDirListInfo;

struct __MpUtilDirListInfo {
	char *ftw_path;
	unsigned long long size;
	int type;
};

GSList *g_copy_list = NULL;
bool g_do_cancel = FALSE;
unsigned long long g_total_size = 0;

static char *__mp_util_move_change_root_name(const char *name, const char *old_root, const char *new_root)
{
	char *new_name = NULL;

	if (name && old_root && new_root) {
		int old_len = strlen(old_root);
		int new_len = strlen(new_root);
		int name_len = strlen(name);
		const char *base = NULL;
		char *name_tmp = NULL;

		if ((strstr(name, old_root) == NULL)
		    || (name_len <= old_len)
		    || ((name[old_len] == '/' && name[old_len + 1] == '\0'))
		    || FALSE) {
			return NULL;
		}

		base = name + old_len;
		if (name[old_len] == '/') {
			base += 1;
		}

		name_tmp = g_strdup(new_root);
		if (name_tmp) {
			if (name_tmp[new_len - 1] == '/') {
				new_name = g_strdup_printf("%s%s", name_tmp, base);
			} else {
				new_name = g_strdup_printf("%s/%s", name_tmp, base);
			}
		}
		MP_FREE_STRING(name_tmp);
	}
	return new_name;
}

bool mp_util_move_cancel_check(void)
{
	return g_do_cancel;
}

void mp_util_move_set_cancel_state(bool bCancel)
{
	g_do_cancel = bCancel;
}
static int __mp_util_move_get_remain_space(const char *path, unsigned long long *size)
{
	struct statfs dst_fs;

	if (!path || !size) {
		return -EINVAL;
	}

	if (statfs(path, &dst_fs) == 0) {
		*size = ((unsigned long long)(dst_fs.f_bsize) * (unsigned long long)(dst_fs.f_bavail));
	} else {
		return -errno;
	}

	return 0;
}

inline bool __mp_util_move_check_exist(const char *path)
{
	if (path && (access(path, F_OK) == 0)) {
		return true;
	}
	return false;
}

static int __mp_util_move_directory_hierarchies(const char *pathname, const struct stat *statptr, int type)
{
	MpUtilDirListInfo *info = NULL;
	MpUtilDirListInfo *info_title = NULL;
	VideoSecureLogInfo("pathname is [%s]\t type is [%d]\t", pathname, type);
	switch (type) {
	case FTW_F:
		if (mp_util_check_video_file(pathname)) {
			struct stat src_info;
			if (stat(pathname, &src_info)) {
				VideoLogInfo("error");
			} else {
				info = calloc(sizeof(MpUtilDirListInfo), 1);
				if (!info) {
					VideoSecureLogInfo("failed to allocate memory");
					return -1;
				}

				info->ftw_path = g_strdup(pathname);
				info->type = type;
				info->size = (unsigned long long)src_info.st_size;
				g_copy_list = g_slist_append(g_copy_list, info);
				//append subtitle
				char *szSubTitle = NULL;
				mp_util_get_subtitle_path(pathname, &szSubTitle);
				if (szSubTitle) {
					if (stat(szSubTitle, &src_info)) {
						VideoLogInfo("error");
					} else {
						info_title = calloc(sizeof(MpUtilDirListInfo), 1);
						if (info_title) {
							info_title->ftw_path = g_strdup(szSubTitle);
							info_title->type = type;
							info_title->size = (unsigned long long)src_info.st_size;
							g_copy_list = g_slist_append(g_copy_list, info_title);
						}
					}
					MP_FREE_STRING(szSubTitle);
				}
			}
		}
		break;
	case FTW_D:
		info = calloc(sizeof(MpUtilDirListInfo), 1);
		if (!info) {
			VideoSecureLogInfo("failed to allocate memory");
			return -1;
		}
		info->ftw_path = g_strdup(pathname);
		info->type = type;
		info->size = MP_VISUAL_FOLDER_SIZE;
		g_copy_list = g_slist_append(g_copy_list, info);

		break;
	default:
		VideoSecureLogInfo("Default pathname is [%s]", pathname);
	}

	return 0;
}

static void __mp_util_move_free_directory_hierarchies(GSList **glist)
{
	if (*glist == NULL)
		return;
	GSList *list = *glist;
	while (list) {
		MpUtilDirListInfo *info = NULL;
		info = (MpUtilDirListInfo *)list->data;
		g_free(info->ftw_path);
		g_free(info);
		list = g_slist_next(list);
	}
	g_slist_free(*glist);
	*glist = NULL;
}

static int __mp_util_move_copy_reg_file(const char *szSrcPath, struct stat *src_statp, const char *szDstPath, unsigned long buf_size, mp_util_copy_cb msg_cb,
			  void *msg_data)
{
	FILE *src_f = NULL;
	FILE *dst_f = NULL;
	void *buf = NULL;
	unsigned long alloc_size = DEF_ALLLOC_SIZE;
	ssize_t r_size = 0;
	mode_t src_mode = 0;

	if (!szSrcPath) {
		VideoLogError("error srcPath");
		return -1;
	}
	if (!szDstPath) {
		VideoLogError("error dstPath");
		return -1;
	}

	if (src_statp) {
		src_mode = src_statp->st_mode;
	} else {
		struct stat src_info;
		if (stat(szSrcPath, &src_info)) {
			VideoLogError("Fail to stat src file : %d", errno);
			return -1;
		}
		src_mode = src_info.st_mode;
	}


	if (!S_ISREG(src_mode)) {
		VideoLogError("src[%s] is not regular file", szSrcPath);
		return -1;
	}

	src_f = fopen(szSrcPath, "rb");
	if (!src_f) {
		VideoLogError("Fail to open src file : %d", errno);
		return -1;
	}

	dst_f = fopen(szDstPath, "wb");
	if (!dst_f) {
		VideoLogError("Fail to open dst file : %d", errno);

		goto ERROR_CLOSE_FD;
	}
	if (buf_size == 0) {
		struct stat dst_info;
		if (stat(szDstPath, &dst_info)) {
			VideoLogError("Fail to stat dst file", szDstPath);
			goto ERROR_CLOSE_FD;
		}
	}
	/*	if (dst_info.st_blksize > 0) {
			alloc_size = dst_info.st_blksize;
		}
	} else {
		alloc_size = buf_size;
	}*/
	alloc_size = DEF_ALLLOC_SIZE;

	buf = malloc(alloc_size);
	if (!buf) {
		VideoLogError("fail to alloc buf, alloc_size : %lu", alloc_size);
		goto ERROR_CLOSE_FD;
	}

	int count = 0;
	ssize_t msg_size = 0;

	int dynamic_size = 0;
	if (g_total_size > PER_HANDLE_MAX_SIZE) {
		dynamic_size = 64;
	} else if (g_total_size <= PER_HANDLE_MAX_SIZE && g_total_size > PER_HANDLE_MIN_SIZE) {
		dynamic_size = 8;
	} else {
		dynamic_size = 1;
	}

	int dst_fd = fileno(dst_f);
	ssize_t total = 0;
	while ((r_size = fread(buf, 1, alloc_size, src_f)) > 0)
	{
		total = r_size;
		void *buf_p = buf;

		count++;
		while (total > 0) {
			ssize_t w_size = 0;
			w_size = fwrite(buf_p, 1, total, dst_f);
			if (ferror(dst_f) != 0 || (r_size != w_size)) {
				VideoLogError("fail to write:%d,%d", dst_f, errno);
				goto ERROR_CLOSE_FD;
			}

			buf_p += w_size;
			total -= w_size;
		}
		if (mp_util_move_cancel_check()) {
			goto CANCEL_CLOSE_FD;
		}
		if ((count == dynamic_size) && msg_cb) {
			msg_size += r_size;
			if (msg_cb) {
				msg_cb(MP_VIDEO_UPDATE, msg_size, 0, msg_data);
			}
			msg_size = 0;
		} else {
			msg_size += r_size;
		}
		int ret = posix_fadvise(dst_fd, 0, msg_size, POSIX_FADV_DONTNEED);
		if (ret != 0) {
			VideoLogInfo(" >>>>posix_fadvise ret=%d", ret);
		}
		count = count % dynamic_size;

	}

	if (msg_size > 0 && msg_cb) {
		msg_size += r_size;
		if (msg_cb) {
			msg_cb(MP_VIDEO_UPDATE, msg_size, 0, msg_data);
		}

		msg_size = 0;
	}

	free(buf);
	buf = NULL;

	fclose(src_f);
	fclose(dst_f);

	if (!vp_file_unlink(szSrcPath)) {
		VideoLogInfo("remove failed.");
	}

	return 0;

ERROR_CLOSE_FD:

	if (src_f) {
		fclose(src_f);
		src_f = NULL;
	}
	if (dst_f) {
		fclose(dst_f);
		dst_f = NULL;
		if (!vp_file_unlink(szDstPath)) {
			VideoLogInfo("remove failed.");
		}
	}
	if (buf) {
		free(buf);
		buf = NULL;
	}
	return -1;


CANCEL_CLOSE_FD:
	if (buf) {
		free(buf);
		buf = NULL;
	}
	if (src_f) {
		fclose(src_f);
		src_f = NULL;
	}
	if (dst_f) {
		fclose(dst_f);
		dst_f = NULL;
		if (!vp_file_unlink(szDstPath)) {
			VideoLogInfo("remove failed.");
		}
	}

	return 1;
}

static int __mp_util_move_copy_directory(const char *szSrcPath, struct stat *src_statp, const char *szDstPath, mp_util_copy_cb msg_cb, void *msg_data)
{
	int ret = -1;
	mode_t src_mode = 0;
	int err = 0;
	if (!szSrcPath) {
		VideoLogError("error srcPath");
		return -1;
	}
	if (!szDstPath) {
		VideoLogError("error dstPath");
		return -1;
	}

	if (src_statp) {
		src_mode = src_statp->st_mode;
	} else {
		struct stat src_info;
		if (stat(szSrcPath, &src_info)) {
			VideoLogError("Fail to stat src file : %d", errno);
			return -1;
		}
		src_mode = src_info.st_mode;
	}

	if (access(szDstPath, F_OK)) {
		if (mkdir(szDstPath, (src_mode & DIR_MODE_BIT))) {
			VideoLogError("Fail to make directory=%d", errno);
			return -1;
		} else {
			if (msg_cb) {
				msg_cb(MP_VIDEO_UPDATE, MP_VISUAL_FOLDER_SIZE, 0, msg_data);
			}
		}
	} else {
		VideoSecureLogInfo("directory[%s] is already existed", szDstPath);
	}

	char *new_dir = NULL;
	__mp_util_move_free_directory_hierarchies(&g_copy_list);
	g_copy_list = NULL;
	ret = ftw(szSrcPath, __mp_util_move_directory_hierarchies, 16);
	if (ret == 0) {
		MpUtilDirListInfo *ent = NULL;
		GSList *list = NULL;
		list = g_copy_list;
		while (list) {
			if (mp_util_move_cancel_check()) {
				goto DO_CANCEL;
			}
			ent = (MpUtilDirListInfo *)list->data;
			VideoSecureLogInfo("name is [%s] type is [%d]", ent->ftw_path, ent->type);
			if (ent->type == FTW_D) {
				if (ent->ftw_path == NULL || strlen(ent->ftw_path) == 0) {
					list = g_slist_next(list);
					continue;
				}
				if (g_strcmp0(ent->ftw_path, szSrcPath) == 0) {
					list = g_slist_next(list);
					continue;
				}
				MP_FREE_STRING(new_dir);
				new_dir = __mp_util_move_change_root_name(ent->ftw_path, szSrcPath, szDstPath);
				VideoSecureLogInfo("copy dir %s to %s", ent->ftw_path, new_dir);
				if (new_dir) {
					if (!__mp_util_move_check_exist(new_dir)) {
						struct stat info;
						if (stat(ent->ftw_path, &info) == 0) {
							if (mkdir(new_dir, (info.st_mode & DIR_MODE_BIT))) {
								/* fts_set(fts, ent, FTS_SKIP); */
								VideoSecureLogError("Fail to make directory");
								MP_FREE_STRING(new_dir);
								goto ERROR_CLOSE_FD;
							} else {
								if (msg_cb) {
									msg_cb(MP_VIDEO_UPDATE, 0, 0, msg_data);
								}
							}
						} else {
							VideoSecureLogError("Fail to stat ");
							/* fts_set(fts, ent, FTS_SKIP); */
							MP_FREE_STRING(new_dir);
							goto ERROR_CLOSE_FD;
						}
					} else {
						struct stat new_dst_info;
						if (stat(new_dir, &new_dst_info) == 0) {
							if (S_ISDIR(new_dst_info.st_mode)) {
								if (msg_cb) {
									msg_cb(MP_VIDEO_UPDATE, 0, 0, msg_data);
								}
							} else {
								VideoSecureLogInfo("[%s] is already existed, and this one is not directory", new_dir);
								/*set FTS_SKIP to skip children of current*/
								/*fts_set(fts, ent, FTS_SKIP);*/
								MP_FREE_STRING(new_dir);
								goto ERROR_CLOSE_FD;
							}
						} else {
							VideoLogError("Fail to stat %d", errno);
							/*fts_set(fts, ent, FTS_SKIP);*/
							MP_FREE_STRING(new_dir);
							goto ERROR_CLOSE_FD;
						}
					}
					MP_FREE_STRING(new_dir);
				} else {
					goto ERROR_CLOSE_FD;
				}
			} else if (ent->type == FTW_F) {
				if (ent->ftw_path == NULL || strlen(ent->ftw_path) == 0) {
					list = g_slist_next(list);
					continue;
				}
				char *new_file = __mp_util_move_change_root_name(ent->ftw_path, szSrcPath, szDstPath);
				if (new_file) {
					err = __mp_util_move_copy_reg_file(ent->ftw_path, NULL, new_file, 0, msg_cb, msg_data);
					if (err == 0) {
						mp_util_svc_scan_file(new_file);
						mp_util_svc_scan_file(ent->ftw_path);
					}
					MP_FREE_STRING(new_file);
					if (err > 0) {
						goto DO_CANCEL;
					} else if (err < 0) {
						goto ERROR_CLOSE_FD;
					}
				} else {
					goto ERROR_CLOSE_FD;
				}
			}
			list = g_slist_next(list);
		}

	} else {
		goto ERROR_CLOSE_FD;
	}
	MP_FREE_STRING(new_dir);
	__mp_util_move_free_directory_hierarchies(&g_copy_list);
	g_copy_list = NULL;
	if (mp_util_check_empty_dir(szSrcPath))
	{
		rmdir(szSrcPath);
	}
	return 0;

ERROR_CLOSE_FD:
	MP_FREE_STRING(new_dir);
	__mp_util_move_free_directory_hierarchies(&g_copy_list);
	g_copy_list = NULL;
	return -1;

DO_CANCEL:
	MP_FREE_STRING(new_dir);
	__mp_util_move_free_directory_hierarchies(&g_copy_list);
	g_copy_list = NULL;
	return 1;
}


int __mp_util_move_copy_internal(const char *szSrcPath, const char *szDstPath, mp_util_copy_cb msg_func, void *msg_data)
{
	int err = 0;
	char *src_basename = NULL;
	char *new_dst = NULL;
	int base_size = 0;
	int root_size = 0;
	int with_slash = 1;
	int alloc_size = 1;	/*for null*/
	struct stat src_info;
	char *szNewestPath = NULL;

	if (!szSrcPath || strlen(szSrcPath) <= 1) {
		VideoLogError("src is NULL");
		return -1;
	}

	if (!szDstPath) {
		VideoLogError("dst is NULL");
		return -1;
	}

	if (access(szDstPath, R_OK | W_OK)) {
		VideoLogError("dst_dir no permission");
		return -1;
	}

	if (stat(szSrcPath, &src_info)) {
		VideoLogError("Fail to stat src ", szSrcPath);
		return -1;
	}

	if (S_ISDIR(src_info.st_mode)) {
		if (g_strcmp0(szDstPath, szSrcPath) == 0) {
			VideoLogError("dst is child of src");
			return -1;
		}
	}
	src_basename = g_path_get_basename(szSrcPath);
	if (!src_basename) {
		VideoLogError("fail to get basename from src");
		return -1;
	}

	base_size = strlen(src_basename);
	root_size = strlen(szDstPath);

	if (szDstPath[root_size - 1] != '/') {
		alloc_size += 1;
		with_slash = 0;
	}

	alloc_size += (base_size + root_size);

	new_dst = malloc(sizeof(char) * (alloc_size));
	if (!new_dst) {
		VideoLogError("fail to alloc new dst");
		goto ERROR_FREE_MEM;
	}

	if (with_slash) {
		snprintf(new_dst, alloc_size, "%s%s", szDstPath, src_basename);
	} else {
		snprintf(new_dst, alloc_size, "%s/%s", szDstPath, src_basename);
	}
	MP_FREE_STRING(src_basename);

	if (mp_util_move_cancel_check()) {
		goto CANCEL_FREE_MEM;
	}

	mp_util_rename_the_same_file(new_dst, &szNewestPath);
	MP_FREE_STRING(new_dst);

	if (access(szNewestPath, F_OK)) {
		if (S_ISDIR(src_info.st_mode)) {
			err = __mp_util_move_copy_directory(szSrcPath, &src_info, szNewestPath, msg_func, msg_data);
			if (err == 0) {
				mp_util_svc_scan_file(szSrcPath);
				mp_util_svc_scan_file(szNewestPath);
			}
		} else if (S_ISREG(src_info.st_mode)) {
			/*just copy*/
			err = __mp_util_move_copy_reg_file(szSrcPath, &src_info, szNewestPath, 0, msg_func, msg_data);
			if (err == 0) {
				mp_util_svc_scan_file(szSrcPath);
				mp_util_svc_scan_file(szNewestPath);
			}
		} else {
			VideoSecureLogInfo("item[%s] is not file or directory", szSrcPath);
			goto ERROR_FREE_MEM;
		}
	} else {
		VideoLogInfo("fail to alloc new dst");
		goto ERROR_FREE_MEM;
	}
	MP_FREE_STRING(szNewestPath);

	if (err > 0) {
		goto CANCEL_FREE_MEM;
	} else if (err < 0) {
		goto ERROR_FREE_MEM;
	}

	return 0;

ERROR_FREE_MEM:
	VideoLogError("Copy error");
	MP_FREE_STRING(src_basename);
	MP_FREE_STRING(szNewestPath);
	MP_FREE_STRING(new_dst);

	return -1;

CANCEL_FREE_MEM:

	VideoLogError("Copy cancelled");
	MP_FREE_STRING(src_basename);
	MP_FREE_STRING(szNewestPath);
	MP_FREE_STRING(new_dst);

	return 1;
}

bool mp_util_move_get_total_dir_size(const char *szDir, unsigned long long *size)
{
	int ret = -1;
	if (!szDir) {
		return FALSE;
	}
	__mp_util_move_free_directory_hierarchies(&g_copy_list);
	g_copy_list = NULL;
	unsigned long long nSizeTmp = 0;
	ret = ftw(szDir, __mp_util_move_directory_hierarchies, 16);
	if (ret == 0) {
		MpUtilDirListInfo *ent = NULL;
		GSList *list = NULL;
		list = g_copy_list;
		while (list) {
			ent = (MpUtilDirListInfo *)list->data;
			if (ent->type == FTW_D) {
				nSizeTmp += MP_VISUAL_FOLDER_SIZE;
			} else if (ent->type == FTW_F) {
				VideoLogInfo("==%lld", ent->size);
				nSizeTmp += ent->size;
			}
			list = g_slist_next(list);
		}
	} else {
		*size = nSizeTmp;
		__mp_util_move_free_directory_hierarchies(&g_copy_list);
		g_copy_list = NULL;
		return FALSE;;
	}
	__mp_util_move_free_directory_hierarchies(&g_copy_list);
	g_copy_list = NULL;
	*size = nSizeTmp;
	return TRUE;
}

int mp_util_move_push(const char *szSrcPath, const char *szDstPath, mp_util_copy_cb cbFun, void *userData, unsigned long long nTotalSize)
{
	VideoLogInfo("start");

	//double s_start = 0.0;
	//double s_stop = 0.0;
	int nRet = 0;
	unsigned long long r_size = 0;
	int errcode = 0;
	//init static data
	g_do_cancel = FALSE;
	if (g_copy_list)
	{
		__mp_util_move_free_directory_hierarchies(&g_copy_list);
		g_copy_list = NULL;
	}
	//s_start = _MpUtilCopyGetTime();

	errcode = __mp_util_move_get_remain_space(szDstPath, &r_size);
	if (errcode < 0) {
		nRet = -1;
		VideoLogError("error code = %d", errcode);

		goto ERROR_END_THREAD;
	}

	if (r_size == 0) {
		VideoLogError("error space = %d", errcode);
		nRet = -1;
		goto ERROR_END_THREAD;
	}

	g_total_size = nTotalSize;

	//s_stop = _MpUtilCopyGetTime();

	/* copy items */
	nRet = __mp_util_move_copy_internal(szSrcPath, szDstPath, cbFun, userData);

	return nRet;
ERROR_END_THREAD:

	VideoLogInfo("The end of MpUtilCopyFunc");
	return nRet;
}

static int __mp_util_move_delete_internal(const char *szSrcPath, void *msg_data)
{
	if (!szSrcPath)
	{
		VideoLogError("szSrcPath is NULL");
		return -1;
	}
	int ret = 0;
	if (g_copy_list)
	{
		__mp_util_move_free_directory_hierarchies(&g_copy_list);
		g_copy_list = NULL;
	}
	ret = ftw(szSrcPath, __mp_util_move_directory_hierarchies, 16);
	if (ret == 0) {
		MpUtilDirListInfo *ent = NULL;
		GSList *list = NULL;
		list = g_copy_list;
		while (list) {
			if (mp_util_move_cancel_check()) {
				goto DO_CANCEL;
			}
			ent = (MpUtilDirListInfo *)list->data;
			VideoSecureLogInfo("name is [%s] type is [%d]", ent->ftw_path, ent->type);
			if (ent->type == FTW_D) {
				list = g_slist_next(list);
				continue;
			} else if (ent->type == FTW_F) {
				if (ent->ftw_path == NULL || strlen(ent->ftw_path) == 0) {
					list = g_slist_next(list);
					continue;
				}
				if (!vp_file_unlink(ent->ftw_path)) {
					VideoLogInfo("remove failed.");
					goto ERROR_CLOSE_FD;
				}
				mp_util_svc_scan_file(ent->ftw_path);
			}
			list = g_slist_next(list);
		}

	} else {
		goto ERROR_CLOSE_FD;
	}

	return 0;

ERROR_CLOSE_FD:
	__mp_util_move_free_directory_hierarchies(&g_copy_list);
	g_copy_list = NULL;
	return -1;

DO_CANCEL:
	__mp_util_move_free_directory_hierarchies(&g_copy_list);
	g_copy_list = NULL;
	return 1;
}

int mp_util_move_delete_video_folder(const char *szFolderPath, void *userData)
{
	VideoLogInfo("start");
	if (!szFolderPath)
	{
		VideoLogError("szSrcPath is NULL");
		return -1;
	}

	int nRet = 0;
	g_do_cancel = FALSE;
	if (g_copy_list)
	{
		__mp_util_move_free_directory_hierarchies(&g_copy_list);
		g_copy_list = NULL;
	}

	/* copy items */
	nRet = __mp_util_move_delete_internal(szFolderPath, userData);
	__mp_util_move_free_directory_hierarchies(&g_copy_list);
	g_copy_list = NULL;

	return nRet;
}

