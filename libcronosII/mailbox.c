/*  Cronos II Mail Client /libcronosII/mailbox.c
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
#include <glib.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <config.h>

#include "error.h"
#include "mailbox.h"
#include "utils.h"

static C2Mailbox *
c2_mailbox_init									(C2Mailbox *mailbox);

static void
c2_mailbox_class_init							(C2MailboxClass *klass);

static void
c2_mailbox_destroy								(GtkObject *object);

static void
c2_mailbox_insert								(C2Mailbox *head, C2Mailbox *mailbox);

static void
c2_mailbox_recreate_tree_ids					(C2Mailbox *head);

#define c2_mailbox_search_by_id(x,y)			_c2_mailbox_search_by_id (x, y, 1)

#define c2_mailbox_get_parent(x,y)				c2_mailbox_search_by_id (x, c2_mailbox_get_parent_id (y->id))

static C2Mailbox *
_c2_mailbox_search_by_id						(C2Mailbox *head, const gchar *id, guint level);

static void
c2_mailbox_set_head								(C2Mailbox *mailbox);

static gchar *
c2_mailbox_create_id_from_parent				(C2Mailbox *parent);

enum
{
	CHANGED_MAILBOXES,
	CHANGED_MAILBOX,
	DB_LOADED,
	LAST_SIGNAL
};

static guint c2_mailbox_signals[LAST_SIGNAL] = { 0 };

static GtkObjectClass *parent_class = NULL;

static C2Mailbox *mailbox_head = NULL;

GtkType
c2_mailbox_get_type (void)
{
	static GtkType c2_mailbox_type = 0;

	if (!c2_mailbox_type)
	{
		static const GtkTypeInfo c2_mailbox_info =
		{
			"C2Mailbox",
			sizeof (C2Mailbox),
			sizeof (C2MailboxClass),
			(GtkClassInitFunc) c2_mailbox_class_init,
			(GtkObjectInitFunc) c2_mailbox_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};

		c2_mailbox_type = gtk_type_unique (gtk_object_get_type (), &c2_mailbox_info);
	}

	return c2_mailbox_type;
}

static void
c2_mailbox_class_init (C2MailboxClass *klass)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_object_get_type ());

	c2_mailbox_signals[CHANGED_MAILBOXES] =
		gtk_signal_new ("changed_mailboxes",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2MailboxClass, changed_mailboxes),
					gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);

	c2_mailbox_signals[CHANGED_MAILBOX] =
		gtk_signal_new ("changed_mailbox",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2MailboxClass, changed_mailbox),
					gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
					GTK_TYPE_POINTER);

	c2_mailbox_signals[DB_LOADED] =
		gtk_signal_new ("db_loaded",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2MailboxClass, db_loaded),
					gtk_marshal_NONE__BOOL, GTK_TYPE_NONE, 1,
					GTK_TYPE_BOOL);

	gtk_object_class_add_signals (object_class, c2_mailbox_signals, LAST_SIGNAL);

	object_class->destroy = c2_mailbox_destroy;

	klass->changed_mailboxes = NULL;
	klass->changed_mailbox = NULL;
	klass->db_loaded = NULL;
}

static C2Mailbox *
c2_mailbox_init (C2Mailbox *mailbox)
{
	mailbox->name = NULL;
	mailbox->id = NULL;
	mailbox->selection = -1;
	mailbox->db = NULL;
	mailbox->last_mid = -1;
	mailbox->next = NULL;
	mailbox->child = NULL;

	return mailbox;
}

static void
c2_mailbox_destroy_node (C2Mailbox *mailbox)
{
	c2_return_if_fail (mailbox, C2EDATA);

	switch (mailbox->type)
	{
		case C2_MAILBOX_IMAP:
			g_free (mailbox->protocol.imap.host);
			g_free (mailbox->protocol.imap.user);
			g_free (mailbox->protocol.imap.pass);
			g_free (mailbox->protocol.imap.path);
			break;
		case C2_MAILBOX_SPOOL:
			g_free (mailbox->protocol.spool.path);
			break;
	}

	g_free (mailbox->name);
	g_free (mailbox->id);
	if (mailbox->db)
		gtk_object_unref (GTK_OBJECT (mailbox->db));
	if (mailbox->child)
		gtk_object_unref (GTK_OBJECT (mailbox->child));
}

static void
c2_mailbox_destroy (GtkObject *object)
{
	C2Mailbox *mailbox, *l, *n;

	c2_return_if_fail (C2_IS_MAILBOX (object), C2EDATA);

	mailbox = C2_MAILBOX (object);
	
	if (c2_mailbox_get_head () == mailbox)
		c2_mailbox_set_head (mailbox->next);
	
	for (l = mailbox->child; l != NULL; l = n)
	{
		n = l->next;
		c2_mailbox_destroy_node (l);
	}
	mailbox->child = NULL;
	c2_mailbox_destroy_node (mailbox);
}

/**
 * c2_mailbox_new
 * @name: Name of mailbox.
 * @id: Id of mailbox.
 * @type: Type of mailbox.
 * @sort_by: Column to sort the mailbox by.
 * @sort_type: Wheter to use ascending or descending sorting.
 * ...: Specific information about the mailbox according to the type
 * 		of mailbox.
 * 		Cronos II Mailbox's type: Null.
 * 		IMAP Mailbox's type: gchar *server, gint port, gchar *user, gchar *pass, gchar *path.
 * 		Spool Mailbox's type: gchar *path.
 *
 * This function will allocate a new C2Mailbox object
 * and fill it with required information.
 * This function should be used when the mailbox already
 * has an Id assigned, when you are creating a new mailbox
 * and you don't know about the Id (you just know the Id
 * of the parent) you should use c2_mailbox_new_with_parent.
 * 
 * This function will also insert the mailbox in the mailboxes hash
 * tree according to the Id.
 *
 * Return Value:
 * The new mailbox object.
 **/
C2Mailbox *
c2_mailbox_new (const gchar *name, const gchar *id, C2MailboxType type,
				C2MailboxSortBy sort_by, GtkSortType sort_type, ...)
{
	C2Mailbox *mailbox;
	va_list edata;

	c2_return_val_if_fail (name, NULL, C2EDATA);
	c2_return_val_if_fail (id, NULL, C2EDATA);
	
	mailbox = gtk_type_new (C2_TYPE_MAILBOX);
	mailbox->name = g_strdup (name);
	mailbox->id = g_strdup (id);
	mailbox->type = type;

	mailbox->sort_by = sort_by;
	mailbox->sort_type = sort_type;
	
	mailbox->selection = -1;
	mailbox->db = NULL;
	mailbox->last_mid = -1;

	switch (type)
	{
		case C2_MAILBOX_CRONOSII:
			break;
		case C2_MAILBOX_IMAP:
			va_start (edata, sort_type);
			mailbox->protocol.imap.host = g_strdup (va_arg (edata, gchar *));
			mailbox->protocol.imap.port = va_arg (edata, gint);
			mailbox->protocol.imap.user = g_strdup (va_arg (edata, gchar *));
			mailbox->protocol.imap.pass = g_strdup (va_arg (edata, gchar *));
			mailbox->protocol.imap.path = g_strdup (va_arg (edata, gchar *));
			mailbox->protocol.imap.sock = -1;
			mailbox->protocol.imap.cmnd_n = 1;
			va_end (edata);
			break;
		case C2_MAILBOX_SPOOL:
			va_start (edata, sort_type);
			mailbox->protocol.spool.path = g_strdup (va_arg (edata, gchar *));
			va_end (edata);
			break;
#ifdef USE_DEBUG
		default:
			g_print ("Unknown mailbox type in %s (%s:%d): %d\n",
					__PRETTY_FUNCTION__, __FILE__, __LINE__, type);
#endif
	}

	if (!mailbox_head)
		c2_mailbox_set_head (mailbox);
	else
		c2_mailbox_insert (mailbox_head, mailbox);

	gtk_signal_emit (GTK_OBJECT (mailbox_head),
						c2_mailbox_signals[CHANGED_MAILBOXES]);

	return mailbox;
}

C2Mailbox *
c2_mailbox_new_with_parent (const gchar *name, const gchar *parent_id, C2MailboxType type,
							C2MailboxSortBy sort_by, GtkSortType sort_type, ...)
{
	C2Mailbox *parent;
	C2Mailbox *value;
	gchar *id;
	gchar *host, *user, *pass, *path;
	gint port;
	va_list edata;
	
	c2_return_val_if_fail (name, NULL, C2EDATA);

	if (parent_id)
	{
		if (!(parent = c2_mailbox_search_by_id (mailbox_head, parent_id)))
		{
			c2_error_set (C2EDATA);
			return NULL;
		}
		
		id = c2_mailbox_create_id_from_parent (parent);
	} else
	{
		if ((parent = c2_mailbox_get_head ()))
		{
			for (; parent->next; parent = parent->next);
			id = g_strdup_printf ("%d", atoi (parent->id)+1);
		} else
		{
			id = g_strdup ("0");
		}
	}

	switch (type)
	{
		case C2_MAILBOX_CRONOSII:
			value = c2_mailbox_new (name, id, type, sort_by, sort_type);
			break;
		case C2_MAILBOX_IMAP:
			va_start (edata, sort_type);
			host = va_arg (edata, gchar *);
			port = va_arg (edata, gint);
			user = va_arg (edata, gchar *);
			pass = va_arg (edata, gchar *);
			path = va_arg (edata, gchar *);
	
			value = c2_mailbox_new (name, id, type, sort_by, sort_type, host, port, user, pass, path);
			va_end (edata);
			break;
		case C2_MAILBOX_SPOOL:
			va_start (edata, sort_type);
			path = va_arg (edata, gchar *);
		
			value = c2_mailbox_new (name, id, type, sort_by, sort_type, path);
			va_end (edata);
			break;
	}
	g_free (id);
	
	return value;
}

/**
 * c2_mailbox_destroy_tree
 *
 * Unref the whole mailboxes tree.
 **/
void
c2_mailbox_destroy_tree (void)
{
	C2Mailbox *l;

	for (l = c2_mailbox_get_head (); l != NULL; l = l->next)
		gtk_object_unref (GTK_OBJECT (l));
	gtk_signal_emit (GTK_OBJECT (mailbox_head),
						c2_mailbox_signals[CHANGED_MAILBOXES]);
	c2_mailbox_set_head (NULL);
}

static void
c2_mailbox_insert (C2Mailbox *head, C2Mailbox *mailbox)
{
	C2Mailbox *l;
	gchar *id;
	
	c2_return_if_fail (head, C2EDATA);
	c2_return_if_fail (mailbox, C2EDATA);

	if (!C2_MAILBOX_IS_TOPLEVEL (mailbox))
	{
		/* Get the ID of the parent */
		id = c2_mailbox_get_parent_id (mailbox->id);
		l = c2_mailbox_search_by_id (head, id);
		g_free (id);
		
		if (!l)
		{
			c2_error_set (C2EDATA);
			return;
		}
		
		if (l->child)
		{
			for (l = l->child; l->next != NULL; l = l->next);
			l->next = mailbox;
		} else
			l->child = mailbox;
	} else
	{
		for (l = head; l->next != NULL; l = l->next);
		l->next = mailbox;
	}
	gtk_signal_emit (GTK_OBJECT (mailbox_head),
						c2_mailbox_signals[CHANGED_MAILBOXES]);
}

/**
 * c2_mailbox_update
 * @mailbox: Mailbox to work on.
 * @name: New mailbox name.
 * @id: New mailbox id.
 * @type: Type of mailbox.
 * ...: Specific information about the mailbox according to the type
 * 		of mailbox.
 * 		Cronos II Mailbox's type: Null.
 * 		IMAP Mailbox's type: gchar *server, gint port, gchar *user, gchar *pass, *gchar *path.
 * 		Spool Mailbox's type: gchar *path.
 * 
 * This function will update a C2Mailbox object.
 **/
void
c2_mailbox_update (C2Mailbox *mailbox, const gchar *name, const gchar *id, C2MailboxType type, ...)
{
	va_list edata;
	
	c2_return_if_fail (mailbox, C2EDATA);

	g_free (mailbox->name);
	mailbox->name = g_strdup (name);

	switch (mailbox->type)
	{
		case C2_MAILBOX_IMAP:
			g_free (mailbox->protocol.imap.host);
			mailbox->protocol.imap.host = NULL;
			g_free (mailbox->protocol.imap.user);
			mailbox->protocol.imap.user = NULL;
			g_free (mailbox->protocol.imap.pass);
			mailbox->protocol.imap.pass = NULL;
			g_free (mailbox->protocol.imap.path);
			mailbox->protocol.imap.path = NULL;
			break;
		case C2_MAILBOX_SPOOL:
			g_free (mailbox->protocol.spool.path);
			mailbox->protocol.spool.path = NULL;
			break;
	}

	switch (type)
	{
		case C2_MAILBOX_CRONOSII:
			mailbox->type = C2_MAILBOX_CRONOSII;
			break;
		case C2_MAILBOX_IMAP:
			va_start (edata, type);
			mailbox->type = C2_MAILBOX_IMAP;
			mailbox->protocol.imap.host = g_strdup (va_arg (edata, gchar *));
			mailbox->protocol.imap.port = va_arg (edata, gint);
			mailbox->protocol.imap.user = g_strdup (va_arg (edata, gchar *));
			mailbox->protocol.imap.pass = g_strdup (va_arg (edata, gchar *));
			mailbox->protocol.imap.path = g_strdup (va_arg (edata, gchar *));
			va_end (edata);
			break;
		case C2_MAILBOX_SPOOL:
			va_start (edata, type);
			mailbox->type = C2_MAILBOX_SPOOL;
			mailbox->protocol.spool.path = g_strdup (va_arg (edata, gchar *));
			va_end (edata);
			break;
	}

	gtk_signal_emit (GTK_OBJECT (c2_mailbox_get_head ()), c2_mailbox_signals[CHANGED_MAILBOXES]);
}

/**
 * c2_mailbox_remove
 * @mailbox: Mailbox to remove.
 *
 * Removes a mailbox and all of its children.
 * 
 * TODO: Ability to delete the mailbox with ID "0".
 **/
void
c2_mailbox_remove (C2Mailbox *mailbox, gboolean archive)
{
	C2Mailbox *parent = NULL, *previous = NULL, *next;
	gchar *parent_id, *previous_id, *next_id;
	gint my_id;
	
	c2_return_if_fail (mailbox, C2EDATA);
	
	/* First get the parent/previous and next mailboxes */
	if (!(my_id = c2_mailbox_get_id (mailbox->id, -1)))
	{
		/* This mailbox has no previous mailbox in the same level */
		if (!(parent_id = c2_mailbox_get_parent_id (mailbox->id)))
			return;
		
		if (!(parent = c2_mailbox_search_by_id (c2_mailbox_get_head (), parent_id)))
			return;
		
		g_free (parent_id);
	} else
	{
		/* This mailbox has a previous mailbox in the same level */
		if (!C2_MAILBOX_IS_TOPLEVEL (mailbox))
			previous_id = g_strdup_printf ("%s-%d", c2_mailbox_get_parent_id (mailbox->id), my_id-1);
		else
			previous_id = g_strdup_printf ("%d", my_id-1);
		
		if (!(previous = c2_mailbox_search_by_id (c2_mailbox_get_head (), previous_id)))
			return;
		
		g_free (previous_id);
	}
	
	/* Here we should already have the parent or the previous mailbox */
	/* Get the next mailbox */
	next = mailbox->next;

	if (parent)
		parent->child = next;
	else
		previous->next = next;

	if (!parent && (parent = c2_mailbox_get_parent (c2_mailbox_get_head (), mailbox)));
	else
		parent = c2_mailbox_get_head ();
	
	c2_mailbox_recreate_tree_ids (parent);

	gtk_signal_emit (GTK_OBJECT (c2_mailbox_get_head ()), c2_mailbox_signals[CHANGED_MAILBOXES]);

	/* Remove the structure */
	if (archive)
		c2_db_archive (mailbox);
	else
		c2_db_remove_structure (mailbox);

	gtk_object_unref (GTK_OBJECT (mailbox));
}

static void
c2_mailbox_recreate_tree_ids (C2Mailbox *head)
{
	C2Mailbox *l;
	gint i;

	if (!head)
	{
		/* Toplevel */
		for (l = c2_mailbox_get_head (), i = 0; l != NULL; l = l->next, i++)
		{
			g_free (l->id);
			l->id = g_strdup_printf ("%d", i);
			if (l->child)
				c2_mailbox_recreate_tree_ids (l);
		}
	} else
	{
		/* Everything below the toplevel */
		for (l = head->child, i = 0; l != NULL; l = l->next, i++)
		{
			g_free (l->id);
			l->id = g_strdup_printf ("%s%d", head->id, i);
			if (l->child)
				c2_mailbox_recreate_tree_ids (l);
		}
	}
}

static C2Mailbox *
_c2_mailbox_search_by_id (C2Mailbox *head, const gchar *id, guint level)
{
	const gchar *ptr;
	C2Mailbox *l;
	gint pos = 0;
	gint top_id;
	gint ck_id;

	c2_return_val_if_fail (head, NULL, C2EDATA);
	c2_return_val_if_fail (id, NULL, C2EDATA);

	top_id = c2_mailbox_get_id (id, level);

	for (l = head; l != NULL; l = l->next)
	{
		ck_id = c2_mailbox_get_id (l->id, level);
		if (top_id == ck_id)
		{
			if (c2_mailbox_get_level (id)-level == 0)
				return l;
			else
				return _c2_mailbox_search_by_id (l->child, id, level+1);
		}
	}

	return l;
}

static gchar *
c2_mailbox_create_id_from_parent (C2Mailbox *parent)
{
	C2Mailbox *l;
	gchar *id;
	
	if (!parent)
	{
		for (l = c2_mailbox_get_head (); l->next != NULL; l = l->next);
		id = g_strdup (l->id);
		/* Add 1 to the previous ID */
		*(id+strlen (id)-1) = (*(id+strlen (id)-1)-48)+1;
	} else
	{
		if (parent->child)
		{
			for (l = parent->child; l->next != NULL; l = l->next);
			id = g_strdup_printf ("%s-%d", c2_mailbox_get_parent_id (l->id), c2_mailbox_get_id (l->id, -1)+1);
		} else
			id = g_strdup_printf ("%s-0", parent->id);
	}

	return id;
}

/**
 * c2_mailbox_get_level
 * @id: Mailbox ID.
 *
 * This function will get the level
 * of the mailbox with ID @id in the
 * mailboxes tree.
 *
 * Return Value:
 * The level of the mailbox.
 **/
gint
c2_mailbox_get_level (const gchar *id)
{
	gint i;
	const gchar *ptr;

	c2_return_val_if_fail (id, -1, C2EDATA);

	for (i = 1, ptr = id; (ptr = strstr (ptr, "-")); i++) ptr++;

	return i;
}

/**
 * c2_mailbox_get_complete_id
 * @id: ID String.
 * @number: Number of id to get.
 * 
 * This function will get certain ID by position in
 * a valid mailbox ID.
 *
 * Return Value:
 * The ID required or null if it wasn't found.
 **/
gchar *
c2_mailbox_get_complete_id (const gchar *id, guint number)
{
	gint i;
	const gchar *ptr;
	
	c2_return_val_if_fail (id, NULL, C2EDATA);

	for (ptr = id, i = 0; i < number; i++, ptr++)
	{
		if (!(ptr = strstr (ptr, "-")))
		{
			if (!number)
				g_strdup (id);
			else
				return NULL;
		}
	}

	/* This is to avoid the use of return g_strndup (id, ptr-id-1) which
	 * will be a bug when ptr is == to id
	 */
	if (ptr != id)
		ptr--;

	return g_strndup (id, ptr-id);
}

/**
 * c2_mailbox_get_id
 * @id: ID String.
 * @number: Number of id to get (-1 for last one).
 * 
 * This function will get certain ID by position in
 * a valid mailbox ID.
 * The difference between this function and c2_mailbox_get_complete_id
 * is that this function will return "70" for
 * c2_mailbox_get_id ("50-60-70-84-47", 3),
 * while c2_mailbox_get_complete_id will return
 * "50-60-70" in the same call.
 *
 * Return Value:
 * The ID required or -1 if it wasn't found.
 **/
gint
c2_mailbox_get_id (const gchar *id, gint number)
{
	gint i;
	const gchar *ptr;
	
	c2_return_val_if_fail (id, -1, C2EDATA);

	if (number < 0)
	{
		for (ptr = id+strlen (id)-1; *ptr != '\0' && *ptr != '-'; ptr--);
		if (!ptr)
			return -1;
	
		return atoi (++ptr);
	}
		
	for (ptr = id, i = 1; i < number; i++)
		if (!(ptr = strstr (ptr, "-")))
			return -1;

	return atoi (ptr);
}


/**
 * c2_mailbox_get_head
 *
 * Gets the head of the mailbox tree.
 *
 * Return Value:
 * Head of mailbox tree or NULL if the tree is empty.
 **/
C2Mailbox *
c2_mailbox_get_head (void)
{
	return mailbox_head;
}

static void
c2_mailbox_set_head (C2Mailbox *mailbox)
{
	mailbox_head = mailbox;
}

/**
 * c2_mailbox_get_by_name
 * @head: Head where to search.
 * @name: Name of mailbox to get.
 *
 * Searches recursively for a mailbox
 * with name @name.
 *
 * Return Value:
 * The requested mailbox or NULL.
 **/
C2Mailbox *
c2_mailbox_get_by_name (C2Mailbox *head, const gchar *name)
{
	C2Mailbox *l;
	
	c2_return_val_if_fail (name, NULL, C2EDATA);

	for (l = head ? head : c2_mailbox_get_head (); l != NULL; l = l->next)
	{
		if (c2_streq (l->name, name))
			return l;
		if (l->child)
		{
			C2Mailbox *s;
			if ((s = c2_mailbox_get_by_name (l->child, name)))
				return s;
		}
	}

	return NULL;
}

/**
 * c2_mailbox_load_db
 * @mailbox: C2Mailbox object to load.
 *
 * This function is the point of connection
 * with the Db module. It will load the Db
 * of the mailbox and fire the db_loaded
 * signal when done.
 **/
void
c2_mailbox_load_db (C2Mailbox *mailbox)
{
	c2_return_if_fail (mailbox, C2EDATA);

	/* Check if it is already loaded */
	if (mailbox->db)
	{
		gtk_signal_emit (GTK_OBJECT (mailbox), c2_mailbox_signals[DB_LOADED], 0);
		return;
	}

	/* We must load the db */
	if (!c2_db_load (mailbox))
	{
		gtk_signal_emit (GTK_OBJECT (mailbox), c2_mailbox_signals[DB_LOADED], -1);
		return;
	}

	gtk_signal_emit (GTK_OBJECT (mailbox), c2_mailbox_signals[DB_LOADED], 0);
}
