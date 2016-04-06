/******************************************************************
  This file is part of PROTO-SWITCH Light controller project.

  Copyright (C) 2016 ViP-PROTO Association, http://vip-proto.com
  Aleksandar Rikalo <aleksandar.rikalo@vip-proto.rs>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
  02111-1307, USA.

  The GNU General Public License is contained in the file COPYING.
*/

#ifndef FFT_H
#define FFT_H

#include <stdint.h>
#include <complex>
#include <vector>

class FFT {
public:
  typedef std::complex<float> Complex;

  /* Initializes FFT. n must be a power of 2. */
  FFT(int n, bool inverse = false);

  /* Computes Discrete Fourier Transform of given buffer. */
  void transform(Complex* buf, Complex* result);
  void transform(float* buf, Complex* result);
  void transform(float* re, float* imm, Complex* result);

private:
  int n, lgN;
  bool inverse;
  std::vector<Complex> omega;
  std::vector<int> index;
  inline void transform_(Complex* result);
};

class PowerSpectrum {
public:
  PowerSpectrum(uint32_t spect_size);
  ~PowerSpectrum();
  bool addSamples(float* samples, uint32_t size);
  float* getSpectrum();
  uint32_t getSpectrumSize();
  void clearSpectrum();
private:
  void transform(float* samples);
  float* spectrum;
  float* buf;
  uint32_t spect_size;
  uint32_t buf_size;
  uint32_t ptr;
  FFT fft;
  FFT::Complex* tmp;
};

#endif
