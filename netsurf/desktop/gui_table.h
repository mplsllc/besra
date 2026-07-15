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
 * top level interface table.
 *
 * \note This should probably not be included directly but rather
 * through netsurf.h or gui_internal.h
 */

#ifndef _NETSURF_DESKTOP_GUI_TABLE_H_
#define _NETSURF_DESKTOP_GUI_TABLE_H_

struct gui_misc_table;
struct gui_window_table;
struct gui_bitmap_table;
struct gui_layout_table;

/**
 * NetSurf operation function table
 *
 * Function table implementing interface operations for the browser core.
 */
struct netsurf_table {

	/**
	 * Browser table.
	 *
	 * Provides miscellaneous browser functionality.
	 *
	 * The table is mandantory and must be provided.
	 */
	struct gui_misc_table *misc;

	/**
	 * Window table.
	 *
	 * Provides all operations which affect a frontends display window.
	 *
	 * The table is mandantory and must be provided.
	 */
	struct gui_window_table *window;

	/**
	 * Bitmap table.
	 *
	 * Used by the image convertors as a generic interface to
	 * native platform-specific image formats.
	 *
	 * The table is mandantory and must be provided.
	 */
	struct gui_bitmap_table *bitmap;

	/**
	 * Layout table
	 *
	 * Used by the layout process to measure glyphs in a frontend
	 * specific manner.
	 *
	 * The table is mandantory and must be provided.
	 */
	struct gui_layout_table *layout;
};

#endif
