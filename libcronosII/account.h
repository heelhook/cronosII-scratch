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
#ifndef __LIBCRONOSII_ACCOUNT_H__
#define __LIBCRONOSII_ACCOUNT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <gtk/gtk.h>

#define C2_TYPE_ACCOUNT							(c2_account_get_type ())
#define C2_ACCOUNT(obj)							(GTK_CHECK_CAST (obj, C2_TYPE_ACCOUNT, C2Account))
#define C2_ACCOUNT_CLASS(klass)					(GTK_CHECK_CLASS (klass, C2_TYPE_ACCOUNT, C2Account))
#define C2_IS_ACCOUNT(obj)						(GTK_CHECK_TYPE (obj, C2_TYPE_ACCOUNT))
#define C2_IS_ACCOUNT_CLASS(klass)				(GTK_CHECK_CLASS_TYPE (klass, C2_TYPE_ACCOUNT))

#define c2_account_next(obj)					(obj ? obj->next : NULL)

typedef struct _C2Account C2Account;
typedef struct _C2AccountClass C2AccountClass;
typedef enum _C2AccountType C2AccountType;

#ifdef HAVE_CONFIG_H
#	include "mailbox.h"
#	include "pop3.h"
#	include "smtp.h"
#else
#	include <cronosII.h>
#endif

enum _C2AccountType
{
	C2_ACCOUNT_POP3,
	C2_ACCOUNT_IMAP
};

struct _C2Account
{
	GtkObject object;

	gchar *name;
	
	gchar *full_name;
	gchar *organization;
	
	gchar *email;
	gchar *reply_to;

	C2AccountType type;
	
	union
	{
		C2POP3 *pop3;
//		C2IMAP *imap;
	} protocol;

	C2SMTP *smtp;

	struct
	{
		gboolean active;
	} options;

	struct
	{
		gchar *plain;
		gchar *html;
	} signature;

	struct _C2Account *next;
};

struct _C2AccountClass
{
	GtkObjectClass parent_class;
};

GtkType
c2_account_get_type									(void);

C2Account *
c2_account_new										(const gchar *name, const gchar *full_name,
													 const gchar *organization, const gchar *email,
													 const gchar *reply_to, gboolean active,
													 const gchar *signature_plain,
													 const gchar *signature_html,
													 C2AccountType account_type, C2SMTPType smtp_type,
													 ...);

void
c2_account_free										(C2Account *account);

void
c2_account_free_all									(C2Account *head);

C2Account *
c2_account_copy										(C2Account *account);

C2Account *
c2_account_append									(C2Account *head, C2Account *obj);

C2Account *
c2_account_last										(C2Account *head);

C2Account *
c2_account_get_by_name								(C2Account *head, const gchar *name);

gint
c2_account_check									(const C2Account *account);

#ifdef __cplusplus
}
#endif

#endif
