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
    		  gr::io_signature::make (1, 1, sizeof(gr_complex)),
    		  gr::io_signature::make (1, 1, sizeof(float))),
			  d_sampling_rate(sampling_rate),
			  d_frequency(carrier_freq),
			  d_pulse_duration(pulse_duration),
			  d_period_samples()
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
      const gr_complex *in = (const gr_complex *) input_items[0];
      float *out = (float *) output_items[0];
      for (int i=0;i<noutput_items;i++){
    	  //out[i] =std::abs(in[i]);
    	  //printf("%f ",out[i]);

    	  if(std::abs(in[i]) > 3.5)
    		  out[i] = 1;
    	  else{
    		  out[i] = 0;
    	  }
    	  printf("%f ",out[i]);

      }


      // Do <+signal processing+>

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace adsb */
} /* namespace gr */

