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
 * Low-level source data cache backing store interface.
 */

#ifndef NETSURF_CONTENT_LLCACHE_PRIVATE_H_
#define NETSURF_CONTENT_LLCACHE_PRIVATE_H_

#include "content/llcache.h"

/** storage control flags */
enum backing_store_flags {
	/** no special processing */
	BACKING_STORE_NONE = 0,
	/** data is metadata */
	BACKING_STORE_META = 1,
};

/**
 * low level cache backing store operation table
 *
 * The low level cache (source objects) has the capability to make
 * objects and their metadata (headers etc) persistent by writing to a
 * backing store using these operations.
 */
/** Initialise the backing store. */
nserror gui_llcache_initialise(const struct llcache_store_parameters *parameters);

/** Finalise the backing store. */
nserror gui_llcache_finalise(void);

/**
 * Place an object in the backing store.
 *
 * The backing store takes a reference to the passed data; the caller should
 * subsequently release it with gui_llcache_release() and not free it directly.
 *
 * @param[in] url The url is used as the unique primary key for the data.
 * @param[in] flags The flags to control how the object is stored.
 * @param[in] data The objects data.
 * @param[in] datalen The length of the \a data.
 * @return NSERROR_OK on success or error code on failure.
 */
nserror gui_llcache_store(struct nsurl *url, enum backing_store_flags flags,
		 uint8_t *data, const size_t datalen);

/**
 * Retrieve an object from the backing store.
 *
 * The returned allocation is owned by the backing store and *must* be freed
 * by calling gui_llcache_release().
 *
 * @param[in] url The url is used as the unique primary key for the data.
 * @param[in] flags The flags to control how the object is retrieved.
 * @param[out] data The retrieved objects data.
 * @param[out] datalen The length of the \a data retrieved.
 * @return NSERROR_OK on success or error code on failure.
 */
nserror gui_llcache_fetch(struct nsurl *url, enum backing_store_flags flags,
		 uint8_t **data, size_t *datalen);

/** Release a previously fetched or stored memory object. */
nserror gui_llcache_release(struct nsurl *url, enum backing_store_flags flags);

/** Invalidate a source object from the backing store. */
nserror gui_llcache_invalidate(struct nsurl *url);
#endif
