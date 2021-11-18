/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "smilem.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Christmas Parents structure
enum
{
	smilem_ArcMain_Idle0,
	smilem_ArcMain_Idle1,
	smilem_ArcMain_Idle2,
	smilem_ArcMain_Idle3,
	smilem_ArcMain_Idle4,
	smilem_ArcMain_Idle5,
	smilem_ArcMain_Idle6,
	smilem_ArcMain_Idle7,
	smilem_ArcMain_LeftA0,
	smilem_ArcMain_LeftA1,
	smilem_ArcMain_LeftA2,
	smilem_ArcMain_LeftA3,
	smilem_ArcMain_DownA0,
	smilem_ArcMain_DownA1,
	smilem_ArcMain_DownA2,
	smilem_ArcMain_DownA3,
	smilem_ArcMain_UpA0,
	smilem_ArcMain_UpA1,
	smilem_ArcMain_UpA2,
	smilem_ArcMain_UpA3,
	smilem_ArcMain_RightA0,
	smilem_ArcMain_RightA1,
	smilem_ArcMain_RightA2,
	smilem_ArcMain_RightA3,
	
	smilem_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[smilem_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_smilem;

//Christmas Parents definitions
static const CharFrame char_smilem_frame[] = {
	{smilem_ArcMain_Idle0, {0,   0, 212, 200}, {130, 182}}, 
	{smilem_ArcMain_Idle1, {0,   0, 212, 200}, {130, 182}}, 
	{smilem_ArcMain_Idle2, {0,   0, 212, 200}, {130, 182}},  
	{smilem_ArcMain_Idle3, {0,   0, 212, 200}, {130, 182}},  
	{smilem_ArcMain_Idle4, {0,   0, 212, 200}, {130, 182}}, 
	{smilem_ArcMain_Idle5, {0,   0, 212, 200}, {130, 182}}, 
	{smilem_ArcMain_Idle6, {0,   0, 212, 200}, {130, 182}},  
	{smilem_ArcMain_Idle7, {0,   0, 212, 200}, {130, 182}},  

	{smilem_ArcMain_LeftA0, {0,   0, 212, 200}, {130, 182}}, 
	{smilem_ArcMain_LeftA1, {0,   0, 212, 200}, {130, 182}}, 
	{smilem_ArcMain_LeftA2, {0,   0, 212, 200}, {130, 182}}, 
	{smilem_ArcMain_LeftA3, {0,   0, 212, 200}, {130, 182}}, 

	{smilem_ArcMain_DownA0, {0,   0, 212, 200}, {130, 182}}, 
	{smilem_ArcMain_DownA1, {0,   0, 212, 200}, {130, 182}}, 
	{smilem_ArcMain_DownA2, {0,   0, 212, 200}, {130, 182}},  
	{smilem_ArcMain_DownA3, {0,   0, 212, 200}, {130, 182}}, 
	
	{smilem_ArcMain_UpA0, {0,   0, 212, 200}, {130, 182}}, 
	{smilem_ArcMain_UpA1, {0,   0, 212, 200}, {130, 182}}, 
	{smilem_ArcMain_UpA2, {0,   0, 212, 200}, {130, 182}}, 
	{smilem_ArcMain_UpA3, {0,   0, 212, 200}, {130, 182}}, 
	
	{smilem_ArcMain_RightA0, {0,   0, 212, 200}, {130, 182}}, 
	{smilem_ArcMain_RightA1, {0,   0, 212, 200}, {130, 182}}, 
	{smilem_ArcMain_RightA2, {0,   0, 212, 200}, {130, 182}}, 
	{smilem_ArcMain_RightA3, {0,   0, 212, 200}, {130, 182}}, 
};

static const Animation char_smilem_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, 5, 6, 7, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 8, 9, 10, 11, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},
	{2, (const u8[]){ 12, 13, 14, 15, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},
	{2, (const u8[]){ 16, 17, 18, 19, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},
	{2, (const u8[]){ 20, 21, 22, 23, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},
};

//Christmas Parents functions
void Char_smilem_SetFrame(void *user, u8 frame)
{
	Char_smilem *this = (Char_smilem*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_smilem_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_smilem_Tick(Character *character)
{
	Char_smilem *this = (Char_smilem*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdleM(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_smilem_SetFrame);
	Character_Draw(character, &this->tex, &char_smilem_frame[this->frame]);
}

void Char_smilem_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_smilem_Free(Character *character)
{
	Char_smilem *this = (Char_smilem*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_smilem_New(fixed_t x, fixed_t y)
{
	//Allocate Christmas Parents object
	Char_smilem *this = Mem_Alloc(sizeof(Char_smilem));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_smilem_New] Failed to allocate Christmas Parents object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_smilem_Tick;
	this->character.set_anim = Char_smilem_SetAnim;
	this->character.free = Char_smilem_Free;
	
	Animatable_Init(&this->character.animatable, char_smilem_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 5;
	
	this->character.focus_x = FIXED_DEC(25,1);
	this->character.focus_y = FIXED_DEC(-100,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\SMILEM.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim",   //smilem_ArcMain_Idle0
		"idle1.tim",   //smilem_ArcMain_Idle1
		"idle2.tim",   //smilem_ArcMain_Idle2
		"idle3.tim",   //smilem_ArcMain_Idle3
		"idle4.tim",   //smilem_ArcMain_Idle0
		"idle5.tim",   //smilem_ArcMain_Idle1
		"idle6.tim",   //smilem_ArcMain_Idle2
		"idle7.tim",   //smilem_ArcMain_Idle3
		"lefta0.tim",  //smilem_ArcMain_LeftA0
		"lefta1.tim",  //smilem_ArcMain_LeftA1
		"lefta2.tim",  //smilem_ArcMain_LeftB0
		"lefta3.tim",  //smilem_ArcMain_LeftB1
		"downa0.tim",  //smilem_ArcMain_DownA0
		"downa1.tim",  //smilem_ArcMain_DownA1
		"downa2.tim",  //smilem_ArcMain_DownB0
		"downa3.tim",  //smilem_ArcMain_DownB1
		"upa0.tim",    //smilem_ArcMain_UpA0
		"upa1.tim",    //smilem_ArcMain_UpA1
		"upa2.tim",    //smilem_ArcMain_UpB0
		"upa3.tim",    //smilem_ArcMain_UpB1
		"righta0.tim", //smilem_ArcMain_RightA0
		"righta1.tim", //smilem_ArcMain_RightA1
		"righta2.tim", //smilem_ArcMain_RightB0
		"righta3.tim", //smilem_ArcMain_RightB1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
