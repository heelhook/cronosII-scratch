DEFS=@DEFS@ -DLOCALEDIR=\"${localedir}\"
INCLUDES = -DBUILDING_C2 -I$(top_srcdir) -I$(includedir) \
	`gnome-config --cflags gnome` \
	 -DG_LOG_DOMAIN=\"CronosII\" -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	 -I../intl -I$(top_srcdir)/intl $(GNOME_INCLUDEDIR) -DDATADIR=\"${datadir}\" \
	 -DPKGDATADIR=\"${pkgdatadir}\" -DGLADEDIR=\"${pkgdatadir}/ui\" $(GCONFCFLAGS) \
	 $(CFLAGS) $(GLIB_CFLAGS) $(GNOME_PRINT_CFLAGS) $(GNORBA_CFLAGS) $(GTK_CFLAGS) \
	 $(LIBGLADE_CFLAGS) $(ORBIT_CFLAGS)

if USE_UI_TEXT_EDITOR
UI_TEXT_EDITOR=ui-text-editor
else
UI_TEXT_EDITOR=
endif

bin_PROGRAMS = cronosII $(UI_TEXT_EDITOR)

cronosII_SOURCES = \
					dialogs.h			\
	install.c							\
	main.c				main.h				\
	object-ui-data.c		object-ui-data.h		\
					preferences.h			\
	widget-HTML-gtkhtml.c		widget-HTML-gtkhtml.h		\
	widget-HTML-gtkxmhtml.c		widget-HTML-gtkxmhtml.h		\
	widget-HTML-text.c			widget-HTML-text.h	\
	widget-HTML.c				widget-HTML.h			\
	widget-application-utils.c	widget-application-utils.h	\
	widget-application.c		widget-application.h		\
	widget-composer.c			widget-composer.h			\
	widget-dialog-preferences.c	widget-dialog-preferences.h	\
	widget-dialog.c				widget-dialog.h				\
	widget-editor.c				widget-editor.h				\
	widget-index.c				widget-index.h				\
	widget-mail.c				widget-mail.h				\
	widget-mailbox-list.c		widget-mailbox-list.h		\
	widget-network-traffic.c	widget-network-traffic.h	\
	widget-part.c				widget-part.h				\
	widget-select-list.c		widget-select-list.h		\
	widget-sidebar.c			widget-sidebar.h			\
	widget-toolbar.c			widget-toolbar.h			\
	widget-transfer-item.c		widget-transfer-item.h		\
	widget-transfer-list.c		widget-transfer-list.h		\
	widget-window-mail.c		widget-window-mail.h		\
	widget-window-main.c		widget-window-main.h		\
	widget-window.c				widget-window.h

cronosII_LDADD = \
	./$(top_builddir)/libcronosII/libcronosII.a \
	$(LIBS) $(INTLLIBS) $(LIBGLADE_LIBS) $(HTMLLIBS) $(GCONFLIBS)

cronosII_LDFLAGS = \
	./././$(top_builddir)/libcronosII/libcronosII.a \
	$(LIBS) $(INTLLIBS) $(GNOME_LIBDIR) $(GNOMEUI_LIBDIR) $(PTHREAD_LIB) \
	$(GNOME_PRINT_LIBS) $(GLIB_LIBS) $(LIBGLADE_LIBS) $(HTMLLIBS) $(GCONFLIBS) -pthread `gnome-config --libs gnome gnomeui libglade`

ui_text_editor_SOURCES = \
	object-ui-data.c			object-ui-data.h			\
	ui-text-editor.c

ui_text_editor_LDADD = \
	./$(top_builddir)/libcronosII/libcronosII.a \
	$(LIBS) $(INTLLIBS) $(LIBGLADE_LIBS) $(HTMLLIBS) $(GCONFLIBS)

ui_text_editor_LDFLAGS = \
	./././$(top_builddir)/libcronosII/libcronosII.a \
	$(LIBS) $(INTLLIBS) $(GNOME_LIBDIR) $(GNOMEUI_LIBDIR) $(PTHREAD_LIB) \
	$(GNOME_PRINT_LIBS) $(GLIB_LIBS) $(LIBGLADE_LIBS) $(HTMLLIBS) $(GCONFLIBS) -pthread  `gnome-config --libs gnome gnomeui libglade`

EXTRA_DIST = \
	README.FAQ			\
	preferences-trans.h	\
	trans.h
