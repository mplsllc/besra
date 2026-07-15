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

/**
 * \file
 *
 * Interface to platform-specific miscellaneous browser operation table.
 */

#ifndef NETSURF_MISC_H_
#define NETSURF_MISC_H_

struct form_control;
struct gui_window;
struct cert_chain;
struct nsurl;

/*
 * Miscellaneous browser operations implemented by the frontend.
 */

/**
 * Schedule a callback.
 *
 * \param t interval before the callback should be made in ms or
 *          negative value to remove any existing callback.
 * \param callback callback function
 * \param p user parameter passed to callback function
 * \return NSERROR_OK on sucess or appropriate error on faliure
 *
 * The callback function will be called as soon as possible after the timeout
 * has elapsed. Additional calls with the same callback and user parameter
 * will reset the callback time to the newly specified value.
 */
nserror gui_misc_schedule(int t, void (*callback)(void *p), void *p);

/**
 * called to allow the gui to cleanup.
 */
void gui_misc_quit(void);

/**
 * core has no fetcher for url
 */
nserror gui_misc_launch_url(struct nsurl *url);

/**
 * Retrieve username/password for a given url+realm if there is one
 * stored in a frontend-specific way (e.g. gnome-keyring)
 *
 * To respond, call the callback with the url, realm, username, and password.
 * Pass "" if the empty string is required.
 *
 * \param url       The URL being verified.
 * \param realm     The authorization realm.
 * \param username  Any current username (or empty string).
 * \param password  Any current password (or empty string).
 * \param cb        Callback upon user decision.
 * \param cbpw      Context pointer passed to cb
 * \return NSERROR_OK on sucess else error and cb never called
 */
nserror gui_misc_login(struct nsurl *url, const char *realm,
		 const char *username, const char *password,
		 nserror (*cb)(struct nsurl *url,
			       const char *realm,
			       const char *username,
			       const char *password,
			       void *pw),
		 void *cbpw);

/**
 * Prompt the user for a password for a PDF.
 */
void gui_misc_pdf_password(char **owner_pass, char **user_pass, char *path);

/**
 * Request that the cookie manager be displayed
 *
 * \param search_term The search term to be set (NULL if no search)
 * \return NSERROR_OK on success
 */
nserror gui_misc_present_cookies(const char *search_term);

#endif
