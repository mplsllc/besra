/*
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
 * Portable low-level socket types and primitives (fd_set, socket address
 * families, socket close).
 *
 * This is the one place raw platform socket headers should be included from.
 * Everything else (the fetch layer, the frontends) should route through
 * libcurl or Qt for anything higher-level than "what type is a socket
 * descriptor and how do I close one."
 */

#ifndef NETSURF_UTILS_INET_H
#define NETSURF_UTILS_INET_H

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#define ns_close_socket closesocket

#else

#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ns_close_socket close

#endif

#endif
