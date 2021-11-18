/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "happym.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Christmas Parents structure
enum
{
	happym_ArcMain_Idle0,
	happym_ArcMain_Idle1,
	happym_ArcMain_Idle2,
	happym_ArcMain_Idle3,
	happym_ArcMain_Idle4,
	happym_ArcMain_Idle5,
	happym_ArcMain_Idle6,
	happym_ArcMain_Idle7,
	happym_ArcMain_LeftA0,
	happym_ArcMain_LeftA1,
	happym_ArcMain_LeftA2,
	happym_ArcMain_LeftA3,
	happym_ArcMain_DownA0,
	happym_ArcMain_DownA1,
	happym_ArcMain_DownA2,
	happym_ArcMain_DownA3,
	happym_ArcMain_UpA0,
	happym_ArcMain_UpA1,
	happym_ArcMain_UpA2,
	happym_ArcMain_UpA3,
	happym_ArcMain_RightA0,
	happym_ArcMain_RightA1,
	happym_ArcMain_RightA2,
	happym_ArcMain_RightA3,

	happym_ArcMain_LeftB0,
	happym_ArcMain_LeftB1,
	happym_ArcMain_LeftB2,
	happym_ArcMain_LeftB3,
	happym_ArcMain_DownB0,
	happym_ArcMain_DownB1,
	happym_ArcMain_DownB2,
	happym_ArcMain_DownB3,
	happym_ArcMain_UpB0,
	happym_ArcMain_UpB1,
	happym_ArcMain_UpB2,
	happym_ArcMain_UpB3,
	happym_ArcMain_RightB0,
	happym_ArcMain_RightB1,
	happym_ArcMain_RightB2,
	happym_ArcMain_RightB3,
	
	happym_ArcMain_Idleb0,
	happym_ArcMain_Idleb1,
	happym_ArcMain_Idleb2,
	happym_ArcMain_Idleb3,
	happym_ArcMain_Idleb4,
	happym_ArcMain_Idleb5,
	happym_ArcMain_Idleb6,
	happym_ArcMain_Idleb7,

	happym_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[happym_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_happym;

//Christmas Parents definitions
static const CharFrame char_happym_frame[] = {
	{happym_ArcMain_Idle0, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_Idle1, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_Idle2, {0,   0, 212, 200}, {130, 182}},  
	{happym_ArcMain_Idle3, {0,   0, 212, 200}, {130, 182}},  
	{happym_ArcMain_Idle4, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_Idle5, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_Idle6, {0,   0, 212, 200}, {130, 182}},  
	{happym_ArcMain_Idle7, {0,   0, 212, 200}, {130, 182}},  

	{happym_ArcMain_LeftA0, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_LeftA1, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_LeftA2, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_LeftA3, {0,   0, 212, 200}, {130, 182}}, 

	{happym_ArcMain_DownA0, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_DownA1, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_DownA2, {0,   0, 212, 200}, {130, 182}},  
	{happym_ArcMain_DownA3, {0,   0, 212, 200}, {130, 182}}, 
	
	{happym_ArcMain_UpA0, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_UpA1, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_UpA2, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_UpA3, {0,   0, 212, 200}, {130, 182}}, 
	
	{happym_ArcMain_RightA0, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_RightA1, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_RightA2, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_RightA3, {0,   0, 212, 200}, {130, 182}}, 

	{happym_ArcMain_LeftB0, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_LeftB1, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_LeftB2, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_LeftB3, {0,   0, 212, 200}, {130, 182}}, 

	{happym_ArcMain_DownB0, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_DownB1, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_DownB2, {0,   0, 212, 200}, {130, 182}},  
	{happym_ArcMain_DownB3, {0,   0, 212, 200}, {130, 182}}, 
	
	{happym_ArcMain_UpB0, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_UpB1, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_UpB2, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_UpB3, {0,   0, 212, 200}, {130, 182}}, 
	
	{happym_ArcMain_RightB0, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_RightB1, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_RightB2, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_RightB3, {0,   0, 212, 200}, {130, 182}}, 

	{happym_ArcMain_Idleb0, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_Idleb1, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_Idleb2, {0,   0, 212, 200}, {130, 182}},  
	{happym_ArcMain_Idleb3, {0,   0, 212, 200}, {130, 182}},  
	{happym_ArcMain_Idleb4, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_Idleb5, {0,   0, 212, 200}, {130, 182}}, 
	{happym_ArcMain_Idleb6, {0,   0, 212, 200}, {130, 182}},  
	{happym_ArcMain_Idleb7, {0,   0, 212, 200}, {130, 182}},  
};

static const Animation char_happym_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, 5, 6, 7, 0, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 8, 9,10,11, ASCR_BACK,1}},         //CharAnim_Left
	{2, (const u8[]){ASCR_CHGANI, CharAnim_IdleAlt}},   
	{2, (const u8[]){ 12, 13, 14, 15, ASCR_BACK, 1}},         //CharAnim_Down
	{2, (const u8[]){ASCR_CHGANI, CharAnim_IdleAlt}},   
	{2, (const u8[]){ 16, 17, 18, 19, ASCR_BACK, 1}},         //CharAnim_Up
	{2, (const u8[]){ASCR_CHGANI, CharAnim_IdleAlt}},   
	{2, (const u8[]){ 20, 21, 22, 23, ASCR_BACK, 1}},         //CharAnim_Right
	{2, (const u8[]){ASCR_CHGANI, CharAnim_IdleAlt}},  

	{2, (const u8[]){ 40, 41, 42, 43, 44, 45, 46, 47, ASCR_BACK, 1}}, //CharAnim_Idleb
	{2, (const u8[]){ 24, 25, 26, 27, ASCR_BACK, 1}}, 
	{2, (const u8[]){ 30, 31, ASCR_BACK, 1}}, 
	{2, (const u8[]){ 32, 33, 34, 35, ASCR_BACK, 1}}, 
	{2, (const u8[]){ 36, 37, 38, 39, ASCR_BACK, 1}}, 

};

//Christmas Parents functions
void Char_happym_SetFrame(void *user, u8 frame)
{
	Char_happym *this = (Char_happym*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_happym_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_happym_Tick(Character *character)
{
	Char_happym *this = (Char_happym*)character;

	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)

	if (stage.song_step >= 447)
		Character_PerformIdleM2(character);

	else
	   Character_PerformIdleM(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_happym_SetFrame);
	Character_Draw(character, &this->tex, &char_happym_frame[this->frame]);
}

void Char_happym_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);

   if (stage.song_step >= 447)
	Character_CheckStartSing2(character);

   else 
	Character_CheckStartSing(character);
}

void Char_happym_Free(Character *character)
{
	Char_happym *this = (Char_happym*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_happym_New(fixed_t x, fixed_t y)
{
	//Allocate Christmas Parents object
	Char_happym *this = Mem_Alloc(sizeof(Char_happym));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_happym_New] Failed to allocate Christmas Parents object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_happym_Tick;
	this->character.set_anim = Char_happym_SetAnim;
	this->character.free = Char_happym_Free;
	
	Animatable_Init(&this->character.animatable, char_happym_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_SWAPANIM;
	
	this->character.health_i = 5;
	
	this->character.focus_x = FIXED_DEC(25,1);
	this->character.focus_y = FIXED_DEC(-100,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\HAPPYM.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim",   //happym_ArcMain_Idle0
		"idle1.tim",   //happym_ArcMain_Idle1
		"idle2.tim",   //happym_ArcMain_Idle2
		"idle3.tim",   //happym_ArcMain_Idle3
		"idle4.tim",   //happym_ArcMain_Idle0
		"idle5.tim",   //happym_ArcMain_Idle1
		"idle6.tim",   //happym_ArcMain_Idle2
		"idle7.tim",   //happym_ArcMain_Idle3
		"lefta0.tim",  //happym_ArcMain_LeftA0
		"lefta1.tim",  //happym_ArcMain_LeftA1
		"lefta2.tim",  //happym_ArcMain_LeftB0
		"lefta3.tim",  //happym_ArcMain_LeftB1
		"downa0.tim",  //happym_ArcMain_DownA0
		"downa1.tim",  //happym_ArcMain_DownA1
		"downa2.tim",  //happym_ArcMain_DownB0
		"downa3.tim",  //happym_ArcMain_DownB1
		"upa0.tim",    //happym_ArcMain_UpA0
		"upa1.tim",    //happym_ArcMain_UpA1
		"upa2.tim",    //happym_ArcMain_UpB0
		"upa3.tim",    //happym_ArcMain_UpB1
		"righta0.tim", //happym_ArcMain_RightA0
		"righta1.tim", //happym_ArcMain_RightA1
		"righta2.tim", //happym_ArcMain_RightB0
		"righta3.tim", //happym_ArcMain_RightB1
		"leftb0.tim",  //happym_ArcMain_LeftA0
		"leftb1.tim",  //happym_ArcMain_LeftA1
		"leftb2.tim",  //happym_ArcMain_LeftB0
		"leftb3.tim",  //happym_ArcMain_LeftB1
		"downb0.tim",  //happym_ArcMain_DownA0
		"downb1.tim",  //happym_ArcMain_DownA1
		"downb2.tim",  //happym_ArcMain_DownB0
		"downb3.tim",  //happym_ArcMain_DownB1
		"upb0.tim",    //happym_ArcMain_UpA0
		"upb1.tim",    //happym_ArcMain_UpA1
		"upb2.tim",    //happym_ArcMain_UpB0
		"upb3.tim",    //happym_ArcMain_UpB1
		"rightb0.tim", //happym_ArcMain_RightA0
		"rightb1.tim", //happym_ArcMain_RightA1
		"rightb2.tim", //happym_ArcMain_RightB0
		"rightb3.tim", //happym_ArcMain_RightB1
		"idleb0.tim",   //happym_ArcMain_Idle0
		"idleb1.tim",   //happym_ArcMain_Idle1
		"idleb2.tim",   //happym_ArcMain_Idle2
		"idleb3.tim",   //happym_ArcMain_Idle3
		"idleb4.tim",   //happym_ArcMain_Idle0
		"idleb5.tim",   //happym_ArcMain_Idle1
		"idleb6.tim",   //happym_ArcMain_Idle2
		"idleb7.tim",   //happym_ArcMain_Idle3
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
