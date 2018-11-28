// Based on:
//
// HPB bot - botman's High Ping Bastard bot
//
// (http://planethalflife.com/botman/)
//
// engine.cpp
//

#include <stdio.h>
#include "parabot.h"
#include "sounds.h"
#include "bot.h"
#include "bot_client.h"

// HolyWars: gets modified in writeString
bool haloOnBase = true;

extern char g_argv[256];
extern bot_t bots[32];
extern int mod_id;
extern float roundStartTime;
extern int isFakeClientCommand;
extern int fake_arg_count;
char valveTeamList[MAX_TEAMS][32];
int valveTeamNumber;
COMMON com;
const Vec3D zerovector = {0.0f, 0.0f, 0.0f};
const Vec3D nullvector = {0.0f, 0.0f, 1.0f};

void (*botMsgFunction)(void *, int) = NULL;
int botMsgIndex;

// messages created in RegUserMsg which will be "caught"
int message_VGUI;
int message_ShowMenu;
int message_WeaponList;
int message_CurWeapon;
int message_AmmoX;
int message_AmmoPickup;
int message_Damage;
int message_Death;
int message_Money;	// for Counter-Strike
int message_HLTV;
int message_SayText;

static const char *
cmd_args_wrap()
{
   if (isFakeClientCommand)
   {
	if(!(com.gamedll_flags & GAMEDLL_METAMOD))
		return &g_argv[0];

	RETURN_META_VALUE(MRES_SUPERCEDE, &g_argv[0]);
   }
   else
   {
	if(!(com.gamedll_flags & GAMEDLL_METAMOD))
		return cmd_args();

	RETURN_META_VALUE(MRES_IGNORED, NULL);
   }
}

static const char *
cmd_argv_wrap(int argc)
{
   if (isFakeClientCommand)
   {
      if (argc == 0)
      {
	if(!(com.gamedll_flags & GAMEDLL_METAMOD))
		return &g_argv[64];

	RETURN_META_VALUE(MRES_SUPERCEDE, &g_argv[64]);
      }
      else if (argc == 1)
      {
	if(!(com.gamedll_flags & GAMEDLL_METAMOD))
		return &g_argv[128];

	RETURN_META_VALUE(MRES_SUPERCEDE, &g_argv[128]);
      }
      else if (argc == 2)
      {
	if(!(com.gamedll_flags & GAMEDLL_METAMOD))
		return &g_argv[192];

	RETURN_META_VALUE(MRES_SUPERCEDE, &g_argv[192]);
      }
      else
      {
	if(!(com.gamedll_flags & GAMEDLL_METAMOD))
		return "???";

	RETURN_META_VALUE(MRES_SUPERCEDE, "???");
      }
   }
   else
   {
		const char *pargv = cmd_argv(argc);
		if(!(com.gamedll_flags & GAMEDLL_METAMOD))
			return pargv;

		RETURN_META_VALUE(MRES_SUPERCEDE, pargv);
   }
}

static int
cmd_argc_wrap()
{
   if (isFakeClientCommand)
   {
	if(!(com.gamedll_flags & GAMEDLL_METAMOD))
		return fake_arg_count;

	RETURN_META_VALUE(MRES_SUPERCEDE, fake_arg_count);
   }
   else
   {
	if(!(com.gamedll_flags & GAMEDLL_METAMOD))
		return cmd_argc();

	RETURN_META_VALUE(MRES_IGNORED, 0);
   }
}

static void
changelevel_wrap(const char *s1, const char *s2)
{
	debugFile("ChangeLevel:\n");

	// kick any bot off of the server after time/frag limit...
	for (int index = 0; index < 32; index++) {
		if (bots[index].is_used) { // is this slot used?
			char cmd[64];

			sprintf(cmd, "kick \"%s\"\n", bots[index].name);
			bots[index].respawn_state = RESPAWN_NEED_TO_RESPAWN;
			bots[index].is_used = false;

			servercommand(cmd); // kick the bot using (kick "name")
		}
	}

	if ((com.gamedll_flags & GAMEDLL_METAMOD))
		RETURN_META(MRES_IGNORED);

	changelevel(s1, s2);
}

static void
MSG_Begin_wrap(int msg_dest, int msg_type, const Vec3D *origin, EDICT *ed)
{
	int index = -1;

	debugFile("MSG_Begin: edict=%p dest=%d type=%d\n", ed, msg_dest, msg_type);

	if (msg_type == message_Death) {
		botMsgFunction = Client_Valve_DeathMsg;
	}

	if (ed) {
		index = getbotindex(ed);

		// is this message for a bot?
		if (index != -1) {
			botMsgFunction = NULL;  // no msg function until known otherwise
			botMsgIndex = index;    // index of bot receiving message

			switch (mod_id) {
			case AG_DLL:
			case VALVE_DLL:
				if (msg_type == message_WeaponList)
					botMsgFunction = BotClient_Valve_WeaponList;
				else if (msg_type == message_CurWeapon)
					botMsgFunction = BotClient_Valve_CurrentWeapon;
				else if (msg_type == message_AmmoX)
					botMsgFunction = BotClient_Valve_AmmoX;
				else if (msg_type == message_AmmoPickup)
					botMsgFunction = BotClient_Valve_AmmoPickup;
				else if (msg_type == message_Damage)
					botMsgFunction = BotClient_Valve_Damage;
				break;
			case HOLYWARS_DLL:
				if (msg_type == message_WeaponList)
					botMsgFunction = BotClient_Holywars_WeaponList;
				else if (msg_type == message_CurWeapon)
					botMsgFunction = BotClient_Holywars_CurrentWeapon;
				else if (msg_type == message_AmmoX)
					botMsgFunction = BotClient_Holywars_AmmoX;
				else if (msg_type == message_AmmoPickup)
					botMsgFunction = BotClient_Holywars_AmmoPickup;
				else if (msg_type == message_Damage)
					botMsgFunction = BotClient_Holywars_Damage;
				break;
			case DMC_DLL:
				if (msg_type == message_WeaponList)
					botMsgFunction = BotClient_DMC_WeaponList;
				else if (msg_type == message_CurWeapon)
					botMsgFunction = BotClient_DMC_CurrentWeapon;
				else if (msg_type == message_AmmoX)
					botMsgFunction = BotClient_DMC_AmmoX;
				else if (msg_type == message_AmmoPickup)
					botMsgFunction = BotClient_DMC_AmmoPickup;
				else if (msg_type == message_Damage)
					botMsgFunction = BotClient_DMC_Damage;
				break;
			case TFC_DLL:
				if (msg_type == message_VGUI)
					botMsgFunction = BotClient_TFC_VGUI;
				else if (msg_type == message_WeaponList)
					botMsgFunction = BotClient_TFC_WeaponList;
				else if (msg_type == message_CurWeapon)
					botMsgFunction = BotClient_TFC_CurrentWeapon;
				else if (msg_type == message_AmmoX)
					botMsgFunction = BotClient_TFC_AmmoX;
				else if (msg_type == message_AmmoPickup)
					botMsgFunction = BotClient_TFC_AmmoPickup;
				else if (msg_type == message_Damage)
					botMsgFunction = BotClient_TFC_Damage;
				break;
			case CSTRIKE_DLL:
				if (msg_type == message_VGUI)
					botMsgFunction = BotClient_CS_VGUI;
				else if (msg_type == message_ShowMenu)
					botMsgFunction = BotClient_CS_ShowMenu;
				else if (msg_type == message_WeaponList)
					botMsgFunction = BotClient_CS_WeaponList;
				else if (msg_type == message_CurWeapon)
					botMsgFunction = BotClient_CS_CurrentWeapon;
				else if (msg_type == message_AmmoX) 
					botMsgFunction = BotClient_CS_AmmoX;
				else if (msg_type == message_AmmoPickup)
					botMsgFunction = BotClient_CS_AmmoPickup;
				else if (msg_type == message_Damage)
					botMsgFunction = BotClient_CS_Damage;
				else if (msg_type == message_Money)
					botMsgFunction = BotClient_CS_Money;
				break;
			case GEARBOX_DLL:
				if (msg_type == message_VGUI)
                                        botMsgFunction = BotClient_Gearbox_VGUI;
				else if (msg_type == message_WeaponList)
					botMsgFunction = BotClient_Gearbox_WeaponList;
				else if (msg_type == message_CurWeapon)
					botMsgFunction = BotClient_Gearbox_CurrentWeapon;
				else if (msg_type == message_AmmoX)
					botMsgFunction = BotClient_Gearbox_AmmoX;
				else if (msg_type == message_AmmoPickup)
					botMsgFunction = BotClient_Gearbox_AmmoPickup;
				else if (msg_type == message_Damage)
					botMsgFunction = BotClient_Gearbox_Damage;
				break;
			case HUNGER_DLL:
				if (msg_type == message_WeaponList)
					botMsgFunction = BotClient_Hunger_WeaponList;
				else if (msg_type == message_CurWeapon)
					botMsgFunction = BotClient_Hunger_CurrentWeapon;
				else if (msg_type == message_AmmoX)
					botMsgFunction = BotClient_Hunger_AmmoX;
				else if (msg_type == message_AmmoPickup)
					botMsgFunction = BotClient_Hunger_AmmoPickup;
				else if (msg_type == message_Damage)
					botMsgFunction = BotClient_Hunger_Damage;
				break;
			}
		} else {
			// message for a human client
			if (msg_type == message_CurWeapon) {
				botMsgIndex = indexofedict(ed);
				botMsgFunction = HumanClient_CurrentWeapon;
			}
		}
	} else {
		// Steam makes the WeaponList message be sent differently

		botMsgFunction = NULL;  // no msg function until known otherwise
		botMsgIndex = -1;       // index of bot receiving message (none)

		if (mod_id == VALVE_DLL || mod_id == AG_DLL) {
			if (msg_type == message_WeaponList)
				botMsgFunction = BotClient_Valve_WeaponList;
		} else if (mod_id == TFC_DLL) {
			if (msg_type == message_WeaponList)
				botMsgFunction = BotClient_TFC_WeaponList;
		}
         else if (mod_id == CSTRIKE_DLL)
         {
            if (msg_type == message_WeaponList)
               botMsgFunction = BotClient_CS_WeaponList;
            else if (msg_type == message_HLTV)
               botMsgFunction = BotClient_CS_HLTV;
         }
         else if (mod_id == GEARBOX_DLL)
         {
            if (msg_type == message_WeaponList)
               botMsgFunction = BotClient_Gearbox_WeaponList;
         }
       else if (mod_id == DMC_DLL)
         {
            if (msg_type == message_WeaponList)
               botMsgFunction = BotClient_DMC_WeaponList;
         }
       else if (mod_id == HOLYWARS_DLL)
         {
            if (msg_type == message_WeaponList)
               botMsgFunction = BotClient_Holywars_WeaponList;
         }
       else if (mod_id == HUNGER_DLL)
         {
            if (msg_type == message_WeaponList)
               botMsgFunction = BotClient_Hunger_WeaponList;
         }
	/*else if (mod_id == FRONTLINE_DLL)
         {
            if (msg_type == message_WeaponList)
               botMsgFunction = BotClient_FLF_WeaponList;
         }*/
      }
	if(com.gamedll_flags & GAMEDLL_METAMOD)
		RETURN_META(MRES_IGNORED);

	MSG_Begin(msg_dest, msg_type, origin, ed);
}

static int
RegUserMSG_wrap(const char *name, int size)
{
	int msg = RegUserMSG(name, size);
	
	//debugFile ("RegUserMSG: name=%s msg=%d\n", name, msg);

	if (Q_STREQ (name, "WeaponList"))
		message_WeaponList = msg;
	else if (Q_STREQ (name, "CurWeapon"))
		message_CurWeapon = msg;
	else if (Q_STREQ (name, "AmmoX"))
		message_AmmoX = msg;
	else if (Q_STREQ (name, "AmmoPickup"))
		message_AmmoPickup = msg;
	else if (Q_STREQ (name, "Damage"))
		message_Damage = msg;
	else if (Q_STREQ (name, "DeathMsg"))
		message_Death = msg;
	// TFC / CS
	else if (Q_STREQ (name, "VGUIMenu"))
		message_VGUI = msg;
	// CS only			
	else if (Q_STREQ (name, "ShowMenu"))
		message_ShowMenu = msg;
	else if (Q_STREQ (name, "Money"))
		message_Money = msg;         		
	else if (Q_STREQ (name, "SayText"))
		message_SayText = msg;

	if (!(com.gamedll_flags & GAMEDLL_METAMOD))
		return msg;

	RETURN_META_VALUE(MRES_SUPERCEDE, msg);
}

static void
MSG_End_wrap()
{
	debugFile("MSG_End:\n");

	// clear out the bot message function pointer...
	botMsgFunction = NULL;

	if ((com.gamedll_flags & GAMEDLL_METAMOD))
		RETURN_META (MRES_IGNORED);

	MSG_End();
}

///////////////////////////////////////////////////////////////////////////////////
//
//  FORWARD ENGINE FUNCTIONS...
//
///////////////////////////////////////////////////////////////////////////////////
static EDICT *
find_entitybystring_wrap(EDICT *start, const char *field, const char *value)
{
	debugFile("pfnFindEntityByString: %s\n",value);

	if ((mod_id == CSTRIKE_DLL) &&
	    (Q_STREQ(field, "classname")) && 
		(Q_STREQ(value, "info_map_parameters"))) {
		// DEBUG_MSG("NEW CS-ROUND!\n");
		roundStartTime = worldtime() + freezetime->value;	// 5 seconds until round starts
	}

	if (!(com.gamedll_flags & GAMEDLL_METAMOD))
		return find_entitybystring(start, field, value);

	RETURN_META_VALUE(MRES_IGNORED, 0);
}

static void
sound_wrap(EDICT *entity, int channel, const char *sample, float volume, float attenuation, int flags, int pitch)
{
	sounds_parseSound(entity, sample, volume);

	debugFile("EmitSound:\n");

	if (com.gamedll_flags & GAMEDLL_METAMOD)
		RETURN_META(MRES_IGNORED);

	sound(entity, channel, sample, volume, attenuation, flags, pitch);
}

static void
ambientsound_wrap(EDICT *e, Vec3D *pos, const char *samp, float vol, float attenuation, int flags, int pitch)
{
	sounds_parseAmbientSound(e, samp, vol);

	debugFile("pfnEmitAmbientSound:\n");

	if(com.gamedll_flags & GAMEDLL_METAMOD)
		RETURN_META(MRES_IGNORED);

	ambientsound(e, pos, samp, vol, attenuation, flags, pitch);
}

static void
clientcommand_wrap(EDICT* player, const char* fmt, ...)
{
	debugFile("ClientCommand=%s\n", fmt);

	if (!(com.gamedll_flags & GAMEDLL_METAMOD)) {
		if (!(player->v.flags & FL_FAKECLIENT)) {
			char tempFmt[1024];

			va_list argp;
			va_start(argp, fmt);
			vsprintf(tempFmt, fmt, argp);
			va_end(argp);

			CLIENT_COMMAND(player, tempFmt);
		}
		return;
	}

	if (!(player->v.flags & FL_FAKECLIENT))
		RETURN_META(MRES_IGNORED);

	RETURN_META(MRES_SUPERCEDE);
}

static void
MSG_WriteByte_wrap(int value)
{
	debugFile("MSG_WriteByte: %d\n", value);

	// if this message is for a bot, call the client message function...
	if (botMsgFunction)
		(*botMsgFunction)((void*)&value, botMsgIndex);

	if (com.gamedll_flags & GAMEDLL_METAMOD)
		RETURN_META(MRES_IGNORED);

	MSG_WriteByte(value);
}

static void
MSG_WriteChar_wrap(int value)
{
	debugFile("MSG_WriteChar: %d\n", value);

	// if this message is for a bot, call the client message function...
	if (botMsgFunction)
		(*botMsgFunction)((void*)&value, botMsgIndex);

	if (com.gamedll_flags & GAMEDLL_METAMOD)
		RETURN_META(MRES_IGNORED);

	MSG_WriteChar(value);
}

static void
MSG_WriteShort_wrap(int value)
{
	debugFile("MSG_WriteShort: %d\n", value);

	// if this message is for a bot, call the client message function...
	if (botMsgFunction)
		(*botMsgFunction)((void*)&value, botMsgIndex);

	if (com.gamedll_flags & GAMEDLL_METAMOD)
		RETURN_META(MRES_IGNORED);

	MSG_WriteShort(value);
}

static void
MSG_WriteLong_wrap(int value)
{
	debugFile("MSG_WriteLong: %d\n", value);

	// if this message is for a bot, call the client message function...
	if (botMsgFunction)
		(*botMsgFunction)((void*)&value, botMsgIndex);

	if (com.gamedll_flags & GAMEDLL_METAMOD)
		RETURN_META(MRES_IGNORED);

	MSG_WriteLong(value);
}

static void
MSG_WriteAngle_wrap(float value)
{
	debugFile("MSG_WriteAngle: %f\n", value);

	// if this message is for a bot, call the client message function...
	if (botMsgFunction)
		(*botMsgFunction)((void*)&value, botMsgIndex);

	if (com.gamedll_flags & GAMEDLL_METAMOD)
		RETURN_META(MRES_IGNORED);

	MSG_WriteAngle(value);
}

static void
MSG_WriteCoord_wrap(float value)
{
	debugFile("MSG_WriteCoord: %f\n", value);

	// if this message is for a bot, call the client message function...
	if (botMsgFunction)
		(*botMsgFunction)((void*)&value, botMsgIndex);

	if (com.gamedll_flags & GAMEDLL_METAMOD)
		RETURN_META(MRES_IGNORED);

	MSG_WriteCoord(value);
}

static void
MSG_WriteString_wrap(const char *sz)
{
	if (mod_id == HOLYWARS_DLL) {
		if (!strncmp(sz, "The halo disappeared", 10)) {
			// DEBUG_MSG("HALO ON BASE!\n");
			haloOnBase = true;
		} else if (!strncmp(sz, "We've got a new saint", 3)) {
			// DEBUG_MSG("HALO NOT ON BASE!\n");
			haloOnBase = false;
		}
	}
	// DEBUG_MSG("MSG: %s\n", sz);
	debugFile("MSG_WriteString: %s\n", sz);

	// if this message is for a bot, call the client message function...
	if (botMsgFunction)
		(*botMsgFunction)((void*)sz, botMsgIndex);

	if (com.gamedll_flags & GAMEDLL_METAMOD)
		RETURN_META(MRES_IGNORED);

	MSG_WriteString(sz);
}

static void
MSG_WriteEntity_wrap(int value)
{
	debugFile("MSG_WriteEntity: %d\n", value);

	// if this message is for a bot, call the client message function...
	if (botMsgFunction)
		(*botMsgFunction)((void*)&value, botMsgIndex);

	if (com.gamedll_flags & GAMEDLL_METAMOD)
		RETURN_META(MRES_IGNORED);

	MSG_WriteEntity(value);
}

static void
setclientkeyvalue_wrap(int clientIndex, const char *infobuffer, const char *key, const char *value)
{
	if (!(com.gamedll_flags & GAMEDLL_TEAMPLAY) && Q_STREQ(key, "team")) {	// init teamlist
		com.gamedll_flags |= GAMEDLL_TEAMPLAY;

		bool teamKnown = false;
		for (int team = 0; team < valveTeamNumber; team++) {
			if (Q_STREQ(value, valveTeamList[team])) {
				teamKnown = true;
				break;
			}
		}
		if (!teamKnown && valveTeamNumber < MAX_TEAMS) {
			strcpy(valveTeamList[valveTeamNumber], value);
			DEBUG_MSG("Registered team %s\n", value);
			valveTeamNumber++;
		}
	}
	debugFile("SetClientKeyValue: %s %s\n", key, value);

	if (com.gamedll_flags & GAMEDLL_METAMOD)
		RETURN_META(MRES_IGNORED);

	setclientkeyvalue(clientIndex, infobuffer, key, value);
}

extern "C" int EXPORT
GetEngineFunctions(ENGINEAPI *engfuncs, int *interfaceVersion)
{
	engfuncs->Cmd_Argc = cmd_argc_wrap;
	engfuncs->Cmd_Argv = cmd_argv_wrap;
	engfuncs->Cmd_Args = cmd_args_wrap;
	engfuncs->Changelevel = changelevel_wrap;
	engfuncs->MessageBegin = MSG_Begin_wrap;
	engfuncs->RegUserMsg = RegUserMSG_wrap;
	engfuncs->MessageEnd = MSG_End_wrap;
	engfuncs->FindEntityByString = find_entitybystring_wrap;
	engfuncs->EmitSound = sound_wrap;
	engfuncs->EmitAmbientSound = ambientsound_wrap;
	engfuncs->ClientCommand = clientcommand_wrap;
	engfuncs->WriteByte = MSG_WriteByte_wrap;
	engfuncs->WriteChar = MSG_WriteChar_wrap;
	engfuncs->WriteShort = MSG_WriteShort_wrap;
	engfuncs->WriteLong = MSG_WriteLong_wrap;
	engfuncs->WriteAngle = MSG_WriteAngle_wrap;
	engfuncs->WriteCoord = MSG_WriteCoord_wrap;
	engfuncs->WriteString = MSG_WriteString_wrap;
	engfuncs->WriteEntity = MSG_WriteEntity_wrap;
	engfuncs->SetClientKeyValue = setclientkeyvalue_wrap;

	return true;
}

#if _WIN32
// Required DLL entry point
BOOL WINAPI
DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
	}
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
		if(!(com.gamedll_flags & GAMEDLL_METAMOD)) {
			if (com.gamedll_handle)
				FreeLibrary( com.gamedll_handle );
		}
		chat_free();
/*
		// try to close ole
		gpITTSCentral->Release();
		
		if (!EndOLE())
			ERROR_MSG( "Can't shut down OLE." );*/
	}
	
	return TRUE;
}
#endif

extern "C" void DLLEXPORT WINAPI
GiveFnptrsToDll(ENGINEAPI *engfuncs, GLOBALENTVARS *globals)
{
	const char *gamedll;
	char game_dir[256], filePath[100];
	int pos = 0;

	com.engfuncs = (ENGINEAPI*)malloc(sizeof(*engfuncs));
	memcpy(com.engfuncs, engfuncs, sizeof(*engfuncs));

	com.globals = globals;

	// find the directory name of the currently running MOD...
	getgamedir( game_dir );

	if(strstr(game_dir,"/")) {
		pos = strlen( game_dir ) - 1;

		// scan backwards till first directory separator...
		while ((pos > 0) && (game_dir[pos] != '/'))
			pos--;
		if (pos == 0)
			ERROR_MSG( "Error determining MOD directory name!" );

		pos++;
	}
	strcpy( com.modname, &game_dir[pos] );

	if( Q_STREQ( com.modname, "ag" ) )
	{
		mod_id = AG_DLL;
	}
	else if( Q_STREQ( com.modname, "Hunger" ) )
	{
		mod_id = HUNGER_DLL;
	}
	else if( Q_STREQ( com.modname, "holywars" ) )
	{
		mod_id = HOLYWARS_DLL;
	}
	else if( Q_STREQ( com.modname, "dmc" ) )
	{
		mod_id = DMC_DLL;
	}
	else if( Q_STREQ( com.modname, "gearbox" ) )
	{
		mod_id = GEARBOX_DLL;
	}
	else
		mod_id = VALVE_DLL;

	strcpy( filePath, com.modname );
	strcat( filePath, "/addons/parabot/config/" );
#if defined(__ANDROID__)
	if( UTIL_FileExists( filePath ) )
	{
		FILE *pfile = fopen( getenv( "PARABOT_EXTRAS_PAK" ), "rb" );
		if( pfile )
		{
			extrpak( pfile, com.modname );
			fclose( pfile );
		}
	}
#endif

	pos = strlen( com.modname );
	filePath[pos] = '\0';
	strcat( filePath, "/addons/parabot/log");
	CreateDirectory( filePath, NULL );

	if(!(com.gamedll_flags & GAMEDLL_METAMOD))
	{
#if defined(__ANDROID__)
#if LOAD_HARDFP
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
		com.gamedll_handle = LoadLibrary( filePath );

		if (com.gamedll_handle == NULL) {	// Directory error or Unsupported MOD!
			ERROR_MSG( "MOD gamedll %s not found (or unsupported MOD)!\n", filePath );
			debugFile( "Library = 0\n" );
			exit(0);
		}

		GetEngineFunctions(engfuncs, NULL);

		// give the engine functions to the other DLL...	
		(*(GIVEFNPTRSTODLL) GetProcAddress (com.gamedll_handle, "GiveFnptrsToDll")) (engfuncs, globals);
	}	
}
