/*
 * This file is part of the buscatukas distribution
 * (https://github.com/matricali/buscatukas).
 *
 * Copyright (c) 2019 Jorge Matricali.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>

#define RESERVED_IP_LEN 15

const uint32_t RESERVED_IPV4[RESERVED_IP_LEN][2] = {
	{ 0x0a000000, 0xff000000 }, // 10.0.0.0/8
	{ 0x64400000, 0xffc00000 }, // 100.64.0.0/10
	{ 0x7f000000, 0xff000000 }, // 127.0.0.0/8
	{ 0xa9fe0000, 0xffff0000 }, // 169.254.0.0/16
	{ 0xac100000, 0xfff00000 }, // 172.16.0.0/12
	{ 0xc0000000, 0xffffff00 }, // 192.0.0.0/24
	{ 0xc0000200, 0xffffff00 }, // 192.0.2.0/24
	{ 0xc0586300, 0xffffff00 }, // 192.88.99.0/24
	{ 0xc0a80000, 0xffff0000 }, // 192.168.0.0/16
	{ 0xc6120000, 0xfffe0000 }, // 198.18.0.0/15
	{ 0xc6336400, 0xffffff00 }, // 198.51.100.0/24
	{ 0xcb007100, 0xffffff00 }, // 203.0.113.0/24
	{ 0xe0000000, 0xf0000000 }, // 224.0.0.0/4
	{ 0xf0000000, 0xf0000000 }, // 240.0.0.0/4
	{ 0xffffffff, 0xffffffff }, // 255.255.255.255/32
};

bool bt_ip_is_reserved(uint32_t addr)
{
	for (int i = 0; i < RESERVED_IP_LEN; ++i) {
		if ((RESERVED_IPV4[i][0] & RESERVED_IPV4[i][1]) ==
		    (addr & RESERVED_IPV4[i][1]))
			return true;
	}

	return false;
}

bool bt_ip_is_valid(uint32_t addr)
{
	/* First octect cannot be 0 */
	if (((addr >> 24 & 0xFF) << 24) == 0)
		return false;

	/* Last octect cannot be 0 */
	if ((addr & 0xFF) == 0)
		return false;

	/* Check for reserved IP */
	return !bt_ip_is_reserved(addr);
}
