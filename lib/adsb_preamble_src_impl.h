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

#ifndef INCLUDED_ADSB_ADSB_PREAMBLE_SRC_IMPL_H
#define INCLUDED_ADSB_ADSB_PREAMBLE_SRC_IMPL_H

#include <adsb/adsb_preamble_src.h>

namespace gr {
  namespace adsb {

    class adsb_preamble_src_impl : public adsb_preamble_src
    {
     private:
      double d_sampling_rate;
      size_t d_pulse_duration;
      size_t d_output_duration;
      gr_complex* d_preamble;
      gr_complex* pulse_wave;

     public:
      adsb_preamble_src_impl(double samp_rate);
      ~adsb_preamble_src_impl();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace adsb
} // namespace gr

#endif /* INCLUDED_ADSB_ADSB_PREAMBLE_SRC_IMPL_H */

