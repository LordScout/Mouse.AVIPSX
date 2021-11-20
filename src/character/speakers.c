#include "speakers.h"

#include "../io.h"
#include "../stage.h"
#include "../timer.h"

//Speaker functions
void Speakers_Init(Speakers *this)
{
	//Initialize speaker state
	this->bump = 0;
	
	//Load speaker graphics
	Gfx_LoadTex(&this->tex, IO_Read("\\CHAR\\SPEAKERS.TIM;1"), GFX_LOADTEX_FREE);
}

void Speakers_Bump(Speakers *this)
{
	//Set bump
	this->bump = FIXED_DEC(4,1) / 24 - 1;
}

void Speakers_Tick(Speakers *this, fixed_t x, fixed_t y)
{
	//Get frame to use according to bump
	u8 frame;
	if (this->bump > 0)
	{
		frame = (this->bump * 24) >> FIXED_SHIFT;
		this->bump -= timer_dt;
	}
	else
	{
		frame = 0;
	}
	
	//Draw speakers
	static const struct SpeakersPiece
	{
		u8 rect[4];
		u8 ox, oy;
	} speakers_draw[4][2] = {
		{ //bump 0
			{{ 97,  88, 158, 88},   0,  0},
			{{  0, 176,  18, 56}, 158, 32},
		},
		{ //bump 1
			{{176,   0,  79, 88},   0,  0},
			{{  0,  88,  97, 88},  79,  0},
		},
		{ //bump 2
			{{  0,   0,  88, 88},   0,  0},
			{{ 88,   0,  88, 88},  88,  0},
		},
		{ //bump 3
			{{  0,   0,  88, 88},   0,  0},
			{{ 88,   0,  88, 88},  88,  0},
		}
	};
	
	const struct SpeakersPiece *pieces = speakers_draw[frame];
	for (int i = 0; i < 2; i++, pieces++)
	{
		//Draw piece
		RECT pieces_src = {pieces->rect[0], pieces->rect[1], pieces->rect[2], pieces->rect[3]};
		RECT_FIXED pieces_dst = {
			x - FIXED_DEC(88,1) + ((fixed_t)pieces->ox << FIXED_SHIFT) - stage.camera.x,
			y + ((fixed_t)pieces->oy << FIXED_SHIFT) - stage.camera.y,
			(fixed_t)pieces->rect[2] << FIXED_SHIFT,
			(fixed_t)pieces->rect[3] << FIXED_SHIFT,
		};
		
		Stage_DrawTex(&this->tex, &pieces_src, &pieces_dst, stage.camera.bzoom);
	}
}
