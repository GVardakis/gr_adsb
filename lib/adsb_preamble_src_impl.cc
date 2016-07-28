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
#include "adsb_preamble_src_impl.h"

#include <volk/volk.h>
#include <gnuradio/fxpt_nco.h>

namespace gr {
  namespace adsb {

    adsb_preamble_src::sptr
    adsb_preamble_src::make(double samp_rate)
    {
      return gnuradio::get_initial_sptr
        (new adsb_preamble_src_impl(samp_rate));
    }

    /*
     * The private constructor
     */
    adsb_preamble_src_impl::adsb_preamble_src_impl(double samp_rate)
      : gr::sync_block("adsb_preamble_src",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(1, samp_rate*16e-6, sizeof(gr_complex)*(samp_rate*500e-6))),
			  d_sampling_rate(samp_rate),
			  d_pulse_duration(samp_rate*500e-9),
			  d_output_duration(samp_rate*500e-6)
    {
    	const int alignment_multiple = volk_get_alignment() / sizeof(gr_complex);
    	set_alignment(std::max(1,alignment_multiple));
    	pulse_wave = (gr_complex *)volk_malloc(d_pulse_duration * sizeof(gr_complex), 32);
    	if(!pulse_wave){
    		throw std::runtime_error("Could not allocate pulse buffer");
    	}
    	d_preamble = (gr_complex *)volk_malloc(d_output_duration * sizeof(gr_complex), 32);
    	if(!d_preamble){
    		throw std::runtime_error("Could not allocate output buffer");
    	}

    	memset(pulse_wave,0,d_pulse_duration);

    	memset(d_preamble,0,d_output_duration);

    	double carrier_freq = 1090e6;
        gr::fxpt_nco nco;
        nco.set_freq(2 * M_PI * carrier_freq / samp_rate);
        nco.sincos(pulse_wave, d_pulse_duration, 1.0);

        memcpy(&d_preamble[0+1],pulse_wave,d_pulse_duration);
        memcpy(&d_preamble[2*d_pulse_duration+1],pulse_wave,d_pulse_duration);
        memcpy(&d_preamble[6*d_pulse_duration+1],pulse_wave,d_pulse_duration);
        memcpy(&d_preamble[8*d_pulse_duration+1],pulse_wave,d_pulse_duration);

    }

    /*
     * Our virtual destructor.
     */
    adsb_preamble_src_impl::~adsb_preamble_src_impl()
    {
    }

    int
    adsb_preamble_src_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      gr_complex *out = (gr_complex *) output_items[0];
      for(int i =0; i<noutput_items; i++){
    	  memcpy(&out[i],d_preamble,d_output_duration);
      }

      return noutput_items;
    }

  } /* namespace adsb */
} /* namespace gr */

