/*
 * Copyright 2014 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "utils/config.h"
#include "utils/errors.h"
#include "utils/file.h"
#include "utils/inet.h"
#include "netsurf/bitmap.h"
#include "content/hlcache.h"
#include "content/backing_store.h"

#include "desktop/save_pdf.h"
#include "desktop/download.h"
#include "desktop/searchweb.h"
#include "netsurf/download.h"
#include "netsurf/fetch.h"
#include "netsurf/misc.h"
#include "netsurf/window.h"
#include "netsurf/core_window.h"
#include "netsurf/search.h"
#include "netsurf/clipboard.h"
#include "netsurf/utf8.h"
#include "netsurf/layout.h"
#include "netsurf/netsurf.h"

/**
 * The global interface table.
 */
struct netsurf_table *guit = NULL;


static void gui_default_window_set_title(struct gui_window *g, const char *title)
{
}

static nserror gui_default_window_set_url(struct gui_window *g, struct nsurl *url)
{
	return NSERROR_OK;
}

static bool gui_default_window_drag_start(struct gui_window *g,
					  gui_drag_type type,
					  const struct rect *rect)
{
	return true;
}

static nserror gui_default_window_save_link(struct gui_window *g,
					 nsurl *url,
					 const char *title)
{
	return NSERROR_OK;
}

static void gui_default_window_set_icon(struct gui_window *g,
					hlcache_handle *icon)
{
}

static void gui_default_window_set_pointer(struct gui_window *g,
					   gui_pointer_shape shape)
{
}

static void gui_default_window_set_status(struct gui_window *g,
					  const char *text)
{
}

static void gui_default_window_place_caret(struct gui_window *g,
					   int x, int y, int height,
					   const struct rect *clip)
{
}

static void gui_default_window_create_form_select_menu(struct gui_window *g,
						struct form_control *control)
{
}

static void gui_default_window_file_gadget_open(struct gui_window *g,
						hlcache_handle *hl,
						struct form_control *gadget)
{
}

static void gui_default_window_drag_save_object(struct gui_window *g,
						hlcache_handle *c,
						gui_save_type type)
{
}

static void gui_default_window_drag_save_selection(struct gui_window *g,
						   const char *selection)
{
}


static void
gui_default_console_log(struct gui_window *gw,
			browser_window_console_source src,
			const char *msg,
			size_t msglen,
			browser_window_console_flags flags)
{
}


/** verify window table is valid */
static nserror verify_window_register(struct gui_window_table *gwt)
{
	/* check table is present */
	if (gwt == NULL) {
		return NSERROR_BAD_PARAMETER;
	}

	/* check the mandantory fields are set */
	if (gwt->create == NULL) {
		return NSERROR_BAD_PARAMETER;
	}
	if (gwt->destroy == NULL) {
		return NSERROR_BAD_PARAMETER;
	}
	if (gwt->invalidate == NULL) {
		return NSERROR_BAD_PARAMETER;
	}
	if (gwt->get_scroll == NULL) {
		return NSERROR_BAD_PARAMETER;
	}
	if (gwt->set_scroll == NULL) {
		return NSERROR_BAD_PARAMETER;
	}
	if (gwt->get_dimensions == NULL) {
		return NSERROR_BAD_PARAMETER;
	}
	if (gwt->event == NULL) {
		return NSERROR_BAD_PARAMETER;
	}


	/* fill in the optional entries with defaults */
	if (gwt->set_title == NULL) {
		gwt->set_title = gui_default_window_set_title;
	}
	if (gwt->set_url == NULL) {
		gwt->set_url = gui_default_window_set_url;
	}
	if (gwt->set_icon == NULL) {
		gwt->set_icon = gui_default_window_set_icon;
	}
	if (gwt->set_status == NULL) {
		gwt->set_status = gui_default_window_set_status;
	}
	if (gwt->set_pointer == NULL) {
		gwt->set_pointer = gui_default_window_set_pointer;
	}
	if (gwt->place_caret == NULL) {
		gwt->place_caret = gui_default_window_place_caret;
	}
	if (gwt->drag_start == NULL) {
		gwt->drag_start = gui_default_window_drag_start;
	}
	if (gwt->save_link == NULL) {
		gwt->save_link = gui_default_window_save_link;
	}
	if (gwt->create_form_select_menu == NULL) {
		gwt->create_form_select_menu =
				gui_default_window_create_form_select_menu;
	}
	if (gwt->file_gadget_open == NULL) {
		gwt->file_gadget_open = gui_default_window_file_gadget_open;
	}
	if (gwt->drag_save_object == NULL) {
		gwt->drag_save_object = gui_default_window_drag_save_object;
	}
	if (gwt->drag_save_selection == NULL) {
		gwt->drag_save_selection = gui_default_window_drag_save_selection;
	}
	if (gwt->console_log == NULL) {
		gwt->console_log = gui_default_console_log;
	}

	return NSERROR_OK;
}



/**
 * verify layout table is valid
 *
 * \param glt The layout table to verify.
 * \return NSERROR_OK if the table is valid else NSERROR_BAD_PARAMETER.
 */
static nserror verify_layout_register(struct gui_layout_table *glt)
{
	/* check table is present */
	if (glt == NULL) {
		return NSERROR_BAD_PARAMETER;
	}

	/* check the mandantory fields are set */
	if (glt->width == NULL) {
		return NSERROR_BAD_PARAMETER;
	}

	if (glt->position == NULL) {
		return NSERROR_BAD_PARAMETER;
	}

	if (glt->split == NULL) {
		return NSERROR_BAD_PARAMETER;
	}

	return NSERROR_OK;
}

static void gui_default_quit(void)
{
}


static nserror gui_default_launch_url(struct nsurl *url)
{
	return NSERROR_NO_FETCH_HANDLER;
}


static nserror gui_default_401login_open(
	nsurl *url, const char *realm,
	const char *username, const char *password,
	nserror (*cb)(nsurl *url, const char * realm,
		      const char *username,
		      const char *password,
		      void *pw),
	void *cbpw)
{
	return NSERROR_NOT_IMPLEMENTED;
}

static void
gui_default_pdf_password(char **owner_pass, char **user_pass, char *path)
{
	*owner_pass = NULL;
	save_pdf(path);
}

static nserror
gui_default_present_cookies(const char *search_term)
{
	return NSERROR_NOT_IMPLEMENTED;
}

/** verify misc table is valid */
static nserror verify_misc_register(struct gui_misc_table *gmt)
{
	/* check table is present */
	if (gmt == NULL) {
		return NSERROR_BAD_PARAMETER;
	}

	/* check the mandantory fields are set */
	if (gmt->schedule == NULL) {
		return NSERROR_BAD_PARAMETER;
	}

	/* fill in the optional entries with defaults */
	if (gmt->quit == NULL) {
		gmt->quit = gui_default_quit;
	}
	if (gmt->launch_url == NULL) {
		gmt->launch_url = gui_default_launch_url;
	}
	if (gmt->login == NULL) {
		gmt->login = gui_default_401login_open;
	}
	if (gmt->pdf_password == NULL) {
		gmt->pdf_password = gui_default_pdf_password;
	}
	if (gmt->present_cookies == NULL) {
		gmt->present_cookies = gui_default_present_cookies;
	}
	return NSERROR_OK;
}


/* exported interface documented in netsurf/netsurf.h */
nserror netsurf_register(struct netsurf_table *gt)
{
	nserror err;

	/* ensure not already initialised */
	if (guit != NULL) {
		return NSERROR_INIT_FAILED;
	}

	/* check table is present */
	if (gt == NULL) {
		return NSERROR_BAD_PARAMETER;
	}

	/* mandantory tables */

	/* miscellaneous table */
	err = verify_misc_register(gt->misc);
	if (err != NSERROR_OK) {
		return err;
	}

	/* window table */
	err = verify_window_register(gt->window);
	if (err != NSERROR_OK) {
		return err;
	}

	/* layout table */
	err = verify_layout_register(gt->layout);
	if (err != NSERROR_OK) {
		return err;
	}

	/* optional tables */



	/* search table */
	guit = gt;

	return NSERROR_OK;
}
