/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/**
 * Maintainer(s) of this file:
 * 		* Pablo Fernández
 * Code of this file by:
 * 		* Pablo Fernández
 */
#ifndef __LIBCRONOSII_UTILS_FILE_H__
#define __LIBCRONOSII_UTILS_FILE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <stdio.h>
#include <pthread.h>
//#include <gtk/gtk.h>
#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include <config.h>
#else
#	include <cronosII.h>
#endif

gchar *
c2_get_tmp_file								(const gchar *template);

gint
c2_get_file									(const gchar *path, gchar **string);

char *
c2_fd_get_line								(FILE *fd);

gchar *
c2_fd_get_word								(FILE *fd);

gint
c2_file_binary_copy							(const gchar *from_path, const gchar *target_path);

gint
c2_file_binary_move							(const gchar *from_path, const gchar *target_path);

gboolean
c2_file_exists								(const gchar *file);

gboolean
c2_file_is_directory						(const gchar *file);

gboolean
c2_fd_move_to								(FILE *fp, gchar c, guint8 cant,
										 	 gboolean forward, gboolean next);
										 	 
#ifdef __cplusplus
}
#endif

#endif
