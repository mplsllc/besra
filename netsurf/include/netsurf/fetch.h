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
 * Interface to platform-specific fetcher operations.
 */

#ifndef _NETSURF_FETCH_H_
#define _NETSURF_FETCH_H_

struct nsurl;

/**
 * Determine the MIME type of a local file.
 *
 * @note used in file fetcher
 *
 * \param unix_path Unix style path to file on disk
 * \return Pointer to MIME type string (should not be freed) -
 *	   invalidated on next call to gui_fetch_filetype.
 */
const char *gui_fetch_filetype(const char *unix_path);

/**
 * Translate resource to full url.
 *
 * @note Only used in resource protocol fetcher
 *
 * Transforms a resource protocol path into a full URL. The returned URL
 * is used as the target for a redirect. The caller takes ownership of
 * the returned nsurl including unrefing it when finished with it.
 *
 * \param path The path of the resource to locate.
 * \return A netsurf url object containing the full URL of the resource
 *         path or NULL if a suitable resource URL can not be generated.
 */
struct nsurl *gui_fetch_get_resource_url(const char *path);

/**
 * Translate resource to source data.
 *
 * @note Only used in resource fetcher
 *
 * Obtains the data for a resource directly
 *
 * \param path The path of the resource to locate.
 * \param data Pointer to recive data into
 * \param data_len Pointer to length of returned data
 * \return NSERROR_OK and the data and length values updated
 *         else appropriate error code.
 */
nserror gui_fetch_get_resource_data(const char *path, const uint8_t **data, size_t *data_len);

/**
 * Releases source data.
 *
 * @note Only used in resource fetcher
 *
 * Releases source data obtained from gui_fetch_get_resource_data()
 *
 * \param data The value returned from a previous gui_fetch_get_resource_data call
 * \return NSERROR_OK on success else appropriate error code.
 */
nserror gui_fetch_release_resource_data(const uint8_t *data);

/**
 * Find a MIME type for a local file
 *
 * \param ro_path path to file on disk
 * \return MIME type string (on heap, caller should free), or NULL
 */
char *gui_fetch_mimetype(const char *ro_path);

#endif
