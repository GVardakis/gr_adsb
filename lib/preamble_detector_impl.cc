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
#include "preamble_detector_impl.h"

namespace gr {
  namespace adsb {

    preamble_detector::sptr
    preamble_detector::make(double sampling_rate, double pulse_duration, double carrier_freq)
    {
      return gnuradio::get_initial_sptr
        (new preamble_detector_impl(sampling_rate, pulse_duration, carrier_freq));
    }

    /*
     * The private constructor
     */
    preamble_detector_impl::preamble_detector_impl(double sampling_rate, double pulse_duration, double carrier_freq)
      : gr::sync_block("preamble_detector",
    		  gr::io_signature::make (1, 1, sizeof(float)),
    		  gr::io_signature::make2 (2, 2, sizeof(float),sizeof(short int))),
			  d_sampling_rate(sampling_rate),
			  d_frequency(carrier_freq),
			  d_pulse_duration(pulse_duration),
			  d_preamble_bin(3422760960),
			  d_history(0),
			  d_receiving(false),
			  d_max_msg_time_us(112),
			  d_max_msg_samples(112*4),
			  d_samples_count(0)
    {

    }

    /*
     * Our virtual destructor.
     */
    preamble_detector_impl::~preamble_detector_impl()
    {
    }

    int
    preamble_detector_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const float *in = (const float *) input_items[0];
      float *out1 = (float *) output_items[0];
      short int *out2 = (short int *) output_items[1];
      uint8_t current = 0;
      for (int i=0;i<noutput_items;i++){
    	  out2[i] = d_receiving;
    	  if(!d_receiving){
    		  if(in[i] -23 > 0)
    			  current = 1 ;
    		  else{
    			  current = 0;
    		  }
    		  d_history = ((d_history<<1) | current) & 0xffffffff;
    		  //printf("%ld   ",d_history);
    		  if(d_history == d_preamble_bin){
    			  printf("Hoooooooray!!!!!!!!!!!!");
    			  d_receiving=true;
    		  }
    	  }
    	  else{
    		  d_samples_count++;
    		  if(d_samples_count > d_max_msg_samples){
    			  d_receiving = false;
    			  d_samples_count =0;
    			  out2[i] = d_receiving;
    		  }
    	  }
    	  out1[i] = in[i];


      }
      // Do <+signal processing+>

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace adsb */
} /* namespace gr */

