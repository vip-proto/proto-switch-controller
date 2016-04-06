CC=g++
CFLAGS=-c -Wall -O2
LDFLAGS=-L/usr/lib -lfltk_forms -lfltk -lopenal

all: led_controller

led_controller: gui_forms.o device.o fft.o generator.o gui_fltk.o main.o
	$(CC) gui_fltk.o device.o fft.o generator.o main.o gui_forms.o $(LDFLAGS) -o led_controller

gui_forms.cxx: gui_forms.fld
	fluid -c gui_forms.fld

device.o: device.cpp
	$(CC) $(CFLAGS) device.cpp

fft.o: fft.cpp
	$(CC) $(CFLAGS) fft.cpp

gui_fltk.o: gui_fltk.cpp
	$(CC) $(CFLAGS) gui_fltk.cpp

generator.o: generator.cpp
	$(CC) $(CFLAGS) generator.cpp

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

gui_forms.o: gui_forms.cxx
	$(CC) $(CFLAGS) gui_forms.cxx

clean:
	rm -rf *.o gui_forms.cxx gui_forms.h led_controller
