#include "parabot.h"
#include "configuration.h"
#include "pb_global.h"
#include "bot.h"
#include <ctype.h>
#include <dirent.h>

extern int mod_id;

static CVAR bot_num = {"bot_num", "5"};
static CVAR bot_minnum = {"bot_minnum", "4"};
static CVAR bot_maxnum = {"bot_maxnum", "6"};
static CVAR bot_realgame = {"bot_realgame", "1"};
static CVAR bot_staytime = {"bot_staytime", "20"};
static CVAR bot_chatenabled = {"bot_chat_enabled", "1"};
static CVAR bot_chatlang = {"bot_chatlang", "en_US"};
static CVAR bot_chatlog = {"bot_chatlog", "0"};
static CVAR bot_chatrespond = {"bot_chatrespond", "1"};
static CVAR bot_peacemode = {"bot_peacemode", "0"};
static CVAR bot_restrictedweapons = {"bot_restrictedweapons", "0"};
static CVAR bot_touringmode = {"bot_touringmode", "0"};
static CVAR bot_maxaimskill = {"bot_maxaimskill", "10"};
static CVAR bot_minaimskill = {"bot_minaimskill", "1"};

static void
configuration_registerconvars()
{
	cvar_register( &bot_num );
	cvar_register( &bot_minnum );
	cvar_register( &bot_maxnum );
	cvar_register( &bot_realgame );
	cvar_register( &bot_staytime );
	cvar_register( &bot_chatenabled );
	cvar_register( &bot_chatlang );
	cvar_register( &bot_chatlog );
	cvar_register( &bot_chatrespond );
	cvar_register( &bot_peacemode );
	cvar_register( &bot_restrictedweapons );
	cvar_register( &bot_touringmode );
	cvar_register( &bot_maxaimskill );
	cvar_register( &bot_minaimskill );
}

bool
configuration_create(const char *configFile)
{
	FILE *file;
	CVAR *list;
	INFO_MSG( "Creating %s... ", configFile  );
	file = fopen( configFile, "wt" );

	if( !file )
	{
		INFO_MSG( "failed!\n" );
		return false;
	}

#define PARABOT_PREAMBULA "//=======================================================================\n" \
			"//\t\tparabot.cfg - the main configuration file for the Parabot.\n" \
			"//=======================================================================\n"

	fputs( PARABOT_PREAMBULA, file );

	for( list = &bot_chatenabled; list; list = list->next )
	{
		if( strncmp( list->name, "bot_", 4 ) )
			break;

		fprintf( file,"%s \"%s\"\n", list->name, list->string );
	}

	fclose( file );

	INFO_MSG( "OK!\n" );
	return true;
}

bool
configuration_init(const char *configPath)
{
	char filename[64], cmd[64];

	configuration_registerconvars();

	strcpy( filename, configPath );
	strcat( filename, "parabot.cfg" );

	if( !fileexists( filename ) )
	{
		INFO_MSG( "Missing %s\n", filename );

		CreateDirectory( configPath, NULL );
		return configuration_create( filename );
	}

	strcpy( cmd, "exec ");
	strcat( cmd, &filename[strlen( com.modname ) + 1] );
	strcat( cmd, "\n" );

	servercommand( cmd );
//	SERVER_EXECUTE;

	return true;
}

int
configuration_minskill()
{
	return bot_minaimskill.value;
}

int
configuration_maxskill()
{
	return bot_maxaimskill.value;
}

int
configuration_numbots()
{
	return bot_num.value;
}

int
configuration_minbots()
{
	return bot_minnum.value;
}

int
configuration_maxbots()
{
	return bot_maxnum.value;
}

float
configuration_staytime()
{
	return bot_staytime.value * 60;
}

const char *
configuration_chatfile()
{
	return bot_chatlang.string;
}

bool
configuration_usingchat()
{
	return bot_chatenabled.value;
}

bool
configuration_onalwaysrespond()
{
	return bot_chatrespond.value;
}

bool
configuration_onservermode()
{
	return bot_realgame.value;
}

bool configuration_onpeacemode()
{
	return bot_peacemode.value;
}

bool
configuration_onrestrictedweaponmode()
{
	return bot_restrictedweapons.value;
}

bool
configuration_ontouringmode()
{
	return bot_touringmode.value;
}

bool
configuration_onchatlog()
{
	return bot_chatlog.value;
}

void
configuration_setfloatconvar(const char *name, float value, int max, int min)
{
	cvar_setfloat(name, clamp(value, max, min));
}
