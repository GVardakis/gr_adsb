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
			  d_finished(false)
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
	size_t df =0;
	size_t tc =0;
    	while(!d_finished){
    		adsb_msg_t message;
    		pmt::pmt_t msg = delete_head_blocking(d_message_in);
    		const char* message_string = (const char*)pmt::blob_data(msg);
    		std::cout<<"Message is"<< std::string(message_string) << std::endl;
    		std::string bin_message(message_string);
    		if(bin_message.substr(0,8).find("-") != -1){
    			printf("Corrupted bit");
    			continue;
    		}
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
    				continue;
    			}
    			std::bitset<8> b(bin_message.substr(i*8,8));
    			d_byte_message[i] =  b.to_ulong() & 0xFF;
    			//printf("%x ",d_byte_message[i]);
    		}
    		message.df = d_byte_message[0] >> 3;
    		message.ca = d_byte_message[0] & 0x07;
    		message.tc = d_byte_message[4] >> 3;
    		memcpy(&message.icao24,&d_byte_message[1], 3*sizeof(uint8_t));
    		memcpy(&message.data,&d_byte_message[4], 7*sizeof(uint8_t));
    		memcpy(&message.crc,&d_byte_message[11],3*sizeof(uint8_t));
    		printf("Downlink format %d Message Subtype %d Type Code %d\n",message.df, message.ca, message.tc);
    		if((message.df == 17) && ((message.tc <= 4) && (message.tc >= 1))){ // Aircraft identification message
    			std::string fnum ="";
    			for(int i =0; i < 8; i++){
    				std::bitset<8> b(bin_message.substr(40 + i*6,6));
    				uint8_t index =  b.to_ulong() & 0x3F;
    				fnum+=letter_table[index];
    			}
    			std::cout << "Aircraft number is "<<fnum<< std::endl;
    		}
    		else if((message.df == 17) && ((message.tc <= 18) && (message.tc >= 9))){
    			position_t pos;
    			pos.surv_status = (d_byte_message[4] >> 1) & 0x03;
    			pos.nic = d_byte_message[4] & 0x01;
    			pos.altitude = (d_byte_message[5] << 4) | ((d_byte_message[6] >> 4));
    			pos.time = (d_byte_message[6] >> 3) & 0x01;
    			pos.frame_flag = (d_byte_message[6] >> 2) & 0x01;
    			pos.lat_cpr = ((d_byte_message[6] & 0x03) << 15) | (d_byte_message[7] << 7) | ((d_byte_message[8] >> 1));
    			pos.lon_cpr = ((d_byte_message[8] & 0x01) << 16) | (d_byte_message[9] << 8) | d_byte_message[10];
    			altitude_calculation(pos.altitude);

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
    		printf("Altitude = %ld \n",ret);
    	}
    	else{
    		printf("Bad Luck \n");
    	}
    	return ret;
    }

  } /* namespace adsb */
} /* namespace gr */

