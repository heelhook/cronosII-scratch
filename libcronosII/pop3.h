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
#ifndef __LIBCRONOSII_POP3_H__
#define __LIBCRONOSII_POP3_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <pthread.h>

#define C2_TYPE_POP3						(c2_pop3_get_type ())
#define C2_POP3(obj)						(GTK_CHECK_CAST (obj, C2_TYPE_POP3, C2POP3))
#define C2_POP3_CLASS(klass)				(GTK_CHECK_CLASS (klass, C2_TYPE_POP3, C2POP3))
#define C2_IS_POP3(obj)						(GTK_CHECK_TYPE (obj, C2_TYPE_POP3))
#define C2_IS_POP3_CLASS(klass)				(GTK_CHECK_CLASS_TYPE (klass, C2_TYPE_POP3))

#define C2_POP3_GET_PASS_FUNC(x)			((C2Pop3GetPass)x)

typedef struct _C2POP3 C2POP3;
typedef struct _C2POP3Class C2POP3Class;
typedef enum _C2POP3AuthenticationMethod C2POP3AuthenticationMethod;
typedef enum _C2POP3Flags C2POP3Flags;

typedef gchar* (*C2POP3GetPass) 			(C2POP3 *pop3, const gchar *error);

#ifdef BUILDING_C2
#	include <config.h>
#	include "account.h"
#	include "net-object.h"
#else
#	include <cronosII.h>
#endif

enum _C2POP3AuthenticationMethod
{
	C2_POP3_AUTHENTICATION_PASSWORD,
	C2_POP3_AUTHENTICATION_APOP
};

enum _C2POP3Flags
{
	C2_POP3_DO_KEEP_COPY			= 1 << 1,	/* Will get a mail and leave a copy on server */
	C2_POP3_DONT_KEEP_COPY			= 0 << 1,	/* Will get a mail and delete it on server */
	C2_POP3_DO_LOSE_PASSWORD		= 1 << 2,	/* Will delete the password once it sended it correctly */
	C2_POP3_DONT_LOSE_PASSWORD		= 0 << 2,	/* Will keep the password unless its wrong */
};

struct _C2POP3
{
	C2NetObject object;
	
	gchar *user;
	gchar *pass;
	gchar *logintoken;
	gint port;

	gint flags;

	C2POP3AuthenticationMethod auth_method;
	C2POP3GetPass wrong_pass_cb;

	pthread_mutex_t run_lock;
};

struct _C2POP3Class
{
	C2NetObjectClass parent_class;

	void (*status) (C2POP3 *pop3, gint mails);
	void (*retrieve) (C2POP3 *pop3, gint16 nth, gint32 received, gint32 total);
};

#ifdef USE_SSL
#define c2_pop3_enable_ssl(pop3)			c2_net_object_append_flags(C2_NET_OBJECT (pop3), C2_NET_OBJECT_SSL)
#endif

GtkType
c2_pop3_get_type								(void);

C2POP3 *
c2_pop3_new										(const gchar *user, const gchar *pass,
												 const gchar *host, gint port);

void
c2_pop3_set_flags								(C2POP3 *pop3, gint flags);

void
c2_pop3_set_wrong_pass_cb						(C2POP3 *pop3, C2POP3GetPass func);

gint
c2_pop3_fetchmail								(C2Account *account);


#ifdef __cplusplus
}
#endif

#endif
