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
    		pmt::pmt_t msg = delete_head_blocking(d_message_in);
    		const char* message_string = (const char*)pmt::blob_data(msg);
    		//std::cout<<"Message is"<< std::string(message_string) << std::endl;
    		std::string bin_message(message_string);
    		std::cout<< bin_message.substr(0,8)<<" ";
    		if(bin_message.substr(0,8).find("-1") != -1){
    			printf("Corrupted bit");
    			continue;
    		}
    		std::bitset<8> b(bin_message.substr(0,8));
    		d_byte_message[0] =  b.to_ulong() & 0xFF;
    		printf("%x ",d_byte_message[0]);
    		size_t num_bits = 0;
    		if((d_byte_message[0]>>3) == 0x11)
    			num_bits = 112;
    		else
    			num_bits = 56;
    		for(int i=1; i<num_bits/8; i++){
    			std::cout<< bin_message.substr(i*8,8)<<" ";
    			if(bin_message.substr(0,8).find("-1") != -1){
    				printf("Corrupted bit");
    				continue;
    			}
    			std::bitset<8> b(bin_message.substr(i*8,8));
    			d_byte_message[i] =  b.to_ulong() & 0xFF;
    			printf("%x ",d_byte_message[i]);
    		}
    		df = d_byte_message[0] >> 3;
    		tc = d_byte_message[4] >> 3;


    	}
    }

  } /* namespace adsb */
} /* namespace gr */

