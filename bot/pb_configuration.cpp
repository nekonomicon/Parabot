#include "pb_configuration.h"
#include "pb_global.h"
#include "bot.h"

extern int mod_id;
extern char mod_name[32];

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

bool PB_Configuration::initConfiguration( const char *configPath )
{
	char str[256];
	strcpy( str, configPath );
	strcat( str, "parabot.cfg" );

	FILE *file = fopen( str, "rt" );
	if (!file) {
		errorMsg( "Missing ", str, "\n" );

		CreateDirectory( configPath, NULL );
		if( !createConfiguration( str ) )
			return false;
		file = fopen( str, "rt" );
	}
	infoMsg( "Reading ", str, "... " );
	
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

bool PB_Configuration::initPersonalities( const char *personalityPath )
{
	char str[256];
	strcpy( str, personalityPath );
	strcat( str, "characters.cfg" );

	FILE *file = fopen( str, "rt" );
	if (!file) {
		errorMsg( "Missing ", str, "\n" );

		if( !createPersonalities( str ) )
			return false;
		file = fopen( str, "rt" );
	}
	infoMsg( "Reading ", str, "... " );

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

bool PB_Configuration::createConfiguration( const char *configFile )
{
	FILE *file;

	infoMsg( "Creating ", configFile, "... " );
	file = fopen( configFile, "a" );

	if( !file )
	{
		infoMsg( "failed!\n" );
		return false;
	}

	fprintf( file, "############################################################################################\n" );
	fprintf( file, "#                                     PARABOT.CFG                                          #\n" );
	fprintf( file, "#                                                                                          #\n" );
	fprintf( file, "#  This is the main configuration file for the Parabot. There are three main sections to   #\n" );
	fprintf( file, "#  configure basic game options, global botskill values and the chat-and-response system.  #\n" );
	fprintf( file, "#                                                                                          #\n" );
	fprintf( file, "#                                                                                          #\n" );
	fprintf( file, "############################################################################################\n\n\n" );
	fprintf( file, "#-------------------------------------------------------------------------------------------\n" );
	fprintf( file, "# GAME CONFIGURATION\n" );
	fprintf( file, "#-------------------------------------------------------------------------------------------\n\n\n" );
	fprintf( file, "# When \"ServerMode\" is turned on, bots will join and leave the server randomly, staying\n" );
	fprintf( file, "# approximately \"AverageStay\" minutes. There will be at least \"MinBots\" on the server but\n" );
	fprintf( file, "# not more than \"MaxBots\".\n\n" );
	fprintf( file, "ServerMode = On\n" );
	fprintf( file, "MinBots = 4\n" );
	fprintf( file, "MaxBots = 8\n" );
	fprintf( file, "AverageStay = 10\n\n" );
	fprintf( file, "# \"NumBots\" specifies the number of bots that should play the game when \"ServerMode\" is off.\n" );
	fprintf( file, "# When \"ServerMode\" is on this number has no effect.\n\n" );
	fprintf( file, "NumBots = 6\n\n\n" );
	fprintf( file, "# PeaceMode = On/Off (Default=Off)\n" );
	fprintf( file, "# If enabled bots won't shoot at you (nor at each other) while they are not attacked\n" );
	fprintf( file, "PeaceMode = Off\n\n\n" );
	fprintf( file, "# RestrictedWeapons = On/Off (Default=Off)\n" );
	fprintf( file, "# If enabled bots can't use the more powerful weapons (MP5, crossbow, shotgun, rpg, gauss\n" );
	fprintf( file, "# and egon). No restrictions for you.\n\n" );
	fprintf( file, "RestrictedWeapons = Off\n\n\n" );
	fprintf( file, "#-------------------------------------------------------------------------------------------\n" );
	fprintf( file, "# SKILL CONFIGURATION\n" );
	fprintf( file, "#-------------------------------------------------------------------------------------------\n\n\n" );
	fprintf( file, "# MinAimSkill sets a minimum AimSkill that overrides lower values in characters.cfg, i.e.\n" );
	fprintf( file, "# all bots will have at least this minimum value\n\n" );
	fprintf( file, "MinAimSkill = 3\n\n\n" );
	fprintf( file, "# MaxAimSkill sets a maximum AimSkill that overrides higher values in characters.cfg, i.e.\n" );
	fprintf( file, "# all bots will have at most this maximum value\n\n" );
	fprintf( file, "MaxAimSkill = 8\n\n\n" );
	fprintf( file, "#-------------------------------------------------------------------------------------------\n" );
	fprintf( file, "# CHAT CONFIGURATION\n" );
	fprintf( file, "#-------------------------------------------------------------------------------------------\n\n\n" );
	fprintf( file, "# BotChat = On/Off (Default=On)" );
	fprintf( file, "# If enabled bots will chat as much as their communication-value permits.\n" );
	fprintf( file, "BotChat = On\n\n\n" );
	fprintf( file, "# ChatFile determines the language the bots use for chatting (Default=\"ChatEnglish.txt\")\n\n" );
	fprintf( file, "ChatFile = \"ChatEnglish.txt\"\n\n\n" );
	fprintf( file, "# AlwaysRespond = On/Off (Default=On)\n" );
	fprintf( file, "# If enabled bots will always respond to things you say, never mind their comm-value.\n\n" );
	fprintf( file, "AlwaysRespond = On\n\n" );
	fprintf( file, "#-------------------------------------------------------------------------------------------" );

	fclose( file );

	infoMsg( "OK!\n" );
	return true;
}

bool PB_Configuration::createPersonalities( const char *PersonalitityFile )
{
	FILE *file;

	infoMsg( "Creating ", PersonalitityFile, "... " );
	file = fopen( PersonalitityFile, "a" );

	if( !file )
	{
		infoMsg( "failed!\n" );
		return false;
	}

	fprintf( file, "############################################################################################\n" );
	fprintf( file, "#                                   CHARACTERS.CFG                                         #\n" );
	fprintf( file, "#                                                                                          #\n" );
	fprintf( file, "#  This file allows you to configure the individual PARABOT characters.                    #\n" );
	fprintf( file, "#  Change these settings or add new entries as you like, using the following parameters:   #\n" );
	fprintf( file, "#                                                                                          #\n" );
	fprintf( file, "#  1) Name:         String (using \"...\") of the playername                                 #\n" );
	fprintf( file, "#  2) Model:        String (using \"...\") of a standard model (e.g. \"gordon\") or a valid    #\n" );
	fprintf( file, "#                   custom model in the <modfolder>/models/player directory                #\n" );
	fprintf( file, "#  3) Aimskill:     Sets the aiming capability, ranging from 1 (lame) to 10 (deadly)       #\n" );
	fprintf( file, "#  4) Aggressivity: Sets the combat behaviour, ranging from 1 (camper) to 10 (berzerk)     #\n" );
	fprintf( file, "#  5) Sensitivity:  Controls the perceptions (seeing & hearing) of the bot, ranging from   #\n" );
	fprintf( file, "#                   1 (nearly blind & deaf) to 10 (capturing everything)                   #\n" );
	fprintf( file, "#  6) Chatrate:     Adjustable from 1 (doesn't chat) to 10 (blablabla... :-) )             #\n" );
	fprintf( file, "#                                                                                          #\n" );
	fprintf( file, "#  Be careful with adding new entries: if the format does not exactly match the above      #\n" );
	fprintf( file, "#  specifications, the game will probably be hanging at startup.                           #\n" );
	fprintf( file, "#  This file must contain at least one entry, the maximum limit is 128.                    #\n" );
	fprintf( file, "#                                                                                          #\n" );
	fprintf( file, "############################################################################################\n\n\n" );
	fprintf( file, "# Botname\t\tBotmodel\t\tAiming\tAggres.\tSensing\tChat\n" );
	fprintf( file, "# -----------------------------------------------------------------------------\n" );

	switch( mod_id )
	{
		case AG_DLL:
		case DMC_DLL:
		case VALVE_DLL:
		case GEARBOX_DLL:
			fprintf( file, "\"[PAS]Detonator\"\t\"scientist\"\t\t10\t10\t10\t8\n" );
			fprintf( file, "\"Charming\"\t\t\"gina\"\t\t\t9\t5\t8\t4\n" );
			fprintf( file, "\"Quantum Neuromancer\"\t\"helmet\"\t3\t3\t6\t5\n" );
			fprintf( file, "\"Renaissance\"\t\"recon\"\t\t4\t1\t8\t10\n" );
			fprintf( file, "\"Arnie\"\t\t\"hgrunt\"\t\t1\t7\t3\t2\n" );
			fprintf( file, "\"Alien Hunter\"\t\t\"zombie\"\t\t6\t5\t5\t6\n" );
			fprintf( file, "\"Lord Helmchen\"\t\"helmet\"\t\t7\t2\t6\t1\n" );
			fprintf( file, "\"Cool J.\"\t\t\"gordon\"\t\t8\t6\t9\t3\n" );
			fprintf( file, "\"Paranoid\"\t\t\"gman\"\t\t\t3\t9\t7\t9\n" );
			fprintf( file, "\"Blastaway\"\t\t\"cannibal\"\t\t10\t6\t3\t7\n" );
			fprintf( file, "\"Afterburner\"\t\t\"skeleton\"\t\t1\t8\t7\t4\n" );
			fprintf( file, "\"Dark Avenger\"\t\t\"scientist\"\t\t2\t5\t2\t6\n" );
			fprintf( file, "\"[RDZ]Pain\"\t\t\"hgrunt\"\t8\t7\t6\t2\n" );
			fprintf( file, "\"[POD]Headshot Deluxe\"\t\"robo\"\t\t\t8\t3\t9\t8\n" );
			fprintf( file, "\"[CGF]Event Horizon\"\t\"robo\"\t\t\t9\t4\t7\t5\n" );
			fprintf( file, "\"[HPB]Roots\"\t\t\"robo\"\t\t\t6\t8\t6\t9\n" );
			fprintf( file, "\"Desperado\"\t\t\"recon\"\t\t\t4\t10\t5\t7\n" );
			fprintf( file, "\"Don Juan\"\t\t\"barney\"\t\t2\t4\t4\t10\n" );
			fprintf( file, "\"[PAS]Bladerunner\"\t\"gordon\"\t\t7\t5\t10\t3\n" );
			fprintf( file, "\"Mad Max\"\t\t\"zombie\"\t4\t6\t5\t1" );
			break;
		case HOLYWARS_DLL:
			fprintf( file, "\"[PAS]Detonator\"\t\"bad\"\t\t\t10\t10\t10\t8\n" );
			fprintf( file, "\"Charming\"\t\t\"bad\"\t\t\t9\t5\t8\t4\n" );
			fprintf( file, "\"Quantum Neuromancer\"\t\"akedo\"\t\t3\t3\t6\t5\n" );
			fprintf( file, "\"Renaissance\"\t\"gordon\"\t\t4\t1\t8\t10\n" );
			fprintf( file, "\"Arnie\"\t\t\"akedo\"\t\t1\t7\t3\t2\n" );
			fprintf( file, "\"Alien Hunter\"\t\t\"bad\"\t\t6\t5\t5\t6\n" );
			fprintf( file, "\"Lord Helmchen\"\t\t\"helmet\"\t\t7\t2\t6\t1\n" );
			fprintf( file, "\"Cool J.\"\t\t\"gordon\"\t\t8\t6\t9\t3\n" );
			fprintf( file, "\"Paranoid\"\t\t\"helmet\"\t\t3\t9\t7\t9\n" );
			fprintf( file, "\"Blastaway\"\t\t\"gordon\"\t\t10\t6\t3\t7\n" );
			fprintf( file, "\"Afterburner\"\t\t\"gordon\"\t\t1\t8\t7\t4\n" );
			fprintf( file, "\"Dark Avenger\"\t\t\"akedo\"\t\t2\t5\t2\t6\n" );
			fprintf( file, "\"[RDZ]Pain\"\t\t\"akedo\"\t\t\t8\t7\t6\t2\n" );
			fprintf( file, "\"[POD]Headshot Deluxe\"\t\"robo\"\t\t\t8\t3\t9\t8\n" );
			fprintf( file, "\"[CGF]Event Horizon\"\t\"robo\"\t\t\t9\t4\t7\t5\n" );
			fprintf( file, "\"[HPB]Roots\"\t\t\"robo\"\t\t\t6\t8\t6\t9\n" );
			fprintf( file, "\"Desperado\"\t\t\"bad\"\t\t\t4\t10\t5\t7\n" );
			fprintf( file, "\"Don Juan\"\t\t\"bad\"\t\t\t2\t4\t4\t10\n" );
			fprintf( file, "\"[PAS]Bladerunner\"\t\"gordon\"\t\t7\t5\t10\t3\n" );
			fprintf( file, "\"Mad Max\"\t\t\"akedo\"\t\t4\t6\t5\t1" );
			break;
		case HUNGER_DLL:
			fprintf( file, "\"Don Juan\"\t\"civie\"\t\t2\t4\t4\t10\n" );
			fprintf( file, "\"[BWG]Dave Waters\"\t\t\"dave\"\t\t\t7\t6\t8\t6\n" );
			fprintf( file, "\"[BWG]Einar Saukas\"\t\"einar\"\t8\t3\t8\t5\n" );
			fprintf( file, "\"[BWG]Einar Saukas\"\t\"einarhev\"\t\t8\t3\t8\t5\n" );
			fprintf( file, "\"Dr. Franklin\"\t\t\"franklin\"\t\t1\t5\t3\t3\n" );
			fprintf( file, "\"Alien Hunter\"\t\t\"gangster\"\t\t6\t5\t5\t6\n" );
			fprintf( file, "\"[BWG]Jack Cooper\"\t\"jack\"\t\t7\t2\t6\t5\n" );
			fprintf( file, "\"[BWG]Magnus Bernekarr\"\t\t\"magnus\"\t\t8\t6\t9\t3\n" );
			fprintf( file, "\"[BWG]Neil Manke\"\t\t\"neil\"\t\t\t8\t7\t8\t9\n" );
			fprintf( file, "\"Nohead Zombie\"\t\t\"nohead\"\t\t1\t1\t1\t1\n" );
			fprintf( file, "\"Blastaway\"\t\t\"nypdcop\"\t\t10\t6\t3\t7\n" );
			fprintf( file, "\"[RDZ]Pain\"\t\t\"orderly\"\t\t8\t7\t6\t2\n" );
			fprintf( file, "\"Paranoid\"\t\t\"patient\"\t3\t9\t7\t9\n" );
			fprintf( file, "\"[BWG]Paul Taylor\"\t\"paul\"\t\t\t6\t5\t7\t5\n" );
			fprintf( file, "\"Chester Rockwood\"\t\t\"sheriff\"\t\t\t6\t5\t4\t9\n" );
			fprintf( file, "\"Desperado\"\t\t\"worker\"\t\t\t6\t4\t6\t7\n" );
			fprintf( file, "\"Zombie Ork\"\t\t\"zork\"\t\t10\t10\t10\t8\n" );
			fprintf( file, "\"[HPB]Roots\"\t\t\"civie\"\t\t\t6\t8\t6\t9\n" );
			fprintf( file, "\"[POD]Headshot Deluxe\"\t\"gangster\"\t\t\t8\t3\t9\t8\n" );
			fprintf( file, "\"Renaissance\"\t\"patient\"\t\t4\t1\t8\t10\n" );
			break;
	}

	fclose( file );

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
