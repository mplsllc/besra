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
 * Interface to platform-specific download operations.
 */

#ifndef _NETSURF_DOWNLOAD_H_
#define _NETSURF_DOWNLOAD_H_

struct gui_window;
struct download_context;

struct gui_download_window *gui_download_create(struct download_context *ctx, struct gui_window *parent);

nserror gui_download_data(struct gui_download_window *dw, const char *data, unsigned int size);

void gui_download_error(struct gui_download_window *dw, const char *error_msg);

void gui_download_done(struct gui_download_window *dw);

#endif
