/*  Copyright 2006 Theo Berkau

    This file is part of Yabause.

    Yabause is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Yabause is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Yabause; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef VIDSOFT_H
#define VIDSOFT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vdp1.h"

#define VIDCORE_SOFT   3

extern pixel_t *dispbuffer;

extern VideoInterface_struct VIDSoft;

void VIDSoftVdp2DrawScreen(int screen);

void VIDSoftSetBilinear(int b);

void VIDSoftSetNumPriorityThreads(int num);

void Vdp2GetInterlaceInfo(int * start_line, int * line_increment);

void VIDSoftSetNumLayerThreads(int num);

void VIDSoftSetVdp1ThreadEnable(int b);

#ifdef __cplusplus
}
#endif


#endif
