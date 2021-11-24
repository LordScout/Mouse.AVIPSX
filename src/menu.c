/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "menu.h"

#include "mem.h"
#include "main.h"
#include "timer.h"
#include "io.h"
#include "gfx.h"
#include "audio.h"
#include "pad.h"
#include "archive.h"
#include "mutil.h"
#include "network.h"

#include "font.h"
#include "trans.h"
#include "loadscr.h"

#include "stage.h"
#include "character/mouset.h"

//Menu messages
static const char *funny_messages[][2] = {
	{"PSX PORT BY CUCKYDEV", "YOU KNOW IT"},
	{"PORTED BY ", "THREE PEOPLE LOL"},
	{"IM GONNA GETCHA", "START RUNNING"},
	{"NO JUMPSCARES", "SORRY"},
	{"THIS TIME", "HAVE MODCHARTS"},
	{"SONIC FORCES BLOWS", "FUCK THAT GAME"},
	{"PSXDEV", "HOMEBREW"},
	{"SO MANY SOULS", "SO LITTLE TIME"},
	{"MOUSE", "INSERT EXTENSION HERE"},
	{"FUN IS INFINITE", "SEGA ENTERPRISES"},
	{"SUICIDE DOT AVI", "HE KILLED MY DAD"},
	{"USE A CONTROLLER", "LOL"},
	{"PLAY SONIC EXE ON GAMEBANANA", "OGS FOREVER"},
	{"PLAYING ON EPSXE HUH", "YOURE THE PROBLEM"},
	{"GET ME TO STOP", "TRY"},
	{"OPEN SOURCE", "FOREVER"},
	{"ITS A PORT", "ITS WORSE"},
	{"BABE WAKE UP", "NEW UPDATE"},
	{"DREAMCASTNIK", "MADE ME GET OFF MY ASS"},
	{"MARSTARBRO NOTICE ME", "SAYS LORD SCOUT"},
    {"NO MORE LOGO EFFECT", "THE GLITCH WORKS DIFFERENT NOW"},
};

static u8 opening = 0;

#ifdef PSXF_NETWORK

//Menu string type
#define MENUSTR_CHARS 0x20
typedef char MenuStr[MENUSTR_CHARS + 1];

#endif

//Menu state
static struct
{
	//Menu state
	u8 page, next_page;
	boolean page_swap;
	u8 select, next_select;
	
	fixed_t scroll;
	fixed_t trans_time;
	
	//Page specific state
	union
	{
		struct
		{
			u8 funny_message;
		} opening;
		struct
		{
			fixed_t logo_bump;
			fixed_t fade, fadespd;
		} title;
		struct
		{
			fixed_t fade, fadespd;
		} story;
		struct
		{
			fixed_t back_r, back_g, back_b;
		} freeplay;
	#ifdef PSXF_NETWORK
		struct
		{
			boolean type;
			MenuStr port;
			MenuStr pass;
		} net_host;
		struct
		{
			boolean type;
			MenuStr ip;
			MenuStr port;
			MenuStr pass;
		} net_join;
		struct
		{
			boolean swap;
		} net_op;
	#endif
	} page_state;
	
	union
	{
		struct
		{
			u8 id, diff;
			boolean story;
		} stage;
	} page_param;
	
	//Menu assets
	Gfx_Tex tex_back, tex_test, tex_story, tex_title, tex_cre0, tex_cre1, tex_cre2, tex_cre3, tex_cre4, tex_cre5, tex_cre6;
	FontData font_bold, font_arial;
	
	Character *mouset; //Title Girlfriend
} menu;

#ifdef PSXF_NETWORK

//Menu string functions
static void MenuStr_Process(MenuStr *this, s32 x, s32 y, const char *fmt, boolean select, boolean type)
{
	//Append typed input
	if (select && type)
	{
		if (pad_type[0] != '\0')
			strncat(*this, pad_type, MENUSTR_CHARS - strlen(*this));
		if (pad_backspace)
		{
			size_t i = strlen(*this);
			if (i != 0)
				(*this)[i - 1] = '\0';
		}
	}
	
	//Get text to draw
	char buf[0x100];
	sprintf(buf, fmt, *this);
	if (select && type && (animf_count & 2))
		strcat(buf, "_");
	
	//Draw text
	menu.font_arial.draw_col(&menu.font_arial, buf, x, y, FontAlign_Left, 0x80, 0x80, select ? 0x00 : 0x80);
	menu.font_arial.draw_col(&menu.font_arial, buf, x+1, y+1, FontAlign_Left, 0x00, 0x00, 0x00);
}

#endif


//Internal menu functions
char menu_text_buffer[0x100];

static const char *Menu_LowerIf(const char *text, boolean lower)
{
	//Copy text
	char *dstp = menu_text_buffer;
	if (lower)
	{
		for (const char *srcp = text; *srcp != '\0'; srcp++)
		{
			if (*srcp >= 'A' && *srcp <= 'Z')
				*dstp++ = *srcp | 0x20;
			else
				*dstp++ = *srcp;
		}
	}
	else
	{
		for (const char *srcp = text; *srcp != '\0'; srcp++)
		{
			if (*srcp >= 'a' && *srcp <= 'z')
				*dstp++ = *srcp & ~0x20;
			else
				*dstp++ = *srcp;
		}
	}
	
	//Terminate text
	*dstp++ = '\0';
	return menu_text_buffer;
}

static void Menu_DrawBack(boolean flash, s32 scroll, u8 r0, u8 g0, u8 b0, u8 r1, u8 g1, u8 b1)
{
	RECT back_src = {0, 0, 255, 255};
	RECT back_dst = {0, -scroll - SCREEN_WIDEADD2, SCREEN_WIDTH, SCREEN_WIDTH * 4 / 5};
	
	if (flash || (animf_count & 4) == 0)
		Gfx_DrawTexCol(&menu.tex_back, &back_src, &back_dst, r0, g0, b0);
	else
		Gfx_DrawTexCol(&menu.tex_back, &back_src, &back_dst, r1, g1, b1);
}

static void Menu_DifficultySelector(s32 x, s32 y)
{
	//Change difficulty
	if (menu.next_page == menu.page && Trans_Idle())
	{
		if (pad_state.press & PAD_LEFT)
		{
			if (menu.page_param.stage.diff > StageDiff_Easy)
				menu.page_param.stage.diff--;
			else
				menu.page_param.stage.diff = StageDiff_Hard;
		}
		if (pad_state.press & PAD_RIGHT)
		{
			if (menu.page_param.stage.diff < StageDiff_Hard)
				menu.page_param.stage.diff++;
			else
				menu.page_param.stage.diff = StageDiff_Easy;
		}
	}
	
	//Draw difficulty arrows
	static const RECT arrow_src[2][2] = {
		{{224, 64, 16, 32}, {224, 96, 16, 32}}, //left
		{{240, 64, 16, 32}, {240, 96, 16, 32}}, //right
	};
	
	Gfx_BlitTex(&menu.tex_story, &arrow_src[0][(pad_state.held & PAD_LEFT) != 0], x - 40 - 16, y - 16);
	Gfx_BlitTex(&menu.tex_story, &arrow_src[1][(pad_state.held & PAD_RIGHT) != 0], x + 40, y - 16);
	
	//Draw difficulty
	static const RECT diff_srcs[] = {
		{  0, 96, 64, 18},
		{ 64, 96, 80, 18},
		{144, 96, 64, 18},
	};
	
	const RECT *diff_src = &diff_srcs[menu.page_param.stage.diff];
	Gfx_BlitTex(&menu.tex_story, diff_src, x - (diff_src->w >> 1), y - 9 + ((pad_state.press & (PAD_LEFT | PAD_RIGHT)) != 0));
}

static void Menu_DrawWeek(const char *week, s32 x, s32 y)
{
	//Draw label
	if (week == NULL)
	{
		//Tutorial
		RECT label_src = {0, 0, 112, 32};
		Gfx_BlitTex(&menu.tex_story, &label_src, x, y);
	}
	else
	{
		//Week
		RECT label_src = {0, 32, 80, 32};
		Gfx_BlitTex(&menu.tex_story, &label_src, x, y);
		
		//Number
		x += 80;
		for (; *week != '\0'; week++)
		{
			//Draw number
			u8 i = *week - '0';
			
			RECT num_src = {128 + ((i & 3) << 5), ((i >> 2) << 5), 32, 32};
			Gfx_BlitTex(&menu.tex_story, &num_src, x, y);
			x += 32;
		}
	}
}

//Menu functions
void Menu_Load(MenuPage page)
{
	//Load menu assets
	IO_Data menu_arc = IO_Read("\\MENU\\MENU.ARC;1");
	Gfx_LoadTex(&menu.tex_back,  Archive_Find(menu_arc, "back.tim"),  0);
	Gfx_LoadTex(&menu.tex_test,    Archive_Find(menu_arc, "test.tim"),    0);
	Gfx_LoadTex(&menu.tex_story, Archive_Find(menu_arc, "story.tim"), 0);
	Gfx_LoadTex(&menu.tex_title, Archive_Find(menu_arc, "title.tim"), 0);
	Gfx_LoadTex(&menu.tex_cre0, Archive_Find(menu_arc, "cre0.tim"), 0);
	Gfx_LoadTex(&menu.tex_cre1, Archive_Find(menu_arc, "cre1.tim"), 0);
	Gfx_LoadTex(&menu.tex_cre2, Archive_Find(menu_arc, "cre2.tim"), 0);
	Gfx_LoadTex(&menu.tex_cre3, Archive_Find(menu_arc, "cre3.tim"), 0);
	Gfx_LoadTex(&menu.tex_cre4, Archive_Find(menu_arc, "cre4.tim"), 0);
	Gfx_LoadTex(&menu.tex_cre5, Archive_Find(menu_arc, "cre5.tim"), 0);
	Gfx_LoadTex(&menu.tex_cre6, Archive_Find(menu_arc, "cre6.tim"), 0);
	Mem_Free(menu_arc);
	
	FontData_Load(&menu.font_bold, Font_Bold);
	FontData_Load(&menu.font_arial, Font_Arial);
	
	menu.mouset = Char_mouset_New(FIXED_DEC(69,1), FIXED_DEC(90,1));
	stage.camera.x = stage.camera.y = FIXED_DEC(0,1);
	stage.camera.bzoom = FIXED_UNIT;
	stage.gf_speed = 4;
	
	//Initialize menu state
	menu.select = menu.next_select = 0;
	
	switch (menu.page = menu.next_page = page)
	{
		case MenuPage_Opening:
			//Get funny message to use
			//Do this here so timing is less reliant on VSync
			#ifdef PSXF_PC
				menu.page_state.opening.funny_message = time(NULL) % COUNT_OF(funny_messages);
			#else
				menu.page_state.opening.funny_message = ((*((volatile u32*)0xBF801120)) >> 3) % COUNT_OF(funny_messages); //sysclk seeding
			#endif
			break;
		default:
			break;
	}
	menu.page_swap = true;
	
	menu.trans_time = 0;
	Trans_Clear();
	
	stage.song_step = 0;
	
	//Play menu music
	Audio_PlayXA_Track(XA_GettinFreaky, 0x40, 0, 1);
	Audio_WaitPlayXA();
	
	//Set background colour
	Gfx_SetClear(0, 0, 0);
}

void Menu_Unload(void)
{
	//Free title Girlfriend
	Character_Free(menu.mouset);
}

void Menu_ToStage(StageId id, StageDiff diff, boolean story)
{
	menu.next_page = MenuPage_Stage;
	menu.page_param.stage.id = id;
	menu.page_param.stage.story = story;
	menu.page_param.stage.diff = diff;
	Trans_Start();
}

void Menu_Tick(void)
{
	//Clear per-frame flags
	stage.flag &= ~STAGE_FLAG_JUST_STEP;
	
	//Get song position
	u16 next_step = Audio_TellXA_Milli() / 147; //100 BPM
	if (next_step != stage.song_step)
	{
		if (next_step >= stage.song_step)
			stage.flag |= STAGE_FLAG_JUST_STEP;
		stage.song_step = next_step;
	}
	
	//Handle transition out
	if (Trans_Tick())
	{
		//Change to set next page
		menu.page_swap = true;
		menu.page = menu.next_page;
		menu.select = menu.next_select;
	}
	
	//Tick menu page
	MenuPage exec_page;
	switch (exec_page = menu.page)
	{
		case MenuPage_Opening:
		{
			u16 beat = stage.song_step >> 2;
			
			//Start title screen if opening ended
			if (beat >= 16)
			{
				menu.page = menu.next_page = MenuPage_Title;
				menu.page_swap = true;
				//Fallthrough
			}
			else
			{
				//Start title screen if start pressed
				if (pad_state.held & PAD_START)
					menu.page = menu.next_page = MenuPage_Title;
				
				//Draw different text depending on beat
				const char **funny_message = funny_messages[menu.page_state.opening.funny_message];
				
				switch (beat)
				{
				case 3:
						menu.font_bold.draw(&menu.font_bold, "PRESENT", SCREEN_WIDTH2, SCREEN_HEIGHT2 + 16, FontAlign_Center);
				//Fallthrough
					case 2:
					case 1:
						menu.font_bold.draw(&menu.font_bold, "LORD SCOUT",   SCREEN_WIDTH2, SCREEN_HEIGHT2 - 32, FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "IGORSOU", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 16, FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "UNSTOPABLE",   SCREEN_WIDTH2, SCREEN_HEIGHT2,      FontAlign_Center);
						break;
					
					case 7:
						menu.font_bold.draw(&menu.font_bold, "MOUSE",    SCREEN_WIDTH2, SCREEN_HEIGHT2 - 32, FontAlign_Center);
				//Fallthrough
					case 6:
					case 5:
						menu.font_bold.draw(&menu.font_bold, "IN ASSOCIATION", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 64, FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "WITH",           SCREEN_WIDTH2, SCREEN_HEIGHT2 - 48, FontAlign_Center);
						break;
					
					case 11:
						menu.font_bold.draw(&menu.font_bold, funny_message[1], SCREEN_WIDTH2, SCREEN_HEIGHT2, FontAlign_Center);
				//Fallthrough
					case 10:
					case 9:
						menu.font_bold.draw(&menu.font_bold, funny_message[0], SCREEN_WIDTH2, SCREEN_HEIGHT2 - 16, FontAlign_Center);
						break;
					
					case 15:
						menu.font_bold.draw(&menu.font_bold, "AVI", SCREEN_WIDTH2, SCREEN_HEIGHT2 + 8, FontAlign_Center);
				//Fallthrough
					case 14:
						menu.font_bold.draw(&menu.font_bold, "dot", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 8, FontAlign_Center);
				//Fallthrough
					case 13:
						menu.font_bold.draw(&menu.font_bold, "MOUSE", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 24, FontAlign_Center);
						break;
				}
				break;
			}
		}
	//Fallthrough
		case MenuPage_Title:
		{
			//Initialize page
			if (menu.page_swap)
			{
				menu.page_state.title.logo_bump = (FIXED_DEC(7,1) / 24) - 1;
				menu.page_state.title.fade = FIXED_DEC(255,1);
				menu.page_state.title.fadespd = FIXED_DEC(90,1);
			}
			
			//Draw white fade
			if (menu.page_state.title.fade > 0)
			{
				static const RECT flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
				u8 flash_col = menu.page_state.title.fade >> FIXED_SHIFT;
				Gfx_BlendRect(&flash, flash_col, flash_col, flash_col, 1);
				menu.page_state.title.fade -= FIXED_MUL(menu.page_state.title.fadespd, timer_dt);
			}
			
			//Go to main menu when start is pressed
			if (menu.trans_time > 0 && (menu.trans_time -= timer_dt) <= 0)
				Trans_Start();
			
			if ((pad_state.press & PAD_START) && menu.next_page == menu.page && Trans_Idle() && opening == 0)
			{
				menu.trans_time = FIXED_UNIT;
				menu.page_state.title.fade = FIXED_DEC(255,1);
				menu.page_state.title.fadespd = FIXED_DEC(300,1);
				menu.next_page = MenuPage_Warning;
				menu.next_select = 0;
			}

			if ((pad_state.press & PAD_START) && menu.next_page == menu.page && Trans_Idle() && opening >= 1)
			{
				menu.trans_time = FIXED_UNIT;
				menu.page_state.title.fade = FIXED_DEC(255,1);
				menu.page_state.title.fadespd = FIXED_DEC(300,1);
				menu.next_page = MenuPage_Main;
				menu.next_select = 0;
			}
			
			//Draw Friday Night Funkin' logo
			if ((stage.flag & STAGE_FLAG_JUST_STEP) && (stage.song_step & 0x3) == 0 && menu.page_state.title.logo_bump == 0)
				menu.page_state.title.logo_bump = (FIXED_DEC(7,1) / 24) - 1;
			
			static const fixed_t logo_scales[] = {
				FIXED_DEC(1,1),
				FIXED_DEC(101,100),
				FIXED_DEC(102,100),
				FIXED_DEC(103,100),
				FIXED_DEC(105,100),
				FIXED_DEC(110,100),
				FIXED_DEC(97,100),
			};
			fixed_t logo_scale = logo_scales[(menu.page_state.title.logo_bump * 24) >> FIXED_SHIFT];
			u32 x_rad = (logo_scale * (230 >> 1)) >> FIXED_SHIFT;
			u32 y_rad = (logo_scale * (105 >> 1)) >> FIXED_SHIFT;
			
			RECT logo_src = {0, 0, 176, 112};
			RECT logo_dst = {
				100 - x_rad + (SCREEN_WIDEADD2 >> 1),
				68 - y_rad,
				x_rad << 1,
				y_rad << 1
			};
			Gfx_DrawTex(&menu.tex_title, &logo_src, &logo_dst);
			
			if (menu.page_state.title.logo_bump > 0)
				if ((menu.page_state.title.logo_bump -= timer_dt) < 0)
					menu.page_state.title.logo_bump = 0;
			
			//Draw "Press Start to Begin"
			if (menu.next_page == menu.page)
			{
				//Blinking blue
				s16 press_lerp = (MUtil_Cos(animf_count << 3) + 0x100) >> 1;
				u8 press_r = 51 >> 1;
				u8 press_g = (58  + ((press_lerp * (255 - 58))  >> 8)) >> 1;
				u8 press_b = (206 + ((press_lerp * (255 - 206)) >> 8)) >> 1;
				
				RECT press_src = {0, 112, 256, 32};
				Gfx_BlitTexCol(&menu.tex_title, &press_src, (SCREEN_WIDTH - 256) / 2, SCREEN_HEIGHT - 48, press_r, press_g, press_b);
			}
			else
			{
				//Flash white
				RECT press_src = {0, (animf_count & 1) ? 144 : 112, 256, 32};
				Gfx_BlitTex(&menu.tex_title, &press_src, (SCREEN_WIDTH - 256) / 2, SCREEN_HEIGHT - 48);
			}

			//Draw Girlfriend
			menu.mouset->tick(menu.mouset);
			break;
		}
		case MenuPage_Warning:
        //Draw different text depending on beat

				if (pad_state.held & PAD_R2)
				{   
					menu.next_page = MenuPage_Main;
					menu.next_select = 0; //Story Mode
					opening++;
					Trans_Start();
				}

				if (pad_state.held & PAD_L2)
				{   
					menu.next_page = MenuPage_Main;
					menu.next_select = 0; //Story Mode
					stage.shake = false;
					Trans_Start();
					opening++;
				}

				if (pad_state.held & PAD_START)
				{   
					menu.next_page = MenuPage_Main;
					menu.next_select = 0; //Story Mode
					Trans_Start();
					opening++;
				}

				RECT warning_src = {0, 0, 256, 256,};
				Gfx_BlitTex(&menu.tex_test, &warning_src, (SCREEN_WIDTH - 256) / 2, SCREEN_HEIGHT - 256);
				break;


		case MenuPage_Main:
		{
			static const char *menu_options[] = {
				"STORY MODE",
				"FREEPLAY",
				"CREDITS",
				"OPTIONS",
				#ifdef PSXF_NETWORK
					"JOIN SERVER",
					"HOST SERVER",
				#endif
			};
			
			//Initialize page
			if (menu.page_swap)
				menu.scroll = menu.select *
				#ifndef PSXF_NETWORK
					FIXED_DEC(8,1);
				#else
					FIXED_DEC(12,1);
				#endif
			
			//Draw version identification
			menu.font_bold.draw(&menu.font_bold,
				"PSXFUNKIN BY CUCKYDEV",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Handle option and selection
			if (menu.trans_time > 0 && (menu.trans_time -= timer_dt) <= 0)
				Trans_Start();
			
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					switch (menu.select)
					{

						case 0: //Story Mode
							menu.next_page = MenuPage_Story;
							break;
						case 1: //Freeplay
							menu.next_page = MenuPage_Freeplay;
							break;
						case 2: //Mods
							menu.next_page = MenuPage_Mods;
							break;
						case 3: //Options
							menu.next_page = MenuPage_Options;
							break;
					#ifdef PSXF_NETWORK
						case 4: //Join Server
							menu.next_page = Network_Inited() ? MenuPage_NetJoin : MenuPage_NetInitFail;
							break;
						case 5: //Host Server
							menu.next_page = Network_Inited() ? MenuPage_NetHost : MenuPage_NetInitFail;
							break;
					#endif
					}
					menu.next_select = 0;
					menu.trans_time = FIXED_UNIT;
				}
				
				//Return to title screen if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					menu.next_page = MenuPage_Title;
					Trans_Start();
				}
			}
			
			//Draw options
			s32 next_scroll = menu.select *
			#ifndef PSXF_NETWORK
				FIXED_DEC(8,1);
			#else
				FIXED_DEC(12,1);
			#endif
			menu.scroll += (next_scroll - menu.scroll) >> 2;
			
			if (menu.next_page == menu.page || menu.next_page == MenuPage_Title)
			{
				//Draw all options
				for (u8 i = 0; i < COUNT_OF(menu_options); i++)
				{
					menu.font_bold.draw(&menu.font_bold,
						Menu_LowerIf(menu_options[i], menu.select != i),
						SCREEN_WIDTH2,
						SCREEN_HEIGHT2 + (i << 5) - 48 - (menu.scroll >> FIXED_SHIFT),
						FontAlign_Center
					);
				}
			}
			else if (animf_count & 2)
			{
				//Draw selected option
				menu.font_bold.draw(&menu.font_bold,
					menu_options[menu.select],
					SCREEN_WIDTH2,
					SCREEN_HEIGHT2 + (menu.select << 5) - 48 - (menu.scroll >> FIXED_SHIFT),
					FontAlign_Center
				);
			}
			
			//Draw background
			Menu_DrawBack(
				menu.next_page == menu.page || menu.next_page == MenuPage_Title,
			#ifndef PSXF_NETWORK
				menu.scroll >> (FIXED_SHIFT + 1),
			#else
				menu.scroll >> (FIXED_SHIFT + 3),
			#endif
				253 >> 1, 231 >> 1, 113 >> 1,
				253 >> 1, 113 >> 1, 155 >> 1
			);
			break;
		}


		case MenuPage_Story:
		{
			static const struct
			{
				const char *week;
				StageId stage;
				const char *name;
				const char *tracks[3];
			} menu_options[] = {
				{"1", StageId_1_1, "THE SIGHTS OF HELL", {"UNHAPPY", "HAPPY", "REALLY HAPPY"}},
			};
			
			//Initialize page
			if (menu.page_swap)
			{
				menu.scroll = 0;
				menu.page_param.stage.diff = StageDiff_Normal;
				menu.page_state.title.fade = FIXED_DEC(0,1);
				menu.page_state.title.fadespd = FIXED_DEC(0,1);
			}
			
			//Draw white fade
			if (menu.page_state.title.fade > 0)
			{
				static const RECT flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
				u8 flash_col = menu.page_state.title.fade >> FIXED_SHIFT;
				Gfx_BlendRect(&flash, flash_col, flash_col, flash_col, 1);
				menu.page_state.title.fade -= FIXED_MUL(menu.page_state.title.fadespd, timer_dt);
			}
			
			//Draw difficulty selector
			Menu_DifficultySelector(SCREEN_WIDTH - 75, 80);
			
			//Handle option and selection
			if (menu.trans_time > 0 && (menu.trans_time -= timer_dt) <= 0)
				Trans_Start();
			
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					menu.next_page = MenuPage_Stage;
					menu.page_param.stage.id = menu_options[menu.select].stage;
					menu.page_param.stage.story = true;
					menu.trans_time = FIXED_UNIT;
					menu.page_state.title.fade = FIXED_DEC(255,1);
					menu.page_state.title.fadespd = FIXED_DEC(510,1);
				}
				
				//Return to main menu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					menu.next_page = MenuPage_Main;
					menu.next_select = 0; //Story Mode
					Trans_Start();
				}
			}
			
			//Draw week name and tracks
			menu.font_bold.draw(&menu.font_bold,
				menu_options[menu.select].name,
				SCREEN_WIDTH - 16,
				24,
				FontAlign_Right
			);
			
			const char * const *trackp = menu_options[menu.select].tracks;
			for (size_t i = 0; i < COUNT_OF(menu_options[menu.select].tracks); i++, trackp++)
			{
				if (*trackp != NULL)
					menu.font_bold.draw(&menu.font_bold,
						*trackp,
						SCREEN_WIDTH - 16,
						SCREEN_HEIGHT - (4 * 24) + (i * 24),
						FontAlign_Right
					);
			}
			
			//Draw upper strip
			RECT name_bar = {0, 16, SCREEN_WIDTH, 32};
			Gfx_DrawRect(&name_bar, 249, 207, 81);
			
			//Draw options
			s32 next_scroll = menu.select * FIXED_DEC(48,1);
			menu.scroll += (next_scroll - menu.scroll) >> 3;
			
			if (menu.next_page == menu.page || menu.next_page == MenuPage_Main)
			{
				//Draw all options
				for (u8 i = 0; i < COUNT_OF(menu_options); i++)
				{
					s32 y = 64 + (i * 48) - (menu.scroll >> FIXED_SHIFT);
					if (y <= 16)
						continue;
					if (y >= SCREEN_HEIGHT)
						break;
					Menu_DrawWeek(menu_options[i].week, 48, y);
				}
			}
			else if (animf_count & 2)
			{
				//Draw selected option
				Menu_DrawWeek(menu_options[menu.select].week, 48, 64 + (menu.select * 48) - (menu.scroll >> FIXED_SHIFT));
			}
			
			break;
		}
		case MenuPage_Freeplay:
		{
			static const struct
			{
				StageId stage;
				u32 col;
				const char *text;
			} menu_options[] = {
				//{StageId_4_4, "TEST"},
				{StageId_1_1, 0xFFBFBEBB, "UNHAPPY"},
				{StageId_1_2, 0xFF9E9D9B, "HAPPY"},
				{StageId_1_3, 0xFF6E6D6B, "REALLY HAPPY"},
				{StageId_1_4, 0xFFEDEDED, "SMILE"},
				{StageId_2_1, 0xFFBFBEBB, "VERY UNHAPPY"},
				{StageId_2_2, 0xFF6E6D6B, "REALLY HAPPY FANMADE"},
				{StageId_2_3, 0xFF6E6D6B, "REALLY HAPPY BABY MODE"},
			};
			
			//Initialize page
			if (menu.page_swap)
			{
				menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
				menu.page_param.stage.diff = StageDiff_Normal;
				menu.page_state.freeplay.back_r = FIXED_DEC(255,1);
				menu.page_state.freeplay.back_g = FIXED_DEC(255,1);
				menu.page_state.freeplay.back_b = FIXED_DEC(255,1);
			}
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"FREEPLAY",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Draw difficulty selector
			Menu_DifficultySelector(SCREEN_WIDTH - 100, SCREEN_HEIGHT2 - 48);
			
			//Handle option and selection
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					menu.next_page = MenuPage_Stage;
					menu.page_param.stage.id = menu_options[menu.select].stage;
					menu.page_param.stage.story = false;
					Trans_Start();
				}
				
				//Return to main menu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					menu.next_page = MenuPage_Main;
					menu.next_select = 1; //Freeplay
					Trans_Start();
				}
			}
			
			//Draw options
			s32 next_scroll = menu.select * FIXED_DEC(24,1);
			menu.scroll += (next_scroll - menu.scroll) >> 4;
			
			for (u8 i = 0; i < COUNT_OF(menu_options); i++)
			{
				//Get position on screen
				s32 y = (i * 24) - 8 - (menu.scroll >> FIXED_SHIFT);
				if (y <= -SCREEN_HEIGHT2 - 8)
					continue;
				if (y >= SCREEN_HEIGHT2 + 8)
					break;
				
				//Draw text
				menu.font_bold.draw(&menu.font_bold,
					Menu_LowerIf(menu_options[i].text, menu.select != i),
					48 + (y >> 2),
					SCREEN_HEIGHT2 + y - 8,
					FontAlign_Left
				);
			}
			
			//Draw background
			fixed_t tgt_r = (fixed_t)((menu_options[menu.select].col >> 16) & 0xFF) << FIXED_SHIFT;
			fixed_t tgt_g = (fixed_t)((menu_options[menu.select].col >>  8) & 0xFF) << FIXED_SHIFT;
			fixed_t tgt_b = (fixed_t)((menu_options[menu.select].col >>  0) & 0xFF) << FIXED_SHIFT;

			menu.page_state.freeplay.back_r += (tgt_r - menu.page_state.freeplay.back_r) >> 4;
			menu.page_state.freeplay.back_g += (tgt_g - menu.page_state.freeplay.back_g) >> 4;
			menu.page_state.freeplay.back_b += (tgt_b - menu.page_state.freeplay.back_b) >> 4;
			Menu_DrawBack(
				true,
				8,
				menu.page_state.freeplay.back_r >> (FIXED_SHIFT + 1),
				menu.page_state.freeplay.back_g >> (FIXED_SHIFT + 1),
				menu.page_state.freeplay.back_b >> (FIXED_SHIFT + 1),
				0, 0, 0
			);
			break;
		}
		case MenuPage_Mods:
		{
			static u16 change = 1;
        
			    //Swap to Main if credits ended
			  {
				if (change > 17)
				menu.page = menu.next_page = MenuPage_Main;
				menu.page_swap = true;	

		 	    }

		
				  if (change > 17)
				  change = 1;

			    //Fallthrough
			{
				//Swap Main if circle pressed
			
				if (pad_state.press & PAD_CIRCLE)
				 {
				  menu.page = menu.next_page = MenuPage_Main;
				  menu.page_swap = true;
					(change) = 1;
				 }
                 //Draw different text 
				RECT src_cre0 = {0, 0, 128, 128};
				RECT src_cre1 = {0, 0, 128, 128};
				RECT src_cre2 = {0, 0, 128, 128};
				RECT src_cre3 = {0, 0, 128, 128};
				RECT src_cre4 = {0, 0, 128, 128};
				RECT src_cre5 = {0,128, 128, 128};
				RECT src_cre6 = {0, 0, 128, 128};

				 if (pad_state.press & (PAD_START | PAD_CROSS))
			    change++;

				//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"CREDITS",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);

				switch (change)
					{
					case 2:

						menu.font_bold.draw(&menu.font_bold, "cuckydev", SCREEN_WIDTH2, SCREEN_HEIGHT2 -48,  FontAlign_Center);
						Gfx_BlitTex(&menu.tex_cre0, &src_cre0, (SCREEN_WIDTH - 128) >> 1, SCREEN_HEIGHT2 - 18);
				//Fallthrough
					case 1:
					{
						menu.font_bold.draw(&menu.font_bold, "PSXFUNKIN",   SCREEN_WIDTH2, SCREEN_HEIGHT2 - 80, FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "BY", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 64, FontAlign_Center);
						break;
					}
					
					case 4:
					    menu.font_bold.draw(&menu.font_bold, "lord scout", SCREEN_WIDTH2, SCREEN_HEIGHT2 -48 , FontAlign_Center);
						Gfx_BlitTex(&menu.tex_cre1, &src_cre1, (SCREEN_WIDTH - 128) >> 1, SCREEN_HEIGHT2 - 18);
						
				    
					
				//Fallthrough
					case 3:
						menu.font_bold.draw(&menu.font_bold, "PORTED", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 80, FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "BY",           SCREEN_WIDTH2, SCREEN_HEIGHT2 - 64, FontAlign_Center);
						break;

				   case 6:
				        Gfx_BlitTex(&menu.tex_cre2, &src_cre2, (SCREEN_WIDTH - 128) >> 1, SCREEN_HEIGHT2 - 18);
					 
					case 5:
						menu.font_bold.draw(&menu.font_bold, "igorsou",    SCREEN_WIDTH2, SCREEN_HEIGHT2 -48 , FontAlign_Center);
						break;
                    
					 case 8:
					       Gfx_BlitTex(&menu.tex_cre3, &src_cre3, (SCREEN_WIDTH - 128) >> 1, SCREEN_HEIGHT2 - 18);


					case 7:
						menu.font_bold.draw(&menu.font_bold, "unstopable", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 48, FontAlign_Center);
						break;
					 
					 case 10:
					    menu.font_bold.draw(&menu.font_bold, "john paul", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 48, FontAlign_Center);
						Gfx_BlitTex(&menu.tex_cre4, &src_cre4, (SCREEN_WIDTH - 128) >> 1, SCREEN_HEIGHT2 - 18);

					case 9:
						menu.font_bold.draw(&menu.font_bold, "PLAYTEST", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 64, FontAlign_Center);
						break;
					
					 case 12:
                	    menu.font_bold.draw(&menu.font_bold, "maymaysdays", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 48, FontAlign_Center);
						Gfx_BlitTex(&menu.tex_cre6, &src_cre6, (SCREEN_WIDTH - 128) >> 1, SCREEN_HEIGHT2 - 18);

					case 11:
						menu.font_bold.draw(&menu.font_bold, "ORIGINAL CREATORS", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 64, FontAlign_Center);
						break;
		
						case 14:
						Gfx_BlitTex(&menu.tex_cre5, &src_cre5, (SCREEN_WIDTH - 128) >> 1, SCREEN_HEIGHT2 - 18);
				
					
						case 13:
						menu.font_bold.draw(&menu.font_bold, "sonicboiforlife", SCREEN_WIDTH2, SCREEN_HEIGHT2 -48, FontAlign_Center);
						break;

						case 15:
						menu.font_bold.draw(&menu.font_bold, "TUTORIALING", SCREEN_WIDTH2, SCREEN_HEIGHT2 -48, FontAlign_Center);
						break;

						case 17:
						menu.font_bold.draw(&menu.font_bold, "miapaisano", SCREEN_WIDTH2, SCREEN_HEIGHT2 -48, FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "zeriben", SCREEN_WIDTH2, SCREEN_HEIGHT2 -32, FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "discord server", SCREEN_WIDTH2, SCREEN_HEIGHT2 -16, FontAlign_Center);

						case 16:
						menu.font_bold.draw(&menu.font_bold, "SPECIAL THANKS", SCREEN_WIDTH2, SCREEN_HEIGHT2 -80, FontAlign_Center);
						break;
				}
				break;
			

			}
		}
		case MenuPage_Options:
		{
			static const char *gamemode_strs[] = {"NORMAL", "SWAP", "TWO PLAYER"};
			static const struct
			{
				enum
				{
					OptType_Boolean,
					OptType_Enum,
				} type;
				const char *text;
				void *value;
				union
				{
					struct
					{
						int a;
					} spec_boolean;
					struct
					{
						s32 max;
						const char **strs;
					} spec_enum;
				} spec;
			} menu_options[] = {
				{OptType_Enum,    "GAMEMODE", &stage.mode, {.spec_enum = {COUNT_OF(gamemode_strs), gamemode_strs}}},
				//{OptType_Boolean, "INTERPOLATION", &stage.expsync},
				{OptType_Boolean, "GHOST TAP ", &stage.ghost, {.spec_boolean = {0}}},
				{OptType_Boolean, "DOWNSCROLL", &stage.downscroll, {.spec_boolean = {0}}},
				{OptType_Boolean, "SCREEN SHAKE", &stage.shake, {.spec_boolean = {0}}},
			};
			
			//Initialize page
			if (menu.page_swap)
				menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"OPTIONS",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Handle option and selection
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Handle option changing
				switch (menu_options[menu.select].type)
				{
					case OptType_Boolean:
						if (pad_state.press & (PAD_CROSS | PAD_LEFT | PAD_RIGHT))
							*((boolean*)menu_options[menu.select].value) ^= 1;
						break;
					case OptType_Enum:
						if (pad_state.press & PAD_LEFT)
							if (--*((s32*)menu_options[menu.select].value) < 0)
								*((s32*)menu_options[menu.select].value) = menu_options[menu.select].spec.spec_enum.max - 1;
						if (pad_state.press & PAD_RIGHT)
							if (++*((s32*)menu_options[menu.select].value) >= menu_options[menu.select].spec.spec_enum.max)
								*((s32*)menu_options[menu.select].value) = 0;
						break;
				}
				
				//Return to main menu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					menu.next_page = MenuPage_Main;
					menu.next_select = 3; //Options
					Trans_Start();
				}
			}
			
			//Draw options
			s32 next_scroll = menu.select * FIXED_DEC(24,1);
			menu.scroll += (next_scroll - menu.scroll) >> 4;
			
			for (u8 i = 0; i < COUNT_OF(menu_options); i++)
			{
				//Get position on screen
				s32 y = (i * 24) - 8 - (menu.scroll >> FIXED_SHIFT);
				if (y <= -SCREEN_HEIGHT2 - 8)
					continue;
				if (y >= SCREEN_HEIGHT2 + 8)
					break;
				
				//Draw text
				char text[0x80];
				switch (menu_options[i].type)
				{
					case OptType_Boolean:
						sprintf(text, "%s %s", menu_options[i].text, *((boolean*)menu_options[i].value) ? "ON" : "OFF");
						break;
					case OptType_Enum:
						sprintf(text, "%s %s", menu_options[i].text, menu_options[i].spec.spec_enum.strs[*((s32*)menu_options[i].value)]);
						break;
				}
				menu.font_bold.draw(&menu.font_bold,
					Menu_LowerIf(text, menu.select != i),
					48 + (y >> 2),
					SCREEN_HEIGHT2 + y - 8,
					FontAlign_Left
				);
			}
			
			//Draw background
			Menu_DrawBack(
				true,
				8,
				253 >> 1, 113 >> 1, 155 >> 1,
				0, 0, 0
			);
			break;
		}
	#ifdef PSXF_NETWORK
		case MenuPage_NetHost:
		{
			const size_t menu_options = 3;
			
			//Initialize page
			if (menu.page_swap)
			{
				menu.page_state.net_host.type = false;
				menu.page_state.net_host.port[0] = '\0';
				menu.page_state.net_host.pass[0] = '\0';
			}
			
			//Handle option and selection
			if (menu.next_page == menu.page && Trans_Idle())
			{
				if (!menu.page_state.net_host.type)
				{
					//Change option
					if (pad_state.press & PAD_UP)
					{
						if (menu.select > 0)
							menu.select--;
						else
							menu.select = menu_options - 1;
					}
					if (pad_state.press & PAD_DOWN)
					{
						if (menu.select < menu_options - 1)
							menu.select++;
						else
							menu.select = 0;
					}
					
					//Handle selection when cross is pressed
					if (pad_state.press & (PAD_START | PAD_CROSS))
					{
						switch (menu.select)
						{
							case 0: //Port
							case 1: //Pass
								menu.page_state.net_host.type = true;
								break;
							case 2: //Host
								if (!Network_HostPort(menu.page_state.net_host.port, menu.page_state.net_host.pass))
								{
									menu.next_page = MenuPage_NetOpWait;
									menu.next_select = 0;
									Trans_Start();
								}
								break;
						}
					}
					
					//Return to main menu if circle is pressed
					if (pad_state.press & PAD_CIRCLE)
					{
						menu.next_page = MenuPage_Main;
						menu.next_select = 5; //Host Server
						Trans_Start();
					}
				}
				else
				{
					//Stop typing when start is pressed
					if (pad_state.press & PAD_START)
					{
						switch (menu.select)
						{
							case 0: //Port
							case 1: //Pass
								menu.page_state.net_host.type = false;
								break;
						}
					}
				}
			}
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"HOST SERVER",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Draw options
			MenuStr_Process(&menu.page_state.net_host.port, 64 + 3 * 0, 64 + 16 * 0, "Port: %s", menu.select == 0, menu.page_state.net_host.type);
			MenuStr_Process(&menu.page_state.net_host.pass, 64 + 3 * 1, 64 + 16 * 1, "Pass: %s", menu.select == 1, menu.page_state.net_host.type);
			menu.font_bold.draw(&menu.font_bold, Menu_LowerIf("HOST", menu.select != 2), 64 + 3 * 2, 64 + 16 * 2, FontAlign_Left);
			
			//Draw background
			Menu_DrawBack(
				true,
				8,
				146 >> 1, 113 >> 1, 253 >> 1,
				0, 0, 0
			);
			break;
		}
		case MenuPage_NetJoin:
		{
			const size_t menu_options = 4;
			
			//Initialize page
			if (menu.page_swap)
			{
				menu.page_state.net_join.type = false;
				menu.page_state.net_join.ip[0] = '\0';
				menu.page_state.net_join.port[0] = '\0';
				menu.page_state.net_join.pass[0] = '\0';
			}
			
			//Handle option and selection
			if (menu.next_page == menu.page && Trans_Idle())
			{
				if (!menu.page_state.net_join.type)
				{
					//Change option
					if (pad_state.press & PAD_UP)
					{
						if (menu.select > 0)
							menu.select--;
						else
							menu.select = menu_options - 1;
					}
					if (pad_state.press & PAD_DOWN)
					{
						if (menu.select < menu_options - 1)
							menu.select++;
						else
							menu.select = 0;
					}
					
					//Handle selection when cross is pressed
					if (pad_state.press & (PAD_START | PAD_CROSS))
					{
						switch (menu.select)
						{
							case 0: //Ip
							case 1: //Port
							case 2: //Pass
								menu.page_state.net_join.type = true;
								break;
							case 3: //Join
								if (!Network_Join(menu.page_state.net_join.ip, menu.page_state.net_join.port, menu.page_state.net_join.pass))
								{
									menu.next_page = MenuPage_NetConnect;
									menu.next_select = 0;
									Trans_Start();
								}
								break;
						}
					}
					
					//Return to main menu if circle is pressed
					if (pad_state.press & PAD_CIRCLE)
					{
						menu.next_page = MenuPage_Main;
						menu.next_select = 4; //Join Server
						Trans_Start();
					}
				}
				else
				{
					//Stop typing when start is pressed
					if (pad_state.press & PAD_START)
					{
						switch (menu.select)
						{
							case 0: //Join
							case 1: //Port
							case 2: //Pass
								menu.page_state.net_join.type = false;
								break;
						}
					}
				}
			}
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"JOIN SERVER",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Draw options
			MenuStr_Process(&menu.page_state.net_join.ip, 64 + 3 * 0, 64 + 16 * 0, "Address: %s", menu.select == 0, menu.page_state.net_join.type);
			MenuStr_Process(&menu.page_state.net_join.port, 64 + 3 * 1, 64 + 16 * 1, "Port: %s", menu.select == 1, menu.page_state.net_join.type);
			MenuStr_Process(&menu.page_state.net_join.pass, 64 + 3 * 2, 64 + 16 * 2, "Pass: %s", menu.select == 2, menu.page_state.net_join.type);
			menu.font_bold.draw(&menu.font_bold, Menu_LowerIf("JOIN", menu.select != 3), 64 + 3 * 3, 64 + 16 * 3, FontAlign_Left);
			
			//Draw background
			Menu_DrawBack(
				true,
				8,
				146 >> 1, 113 >> 1, 253 >> 1,
				0, 0, 0
			);
			break;
		}
		case MenuPage_NetConnect:
		{
			//Change state according to network state
			if (menu.next_page == menu.page && Trans_Idle())
			{
				if (!Network_Connected())
				{
					//Disconnected
					menu.next_page = MenuPage_NetFail;
					menu.next_select = 0;
					Trans_Start();
				}
				else if (Network_Allowed())
				{
					//Allowed to join
					menu.next_page = MenuPage_NetLobby;
					menu.next_select = 0;
					Trans_Start();
				}
			}
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"CONNECTING",
				SCREEN_WIDTH2,
				SCREEN_HEIGHT2 - 8,
				FontAlign_Center
			);
			
			//Draw background
			Menu_DrawBack(
				true,
				8,
				113 >> 1, 146 >> 1, 253 >> 1,
				0, 0, 0
			);
			break;
		}
		case MenuPage_NetOpWait:
		{
			//Change state according to network state
			if (menu.next_page == menu.page && Trans_Idle())
			{
				if (!Network_Connected())
				{
					//Disconnected
					menu.next_page = MenuPage_NetFail;
					menu.next_select = 0;
					Trans_Start();
				}
				else if (Network_HasPeer())
				{
					//Peer has joined
					menu.next_page = MenuPage_NetOp;
					menu.next_select = 0;
					Trans_Start();
				}
			}
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"WAITING FOR PEER",
				SCREEN_WIDTH2,
				SCREEN_HEIGHT2 - 8,
				FontAlign_Center
			);
			
			//Draw background
			Menu_DrawBack(
				true,
				8,
				113 >> 1, 146 >> 1, 253 >> 1,
				0, 0, 0
			);
			break;
		}
		case MenuPage_NetOp:
		{
			static const struct
			{
				boolean diff;
				StageId stage;
				const char *text;
			} menu_options[] = {
				//{StageId_4_4, "TEST"},
				{true,  StageId_1_4, "TUTORIAL"},
				{true,  StageId_1_1, "BOPEEBO"},
				{true,  StageId_1_2, "FRESH"},
				{true,  StageId_1_3, "DADBATTLE"},
				{true,  StageId_2_1, "SPOOKEEZ"},
				{true,  StageId_2_2, "SOUTH"},
				{true,  StageId_2_3, "MONSTER"},
				{true,  StageId_3_1, "PICO"},
				{true,  StageId_3_2, "PHILLY NICE"},
				{true,  StageId_3_3, "BLAMMED"},
				{true,  StageId_4_1, "SATIN PANTIES"},
				{true,  StageId_4_2, "HIGH"},
				{true,  StageId_4_3, "MILF"},
				{true,  StageId_5_1, "COCOA"},
				{true,  StageId_5_2, "EGGNOG"},
				{true,  StageId_5_3, "WINTER HORRORLAND"},
				{true,  StageId_6_1, "SENPAI"},
				{true,  StageId_6_2, "ROSES"},
				{true,  StageId_6_3, "THORNS"},
				{true,  StageId_7_1, "UGH"},
				{true,  StageId_7_2, "GUNS"},
				{true,  StageId_7_3, "STRESS"},
				{false, StageId_Kapi_1, "WOCKY"},
				{false, StageId_Kapi_2, "BEATHOVEN"},
				{false, StageId_Kapi_3, "HAIRBALL"},
				{false, StageId_Kapi_4, "NYAW"},
				{true,  StageId_Clwn_1, "IMPROBABLE OUTSET"},
				{true,  StageId_Clwn_2, "MADNESS"},
				{true,  StageId_Clwn_3, "HELLCLOWN"},
				{false, StageId_Clwn_4, "EXPURGATION"},
				{false, StageId_2_4, "CLUCKED"},
			};
			
			//Initialize page
			if (menu.page_swap)
			{
				menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
				menu.page_param.stage.diff = StageDiff_Normal;
				menu.page_state.net_op.swap = false;
			}
			
			//Handle option and selection
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Check network state
				if (!Network_Connected())
				{
					//Disconnected
					menu.next_page = MenuPage_NetFail;
					menu.next_select = 0;
					Trans_Start();
				}
				else if (!Network_HasPeer())
				{
					//Peer disconnected
					menu.next_page = MenuPage_NetOpWait;
					menu.next_select = 0;
					Trans_Start();
				}
				
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					//Load stage
					Network_SetReady(false);
					stage.mode = menu.page_state.net_op.swap ? StageMode_Net2 : StageMode_Net1;
					menu.next_page = MenuPage_Stage;
					menu.page_param.stage.id = menu_options[menu.select].stage;
					if (!menu_options[menu.select].diff)
						menu.page_param.stage.diff = StageDiff_Hard;
					menu.page_param.stage.story = false;
					Trans_Start();
				}
				
				//Swap characters if triangle is pressed
				if (pad_state.press & PAD_TRIANGLE)
					menu.page_state.net_op.swap ^= true;
			}
			
			//Draw op controls
			const char *control_txt;
			
			control_txt = menu.page_state.net_op.swap ? "You will be Player 2. Press Triangle to swap." : "You will be Player 1. Press Triangle to swap.";
			menu.font_arial.draw_col(&menu.font_arial, control_txt, 24, SCREEN_HEIGHT - 24 - 12, FontAlign_Left, 0x80, 0x80, 0x80);
			menu.font_arial.draw_col(&menu.font_arial, control_txt, 24 + 1, SCREEN_HEIGHT - 24 - 12 + 1, FontAlign_Left, 0x00, 0x00, 0x00);
			
			//Draw difficulty selector
			if (menu_options[menu.select].diff)
				Menu_DifficultySelector(SCREEN_WIDTH - 100, SCREEN_HEIGHT2 - 48);
			
			//Draw options
			s32 next_scroll = menu.select * FIXED_DEC(24,1);
			menu.scroll += (next_scroll - menu.scroll) >> 4;
			
			for (u8 i = 0; i < COUNT_OF(menu_options); i++)
			{
				//Get position on screen
				s32 y = (i * 24) - 8 - (menu.scroll >> FIXED_SHIFT);
				if (y <= -SCREEN_HEIGHT2 - 8)
					continue;
				if (y >= SCREEN_HEIGHT2 + 8)
					break;
				
				//Draw text
				menu.font_bold.draw(&menu.font_bold,
					Menu_LowerIf(menu_options[i].text, menu.select != i),
					48 + (y >> 2),
					SCREEN_HEIGHT2 + y - 8,
					FontAlign_Left
				);
			}
			
			//Draw background
			Menu_DrawBack(
				true,
				8,
				113 >> 1, 253 >> 1, 146 >> 1,
				0, 0, 0
			);
			break;
		}
		case MenuPage_NetLobby:
		{
			//Check network state
			if (menu.next_page == menu.page && Trans_Idle())
			{
				if (!Network_Connected())
				{
					//Disconnected
					menu.next_page = MenuPage_NetFail;
					menu.next_select = 0;
					Trans_Start();
				}
			}
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"WAITING FOR HOST",
				SCREEN_WIDTH2,
				SCREEN_HEIGHT2 - 8,
				FontAlign_Center
			);
			
			//Draw background
			Menu_DrawBack(
				true,
				8,
				253 >> 1, 146 >> 1, 113 >> 1,
				0, 0, 0
			);
			break;
		}
		case MenuPage_NetFail:
		{
			//Return to main menu if circle or start is pressed
			if (menu.next_page == menu.page && Trans_Idle())
			{
				if (pad_state.press & (PAD_CIRCLE | PAD_START))
				{
					menu.next_page = MenuPage_Main;
					menu.next_select = 0;
					Trans_Start();
				}
			}
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"DISCONNECTED",
				SCREEN_WIDTH2,
				SCREEN_HEIGHT2 - 8,
				FontAlign_Center
			);
			
			//Draw background
			Menu_DrawBack(
				true,
				8,
				253 >> 1, 30 >> 1, 15 >> 1,
				0, 0, 0
			);
			break;
		}
		case MenuPage_NetInitFail:
		{
			//Return to main menu if circle or start is pressed
			if (menu.next_page == menu.page && Trans_Idle())
			{
				if (pad_state.press & (PAD_CIRCLE | PAD_START))
				{
					menu.next_page = MenuPage_Main;
					menu.next_select = 0;
					Trans_Start();
				}
			}
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"WSA INIT FAILED",
				SCREEN_WIDTH2,
				SCREEN_HEIGHT2 - 8,
				FontAlign_Center
			);
			
			//Draw background
			Menu_DrawBack(
				true,
				8,
				253 >> 1, 30 >> 1, 15 >> 1,
				0, 0, 0
			);
			break;
		}
	#endif
		case MenuPage_Stage:
		{
			//Unload menu state
			Menu_Unload();
			
			//Load new stage
			LoadScr_Start();
			Stage_Load(menu.page_param.stage.id, menu.page_param.stage.diff, menu.page_param.stage.story);
			gameloop = GameLoop_Stage;
			LoadScr_End();
			break;
		}
		default:
			break;
	}
	
	//Clear page swap flag
	menu.page_swap = menu.page != exec_page;
}
