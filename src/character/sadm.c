/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "sadm.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Christmas Parents structure
enum
{
	XmasP_ArcMain_Idle0,
	XmasP_ArcMain_Idle1,
	XmasP_ArcMain_Idle2,
	XmasP_ArcMain_Idle3,
	XmasP_ArcMain_Idle4,
	XmasP_ArcMain_Idle5,
	XmasP_ArcMain_Idle6,
	XmasP_ArcMain_Idle7,
	XmasP_ArcMain_LeftA0,
	XmasP_ArcMain_LeftA1,
	XmasP_ArcMain_LeftA2,
	XmasP_ArcMain_LeftA3,
	XmasP_ArcMain_DownA0,
	XmasP_ArcMain_DownA1,
	XmasP_ArcMain_DownA2,
	XmasP_ArcMain_DownA3,
	XmasP_ArcMain_UpA0,
	XmasP_ArcMain_UpA1,
	XmasP_ArcMain_UpA2,
	XmasP_ArcMain_UpA3,
	XmasP_ArcMain_RightA0,
	XmasP_ArcMain_RightA1,
	XmasP_ArcMain_RightA2,
	XmasP_ArcMain_RightA3,
	
	XmasP_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[XmasP_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_XmasP;

//Christmas Parents definitions
static const CharFrame char_xmasp_frame[] = {
	{XmasP_ArcMain_Idle0, {0,   0, 102, 171}, {43, 167}}, 
	{XmasP_ArcMain_Idle1, {0,   0, 103, 170}, {44, 166}}, 
	{XmasP_ArcMain_Idle2, {0,   0, 112, 161}, {48, 157}},  
	{XmasP_ArcMain_Idle3, {0,   0, 118, 151}, {50, 148}},  
	{XmasP_ArcMain_Idle4, {0,   0, 120, 150}, {51, 148}}, 
	{XmasP_ArcMain_Idle5, {0,   0, 120, 150}, {51, 148}}, 
	{XmasP_ArcMain_Idle6, {0,   0, 115, 155}, {49, 153}},  
	{XmasP_ArcMain_Idle7, {0,   0, 107, 166}, {45, 163}},  

	{XmasP_ArcMain_LeftA0, {0,   0, 103, 169}, {44, 166}}, 
	{XmasP_ArcMain_LeftA1, {0,   0, 102, 169}, {43, 166}}, 
	{XmasP_ArcMain_LeftA2, {0,   0, 103, 168}, {44, 166}}, 
	{XmasP_ArcMain_LeftA3, {0,   0, 103, 168}, {44, 165}}, 

	{XmasP_ArcMain_DownA0, {0,   0, 110, 172}, {46, 169}}, 
	{XmasP_ArcMain_DownA1, {0,   0, 104, 176}, {45, 173}},
	{XmasP_ArcMain_DownA2, {0,   0, 104, 177}, {44, 174}},
	{XmasP_ArcMain_DownA3, {0,   0, 104, 177}, {45, 174}},
	
	{XmasP_ArcMain_UpA0, {0,   0, 103, 177}, {47, 174}},
	{XmasP_ArcMain_UpA1, {0,   0, 104, 176}, {46, 173}},
	{XmasP_ArcMain_UpA2, {0,   0, 104, 176}, {45, 173}},
	{XmasP_ArcMain_UpA3, {0,   0, 104, 175}, {46, 172}},
	
	{XmasP_ArcMain_RightA0, {0,   0, 103, 172}, {44, 169}},
	{XmasP_ArcMain_RightA1, {0,   0, 103, 173}, {44, 170}},
	{XmasP_ArcMain_RightA2, {0,   0, 104, 173}, {46, 170}},
	{XmasP_ArcMain_RightA3, {0,   0, 104, 173}, {46, 170 }},
};

static const Animation char_xmasp_anim[CharAnim_Max] = {
	{1, (const u8[]){ 1, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 7, 0, 1, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 0, ASCR_BACK,0}}, //CharAnim_Idle
	{2, (const u8[]){ 8, 9, 10, 11, ASCR_BACK, 4}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},
	{2, (const u8[]){ 12, 13, 14, 15, ASCR_BACK, 4}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},
	{2, (const u8[]){ 16, 17, 18, 19, ASCR_BACK, 4}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},
	{2, (const u8[]){ 20, 21, 22, 23, ASCR_BACK, 4}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},
};

//Christmas Parents functions
void Char_XmasP_SetFrame(void *user, u8 frame)
{
	Char_XmasP *this = (Char_XmasP*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_xmasp_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_XmasP_Tick(Character *character)
{
	Char_XmasP *this = (Char_XmasP*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdleM(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_XmasP_SetFrame);
	Character_Draw(character, &this->tex, &char_xmasp_frame[this->frame]);
}

void Char_XmasP_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_XmasP_Free(Character *character)
{
	Char_XmasP *this = (Char_XmasP*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_XmasP_New(fixed_t x, fixed_t y)
{
	//Allocate Christmas Parents object
	Char_XmasP *this = Mem_Alloc(sizeof(Char_XmasP));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_XmasP_New] Failed to allocate Christmas Parents object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_XmasP_Tick;
	this->character.set_anim = Char_XmasP_SetAnim;
	this->character.free = Char_XmasP_Free;
	
	Animatable_Init(&this->character.animatable, char_xmasp_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 3;
	
	this->character.focus_x = FIXED_DEC(25,1);
	this->character.focus_y = FIXED_DEC(-100,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\XMASP.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim",   //XmasP_ArcMain_Idle0
		"idle1.tim",   //XmasP_ArcMain_Idle1
		"idle2.tim",   //XmasP_ArcMain_Idle2
		"idle3.tim",   //XmasP_ArcMain_Idle3
		"idle4.tim",   //XmasP_ArcMain_Idle0
		"idle5.tim",   //XmasP_ArcMain_Idle1
		"idle6.tim",   //XmasP_ArcMain_Idle2
		"idle7.tim",   //XmasP_ArcMain_Idle3
		"lefta0.tim",  //XmasP_ArcMain_LeftA0
		"lefta1.tim",  //XmasP_ArcMain_LeftA1
		"lefta2.tim",  //XmasP_ArcMain_LeftB0
		"lefta3.tim",  //XmasP_ArcMain_LeftB1
		"downa0.tim",  //XmasP_ArcMain_DownA0
		"downa1.tim",  //XmasP_ArcMain_DownA1
		"downa2.tim",  //XmasP_ArcMain_DownB0
		"downa3.tim",  //XmasP_ArcMain_DownB1
		"upa0.tim",    //XmasP_ArcMain_UpA0
		"upa1.tim",    //XmasP_ArcMain_UpA1
		"upa2.tim",    //XmasP_ArcMain_UpB0
		"upa3.tim",    //XmasP_ArcMain_UpB1
		"righta0.tim", //XmasP_ArcMain_RightA0
		"righta1.tim", //XmasP_ArcMain_RightA1
		"righta2.tim", //XmasP_ArcMain_RightB0
		"righta3.tim", //XmasP_ArcMain_RightB1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
