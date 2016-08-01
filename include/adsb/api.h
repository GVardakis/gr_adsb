/*
 * Copyright 2011 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_ADSB_API_H
#define INCLUDED_ADSB_API_H

#include <gnuradio/attributes.h>

#ifdef gnuradio_adsb_EXPORTS
#  define ADSB_API __GR_ATTR_EXPORT
#else
#  define ADSB_API __GR_ATTR_IMPORT
#endif


#define MAX_PACKET_TIME 112
const char letter_table[64]={'#','A','B','C','D','E','F','G','H',
	'I','J','K','L','M','N','O','P','Q','R','S','T','U','V',
	'W','X','Y','Z','#','#','#','#','#','_','#','#','#','#',
	'#','#','#','#','#','#','#','#','#','#','#','0','1','2',
	'3','4','5','6','7','8','9','#','#','#','#','#','#'};

typedef struct{
	uint8_t df; /* Downlink format */
	uint8_t tc; /* Type Code */
	uint8_t ca; /*Message Subtype */
	uint8_t icao24[3]; /*ICAO aircraft address*/
	uint8_t data[7]; /*Data frame*/
	uint8_t crc[3]; /* CRC */
}adsb_msg_t;

typedef struct{
	uint8_t surv_status; /*Surveillance status*/
	uint8_t nic; /*NIC supplement-B*/
	uint16_t altitude; /*Altitude*/
	uint8_t time; /*Time*/
	uint8_t frame_flag; /*CPR odd/even flag*/
	uint32_t lat_cpr; /*Latitude in CPR format*/
	uint32_t lon_cpr; /*Longitude in CPR format*/
}position_t;

#endif /* INCLUDED_ADSB_API_H */
