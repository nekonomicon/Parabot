#include "parabot.h"
#include "personalities.h"
#include <ctype.h>
#include <dirent.h>

static bool	personalities_loadmodellist(const char *gamedir);
static void	personalities_clearmodellist();
static void	personalities_getcolor(int persNr, int modulo, char *color);
static bool	personalities_create(const char *personalityFile);
static bool	personalities_save(const char *personalityFile);

static std::vector<PERSONALITY> character;
static std::vector<char*> playermodellist;

PERSONALITY
personalities_get(int index)
{ 
	debugFile( "  %i: %s  ", index, character[index].name );
	return character[index]; 
}

bool
personalities_init(const char *personalityPath)
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
		return personalities_create( filename );
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
		    &newPers.aimskill,		// aim skill
		    &newPers.aggression,	// aggression
		    &newPers.sensitivity,	// sensitivity
		    &newPers.communication,	// communication
		    &iTopColor,			// top color
		    &iBottomColor)) {		// bottom color
			continue;
		}

		newPers.aimskill = clamp( newPers.aimskill, 1, 10 );
		newPers.aggression = clamp( newPers.aggression, 1, 10 );
		newPers.sensitivity = clamp( newPers.sensitivity, 1, 10 );
		newPers.communication = clamp( newPers.communication, 1, 10 );
		iTopColor = clamp( iTopColor, 0, 255 );
		iBottomColor = clamp( iBottomColor, 0, 255 );
		sprintf( newPers.topcolor,"%i", iTopColor );
		sprintf( newPers.bottomcolor,"%i", iBottomColor );
		character.push_back( newPers );
	}

	freefile( pMemFile );

	if( !character.size() )
	{
		INFO_MSG( "failed\n" );
		ERROR_MSG( "Empty or broken %s\n", filename );
		return personalities_create( filename );
	}

	INFO_MSG( "OK!\n" );
	return true;
}

bool
personalities_create(const char *personalityFile)
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

	if( !personalities_loadmodellist( com.modname ) )
		return false;

	INFO_MSG( "Reading %s... ", filename );

	pMemFile = loadfileforme( &filename[strlen( com.modname ) + 1], &fileSize );
	if( !pMemFile )
	{
		INFO_MSG( "failed\n" );
		personalities_clearmodellist();
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
			strcpy(newPers.model, playermodellist[randomint(0, playermodellist.size() - 1)]);

		newPers.aimskill = randomint(1, 10);
		newPers.aggression = randomint(1, 10 );
		newPers.sensitivity = randomint( 1, 10 );
		newPers.communication = randomint( 1, 10 );
		character.push_back(newPers);
		personalities_getcolor(character.size() - 1, 371, character[character.size() - 1].topcolor);
		personalities_getcolor(character.size() - 1, 97, character[character.size() - 1].bottomcolor);
	}
	
	freefile( pMemFile );

	INFO_MSG( "OK!\n" );

	personalities_clearmodellist();

	return personalities_save( personalityFile );
}

bool
personalities_save(const char *personalityFile)
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
		    character[i].aimskill,
		    character[i].aggression,
		    character[i].sensitivity,
		    character[i].communication,
		    character[i].topcolor,
		    character[i].bottomcolor );
	}

	fclose( file );

	INFO_MSG( "OK!\n" );
	return true;
}

bool
personalities_loadmodellist(const char *gamedir)
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

		playermodellist.push_back(strdup(model->d_name));
	}
	closedir(dfd);

	if( !playermodellist.size() ) {
		INFO_MSG( "failed!\n" );

		if( bGamedirIsEmpty )
			return false;

		bGamedirIsEmpty = true;
		return personalities_loadmodellist( "valve" ); // TODO: add check for fallback_dir
	}

	INFO_MSG( "OK!\n" );
	return true;
}

void
personalities_clearmodellist()
{
	int i;

	for(i = 0; i < playermodellist.size(); i++)
	{
		free( playermodellist[i] );
	}

	playermodellist.clear();
}

void
personalities_getcolor(int persNr, int modulo, char *color)
{
	int code = 0;

	int nameLen = strlen(character[persNr].name);
	for (int i = 0; i < nameLen; i++) 
		code += ((int)character[persNr].name[i] * (729 + i)) % modulo;
	code = (code % 255) + 1;
	sprintf(color, "%i", code);
}

const char *
personalities_gettopcolor(int index)
{
	return character[index].topcolor;
}

const char *
personalities_getbottomcolor(int index)
{
	return character[index].bottomcolor;
}

void
personalities_joins(int index)
{
	character[index].inuse = true;
}

void
personalities_leave(int index)
{
	character[index].inuse = false;
}

int
personalities_count()
{
	return character.size();
}
