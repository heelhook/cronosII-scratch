/*  Cronos II - A GNOME mail client
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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef __WIDGET_COMPOSER_H__
#define __WIDGET_COMPOSER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gnome.h>
#include <glade/glade.h>

#include <libcronosII/message.h>

#define C2_COMPOSER(obj)					(GTK_CHECK_CAST (obj, c2_composer_get_type (), C2Composer))
#define C2_COMPOSER_CLASS(klass)			(GTK_CHECK_CLASS_CAST (klass, c2_composer_get_type (), C2ComposerClass))

typedef struct _C2Composer C2Composer;
typedef struct _C2ComposerClass C2ComposerClass;

struct _C2Composer
{
	GtkWidget widget;
	
	GladeXML *xml;

	/* Save */
	gchar *file;

	/* Draft recovery */
	gint draft_id;
 
	/* Undo and Redo */
	GList *operations; /* History of operations */
	GList *operations_ptr;
};

struct _C2ComposerClass
{
	GtkWidgetClass parent_class;

	void (*changed_title) (C2Composer *composer, const gchar *title);

	void (*send_now) (C2Composer *composer);
	void (*send_later) (C2Composer *composer);
	
	void (*save_draft) (C2Composer *composer, gint draft_id);
	
	void (*close) (C2Composer *composer);
};

GtkType
c2_composer_get_type						(void);

GtkWidget *
c2_composer_new								(GtkToolbarStyle style);

void
c2_composer_construct						(C2Composer *composer, GtkToolbarStyle style);

GtkWidget *
c2_composer_get_window						(C2Composer *composer);

C2Message *
c2_composer_get_message						(C2Composer *composer);

void
c2_composer_set_message						(C2Composer *composer, C2Message *message);

void
c2_composer_close							(C2Composer *composer);

#ifdef __cplusplus
}
#endif

#endif