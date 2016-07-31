/* -*- c++ -*- */

#define ADSB_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "adsb_swig_doc.i"

%{
#include "adsb/preamble_detector.h"
#include "adsb/adsb_preamble_src.h"
#include "adsb/adsb_demodulator.h"
#include "adsb/msg_decoder.h"
%}


%include "adsb/preamble_detector.h"
GR_SWIG_BLOCK_MAGIC2(adsb, preamble_detector);
%include "adsb/adsb_preamble_src.h"
GR_SWIG_BLOCK_MAGIC2(adsb, adsb_preamble_src);
%include "adsb/adsb_demodulator.h"
GR_SWIG_BLOCK_MAGIC2(adsb, adsb_demodulator);
%include "adsb/msg_decoder.h"
GR_SWIG_BLOCK_MAGIC2(adsb, msg_decoder);
