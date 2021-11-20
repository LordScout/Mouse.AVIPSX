#ifndef _SPEAKERS_H
#define _SPEAKERS_H

#include "../gfx.h"
#include "../fixed.h"

//Speaker structure
typedef struct
{
	//Speaker state
	Gfx_Tex tex;
	fixed_t bump;
} Speakers;

//Speaker functions
void Speakers_Init(Speakers *this);
void Speakers_Bump(Speakers *this);
void Speakers_Tick(Speakers *this, fixed_t x, fixed_t y);

#endif
