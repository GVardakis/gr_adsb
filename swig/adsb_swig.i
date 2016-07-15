/* -*- c++ -*- */

#define ADSB_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "adsb_swig_doc.i"

%{
#include "adsb/preamble_detector.h"
%}


%include "adsb/preamble_detector.h"
GR_SWIG_BLOCK_MAGIC2(adsb, preamble_detector);
