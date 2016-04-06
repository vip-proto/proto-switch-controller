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

#include "fft.h"
#include <vector>
#include <cassert>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define PI 3.14159265

FFT::FFT(int n, bool inverse)
  : n(n), inverse(inverse), index(std::vector<int>(n)) {
  lgN = 0;

  for (int i = n; i > 1; i >>= 1) {
    ++lgN;
    assert((i & 1) == 0);
  }

  omega.resize(lgN);
  int m = 1;

  for (int s = 0; s < lgN; ++s) {
    m <<= 1;

    if (inverse)
      omega[s] = exp(Complex(0, 2.0 * PI / m));
    else
      omega[s] = exp(Complex(0, -2.0 * PI / m));
  }

  for (int i = 0; i < n; ++i) {
    int indexx = i, rev = 0;

    for (int j = 0; j < lgN; ++j) {
      rev = (rev << 1) | (indexx & 1);
      indexx >>= 1;
    }

    index[i] = rev;
  }
}

void FFT::transform(Complex* buf, Complex* result) {
  for (int i = 0; i < n; i++) result[index[i]] = buf[i];

  transform_(result);
}

void FFT::transform(float* buf, Complex* result) {
  memset(result, 0, n * sizeof(Complex));

  for (int i = 0; i < n; i++)
    result[index[i]].real(buf[i]);

  transform_(result);
}

void FFT::transform(float* re, float* imm, Complex* result) {
  for (int i = 0; i < n; i++) {
    result[index[i]].real(re[i]);
    result[index[i]].imag(imm[i]);
  }

  transform_(result);
}


void FFT::transform_(Complex* result) {
  int m = 1;

  for (int s = 0; s < lgN; ++s) {
    m <<= 1;

    for (int k = 0; k < n; k += m) {
      Complex current_omega = 1;

      for (int j = 0; j < (m >> 1); ++j) {
        Complex t = current_omega * result[k + j + (m >> 1)];
        Complex u = result[k + j];
        result[k + j] = u + t;
        result[k + j + (m >> 1)] = u - t;
        current_omega *= omega[s];
      }
    }
  }

  if (inverse == false)
    for (int i = 0; i < n; ++i)
      result[i] /= n;
}

PowerSpectrum::PowerSpectrum(uint32_t spect_size) : spect_size(spect_size),
  fft(2 * spect_size) {
  buf_size = 2 * spect_size;
  buf = (float*)malloc(buf_size * sizeof(float));
  spectrum = (float*)malloc(spect_size * sizeof(float));
  tmp = (FFT::Complex*)malloc(buf_size * sizeof(FFT::Complex));
  clearSpectrum();
  ptr = 0;
}

PowerSpectrum::~PowerSpectrum() {
  free(spectrum);
  free(buf);
  free(tmp);
}

bool PowerSpectrum::addSamples(float* samples, uint32_t size) {
  if ((ptr == 0) && (size == buf_size)) {
    transform(samples);
    return true;
  } else if (size <= buf_size - ptr) { //samples staje u slobodni deo buf-a
    memcpy(buf + ptr, samples, size * sizeof(float));
    ptr += size;

    if (ptr == buf_size) {
      transform(buf);
      return true;
    }

    return false;
  } else { //samples ne staje u slobodni deo buf-a
    uint32_t len = buf_size - ptr;
    memcpy(buf + ptr, samples, len * sizeof(float));
    transform(buf);
    addSamples(samples + len, size - len);
    return true;
  }
}

void PowerSpectrum::transform(float* samples) {
  fft.transform(samples, tmp);

  for (uint32_t i = 0; i < spect_size; i++) spectrum[i] += tmp[i].real() * tmp[i].real() + tmp[i].imag() * tmp[i].imag();

  ptr = 0;
}

void PowerSpectrum::clearSpectrum() {
  memset(spectrum, 0, spect_size * sizeof(float));
}

float* PowerSpectrum::getSpectrum() {
  return spectrum;
}

uint32_t PowerSpectrum::getSpectrumSize() {
  return spect_size;
}
