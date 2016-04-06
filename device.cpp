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

#include "device.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

Device::Device(uint8_t num_channels) {
  assert(num_channels);
  this->num_channels = num_channels;
  out_buffer = (uint8_t*)malloc(num_channels + 1);
  clearGenerators();
  clearBuffer();
  devices.push_back(this);
  resource = NULL;
}

Device::~Device() {
  for (std::list<Device*>::iterator it = devices.begin(); it != devices.end(); it++)
    if (*it == this) {
      devices.erase(it);
      break;
    }

  free(out_buffer);
}

uint8_t Device::pushGenerator(Generator *g) {
  if (num_generators < 14 && (getNumFreeChannels() >= g->getNumChannels())) {
    generators[num_generators] = g;
    return num_generators++;
  }

  return 0xff;
}

Generator* Device::popGenerator() {
  if (num_generators) {
    Generator* g = generators[--num_generators];
    generators[num_generators] = NULL;
    return g;
  }

  return NULL;
}

Generator* Device::replaceGenerator(Generator *g, uint8_t index) {
  assert (index <= num_generators);

  if (index == num_generators) {
    if (g == NULL) return NULL;

    assert (index < 14);
    pushGenerator(g);
    return NULL;
  }

  Generator* ret = generators[index];
  assert (g || (index == num_generators - 1));

  if (g == NULL) {
    num_generators--;
  } else {
    assert (ret->getNumChannels() == g->getNumChannels());
    generators[index] = g;
  }

  return ret;
}

void Device::clearGenerators() {
  memset(generators, 0, 15 * sizeof(void*));
  num_generators = 0;
}

Generator* Device::getGenerator(uint8_t index) {
  if (index < num_generators) return generators[index];

  return NULL;
}

void Device::pokeBuffer(const uint8_t* buf, uint8_t addr, uint8_t len) {
  //todo
  memcpy(out_buffer + addr, buf, len);
  out_buffer_dirty = true;
}

void Device::pokeBuffer(uint8_t val, uint8_t addr) {
  assert (addr <= num_channels);
  out_buffer[addr] = val;
  out_buffer_dirty = true;
}

void Device::flushBuffer() {
  if (out_buffer_dirty) {
    this->send(out_buffer, num_channels + 1);
    clearBuffer();
  }
}

void Device::clearBuffer() {
  memset(out_buffer + 1, 0x40, num_channels);
  out_buffer[0] = 0x80;
  out_buffer_dirty = false;
}

bool Device::isBufferDirty() {
  return out_buffer_dirty;
}

void Device::listDevices() {
  for (std::list<Device*>::iterator it = devices.begin(); it != devices.end(); it++) {
    printf("%p\n", *it);
  }
}

void Device::flushAllBuffers() {
  for (std::list<Device*>::iterator it = devices.begin(); it != devices.end(); it++)
    (*it)->flushBuffer();
}

bool Device::isGeneratorAttached(Generator *g) {
  for (std::list<Device*>::iterator it = devices.begin(); it != devices.end(); it++) {
    Device *dev = *it;

    for (int i = 0; i < dev->num_generators; i++)
      if (dev->generators[i] == g) return true;
  }

  return false;
}

void Device::populateAllBuffers(Generator *g, bool flush) {
  for (std::list<Device*>::iterator it = devices.begin(); it != devices.end(); it++) {
    Device *dev = *it;
    uint8_t ch = 1;

    for (int i = 0; i < dev->num_generators; i++) {
      if (dev->generators[i] == g)
        dev->populateBufferFromGenerator(g, ch);

      ch += dev->generators[i]->getNumChannels();
    }

    if (flush) dev->flushBuffer();
  }
}

void Device::pupulateBufferFromGeneratorStack() {
  uint8_t ch = 1;

  for (int i = 0; i < num_generators; i++)
    ch += populateBufferFromGenerator(generators[i], ch);
}

uint8_t Device::populateBufferFromGenerator(Generator *g, uint8_t channel) {
  int noch = g->getNumChannels();
  assert((channel > 0) && (channel <= num_channels));

  if (noch > num_channels - channel + 1) noch = num_channels - channel + 1;

  for (int i = 0; i < noch; i++) {
    uint8_t v = g->ch[i] * 63.f;

    if (v > 63) v = 63;

    out_buffer[channel + i] = v;
  }

  out_buffer_dirty |= !!noch;
  return noch;
}

uint8_t Device::getNumChannels() {
  return num_channels;
}

uint8_t Device::getNumFreeChannels() {
  int ret = 0;

  for (int i = 0; i < num_generators; i++) ret += generators[i]->getNumChannels();

  return num_channels - ret;
}

list<Device*> Device::devices;

DebugDevice::DebugDevice() : Device(10) {
  resource = (char*)"DebugDevice";
  printf("Debug device created (%p)\n", this);
}

DebugDevice::~DebugDevice() {
  printf("Debug device destroyed (%p)\n", this);
}

int DebugDevice::open() {
  printf("Debug device open (%p)\n", this);
  return 0;
}

void DebugDevice::close() {
  printf("Debug device closed (%p)\n", this);
}

void DebugDevice::send(const uint8_t* buf, uint8_t len) {
  printf("Debug device send:");

  for (int i = 0; i < len; i++) printf(" %x", buf[i]);

  printf("\n");
}

UDPDevice::UDPDevice() : Device(6) {
  sockfd = 0;
  port = 6000;
}

UDPDevice::~UDPDevice() {
  if (sockfd) close();
}

int UDPDevice::open() {
  assert(resource);
  // todo: resource should be parsed properly ([IP|host]:port)
  hostent* hent = gethostbyname(resource);

  if ((hent == NULL) || (hent->h_addrtype != AF_INET)) return 0;

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = hent->h_addrtype;
  servaddr.sin_addr.s_addr = *((unsigned long *)hent->h_addr_list[0]);
  servaddr.sin_port = htons(port);
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  return 0;
}

void UDPDevice::close() {
  shutdown(sockfd, 2); // SHUT_RDWR
}

void UDPDevice::send(const uint8_t* buf, uint8_t len) {
  sendto(sockfd, (char*)buf, 7, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
}

