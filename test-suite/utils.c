/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández Navarro
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
#include <libcronosII/utils.h>

gint
main (gint argc, gchar **argv)
{
	gchar *string;
	FILE *fd;

	fd = fopen ("test-string.txt", "r");
	
L	for (;(string = c2_fd_get_line (fd)) != NULL;)
	{
L		printf ("%s\n", string);
	}
	fseek (fd, 0, SEEK_SET);
L	fclose (fd);

	return 0;
}
