#include "pb_configuration.h"
#include "pb_global.h"




PB_Configuration::PB_Configuration()
{
	// number of bots
	myNumBots = 5;
	myMinBots = 4;
	myMaxBots = 6;

	// skills
	minAimSkill = 1;
	maxAimSkill = 10;

	// chat
	botChat = true;
	strcpy( chatFileName, "ChatEnglish.txt" );
	strcpy( menuKey, "=" );
	chatAlwaysRespond = false;
	
	// game modes:
	peaceMode = false;
	restrictedWeaponMode = false;
	serverMode = false;
	myStayTime = 20.0 * 60;

	// personalities:
	maxPers = 0;
}


PB_Personality PB_Configuration::personality( int index )
{ 
/*	char dbgBuffer[256];
	sprintf( dbgBuffer, "  %i: %s  ", index, character[index].name );
	debugFile( dbgBuffer );*/
	return character[index]; 
}


int PB_Configuration::clampInt( char *str, int min, int max )
{
	int val = atoi( str );
	if (val<min) val=min;
	else if (val>max) val=max;
	return val;
}


bool PB_Configuration::varSet( char *srcName, FILE *file, char *varName, bool &var )
{
	char buffer[1024];

	if ( _stricmp( srcName, varName ) == 0 ) {
		fscanf( file, " = %s ", buffer );
		if ( _stricmp( buffer, "ON" ) == 0 ) var = true;
		else if ( _stricmp( buffer, "OFF" ) == 0 ) var = false;
		return true;
	}
	else return false;
}


bool PB_Configuration::varSet( const char *srcName, const char *srcValue, char *varName, bool &var )
{
	if ( _stricmp( srcName, varName ) == 0 ) {
		if (srcValue == 0) {
			if (var) infoMsg( varName, " is on.\n" );
			else	 infoMsg( varName, " is off.\n" );
		}
		else {
			if ( _stricmp( srcValue, "ON" ) == 0 ) {
				var = true;
				infoMsg( varName, " activated.\n" );
			}
			else if ( _stricmp( srcValue, "OFF" ) == 0 ) {
				var = false;
				infoMsg( varName, " deactivated.\n" );
			}
			else {
				infoMsg( "Usage: ", varName, " on/off\n"	);
			}
		}
		return true;
	}
	else return false;
}


bool PB_Configuration::varSet( const char *srcName, int srcValue, char *varName, int &var )
{
	char buffer[64];
	if ( _stricmp( srcName, varName ) == 0 ) {
		var = srcValue;
		sprintf( buffer, "%s set to %i\n", varName, srcValue );
		infoMsg( buffer );
		return true;
	}
	else return false;
}


bool PB_Configuration::setBoolVar( const char *name, const char *value )
{
	if (!varSet( name, value, "BotChat",			botChat					)) 
	if (!varSet( name, value, "AlwaysRespond",		chatAlwaysRespond		))
	if (!varSet( name, value, "ChatLog",			chatLog					))
	if (!varSet( name, value, "PeaceMode",			peaceMode				))
	if (!varSet( name, value, "RestrictedWeapons",	restrictedWeaponMode	))
	if (!varSet( name, value, "HideWelcome",		touringMode				))
	if (!varSet( name, value, "ServerMode",			serverMode				))
		return false;
	return true;
}


bool PB_Configuration::setIntVar( const char *name, int value, int min, int max )
{
	if (value < min) value = min;
	if (value > max) value = max;

	if (!varSet( name, value, "MinBots",		myMinBots	)) 
	if (!varSet( name, value, "MaxBots",		myMaxBots	)) 
	if (!varSet( name, value, "NumBots",		myNumBots	)) 
//	if (!varSet( name, value, "StayTime",		myStayTime	)) 
	if (!varSet( name, value, "MinAimSkill",	minAimSkill	)) 
	if (!varSet( name, value, "MaxAimSkill",	maxAimSkill	)) 
		return false;
	return true;
}


bool PB_Configuration::initConfiguration( const char *configFile )
{
	char str[256];

	FILE *file = fopen( configFile, "rt" );
	if (!file) {
		errorMsg( "Missing ", configFile );
		return false;
	}
	infoMsg( "Reading ", configFile, "... " );
	
	while (!feof(file)) {
		fscanf( file, "%1s", str );			// read first char
		if (feof(file)) break;

		while (str[0]=='#') {				// Comments:
			fscanf( file, "%[^\n]", str );	//   read entire line
			fscanf( file, "%1s", str );		//   read first char
		}
		if ( !feof(file) ) {
			
			fseek( file, -1, SEEK_CUR );	// reset filepointer
			fscanf( file, "%[a-zA-Z]", str);// now we've got a complete word
			
			if ( _stricmp( str, "NumBots" ) == 0 ) {
				fscanf( file, " = %[0-9] ", str );		// read number of bots
				myNumBots = clampInt( str, 0, 32 );
			}
			else if ( _stricmp( str, "MinBots" ) == 0 ) {
				fscanf( file, " = %[0-9] ", str );		// read min. number of bots
				myMinBots = clampInt( str, 0, 32 );
			}
			else if ( _stricmp( str, "MaxBots" ) == 0 ) {
				fscanf( file, " = %[0-9] ", str );		// read max. number of bots
				myMaxBots = clampInt( str, myMinBots, 32 );
			}
			else if ( _stricmp( str, "AverageStay" ) == 0 ) {
				fscanf( file, " = %[0-9] ", str );		// read staytime
				myStayTime = 60.0 * (float)clampInt( str, 2, 180 );
			}
			else if ( _stricmp( str, "MinAimSkill" ) == 0 ) {
				fscanf( file, " = %[0-9] ", str );		// read min aim skill
				minAimSkill = clampInt( str, 1, 10 );
			}
			else if ( _stricmp( str, "MaxAimSkill" ) == 0 ) {
				fscanf( file, " = %[0-9] ", str );		// read max aim skill
				maxAimSkill = clampInt( str, minAimSkill, 10 );
			}
			else if ( _stricmp( str, "ChatFile" ) == 0 ) {
				fscanf( file, " = \"%[^\"]\" ", str );		// read chat file
				strcpy( chatFileName, str );
			}
			else if ( _stricmp( str, "MenuKey" ) == 0 ) {
				fscanf( file, " = \"%[^\"]\" ", str );		// read menu key
				strcpy( menuKey, str );
			}
			else {
				// simple on/off switches:
				if (!varSet( str, file, "BotChat",				botChat					))
				if (!varSet( str, file, "AlwaysRespond",		chatAlwaysRespond		))
				if (!varSet( str, file, "ChatLog",				chatLog					))
				if (!varSet( str, file, "PeaceMode",			peaceMode				))
				if (!varSet( str, file, "RestrictedWeapons",	restrictedWeaponMode	))
				if (!varSet( str, file, "HideWelcome",			touringMode				))
				if (!varSet( str, file, "ServerMode",			serverMode				)) {
					debugMsg( "Unknown variable ", str, "\n" );
					fscanf( file, "%[^\n]", str );	//   ignore entire line
				}
			}
		}
	}
	fclose(	file );
	infoMsg( "OK!\n" );
	return true;
}


bool PB_Configuration::initPersonalities( const char *personalityFile )
{
	char str[256];

	FILE *file = fopen( personalityFile, "rt" );
	if (!file) {
		errorMsg( "Missing ", personalityFile );
		return false;
	}
	infoMsg( "Reading ", personalityFile, "... " );

	int nr = 0;
	while (!feof(file)) {
		fscanf( file, "%1s", str );						// supposed name start '"'
		if (feof(file)) break;

		while (str[0]=='#') {							// Comments
			fscanf( file, "%[^\n]", str );				// read entire line
			fscanf( file, "%1s", str );					// supposed name start '"'
		}
		if ( !feof(file) ) {
			
			fscanf( file, "%[^\"]\"", character[nr].name );		// read entire name
						
			fscanf( file, " \"%[^\"]\" ", character[nr].model );	// read entire model
						
			fscanf( file, "%s", str );						// aim skill
			character[nr].aimSkill = clampInt( str, 1, 10 );
			
			fscanf( file, "%s", str );						// aggression
			character[nr].aggression = clampInt( str, 1, 10 );
			
			fscanf( file, "%s", str );						// sensitivity
			character[nr].sensitivity = clampInt( str, 1, 10 );

			fscanf( file, "%s", str );						// communication
			character[nr].communication = clampInt( str, 1, 10 );
			
			nr++;
			if (nr==MAX_PERS) break;
		}
	}
	maxPers = nr;
	fclose(	file );
	for (nr=0; nr<MAX_PERS; nr++) character[nr].inUse = false;
	infoMsg( "OK!\n" );
	return true;
}


char* PB_Configuration::getColor( int persNr, int modulo )
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
