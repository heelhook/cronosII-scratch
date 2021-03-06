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
 * 		* Bosko Blagojevic
 **/
#include <libcronosII/imap.h>
#include <libcronosII/error.h>

#include "widget-application.h"
#include "widget-application-utils.h"
#include "widget-dialog-preferences.h"
#include "widget-mailbox-list.h"

static void
class_init									(C2MailboxListClass *klass);

static void
init										(C2MailboxList *mlist);

static void
destroy										(GtkObject *object);

static void
on_tree_select_row							(GtkCTree *ctree, GtkCTreeNode *row, gint column,
											 C2MailboxList *mlist);

static void
on_tree_unselect_row						(GtkCTree *ctree, GtkCTreeNode *row, gint column,
											 C2MailboxList *mlist);

static void
on_mlist_button_press_event					(GtkWidget *mlist, GdkEvent *event);

static void
on_menu_new_mailbox_activate				(GtkWidget *widget, C2MailboxList *mlist);

static void
on_menu_delete_mailbox_activate				(GtkWidget *widget, C2MailboxList *mlist);

static void
on_menu_properties_activate					(GtkWidget *widget, C2MailboxList *mlist);

static void
on_menu_use_as_inbox_toggled				(GtkWidget *widget, C2MailboxList *mlist);

static void
on_menu_use_as_outbox_toggled				(GtkWidget *widget, C2MailboxList *mlist);

static void
on_menu_use_as_sent_items_toggled			(GtkWidget *widget, C2MailboxList *mlist);

static void
on_menu_use_as_trash_toggled				(GtkWidget *widget, C2MailboxList *mlist);

static void
on_menu_use_as_drafts_toggled				(GtkWidget *widget, C2MailboxList *mlist);

static void
on_menu_subscribed_toggled					(GtkWidget *widget, C2MailboxList *mlist);

static void
on_menu_mailboxes_list_activate				(GtkWidget *widget, C2MailboxList *mlist);

static void
on_menu_only_subscribed_toggled				(GtkWidget *widget, C2MailboxList *mlist);

static void
on_tree_expand								(GtkCTree *ctree, GtkCTreeNode *node);

static void
on_tree_collapse							(GtkCTree *ctree, GtkCTreeNode *node);

static void
on_application_reload_mailboxes				(C2Application *application, C2MailboxList *mlist);

static void
on_application_preferences_changed			(C2Application *application, gint key, gpointer value,
											 C2MailboxList *mlist);

static GtkObject *
get_object_from_node						(C2MailboxList *mlist, GtkCTreeNode *node);

static void
on_mailbox_changed_mailbox					(C2Mailbox *mailbox, C2MailboxChangeType type,
											 C2Db *db, C2Pthread2 *data);

static void
account_node_fill							(GtkCTree *ctree, GtkCTreeNode *cnode,
											 C2Account *account);

static void
mailbox_node_fill							(GtkCTree *ctree, GtkCTreeNode *cnode,
											 C2Mailbox *mailbox, C2Db *start_db, gint *unreaded);

static void
tree_fill									(C2MailboxList *mlist, C2Mailbox *mailbox,
											 C2Account *account, GtkCTreeNode *node);

static GtkWidget *
get_pixmap									(C2Mailbox *mailbox, gboolean open);

static void
on_imap_mailbox_list						(C2IMAP *imap, C2Mailbox *head, C2Pthread2 *data);

static gboolean
on_imap_login_failed						(C2IMAP *imap, const gchar *error, gchar **user,
											 gchar **pass, C2Mutex *mutex);

enum
{
	OBJECT_SELECTED,
	OBJECT_UNSELECTED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GtkCTreeClass *parent_class = NULL;

GtkType
c2_mailbox_list_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2MailboxList",
			sizeof (C2MailboxList),
			sizeof (C2MailboxListClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (gtk_ctree_get_type (), &info);
	}

	return type;
}

static void
class_init (C2MailboxListClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_ctree_get_type ());

	signals[OBJECT_SELECTED] =
		gtk_signal_new ("object_selected",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2MailboxListClass, object_selected),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	signals[OBJECT_UNSELECTED] =
		gtk_signal_new ("object_unselected",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2MailboxListClass, object_unselected),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals(object_class, signals, LAST_SIGNAL);

	klass->object_selected = NULL;
	klass->object_unselected = NULL;
	object_class->destroy = destroy;
}

static void
init (C2MailboxList *mlist)
{	
	mlist->selected_node = NULL;
	mlist->mailbox_list = NULL;
	mlist->data_list = NULL;
}

static void
destroy (GtkObject *object)
{
	C2MailboxList *mlist;
	GList *ml, *dl;

	mlist = C2_MAILBOX_LIST (object);

	for (ml = mlist->mailbox_list, dl = mlist->data_list;
		 ml && dl;
		 ml = g_list_next (ml), dl = g_list_next (dl))
	{
		if (C2_IS_MAILBOX (ml->data))
			gtk_signal_disconnect_by_data (GTK_OBJECT (ml->data), dl->data);
		g_free (dl->data);
	}

	g_list_free (mlist->mailbox_list);
	g_list_free (mlist->data_list);
}

GtkWidget *
c2_mailbox_list_new (C2Application *application)
{
	C2MailboxList *mlist;
	GtkWidget *widget;
	GladeXML *xml;
	gchar *titles[] =
	{
		N_("Mailboxes")
	};

	mlist = gtk_type_new (c2_mailbox_list_get_type ());

	gtk_object_set_data (GTK_OBJECT (mlist), "application", application);

	gtk_ctree_construct (GTK_CTREE (mlist), 1, 0, titles);
	tree_fill (mlist, application->mailbox, application->account, NULL);

	gtk_signal_connect (GTK_OBJECT (mlist), "tree_select_row",
							GTK_SIGNAL_FUNC (on_tree_select_row), mlist);
	gtk_signal_connect (GTK_OBJECT (mlist), "tree_unselect_row",
							GTK_SIGNAL_FUNC (on_tree_unselect_row), mlist);
	gtk_signal_connect (GTK_OBJECT (mlist), "tree_expand",
							GTK_SIGNAL_FUNC (on_tree_expand), mlist);
	gtk_signal_connect (GTK_OBJECT (mlist), "tree_collapse",
							GTK_SIGNAL_FUNC (on_tree_collapse), mlist);
	gtk_signal_connect (GTK_OBJECT (mlist), "button_press_event",
      			GTK_SIGNAL_FUNC (on_mlist_button_press_event), NULL);
	
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "mnu_mlist");
			
	gtk_object_set_data (GTK_OBJECT (mlist), "menu", xml);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "new_mailbox")), "activate",
						GTK_SIGNAL_FUNC (on_menu_new_mailbox_activate), mlist);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "delete_mailbox")), "activate",
						GTK_SIGNAL_FUNC (on_menu_delete_mailbox_activate), mlist);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "properties")), "activate",
						GTK_SIGNAL_FUNC (on_menu_properties_activate), mlist);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "use_as_inbox")), "toggled",
						GTK_SIGNAL_FUNC (on_menu_use_as_inbox_toggled), mlist);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "use_as_outbox")), "toggled",
						GTK_SIGNAL_FUNC (on_menu_use_as_outbox_toggled), mlist);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "use_as_sent_items")), "toggled",
						GTK_SIGNAL_FUNC (on_menu_use_as_sent_items_toggled), mlist);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "use_as_trash")), "toggled",
						GTK_SIGNAL_FUNC (on_menu_use_as_trash_toggled), mlist);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "use_as_drafts")), "toggled",
						GTK_SIGNAL_FUNC (on_menu_use_as_drafts_toggled), mlist);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "subscribed")), "toggled",
						GTK_SIGNAL_FUNC (on_menu_subscribed_toggled), mlist);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "mailboxes_list")), "activate",
						GTK_SIGNAL_FUNC (on_menu_mailboxes_list_activate), mlist);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (xml, "only_subscribed")), "toggled",
						GTK_SIGNAL_FUNC (on_menu_only_subscribed_toggled), mlist);

	gtk_signal_connect (GTK_OBJECT (application), "reload_mailboxes",
							GTK_SIGNAL_FUNC (on_application_reload_mailboxes), mlist);
	gtk_signal_connect (GTK_OBJECT (application), "application_preferences_changed",
							GTK_SIGNAL_FUNC (on_application_preferences_changed), mlist);
	
	widget = GTK_WIDGET (mlist);
	GTK_WIDGET_UNSET_FLAGS (widget, GTK_CAN_FOCUS);
	gtk_clist_column_titles_passive (GTK_CLIST (mlist));
	return widget;
}

C2Mailbox *
c2_mailbox_list_get_selected_mailbox (C2MailboxList *mlist)
{
	GtkObject *object = get_object_from_node (mlist, mlist->selected_node);

	if (C2_IS_MAILBOX (object))
		return C2_MAILBOX (object);

	return NULL;
}

GtkObject *
c2_mailbox_list_get_selected_object (C2MailboxList *mlist)
{
	return get_object_from_node (mlist, mlist->selected_node);
}

void
c2_mailbox_list_set_selected_object (C2MailboxList *mlist, GtkObject *object)
{
	GtkCTreeNode *node;
	
	node = gtk_ctree_find_by_row_data (GTK_CTREE (mlist), gtk_ctree_node_nth (GTK_CTREE (mlist), 0), object);

	if (!node)
		return;

	gtk_ctree_select (GTK_CTREE (mlist), node);

	/* [TODO] Check if the selected signal is emitted, if not, emit it. */
}

static void
on_tree_select_row (GtkCTree *ctree, GtkCTreeNode *row, gint column, C2MailboxList *mlist)
{
	mlist->selected_node = row;

	gtk_signal_emit (GTK_OBJECT (mlist), signals[OBJECT_SELECTED], get_object_from_node (mlist, row));
}

static void
on_tree_unselect_row (GtkCTree *ctree, GtkCTreeNode *row, gint column, C2MailboxList *mlist)
{
	mlist->selected_node = NULL;

	gtk_signal_emit (GTK_OBJECT (mlist), signals[OBJECT_UNSELECTED]);
}

static void
on_mlist_button_press_event (GtkWidget *mlist, GdkEvent *event)
{
	GladeXML *xml;
	
	if (event->button.button == 3)
	{
		GdkEventButton *e = (GdkEventButton *) event;
		GtkCTreeNode *node;
		gint mbox_n, row, column;
		GtkObject *object;
		
		xml = GLADE_XML (gtk_object_get_data (GTK_OBJECT (mlist), "menu"));

		mbox_n = gtk_clist_get_selection_info (GTK_CLIST (mlist), e->x, e->y, &row, &column);

		if (mbox_n)
		{
			node = gtk_ctree_node_nth (GTK_CTREE (mlist), row);
			
			object = gtk_ctree_node_get_row_data (GTK_CTREE (mlist), node);
			c2_mailbox_list_set_selected_object (C2_MAILBOX_LIST (mlist), object);
			gtk_ctree_select (GTK_CTREE (mlist), node);

		} else
		{
			c2_mailbox_list_set_selected_object (C2_MAILBOX_LIST (mlist), NULL);
			object = NULL;
		}

		/* Set visibility */
		if (C2_IS_MAILBOX (object))
		{
			gtk_widget_show (glade_xml_get_widget (xml, "delete_mailbox"));
			gtk_widget_show (glade_xml_get_widget (xml, "properties"));
			gtk_widget_show (glade_xml_get_widget (xml, "use_as_inbox"));
			gtk_widget_show (glade_xml_get_widget (xml, "use_as_outbox"));
			gtk_widget_show (glade_xml_get_widget (xml, "use_as_sent_items"));
			gtk_widget_show (glade_xml_get_widget (xml, "use_as_trash"));
			gtk_widget_show (glade_xml_get_widget (xml, "use_as_drafts"));
			gtk_widget_show (glade_xml_get_widget (xml, "sep01"));
			gtk_widget_show (glade_xml_get_widget (xml, "sep02"));
			gtk_widget_hide (glade_xml_get_widget (xml, "mailboxes_list"));
			gtk_widget_hide (glade_xml_get_widget (xml, "only_subscribed"));

			if (C2_MAILBOX (object)->type == C2_MAILBOX_IMAP)
			{
				if (!C2_MAILBOX (object)->protocol.IMAP.noselect)
				{
					gtk_widget_hide (glade_xml_get_widget (xml, "subscribe"));
					gtk_widget_show (glade_xml_get_widget (xml, "unsubscribe"));
				} else
				{
					gtk_widget_show (glade_xml_get_widget (xml, "subscribe"));
					gtk_widget_hide (glade_xml_get_widget (xml, "unsubscribe"));
				}
				gtk_widget_show (glade_xml_get_widget (xml, "sep03"));
			} else
			{
				gtk_widget_hide (glade_xml_get_widget (xml, "subscribe"));
				gtk_widget_hide (glade_xml_get_widget (xml, "unsubscribe"));
				gtk_widget_hide (glade_xml_get_widget (xml, "sep03"));
			}
		} else
		{
			gtk_widget_hide (glade_xml_get_widget (xml, "delete_mailbox"));
			gtk_widget_hide (glade_xml_get_widget (xml, "properties"));
			gtk_widget_hide (glade_xml_get_widget (xml, "use_as_inbox"));
			gtk_widget_hide (glade_xml_get_widget (xml, "use_as_outbox"));
			gtk_widget_hide (glade_xml_get_widget (xml, "use_as_sent_items"));
			gtk_widget_hide (glade_xml_get_widget (xml, "use_as_trash"));
			gtk_widget_hide (glade_xml_get_widget (xml, "use_as_drafts"));
			gtk_widget_hide (glade_xml_get_widget (xml, "sep01"));
			gtk_widget_hide (glade_xml_get_widget (xml, "sep03"));

			if (C2_IS_ACCOUNT (object))
			{
				gtk_widget_show (glade_xml_get_widget (xml, "sep02"));
				gtk_widget_show (glade_xml_get_widget (xml, "mailboxes_list"));
				gtk_widget_show (glade_xml_get_widget (xml, "only_subscribed"));
			} else
			{
				gtk_widget_hide (glade_xml_get_widget (xml, "sep02"));
				gtk_widget_hide (glade_xml_get_widget (xml, "mailboxes_list"));
				gtk_widget_hide (glade_xml_get_widget (xml, "only_subscribed"));
			}
		}

		/* Set data */
		if (C2_IS_MAILBOX (object))
		{
			C2Mailbox *mailbox = C2_MAILBOX (object);
			GtkWidget *widget;
			
			widget = glade_xml_get_widget(xml,"use_as_inbox");
			gtk_signal_handler_block_by_func (GTK_OBJECT (widget),
												GTK_SIGNAL_FUNC (on_menu_use_as_inbox_toggled), mlist);
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(widget),
											mailbox->use_as & C2_MAILBOX_USE_AS_INBOX);
			gtk_signal_handler_unblock_by_func (GTK_OBJECT (widget),
												GTK_SIGNAL_FUNC (on_menu_use_as_inbox_toggled), mlist);
			gtk_widget_set_sensitive (widget, !(mailbox->use_as & C2_MAILBOX_USE_AS_INBOX));

			widget = glade_xml_get_widget(xml,"use_as_outbox");
			gtk_signal_handler_block_by_func (GTK_OBJECT (widget),
												GTK_SIGNAL_FUNC (on_menu_use_as_outbox_toggled), mlist);
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(widget),
											mailbox->use_as & C2_MAILBOX_USE_AS_OUTBOX);
			gtk_signal_handler_unblock_by_func (GTK_OBJECT (widget),
												GTK_SIGNAL_FUNC (on_menu_use_as_outbox_toggled), mlist);
			gtk_widget_set_sensitive (widget, !(mailbox->use_as & C2_MAILBOX_USE_AS_OUTBOX));

			widget = glade_xml_get_widget(xml,"use_as_sent_items");
			gtk_signal_handler_block_by_func (GTK_OBJECT (widget),
												GTK_SIGNAL_FUNC (on_menu_use_as_sent_items_toggled), mlist);
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(widget),
											mailbox->use_as & C2_MAILBOX_USE_AS_SENT_ITEMS);
			gtk_signal_handler_unblock_by_func (GTK_OBJECT (widget),
												GTK_SIGNAL_FUNC (on_menu_use_as_sent_items_toggled), mlist);
			gtk_widget_set_sensitive (widget, !(mailbox->use_as & C2_MAILBOX_USE_AS_SENT_ITEMS));

			widget = glade_xml_get_widget(xml,"use_as_trash");
			gtk_signal_handler_block_by_func (GTK_OBJECT (widget),
												GTK_SIGNAL_FUNC (on_menu_use_as_trash_toggled), mlist);
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(widget),
											mailbox->use_as & C2_MAILBOX_USE_AS_TRASH);
			gtk_signal_handler_unblock_by_func (GTK_OBJECT (widget),
												GTK_SIGNAL_FUNC (on_menu_use_as_trash_toggled), mlist);
			gtk_widget_set_sensitive (widget, !(mailbox->use_as & C2_MAILBOX_USE_AS_TRASH));

			widget = glade_xml_get_widget(xml,"use_as_drafts");
			gtk_signal_handler_block_by_func (GTK_OBJECT (widget),
												GTK_SIGNAL_FUNC (on_menu_use_as_drafts_toggled), mlist);
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(widget),
											mailbox->use_as & C2_MAILBOX_USE_AS_DRAFTS);
			gtk_signal_handler_unblock_by_func (GTK_OBJECT (widget),
												GTK_SIGNAL_FUNC (on_menu_use_as_drafts_toggled), mlist);
			gtk_widget_set_sensitive (widget, !(mailbox->use_as & C2_MAILBOX_USE_AS_DRAFTS));
		} else if (C2_IS_ACCOUNT (object))
		{
			C2Account *account = C2_ACCOUNT (object);
			C2IMAP *imap = C2_IMAP (c2_account_get_extra_data (account, C2_ACCOUNT_KEY_INCOMING, NULL));
			GtkWidget *widget = glade_xml_get_widget (xml, "only_subscribed");
			
			/* I have NO IDEA why when the callback is not blocked it will continue
			 * to emit the signal (or at least call the callback) until it crashes,
			 * this is just a wrap around, but I'd like to fix this in the proper way.
			 */
			gtk_signal_handler_block_by_func (GTK_OBJECT (widget),
												GTK_SIGNAL_FUNC (on_menu_only_subscribed_toggled), mlist);
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget),
											imap->only_subscribed);
			gtk_signal_handler_unblock_by_func (GTK_OBJECT (widget),
												GTK_SIGNAL_FUNC (on_menu_only_subscribed_toggled), mlist);
		}

		gnome_popup_menu_do_popup (glade_xml_get_widget (xml, "mnu_mlist"),
										NULL, NULL, e, NULL);
	}
}

static void
on_menu_new_mailbox_activate (GtkWidget *widget, C2MailboxList *mlist)
{
	C2Application *application;

	application = C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (mlist), "application"));

	c2_application_dialog_add_mailbox (application);
}

static void
on_menu_delete_mailbox_activate (GtkWidget *widget, C2MailboxList *mlist)
{
	C2Application *application;
	C2Mailbox *mailbox;

	/* Get the selected mailbox */
L	if (!C2_IS_MAILBOX ((mailbox = C2_MAILBOX (c2_mailbox_list_get_selected_object (mlist)))))
		return;
	
L	application = C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (mlist), "application"));

L	c2_application_dialog_remove_mailbox (application, mailbox);
}

static void
on_menu_properties_activate (GtkWidget *widget, C2MailboxList *mlist)
{
}

static void
on_menu_use_as_inbox_toggled (GtkWidget *widget, C2MailboxList *mlist)
{
	C2Mailbox *mailbox;
	gchar *path;
	gint id;

	/* Get the selected mailbox */
	if (!C2_IS_MAILBOX ((mailbox = C2_MAILBOX (c2_mailbox_list_get_selected_object (mlist)))))
		return;

	/* Update the mark */
	if (mailbox->type == C2_MAILBOX_IMAP)
		c2_mailbox_set_use_as (mailbox->protocol.IMAP.imap->mailboxes, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_INBOX);
	else
		c2_mailbox_set_use_as (C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (mlist),
													"application"))->mailbox, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_INBOX);

	/* Update the CheckMenuItem */
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget),
									mailbox->use_as & C2_MAILBOX_USE_AS_INBOX);

	/* Update the configuration */
	if ((id = c2_application_get_mailbox_configuration_id_by_name (mailbox->name)) < 0)
		return;

	path = g_strdup_printf (PACKAGE "/Mailbox %d/use_as", id);
	gnome_config_set_int (path, mailbox->use_as);
	gnome_config_sync ();
	g_free (path);
}

static void
on_menu_use_as_outbox_toggled (GtkWidget *widget, C2MailboxList *mlist)
{
	C2Mailbox *mailbox;
	gchar *path;
	gint id;

	/* Get the selected mailbox */
	if (!C2_IS_MAILBOX ((mailbox = C2_MAILBOX (c2_mailbox_list_get_selected_object (mlist)))))
		return;

	/* Update the mark */
	if (mailbox->type == C2_MAILBOX_IMAP)
		c2_mailbox_set_use_as (mailbox->protocol.IMAP.imap->mailboxes, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_OUTBOX);
	else
		c2_mailbox_set_use_as (C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (mlist),
													"application"))->mailbox, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_OUTBOX);

	/* Update the CheckMenuItem */
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget),
									mailbox->use_as & C2_MAILBOX_USE_AS_OUTBOX);

	/* Update the configuration */
	if ((id = c2_application_get_mailbox_configuration_id_by_name (mailbox->name)) < 0)
		return;

	path = g_strdup_printf (PACKAGE "/Mailbox %d/use_as", id);
	gnome_config_set_int (path, mailbox->use_as);
	gnome_config_sync ();
	g_free (path);
}

static void
on_menu_use_as_sent_items_toggled (GtkWidget *widget, C2MailboxList *mlist)
{
	C2Mailbox *mailbox;
	gchar *path;
	gint id;

	/* Get the selected mailbox */
	if (!C2_IS_MAILBOX ((mailbox = C2_MAILBOX (c2_mailbox_list_get_selected_object (mlist)))))
		return;

	/* Update the mark */
	if (mailbox->type == C2_MAILBOX_IMAP)
		c2_mailbox_set_use_as (mailbox->protocol.IMAP.imap->mailboxes, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_SENT_ITEMS);
	else
		c2_mailbox_set_use_as (C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (mlist),
													"application"))->mailbox, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_SENT_ITEMS);

	/* Update the CheckMenuItem */
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget),
									mailbox->use_as & C2_MAILBOX_USE_AS_SENT_ITEMS);

	/* Update the configuration */
	if ((id = c2_application_get_mailbox_configuration_id_by_name (mailbox->name)) < 0)
		return;

	path = g_strdup_printf (PACKAGE "/Mailbox %d/use_as", id);
	gnome_config_set_int (path, mailbox->use_as);
	gnome_config_sync ();
	g_free (path);
}

static void
on_menu_use_as_trash_toggled (GtkWidget *widget, C2MailboxList *mlist)
{
	C2Mailbox *mailbox;
	gchar *path;
	gint id;

	/* Get the selected mailbox */
	if (!C2_IS_MAILBOX ((mailbox = C2_MAILBOX (c2_mailbox_list_get_selected_object (mlist)))))
		return;

	/* Update the mark */
	if (mailbox->type == C2_MAILBOX_IMAP)
		c2_mailbox_set_use_as (mailbox->protocol.IMAP.imap->mailboxes, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_TRASH);
	else
		c2_mailbox_set_use_as (C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (mlist),
													"application"))->mailbox, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_TRASH);

	/* Update the CheckMenuItem */
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget),
									mailbox->use_as & C2_MAILBOX_USE_AS_TRASH);

	/* Update the configuration */
	if ((id = c2_application_get_mailbox_configuration_id_by_name (mailbox->name)) < 0)
		return;

	path = g_strdup_printf (PACKAGE "/Mailbox %d/use_as", id);
	gnome_config_set_int (path, mailbox->use_as);
	gnome_config_sync ();
	g_free (path);
}

static void
on_menu_use_as_drafts_toggled (GtkWidget *widget, C2MailboxList *mlist)
{
	C2Mailbox *mailbox;
	gchar *path;
	gint id;

	/* Get the selected mailbox */
	if (!C2_IS_MAILBOX ((mailbox = C2_MAILBOX (c2_mailbox_list_get_selected_object (mlist)))))
		return;

	/* Update the mark */
	if (mailbox->type == C2_MAILBOX_IMAP)
		c2_mailbox_set_use_as (mailbox->protocol.IMAP.imap->mailboxes, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_DRAFTS);
	else
		c2_mailbox_set_use_as (C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (mlist),
													"application"))->mailbox, mailbox,
								mailbox->use_as | C2_MAILBOX_USE_AS_DRAFTS);

	/* Update the CheckMenuItem */
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget),
									mailbox->use_as & C2_MAILBOX_USE_AS_DRAFTS);

	/* Update the configuration */
	if ((id = c2_application_get_mailbox_configuration_id_by_name (mailbox->name)) < 0)
		return;

	path = g_strdup_printf (PACKAGE "/Mailbox %d/use_as", id);
	gnome_config_set_int (path, mailbox->use_as);
	gnome_config_sync ();
	g_free (path);
}

static void
on_menu_subscribed_toggled (GtkWidget *widget, C2MailboxList *mlist)
{
}

static void
on_menu_mailboxes_list_activate (GtkWidget *widget, C2MailboxList *mlist)
{
}

static void
on_menu_only_subscribed_toggled_thread (C2IMAP *imap)
{
	c2_imap_populate_folders (imap);
}

static void
on_menu_only_subscribed_toggled (GtkWidget *widget, C2MailboxList *mlist)
{
	C2Account *account = C2_ACCOUNT (c2_mailbox_list_get_selected_object (mlist));
	C2IMAP *imap = C2_IMAP (c2_account_get_extra_data (account, C2_ACCOUNT_KEY_INCOMING, NULL));
	C2Mailbox *l;
	GtkCTreeNode *f_node, *a_node, *cnode;
	C2Pthread2 *data;
	pthread_t thread;

	if (!C2_IS_IMAP (imap))
		return;

	imap->only_subscribed ^= TRUE;
	
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget), imap->only_subscribed);

	/* Now destroy the tree we already had */
	gtk_clist_freeze (GTK_CLIST (mlist));

	/* Get the first node, this node WILL BE in the toplevel
	 * of the tree */
	f_node = gtk_ctree_node_nth (GTK_CTREE (mlist), 0);

	/* Now look for this account's node */
	a_node = gtk_ctree_find_by_row_data (GTK_CTREE (mlist), f_node, account);

	/* Get the nodes and remove them */
	for (l = imap->mailboxes; l; l = l->next)
	{
		cnode = gtk_ctree_find_by_row_data (GTK_CTREE (mlist), a_node, l);
		if (!cnode)
			continue;

		gtk_ctree_remove_node (GTK_CTREE (mlist), cnode);
	}
	
	gtk_clist_thaw (GTK_CLIST (mlist));

	data = g_new0 (C2Pthread2, 1);
	data->v1 = mlist;
	data->v2 = a_node;
	gtk_signal_connect (GTK_OBJECT (imap), "mailbox_list",
							GTK_SIGNAL_FUNC (on_imap_mailbox_list), data);
	
	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (on_menu_only_subscribed_toggled_thread), imap);
}

static void
on_tree_expand (GtkCTree *ctree, GtkCTreeNode *node)
{
	C2Mailbox *mailbox;
	C2Account *account;
	gint unreaded;

	if (C2_IS_MAILBOX (mailbox = (C2Mailbox*) gtk_ctree_node_get_row_data (ctree, node)))
		mailbox_node_fill (ctree, node, mailbox, NULL, &unreaded);
	else if (C2_IS_ACCOUNT (account = (C2Account*) gtk_ctree_node_get_row_data (ctree, node)) || !account)
		account_node_fill (ctree, node, account);
}

static void
on_tree_collapse (GtkCTree *ctree, GtkCTreeNode *node)
{
	C2Mailbox *mailbox;
	C2Account *account;
	gint unreaded;

	if (C2_IS_MAILBOX (mailbox = (C2Mailbox*) gtk_ctree_node_get_row_data (ctree, node)))
		mailbox_node_fill (ctree, node, mailbox, NULL, &unreaded);
	else if (C2_IS_ACCOUNT (account = (C2Account*) gtk_ctree_node_get_row_data (ctree, node)) || !account)
		account_node_fill (ctree, node, account);
}

static void
on_application_reload_mailboxes (C2Application *application, C2MailboxList *mlist)
{
	tree_fill (mlist, application->mailbox, application->account, NULL);
}

static void
on_application_preferences_changed (C2Application *application, gint key, gpointer value,
	 C2MailboxList *mlist)
{
	if (key == C2_DIALOG_PREFERENCES_KEY_INTERFACE_FONTS_UNREADED_MAILBOX ||
		key == C2_DIALOG_PREFERENCES_KEY_INTERFACE_FONTS_READED_MAILBOX)
	{
		tree_fill (mlist, application->mailbox, application->account, NULL);
	}
}

static GtkObject *
get_object_from_node (C2MailboxList *mlist, GtkCTreeNode *node)
{
	return GTK_OBJECT (gtk_ctree_node_get_row_data (GTK_CTREE (mlist), node));
}

static void
on_mailbox_changed_mailbox (C2Mailbox *mailbox, C2MailboxChangeType type, C2Db *db, C2Pthread2 *data)
{
	GtkCTree *ctree = GTK_CTREE (data->v1);
	GtkCTreeNode *cnode = GTK_CTREE_NODE (data->v2);
	gint unreaded;

	switch (type)
	{
		case C2_MAILBOX_CHANGE_ANY:
		case C2_MAILBOX_CHANGE_ADD:
		case C2_MAILBOX_CHANGE_REMOVE:
		case C2_MAILBOX_CHANGE_STATE:
			printf ("entrando\n");
			gdk_threads_enter ();
L			mailbox_node_fill (ctree, cnode, mailbox, NULL, &unreaded);
L			gdk_threads_leave ();
			printf ("saliendo\n");
			break;
	}
}

static void
account_node_fill (GtkCTree *ctree, GtkCTreeNode *cnode, C2Account *account)
{
	GtkStyle *style;
	GtkWidget *pixmap;
	C2Application *application;

	application = C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (ctree), "application"));
	
	if (!C2_IS_ACCOUNT (account))
	{
		gchar *text;
		gchar *buf1, *buf2;

		
		buf1 = g_getenv ("HOSTNAME");
		buf2 = g_get_user_name ();
		if (!buf1 || !strlen (buf1))
			text = g_strdup (_("Local Mailboxes"));
		else
			text = g_strdup_printf ("%s@%s", buf2, buf1);
		
		pixmap = get_pixmap (NULL, TRUE);
		gtk_ctree_node_set_pixtext (ctree, cnode, 0, text, 2, GNOME_PIXMAP (pixmap)->pixmap,
									GNOME_PIXMAP (pixmap)->mask);

		g_free (text);
	} else
	{
		pixmap = get_pixmap (NULL, TRUE);
		gtk_ctree_node_set_pixtext (ctree, cnode, 0, account->name, 2, GNOME_PIXMAP (pixmap)->pixmap,
									GNOME_PIXMAP (pixmap)->mask);
	}

	style = gtk_ctree_node_get_row_style (ctree, cnode);
	if (!style)
		style = gtk_style_copy (GTK_WIDGET (ctree)->style);
	
	style->font = application->fonts_gdk_unreaded_mailbox;
	
	gtk_ctree_node_set_row_style (ctree, cnode, style);
	gtk_ctree_node_set_row_data (ctree, cnode, (gpointer) account);
}

static void
mailbox_node_fill (GtkCTree *ctree, GtkCTreeNode *cnode, C2Mailbox *mailbox, C2Db *start_db, gint *unreaded)
{
	C2Application *application;
	GtkWidget *pixmap;
	GtkStyle *style;
	C2Db *ldb;
	gchar *text;

	application = C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (ctree), "application"));
	
	pixmap = get_pixmap (mailbox, GTK_CTREE_ROW (cnode)->expanded);

	ldb = start_db ? start_db : mailbox->db;

	*unreaded = c2_db_length_type (mailbox, C2_MESSAGE_UNREADED);
	
	if ((*unreaded))
		text = g_strdup_printf ("%s (%d)", mailbox->name, (*unreaded));
	else
		text = mailbox->name;

	gtk_ctree_node_set_pixtext (ctree, cnode, 0, text, 2, GNOME_PIXMAP (pixmap)->pixmap,
								GNOME_PIXMAP (pixmap)->mask);

	if ((*unreaded))
		g_free (text);

	if (GTK_CTREE_ROW (cnode)->expanded)
	{
		GTK_CTREE_ROW (cnode)->pixmap_opened = GNOME_PIXMAP (pixmap)->pixmap;
		GTK_CTREE_ROW (cnode)->mask_opened = GNOME_PIXMAP (pixmap)->mask;
		pixmap = get_pixmap (mailbox, !GTK_CTREE_ROW (cnode)->expanded);
		GTK_CTREE_ROW (cnode)->pixmap_closed = GNOME_PIXMAP (pixmap)->pixmap;
		GTK_CTREE_ROW (cnode)->mask_closed = GNOME_PIXMAP (pixmap)->mask;
	} else
	{
		GTK_CTREE_ROW (cnode)->pixmap_closed = GNOME_PIXMAP (pixmap)->pixmap;
		GTK_CTREE_ROW (cnode)->mask_closed = GNOME_PIXMAP (pixmap)->mask;
		pixmap = get_pixmap (mailbox, !GTK_CTREE_ROW (cnode)->expanded);
		GTK_CTREE_ROW (cnode)->pixmap_opened = GNOME_PIXMAP (pixmap)->pixmap;
		GTK_CTREE_ROW (cnode)->mask_opened = GNOME_PIXMAP (pixmap)->mask;
	}

	style = gtk_ctree_node_get_row_style (ctree, cnode);
	if (!style)
		style = gtk_style_copy (GTK_WIDGET (ctree)->style);
	
	if ((*unreaded))
	{
		style->font = application->fonts_gdk_unreaded_mailbox;
	} else
	{
		style->font = application->fonts_gdk_readed_mailbox;
	}
	
	gtk_ctree_node_set_row_style (ctree, cnode, style);
}

static void
init_imap (C2IMAP *imap)
{
	if (c2_imap_init (imap) < 0)
	{
		gchar *error;
		GtkWidget *dialog;
		
		gdk_threads_enter ();
		error = c2_error_object_get (GTK_OBJECT (imap));

		if (error)
		{
			dialog = gnome_warning_dialog (error);
			gtk_widget_show (dialog);
		}
		
		gdk_threads_leave ();
		
		return;
	}
	
	c2_imap_populate_folders (imap);
}

static void
tree_fill (C2MailboxList *mlist, C2Mailbox *mailbox, C2Account *account,
			GtkCTreeNode *node)
{
	C2Pthread2 *data;
	C2Mailbox *l;
	GtkCTree *ctree = GTK_CTREE (mlist);
	GtkCTreeNode *cnode;
	gint unreaded;
	gboolean toplevel = node?FALSE:TRUE;

	if (toplevel)
	{
		gtk_clist_freeze (GTK_CLIST (mlist));
		gtk_clist_clear (GTK_CLIST (mlist));

		node = gtk_ctree_insert_node (ctree, NULL, NULL, NULL, 4, NULL, NULL, NULL, NULL, FALSE, TRUE);
		account_node_fill (ctree, node, NULL);
	}

	for (l = mailbox; l; l = l->next)
	{
		data = g_new0 (C2Pthread2, 1);
		data->v1 = (gpointer) ctree;
		cnode = gtk_ctree_insert_node (ctree, node, NULL, NULL, 4, NULL, NULL, NULL, NULL, FALSE, TRUE);
		gtk_ctree_node_set_row_data (ctree, cnode, (gpointer) l);
		data->v2 = (gpointer) cnode;

		mailbox_node_fill (ctree, cnode, l, NULL, &unreaded);

		gtk_signal_connect (GTK_OBJECT (l), "changed_mailbox",
							GTK_SIGNAL_FUNC (on_mailbox_changed_mailbox), data);
		mlist->data_list = g_list_prepend (mlist->data_list, data);
		mlist->mailbox_list = g_list_prepend (mlist->mailbox_list, l);
		
		if (l->child)
			tree_fill (mlist, l->child, account, cnode);
	}

	if (toplevel)
	{
		C2Account *la;
		C2Application *application;

		application = C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (mlist), "application"));
		
		for (la = account; la; la = c2_account_next (la))
		{
			if (la->type == C2_ACCOUNT_IMAP)
			{
				C2IMAP *imap;
				C2Pthread2 *data;
				pthread_t thread;
				
				/* Create the node */
				cnode = gtk_ctree_insert_node (ctree, NULL, NULL, NULL, 4, NULL, NULL, NULL, NULL,
												FALSE, TRUE);
				account_node_fill (ctree, cnode, la);

				/* Get the IMAP object */
				imap = C2_IMAP (c2_account_get_extra_data (la, C2_ACCOUNT_KEY_INCOMING, NULL));

				/* Set data in the IMAP object */
				C2_IMAP_CLASS_FW (imap)->login_failed = on_imap_login_failed;
				gtk_object_set_data (GTK_OBJECT (imap), "account", account);
				gtk_object_set_data (GTK_OBJECT (imap), "login_failed::data", mlist);
				
				if (!imap->mailboxes)
				{
					data = g_new0 (C2Pthread2, 1);
					data->v1 = mlist;
					data->v2 = cnode;
					gtk_signal_connect (GTK_OBJECT (imap), "mailbox_list",
										GTK_SIGNAL_FUNC (on_imap_mailbox_list), data);

					/* Initializate it */
					pthread_create (&thread, NULL, C2_PTHREAD_FUNC (init_imap), imap);
				} else
				{
					tree_fill (mlist, imap->mailboxes, NULL, cnode);
				}
			}
		}
		
		gtk_clist_thaw (GTK_CLIST (mlist));
		/* FIXME - Here's where The Fucking Problem is.
		 * TFP is (no, is not FTP..) that when the indicator
		 * of new-mails in the mailbox-list is switched (that is
		 * bold is used or not used) the font is not updated until
		 * a new configure_event is signaled. */
	}
}

static GtkWidget *
get_pixmap (C2Mailbox *mailbox, gboolean open)
{
	if (C2_IS_MAILBOX (mailbox))
	{
		if (mailbox->use_as & C2_MAILBOX_USE_AS_INBOX)
			return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/inbox.png");
		else if (mailbox->use_as & C2_MAILBOX_USE_AS_SENT_ITEMS)
			return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/outbox.png");
		else if (mailbox->use_as & C2_MAILBOX_USE_AS_OUTBOX)
			return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/queue.png");
		else if (mailbox->use_as & C2_MAILBOX_USE_AS_TRASH)
			return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/garbage.png");
		else if (mailbox->use_as & C2_MAILBOX_USE_AS_DRAFTS)
			return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/drafts.png");
		else
		{
			if (mailbox->child)
			{
				if (!open)
					return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/folder-closed.png");
				else
					return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/folder-opened.png");
			} else
			{
				return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/mailbox.png");
			}
		}
	} else
	{
		if (!open)
				return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/account16.png");
			else
				return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/account16.png");
	}

	return gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/mailbox.png");
}

static void
on_imap_mailbox_list (C2IMAP *imap, C2Mailbox *head, C2Pthread2 *data)
{
	C2MailboxList *mlist = C2_MAILBOX_LIST (data->v1);
	GtkCTreeNode *cnode = (GtkCTreeNode*) data->v2;
	C2Mailbox *l;

	g_free (data);
	
	gdk_threads_enter ();
	tree_fill (mlist, head, NULL, cnode);
	gdk_threads_leave ();

	gtk_signal_disconnect_by_func (GTK_OBJECT (imap),
									GTK_SIGNAL_FUNC (on_imap_mailbox_list), data);
}

static void
on_imap_login_failed_ok_clicked (GtkWidget *widget, C2Pthread2 *data)
{
	C2Mutex *lock = (C2Mutex*) data->v1;
	
	GPOINTER_TO_INT (data->v2) = 0;
	
	c2_mutex_unlock (lock);
}

static void
on_imap_login_failed_cancel_clicked (GtkWidget *widget, C2Pthread2 *data)
{
	C2Mutex *lock = (C2Mutex*) data->v1;
	
	GPOINTER_TO_INT (data->v2) = 1;
	
	c2_mutex_unlock (lock);
}

static gboolean
on_imap_login_failed_dialog_delete_event (GtkWidget *widget, GdkEvent *e, C2Pthread2 *data)
{
	C2Mutex *lock = (C2Mutex*) data->v1;
	
	GPOINTER_TO_INT (data->v2) = 1;
	
	c2_mutex_unlock (lock);
	
	return FALSE;
}

static gboolean
on_imap_login_failed (C2IMAP *imap, const gchar *error, gchar **user, gchar **pass, C2Mutex *mutex)
{
	GladeXML *xml;
	GtkWidget *dialog;
	GtkWidget *contents;
	GtkWidget *mlist;
	C2Account *account;
	C2Pthread2 *data;
	C2Mutex local_lock;
	gchar *label, *buffer;
	gint button;
	C2Application *application;

	account = C2_ACCOUNT (gtk_object_get_data (GTK_OBJECT (imap), "account"));
	data = g_new0 (C2Pthread2, 1);
	c2_mutex_init (&local_lock);
	c2_mutex_lock (&local_lock);
	data->v1 = (gpointer) &local_lock;
	
	gdk_threads_enter ();
	
	mlist = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (imap), "login_failed::data"));
	application = C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (mlist), "application"));
	
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"), "dlg_req_pass_contents");
	contents = glade_xml_get_widget (xml, "dlg_req_pass_contents");
	dialog = c2_dialog_new (application, _("Login failed"), "req_pass", NULL,
							GNOME_STOCK_BUTTON_OK, GNOME_STOCK_BUTTON_CANCEL, NULL);
	C2_DIALOG (dialog)->xml = xml;
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), contents, TRUE, TRUE, 0);
	
	contents = glade_xml_get_widget (xml, "error_label");
	gtk_label_get (GTK_LABEL (contents), &buffer);
	label = g_strdup_printf ("%s '%s'.", buffer, error);
	gtk_label_set_text (GTK_LABEL (contents), label);
	
	contents = glade_xml_get_widget (xml, "account");
	gtk_entry_set_text (GTK_ENTRY (contents), account->name);
	contents = glade_xml_get_widget (xml, "user");
	gtk_entry_set_text (GTK_ENTRY (contents), imap->user);
	contents = glade_xml_get_widget (xml, "pass");
	gtk_entry_set_text (GTK_ENTRY (contents), imap->pass);
	gtk_widget_grab_focus (contents);
	
	gnome_dialog_button_connect (GNOME_DIALOG (dialog), 0,
								GTK_SIGNAL_FUNC (on_imap_login_failed_ok_clicked), data);
	gnome_dialog_button_connect (GNOME_DIALOG (dialog), 1,
								GTK_SIGNAL_FUNC (on_imap_login_failed_cancel_clicked), data);
	gtk_signal_connect (GTK_OBJECT (dialog), "delete_event",
								GTK_SIGNAL_FUNC (on_imap_login_failed_dialog_delete_event), data);
	gtk_widget_show (dialog);
	
	gdk_threads_leave ();
	
	c2_mutex_lock (&local_lock);
	c2_mutex_unlock (&local_lock);
	c2_mutex_destroy (&local_lock);
	
	button = GPOINTER_TO_INT (data->v2);
	gdk_threads_enter ();
	if (!button)
	{
		gint nth;
		
		contents = glade_xml_get_widget (xml, "user");
		*user = gtk_entry_get_text (GTK_ENTRY (contents));
		contents = glade_xml_get_widget (xml, "pass");
		*pass = gtk_entry_get_text (GTK_ENTRY (contents));
		
		nth = c2_account_get_position (application->account, account);
		buffer = g_strdup_printf ("/"PACKAGE"/Account %d/", nth);
		gnome_config_push_prefix (buffer);
		gnome_config_set_string ("incoming_server_username", *user);
		if (imap->auth_remember)
			gnome_config_set_string ("incoming_server_password", *pass);
		gnome_config_pop_prefix ();
		gnome_config_sync ();
		g_free (buffer);
	}
	
	gtk_widget_destroy (dialog);
	gdk_threads_leave ();
	c2_mutex_unlock (mutex);
	
	return !button;
}
