/*  Cronos II Mail Client /libcronosII/pop3.c
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
#include <unistd.h>
#include <string.h>
#include <config.h>

#include "error.h"
#include "net-object.h"
#include "pop3.h"
#include "utils.h"

#define DEFAULT_FLAGS C2_POP3_DONT_KEEP_COPY | C2_POP3_DONT_LOSE_PASSWORD

static void
class_init										(C2Pop3Class *klass);

static void
init											(C2Pop3 *pop3);

static void
destroy											(GtkObject *object);

static gint
welcome											(C2Pop3 *pop3);

static gint
login											(C2Pop3 *pop3);

enum
{
	STATUS,
	RETRIEVE,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static C2NetObject *parent_class = NULL;

/**
 * c2_pop3_new
 * @user: Username.
 * @pass: Password (might be NULL).
 * @host: Hostame.
 * @port: Port.
 *
 * This function will create a new C2Pop3 with
 * the data you pass it and with some default configuration.
 * 
 * Return Value:
 * The allocated C2Pop3 object or NULL if there was an error.
 **/
C2Pop3 *
c2_pop3_new (const gchar *user, const gchar *pass, const gchar *host, gint port)
{
	C2Pop3 *pop3;
	
	c2_return_val_if_fail (user || host, NULL, C2EDATA);
	
	pop3 = gtk_type_new (C2_TYPE_POP3);

	pop3->user = g_strdup (user);
	pop3->pass = g_strdup (pass);

	c2_net_object_construct (C2_NET_OBJECT (pop3), host, port);

	return pop3;
}

/**
 * c2_pop3_set_flags
 * @pop3: An allocated C2Pop3 object.
 * @flags: Flags to set in the object.
 *
 * This function will force to change the flags
 * of a C2Pop3 object.
 * Flags let the object customize according
 * to the users preferences.
 **/
void
c2_pop3_set_flags (C2Pop3 *pop3, gint flags)
{
	c2_return_if_fail (pop3, C2EDATA);

	pop3->flags = flags;
}

/**
 * c2_pop3_set_wrong_pass_cb
 * @pop3: C2Pop3 object.
 * @func: C2Pop3GetPass function.
 *
 * This function sets the function that will be called when a wrong
 * password in this C2Pop3 object is found.
 * The C2Pop3GetPass type function should return the new password
 * or NULL if it want's to cancel the fetching.
 */
void
c2_pop3_set_wrong_pass_cb (C2Pop3 *pop3, C2Pop3GetPass func)
{
	c2_return_if_fail (pop3, C2EDATA);

	pop3->wrong_pass_cb = func;
}

/**
 * c2_pop3_fetchmail
 * @pop3: Loaded C2Pop3 object.
 *
 * This function will download
 * messages from 
 **/
gint
c2_pop3_fetchmail (C2Pop3 *pop3)
{
	c2_return_val_if_fail (pop3, -1, C2EDATA);

	/* Lock the mutex */
	pthread_mutex_lock (&pop3->run_lock);

	if (c2_net_object_run (C2_NET_OBJECT (pop3)) < 0)
	{
		pthread_mutex_unlock (&pop3->run_lock);
		return -1;
	}

	if (welcome (pop3) < 0)
	{
		c2_net_object_disconnect_with_error (C2_NET_OBJECT (pop3));
		pthread_mutex_unlock (&pop3->run_lock);
		return -1;
	}

	if (login (pop3) < 0)
	{
		c2_net_object_disconnect_with_error (C2_NET_OBJECT (pop3));
		pthread_mutex_unlock (&pop3->run_lock);
		return -1;
	}

	c2_net_object_disconnect (C2_NET_OBJECT (pop3));

	/* Unlock the mutex */
	pthread_mutex_unlock (&pop3->run_lock);

	return 0;
}

static gint
welcome (C2Pop3 *pop3)
{
	gchar *string;

	if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
		return -1;

	if (c2_strnne (string, "+OK", 3))
	{
		string = strstr (string, " ");
		if (string)
			string++;

		c2_error_set_custom (string);
		return -1;
	}
	
	return 0;
}

static gint
login (C2Pop3 *pop3)
{
	gchar *string;
	gint i = 0;
	gboolean logged_in = FALSE;

	/* Username */
	if (c2_net_object_send (C2_NET_OBJECT (pop3), "USER %s\r\n", pop3->user) < 0)
		return -1;

	if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
		return -1;

	C2_DEBUG (string);

	if (c2_strnne (string, "+OK", 3))
	{
		string = strstr (string, " ");
		if (string)
			string++;

		c2_error_set_custom (string);
		return -1;
	}

	/* Password */
	do
	{
		g_free (string);
		if (c2_net_object_send (C2_NET_OBJECT (pop3), "PASS %s\r\n", pop3->pass) < 0)
			return -1;
		
		if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
			return -1;

		if (c2_strnne (string, "+OK", 3))
		{
			string = strstr (string, " ");
			if (string)
				string++;
			
			g_free (pop3->pass);

			if (pop3->wrong_pass_cb)
				pop3->pass = pop3->wrong_pass_cb (pop3, string);
		} else
			logged_in = TRUE;
	} while (i++ < 3 && !logged_in && pop3->pass);
	
	if (!logged_in)
	{
L		return -1;
	}

L	return 0;
}

GtkType
c2_pop3_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static const GtkTypeInfo info = {
			"C2Pop3",
			sizeof (C2Pop3),
			sizeof (C2Pop3Class),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (c2_net_object_get_type (), &info);
	}

	return type;
}

static void
class_init (C2Pop3Class *klass)
{
	GtkObjectClass *object_class;
	
	object_class = (GtkObjectClass *) klass;
	
	parent_class = gtk_type_class (c2_net_object_get_type ());

	signals[STATUS] =
		gtk_signal_new ("status",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2Pop3Class, status),
					gtk_marshal_NONE__INT, GTK_TYPE_NONE, 1,
					GTK_TYPE_INT);

	signals[RETRIEVE] =
		gtk_signal_new ("retrieve",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2Pop3Class, retrieve),
					gtk_marshal_NONE__INT, GTK_TYPE_NONE, 3,
					GTK_TYPE_INT, GTK_TYPE_INT, GTK_TYPE_INT);
	
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
	
	object_class->destroy = destroy;
}

static void
init (C2Pop3 *pop3)
{
	pop3->user = pop3->pass = NULL;
	pop3->flags = DEFAULT_FLAGS;
	pop3->wrong_pass_cb = NULL;
	pthread_mutex_init (&pop3->run_lock, NULL);
}

static void
destroy (GtkObject *object)
{
	C2Pop3 *pop3 = C2_POP3 (object);

	if (c2_net_object_is_offline (C2_NET_OBJECT (pop3)))
#ifdef USE_DEBUG
	{
		g_print ("A C2Pop3 object is being freed while a connection is "
				 "being used (%s)!\n", pop3->user);
#endif
		c2_net_object_disconnect (C2_NET_OBJECT (pop3));
#ifdef USE_DEBUG
	}
#endif
	g_free (pop3->user);
	g_free (pop3->pass);
	pthread_mutex_destroy (&pop3->run_lock);
}
