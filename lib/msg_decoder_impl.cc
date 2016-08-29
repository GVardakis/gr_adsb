/* -*- c++ -*- */
/* 
 * Copyright 2016 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "msg_decoder_impl.h"
#include <pmt/pmt.h>
#include <math.h>

#define NZ 15
#define DLATE 360.0/60
#define DLATO 360.0/59
#define ERROR_CODE -200

namespace gr {
  namespace adsb {

    msg_decoder::sptr
    msg_decoder::make()
    {
      return gnuradio::get_initial_sptr
        (new msg_decoder_impl());
    }

    /*
     * The private constructor
     */
    msg_decoder_impl::msg_decoder_impl()
      : gr::block("msg_decoder",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(0, 0, 0)),
			  d_message_in(pmt::mp("message")),
			  d_finished(false),
			  d_correct_frames(0)
    {
    	message_port_register_in(d_message_in);
    	d_byte_message = (uint8_t*)malloc((MAX_PACKET_TIME/8)*sizeof(uint8_t));
        boost::shared_ptr<boost::thread> (
        new boost::thread (
  	      boost::bind (&msg_decoder_impl::msg_decoder, this)));
    }

    /*
     * Our virtual destructor.
     */
    msg_decoder_impl::~msg_decoder_impl()
    {
    	d_finished = true;
    }

    void
	msg_decoder_impl::msg_decoder()
    {
    	int error = 0;
    	while(!d_finished){
    		error=0;
    		adsb_msg_t message;
    		pmt::pmt_t msg = delete_head_blocking(d_message_in);
    		const char* message_string = (const char*)pmt::blob_data(msg);
    		//std::cout<<"Message is"<< std::string(message_string) << std::endl;
    		std::string bin_message(message_string);
    		if(bin_message.substr(0,8).find("-") != -1){
    			printf("Corrupted bit\n");
    			error = -1;
    		}
    		if(error == -1)
    			continue;
    		std::bitset<8> b(bin_message.substr(0,8));
    		d_byte_message[0] =  b.to_ulong() & 0xFF;
    		//printf("%x ",d_byte_message[0]);
    		size_t num_bits = 0;
    		if((d_byte_message[0]>>3) == 0x11)
    			num_bits = 112;
    		else
    			num_bits = 56;
    		for(int i=1; i<num_bits/8; i++){
    			if(bin_message.substr(i*8,8).find("-") != -1){
    				printf("Corrupted bit");
    				error = -1;
    				break;
    			}
    			std::bitset<8> b(bin_message.substr(i*8,8));
    			d_byte_message[i] =  b.to_ulong() & 0xFF;
    			//printf("%x ",d_byte_message[i]);
    		}
    		if(error == -1)
    			continue;
    		message.df = d_byte_message[0] >> 3;
    		message.ca = d_byte_message[0] & 0x07;
    		message.tc = d_byte_message[4] >> 3;
    		message.icao24 = (d_byte_message[1] << 16) | (d_byte_message[2] << 8) | d_byte_message[3];
    		memcpy(&message.data,&d_byte_message[4], 7*sizeof(uint8_t));
    		memcpy(&message.crc,&d_byte_message[11],3*sizeof(uint8_t));
    		//printf("Downlink format %d Message Subtype %d Type Code %d\n",message.df, message.ca, message.tc);
    		d_correct_frames += 1;
    		printf("Correct frames = %ld\n", d_correct_frames);
    		if((message.df == 17) && ((message.tc <= 4) && (message.tc >= 1))){ // Aircraft identification message
    			std::string fnum ="";
    			for(int i =0; i < 8; i++){
    				std::bitset<8> b(bin_message.substr(40 + i*6,6));
    				uint8_t index =  b.to_ulong() & 0x3F;
    				fnum+=letter_table[index];
    			}
    			std::cout << "Aircraft number is "<<fnum<< std::endl;
    		}
    		else if((message.df == 17) && ((message.tc <= 18) && (message.tc >= 9))){ // Aircraft Position
    			position_t pos;
    			size_t altitude =0;
    			pos.surv_status = (d_byte_message[4] >> 1) & 0x03;
    			pos.nic = d_byte_message[4] & 0x01;
    			pos.altitude = (d_byte_message[5] << 4) | ((d_byte_message[6] >> 4));
    			pos.time = (d_byte_message[6] >> 3) & 0x01;
    			pos.frame_flag = (d_byte_message[6] >> 2) & 0x01;
    			pos.lat_cpr = ((d_byte_message[6] & 0x03) << 15) | (d_byte_message[7] << 7) | ((d_byte_message[8] >> 1));
    			pos.lon_cpr = ((d_byte_message[8] & 0x01) << 16) | (d_byte_message[9] << 8) | d_byte_message[10];
    			altitude = altitude_calculation(pos.altitude); /*Calculate altitude*/
    			coordinates_t coord;
    			printf("Icao = %ld flag = %d\n", message.icao24, pos.frame_flag);
    			position_t even_frame;
    			position_t odd_frame;
    			bool latest;
    			if(d_past_cpr.count(message.icao24) != 0){
    				position_t past = d_past_cpr.at(message.icao24);
    				if(past.frame_flag != pos.frame_flag){
    					if(past.frame_flag){ /*Differentiate between odd and even flags*/
    						even_frame = pos;
    						odd_frame = past;
    						latest = true;
    					}
    					else{
    						even_frame = past;
    						odd_frame = pos;
    						latest = false;
    					}
						coord = coordinates_calculation(odd_frame,even_frame,latest);
						if((coord.latitude == ERROR_CODE) || (coord.longitude == ERROR_CODE)){
							/*Coordinates not calculated*/
							continue;
						}
						printf("Latitude = %lf longitude = %lf altitude = %ld\n",coord.latitude, coord.longitude, altitude);
    				}
    				else{
    					d_past_cpr.erase(message.icao24);
    					d_past_cpr[message.icao24] = pos;
    				}
    			}
    			else{
    				d_past_cpr[message.icao24] = pos;
    			}

    		}
    		else if((message.df == 17) && (message.tc == 19)){
    			velocity_t vel;
    			vel.subtype = message.data[0] & 0x07;
    			vel.ic = message.data[1] >> 7;
    			vel.resv_a = (message.data[1] >> 6) & 0x01;
    			vel.nac = (message.data[1] >> 3) & 0x07;
    			vel.s_ew = (message.data[1] >> 2) & 0x01;
    			vel.v_ew = ((message.data[1] & 0x03) << 8) | message.data[2];
    			vel.s_ns = message.data[3] >> 7;
    			vel.v_ns = ((message.data[3] & 0x7F) << 3) | (message.data[4] >> 5);
    			vel.vrsrc = (message.data[4] >> 4) & 0x01;
    			vel.s_vr = (message.data[4] >> 3) & 0x01;
    			vel.vr = ((message.data[4] & 0x03) << 6) | (message.data[5] >> 2);
    			vel.resv_b = message.data[5] & 0x03;
    			vel.s_dif = message.data[6] >> 7;
    			vel.dif = message.data[6] & 0x7f;
    			switch (vel.subtype){
    				case 1:
    					break;
    				case 2:
    					break;
    				case 3:
    					break;
    				case 4:
    					break;
    				default:
    					break;
    			}

    		}

    	}
    }
    size_t
	msg_decoder_impl::altitude_calculation(uint16_t alt){
    	uint8_t q = (alt >> 4) & 0x01;
    	uint16_t temp =0;
    	size_t ret =0;
    	if(1 == 1){
    		temp = (((alt >> 5) & 0x7F) << 4) | (alt & 0x0F);
    		//printf("alt = %ld \n", temp);
    		ret = temp*25 - 1000;
    		//printf("Altitude = %ld \n",ret);
    	}
    	else{
    		printf("Bad Luck \n");
    	}
    	return ret;
    }

    uint8_t
	msg_decoder_impl::compute_nl(size_t lat){
    	double a = 1 - std::cos(M_PI/(2*NZ));
    	double b = std::pow(std::cos(M_PI/180 * std::abs(lat)),2);
    	printf("a = %lf b =%lf \n",a,std::cos(M_PI/180 * std::abs(lat)));
    	return uint8_t(std::floor((2* M_PI)/(std::acos(1 - (a/b)))));
    }

    coordinates_t
	msg_decoder_impl::coordinates_calculation(position_t odd_pos, position_t even_pos, bool latest){
    	coordinates_t coord;
    	uint8_t ni;
    	int nl;
    	double DLon;
    	double calc_lon;
    	int indexM;
    	double LatE;
    	double LatO;
    	double lat_cpr_even = 1.0*even_pos.lat_cpr / pow(2,17);
    	double lat_cpr_odd = 1.0*odd_pos.lat_cpr / pow(2,17);
    	double lon_cpr_even = 1.0*even_pos.lon_cpr / pow(2,17);
    	double lon_cpr_odd = 1.0*odd_pos.lon_cpr / pow(2,17);
    	int indexJ = int(std::floor(59*lat_cpr_even - 60*lat_cpr_odd + 0.5));
    	LatE = DLATE*((indexJ%60) + lat_cpr_even);
    	LatO = DLATO*((indexJ%59) + lat_cpr_odd);
    	/* For southern hemisphere*/
    	if(LatE >= 270){
    		LatE = LatE - 360;
    	}
    	if(LatO >= 270){
    		LatO = LatO - 360;
    	}
    	if(compute_nl(LatO) != compute_nl(LatE)){
    		coord.latitude = ERROR_CODE;
    		coord.longitude = ERROR_CODE;
    		return coord; /*The two calculated latitudes fall within different ranges. Error*/
    	}
    	if(latest){
    		coord.latitude = LatE;
    	}
    	else{
    		coord.latitude = LatO;
    	}
    	if(latest){
    		nl =compute_nl(LatE);
    		printf("NL even = %d\n",nl);
    		ni = std::max(nl,1);
    		DLon = 360.0/ni;
    		indexM = int(std::floor(lon_cpr_even*(nl - 1) - lon_cpr_odd*nl + 0.5));
    		calc_lon = DLon*(indexM%ni + lon_cpr_even);
    	}
    	else{
    		nl = compute_nl(LatO);
    		printf("NL odd= %d\n",nl);
    		ni = std::max(nl-1,1);
    		DLon = 360.0/ni;
    		indexM = int(std::floor(lon_cpr_even*(nl - 1) - lon_cpr_odd*nl + 0.5));
    		calc_lon = DLon*(indexM%ni + lon_cpr_odd);
    	}
		if(calc_lon >= 180)
			coord.longitude = calc_lon -360;
		else
			coord.longitude = calc_lon;
	    return coord;
    }
    void
	msg_decoder_impl::velocity_calculation(velocity_t vel){
    	size_t v_we =0;
    	size_t v_sn =0;
    	double velocity;
    	double heading;
    	std::string vert_rate_sign;
    	std::string measurement_src;
    	if (vel.s_ns == 1){
    		v_sn = -1*(vel.v_ns - 1);
    	}
    	else{
    		v_sn = vel.v_ns - 1;
    	}
    	if(vel.s_ew == 1){
    		v_we = -1*(vel.v_ew - 1);
    	}
    	else{
    		v_we = vel.v_ew - 1;
    	}
    	velocity = std::sqrt( std::pow(v_sn,2) + std::pow(v_we,2));
    	heading = atan(1.0*v_we/v_sn) * (360/M_PI);
    	if (heading < 0)
    		heading = heading + 360;
    	if(vel.s_vr)
    		vert_rate_sign = "down";
    	else
    		vert_rate_sign = "up";
    	if(vel.vrsrc){
    		measurement_src = "Geometric Altitude change rate";
    	}
    	else{
    		measurement_src = "Baro-pressure altitude change rate";
    	}
    	printf("Velocity frame: Velocity = %lf knots, heading = %lf degrees, vertical direction = %s,"
    			" vertical rate = %d ft/min, measurement: %s\n",
				velocity,heading,vert_rate_sign,vel.vr,measurement_src);
    }

  } /* namespace adsb */
} /* namespace gr */

