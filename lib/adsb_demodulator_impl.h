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

#ifndef INCLUDED_ADSB_ADSB_DEMODULATOR_IMPL_H
#define INCLUDED_ADSB_ADSB_DEMODULATOR_IMPL_H

#include <adsb/adsb_demodulator.h>

namespace gr {
  namespace adsb {

    class adsb_demodulator_impl : public adsb_demodulator
    {
     private:

    	size_t d_samps_per_pulse;
    	size_t d_count;
    	std::string d_message;
    	size_t d_bit_one;
    	size_t d_bit_zero;
    	size_t d_current_bit;
     public:
      adsb_demodulator_impl();
      ~adsb_demodulator_impl();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace adsb
} // namespace gr

#endif /* INCLUDED_ADSB_ADSB_DEMODULATOR_IMPL_H */

