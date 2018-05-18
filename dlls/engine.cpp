// Based on:
//
// HPB bot - botman's High Ping Bastard bot
//
// (http://planethalflife.com/botman/)
//
// engine.cpp
//

#include "extdll.h"
#include "dllapi.h"
#include "meta_api.h"
#include "entity_state.h"
#include "pb_global.h"
#include "sounds.h"

#include "bot.h"
#include "bot_client.h"
#include "engine.h"
// HolyWars: gets modified in writeString
bool haloOnBase = true;

extern enginefuncs_t g_engfuncs;
extern bot_t bots[32];
extern int mod_id;
extern float roundStartTime;
extern Sounds playerSounds;

char valveTeamList[MAX_TEAMS][32] = { "","","","","","","","","","","","","","","","" };
int  valveTeamNumber = 0;

void (*botMsgFunction)(void *, int) = NULL;
int botMsgIndex;

// messages created in RegUserMsg which will be "caught"
int message_VGUI = 0;
int message_ShowMenu = 0;
int message_WeaponList = 0;
int message_CurWeapon = 0;
int message_AmmoX = 0;
int message_AmmoPickup = 0;
int message_Damage = 0;
int message_Death = 0;
int message_Money = 0;  // for Counter-Strike
int message_HLTV = 0;

void pfnChangeLevel(const char *s1, const char *s2)
{
   debugFile("pfnChangeLevel:\n");

   // kick any bot off of the server after time/frag limit...
   for (int index = 0; index < 32; index++)
   {
      if (bots[index].is_used)  // is this slot used?
      {
         char cmd[40];

         sprintf(cmd, "kick \"%s\"\n", bots[index].name);

         bots[index].respawn_state = RESPAWN_NEED_TO_RESPAWN;
		 bots[index].is_used = false;

         SERVER_COMMAND(cmd);  // kick the bot using (kick "name")
      }
   }
	if(FBitSet( g_uiGameFlags, GAME_METAMOD ))
		RETURN_META(MRES_IGNORED);

	(*g_engfuncs.pfnChangeLevel)(s1, s2);
}


void pfnMessageBegin(int msg_dest, int msg_type, const float *pOrigin, edict_t *ed)
{
   if (gpGlobals->deathmatch)
   {
      int index = -1;

      debugFile("pfnMessageBegin: edict=%p dest=%d type=%d\n",ed,msg_dest,msg_type);

	  if (msg_type == message_Death) {
		  botMsgFunction = Client_Valve_DeathMsg;
	  }

      if (ed)
      {
		  
         index = UTIL_GetBotIndex(ed);

         // is this message for a bot?
         if (index != -1)
         {
            botMsgFunction = NULL;  // no msg function until known otherwise
            botMsgIndex = index;    // index of bot receiving message

            switch( mod_id ) {
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
         }
		 else {	// message for a human client
			if (msg_type == message_CurWeapon) {
				 botMsgIndex = ENTINDEX( ed );
				 botMsgFunction = HumanClient_CurrentWeapon;
			 }
		 }
      }
   else
      {
         // Steam makes the WeaponList message be sent differently

         botMsgFunction = NULL;  // no msg function until known otherwise
         botMsgIndex = -1;       // index of bot receiving message (none)

         if (mod_id == VALVE_DLL || mod_id == AG_DLL)
         {
            if (msg_type == message_WeaponList)
               botMsgFunction = BotClient_Valve_WeaponList;
         }
         else if (mod_id == TFC_DLL)
         {
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
}
	if(FBitSet( g_uiGameFlags, GAME_METAMOD ))
		RETURN_META(MRES_IGNORED);

	(*g_engfuncs.pfnMessageBegin)(msg_dest, msg_type, pOrigin, ed);
}

int pfnRegUserMsg(const char *pszName, int iSize)
{
	int msg = (*g_engfuncs.pfnRegUserMsg)(pszName, iSize);
	
	if (gpGlobals->deathmatch)
	{
		//debugFile("pfnRegUserMsg: pszName=%s msg=%d\n",pszName,msg);

			 if (FStrEq( pszName, "WeaponList" ) )	message_WeaponList = msg;
		else if (FStrEq( pszName, "CurWeapon"	) )	message_CurWeapon = msg;
		else if (FStrEq( pszName, "AmmoX"       ) )	message_AmmoX = msg;
		else if (FStrEq( pszName, "AmmoPickup"  ) )	message_AmmoPickup = msg;
		else if (FStrEq( pszName, "Damage"      ) )	message_Damage = msg;
		else if (FStrEq( pszName, "DeathMsg"    ) )	message_Death = msg;
		// TFC / CS
		else if (FStrEq(pszName, "VGUIMenu"	) ) message_VGUI = msg;
		// CS only			
		else if (FStrEq(pszName, "ShowMenu"	) ) message_ShowMenu = msg;
		else if (FStrEq(pszName, "Money"	) ) message_Money = msg;         		
	}
	if(!FBitSet( g_uiGameFlags, GAME_METAMOD ))
		return msg;

	RETURN_META_VALUE(MRES_SUPERCEDE, msg);
}

void pfnMessageEnd(void)
{
   if (gpGlobals->deathmatch)
   {
      debugFile("pfnMessageEnd:\n");

      // clear out the bot message function pointer...
      botMsgFunction = NULL;
   }
	if(FBitSet( g_uiGameFlags, GAME_METAMOD ))
		RETURN_META(MRES_IGNORED);

	(*g_engfuncs.pfnMessageEnd)();
}

///////////////////////////////////////////////////////////////////////////////////
//
//  FORWARD ENGINE FUNCTIONS...
//
///////////////////////////////////////////////////////////////////////////////////
edict_t* pfnFindEntityByString(edict_t *pEdictStartSearchAfter, const char *pszField, const char *pszValue)
{
	debugFile("pfnFindEntityByString: %s\n",pszValue);

	if (( mod_id == CSTRIKE_DLL ) &&
		( FStrEq( pszField, "classname" ) ) && 
		( FStrEq( pszValue, "info_map_parameters" ) ) ) {
		//debugMsg( "NEW CS-ROUND!\n" );
		roundStartTime = worldTime() + freezetime->value;	// 5 seconds until round starts
	}

	if(!FBitSet( g_uiGameFlags, GAME_METAMOD ))
		return (*g_engfuncs.pfnFindEntityByString)(pEdictStartSearchAfter, pszField, pszValue);

	RETURN_META_VALUE(MRES_IGNORED, 0);
}

void pfnEmitSound(edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch)
{
	playerSounds.parseSound( entity, sample, volume );

	debugFile("pfnEmitSound:\n");

	if(FBitSet( g_uiGameFlags, GAME_METAMOD ))
		RETURN_META(MRES_IGNORED);

	(*g_engfuncs.pfnEmitSound)(entity, channel, sample, volume, attenuation, fFlags, pitch);
}

void pfnEmitAmbientSound(edict_t *entity, float *pos, const char *samp, float vol, float attenuation, int fFlags, int pitch)
{
	playerSounds.parseAmbientSound( entity, samp, vol );

	debugFile("pfnEmitAmbientSound:\n");

	if(FBitSet( g_uiGameFlags, GAME_METAMOD ))
		RETURN_META(MRES_IGNORED);

	(*g_engfuncs.pfnEmitAmbientSound)(entity, pos, samp, vol, attenuation, fFlags, pitch);
}

void pfnServerCommand(const char* str)
{
	infoMsg( "ServerCommand: %s\n", str );

	debugFile("pfnServerCommand: %s\n",str);

	if(FBitSet( g_uiGameFlags, GAME_METAMOD ))
		RETURN_META(MRES_IGNORED);

	(*g_engfuncs.pfnServerCommand)(str);
}

void pfnClientCommand(edict_t* pEdict, const char* szFmt, ...)
{
	debugFile( "pfnClientCommand=%s\n",szFmt);

	if(!FBitSet( g_uiGameFlags, GAME_METAMOD ))
	{
		if (!(pEdict->v.flags & FL_FAKECLIENT))
		{
			char tempFmt[256];
			va_list argp;
			va_start(argp, szFmt);
			vsprintf(tempFmt, szFmt, argp);
			(*g_engfuncs.pfnClientCommand)(pEdict, tempFmt);
			va_end(argp);
		}
		return;
	}
	else
	{
		if (!(pEdict->v.flags & FL_FAKECLIENT))
			RETURN_META(MRES_IGNORED);

		RETURN_META(MRES_SUPERCEDE);
	}
}

void pfnWriteByte(int iValue)
{
   if (gpGlobals->deathmatch)
   {
      debugFile("pfnWriteByte: %d\n",iValue);

      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&iValue, botMsgIndex);
   }
	if(FBitSet( g_uiGameFlags, GAME_METAMOD ))
		RETURN_META(MRES_IGNORED);

	(*g_engfuncs.pfnWriteByte)(iValue);
}

void pfnWriteChar(int iValue)
{
   if (gpGlobals->deathmatch)
   {
      debugFile("pfnWriteChar: %d\n",iValue);

      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&iValue, botMsgIndex);
   }
	if(FBitSet( g_uiGameFlags, GAME_METAMOD ))
		RETURN_META(MRES_IGNORED);

	(*g_engfuncs.pfnWriteChar)(iValue);
}

void pfnWriteShort(int iValue)
{
   if (gpGlobals->deathmatch)
   {
      debugFile("prnWriteShort: %d\n",iValue);

      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&iValue, botMsgIndex);
   }
	if(FBitSet( g_uiGameFlags, GAME_METAMOD ))
		RETURN_META(MRES_IGNORED);

	(*g_engfuncs.pfnWriteShort)(iValue);
}

void pfnWriteLong(int iValue)
{
   if (gpGlobals->deathmatch)
   {
      debugFile("pfnWriteLong: %d\n",iValue);

      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&iValue, botMsgIndex);
   }
	if(FBitSet( g_uiGameFlags, GAME_METAMOD ))
		RETURN_META(MRES_IGNORED);

	(*g_engfuncs.pfnWriteLong)(iValue);
}

void pfnWriteAngle(float flValue)
{
   if (gpGlobals->deathmatch)
   {
      debugFile("pfnWriteAngle: %f\n",flValue);

      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&flValue, botMsgIndex);
   }
	if(FBitSet( g_uiGameFlags, GAME_METAMOD ))
		RETURN_META(MRES_IGNORED);

	(*g_engfuncs.pfnWriteAngle)(flValue);
}

void pfnWriteCoord(float flValue)
{
   if (gpGlobals->deathmatch)
   {
      debugFile("pfnWriteCoord: %f\n",flValue);

      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&flValue, botMsgIndex);
   }
	if(FBitSet( g_uiGameFlags, GAME_METAMOD ))
		RETURN_META(MRES_IGNORED);

	(*g_engfuncs.pfnWriteCoord)(flValue);
}

void pfnWriteString(const char *sz)
{
   if (gpGlobals->deathmatch)
   {
	   if (mod_id==HOLYWARS_DLL) {
		   if ( strncmp(sz, "The halo disappeared", 10)==0 ) {
			   //debugMsg( "HALO ON BASE!\n" );
			   haloOnBase = true;
		   }
		   else if ( strncmp(sz, "We've got a new saint", 3)==0 ) {
			   //debugMsg( "HALO NOT ON BASE!\n" );
			   haloOnBase = false;
		   }
	   }
	   //debugMsg( "MSG: %s\n", sz );
      debugFile("pfnWriteString: %s\n",sz);

      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)sz, botMsgIndex);
   }
	if(FBitSet( g_uiGameFlags, GAME_METAMOD ))
		RETURN_META(MRES_IGNORED);

	(*g_engfuncs.pfnWriteString)(sz);
}

void pfnWriteEntity(int iValue)
{
   if (gpGlobals->deathmatch)
   {
	debugFile("pfnWriteEntity: %d\n",iValue);

      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&iValue, botMsgIndex);
   }
	if(FBitSet( g_uiGameFlags, GAME_METAMOD ))
		RETURN_META(MRES_IGNORED);

	(*g_engfuncs.pfnWriteEntity)(iValue);
}

void pfnSetClientKeyValue( int clientIndex, const char *infobuffer, const char *key, const char *value )
{
	if ((mod_id == VALVE_DLL || mod_id == DMC_DLL || mod_id == HUNGER_DLL || mod_id == GEARBOX_DLL) && (FStrEq( key, "team" ) ) ) {	// init teamlist
		if( !FBitSet( g_uiGameFlags, GAME_TEAMPLAY ))
			SetBits( g_uiGameFlags, GAME_TEAMPLAY );

		bool teamKnown = false;
		for (int team=0; team<valveTeamNumber; team++) 
			if (FStrEq( value, valveTeamList[team] ) ) {
				teamKnown = true;
				break;
			}
		if (!teamKnown && valveTeamNumber<MAX_TEAMS) {
			strcpy( valveTeamList[valveTeamNumber], value );
			debugMsg( "Registered team %s\n", value );
			valveTeamNumber++;
		}
	}
	debugFile("pfnSetClientKeyValue: %s %s\n",key,value);

	if(FBitSet( g_uiGameFlags, GAME_METAMOD ))
		RETURN_META(MRES_IGNORED);

	(*g_engfuncs.pfnSetClientKeyValue)(clientIndex, infobuffer, key, value);
}

extern "C" int EXPORT GetEngineFunctions(enginefuncs_t *pengfuncsFromEngine, int *interfaceVersion)
{
	pengfuncsFromEngine->pfnCmd_Argc = Cmd_Argc;
	pengfuncsFromEngine->pfnCmd_Argv = Cmd_Argv;
	pengfuncsFromEngine->pfnCmd_Args = Cmd_Args;
	pengfuncsFromEngine->pfnChangeLevel = pfnChangeLevel;
	pengfuncsFromEngine->pfnMessageBegin = pfnMessageBegin;
	pengfuncsFromEngine->pfnRegUserMsg = pfnRegUserMsg;
	pengfuncsFromEngine->pfnMessageEnd = pfnMessageEnd;
	pengfuncsFromEngine->pfnFindEntityByString = pfnFindEntityByString;
	pengfuncsFromEngine->pfnEmitSound = pfnEmitSound;
	pengfuncsFromEngine->pfnEmitAmbientSound = pfnEmitAmbientSound;
	pengfuncsFromEngine->pfnServerCommand = pfnServerCommand;
	pengfuncsFromEngine->pfnClientCommand = pfnClientCommand;
	pengfuncsFromEngine->pfnWriteByte = pfnWriteByte;
	pengfuncsFromEngine->pfnWriteChar = pfnWriteChar;
	pengfuncsFromEngine->pfnWriteShort = pfnWriteShort;
	pengfuncsFromEngine->pfnWriteLong = pfnWriteLong;
	pengfuncsFromEngine->pfnWriteAngle = pfnWriteAngle;
	pengfuncsFromEngine->pfnWriteCoord = pfnWriteCoord;
	pengfuncsFromEngine->pfnWriteString = pfnWriteString;
	pengfuncsFromEngine->pfnWriteEntity = pfnWriteEntity;
	pengfuncsFromEngine->pfnSetClientKeyValue = pfnSetClientKeyValue;

	return TRUE;
}
