/*  Cronos II - A GNOME mail client
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
#include <glade/glade.h>

#include "widget-composer.h"

#define XML_FILE "cronosII-composer"

GtkWidget *
c2_composer_new (GtkToolbarStyle style)
{
	return NULL;
}

GtkType
c2_composer_get_type (void)
{
	return 0;
}

#if 0
static void
class_init									(C2ComposerClass *klass);

static void
init										(C2Composer *composer);

enum
{
	CHANGED_TITLE,
	SEND_NOW,
	SEND_LATER,
	SAVE_DRAFT,
	CLOSE,
	LAST_SIGNAL
};

static gint signals[LAST_SIGNAL] = { 0 };

static GtkWidgetClass *parent_class = NULL;

GtkType
c2_composer_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		GtkTypeInfo info =
		{
			"C2Composer",
			sizeof (C2Composer),
			sizeof (C2ComposerClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_widget_get_type (), &info);
	}

	return type;
}

static void
class_init (C2ComposerClass *klass)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_widget_get_type ());

	signals[CHANGED_TITLE] =
		gtk_signal_new ("changed_title",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2ComposerClass, changed_title),
						gtk_marshal_NONE__STRING, GTK_TYPE_NONE, 1,
						GTK_TYPE_STRING);

	signals[SEND_NOW] =
		gtk_signal_new ("send_now",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2ComposerClass, send_now),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);

	signals[SEND_LATER] =
		gtk_signal_new ("send_later",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2ComposerClass, send_later),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);

	signals[SAVE_DRAFT] =
		gtk_signal_new ("save_draft",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2ComposerClass, save_draft),
						gtk_marshal_NONE__INT, GTK_TYPE_NONE, 1,
						GTK_TYPE_INT);

	signals[CLOSE] =
		gtk_signal_new ("close",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2ComposerClass, close),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	klass->changed_title = NULL;
	klass->send_now = NULL;
	klass->send_later = NULL;
	klass->save_draft = NULL;
	klass->close = NULL;
}

static void
init (C2Composer *composer)
{
	composer->file = NULL;
	composer->draft_id = -1;
	composer->operations = NULL;
	composer->operations_ptr = NULL;
}

GtkWidget *
c2_composer_new (GtkToolbarStyle style)
{
	C2Composer *composer;

	composer = gtk_type_new (c2_composer_get_type ());
	c2_composer_construct (composer, style);

	return GTK_WIDGET (composer);
}

void
c2_composer_construct (C2Composer *composer, GtkToolbarStyle style)
{
	GtkWidget *menu;
	GtkWidget *widget;
	gchar *str;
	gint i;
	C2Account *account;
	
	composer->xml = glade_xml_new (C2_APP_GLADE_FILE (XML_FILE), "wnd_composer");
	
	for (i = 0; i <= 9; i++)
	{
		switch (i)
		{
			case 0:
				str = "file_send_now";
				break;
			case 1:
				str = "file_send_later";
				break;
			case 2:
				str = "edit_undo";
				break;
			case 3:
				str = "edit_redo";
				break;
			case 4:
				str = "insert_image";
				break;
			case 5:
				str = "insert_link";
				break;
			case 6:
				str = "send_now_btn";
				break;
			case 7:
				str = "send_later_btn";
				break;
			case 8:
				str = "undo_btn";
				break;
			case 9:
				str = "redo_btn";
				break;
		}
		
		if ((widget = glade_xml_get_widget (composer->xml, str)))
			gtk_widget_set_sensitive (widget, FALSE);
	}

	menu = glade_xml_get_widget (composer->xml, "account_menu");
	for (account = c2_app.account; account; account = account->next)
	{
		str = g_strdup_printf ("\"%s\" <%s> (%s)", account->per_name, account->email, account->name);
		widget = gtk_menu_item_new_with_label (str);
		gtk_widget_show (widget);
		gtk_menu_append (GTK_MENU (GTK_OPTION_MENU (menu)->menu), widget);
	}
	gtk_option_menu_set_history (GTK_OPTION_MENU (menu), 0);

	gtk_toolbar_set_style (GTK_TOOLBAR (glade_xml_get_widget (composer->xml, "toolbar")), style);
	gtk_widget_queue_resize (glade_xml_get_widget (composer->xml, "wnd_composer"));
}

GtkWidget *
c2_composer_get_window (C2Composer *composer)
{
	return glade_xml_get_widget (composer->xml, "wnd_composer");
}
#endif
