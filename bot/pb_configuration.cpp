#include "parabot.h"
#include "pb_configuration.h"
#include "pb_global.h"
#include "bot.h"
#include <ctype.h>
#include <dirent.h>

extern int mod_id;

CVAR bot_num = { "bot_num", "5" };
CVAR bot_minnum = { "bot_minnum", "4" };
CVAR bot_maxnum = { "bot_maxnum", "6" };
CVAR bot_realgame = { "bot_realgame", "1" };
CVAR bot_staytime = { "bot_staytime", "20" };
CVAR bot_chatenabled = { "bot_chat_enabled", "1" };
CVAR bot_chatlang = { "bot_chatlang", "en_US" };
CVAR bot_chatlog = { "bot_chatlog", "0" };
CVAR bot_chatrespond = { "bot_chatrespond", "1" };
CVAR bot_peacemode = { "bot_peacemode", "0" };
CVAR bot_restrictedweapons = { "bot_restrictedweapons", "0" };
CVAR bot_touringmode = { "bot_touringmode", "0" };
CVAR bot_maxaimskill = { "bot_maxaimskill", "10" };
CVAR bot_minaimskill = { "bot_minaimskill", "1" };

PB_Configuration::PB_Configuration()
{
}

PERSONALITY PB_Configuration::personality( int index )
{ 
	debugFile( "  %i: %s  ", index, character[index].name );
	return character[index]; 
}

bool PB_Configuration::initConfiguration( const char *configPath )
{
	char filename[64], cmd[64];

	registerVars();

	strcpy( filename, configPath );
	strcat( filename, "parabot.cfg" );

	if( !fileexists( filename ) )
	{
		INFO_MSG( "Missing %s\n", filename );

		CreateDirectory( configPath, NULL );
		return createConfiguration( filename );
	}

	strcpy( cmd, "exec ");
	strcat( cmd, &filename[strlen( com.modname ) + 1] );
	strcat( cmd, "\n" );

	servercommand( cmd );
//	SERVER_EXECUTE;

	return true;
}

void PB_Configuration::registerVars()
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

bool PB_Configuration::initPersonalities( const char *personalityPath )
{
	byte *pMemFile;
	int fileSize, filePos = 0;
	const char *pBuffer;
	char filename[64];

	strcpy( filename, personalityPath );
	strcat( filename, "characters.cfg" );
	
	INFO_MSG( "Reading %s... ", filename );

	pMemFile = loadfileforme( &filename[strlen( com.modname ) + 1], &fileSize );
	if( !pMemFile )
	{
		INFO_MSG( "failed\n" );
		return createPersonalities( filename );
	}

	while( ( pBuffer = memfgets( pMemFile, fileSize, &filePos ) ) )
	{
		int iTopColor = 0, iBottomColor = 0;
		PERSONALITY newPers = {0};

		// skip whitespace
		while( *pBuffer && isspace( *pBuffer ) )
			pBuffer++;

		if( !*pBuffer )
			continue;

		// skip comment lines
		if( *pBuffer == '/' )
			continue;

		if( 8 > sscanf( pBuffer, "\"%31[^\"]\" \"%31[^\"]\" %2i%2i%2i%2i%3i%3i",
		    newPers.name,		// read entire name
		    newPers.model,		// read entire model
		    &newPers.aimSkill,		// aim skill
		    &newPers.aggression,	// aggression
		    &newPers.sensitivity,	// sensitivity
		    &newPers.communication,	// communication
		    &iTopColor,			// top color
		    &iBottomColor)) {		// bottom color
			continue;
		}

		newPers.aimSkill = clamp( newPers.aimSkill, 1, 10 );
		newPers.aggression = clamp( newPers.aggression, 1, 10 );
		newPers.sensitivity = clamp( newPers.sensitivity, 1, 10 );
		newPers.communication = clamp( newPers.communication, 1, 10 );
		iTopColor = clamp( iTopColor, 0, 255 );
		iBottomColor = clamp( iBottomColor, 0, 255 );
		sprintf( newPers.topColor,"%i", iTopColor );
		sprintf( newPers.bottomColor,"%i", iBottomColor );
		character.push_back( newPers );
	}

	freefile( pMemFile );

	if( !character.size() )
	{
		INFO_MSG( "failed\n" );
		ERROR_MSG( "Empty or broken %s\n", filename );
		return createPersonalities( filename );
	}

	INFO_MSG( "OK!\n" );
	return true;
}

bool PB_Configuration::createConfiguration( const char *configFile )
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

bool PB_Configuration::createPersonalities( const char *personalityFile )
{
	byte *pMemFile;
	int fileSize, filePos = 0;
	const char *pBuffer;
	char filename[64];

	strcpy( filename, com.modname );
	strcat( filename, "/addons/parabot/config/common/namelist.txt" );

	if(fileexists( filename ) )
	{
		INFO_MSG( "Missing %s\n", filename );
		return false;
	}

	if( !loadModelList( com.modname ) )
		return false;

	INFO_MSG( "Reading %s... ", filename );

	pMemFile = loadfileforme( &filename[strlen( com.modname ) + 1], &fileSize );
	if( !pMemFile )
	{
		INFO_MSG( "failed\n" );
		clearModelList();
		return false;
	}

	while (( pBuffer = memfgets( pMemFile, fileSize, &filePos)))
	{
		char gamedir[256];
		PERSONALITY newPers = {0};
		int args;
		static bool readNextLine = false;

		// skip whitespace
		while( *pBuffer && isspace( *pBuffer ) )
			pBuffer++;

		if( !*pBuffer )
			continue;

		// skip comment lines
		if( *pBuffer == '/' )
			continue;

		if( *pBuffer == '[' )
		{
			if( 1 != sscanf( pBuffer,"[%[^]]]", gamedir ) )
				continue;

			if( Q_STRIEQ( gamedir, com.modname )
				|| Q_STRIEQ( gamedir, "valve" ))
				readNextLine = true;
			else
				readNextLine = false;
			continue;
		}
		else if( !readNextLine )
			continue;

		args = sscanf( pBuffer, "\"%31[^\"]\" \"%31[^\"]\"", newPers.name,	// read entire name
								newPers.model );	// read entire model

		if( args == 0 )
			continue;
		else if( args < 2 )
			strcpy(newPers.model, playerModelList[randomint(0, playerModelList.size() - 1)]);

		newPers.aimSkill = randomint(1, 10);
		newPers.aggression = randomint(1, 10 );
		newPers.sensitivity = randomint( 1, 10 );
		newPers.communication = randomint( 1, 10 );
		character.push_back(newPers);
		strcpy( character[character.size() - 1].topColor, getColor( character.size() - 1, 371 ) );
		strcpy( character[character.size() - 1].bottomColor, getColor( character.size() - 1, 97 ) );
	}
	
	freefile( pMemFile );

	INFO_MSG( "OK!\n" );

	clearModelList();

	return savePersonalities( personalityFile );
}

bool PB_Configuration::savePersonalities( const char *personalityFile )
{
	int i;
	FILE *file;

	INFO_MSG( "Creating %s... ", personalityFile );
	file = fopen( personalityFile, "wt" );

	if( !file ) {
		INFO_MSG( "failed!\n" );
		return false;
	}

#define CHARACTERS_PREAMBULA "//==========================================================================================\n" \
				"//  characters.cfg - uses to configure the individual PARABOT characters.\n" \
				"//  Change these settings or add new entries as you like, using the following parameters:\n" \
				"//\n" \
				"//  1) Name:         String (using \"...\") of the playername\n" \
				"//  2) Model:        String (using \"...\") of a standard model (e.g. \"gordon\") or a valid\n" \
				"//                   custom model in the <modfolder>/models/player directory\n" \
				"//  3) Aimskill:     Sets the aiming capability, ranging from 1 (lame) to 10 (deadly)\n" \
				"//  4) Aggressivity: Sets the combat behavior, ranging from 1 (camper) to 10 (berzerk)\n" \
				"//  5) Sensitivity:  Controls the perceptions (seeing & hearing) of the bot, ranging from\n" \
				"//                   1 (nearly blind & deaf) to 10 (capturing everything)\n" \
				"//  6) Chatrate:     Adjustable from 1 (doesn't chat) to 10 (blablabla... :-) )\n" \
				"//  7) Topcolor:     Sets color of model top.\n" \
				"//  8) Bottomcolor:  Sets color of model bottom.\n" \
				"//\n" \
				"//  Be careful with adding new entries: if the format does not exactly match the above\n" \
				"//  specifications, the game will probably be hanging at startup.\n" \
				"//  This file must contain at least one entry, no limits on entries count.\n" \
				"//==========================================================================================\n" \
				"// Botname\tBotmodel\tAiming\tAggres.\tSensing\tChat\tTopcolor\tBottomcolor\n" \
				"//------------------------------------------------------------------------------------------\n"

	fputs( CHARACTERS_PREAMBULA, file );

	for( i = 0; i < character.size(); i++ ) {
		fprintf( file, "\"%s\"\t\"%s\"\t%i\t%i\t%i\t%i\t%s\t%s\n", character[i].name,
		    character[i].model,
		    character[i].aimSkill,
		    character[i].aggression,
		    character[i].sensitivity,
		    character[i].communication,
		    character[i].topColor,
		    character[i].bottomColor );
	}

	fclose( file );

	INFO_MSG( "OK!\n" );
	return true;
}

bool PB_Configuration::loadModelList( const char *gamedir )
{
	char filePath[64];
	DIR *dfd;
	struct dirent *model;
	static bool bGamedirIsEmpty = false;

	strcpy( filePath, gamedir );
	strcat( filePath, "/models/player" );

	INFO_MSG( "Loading list of models from %s... ", filePath );

	dfd = opendir( filePath );

	while((model = readdir(dfd))) {
		if( Q_STREQ( model->d_name, ".") )
			continue;

		if( Q_STREQ( model->d_name, "..") )
			continue;

		playerModelList.push_back(strdup(model->d_name));
	}
	closedir(dfd);

	if( !playerModelList.size() ) {
		INFO_MSG( "failed!\n" );

		if( bGamedirIsEmpty )
			return false;

		bGamedirIsEmpty = true;
		return loadModelList( "valve" ); // TODO: add check for fallback_dir
	}

	INFO_MSG( "OK!\n" );
	return true;
}

void PB_Configuration::clearModelList()
{
	int i;

	for(i = 0; i < playerModelList.size(); i++)
	{
		free( playerModelList[i] );
	}

	playerModelList.clear();
}

const char* PB_Configuration::getColor(int persNr, int modulo)
{
	static char color[4];
	int code = 0;
	int nameLen = strlen( character[persNr].name );
	for (int i=0; i<nameLen; i++) 
		code += ((int)character[persNr].name[i] * (729+i)) % modulo;
	code = (code % 255) + 1;
	sprintf( color, "%i", code );
	return color;
}
