/*=============================================================================
Copyright (C) 2016-2017 Authors of rpi-open-firmware
All rights reserved.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

FILE DESCRIPTION
Crappy C++ runtime.

=============================================================================*/

#include <stdint.h>
#include <stdio.h>
#include <core.h>
#include <runtime.h>

extern uintptr_t* __init_array_start;
extern uintptr_t* __init_array_end;

#ifdef __arm__
// TODO, use split sections to just not include them at link time
void* operator new[] (size_t sz) {
	return malloc(sz);
}

void* operator new (size_t sz) {
	return malloc(sz);
}

void operator delete (void* ptr) {
	free(ptr);
}

void operator delete[] (void* ptr) {
	free(ptr);
}
#endif

extern "C" void __cxa_pure_virtual() {
	panic("__cxa_pure_virtual!");
}

void __cxx_init() {
  unsigned ctor_count = (unsigned)(&__init_array_end - &__init_array_start);
  void (*static_ctor)();

  //printf("__cxx_init: calling %d static constructors (0x%p - 0x%p) ...\n", ctor_count, &__init_array_start, &__init_array_end);

  for (unsigned i = 0; i < ctor_count; i++) {
    uintptr_t* ptr = (((uintptr_t*)&__init_array_start) + i);
    //printf("ctor#%d is %lx\n", i, *ptr);
    static_ctor = reinterpret_cast<void (*)()>(*ptr);
    static_ctor();
  }
}

