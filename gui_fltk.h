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

#ifndef GUI_FLTK_H
#define GUI_FLTK_H

#include "device.h"
#include "gui_forms.h"

class GeneratorChoice : public Fl_Group {
public:
  GeneratorChoice(uint8_t index, uint8_t channel, uint8_t size = 0);
  void populate();
  uint8_t getIndex();
  uint8_t getChannel();
  Generator* generator;
protected:
  uint8_t index;
  uint8_t channel;
  Fl_Choice* c;
  Fl_Box* l;
  void populate(uint8_t size);
  uint8_t max_size;
private:
  static void cmb_chng(Fl_Widget* w, Generator* g);
};

class DeviceGUI : public DeviceGUI_ {
public:
  DeviceGUI(Device* dev, int X, int Y, const char *L = 0);
  void repopulate();
protected:
  Device* dev;
private:
  static void gen_chng(Fl_Widget* w, DeviceGUI* g);
  static void remove_choice(Fl_Widget* w, DeviceGUI* g);
};

class RGBPickerGenerator : public RGBPickerGUI, public Generator {
public:
  RGBPickerGenerator(int X, int Y, const char *L = 0);
private:
  static void color_ch_callback(Fl_Widget* w, RGBPickerGenerator *p);
  static void btn_on_off_callback(Fl_Widget* w, RGBPickerGenerator *p);
  void update_output();
  bool enabled;
};

class AudioRGBLMHGUIGenerator : public AudioRGBLMHGUI, public AudioRGBLMHGenerator {
public:
  AudioRGBLMHGUIGenerator(PowerSpectrum* ps, int X, int Y, const char *L = 0);
private:
  static void sld_callback(Fl_Widget* w, AudioRGBLMHGUIGenerator* a);
};

#endif
