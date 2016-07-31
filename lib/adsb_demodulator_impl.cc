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
#include "adsb_demodulator_impl.h"

namespace gr {
  namespace adsb {

    adsb_demodulator::sptr
    adsb_demodulator::make()
    {
      return gnuradio::get_initial_sptr
        (new adsb_demodulator_impl());
    }

    /*
     * The private constructor
     */
    adsb_demodulator_impl::adsb_demodulator_impl()
      : gr::sync_block("adsb_demodulator",
              gr::io_signature::make2(2, 2, sizeof(float),sizeof(short int)),
              gr::io_signature::make(0,0,0)),
			  d_samps_per_pulse(2),
			  d_count(0),
			  d_message("")
    {
    	d_bit_one = (size_t)((std::floor(std::pow(2,d_samps_per_pulse)))-1) << d_samps_per_pulse;
    	d_bit_zero =(size_t)((std::floor(std::pow(2,d_samps_per_pulse)))-1);
    	printf("One = %ld zero = %ld\n",d_bit_one ,d_bit_zero);
    }

    /*
     * Our virtual destructor.
     */
    adsb_demodulator_impl::~adsb_demodulator_impl()
    {
    }

    int
    adsb_demodulator_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const float *in = (const float *) input_items[0];
      const short int *trigger = (const short int *) input_items[1];
      //<+OTYPE+> *out = (<+OTYPE+> *) output_items[0];
      uint8_t current = 0;
      for(int i=0; i<noutput_items; i++){
    	  if (trigger[i] == 1){
    		  if(d_count<2*d_samps_per_pulse){
    			  if(in[i] -23 < 0)
    				  current = 0;
    			  else
    				  current = 1;
    			  d_current_bit = (d_current_bit<<1) | current;
    			  d_count++;
    		  }
    		  else{
    			  if(d_current_bit == d_bit_one)
    				  d_message+="1";
    			  else if (d_current_bit == d_bit_zero)
    				  d_message+="0";
    			  else{
    				  //printf("%d crap\n",d_current_bit);
    				  d_message+="-1";
    			  }
    			  d_count = 0;
    			  d_current_bit = 0;
    			  if(in[i] -23 < 0)
    				  current = 0;
    			  else
    				  current = 1;
    			  d_current_bit = (d_current_bit<<1) | current;
    			  d_count=1;
    		  }
    	  }
    	  else if(trigger[i] ==0){
    		  if (d_message != ""){
    			  std::cout<<"Message " << d_message << std::endl;
    			  //printf("Message: %s\n",d_message);
    			  d_message = "";
    		  }
    		  d_count = 0;
    		  d_current_bit =0;
    	  }
      }

      // Do <+signal processing+>

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace adsb */
} /* namespace gr */

