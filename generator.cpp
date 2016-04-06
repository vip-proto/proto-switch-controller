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

#include "generator.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

Generator::Generator(uint8_t num_channels) {
  assert(num_channels);
  this->num_channels = num_channels;
  generators.push_back(this);
  memset(ch, 0, 15 * sizeof(float));
}

Generator::~Generator() {
  for (std::vector<Generator*>::iterator it = generators.begin(); it != generators.end(); it++)
    if (*it == this) {
      generators.erase(it);
      break;
    }
}

uint8_t Generator::getNumChannels() {
  return num_channels;
}

Generator** Generator::getGeneratorsArray() {
  return generators.data();
}

int Generator::getGeneratorsArraySize() {
  return generators.size();
}

std::vector<Generator*> Generator::generators;

void TimeGenerator::updateAll() {
  for (uint32_t i = 0; i < generators.size(); i++) {
    generators[i]->update();
  }
}

TimeGenerator** TimeGenerator::getGeneratorsArray() {
  return generators.data();
}

int TimeGenerator::getGeneratorsArraySize() {
  return generators.size();
}

TimeGenerator::TimeGenerator(uint8_t num_channels) : Generator(num_channels) {
  generators.push_back(this);
}

TimeGenerator::~TimeGenerator() {
  for (std::vector<TimeGenerator*>::iterator it = generators.begin(); it != generators.end(); it++)
    if (*it == this) {
      generators.erase(it);
      break;
    }
}

std::vector<TimeGenerator*> TimeGenerator::generators;

AudioLMHGenerator::AudioLMHGenerator(PowerSpectrum* ps, uint8_t num_channels) : TimeGenerator(num_channels) {
  assert(ps);
  this->ps = ps;
  setBandwith(0.01, 0.01); // default values
}

void AudioLMHGenerator::update() {
  uint32_t i, sz;
  float* spectar = ps->getSpectrum();
  l = 0;
  m = 0;
  h = 0;
  total = 0;
  sz = ps->getSpectrumSize();

  for (i = 0; i < b1; i++) {
    l += spectar[i];
    total += spectar[i];
  }

  for (; i < b2; i++) {
    m += spectar[i];
    total += spectar[i];
  }

  for (; i < sz; i++) {
    h += spectar[i];
    total += spectar[i];
  }

  update_output();
}

void AudioLMHGenerator::setBandwith(float b1, float b2) {
  f1 = b1;
  f2 = b2;
  this->b1 = (b1 * (ps->getSpectrumSize() - 1) + 0.5);
  this->b2 = (b2 * (ps->getSpectrumSize() - 1) + 0.5);
}


AudioRGBLMHGenerator::AudioRGBLMHGenerator(const char* name, PowerSpectrum* ps) : AudioLMHGenerator(ps, 3) {
  this->name = (char*)name;
}

void AudioRGBLMHGenerator::update_output() {
  // todo: dynamic params should be used
  static float stotal = 0, sl = 0, sm = 0, sh = 0;
  stotal = 0.1 * total + 0.9 * stotal;
  sl = 0.7 * l + 0.3 * sl;
  sm = 0.1 * m + 0.9 * sm;
  sh = 0.1 * h + 0.9 * sh;
  ch[0] = l / stotal;
  ch[1] = m / stotal;
  ch[2] = h / stotal;

  if (ch[0] > 1) ch[0] = 1.;
  if (ch[0] < 0) ch[0] = 0;
  if (ch[1] > 1) ch[1] = 1.;
  if (ch[1] < 0) ch[1] = 0;
  if (ch[2] > 1) ch[2] = 1.;
  if (ch[2] < 0) ch[2] = 0;
}
