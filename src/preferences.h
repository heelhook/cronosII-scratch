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
#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

#ifdef __cplusplus
extern "C" {
#endif

#define GENERAL "General"
#define INTERFACE "Interface"
#define ADVANCED "Advanced"
#define EXTRA "Extra"

#define OPTIONS "Options"
#define PATHS "Paths"
#define FONTS "Fonts"
#define HTML "HTML"
#define COMPOSER "Composer"
#define MISC "Misc"

#define c2_preferences_get_general_options_start_check() \
	gnome_config_get_bool_with_default ("/"PACKAGE"/"GENERAL"-"OPTIONS"/start_check=true", NULL)
#define c2_preferences_set_general_options_start_check(val) \
	gnome_config_set_bool ("/"PACKAGE"/"GENERAL"-"OPTIONS"/start_check", val)
	
#define c2_preferences_get_general_options_start_load() \
	gnome_config_get_bool_with_default ("/"PACKAGE"/"GENERAL"-"OPTIONS"/start_load=true", NULL)
#define c2_preferences_set_general_options_start_load(val) \
	gnome_config_set_bool ("/"PACKAGE"/"GENERAL"-"OPTIONS"/start_load", val)
	
#define c2_preferences_get_general_options_exit_expunge() \
	gnome_config_get_bool_with_default ("/"PACKAGE"/"GENERAL"-"OPTIONS"/exit_expunge=false", NULL)
#define c2_preferences_set_general_options_exit_expunge(val) \
	gnome_config_set_bool ("/"PACKAGE"/"GENERAL"-"OPTIONS"/exit_expunge", val)
	
#define c2_preferences_get_general_options_timeout_mark() \
	gnome_config_get_float_with_default ("/"PACKAGE"/"GENERAL"-"OPTIONS"/timeout_mark=1.5", NULL)
#define c2_preferences_set_general_options_timeout_mark(val) \
	gnome_config_set_float ("/"PACKAGE"/"GENERAL"-"OPTIONS"/timeout_mark", val)
	
#define c2_preferences_get_general_options_timeout_check() \
	gnome_config_get_int_with_default ("/"PACKAGE"/"GENERAL"-"OPTIONS"/timeout_check=20", NULL)
#define c2_preferences_set_general_options_timeout_check(val) \
	gnome_config_set_int ("/"PACKAGE"/"GENERAL"-"OPTIONS"/timeout_check", val)
	
#define c2_preferences_get_general_options_incoming_warn() \
	gnome_config_get_bool_with_default ("/"PACKAGE"/"GENERAL"-"OPTIONS"/incoming_warn=false", NULL)
#define c2_preferences_set_general_options_incoming_warn(val) \
	gnome_config_set_bool ("/"PACKAGE"/"GENERAL"-"OPTIONS"/incoming_warn", val)
	
#define c2_preferences_get_general_options_outgoing_sent_items() \
	gnome_config_get_bool_with_default ("/"PACKAGE"/"GENERAL"-"OPTIONS"/outbox_sent_items=true", NULL)
#define c2_preferences_set_general_options_outgoing_sent_items(val) \
	gnome_config_set_bool ("/"PACKAGE"/"GENERAL"-"OPTIONS"/outbox_sent_items", val)
	
#define c2_preferences_get_general_options_delete_use_trash()	\
	gnome_config_get_bool_with_default ("/"PACKAGE"/"GENERAL"-"OPTIONS"/delete_use_trash=true", NULL)
#define c2_preferences_set_general_options_delete_use_trash(val)	\
	gnome_config_set_bool ("/"PACKAGE"/"GENERAL"-"OPTIONS"/delete_use_trash", val)
	
#define c2_preferences_get_general_options_delete_confirmation() \
	gnome_config_get_bool_with_default ("/"PACKAGE"/"GENERAL"-"OPTIONS"/delete_confirmation=true", NULL)
#define c2_preferences_set_general_options_delete_confirmation(val) \
	gnome_config_set_bool ("/"PACKAGE"/"GENERAL"-"OPTIONS"/delete_confirmation", val)
	
#define c2_preferences_get_general_options_delete_archive() \
	gnome_config_get_bool_with_default ("/"PACKAGE"/"GENERAL"-"OPTIONS"/delete_archive=true", NULL)
#define c2_preferences_set_general_options_delete_archive(val) \
	gnome_config_set_bool ("/"PACKAGE"/"GENERAL"-"OPTIONS"/delete_archive", val)
	


#define c2_preferences_get_general_paths_save(var) \
	{ \
		gchar *__default__; \
 		 \
		__default__ = g_strdup_printf ("/"PACKAGE"/"GENERAL"-"PATHS"/save=%s/", \
										g_get_home_dir ()); \
		 \
		var = gnome_config_get_string_with_default (__default__, NULL); \
		g_free (__default__); \
	}
#define c2_preferences_set_general_paths_save(val) \
	gnome_config_set_string ("/"PACKAGE"/"GENERAL"-"PATHS"/save", val)
	
#define c2_preferences_get_general_paths_get(var) \
	{ \
		gchar *__default__; \
 		 \
		__default__ = g_strdup_printf ("/"PACKAGE"/"GENERAL"-"PATHS"/get=%s/", \
										g_get_home_dir ()); \
		 \
		var = gnome_config_get_string_with_default (__default__, NULL); \
		g_free (__default__); \
	}
#define c2_preferences_set_general_paths_get(val) \
	gnome_config_set_string ("/"PACKAGE"/"GENERAL"-"PATHS"/get", val)
	
#define c2_preferences_get_general_paths_smart() \
	gnome_config_get_bool_with_default ("/"PACKAGE"/"GENERAL"-"PATHS"/smart=true", NULL)
#define c2_preferences_set_general_paths_smart(val) \
	gnome_config_set_bool ("/"PACKAGE"/"GENERAL"-"PATHS"/smart", val)
	

#define c2_preferences_get_interface_fonts_readed_mails() \
	gnome_config_get_string_with_default ("/"PACKAGE"/"INTERFACE"-"FONTS"/readed_mails=" \
										  "-adobe-helvetica-medium-r-normal-*-*-120-*-*-p-*-iso8859-1", NULL)
#define c2_preferences_set_interface_fonts_readed_mails(val) \
	gnome_config_set_string ("/"PACKAGE"/"INTERFACE"-"FONTS"/readed_mails", val)
	
#define c2_preferences_get_interface_fonts_unreaded_mails() \
	gnome_config_get_string_with_default ("/"PACKAGE"/"INTERFACE"-"FONTS"/unreaded_mails=" \
										  "-adobe-helvetica-bold-r-normal-*-*-120-*-*-p-*-iso8859-1", NULL)
#define c2_preferences_set_interface_fonts_unreaded_mails(val) \
	gnome_config_set_string ("/"PACKAGE"/"INTERFACE"-"FONTS"/unreaded_mails", val)
	
#define c2_preferences_get_interface_fonts_unreaded_mailbox() \
	gnome_config_get_string_with_default ("/"PACKAGE"/"INTERFACE"-"FONTS"/unreaded_mailbox=" \
										  "-adobe-helvetica-bold-r-normal-*-*-120-*-*-p-*-iso8859-1", NULL)
#define c2_preferences_set_interface_fonts_unreaded_mailbox(val) \
	gnome_config_set_string ("/"PACKAGE"/"INTERFACE"-"FONTS"/unreaded_mailbox", val)
	
#define c2_preferences_get_interface_fonts_message_body() \
	gnome_config_get_string_with_default ("/"PACKAGE"/"INTERFACE"-"FONTS"/message_body=Helvetica", NULL)
#define c2_preferences_set_interface_fonts_message_body(val) \
	gnome_config_set_string ("/"PACKAGE"/"INTERFACE"-"FONTS"/message_body", val)
	


#define c2_preferences_get_interface_html_image() \
	gnome_config_get_string_with_default ("/"PACKAGE"/"INTERFACE"-"HTML"/image=full_download", NULL)
#define c2_preferences_set_interface_html_image(val) \
	gnome_config_set_string ("/"PACKAGE"/"INTERFACE"-"HTML"/image", val)
	
#define c2_preferences_get_interface_html_image_confirmation() \
	gnome_config_get_bool_with_default ("/"PACKAGE"/"INTERFACE"-"HTML"/image_confirmation=false", NULL)
#define c2_preferences_set_interface_html_image_confirmation(val) \
	gnome_config_set_bool ("/"PACKAGE"/"INTERFACE"-"HTML"/image_confirmation", val)
	
#define c2_preferences_get_interface_html_links() \
	gnome_config_get_string_with_default ("/"PACKAGE"/"INTERFACE"-"HTML"/links=default", NULL)
#define c2_preferences_set_interface_html_links(val) \
	gnome_config_set_string ("/"PACKAGE"/"INTERFACE"-"HTML"/links", val)
	
#define c2_preferences_get_interface_html_links_confirmation() \
	gnome_config_get_bool_with_default ("/"PACKAGE"/"INTERFACE"-"HTML"/links_confirmation=false", NULL)
#define c2_preferences_set_interface_html_links_confirmation(val) \
	gnome_config_set_bool ("/"PACKAGE"/"INTERFACE"-"HTML"/links_confirmation=false", val)
	
#define c2_preferences_get_interface_html_interpret_text_plain() \
	gnome_config_get_bool_with_default ("/"PACKAGE"/"INTERFACE"-"HTML"/interpret_text_plain=true", NULL)
#define c2_preferences_set_interface_html_interpret_text_plain(val) \
	gnome_config_set_bool ("/"PACKAGE"/"INTERFACE"-"HTML"/interpret_text_plain", val)
	


#define c2_preferences_get_interface_composer_quote_character() \
	gnome_config_get_string_with_default ("/"PACKAGE"/"INTERFACE"-"COMPOSER"/quote_character=> ", NULL)
#define c2_preferences_set_interface_composer_quote_character(val) \
	gnome_config_set_string ("/"PACKAGE"/"INTERFACE"-"COMPOSER"/quote_character", val)
	
#define c2_preferences_get_interface_composer_quote_color(r,g,b) \
	r = gnome_config_get_int_with_default ("/"PACKAGE"/"INTERFACE"-"COMPOSER"/quote_color_red=55535", NULL); \
	g = gnome_config_get_int_with_default ("/"PACKAGE"/"INTERFACE"-"COMPOSER"/quote_color_green=55535", NULL);\
	b = gnome_config_get_int_with_default ("/"PACKAGE"/"INTERFACE"-"COMPOSER"/quote_color_blue=55535", NULL)
#define c2_preferences_set_interface_composer_quote_color(r,g,b) \
	gnome_config_set_int ("/"PACKAGE"/"INTERFACE"-"COMPOSER"/quote_color_red", r); \
	gnome_config_set_int ("/"PACKAGE"/"INTERFACE"-"COMPOSER"/quote_color_green", g); \
	gnome_config_set_int ("/"PACKAGE"/"INTERFACE"-"COMPOSER"/quote_color_blue", b)
	
#define c2_preferences_get_interface_composer_editor() \
	gnome_config_get_string_with_default ("/"PACKAGE"/"INTERFACE"-"COMPOSER"/editor=internal", NULL)
#define c2_preferences_set_interface_composer_editor(val) \
	gnome_config_set_string ("/"PACKAGE"/"INTERFACE"-"COMPOSER"/editor", val)
	
#define c2_preferences_get_interface_composer_editor_external_cmnd() \
	gnome_config_get_string_with_default ("/"PACKAGE"/"INTERFACE"-"COMPOSER"/" \
										  "editor_external_cmnd=gnome-edit", NULL)
#define c2_preferences_set_interface_composer_editor_external_cmnd(val) \
	gnome_config_set_string ("/"PACKAGE"/"INTERFACE"-"COMPOSER"/editor_external_cmnd", val)



#define c2_preferences_get_interface_misc_title() \
	gnome_config_get_string_with_default ("/"PACKAGE"/"INTERFACE"-"MISC"/" \
										  "title=%a v.%v - %M (%m messages, %n new)", NULL)
#define c2_preferences_set_interface_misc_title(val) \
	gnome_config_set_string ("/"PACKAGE"/"INTERFACE"-"MISC"/title", val)
	
#define c2_preferences_get_interface_misc_date() \
	gnome_config_get_string_with_default ("/"PACKAGE"/"INTERFACE"-"MISC"/" \
										  "date=%d.%m.%Y %H:%M %z", NULL)
#define c2_preferences_set_interface_misc_date(val) \
	gnome_config_set_string ("/"PACKAGE"/"INTERFACE"-"MISC"/date", val)
	
#if defined (USE_GTKHTML) || defined (USE_GTKXMHTML)
#define c2_preferences_get_interface_misc_attachments_default() \
	gnome_config_get_string_with_default ("/"PACKAGE"/"INTERFACE"-"MISC"/" \
										  "attachments_default=text/html", NULL)
#else
#define c2_preferences_get_interface_misc_attachments_default() \
	gnome_config_get_string_with_default ("/"PACKAGE"/"INTERFACE"-"MISC"/" \
										  "attachments_default=text/plain", NULL)
#endif
#define c2_preferences_set_interface_misc_attachments_default(val) \
	gnome_config_set_string ("/"PACKAGE"/"INTERFACE"-"MISC"/attachments_default", val)
	


#define c2_preferences_get_advanced_misc_proxy_http() \
	gnome_config_get_bool_with_default ("/"PACKAGE"/"ADVANCED"-"MISC"/http=false", NULL)
#define c2_preferences_set_advanced_misc_proxy_http(val) \
	gnome_config_set_bool ("/"PACKAGE"/"ADVANCED"-"MISC"/http", val)
	
#define c2_preferences_get_advanced_misc_proxy_http_addr() \
	gnome_config_get_string_with_default ("/"PACKAGE"/"ADVANCED"-"MISC"/http_addr=", NULL)
#define c2_preferences_set_advanced_misc_proxy_http_addr(val) \
	gnome_config_set_string ("/"PACKAGE"/"ADVANCED"-"MISC"/http_addr", val)
	
#define c2_preferences_get_advanced_misc_proxy_http_port() \
	gnome_config_get_int_with_default ("/"PACKAGE"/"ADVANCED"-"MISC"/http_port=8080", NULL)
#define c2_preferences_set_advanced_misc_proxy_http_port(val) \
	gnome_config_set_int ("/"PACKAGE"/"ADVANCED"-"MISC"/http_port", val)
	
#define c2_preferences_get_advanced_misc_proxy_ftp() \
	gnome_config_get_bool_with_default ("/"PACKAGE"/"ADVANCED"-"MISC"/ftp=false", NULL)
#define c2_preferences_set_advanced_misc_proxy_ftp(val) \
	gnome_config_set_bool ("/"PACKAGE"/"ADVANCED"-"MISC"/ftp", val)
	
#define c2_preferences_get_advanced_misc_proxy_ftp_addr() \
	gnome_config_get_string_with_default ("/"PACKAGE"/"ADVANCED"-"MISC"/ftp_addr=", NULL)
#define c2_preferences_set_advanced_misc_proxy_ftp_addr(val) \
	gnome_config_set_string ("/"PACKAGE"/"ADVANCED"-"MISC"/ftp_addr", val)
	
#define c2_preferences_get_advanced_misc_proxy_ftp_port() \
	gnome_config_get_int_with_default ("/"PACKAGE"/"ADVANCED"-"MISC"/ftp_port=8080", NULL)
#define c2_preferences_set_advanced_misc_proxy_ftp_port(val) \
	gnome_config_set_int ("/"PACKAGE"/"ADVANCED"-"MISC"/ftp_port", val)


	
#define c2_preferences_get_extra_release_information_show() \
	gnome_config_get_bool_with_default ("/"PACKAGE"/"EXTRA"-release_information/show=true", NULL)
#define c2_preferences_set_extra_release_information_show(val) \
	gnome_config_set_bool ("/"PACKAGE"/"EXTRA"-release_information/show", val)
	


#define c2_preferences_commit() gnome_config_sync ()

#ifdef __cplusplus
}
#endif

#endif
