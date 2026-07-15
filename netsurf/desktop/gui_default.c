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
 * Core-provided default implementations for frontend operations that the
 * single linked frontend does not itself provide.
 *
 * Besra collapsed NetSurf's per-operation function-pointer tables (the
 * netsurf_table / guit indirection assembled by the former
 * desktop/gui_factory.c) into direct link-time calls. Where the frontend
 * implements an operation, that implementation is the definition of the
 * corresponding gui_<table>_<op>() symbol. Where it historically relied on
 * gui_factory's auto-filled default, the default lives here instead.
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "utils/errors.h"
#include "netsurf/utf8.h"
#include "netsurf/search.h"

/* utf8 -------------------------------------------------------------------- */

/**
 * Default utf8 conversion.
 *
 * The default implementation assumes the local encoding is utf8, so the
 * conversion in either direction is a simple copy.
 */
static nserror gui_default_utf8(const char *string, size_t len, char **result)
{
	assert(string && result);

	if (len == 0)
		len = strlen(string);

	*result = strndup(string, len);
	if (!(*result))
		return NSERROR_NOMEM;

	return NSERROR_OK;
}

nserror gui_utf8_utf8_to_local(const char *string, size_t len, char **result)
{
	return gui_default_utf8(string, len, result);
}

nserror gui_utf8_local_to_utf8(const char *string, size_t len, char **result)
{
	return gui_default_utf8(string, len, result);
}

/* page search --------------------------------------------------------------
 *
 * The frontend implements forward_state/back_state; the remaining notifiers
 * default to no-ops (a frontend need not surface search status/history).
 */

void gui_search_status(bool found, void *p)
{
}

void gui_search_hourglass(bool active, void *p)
{
}

void gui_search_add_recent(const char *string, void *p)
{
}
