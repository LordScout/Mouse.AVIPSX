/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "happym2.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Happy Mouse2 structure
enum
{
    happym2_ArcMain_Idleb0,
	happym2_ArcMain_Idleb1,
	happym2_ArcMain_Idleb2,
	happym2_ArcMain_Idleb3,
	happym2_ArcMain_Idleb4,
	happym2_ArcMain_Idleb5,
	happym2_ArcMain_Idleb6,
	happym2_ArcMain_Idleb7,
	happym2_ArcMain_LeftB0,
	happym2_ArcMain_LeftB1,
	happym2_ArcMain_LeftB2,
	happym2_ArcMain_LeftB3,
	happym2_ArcMain_DownB0,
	happym2_ArcMain_DownB1,
	happym2_ArcMain_DownB2,
	happym2_ArcMain_DownB3,
	happym2_ArcMain_UpB0,
	happym2_ArcMain_UpB1,
	happym2_ArcMain_UpB2,
	happym2_ArcMain_UpB3,
	happym2_ArcMain_RightB0,
	happym2_ArcMain_RightB1,
	happym2_ArcMain_RightB2,
	happym2_ArcMain_RightB3,

	happym2_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[happym2_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_happym2;

//Happy Mouse2 definitions
static const CharFrame char_happym2_frame[] = {
	{happym2_ArcMain_Idleb0, {0,   0, 212, 200}, {130, 182}}, 
	{happym2_ArcMain_Idleb1, {0,   0, 212, 200}, {130, 182}}, 
	{happym2_ArcMain_Idleb2, {0,   0, 212, 200}, {130, 182}},  
	{happym2_ArcMain_Idleb3, {0,   0, 212, 200}, {130, 182}},  
	{happym2_ArcMain_Idleb4, {0,   0, 212, 200}, {130, 182}}, 
	{happym2_ArcMain_Idleb5, {0,   0, 212, 200}, {130, 182}}, 
	{happym2_ArcMain_Idleb6, {0,   0, 212, 200}, {130, 182}},  
	{happym2_ArcMain_Idleb7, {0,   0, 212, 200}, {130, 182}},  

	{happym2_ArcMain_LeftB0, {0,   0, 212, 200}, {130, 182}}, 
	{happym2_ArcMain_LeftB1, {0,   0, 212, 200}, {130, 182}}, 
	{happym2_ArcMain_LeftB2, {0,   0, 212, 200}, {130, 182}}, 
	{happym2_ArcMain_LeftB3, {0,   0, 212, 200}, {130, 182}}, 

	{happym2_ArcMain_DownB0, {0,   0, 212, 200}, {130, 182}}, 
	{happym2_ArcMain_DownB1, {0,   0, 212, 200}, {130, 182}}, 
	{happym2_ArcMain_DownB2, {0,   0, 212, 200}, {130, 182}},  
	{happym2_ArcMain_DownB3, {0,   0, 212, 200}, {130, 182}}, 
	
	{happym2_ArcMain_UpB0, {0,   0, 212, 200}, {130, 182}}, 
	{happym2_ArcMain_UpB1, {0,   0, 212, 200}, {130, 182}}, 
	{happym2_ArcMain_UpB2, {0,   0, 212, 200}, {130, 182}}, 
	{happym2_ArcMain_UpB3, {0,   0, 212, 200}, {130, 182}}, 
	
	{happym2_ArcMain_RightB0, {0,   0, 212, 200}, {130, 182}}, 
	{happym2_ArcMain_RightB1, {0,   0, 212, 200}, {130, 182}}, 
	{happym2_ArcMain_RightB2, {0,   0, 212, 200}, {130, 182}}, 
	{happym2_ArcMain_RightB3, {0,   0, 212, 200}, {130, 182}}, 

};

static const Animation char_happym2_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, 5, 6, 7, 0, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 8, 9,10,11, ASCR_BACK,1}},         //CharAnim_Left
	{2, (const u8[]){0, 1, 2, 3, 4, ASCR_BACK, 1}},   
	{2, (const u8[]){ 12, 13, 14, 15, ASCR_BACK, 1}},         //CharAnim_Down
	{2, (const u8[]){ASCR_CHGANI, CharAnim_IdleAlt}},   
	{2, (const u8[]){ 16, 17, 18, 19, ASCR_BACK, 1}},         //CharAnim_Up
	{2, (const u8[]){ASCR_CHGANI, CharAnim_IdleAlt}},   
	{2, (const u8[]){ 20, 21, 22, 23, ASCR_BACK, 1}},         //CharAnim_Right
	{2, (const u8[]){5, 6, 7, 0, ASCR_BACK, 1}},  

	{2, (const u8[]){ASCR_CHGANI, CharAnim_IdleAlt}}, //CharAnim_Idleb
	{2, (const u8[]){ASCR_CHGANI, CharAnim_IdleAlt}},
	{2, (const u8[]){ASCR_CHGANI, CharAnim_IdleAlt}}, 
	{2, (const u8[]){ASCR_CHGANI, CharAnim_IdleAlt}}, 
	{2, (const u8[]){ASCR_CHGANI, CharAnim_IdleAlt}},

};

//Happy Mouse2 functions
void Char_happym2_SetFrame(void *user, u8 frame)
{
	Char_happym2 *this = (Char_happym2*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_happym2_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_happym2_Tick(Character *character)
{
	Char_happym2 *this = (Char_happym2*)character;
	

	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
	  Character_CheckEndSing(character);
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		if ((Animatable_Ended(&character->animatable) || character->animatable.anim == CharAnim_LeftAlt || character->animatable.anim == CharAnim_RightAlt) &&
		    (character->animatable.anim != CharAnim_Left &&
		     character->animatable.anim != CharAnim_Down &&
		     character->animatable.anim != CharAnim_Up &&
		     character->animatable.anim != CharAnim_Right) &&
		    (stage.song_step & 0x3) == 0)
			character->set_anim(character, CharAnim_Idle);
	}
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_happym2_SetFrame);
	Character_Draw(character, &this->tex, &char_happym2_frame[this->frame]);
}

void Char_happym2_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);

	if (anim == CharAnim_Idle)
	{
		if (character->animatable.anim == CharAnim_LeftAlt)
			anim = CharAnim_RightAlt;
		else
			anim = CharAnim_LeftAlt;
		character->sing_end = FIXED_DEC(0x7FFF,1);
	}
	else
		Character_CheckStartSing(character);
}


void Char_happym2_Free(Character *character)
{
	Char_happym2 *this = (Char_happym2*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_happym2_New(fixed_t x, fixed_t y)
{
	//Allocate Happy Mouse2 object
	Char_happym2 *this = Mem_Alloc(sizeof(Char_happym2));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_happym2_New] Failed to allocate Happy Mouse2 object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_happym2_Tick;
	this->character.set_anim = Char_happym2_SetAnim;
	this->character.free = Char_happym2_Free;
	
	Animatable_Init(&this->character.animatable, char_happym2_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 5;

	this->character.focus_x = FIXED_DEC(25,1);
	this->character.focus_y = FIXED_DEC(-100,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\HAPPYM2.ARC;1");
	
	const char **pathp = (const char *[]){
		"idleb0.tim",   //happym2_ArcMain_Idle0
		"idleb1.tim",   //happym2_ArcMain_Idle1
		"idleb2.tim",   //happym2_ArcMain_Idle2
		"idleb3.tim",   //happym2_ArcMain_Idle3
		"idleb4.tim",   //happym2_ArcMain_Idle0
		"idleb5.tim",   //happym2_ArcMain_Idle1
		"idleb6.tim",   //happym2_ArcMain_Idle2
		"idleb7.tim",   //happym2_ArcMain_Idle3
		"leftb0.tim",  //happym2_ArcMain_LeftA0
		"leftb1.tim",  //happym2_ArcMain_LeftA1
		"leftb2.tim",  //happym2_ArcMain_LeftB0
		"leftb3.tim",  //happym2_ArcMain_LeftB1
		"downb0.tim",  //happym2_ArcMain_DownA0
		"downb1.tim",  //happym2_ArcMain_DownA1
		"downb2.tim",  //happym2_ArcMain_DownB0
		"downb3.tim",  //happym2_ArcMain_DownB1
		"upb0.tim",    //happym2_ArcMain_UpA0
		"upb1.tim",    //happym2_ArcMain_UpA1
		"upb2.tim",    //happym2_ArcMain_UpB0
		"upb3.tim",    //happym2_ArcMain_UpB1
		"rightb0.tim", //happym2_ArcMain_RightA0
		"rightb1.tim", //happym2_ArcMain_RightA1
		"rightb2.tim", //happym2_ArcMain_RightB0
		"rightb3.tim", //happym2_ArcMain_RightB1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
