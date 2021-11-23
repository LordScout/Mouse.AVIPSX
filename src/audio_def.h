#define XA_LENGTH(x) (((u64)(x) * 75) / 100 * IO_SECT_SIZE) //Centiseconds to sectors in bytes (w)

typedef struct
{
	XA_File file;
	u32 length;
} XA_TrackDef;

static const XA_TrackDef xa_tracks[] = {
	//MENU.XA
	{XA_Menu, XA_LENGTH(4400)}, //XA_GettinFreaky
	{XA_Menu, XA_LENGTH(12100)},  //XA_GameOver
	//WEEK1A.XA
	{XA_Week1A, XA_LENGTH(6900)}, //XA_Unhappy
	{XA_Week1A, XA_LENGTH(12200)}, //XA_Happy
	//WEEK1B.XA
	{XA_Week1B, XA_LENGTH(10900)}, //XA_Really Happy
	{XA_Week1B, XA_LENGTH(8100)}, //XA_Smile
	//WEEKM2.XA
	{XA_WeekM2, XA_LENGTH(8800)}, //XA_Very Unhappy
	{XA_WeekM2, XA_LENGTH(8300)}, //XA_Really Happy F
};

static const char *xa_paths[] = {
	"\\MUSIC\\MENU.XA;1",   //XA_Menu
	"\\MUSIC\\WEEK1A.XA;1", //XA_Week1A
	"\\MUSIC\\WEEK1B.XA;1", //XA_Week1B
	"\\MUSIC\\WEEKM2.XA;1", //XA_WeekM2
	NULL,
};

typedef struct
{
	const char *name;
	boolean vocal;
} XA_Mp3;

static const XA_Mp3 xa_mp3s[] = {
	//MENU.XA
	{"freaky", false},   //XA_GettinFreaky
	{"gameover", false}, //XA_GameOver
	//WEEK1A.XA
	{"unhappy", true}, //XA_Unhappy
	{"happy", true},   //XA_Happy
	//WEEK1B.XA
	{"reallyhappy", true}, //XA_Really Happy
	{"smile", true}, //XA_Smile
	//WEEKM2.XA
	{"veryunhappy", true}, //XA_Very Unhappy
	{"reallyhappyf",true}, //XA_Really Happy Fan made
	{NULL, false}
};
