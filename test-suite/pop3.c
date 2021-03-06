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
#include <stdio.h>
#include <glib.h>

#include <libcronosII/mailbox.h>
#include <libcronosII/pop3.h>
#include <libcronosII/smtp.h>
#include <libcronosII/account.h>
#include <libcronosII/error.h>
#include <libcronosII/utils.h>

gint
main (gint argc, gchar **argv)
{
	C2Mailbox *inbox, *head = NULL;
	C2POP3 	*pop3 = NULL;
	gint	flags =	0;
	C2Account *act = NULL;

	gtk_init (&argc, &argv);

	flags = C2_POP3_DO_KEEP_COPY;

	inbox = c2_mailbox_new (&head, "Inbox2", "0", C2_MAILBOX_CRONOSII, 0, 0);
	pop3 = c2_pop3_new(g_strdup (argv[1]), 110, g_strdup (argv[2]), g_strdup (argv[3]), FALSE);
	act  = c2_account_new(C2_ACCOUNT_POP3, "Me", "root@localhost");

	c2_pop3_set_flags(pop3,flags);

//	if(c2_net_object_run (C2_NET_OBJECT (pop3)) < 0)
//	{
//		
//	}

	c2_pop3_fetchmail(pop3, act, inbox);


	//c2_net_object_disconnect_with_error (C2_NET_OBJECT (pop3));



	return 0;
}

