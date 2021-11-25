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
	happym_ArcMain_LeftA0,
	happym_ArcMain_LeftA1,
	happym_ArcMain_DownA0,
	happym_ArcMain_DownA1,
	happym_ArcMain_UpA0,
	happym_ArcMain_UpA1,
	happym_ArcMain_RightA0,
	happym_ArcMain_RightA1,

	happym_ArcMain_LeftB0,
	happym_ArcMain_LeftB1,
	happym_ArcMain_DownB0,
	happym_ArcMain_DownB1,
	happym_ArcMain_UpB0,
	happym_ArcMain_UpB1,
	happym_ArcMain_RightB0,
	happym_ArcMain_RightB1,
	
	happym_ArcMain_Idleb0,
	happym_ArcMain_Idleb1,
	happym_ArcMain_Idleb2,
	happym_ArcMain_Idleb3,

	happym_ArcMain_Laugh0,
	happym_ArcMain_Laugh1,

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
	{happym_ArcMain_Idle0, {0,   0, 106, 175}, {47, 172}}, 
	{happym_ArcMain_Idle0, {107, 0, 109, 174}, {46, 170}}, 
	{happym_ArcMain_Idle1, {0,   0, 112, 165}, {48, 162}},  
	{happym_ArcMain_Idle1, {114, 0, 118, 156}, {50, 152}},
	{happym_ArcMain_Idle2, {0,   0, 120, 154}, {52, 152}},
	{happym_ArcMain_Idle2, {122, 0, 122, 155}, {52, 153}},
	{happym_ArcMain_Idle3, {5,   0, 116, 159}, {50, 156}},
	{happym_ArcMain_Idle3, {125, 0, 106, 169}, {46, 166}},

	{happym_ArcMain_LeftA0, {0,   0, 103, 172}, {44, 170}}, 
	{happym_ArcMain_LeftA0, {110, 0, 102, 171}, {44, 169}}, 
	{happym_ArcMain_LeftA1, {0,   0, 103, 171}, {44, 169}}, 
	{happym_ArcMain_LeftA1, {109, 0, 103, 172}, {44, 170}}, 

	{happym_ArcMain_DownA0, {0,   0, 110, 174}, {47, 171}}, 
	{happym_ArcMain_DownA0, {112, 0, 104, 178}, {45, 175}}, 
	{happym_ArcMain_DownA1, {0,   0, 105, 179}, {45, 177}},  
	{happym_ArcMain_DownA1, {111, 0, 105, 178}, {46, 176}}, 
	
	{happym_ArcMain_UpA0, {0,   0, 107, 177}, {50, 174}}, 
	{happym_ArcMain_UpA0, {108, 0, 108, 177}, {49, 174}}, 
	{happym_ArcMain_UpA1, {0,   0, 107, 176}, {49, 173}}, 
	{happym_ArcMain_UpA1, {108, 0, 108, 176}, {49, 173}}, 
	
	{happym_ArcMain_RightA0, {0,   0, 103, 178}, {44, 175}}, 
	{happym_ArcMain_RightA0, {106, 0, 106, 179}, {47, 176}}, 
	{happym_ArcMain_RightA1, {0,   0, 107, 179}, {48, 176}}, 
	{happym_ArcMain_RightA1, {109, 0, 107, 179}, {48, 176}}, 

	{happym_ArcMain_LeftB0, {0,   0, 108, 175}, {49, 172}}, 
	{happym_ArcMain_LeftB0, {109, 0, 107, 174}, {48, 171}}, 
	{happym_ArcMain_LeftB1, {0,   0, 104, 173}, {45, 171}}, 
	{happym_ArcMain_LeftB1, {106, 0, 106, 174}, {47, 171}}, 

	{happym_ArcMain_DownB0, {0,   0, 110, 169}, {46, 166}}, 
	{happym_ArcMain_DownB0, {112, 0, 104, 174}, {44, 170}}, 
	{happym_ArcMain_DownB1, {0,   0, 103, 175}, {43, 172}},  
	{happym_ArcMain_DownB1, {109, 0, 107, 175}, {44, 172}}, 
	
	{happym_ArcMain_UpB0, {0,   0, 104, 186}, {47, 183}}, 
	{happym_ArcMain_UpB0, {105, 0, 107, 186}, {47, 183}}, 
	{happym_ArcMain_UpB1, {0,   0, 104, 187}, {44, 185}}, 
	{happym_ArcMain_UpB1, {104, 0, 108, 186}, {49, 184}}, 
	
	{happym_ArcMain_RightB0, {0,   0, 103, 178}, {44, 175}}, 
	{happym_ArcMain_RightB0, {110, 0, 102, 179}, {42, 176}}, 
	{happym_ArcMain_RightB1, {0,   0, 107, 179}, {47, 176}}, 
	{happym_ArcMain_RightB1, {109, 0, 103, 179}, {43, 176}}, 

	{happym_ArcMain_Idleb0, {0,   0, 104, 178}, {44, 175}}, 
	{happym_ArcMain_Idleb0, {108, 0, 104, 177}, {43, 174}}, 
	{happym_ArcMain_Idleb1, {0,   0, 112, 167}, {47, 164}},  
	{happym_ArcMain_Idleb1, {113, 0, 119, 158}, {50, 156}},  
	{happym_ArcMain_Idleb2, {0,   0, 122, 157}, {50, 155}}, 
	{happym_ArcMain_Idleb2, {128, 0, 116, 162}, {49, 159}}, 
	{happym_ArcMain_Idleb3, {0,   0, 107, 173}, {45, 170}},  
	{happym_ArcMain_Idleb3, {108, 0, 104, 176}, {44, 173}},  

	{happym_ArcMain_Laugh0, {0,   0, 103, 174}, {44, 171}}, 
	{happym_ArcMain_Laugh0, {110, 0, 102, 173}, {43, 170}}, 
	{happym_ArcMain_Laugh1, {0,   0, 103, 173}, {43, 170}},
	{happym_ArcMain_Laugh1, {109, 0, 102, 172}, {43, 170}}, 

};

static const Animation char_happym_anim[CharAnim_Max] = {
	{1, (const u8[]){ 0, 1, 2, 3, 4, 4, 5, 5, 6, 6, 7, 7, 0, 1, 2, 3, 4, 4, 5, 5, 6, 6, 7, 0, ASCR_BACK,1 }}, //CharAnim_Idle
	{2, (const u8[]){ 8, 9,10,11, ASCR_BACK,4}},         //CharAnim_Left
	{2, (const u8[]){ASCR_CHGANI, CharAnim_IdleAlt}},   
	{2, (const u8[]){ 12, 13, 14, 15, ASCR_BACK, 4}},         //CharAnim_Down
	{2, (const u8[]){ 48, 49, 50, 51,ASCR_BACK, 1}},
	{2, (const u8[]){ 16, 17, 18, 19, ASCR_BACK, 4}},         //CharAnim_Up
	{2, (const u8[]){ASCR_CHGANI, CharAnim_IdleAlt}},   
	{2, (const u8[]){ 20, 21, 22, 23, ASCR_BACK, 4}},         //CharAnim_Right
	{2, (const u8[]){ASCR_CHGANI, CharAnim_IdleAlt}},  

	{1, (const u8[]){ 40, 41, 42, 43, 44, 45, 46, 47, 47, 40, 41, 42, 43, 44, 45, 46, 47, 40, ASCR_BACK, 1}}, //CharAnim_Idleb
	{2, (const u8[]){ 24, 25, 26, 27, ASCR_BACK, 4}}, 
	{2, (const u8[]){ 28, 29, 30, 31, ASCR_BACK, 4}}, 
	{2, (const u8[]){ 32, 33, 34, 35, ASCR_BACK, 4}}, 
	{2, (const u8[]){ 36, 37, 38, 39, ASCR_BACK, 4}}, 

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

	//Camera stuff
	if ((stage.flag & STAGE_FLAG_JUST_STEP) && stage.song_step >= 444 && stage.song_step)
	{
		this->character.focus_x = FIXED_DEC(25,1);
	    this->character.focus_y = FIXED_DEC(-130,1);
    	this->character.focus_zoom = FIXED_DEC(2,1);
	}

	if ((stage.flag & STAGE_FLAG_JUST_STEP) && stage.song_step >= 447 && stage.song_step)
	{
		this->character.focus_x = FIXED_DEC(25,1);
	    this->character.focus_y = FIXED_DEC(-120,1);
    	this->character.focus_zoom = FIXED_DEC(1,1);
	}

	if ((stage.flag & STAGE_FLAG_JUST_STEP) && stage.song_step >= 703 && stage.song_step)
	{
		this->character.focus_x = FIXED_DEC(25, 1);
		this->character.focus_y = FIXED_DEC(-100, 1);
		this->character.focus_zoom = FIXED_DEC(1, 1);
	}

	if ((stage.flag & STAGE_FLAG_JUST_STEP) && stage.song_step >= 447)
		this->character.health_i = 1;

	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
{
	if (stage.song_step >= 447)
		Character_PerformIdleM2(character);

	else
	   Character_PerformIdleM(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_happym_SetFrame);
	Character_Draw(character, &this->tex, &char_happym_frame[this->frame]);
}
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
	
	this->character.health_i = 4;

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
		"lefta0.tim",  //happym_ArcMain_LeftA0
		"lefta1.tim",  //happym_ArcMain_LeftA1
		"downa0.tim",  //happym_ArcMain_DownA0
		"downa1.tim",  //happym_ArcMain_DownA1
		"upa0.tim",    //happym_ArcMain_UpA0
		"upa1.tim",    //happym_ArcMain_UpA1
		"righta0.tim", //happym_ArcMain_RightA0
		"righta1.tim", //happym_ArcMain_RightA1
		"leftb0.tim",  //happym_ArcMain_LeftA0
		"leftb1.tim",  //happym_ArcMain_LeftA1
		"downb0.tim",  //happym_ArcMain_DownA0
		"downb1.tim",  //happym_ArcMain_DownA1
		"upb0.tim",    //happym_ArcMain_UpA0
		"upb1.tim",    //happym_ArcMain_UpA1
		"rightb0.tim", //happym_ArcMain_RightA0
		"rightb1.tim", //happym_ArcMain_RightA1
		"idleb0.tim",   //happym_ArcMain_Idle0
		"idleb1.tim",   //happym_ArcMain_Idle1
		"idleb2.tim",   //happym_ArcMain_Idle2
		"idleb3.tim",   //happym_ArcMain_Idle1
		"laugh0.tim",   //happym_ArcMain_Laught
		"laugh1.tim",   //happym_ArcMain_Laught
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
