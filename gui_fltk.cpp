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

#include "gui_fltk.h"

GeneratorChoice::GeneratorChoice(uint8_t index, uint8_t channel, uint8_t size) : Fl_Group(0, 0, 150, 20) {
  c = new Fl_Choice(0, 0, 100, 20);
  l = new Fl_Box(100, 0, 50, 20);
  end();
  this->index = index;
  this->channel = channel;
  generator = NULL;
  max_size = size;
  populate(0);
}

void GeneratorChoice::cmb_chng(Fl_Widget* w, Generator* g) {
  GeneratorChoice* gc = (GeneratorChoice*)(w->parent());
  gc->generator = g;

  if (g) gc->populate(g->getNumChannels());
  else gc->populate(0);

  if (gc->callback()) gc->callback()(gc, gc->user_data());
}

void GeneratorChoice::populate() {
  populate(0);
}

void GeneratorChoice::populate(uint8_t size) {
  int no_gen;
  Generator** gens;
  bool is_last;
  char s[16];
  int sel_index, num_options = 0;
  is_last = (parent() == NULL) || (parent()->find((Fl_Widget*)this) == (parent()->children() - 1));

  if (size == 0 && generator) size = generator->getNumChannels();

  //if (c->value() >= 0) sel = (Generator*)(c->menu() + c->value())->user_data();
  c->clear();

  if (size == 0 || is_last) {
    c->add("NONE", 0, (Fl_Callback*)cmb_chng, NULL);
    sel_index = num_options++;
  }

  no_gen = Generator::getGeneratorsArraySize();
  gens = Generator::getGeneratorsArray();

  for (int i = 0; i < no_gen; i++) {
    if (((size == 0) || (size == gens[i]->getNumChannels()) || is_last) && (gens[i]->getNumChannels() <= max_size)) {
      c->add(gens[i]->name, 0, (Fl_Callback*)cmb_chng, gens[i]);

      if (gens[i] == generator) sel_index = num_options;

      num_options++;
    }
  }

  if (size > 1 && generator) {
    sprintf(s, "%u - %u", channel, channel + size - 1);
  } else if (max_size > 1) {
    sprintf(s, "%u - %u", channel, channel + max_size - 1);
  } else {
    sprintf(s, "%u", channel);
  }

  l->copy_label(s);
  c->value(sel_index);
  redraw();
}

uint8_t GeneratorChoice::getIndex() {
  return index;
}

uint8_t GeneratorChoice::getChannel() {
  return channel;
}

DeviceGUI::DeviceGUI(Device* dev, int X, int Y, const char *L) : DeviceGUI_(0, 0, 260, 260, NULL) {
  Generator *g;
  char tmp[32];
  int i, l;
  this->dev = dev;
  lbl_name->label(L);
  lbl_addr->label(dev->resource);
  sprintf(tmp, "%u channels", dev->getNumChannels());
  lbl_ch->copy_label(tmp);
  btn_minus->callback((Fl_Callback*)remove_choice, this);
  position(X, Y);
  l = 0;

  for (i = 0; (g = dev->getGenerator(i)); i++) {
    GeneratorChoice* c = new GeneratorChoice(i, l + 1, dev->getNumChannels() - l);
    c->callback((Fl_Callback*)gen_chng, this);
    c->generator = g;
    l += g->getNumChannels();
    pck_assigns->add(c);
  }

  if (dev->getNumFreeChannels()) {
    GeneratorChoice* c = new GeneratorChoice(i, l + 1, dev->getNumFreeChannels());
    c->callback((Fl_Callback*)gen_chng, this);
    pck_assigns->add(c);
  }

  if (l)
    repopulate();
  else
    btn_minus->deactivate();
}

void DeviceGUI::gen_chng(Fl_Widget* w, DeviceGUI* g) {
  GeneratorChoice* c = (GeneratorChoice*)w;
  g->dev->replaceGenerator(c->generator, c->getIndex());

  if (c->generator) {
    g->dev->populateBufferFromGenerator(c->generator, c->getChannel());
    g->dev->flushBuffer();

    if (g->pck_assigns->child(g->pck_assigns->children() - 1) == w) {
      uint8_t free_channels = g->dev->getNumFreeChannels();

      if (free_channels) {
        GeneratorChoice* c1 = new GeneratorChoice(c->getIndex() + 1, g->dev->getNumChannels() - free_channels + 1,
            free_channels);
        c1->callback((Fl_Callback*)gen_chng, g);
        g->pck_assigns->add(c1);
        g->btn_minus->activate();
        g->repopulate();
      }
    }
  }
}

void DeviceGUI::remove_choice(Fl_Widget* w, DeviceGUI* g) {
  if (g->pck_assigns->children() <= 2) g->btn_minus->deactivate();

  if (g->pck_assigns->children() > 1) {
    GeneratorChoice* gc = (GeneratorChoice*)g->pck_assigns->child(g->pck_assigns->children() - 1);
    g->dev->replaceGenerator(NULL, gc->getIndex());
    delete gc;
    g->repopulate();
  }
}

void DeviceGUI::repopulate() {
  int len = pck_assigns->children();

  for (int i = 0; i < len; i++) {
    GeneratorChoice* c = (GeneratorChoice*)pck_assigns->child(i);
    c->populate();
  }

  redraw();
}

RGBPickerGenerator::RGBPickerGenerator(int X, int Y, const char *L) : RGBPickerGUI(0, 0, 220,
      220, NULL), Generator(3) {
  position(X, Y);
  color->callback((Fl_Callback*)color_ch_callback, this);
  btn_on_off->callback((Fl_Callback*)btn_on_off_callback, this);
  btn_on_off->value(1);
  enabled = true;
  name = (char*)L;
  lbl_main->label(name);
  cmb_perm->add("RGB", "", (Fl_Callback*)color_ch_callback, this);
  cmb_perm->add("RBG", "", (Fl_Callback*)color_ch_callback, this);
  cmb_perm->add("GRB", "", (Fl_Callback*)color_ch_callback, this);
  cmb_perm->add("GBR", "", (Fl_Callback*)color_ch_callback, this);
  cmb_perm->add("BRG", "", (Fl_Callback*)color_ch_callback, this);
  cmb_perm->add("BGR", "", (Fl_Callback*)color_ch_callback, this);
  cmb_perm->value(0);
};

void RGBPickerGenerator::color_ch_callback(Fl_Widget* w, RGBPickerGenerator *p) {
  p->update_output();
}

void RGBPickerGenerator::btn_on_off_callback(Fl_Widget* w, RGBPickerGenerator *p) {
  p->enabled = !p->enabled;
  p->update_output();
}

void RGBPickerGenerator::update_output() {
  if (enabled) {
    switch (cmb_perm->value()) {
      case 0:
        ch[0] = color->r();
        ch[1] = color->g();
        ch[2] = color->b();
        break;

      case 1:
        ch[0] = color->r();
        ch[1] = color->b();
        ch[2] = color->g();
        break;

      case 2:
        ch[0] = color->g();
        ch[1] = color->r();
        ch[2] = color->b();
        break;

      case 3:
        ch[0] = color->g();
        ch[1] = color->b();
        ch[2] = color->r();
        break;

      case 4:
        ch[0] = color->b();
        ch[1] = color->r();
        ch[2] = color->g();
        break;

      case 5:
        ch[0] = color->b();
        ch[1] = color->g();
        ch[2] = color->r();
        break;
    };
  } else {
    ch[0] = 0;
    ch[1] = 0;
    ch[2] = 0;
  }

  if (callback()) callback()(this, user_data());
}

AudioRGBLMHGUIGenerator::AudioRGBLMHGUIGenerator(PowerSpectrum* ps, int X, int Y, const char *L) : AudioRGBLMHGUI(0, 0,
      220, 140, NULL), AudioRGBLMHGenerator(L, ps) {
  sld_low->callback((Fl_Callback*)sld_callback, this);
  sld_high->callback((Fl_Callback*)sld_callback, this);
  sld_low->value(f1);
  sld_high->value(f2);
  sld_callback(NULL, this);
}

void AudioRGBLMHGUIGenerator::sld_callback(Fl_Widget* w, AudioRGBLMHGUIGenerator* a) {
  char s[32];
  a->setBandwith(a->sld_low->value(), a->sld_high->value());
  sprintf(s, "Low: %u Hz", (uint32_t)(0.5 + (a->sld_low->value()) * 22050));
  a->sld_low->copy_label(s);
  sprintf(s, "High: %u Hz", (uint32_t)(0.5 + (a->sld_high->value()) * 22050));
  a->sld_high->copy_label(s);
  a->redraw();
}
