#include "pb_configuration.h"
#include "pb_global.h"
#include "bot.h"
#include <dirent.h>

extern int mod_id;
extern char mod_name[32];

cvar_t bot_num = { "bot_num", "5" };
cvar_t bot_minnum = { "bot_minnum", "4" };
cvar_t bot_maxnum = { "bot_maxnum", "6" };
cvar_t bot_realgame = { "bot_realgame", "1" };
cvar_t bot_staytime = { "bot_staytime", "20" };
cvar_t bot_chatenabled = { "bot_chat_enabled", "1" };
cvar_t bot_chatlang = { "bot_chatlang", "en_US" };
cvar_t bot_chatlog = { "bot_chatlog", "0" };
cvar_t bot_chatrespond = { "bot_chatrespond", "1" };
cvar_t bot_peacemode = { "bot_peacemode", "0" };
cvar_t bot_restrictedweapons = { "bot_restrictedweapons", "0" };
cvar_t bot_touringmode = { "bot_touringmode", "0" };
cvar_t bot_maxaimskill = { "bot_maxaimskill", "10" };
cvar_t bot_minaimskill = { "bot_minaimskill", "1" };

PB_Configuration::PB_Configuration()
{
}

PB_Personality PB_Configuration::personality( int index )
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

	if( !UTIL_FileExists( filename ) )
	{
		infoMsg( "Missing %s\n", filename );

		CreateDirectory( configPath, NULL );
		return createConfiguration( filename );
	}

	strcpy( cmd, "exec ");
	strcat( cmd, &filename[strlen( mod_name ) + 1] );
	strcat( cmd, "\n" );

	SERVER_COMMAND( cmd );
//	SERVER_EXECUTE;

	return true;
}

void PB_Configuration::registerVars()
{
	CVAR_REGISTER( &bot_num );
        CVAR_REGISTER( &bot_minnum );
        CVAR_REGISTER( &bot_maxnum );
        CVAR_REGISTER( &bot_realgame );
        CVAR_REGISTER( &bot_staytime );
        CVAR_REGISTER( &bot_chatenabled );
        CVAR_REGISTER( &bot_chatlang );
        CVAR_REGISTER( &bot_chatlog );
        CVAR_REGISTER( &bot_chatrespond );
        CVAR_REGISTER( &bot_peacemode );
        CVAR_REGISTER( &bot_restrictedweapons );
        CVAR_REGISTER( &bot_touringmode );
        CVAR_REGISTER( &bot_maxaimskill );
        CVAR_REGISTER( &bot_minaimskill );
}

bool PB_Configuration::initPersonalities( const char *personalityPath )
{
	byte *pMemFile;
	int fileSize, filePos = 0;
	const char *pBuffer;
	char filename[64];

	strcpy( filename, personalityPath );
	strcat( filename, "characters.cfg" );
	
	infoMsg( "Reading %s... ", filename );

	pMemFile = LOAD_FILE_FOR_ME( &filename[strlen( mod_name ) + 1], &fileSize );
	if( !pMemFile )
	{
		infoMsg( "failed\n" );
		return createPersonalities( filename );
	}

	while( ( pBuffer = UTIL_memfgets( pMemFile, fileSize, filePos ) ) )
	{
		int iTopColor = 0, iBottomColor = 0;
		char name[256], model[256]; 
		PB_Personality newPers = {0};

		// skip whitespace
		while( *pBuffer && isspace( *pBuffer ) )
			pBuffer++;

		if( !*pBuffer )
			continue;

		// skip comment lines
		if( *pBuffer == '/' )
			continue;

		if( 8 > sscanf( pBuffer, "\"%[^\"]\" \"%[^\"]\" %i%i%i%i%i%i", name,	// read entire name
								model,		// read entire model
								&newPers.aimSkill,		// aim skill
								&newPers.aggression,	// aggression
								&newPers.sensitivity,	// sensitivity
								&newPers.communication,	// communication
								&iTopColor,	// top color
								&iBottomColor ) ) 	// bottom color
			continue;

		strcpy_s( newPers.name, BOT_NAME_LEN, name );
		strcpy_s( newPers.model, BOT_SKIN_LEN, model );
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

	FREE_FILE( pMemFile );

	if( !character.size() )
	{
		infoMsg( "failed\n" );
		errorMsg( "Empty or broken %s\n", filename );
		return createPersonalities( filename );
	}

	infoMsg( "OK!\n" );
	return true;
}

bool PB_Configuration::createConfiguration( const char *configFile )
{
	FILE *file;
	cvar_t *list;
	infoMsg( "Creating %s... ", configFile  );
	file = fopen( configFile, "wt" );

	if( !file )
	{
		infoMsg( "failed!\n" );
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

	infoMsg( "OK!\n" );
	return true;
}

bool PB_Configuration::createPersonalities( const char *personalityFile )
{
	byte *pMemFile;
	int fileSize, filePos = 0;
	const char *pBuffer;
	char filename[64];

	strcpy( filename, mod_name );
	strcat( filename, "/addons/parabot/config/common/namelist.txt" );

	if( !UTIL_FileExists( filename ) )
	{
		infoMsg( "Missing %s\n", filename );
		return false;
	}

	if( !loadModelList( mod_name ) )
		return false;

	infoMsg( "Reading %s... ", filename );

	pMemFile = LOAD_FILE_FOR_ME( &filename[strlen( mod_name ) + 1], &fileSize );
	if( !pMemFile )
	{
		infoMsg( "failed\n" );
		clearModelList();
		return false;
	}

	while( ( pBuffer = UTIL_memfgets( pMemFile, fileSize, filePos ) ) )
	{
		char gamedir[256], name[256], model[256];
		const char *pszModel;
		PB_Personality newPers = {0};
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

			if( FStriEq( gamedir, mod_name )
				|| FStriEq( gamedir, "valve" ))
				readNextLine = true;
			else
				readNextLine = false;
			continue;
		}
		else if( !readNextLine )
			continue;

		args = sscanf( pBuffer, "\"%[^\"]\" \"%[^\"]\"", name,	// read entire name
								model );	// read entire model

		if( args == 0 )
			continue;
		else if( args < 2 )
			pszModel = playerModelList[RANDOM_LONG( 0, playerModelList.size() - 1 )];
		else
			pszModel = model;

		strcpy_s( newPers.name, BOT_NAME_LEN, name );
		strcpy_s( newPers.model, BOT_SKIN_LEN, pszModel );

		newPers.aimSkill = RANDOM_LONG( 1, 10 );
		newPers.aggression = RANDOM_LONG( 1, 10 );
		newPers.sensitivity = RANDOM_LONG( 1, 10 );
		newPers.communication = RANDOM_LONG( 1, 10 );
		character.push_back( newPers );
		strcpy( character[character.size() - 1].topColor, getColor( character.size() - 1, 371 ) );
		strcpy( character[character.size() - 1].bottomColor, getColor( character.size() - 1, 97 ) );
	}
	
	FREE_FILE( pMemFile );

	infoMsg( "OK!\n" );

	clearModelList();

	return savePersonalities( personalityFile );
}

bool PB_Configuration::savePersonalities( const char *personalityFile )
{
	int i;
	FILE *file;

	infoMsg( "Creating %s... ", personalityFile );
	file = fopen( personalityFile, "wt" );

	if( !file )
	{
		infoMsg( "failed!\n" );
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

	for( i = 0; i < character.size(); i++ )
		fprintf( file, "\"%s\"\t\"%s\"\t%i\t%i\t%i\t%i\t%s\t%s\n", character[i].name,
									character[i].model,
									character[i].aimSkill,
									character[i].aggression,
									character[i].sensitivity,
									character[i].communication,
									character[i].topColor,
									character[i].bottomColor );

	fclose( file );

	infoMsg( "OK!\n" );
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

	infoMsg( "Loading list of models from %s... ", filePath );

	dfd = opendir( filePath );

	while( ( model = readdir( dfd ) ) )
	{
		if( FStrEq( model->d_name, ".") )
			continue;

		if( FStrEq( model->d_name, "..") )
			continue;

		playerModelList.push_back( strdup( model->d_name ) );
	}
	closedir( dfd );

	if( !playerModelList.size() )
	{
		infoMsg( "failed!\n" );

		if( bGamedirIsEmpty )
			return false;

		bGamedirIsEmpty = true;
		return loadModelList( "valve" ); // TODO: add check for fallback_dir
	}

	infoMsg( "OK!\n" );
	return true;
}

void PB_Configuration::clearModelList()
{
	int i;

	for( i = 0; i < playerModelList.size(); i++ )
	{
		free( playerModelList[i] );
	}

	playerModelList.clear();
}

const char* PB_Configuration::getColor( int persNr, int modulo )
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
