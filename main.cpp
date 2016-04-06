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
#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <AL/al.h>
#include <AL/alc.h>
#include "device.h"
#include "gui_fltk.h"

#define NUM_SAMPLES 2048
#define NUM_TMP_SAMPLES 256

ALshort buffer[NUM_SAMPLES * 2];
ALCdevice *device = NULL;
PowerSpectrum psl(NUM_SAMPLES / 2), psr(NUM_SAMPLES / 2);
float lch[NUM_TMP_SAMPLES], rch[NUM_TMP_SAMPLES];
uint32_t ptr = 0;

/* Get audio samples */
bool dosample() {
  ALint sample;
  bool ret = false;
  alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, (ALCsizei)sizeof(ALint), &sample);

  if (sample <= 0) {
    return false;
  }

  if (sample > NUM_SAMPLES) sample = NUM_SAMPLES; // should never be happened ?

  alcCaptureSamples(device, (ALCvoid *)buffer, sample);
  sample *= 2;

  for (int i = 0; i < sample; i += 2) {
    lch[ptr] = .000030517578125f * ((float)buffer[i]);
    rch[ptr] = .000030517578125f * ((float)buffer[i + 1]);
    ptr++;

    if (ptr == NUM_TMP_SAMPLES) {
      ret |= psl.addSamples(lch, NUM_TMP_SAMPLES);
      psr.addSamples(rch, NUM_TMP_SAMPLES);
      ptr = 0;
    }
  }

  return ret;
}

/* Trigger time based generators, and push new audio data */
void FLidlecallback(void *w) {
  Fl::repeat_timeout(0.05, FLidlecallback);

  if (dosample()) {
    for (int i = 0; i < TimeGenerator::getGeneratorsArraySize(); i++) {
      TimeGenerator::getGeneratorsArray()[i]->update();
      Device::populateAllBuffers(TimeGenerator::getGeneratorsArray()[i], true);
      psl.clearSpectrum();
      psr.clearSpectrum();
    }
  }
}

void rgbgencallback(Fl_Widget* w, void *p) {
  Device::populateAllBuffers((Generator*)((RGBPickerGenerator*)w), true);
}

int main() {
#ifdef WIN32
  WORD wVersionRequested;
  WSADATA wsaData;
  wVersionRequested = MAKEWORD(2, 2);
  WSAStartup(wVersionRequested, &wsaData);
#endif
  /* Get available audio input devices */
  //  const ALchar *pDeviceList = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
  //  if (pDeviceList) {
  //      printf("Available Capture Devices are:\n");
  //      while (*pDeviceList) {
  //          printf("%s\n", pDeviceList);
  //          pDeviceList += strlen(pDeviceList) + 1;
  //      }
  //  }
  /* Open default recording device */
  device = alcCaptureOpenDevice(NULL, 44100, AL_FORMAT_STEREO16, NUM_SAMPLES);

  /* FlTk init */
  Fl::scheme("gtk+");
  Fl::background(0x33, 0x88, 0xaa);

  /* Make main Window */
  wnd_main = make_main();

  pck_generators->spacing(20);
  pck_devices->spacing(20);

  /* Light generator definitions, hardcoded for testing */

  /* First RGB generator */
  RGBPickerGenerator* p = new RGBPickerGenerator(0, 0, "RGB1");
  p->callback(rgbgencallback, NULL);
  pck_generators->add(p);

  /* Seccond RGB generator */
  RGBPickerGenerator* q = new RGBPickerGenerator(0, 0, "RGB2");
  q->callback(rgbgencallback, NULL);
  pck_generators->add(q);

  /* Audio generator */
  AudioRGBLMHGUIGenerator* a = new AudioRGBLMHGUIGenerator(&psl, 0, 0, "Audio");
  pck_generators->add(a);

  /* Device definitions, hardcoded for testing */

  UDPDevice *d = new UDPDevice();
  d->resource = (char*)"127.0.0.1"; /* Hardcoded IP address */
  d->open();

  DebugDevice *dd = new DebugDevice();

  /* Assign generator(s) to device(s) (initially), this can be changed in runtime */
  d->pushGenerator(p);

  /* GUI wrapper(s) for device(s) */
  DeviceGUI* d1 = new DeviceGUI(d, 0, 0, "My device");
  pck_devices->add(d1);

  DeviceGUI* d2 = new DeviceGUI(dd, 0, 0, "Debug device");
  pck_devices->add(d2);

  /* Show the main window */
  wnd_main->show();

  /* Start audio capture */
  alcCaptureStart(device);
  Fl::add_timeout(0.1, FLidlecallback, NULL);
  Fl::run();
  alcCaptureStop(device);
  alcCaptureCloseDevice(device);
  delete d1;
  delete d2;
  delete d;
  delete dd;
  delete p;
  delete q;
  delete a;
}
