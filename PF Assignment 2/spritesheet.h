#ifndef SPRITESHEET_H
#define SPRITESHEET_H

static const char* spritesheet[128] =
{
	u8"█",		// pillar
	u8"+",		// health pickup
	u8"▒"		// translucent pillar
};

/* this should be formatted as follows :
*	each layout is a multiline string
*	they should be run-length-encoded
*	possible letter codes are:
*		bl - blank
*		sp - solid pillar
*		tp - translucent pillar
*		po - poison
*		gr - grass
*		bp - bomb pickup
*		hp - health pickup
*		wu - weapon upgrade
*		ew - enemy wizard spawnpoint
*		eb - enemy brawler spawnpoint
*		ed - enemy dasher spawnpoint
*		eh - enemy horror spawnpoint
*		er - random enemy spawnpoint
*		fs - fire shooter
*	leave a space between each RLE pair
*	each line should represent a new row
*/
// i probably can't change the room size now :(
// the overall room size is 45x21, however, with two chars on each side that makes it actually 41x17
static const unsigned int num_room_layouts = 3;
static const char* room_layout[16] =
{
	"tp8 sp1 bl23 sp1 tp8\n"
	"tp8 sp1 bl23 sp1 tp8\n"
	"tp8 sp1 bl23 sp1 tp8\n"
	"tp8 sp1 bl23 sp1 tp8\n"
	"sp9 bl23 sp9\n"
	"\n\n\n\n\n\n\n"
	"sp9 bl23 sp9\n"
	"tp8 sp1 bl23 sp1 tp8\n"
	"tp8 sp1 bl23 sp1 tp8\n"
	"tp8 sp1 bl23 sp1 tp8\n"
	"tp8 sp1 bl23 sp1 tp8\n",

	"\n\n\n"
	"bl8 sp8 bl9 sp8 bl8\n"
	"bl8 sp1 tp6 sp1 bl9 sp1 tp6 sp1 bl8\n"
	"bl8 sp1 tp6 sp1 bl9 sp1 tp6 sp1 bl8\n"
	"bl8 sp1 tp6 sp1 bl9 sp1 tp6 sp1 bl8\n"
	"bl8 sp1 tp6 sp1 bl9 sp1 tp6 sp1 bl8\n"
	"bl8 sp1 tp6 sp1 bl9 sp1 tp6 sp1 bl8\n"
	"bl8 sp1 tp6 sp1 bl9 sp1 tp6 sp1 bl8\n"
	"bl8 sp1 tp6 sp1 bl9 sp1 tp6 sp1 bl8\n"
	"bl8 sp1 tp6 sp1 bl9 sp1 tp6 sp1 bl8\n"
	"bl8 sp1 tp6 sp1 bl9 sp1 tp6 sp1 bl8\n"
	"bl8 sp8 bl9 sp8 bl8\n"
	"\n\n\n",

	"\n\n\n\n\n"
	"bl4 sp33 bl4\n"
	"bl4 sp1 tp31 sp1 bl4\n"
	"bl4 sp1 tp31 sp1 bl4\n"
	"bl4 sp1 tp31 sp1 bl4\n"
	"bl4 sp1 tp31 sp1 bl4\n"
	"bl4 sp1 tp31 sp1 bl4\n"
	"bl4 sp33 bl4\n"
	"\n\n\n\n\n"

};

#endif