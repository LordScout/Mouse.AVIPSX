#include "gfs.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

#include "speakers.h"

//gfs character structure
enum
{
	gfs_ArcMain_BopLeft,
	gfs_ArcMain_BopRight,
	gfs_ArcMain_Cry,
	
	gfs_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[gfs_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
	
	//Speaker
	Speakers speakers;
	
	//Pico test
	u16 *pico_p;
} Char_gfs;

//gfs character definitions
static const CharFrame char_gfs_frame[] = {
	{gfs_ArcMain_BopLeft, {  0,   0,  74, 103}, { 40,  73}}, //0 bop left 1
	{gfs_ArcMain_BopLeft, { 74,   0,  73, 102}, { 39,  73}}, //1 bop left 2
	{gfs_ArcMain_BopLeft, {147,   0,  73, 102}, { 39,  73}}, //2 bop left 3
	{gfs_ArcMain_BopLeft, {  0, 103,  73, 103}, { 39,  74}}, //3 bop left 4
	{gfs_ArcMain_BopLeft, { 73, 102,  82, 105}, { 43,  76}}, //4 bop left 5
	{gfs_ArcMain_BopLeft, {155, 102,  81, 105}, { 43,  76}}, //5 bop left 6
	
	{gfs_ArcMain_BopRight, {  0,   0,  81, 103}, { 43,  74}}, //6 bop right 1
	{gfs_ArcMain_BopRight, { 81,   0,  81, 103}, { 43,  74}}, //7 bop right 2
	{gfs_ArcMain_BopRight, {162,   0,  80, 103}, { 42,  74}}, //8 bop right 3
	{gfs_ArcMain_BopRight, {  0, 103,  79, 103}, { 41,  74}}, //9 bop right 4
	{gfs_ArcMain_BopRight, { 79, 103,  73, 105}, { 35,  76}}, //10 bop right 5
	{gfs_ArcMain_BopRight, {152, 103,  74, 104}, { 35,  75}}, //11 bop right 6
	
	{gfs_ArcMain_Cry, {  0,   0,  73, 101}, { 37,  73}}, //12 cry
	{gfs_ArcMain_Cry, { 73,   0,  73, 101}, { 37,  73}}, //13 cry
};

static const Animation char_gfs_anim[CharAnim_Max] = {
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_Idle
	{1, (const u8[]){ 0,  0,  1,  1,  2,  2,  3,  4,  4,  5, ASCR_BACK, 1}}, //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_LeftAlt
	{2, (const u8[]){12, 13, ASCR_REPEAT}},                                  //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_DownAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_UpAlt
	{1, (const u8[]){ 6,  6,  7,  7,  8,  8,  9, 10, 10, 11, ASCR_BACK, 1}}, //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_RightAlt
};

//gfs character functions
void Char_gfs_SetFrame(void *user, u8 frame)
{
	Char_gfs *this = (Char_gfs*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_gfs_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_gfs_Tick(Character *character)
{
	Char_gfs *this = (Char_gfs*)character;
	{
		if (stage.flag & STAGE_FLAG_JUST_STEP)
		{
			//Perform dance
			if ((stage.song_step % stage.gf_speed) == 0)
			{
				//Switch animation
				if (character->animatable.anim == CharAnim_Left)
					character->set_anim(character, CharAnim_Right);
				else
					character->set_anim(character, CharAnim_Left);
				
				//Bump speakers
				Speakers_Bump(&this->speakers);
			}
		}
	}
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_gfs_SetFrame);
	Character_Draw(character, &this->tex, &char_gfs_frame[this->frame]);
	
	//Tick speakers
	Speakers_Tick(&this->speakers, character->x, character->y);
}

void Char_gfs_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
}

void Char_gfs_Free(Character *character)
{
	Char_gfs *this = (Char_gfs*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_gfs_New(fixed_t x, fixed_t y)
{
	//Allocate gfs object
	Char_gfs *this = Mem_Alloc(sizeof(Char_gfs));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_gfs_New] Failed to allocate gfs object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_gfs_Tick;
	this->character.set_anim = Char_gfs_SetAnim;
	this->character.free = Char_gfs_Free;
	
	Animatable_Init(&this->character.animatable, char_gfs_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character stage information
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(16,1);
	this->character.focus_y = FIXED_DEC(-50,1);
	this->character.focus_zoom = FIXED_DEC(13,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\GFS.ARC;1");
	
	const char **pathp = (const char *[]){
		"bopleft.tim",  //gfs_ArcMain_BopLeft
		"bopright.tim", //gfs_ArcMain_BopRight
		"cry.tim",      //gfs_ArcMain_Cry
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize speaker
	Speakers_Init(&this->speakers);
	
		this->pico_p = NULL;
	
	return (Character*)this;
}
