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
extern bool g_meta_init;

bool valveTeamPlayMode = false;
char valveTeamList[MAX_TEAMS][32] = { "","","","","","","","","","","","","","","","" };
int  valveTeamNumber = 0;



int debug_engine = 0;

void (*botMsgFunction)(void *, int) = NULL;
int botMsgIndex;

// messages created in RegUserMsg which will be "caught"
int message_VGUI = 0;
int message_ShowMenu = 0;
int message_WeaponList = 0;
int message_CurWeapon = 0;
int message_AmmoX = 0;
int message_WeapPickup = 0;
int message_AmmoPickup = 0;
int message_ItemPickup = 0;
int message_Health = 0;
int message_Battery = 0;  // Armor
int message_Damage = 0;
int message_Death = 0;
int message_Money = 0;  // for Counter-Strike

static FILE *fp;

void pfnChangeLevel(char* s1, char* s2)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnChangeLevel:\n"); fclose(fp); }
#endif
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
	if(!g_meta_init)
		(*g_engfuncs.pfnChangeLevel)(s1, s2);
	else
		RETURN_META(MRES_IGNORED);
}


void pfnMessageBegin(int msg_dest, int msg_type, const float *pOrigin, edict_t *ed)
{
   if (gpGlobals->deathmatch)
   {
      int index = -1;
#ifdef _DEBUG
      if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnMessageBegin: edict=%p dest=%d type=%d\n",ed,msg_dest,msg_type); fclose(fp); }
#endif
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
				else if (msg_type == message_WeapPickup)
					botMsgFunction = BotClient_Valve_WeaponPickup;
				else if (msg_type == message_ItemPickup)
					botMsgFunction = BotClient_Valve_ItemPickup;
				else if (msg_type == message_Health)
					botMsgFunction = BotClient_Valve_Health;
				else if (msg_type == message_Battery)
					botMsgFunction = BotClient_Valve_Battery;
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
				else if (msg_type == message_WeapPickup)
					botMsgFunction = BotClient_Holywars_WeaponPickup;
				else if (msg_type == message_ItemPickup)
					botMsgFunction = BotClient_Holywars_ItemPickup;
				else if (msg_type == message_Health)
					botMsgFunction = BotClient_Holywars_Health;
				else if (msg_type == message_Battery)
					botMsgFunction = BotClient_Holywars_Battery;
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
				else if (msg_type == message_WeapPickup)
					botMsgFunction = BotClient_DMC_WeaponPickup;
				else if (msg_type == message_ItemPickup)
					botMsgFunction = BotClient_DMC_ItemPickup;
				else if (msg_type == message_Health)
					botMsgFunction = BotClient_DMC_Health;
				else if (msg_type == message_Battery)
					botMsgFunction = BotClient_DMC_Battery;
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
				else if (msg_type == message_WeapPickup)
					botMsgFunction = BotClient_TFC_WeaponPickup;
				else if (msg_type == message_ItemPickup)
					botMsgFunction = BotClient_TFC_ItemPickup;
				else if (msg_type == message_Health)
					botMsgFunction = BotClient_TFC_Health;
				else if (msg_type == message_Battery)
					botMsgFunction = BotClient_TFC_Battery;
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
				else if (msg_type == message_WeapPickup)
					botMsgFunction = BotClient_CS_WeaponPickup;
				else if (msg_type == message_AmmoPickup)
					botMsgFunction = BotClient_CS_AmmoPickup;
				else if (msg_type == message_ItemPickup)
					botMsgFunction = BotClient_CS_ItemPickup;
				else if (msg_type == message_Health)
					botMsgFunction = BotClient_CS_Health;
				else if (msg_type == message_Battery)
					botMsgFunction = BotClient_CS_Battery;
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
				else if (msg_type == message_WeapPickup)
					botMsgFunction = BotClient_Gearbox_WeaponPickup;
				else if (msg_type == message_ItemPickup)
					botMsgFunction = BotClient_Gearbox_ItemPickup;
				else if (msg_type == message_Health)
					botMsgFunction = BotClient_Gearbox_Health;
				else if (msg_type == message_Battery)
					botMsgFunction = BotClient_Gearbox_Battery;
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
				else if (msg_type == message_WeapPickup)
					botMsgFunction = BotClient_Hunger_WeaponPickup;
				else if (msg_type == message_ItemPickup)
					botMsgFunction = BotClient_Hunger_ItemPickup;
				else if (msg_type == message_Health)
					botMsgFunction = BotClient_Hunger_Health;
				else if (msg_type == message_Battery)
					botMsgFunction = BotClient_Hunger_Battery;
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
   }
	if(!g_meta_init)
		(*g_engfuncs.pfnMessageBegin)(msg_dest, msg_type, pOrigin, ed);
	else
		RETURN_META(MRES_IGNORED);
}

int pfnRegUserMsg(const char *pszName, int iSize)
{
	int msg = (*g_engfuncs.pfnRegUserMsg)(pszName, iSize);
	
	if (gpGlobals->deathmatch)
	{
		//fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnRegUserMsg: pszName=%s msg=%d\n",pszName,msg); fclose(fp);
				
			 if (strcmp( pszName, "WeaponList"	) == 0)	message_WeaponList = msg;
		else if (strcmp( pszName, "CurWeapon"	) == 0)	message_CurWeapon = msg;
		else if (strcmp( pszName, "AmmoX"       ) == 0)	message_AmmoX = msg;
		else if (strcmp( pszName, "AmmoPickup"  ) == 0)	message_AmmoPickup = msg;
		else if (strcmp( pszName, "WeapPickup"  ) == 0)	message_WeapPickup = msg;
		else if (strcmp( pszName, "ItemPickup"  ) == 0)	message_ItemPickup = msg;
		else if (strcmp( pszName, "Health"      ) == 0)	message_Health = msg;
		else if (strcmp( pszName, "Battery"     ) == 0)	message_Battery = msg;
		else if (strcmp( pszName, "Damage"      ) == 0)	message_Damage = msg;
		else if (strcmp( pszName, "DeathMsg"    ) == 0)	message_Death = msg;
		// TFC / CS
		else if (strcmp(pszName, "VGUIMenu"		) == 0) message_VGUI = msg;
		// CS only			
		else if (strcmp(pszName, "ShowMenu"		) == 0) message_ShowMenu = msg;
		else if (strcmp(pszName, "Money"		) == 0) message_Money = msg;         		
	}
	if(!g_meta_init)
		return msg;
	else
		RETURN_META_VALUE(MRES_SUPERCEDE, msg);
}


void pfnMessageEnd(void)
{
   if (gpGlobals->deathmatch)
   {
#ifdef _DEBUG
      if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnMessageEnd:\n"); fclose(fp); }
#endif
      // clear out the bot message function pointer...
      botMsgFunction = NULL;
   }
	if(!g_meta_init)
		(*g_engfuncs.pfnMessageEnd)();
	else
		RETURN_META(MRES_IGNORED);
}

///////////////////////////////////////////////////////////////////////////////////
//
//  FORWARD ENGINE FUNCTIONS...
//
///////////////////////////////////////////////////////////////////////////////////
int pfnPrecacheModel(char* s)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnPrecacheModel: %s\n",s); fclose(fp); }
#endif
   return (*g_engfuncs.pfnPrecacheModel)(s);
}

int pfnPrecacheSound(char* s)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnPrecacheSound: %s\n",s); fclose(fp); }
#endif
   return (*g_engfuncs.pfnPrecacheSound)(s);
}

void pfnSetModel(edict_t *e, const char *m)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetModel: edict=%p %s\n",e,m); fclose(fp); }
#endif
   (*g_engfuncs.pfnSetModel)(e, m);
}

int pfnModelIndex(const char *m)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnModelIndex: %s\n",m); fclose(fp); }
#endif
   return (*g_engfuncs.pfnModelIndex)(m);
}

int pfnModelFrames(int modelIndex)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnModelFrames:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnModelFrames)(modelIndex);
}

void pfnSetSize(edict_t *e, const float *rgflMin, const float *rgflMax)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetSize: %p\n",e); fclose(fp); }
#endif
   (*g_engfuncs.pfnSetSize)(e, rgflMin, rgflMax);
}

void pfnGetSpawnParms(edict_t *ent)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetSpawnParms:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnGetSpawnParms)(ent);
}

void pfnSaveSpawnParms(edict_t *ent)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSaveSpawnParms:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnSaveSpawnParms)(ent);
}

float pfnVecToYaw(const float *rgflVector)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnVecToYaw:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnVecToYaw)(rgflVector);
}

void pfnVecToAngles(const float *rgflVectorIn, float *rgflVectorOut)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnVecToAngles:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnVecToAngles)(rgflVectorIn, rgflVectorOut);
}

void pfnMoveToOrigin(edict_t *ent, const float *pflGoal, float dist, int iMoveType)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnMoveToOrigin:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnMoveToOrigin)(ent, pflGoal, dist, iMoveType);
}

void pfnChangeYaw(edict_t* ent)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnChangeYaw:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnChangeYaw)(ent);
}

void pfnChangePitch(edict_t* ent)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnChangePitch:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnChangePitch)(ent);
}

edict_t* pfnFindEntityByString(edict_t *pEdictStartSearchAfter, const char *pszField, const char *pszValue)
{
#ifdef _DEBUG
	//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnFindEntityByString: %s\n",pszValue); fclose(fp); }
#endif
	if (( mod_id == CSTRIKE_DLL ) &&
		( strcmp( pszField, "classname" ) == 0 ) && 
		( strcmp( pszValue, "info_map_parameters" ) == 0 ) ) {
		//debugMsg( "NEW CS-ROUND!\n" );
		roundStartTime = worldTime() + CVAR_GET_FLOAT("mp_freezetime");	// 5 seconds until round starts
	}
	if(!g_meta_init)
		return (*g_engfuncs.pfnFindEntityByString)(pEdictStartSearchAfter, pszField, pszValue);
	else
		RETURN_META_VALUE(MRES_IGNORED, 0);
}

int pfnGetEntityIllum(edict_t* pEnt)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetEntityIllum:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnGetEntityIllum)(pEnt);
}

edict_t* pfnFindEntityInSphere(edict_t *pEdictStartSearchAfter, const float *org, float rad)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnFindEntityInSphere:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnFindEntityInSphere)(pEdictStartSearchAfter, org, rad);
}

edict_t* pfnFindClientInPVS(edict_t *pEdict)
{
#ifdef _DEBUG
   //if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnFindClientInPVS:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnFindClientInPVS)(pEdict);
}

edict_t* pfnEntitiesInPVS(edict_t *pplayer)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnEntitiesInPVS:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnEntitiesInPVS)(pplayer);
}

void pfnMakeVectors(const float *rgflVector)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnMakeVectors:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnMakeVectors)(rgflVector);
}

void pfnAngleVectors(const float *rgflVector, float *forward, float *right, float *up)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnAngleVectors:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnAngleVectors)(rgflVector, forward, right, up);
}

edict_t* pfnCreateEntity(void)
{
   edict_t *pent = (*g_engfuncs.pfnCreateEntity)();
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCreateEntity: %p\n",pent); fclose(fp); }
#endif
   return pent;
}

void pfnRemoveEntity(edict_t* e)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnRemoveEntity: %p\n",e); fclose(fp); }
   if (debug_engine)
   {
      fp = UTIL_OpenDebugLog();
      fprintf(fp,"pfnRemoveEntity: %p\n",e);
      if (e->v.model != 0)
         fprintf(fp," model=%s\n", STRING(e->v.model));
      fclose(fp);
   }
#endif
   (*g_engfuncs.pfnRemoveEntity)(e);
}

edict_t* pfnCreateNamedEntity(int className)
{
   edict_t *pent = (*g_engfuncs.pfnCreateNamedEntity)(className);
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCreateNamedEntity: edict=%p name=%s\n",pent,STRING(className)); fclose(fp); }
#endif
   return pent;
}

void pfnMakeStatic(edict_t *ent)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnMakeStatic:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnMakeStatic)(ent);
}

int pfnEntIsOnFloor(edict_t *e)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnEntIsOnFloor:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnEntIsOnFloor)(e);
}

int pfnDropToFloor(edict_t* e)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnDropToFloor:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnDropToFloor)(e);
}

int pfnWalkMove(edict_t *ent, float yaw, float dist, int iMode)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnWalkMove:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnWalkMove)(ent, yaw, dist, iMode);
}

void pfnSetOrigin(edict_t *e, const float *rgflOrigin)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetOrigin:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnSetOrigin)(e, rgflOrigin);
}

void pfnEmitSound(edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch)
{
	playerSounds.parseSound( entity, sample, volume );
#ifdef _DEBUG
	if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnEmitSound:\n"); fclose(fp); }
#endif
	if(!g_meta_init)
		(*g_engfuncs.pfnEmitSound)(entity, channel, sample, volume, attenuation, fFlags, pitch);
	else
		RETURN_META(MRES_IGNORED);
}

void pfnEmitAmbientSound(edict_t *entity, float *pos, const char *samp, float vol, float attenuation, int fFlags, int pitch)
{
	playerSounds.parseAmbientSound( entity, samp, vol );
#ifdef _DEBUG
	if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnEmitAmbientSound:\n"); fclose(fp); }
#endif
	if(!g_meta_init)
		(*g_engfuncs.pfnEmitAmbientSound)(entity, pos, samp, vol, attenuation, fFlags, pitch);
	else
		RETURN_META(MRES_IGNORED);
}

void pfnTraceLine(const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnTraceLine:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnTraceLine)(v1, v2, fNoMonsters, pentToSkip, ptr);
}

void pfnTraceToss(edict_t* pent, edict_t* pentToIgnore, TraceResult *ptr)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnTraceToss:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnTraceToss)(pent, pentToIgnore, ptr);
}

int pfnTraceMonsterHull(edict_t *pEdict, const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnTraceMonsterHull:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnTraceMonsterHull)(pEdict, v1, v2, fNoMonsters, pentToSkip, ptr);
}

void pfnTraceHull(const float *v1, const float *v2, int fNoMonsters, int hullNumber, edict_t *pentToSkip, TraceResult *ptr)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnTraceHull:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnTraceHull)(v1, v2, fNoMonsters, hullNumber, pentToSkip, ptr);
}

void pfnTraceModel(const float *v1, const float *v2, int hullNumber, edict_t *pent, TraceResult *ptr)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnTraceModel:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnTraceModel)(v1, v2, hullNumber, pent, ptr);
}

const char *pfnTraceTexture(edict_t *pTextureEntity, const float *v1, const float *v2 )
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnTraceTexture:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnTraceTexture)(pTextureEntity, v1, v2);
}

void pfnTraceSphere(const float *v1, const float *v2, int fNoMonsters, float radius, edict_t *pentToSkip, TraceResult *ptr)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnTraceSphere:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnTraceSphere)(v1, v2, fNoMonsters, radius, pentToSkip, ptr);
}

void pfnGetAimVector(edict_t* ent, float speed, float *rgflReturn)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetAimVector:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnGetAimVector)(ent, speed, rgflReturn);
}

//void BotCreate( edict_t *pPlayer, const char *botTeam, const char *botClass, const char *arg3, const char *arg4 );

void pfnServerCommand(char* str)
{
	infoMsg( "ServerCommand: ", str, "\n" );
#ifdef _DEBUG
    if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnServerCommand: %s\n",str); fclose(fp); }
#endif
/*    if (FStrEq(str, "addbot")) {	// we've got this is DSaddbot
		BotCreate();
		return;
	}*/
	if(!g_meta_init)
		(*g_engfuncs.pfnServerCommand)(str);
	else
		RETURN_META(MRES_IGNORED);
}

void pfnServerExecute(void)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnServerExecute:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnServerExecute)();
}

void pfnClientCommand(edict_t* pEdict, char* szFmt, ...)
{
#ifdef _DEBUG
	if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnClientCommand=%s\n",szFmt); fclose(fp); }
#endif
	if(!g_meta_init)
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

void pfnParticleEffect(const float *org, const float *dir, float color, float count)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnParticleEffect:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnParticleEffect)(org, dir, color, count);
}

void pfnLightStyle(int style, char* val)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnLightStyle:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnLightStyle)(style, val);
}

int pfnDecalIndex(const char *name)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnDecalIndex:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnDecalIndex)(name);
}

int pfnPointContents(const float *rgflVector)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnPointContents:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnPointContents)(rgflVector);
}

void pfnWriteByte(int iValue)
{
   if (gpGlobals->deathmatch)
   {
#ifdef _DEBUG
      if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnWriteByte: %d\n",iValue); fclose(fp); }
#endif
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&iValue, botMsgIndex);
   }
	if(!g_meta_init)
		(*g_engfuncs.pfnWriteByte)(iValue);
	else
		RETURN_META(MRES_IGNORED);

}

void pfnWriteChar(int iValue)
{
   if (gpGlobals->deathmatch)
   {
#ifdef _DEBUG
      if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnWriteChar: %d\n",iValue); fclose(fp); }
#endif
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&iValue, botMsgIndex);
   }
	if(!g_meta_init)
		(*g_engfuncs.pfnWriteChar)(iValue);
	else
		RETURN_META(MRES_IGNORED);
}

void pfnWriteShort(int iValue)
{
   if (gpGlobals->deathmatch)
   {
#ifdef _DEBUG
      if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"prnWriteShort: %d\n",iValue); fclose(fp); }
#endif
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&iValue, botMsgIndex);
   }
	if(!g_meta_init)
		(*g_engfuncs.pfnWriteShort)(iValue);
	else
		RETURN_META(MRES_IGNORED);
}

void pfnWriteLong(int iValue)
{
   if (gpGlobals->deathmatch)
   {
#ifdef _DEBUG
      if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnWriteLong: %d\n",iValue); fclose(fp); }
#endif
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&iValue, botMsgIndex);
   }
	if(!g_meta_init)
		(*g_engfuncs.pfnWriteLong)(iValue);
	else
		RETURN_META(MRES_IGNORED);
}

void pfnWriteAngle(float flValue)
{
   if (gpGlobals->deathmatch)
   {
#ifdef _DEBUG
      if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnWriteAngle: %f\n",flValue); fclose(fp); }
#endif
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&flValue, botMsgIndex);
   }
	if(!g_meta_init)
		(*g_engfuncs.pfnWriteAngle)(flValue);
	else
		RETURN_META(MRES_IGNORED);
}

void pfnWriteCoord(float flValue)
{
   if (gpGlobals->deathmatch)
   {
#ifdef _DEBUG
      if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnWriteCoord: %f\n",flValue); fclose(fp); }
#endif
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&flValue, botMsgIndex);
   }
	if(!g_meta_init)
		(*g_engfuncs.pfnWriteCoord)(flValue);
	else
		RETURN_META(MRES_IGNORED);
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
	   //debugMsg( "MSG: ", sz, "\n" );
#ifdef _DEBUG
      if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnWriteString: %s\n",sz); fclose(fp); }
#endif
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)sz, botMsgIndex);
   }
	if(!g_meta_init)
		(*g_engfuncs.pfnWriteString)(sz);
	else
		RETURN_META(MRES_IGNORED);
}

void pfnWriteEntity(int iValue)
{
   if (gpGlobals->deathmatch)
   {
#ifdef _DEBUG
      if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnWriteEntity: %d\n",iValue); fclose(fp); }
#endif
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)&iValue, botMsgIndex);
   }
	if(!g_meta_init)
		(*g_engfuncs.pfnWriteEntity)(iValue);
	else
		RETURN_META(MRES_IGNORED);
}

void pfnCVarRegister(cvar_t *pCvar)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCVarRegister:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnCVarRegister)(pCvar);
}

float pfnCVarGetFloat(const char *szVarName)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCVarGetFloat: %s\n",szVarName); fclose(fp); }
#endif
   return (*g_engfuncs.pfnCVarGetFloat)(szVarName);
}

const char* pfnCVarGetString(const char *szVarName)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCVarGetString:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnCVarGetString)(szVarName);
}

void pfnCVarSetFloat(const char *szVarName, float flValue)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCVarSetFloat:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnCVarSetFloat)(szVarName, flValue);
}

void pfnCVarSetString(const char *szVarName, const char *szValue)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCVarSetString:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnCVarSetString)(szVarName, szValue);
}

void* pfnPvAllocEntPrivateData(edict_t *pEdict, long cb)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnPvAllocEntPrivateData:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnPvAllocEntPrivateData)(pEdict, cb);
}

void* pfnPvEntPrivateData(edict_t *pEdict)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnPvEntPrivateData:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnPvEntPrivateData)(pEdict);
}

void pfnFreeEntPrivateData(edict_t *pEdict)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnFreeEntPrivateData:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnFreeEntPrivateData)(pEdict);
}

const char* pfnSzFromIndex(int iString)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSzFromIndex:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnSzFromIndex)(iString);
}

int pfnAllocString(const char *szValue)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnAllocString:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnAllocString)(szValue);
}

entvars_t* pfnGetVarsOfEnt(edict_t *pEdict)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetVarsOfEnt:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnGetVarsOfEnt)(pEdict);
}

edict_t* pfnPEntityOfEntOffset(int iEntOffset)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnPEntityOfEntOffset:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnPEntityOfEntOffset)(iEntOffset);
}

int pfnEntOffsetOfPEntity(const edict_t *pEdict)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnEntOffsetOfPEntity: %p\n",pEdict); fclose(fp); }
#endif
   return (*g_engfuncs.pfnEntOffsetOfPEntity)(pEdict);
}

int pfnIndexOfEdict(const edict_t *pEdict)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnIndexOfEdict: %p\n",pEdict); fclose(fp); }
#endif
   return (*g_engfuncs.pfnIndexOfEdict)(pEdict);
}

edict_t* pfnPEntityOfEntIndex(int iEntIndex)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnPEntityOfEntIndex:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnPEntityOfEntIndex)(iEntIndex);
}

edict_t* pfnFindEntityByVars(entvars_t* pvars)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnFindEntityByVars:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnFindEntityByVars)(pvars);
}

void* pfnGetModelPtr(edict_t* pEdict)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetModelPtr: %p\n",pEdict); fclose(fp); }
#endif
   return (*g_engfuncs.pfnGetModelPtr)(pEdict);
}

void pfnAnimationAutomove(const edict_t* pEdict, float flTime)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnAnimationAutomove:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnAnimationAutomove)(pEdict, flTime);
}

void pfnGetBonePosition(const edict_t* pEdict, int iBone, float *rgflOrigin, float *rgflAngles )
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetBonePosition:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnGetBonePosition)(pEdict, iBone, rgflOrigin, rgflAngles);
}

unsigned long pfnFunctionFromName( const char *pName )
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnFunctionFromName:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnFunctionFromName)(pName);
}

const char *pfnNameForFunction( unsigned long function )
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnNameForFunction:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnNameForFunction)(function);
}

void pfnClientPrintf( edict_t* pEdict, PRINT_TYPE ptype, const char *szMsg )
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnClientPrintf:\n"); fclose(fp); }
#endif
   if (!(pEdict->v.flags & FL_FAKECLIENT)) (*g_engfuncs.pfnClientPrintf)(pEdict, ptype, szMsg);
}

void pfnServerPrint( const char *szMsg )
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnServerPrint:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnServerPrint)(szMsg);
}

void pfnGetAttachment(const edict_t *pEdict, int iAttachment, float *rgflOrigin, float *rgflAngles )
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetAttachment:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnGetAttachment)(pEdict, iAttachment, rgflOrigin, rgflAngles);
}

void pfnCRC32_Init(CRC32_t *pulCRC)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCRC32_Init:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnCRC32_Init)(pulCRC);
}

void pfnCRC32_ProcessBuffer(CRC32_t *pulCRC, void *p, int len)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCRC32_ProcessBuffer:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnCRC32_ProcessBuffer)(pulCRC, p, len);
}

void pfnCRC32_ProcessByte(CRC32_t *pulCRC, unsigned char ch)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCRC32_ProcessByte:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnCRC32_ProcessByte)(pulCRC, ch);
}

CRC32_t pfnCRC32_Final(CRC32_t pulCRC)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCRC32_Final:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnCRC32_Final)(pulCRC);
}

long pfnRandomLong(long lLow, long lHigh)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnRandomLong: lLow=%d lHigh=%d\n",lLow,lHigh); fclose(fp); }
#endif
   return (*g_engfuncs.pfnRandomLong)(lLow, lHigh);
}

float pfnRandomFloat(float flLow, float flHigh)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnRandomFloat:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnRandomFloat)(flLow, flHigh);
}

void pfnSetView(const edict_t *pClient, const edict_t *pViewent )
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetView:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnSetView)(pClient, pViewent);
}

float pfnTime( void )
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnTime:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnTime)();
}

void pfnCrosshairAngle(const edict_t *pClient, float pitch, float yaw)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCrosshairAngle:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnCrosshairAngle)(pClient, pitch, yaw);
}

byte *pfnLoadFileForMe(char *filename, int *pLength)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnLoadFileForMe: filename=%s\n",filename); fclose(fp); }
#endif
   return (*g_engfuncs.pfnLoadFileForMe)(filename, pLength);
}

void pfnFreeFile(void *buffer)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnFreeFile:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnFreeFile)(buffer);
}

void pfnEndSection(const char *pszSectionName)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnEndSection:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnEndSection)(pszSectionName);
}

int pfnCompareFileTime(char *filename1, char *filename2, int *iCompare)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCompareFileTime:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnCompareFileTime)(filename1, filename2, iCompare);
}

void pfnGetGameDir(char *szGetGameDir)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetGameDir:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnGetGameDir)(szGetGameDir);
}

void pfnCvar_RegisterVariable(cvar_t *variable)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCvar_RegisterVariable:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnCvar_RegisterVariable)(variable);
}

void pfnFadeClientVolume(const edict_t *pEdict, int fadePercent, int fadeOutSeconds, int holdTime, int fadeInSeconds)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnFadeClientVolume:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnFadeClientVolume)(pEdict, fadePercent, fadeOutSeconds, holdTime, fadeInSeconds);
}

void pfnSetClientMaxspeed(const edict_t *pEdict, float fNewMaxspeed)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetClientMaxspeed: edict=%p %f\n",pEdict,fNewMaxspeed); fclose(fp); }
#endif
   (*g_engfuncs.pfnSetClientMaxspeed)(pEdict, fNewMaxspeed);
}

edict_t * pfnCreateFakeClient(const char *netname)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCreateFakeClient:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnCreateFakeClient)(netname);
}

void pfnRunPlayerMove(edict_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, byte impulse, byte msec )
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnRunPlayerMove: impulse=%i\n", impulse); fclose(fp); }
#endif
   (*g_engfuncs.pfnRunPlayerMove)(fakeclient, viewangles, forwardmove, sidemove, upmove, buttons, impulse, msec);
}

int pfnNumberOfEntities(void)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnNumberOfEntities:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnNumberOfEntities)();
}

char* pfnGetInfoKeyBuffer(edict_t *e)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetInfoKeyBuffer:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnGetInfoKeyBuffer)(e);
}

char* pfnInfoKeyValue(char *infobuffer, char *key)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnInfoKeyValue: %s %s\n",infobuffer,key); fclose(fp); }
#endif
   return (*g_engfuncs.pfnInfoKeyValue)(infobuffer, key);
}

void pfnSetKeyValue(char *infobuffer, char *key, char *value)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetKeyValue: %s %s\n",key,value); fclose(fp); }
#endif
   (*g_engfuncs.pfnSetKeyValue)(infobuffer, key, value);
}

void pfnSetClientKeyValue( int clientIndex, char *infobuffer, char *key, char *value )
{
	if ((mod_id == VALVE_DLL || mod_id == AG_DLL || mod_id == DMC_DLL || mod_id == HUNGER_DLL || mod_id == GEARBOX_DLL) && (strcmp( key, "team" ) == 0)) {	// init teamlist
		valveTeamPlayMode = true;

		bool teamKnown = false;
		for (int team=0; team<valveTeamNumber; team++) 
			if (strcmp( value, valveTeamList[team] ) == 0) {
				teamKnown = true;
				break;
			}
		if (!teamKnown && valveTeamNumber<MAX_TEAMS) {
			strcpy( valveTeamList[valveTeamNumber], value );
			debugMsg( "Registered team ", value, "\n" );
			valveTeamNumber++;
		}
	}
#ifdef _DEBUG
	if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetClientKeyValue: %s %s\n",key,value); fclose(fp); }
#endif
	if(!g_meta_init)
		(*g_engfuncs.pfnSetClientKeyValue)(clientIndex, infobuffer, key, value);
	else
		RETURN_META(MRES_IGNORED);
}

int pfnIsMapValid(char *filename)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnIsMapValid:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnIsMapValid)(filename);
}

void pfnStaticDecal( const float *origin, int decalIndex, int entityIndex, int modelIndex )
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnStaticDecal:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnStaticDecal)(origin, decalIndex, entityIndex, modelIndex);
}

int pfnPrecacheGeneric(char* s)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnPrecacheGeneric: %s\n",s); fclose(fp); }
#endif
   return (*g_engfuncs.pfnPrecacheGeneric)(s);
}

int pfnGetPlayerUserId(edict_t *e )
{
   if (gpGlobals->deathmatch)
   {
#ifdef _DEBUG
      if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetPlayerUserId: %p\n",e); fclose(fp); }
#endif
   }

   return (*g_engfuncs.pfnGetPlayerUserId)(e);
}

void pfnBuildSoundMsg(edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch, int msg_dest, int msg_type, const float *pOrigin, edict_t *ed)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnBuildSoundMsg:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnBuildSoundMsg)(entity, channel, sample, volume, attenuation, fFlags, pitch, msg_dest, msg_type, pOrigin, ed);
}

int pfnIsDedicatedServer(void)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnIsDedicatedServer:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnIsDedicatedServer)();
}

cvar_t* pfnCVarGetPointer(const char *szVarName)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCVarGetPointer: %s\n",szVarName); fclose(fp); }
#endif
   return (*g_engfuncs.pfnCVarGetPointer)(szVarName);
}

unsigned int pfnGetPlayerWONId(edict_t *e)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetPlayerWONId: %p\n",e); fclose(fp); }
#endif
   return (*g_engfuncs.pfnGetPlayerWONId)(e);
}


// new stuff for SDK 2.0

void pfnInfo_RemoveKey(char *s, const char *key)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnInfo_RemoveKey:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnInfo_RemoveKey)(s, key);
}

const char *pfnGetPhysicsKeyValue(const edict_t *pClient, const char *key)
{
	const char *res = (*g_engfuncs.pfnGetPhysicsKeyValue)(pClient, key);
#ifdef _DEBUG
	if (debug_engine) { 
		fp = UTIL_OpenDebugLog(); 
		fprintf(fp,"pfnGetPhysicsKeyValue: key=%s, result=%s\n", key, res); 
		fclose(fp); 
	}
#endif
	//int ir = (int) res[0];
	//debugMsg( "PK=%i\n", ir );
	return res;
}

void pfnSetPhysicsKeyValue(const edict_t *pClient, const char *key, const char *value)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetPhysicsKeyValue:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnSetPhysicsKeyValue)(pClient, key, value);
}

const char *pfnGetPhysicsInfoString(const edict_t *pClient)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetPhysicsInfoString:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnGetPhysicsInfoString)(pClient);
}

unsigned short pfnPrecacheEvent(int type, const char *psz)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnPrecacheEvent:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnPrecacheEvent)(type, psz);
}

void pfnPlaybackEvent(int flags, const edict_t *pInvoker, unsigned short eventindex, float delay,
   float *origin, float *angles, float fparam1,float fparam2, int iparam1, int iparam2, int bparam1, int bparam2)
{
#ifdef _DEBUG
	if (debug_engine) { 
		fp = UTIL_OpenDebugLog(); 
		fprintf(fp,"pfnPlaybackEvent(flags=%i,index=%i, delay=%.2f)\n", flags, eventindex, delay); 
		fclose(fp); 
	}
#endif
   (*g_engfuncs.pfnPlaybackEvent)(flags, pInvoker, eventindex, delay, origin, angles, fparam1, fparam2, iparam1, iparam2, bparam1, bparam2);
}

unsigned char *pfnSetFatPVS(float *org)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetFatPVS:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnSetFatPVS)(org);
}

unsigned char *pfnSetFatPAS(float *org)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetFatPAS:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnSetFatPAS)(org);
}

int pfnCheckVisibility(const edict_t *entity, unsigned char *pset)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCheckVisibility:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnCheckVisibility)(entity, pset);
}

void pfnDeltaSetField(struct delta_s *pFields, const char *fieldname)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnDeltaSetField:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnDeltaSetField)(pFields, fieldname);
}

void pfnDeltaUnsetField(struct delta_s *pFields, const char *fieldname)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnDeltaUnsetField:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnDeltaUnsetField)(pFields, fieldname);
}

void pfnDeltaAddEncoder(char *name, void (*conditionalencode)( struct delta_s *pFields, const unsigned char *from, const unsigned char *to))
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnDeltaAddEncoder:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnDeltaAddEncoder)(name, conditionalencode);
}

int pfnGetCurrentPlayer(void)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetCurrentPlayer: "); fclose(fp); }
#endif
   return (*g_engfuncs.pfnGetCurrentPlayer)();
}

int pfnCanSkipPlayer(const edict_t *player)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCanSkipPlayer:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnCanSkipPlayer)(player);
}

int pfnDeltaFindField(struct delta_s *pFields, const char *fieldname)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnDeltaFindField:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnDeltaFindField)(pFields, fieldname);
}

void pfnDeltaSetFieldByIndex(struct delta_s *pFields, int fieldNumber)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnDeltaSetFieldByIndex:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnDeltaSetFieldByIndex)(pFields, fieldNumber);
}

void pfnDeltaUnsetFieldByIndex(struct delta_s *pFields, int fieldNumber)
{
#ifdef _DEBUG
//   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnDeltaUnsetFieldByIndex:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnDeltaUnsetFieldByIndex)(pFields, fieldNumber);
}

void pfnSetGroupMask(int mask, int op)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetGroupMask:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnSetGroupMask)(mask, op);
}

int pfnCreateInstancedBaseline(int classname, struct entity_state_s *baseline)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCreateInstancedBaseline:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnCreateInstancedBaseline)(classname, baseline);
}

void pfnCvar_DirectSet(struct cvar_s *var, char *value)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCvar_DirectSet:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnCvar_DirectSet)(var, value);
}

void pfnForceUnmodified(FORCE_TYPE type, float *mins, float *maxs, const char *filename)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnForceUnmodified:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnForceUnmodified)(type, mins, maxs, filename);
}

void pfnGetPlayerStats(const edict_t *pClient, int *ping, int *packet_loss)
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetPlayerStats:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnGetPlayerStats)(pClient, ping, packet_loss);
}

void pfnAddServerCommand( char *cmd_name, void (*function) (void) )
{
#ifdef _DEBUG
   if (debug_engine) { fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnAddServerCommand:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnAddServerCommand)( cmd_name, function );
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

