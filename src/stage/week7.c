#include "week7.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"
#include "../random.h"
#include "../mutil.h"
#include "../timer.h"

void Back_Week7_DrawBG(StageBack *back)
{
	Back_Week7 *this = (Back_Week7*)back;
	
	fixed_t fx, fy;
	
	//Draw foreground
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT fg_src = {0, 0, 256, 140};
	RECT_FIXED fg_dst = {
		FIXED_DEC(-260,1) - fx,
		FIXED_DEC(-145,1) - fy,
		FIXED_DEC(512,1),
		FIXED_DEC(300,1)
	};
	
	Stage_DrawTex(&this->tex_back0, &fg_src, &fg_dst, stage.camera.bzoom);
}

void Back_Week7_Free(StageBack *back)
{
	Back_Week7 *this = (Back_Week7*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Week7_New(void)
{
	//Allocate background structure
	Back_Week7 *this = (Back_Week7*)Mem_Alloc(sizeof(Back_Week7));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Week7_DrawBG;
	this->back.free = Back_Week7_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK7\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Mem_Free(arc_back);

	
	//Use sky coloured background
	Gfx_SetClear( 0, 0, 0);
	
	return (StageBack*)this;
}
