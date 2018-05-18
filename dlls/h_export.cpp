// Based on:
//
// HPB bot - botman's High Ping Bastard bot
//
// (http://planethalflife.com/botman/)
//
// h_export.cpp
//

#include "extdll.h"
#include "enginecallback.h"
#include "dllapi.h"
#include "meta_api.h"
#include "entity_state.h"
#include "bot.h"
#include "engine.h"
#include "pb_global.h"
#include "pb_configuration.h"
#include "pb_chat.h"
#include "pakextractor.h"

extern int mod_id;
PB_Configuration pbConfig;
PB_Chat chat;
HINSTANCE h_Library = NULL;
char mod_name[32];
unsigned int g_uiGameFlags;
enginefuncs_t g_engfuncs;
globalvars_t  *gpGlobals;
GETENTITYAPI other_GetEntityAPI;
GIVEFNPTRSTODLL other_GiveFnptrsToDll;
GETNEWDLLFUNCTIONS other_GetNewDLLFunctions; 

float sineTable[256];					// sine table for e.g. look-arounds

void initSineTable()
{
	for (int i=0; i<256; i++) {
		float f = (float) i;
		f *= 2*3.1415927/256;
		sineTable[i] = sin( f );
	}
}



#ifdef _WIN32
// Required DLL entry point
BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
	}
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
		if( !FBitSet( g_uiGameFlags, GAME_METAMOD ) )
		{
			if (h_Library)
				FreeLibrary( h_Library );
		}
		chat.free();
/*
		// try to close ole
		gpITTSCentral->Release();
		
		if (!EndOLE())
			errorMsg( "Can't shut down OLE." );*/
	}
	
	return TRUE;
}
#endif


extern "C" void DLLEXPORT WINAPI GiveFnptrsToDll( enginefuncs_t* pengfuncsFromEngine, globalvars_t *pGlobals )
{
	const char *gamedll;
	char game_dir[256], filePath[100];
	int pos = 0;

	// get the engine functions from the engine...
	memcpy( &g_engfuncs, pengfuncsFromEngine, sizeof(enginefuncs_t) );
	gpGlobals = pGlobals;
	
	// find the directory name of the currently running MOD...
	(*g_engfuncs.pfnGetGameDir)( game_dir );

	if(strstr(game_dir,"/"))
	{
		pos = strlen( game_dir ) - 1;

		// scan backwards till first directory separator...
		while ((pos > 0) && (game_dir[pos] != '/'))
			pos--;
		if (pos == 0)
			errorMsg( "Error determining MOD directory name!" );

		pos++;
	}
	strcpy( mod_name, &game_dir[pos] );
	
	if( FStrEq( mod_name, "ag" ) )
	{
		mod_id = AG_DLL;
	}
	else if( FStrEq( mod_name, "Hunger" ) )
	{
		mod_id = HUNGER_DLL;
	}
	else if( FStrEq( mod_name, "holywars" ) )
	{
		mod_id = HOLYWARS_DLL;
	}
	else if( FStrEq( mod_name, "dmc" ) )
	{
		mod_id = DMC_DLL;
	}
	else if( FStrEq( mod_name, "gearbox" ) )
	{
		mod_id = GEARBOX_DLL;
	}
	else
		mod_id = VALVE_DLL;

	sprintf( filePath, "%s/addons/parabot/config/", mod_name );
#if defined(__ANDROID__)
	struct stat checkdir;
	if( 0 > stat( filePath, &checkdir ) )
	{
		FILE *pfile = fopen( getenv( "PARABOT_EXTRAS_PAK" ), "rb" );
		if( pfile )
		{
			extrpak( pfile, mod_name );
			fclose( pfile );
		}
	}
#endif
	strcat( filePath, mod_name );
	strcat( filePath, "/" );
	pbConfig.initConfiguration( filePath );
	pbConfig.initPersonalities( filePath );

	pos = strlen( mod_name );
	filePath[pos] = '\0';
	strcat( filePath, "/addons/parabot/log");
	CreateDirectory( filePath, NULL );

	// always load chatfile, might be enabled ingame:
	filePath[pos] = '\0';
	strcat( filePath, "/addons/parabot/config/lang/" );
	strcat( filePath, pbConfig.chatFile() );
	chat.load( filePath );
	initSineTable();

	if( !FBitSet( g_uiGameFlags, GAME_METAMOD ) )
	{
#if defined(__ANDROID__)
#ifdef LOAD_HARDFP
		gamedll = "libserver_hardfp.so";
#else
		gamedll = "libserver.so";
#endif
		snprintf( filePath, sizeof(filePath), "%s/%s", getenv( "XASH3D_GAMELIBDIR" ), serverdll );
#else
		filePath[pos]= '\0';

		switch( mod_id )
		{
			case AG_DLL:
				gamedll = "/dlls/ag."OS_LIB_EXT;
				break;
			default:
			case VALVE_DLL:
				gamedll = "/dlls/hl."OS_LIB_EXT;
				break;
			case DMC_DLL:
				gamedll = "/dlls/dmc."OS_LIB_EXT;
				break;
			case GEARBOX_DLL:
				gamedll = "/dlls/opfor."OS_LIB_EXT;
				break;
			case HOLYWARS_DLL:
				gamedll = "/dlls/holywars."OS_LIB_EXT;
				break;
			case HUNGER_DLL:
                                gamedll = "/dlls/einar."OS_LIB_EXT;
                                break;
		}
		strcat( filePath, gamedll );
#endif
		h_Library = LoadLibrary( filePath );

		if (h_Library == NULL) {	// Directory error or Unsupported MOD!
			errorMsg( "MOD Dll not found (or unsupported MOD)!" );
			debugFile( "Library = 0\n" );
		}

		GetEngineFunctions( pengfuncsFromEngine, NULL );

		// give the engine functions to the other DLL...	
		(*(GIVEFNPTRSTODLL) GetProcAddress (h_Library, "GiveFnptrsToDll")) (pengfuncsFromEngine, pGlobals);
	}	
}

extern "C" EXPORT int Server_GetBlendingInterface( int version, struct sv_blending_interface_s **ppinterface, struct engine_studio_api_s *pstudio, float (*rotationmatrix)[3][4], float (*bonetransform)[MAXSTUDIOBONES][3][4] )
{
	static SERVER_GETBLENDINGINTERFACE other_Server_GetBlendingInterface = NULL;
	static bool missing = FALSE;

	// if the blending interface has been formerly reported as missing, give up
	if (missing)
		return FALSE;

	// do we NOT know if the blending interface is provided ? if so, look for its address
	if (other_Server_GetBlendingInterface == NULL)
		other_Server_GetBlendingInterface = (SERVER_GETBLENDINGINTERFACE) GetProcAddress (h_Library, "Server_GetBlendingInterface");

	// have we NOT found it ?
	if (!other_Server_GetBlendingInterface)
	{
		missing = TRUE; // then mark it as missing, no use to look for it again in the future
		return FALSE; // and give up
	}

	// else call the function that provides the blending interface on request
	return ((other_Server_GetBlendingInterface) (version, ppinterface, pstudio, rotationmatrix, bonetransform));
}
