/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fern�ndez Navarro
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details._
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include <pthread.h>

#include "widget-transfer-item.h"

#define MAX_CHARS_IN_LABEL		25

static void
class_init									(C2TransferItemClass *klass);

static void
init										(C2TransferItem *ti);

static void
destroy										(GtkObject *object);

static void
on_cancel_clicked							(GtkWidget *button, C2TransferItem *ti);

static void
on_pop3_resolve								(GtkObject *object, C2TransferItem *ti);

static void
on_pop3_status								(GtkObject *object, gint mails, C2TransferItem *ti);

static void
on_pop3_retrieve							(GtkObject *object, gint16 nth, gint32 received,
											 gint32 total, C2TransferItem *ti);

static void
on_pop3_disconnect							(GtkObject *object, gboolean success, C2TransferItem *ti);

enum
{
	STATE_CHANGED,
	FINISH,
	CANCEL,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GtkObjectClass *parent_class = NULL;

GtkType
c2_transfer_item_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2TransferItem",
			sizeof (C2TransferItem),
			sizeof (C2TransferItemClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (gtk_object_get_type (), &info);
	}

	return type;
}

static void
class_init (C2TransferItemClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_object_get_type ());

	signals[STATE_CHANGED] =
		gtk_signal_new ("state_changed",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2TransferItemClass, state_changed),
						gtk_marshal_NONE__INT, GTK_TYPE_NONE, 1,
						GTK_TYPE_INT);
	signals[FINISH] =
		gtk_signal_new ("finish",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2TransferItemClass, finish),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	signals[CANCEL] =
		gtk_signal_new ("cancel",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2TransferItemClass, cancel),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	gtk_object_class_add_signals(object_class, signals, LAST_SIGNAL);

	klass->state_changed = NULL;
	klass->finish = NULL;
	klass->cancel = NULL;
}

static void
init (C2TransferItem *ti)
{
	ti->state = 0;
	ti->tooltip = NULL;
	ti->account = NULL;
}

C2TransferItem *
c2_transfer_item_new (C2Account *account, C2TransferItemType type, ...)
{
	C2TransferItem *ti;
	va_list args;

	ti = gtk_type_new (c2_transfer_item_get_type ());

	va_start (args, type);
	c2_transfer_item_construct (ti, account, type, args);
	va_end (args);

	return ti;
}

void
c2_transfer_item_construct (C2TransferItem *ti, C2Account *account, C2TransferItemType type, va_list args)
{
	GtkTooltips *tooltips;
	GtkWidget *label, *button, *pixmap, *box, *bpixmap;
	gchar *buffer, *subject;
	
	ti->account = account;
	ti->type = type;

	/* Store the extra-information */
	if (type == C2_TRANSFER_ITEM_RECEIVE)
	{
	} else if (type == C2_TRANSFER_ITEM_SEND)
	{
		ti->type_info.send.smtp = va_arg (args, C2SMTP *);
		ti->type_info.send.message = va_arg (args, C2Message *);
	} else
		g_assert_not_reached ();

	ti->table = gtk_table_new (1, 4, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (ti->table), 4);
	gtk_widget_show (ti->table);

	tooltips = gtk_tooltips_new ();

	if (type == C2_TRANSFER_ITEM_RECEIVE)
	{
		pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/receive.png");
		buffer = g_strdup_printf (_("Receive �%s�"), account->name);
	} else
	{
		pixmap = gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/send.png");

		if (ti->type_info.send.message)
			subject = c2_message_get_header_field (ti->type_info.send.message, "Subject:");
		else
			subject = NULL;
		if (!subject)
			subject = g_strdup (_("This is a mail with a really long subject"));
		buffer = g_strdup_printf (_("Send �%s�"), subject);
		g_free (subject);
	}
	
	gtk_widget_show (pixmap);
	
	if (strlen (buffer) > MAX_CHARS_IN_LABEL)
	{
		gchar *short_buffer = g_new0 (gchar, MAX_CHARS_IN_LABEL);
		strncpy (short_buffer, buffer, MAX_CHARS_IN_LABEL-3);
		strcat (short_buffer, "...");
		g_free (buffer);
		buffer = short_buffer;
	}
	
	label = gtk_label_new (buffer);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_widget_show (label);
	gtk_widget_set_usize (label, 138, -1);

	box = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (box);

	ti->progress_mail = gtk_progress_bar_new ();
	gtk_box_pack_start (GTK_BOX (box), ti->progress_mail, FALSE, TRUE, 0);
	gtk_widget_show (ti->progress_mail);

	ti->progress_byte = gtk_progress_bar_new ();
	gtk_box_pack_start (GTK_BOX (box), ti->progress_byte, FALSE, TRUE, 0);
	gtk_widget_set_usize (ti->progress_byte, -1, 5);

	button = gtk_button_new ();

	bpixmap = gnome_stock_pixmap_widget_new (GTK_WIDGET (ti), GNOME_STOCK_PIXMAP_STOP);
	gtk_container_add (GTK_CONTAINER (button), bpixmap);
	gtk_widget_show (bpixmap);

	gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
	gtk_tooltips_set_tip (tooltips, button, _("Cancel the transfer"), NULL);
	gtk_widget_show (button);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
						GTK_SIGNAL_FUNC (on_cancel_clicked), ti);
	ti->cancel_button = button;

	gtk_table_attach (GTK_TABLE (ti->table), pixmap, 0, 1, 0, 1, 0, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (ti->table), label, 1, 2, 0, 1, GTK_FILL, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (ti->table), box, 2, 3, 0, 1, 0, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (ti->table), button, 3, 4, 0, 1, 0, 0, 0, 0);
}

static void
on_cancel_clicked (GtkWidget *button, C2TransferItem *ti)
{
	C2Account *account = ti->account;
	
	gtk_signal_emit (GTK_OBJECT (ti), signals[CANCEL], account);

	/* [XXX] This won't be here, I'm testing */
	gtk_signal_emit (GTK_OBJECT (ti), signals[FINISH]);

#if 0
	if (account->type == C2_ACCOUNT_POP3)
		c2_pop3_cancel (account->protocol.pop3);
#endif
}

static void
fire_c2_account_check_in_its_own_thread (C2Account *account)
{
	c2_account_check (account);
}

void
c2_transfer_item_start (C2TransferItem *ti)
{
	pthread_t thread;

	if (ti->type == C2_TRANSFER_ITEM_RECEIVE)
	{
		if (ti->account->type == C2_ACCOUNT_POP3)
		{
			gtk_signal_connect (GTK_OBJECT (ti->account->protocol.pop3), "resolve",
								GTK_SIGNAL_FUNC (on_pop3_resolve), ti);

			gtk_signal_connect (GTK_OBJECT (ti->account->protocol.pop3), "status",
								GTK_SIGNAL_FUNC (on_pop3_status), ti);

			gtk_signal_connect (GTK_OBJECT (ti->account->protocol.pop3), "retrieve",
								GTK_SIGNAL_FUNC (on_pop3_retrieve), ti);

			gtk_signal_connect (GTK_OBJECT (ti->account->protocol.pop3), "disconnect",
								GTK_SIGNAL_FUNC (on_pop3_disconnect), ti);

			gtk_progress_set_show_text (GTK_PROGRESS (ti->progress_mail), TRUE);
			gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("Resolving"));
		} else
		{
			g_assert_not_reached ();
			return;
		}
		
		pthread_create (&thread, NULL, C2_PTHREAD_FUNC (c2_account_check), ti->account);
	}
}

static void
on_pop3_resolve (GtkObject *object, C2TransferItem *ti)
{
	gdk_threads_enter ();
	gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("Connecting"));
	gdk_threads_leave ();
}

static void
on_pop3_status (GtkObject *object, gint mails, C2TransferItem *ti)
{
	gtk_progress_configure (GTK_PROGRESS (ti->progress_mail), 0, 0, mails);

	if (!mails)
		gtk_progress_set_percentage (GTK_PROGRESS (ti->progress_mail), 1.0);
	else
	{
		if (mails == 1)
			gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("%u message."));
		else
			gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("%u messages."));
		
		gtk_progress_configure (GTK_PROGRESS (ti->progress_mail), 0, 0, mails);
	}
}

static void
on_pop3_retrieve (GtkObject *object, gint16 nth, gint32 received, gint32 total, C2TransferItem *ti)
{
	if (!received)
	{
		if (nth == 1)
			gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("Receiving %v of %u"));
		gtk_progress_set_value (GTK_PROGRESS (ti->progress_mail), nth);
	}
}

static void
on_pop3_disconnect (GtkObject *object, gboolean success, C2TransferItem *ti)
{
	gtk_progress_set_format_string (GTK_PROGRESS (ti->progress_mail), _("Completed"));
	gtk_progress_set_percentage (GTK_PROGRESS (ti->progress_mail), 1.0);
	gtk_widget_set_sensitive (ti->cancel_button, FALSE);
}