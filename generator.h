/******************************************************************
  This file is part of PROTO-SWITCH Light controller project.

  Copyright (C) 2015 ViP-PROTO Association, http://vip-proto.com
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

#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdint.h>
#include "fft.h"

class Generator {
public:
  char* name;
  uint8_t getNumChannels();
  float ch[16];
  static Generator** getGeneratorsArray();
  static int getGeneratorsArraySize();
  virtual ~Generator();
protected:
  Generator(uint8_t num_channels);
  uint8_t num_channels;
  static std::vector<Generator*> generators;
};

class TimeGenerator : public Generator {
public:
  virtual void update() = 0;
  static void updateAll();
  static TimeGenerator** getGeneratorsArray();
  static int getGeneratorsArraySize();
  virtual ~TimeGenerator();
protected:
  TimeGenerator(uint8_t num_channels);
  static std::vector<TimeGenerator*> generators;
};

class AudioLMHGenerator : public TimeGenerator {
public:
  virtual void update();
  float* spectar;
  void setBandwith(float b1, float b2);
protected:
  float total;
  float l, m, h;
  virtual void update_output() = 0;
  AudioLMHGenerator(PowerSpectrum* ps, uint8_t num_channels);
  uint32_t spectar_size, b1, b2;
  float f1, f2;
  PowerSpectrum* ps;
};

class AudioRGBLMHGenerator : public AudioLMHGenerator {
public:
  AudioRGBLMHGenerator(const char* name, PowerSpectrum* ps);
protected:
  virtual void update_output();
};
#endif
