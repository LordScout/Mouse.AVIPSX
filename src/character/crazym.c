/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "crazym.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Christmas Parents structure
enum
{
	crazym_ArcMain_Idle0,
	crazym_ArcMain_Idle1,
	crazym_ArcMain_Idle2,
	crazym_ArcMain_Idle3,
	crazym_ArcMain_LeftA0,
	crazym_ArcMain_LeftA1,
	crazym_ArcMain_LeftA2,
	crazym_ArcMain_LeftA3,
	crazym_ArcMain_DownA0,
	crazym_ArcMain_DownA1,
	crazym_ArcMain_DownA2,
	crazym_ArcMain_DownA3,
	crazym_ArcMain_UpA0,
	crazym_ArcMain_UpA1,
	crazym_ArcMain_UpA2,
	crazym_ArcMain_UpA3,
	crazym_ArcMain_RightA0,
	crazym_ArcMain_RightA1,
	crazym_ArcMain_RightA2,
	crazym_ArcMain_RightA3,
	crazym_ArcMain_Spook0,
	crazym_ArcMain_Spook1,
	crazym_ArcMain_Spook2,
	crazym_ArcMain_Spook3,
	crazym_ArcMain_Spook4,
	crazym_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[crazym_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_crazym;

//Christmas Parents definitions
static const CharFrame char_crazym_frame[] = {
	{crazym_ArcMain_Idle0, {  0,   0, 91, 167}, { 41, 162}}, 
	{crazym_ArcMain_Idle0, { 92,   0, 95, 166}, { 42, 161}}, 
	{crazym_ArcMain_Idle1, {  0,   0, 96, 159}, { 44, 154}},  
	{crazym_ArcMain_Idle1, { 97,   0, 113, 134}, { 54, 129}},  
	{crazym_ArcMain_Idle2, {  0,   0, 108, 133}, { 49, 128}}, 
	{crazym_ArcMain_Idle2, {109,   0, 108, 134}, { 48, 129}}, 
	{crazym_ArcMain_Idle3, {  0,   0, 107, 136}, { 48, 131}},  
	{crazym_ArcMain_Idle3, {108,   0, 100, 149}, { 45, 145}},  

	{crazym_ArcMain_LeftA0, {0,   0, 102, 166}, {52, 161}}, 
	{crazym_ArcMain_LeftA1, {0,   0, 91, 168}, {40, 163}}, 
	{crazym_ArcMain_LeftA2, {0,   0, 97, 168}, {47, 163}}, 
	{crazym_ArcMain_LeftA3, {0,   0, 92, 168}, {42, 163}}, 

	{crazym_ArcMain_DownA0, {0,   0, 110, 126}, {52, 125}}, 
	{crazym_ArcMain_DownA1, {0,   0, 105, 129}, {50, 128}}, 
	{crazym_ArcMain_DownA2, {0,   0, 103, 136}, {49, 136}},  
	{crazym_ArcMain_DownA3, {0,   0, 102, 134}, {48, 134}}, 
	
	{crazym_ArcMain_UpA0, {0,   0, 124, 160}, {68, 155}}, 
	{crazym_ArcMain_UpA1, {0,   0, 110, 154}, {60, 149}},
	{crazym_ArcMain_UpA2, {0,   0, 131, 160}, {69, 155}}, 
	{crazym_ArcMain_UpA3, {0,   0, 118, 152}, {63, 147}}, 
	
	{crazym_ArcMain_RightA0, {0,   0, 113, 160}, {43, 155}}, 
	{crazym_ArcMain_RightA1, {0,   0, 111, 158}, {41, 153}}, 
	{crazym_ArcMain_RightA2, {0,   0, 105, 161}, {41, 156}}, 
	{crazym_ArcMain_RightA3, {0,   0, 103, 160}, {41, 155}}, 

	{crazym_ArcMain_Spook0, {0,   0, 139, 210}, {72, 205}}, 
	{crazym_ArcMain_Spook1, {0,   0, 134, 216}, {71, 221}}, 
	{crazym_ArcMain_Spook2, {0,   0, 142, 217}, {73, 212}},  
	{crazym_ArcMain_Spook3, {0,   0, 141, 213}, {76, 208}},  
	{crazym_ArcMain_Spook4, {0,   0, 144, 217}, {74, 212}}, 
};

static const Animation char_crazym_anim[CharAnim_Max] = {
	{1, (const u8[]){ 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 0, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 8, 9, 10, 11, ASCR_BACK, 1}},         //CharAnim_Left
	{1, (const u8[]){ 24, 25, ASCR_BACK, 4}},         //CharAnim_spook
	{2, (const u8[]){ 12, 13, 14, 15, ASCR_BACK, 1}},         //CharAnim_Down
	{1, (const u8[]){ 26, 27, ASCR_BACK, 4}},         //CharAnim_spook
	{2, (const u8[]){ 16, 17, 18, 19, ASCR_BACK, 1}},         //CharAnim_Up
	{1, (const u8[]){ 27, 28, ASCR_BACK, 4}},         //CharAnim_spook
	{2, (const u8[]){ 20, 21, 22, 23, ASCR_BACK, 1}},         //CharAnim_Right
	{2, (const u8[]){ 28, ASCR_BACK, 4}},         //CharAnim_spook
};

//Christmas Parents functions
void Char_crazym_SetFrame(void *user, u8 frame)
{
	Char_crazym *this = (Char_crazym*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_crazym_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_crazym_Tick(Character *character)
{
	Char_crazym *this = (Char_crazym*)character;

	if (stage.song_step >= 959)
	{
	this->character.focus_x = FIXED_DEC(-10,1);
	this->character.focus_y = FIXED_DEC(-120,1);
	this->character.focus_zoom = FIXED_DEC(18,10);
	}
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdleM(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_crazym_SetFrame);
	Character_Draw(character, &this->tex, &char_crazym_frame[this->frame]);
}

void Char_crazym_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_crazym_Free(Character *character)
{
	Char_crazym *this = (Char_crazym*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_crazym_New(fixed_t x, fixed_t y)
{
	//Allocate Christmas Parents object
	Char_crazym *this = Mem_Alloc(sizeof(Char_crazym));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_crazym_New] Failed to allocate Christmas Parents object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_crazym_Tick;
	this->character.set_anim = Char_crazym_SetAnim;
	this->character.free = Char_crazym_Free;
	
	Animatable_Init(&this->character.animatable, char_crazym_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_SWAPANIM2;
	
	this->character.health_i = 5;
	
	this->character.focus_x = FIXED_DEC(25,1);
	this->character.focus_y = FIXED_DEC(-90,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\CRAZYM.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim",   //crazym_ArcMain_Idle0
		"idle1.tim",   //crazym_ArcMain_Idle1
		"idle2.tim",   //crazym_ArcMain_Idle2
		"idle3.tim",   //crazym_ArcMain_Idle3
		"lefta0.tim",  //crazym_ArcMain_LeftA0
		"lefta1.tim",  //crazym_ArcMain_LeftA1
		"lefta2.tim",  //crazym_ArcMain_LeftB0
		"lefta3.tim",  //crazym_ArcMain_LeftB1
		"downa0.tim",  //crazym_ArcMain_DownA0
		"downa1.tim",  //crazym_ArcMain_DownA1
		"downa2.tim",  //crazym_ArcMain_DownB0
		"downa3.tim",  //crazym_ArcMain_DownB1
		"upa0.tim",    //crazym_ArcMain_UpA0
		"upa1.tim",    //crazym_ArcMain_UpA1
		"upa2.tim",    //crazym_ArcMain_UpB0
		"upa3.tim",    //crazym_ArcMain_UpB1
		"righta0.tim", //crazym_ArcMain_RightA0
		"righta1.tim", //crazym_ArcMain_RightA1
		"righta2.tim", //crazym_ArcMain_RightB0
		"righta3.tim", //crazym_ArcMain_RightB1
		"spook0.tim",   //crazym_ArcMain_Idle0
		"spook1.tim",   //crazym_ArcMain_Idle1
		"spook2.tim",   //crazym_ArcMain_Idle2
		"spook3.tim",   //crazym_ArcMain_Idle3
		"spook4.tim",   //crazym_ArcMain_Idle0
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
