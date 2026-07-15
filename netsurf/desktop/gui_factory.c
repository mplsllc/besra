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

	/* layout table */
	err = verify_layout_register(gt->layout);
	if (err != NSERROR_OK) {
		return err;
	}

	guit = gt;

	return NSERROR_OK;
}
