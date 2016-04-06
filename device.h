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

#ifndef DEVICE_H
#define DEVICE_H

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif
#include "generator.h"
#include <stdint.h>

using namespace std;

#include <list>

class Device {
public:
  Device(uint8_t num_channels);
  virtual ~Device();

  // returns index of added generator
  uint8_t pushGenerator(Generator *g);
  // returns poped Generator*
  Generator* popGenerator();
  // replaces generator on index, returns old Generator*
  Generator* replaceGenerator(Generator *g, uint8_t index);
  void clearGenerators();
  Generator* getGenerator(uint8_t index);


  uint8_t getNumChannels();
  uint8_t getNumFreeChannels();

  // addr is raw address in packet buffer
  void pokeBuffer(const uint8_t* buf, uint8_t addr, uint8_t len);
  void pokeBuffer(uint8_t val, uint8_t addr);
  void flushBuffer();
  void clearBuffer();
  bool isBufferDirty();
  void pupulateBufferFromGeneratorStack();
  // channel is starting channel number [1 - num_channels]
  uint8_t populateBufferFromGenerator(Generator *g, uint8_t channel);

  // hardware address depending on implementation
  // can be IP address:port, /dev/vip_spi_xx, /dev/USB_xx, /dev/TTY
  char* resource;

  static void listDevices();
  static void populateAllBuffers(Generator *g, bool flush = true);
  static void flushAllBuffers();
  static bool isGeneratorAttached(Generator *g);

  virtual int open() = 0;
  virtual void close() = 0;
  virtual void send(const uint8_t* buf, uint8_t len) = 0;

protected:

  // number of hw LED channels
  uint8_t num_channels;

private:
  // devices can have max 15 generatos attached (15 is max number of hw LED channels)
  Generator* generators[15];
  uint8_t num_generators;
  uint8_t* out_buffer;
  bool out_buffer_dirty;
  static list<Device*> devices;
};

class DebugDevice : public Device {
public:
  DebugDevice();
  virtual ~DebugDevice();
  virtual int open();
  virtual void close();
  virtual void send(const uint8_t* buf, uint8_t len);
};

class UDPDevice : public Device {
public:
  UDPDevice();
  virtual ~UDPDevice();
  virtual int open();
  virtual void close();
  virtual void send(const uint8_t* buf, uint8_t len);
  uint16_t port;
protected:
  int sockfd;
  struct sockaddr_in servaddr;
};

#endif
