/***
*
*  Copyright (c) 1999, Valve LLC. All rights reserved.
*
*  This product contains software technology licensed from Id 
*  Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*  All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

//
// HPB_bot - botman's High Ping Bastard bot
//
// (http://planethalflife.com/botman/)
//
// util.cpp
//


#include "pb_global.h"
#include "bot.h"
#include "bot_func.h"

extern char mod_name[32];
extern int mod_id;
extern bot_t bots[32];

int gmsgTextMsg = 0;
int gmsgSayText = 0;
int gmsgShowMenu = 0;

Vector UTIL_VecToAngles( const Vector &vec )
{
   float rgflVecOut[3];
   VEC_TO_ANGLES(vec, rgflVecOut);
   return Vector(rgflVecOut);
}


Vector UTIL_GetRight( const Vector &vec )
{
	Vector angles = UTIL_VecToAngles( vec );
	MAKE_VECTORS( angles );
	return gpGlobals->v_right;
}


// Overloaded to add IGNORE_GLASS
void UTIL_TraceLine( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, edict_t *pentIgnore, TraceResult *ptr )
{
	TRACE_LINE( vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE) | (ignoreGlass?0x100:0), pentIgnore, ptr );
}


void UTIL_TraceLine( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr )
{
   TRACE_LINE( vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE), pentIgnore, ptr );
}


void UTIL_TraceHull( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t *pentIgnore, TraceResult *ptr )
{
	TRACE_HULL( vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE), hullNumber, pentIgnore, ptr );
}


void UTIL_MakeVectors( const Vector &vecAngles )
{
   MAKE_VECTORS( vecAngles );
}

int UTIL_PointContents( const Vector &vec )
{
   return POINT_CONTENTS(vec);
}


void UTIL_SetSize( entvars_t *pev, const Vector &vecMin, const Vector &vecMax )
{
   SET_SIZE( ENT(pev), vecMin, vecMax );
}


void UTIL_SetOrigin( entvars_t *pev, const Vector &vecOrigin )
{
   SET_ORIGIN(ENT(pev), vecOrigin );
}


void ClientPrint( entvars_t *client, int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4 )
{
   if (gmsgTextMsg == 0)
      gmsgTextMsg = REG_USER_MSG( "TextMsg", -1 );

   MESSAGE_BEGIN( client ? MSG_ONE : MSG_ALL , gmsgTextMsg, NULL, client );

   WRITE_BYTE( msg_dest );
   WRITE_STRING( msg_name );

   if ( param1 )
      WRITE_STRING( param1 );
   if ( param2 )
      WRITE_STRING( param2 );
   if ( param3 )
      WRITE_STRING( param3 );
   if ( param4 )
      WRITE_STRING( param4 );

   MESSAGE_END();
}

void UTIL_ClientPrintAll( int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4 )
{
	ClientPrint( 0, msg_dest, msg_name, param1, param2, param3, param4 );
}

void UTIL_SayText( const char *pText, edict_t *pEdict )
{
   if (gmsgSayText == 0)
      gmsgSayText = REG_USER_MSG( "SayText", -1 );

   MESSAGE_BEGIN( MSG_ONE, gmsgSayText, NULL, pEdict );

   WRITE_BYTE( ENTINDEX(pEdict) );
   WRITE_STRING( pText );

   MESSAGE_END();
}

extern char valveTeamList[MAX_TEAMS][32];
extern int  valveTeamNumber;

// return team number 0 through 3 based what MOD uses for team numbers
int UTIL_GetTeam(edict_t *pEntity)
{
	const char *infobuffer;
	char teamName[32];
	char modelName[32];
	int i;

	switch( mod_id ) 
	{
	case DMC_DLL:
	case VALVE_DLL:
	case HUNGER_DLL:
	case GEARBOX_DLL:
		infobuffer = (*g_engfuncs.pfnGetInfoKeyBuffer)( pEntity );

		if(FBitSet( g_uiGameFlags, GAME_CTF ))
		{
			strcpy( modelName, (g_engfuncs.pfnInfoKeyValue( infobuffer, "model" )) );
			if ((FStrEq(modelName, "ctf_barney") ) ||
				(FStrEq(modelName, "cl_suit") ) ||
				(FStrEq(modelName, "ctf_gina") ) ||
				(FStrEq(modelName, "ctf_gordon") ) ||
				(FStrEq(modelName, "otis") ) ||
				(FStrEq(modelName, "ctf_scientist") ))
			{
				return 0;
			}
			else if ((FStrEq(modelName, "beret") ) ||
				(FStrEq(modelName, "drill") ) ||
				(FStrEq(modelName, "grunt") ) ||
				(FStrEq(modelName, "recruit") ) ||
				(FStrEq(modelName, "shephard") ) ||
				(FStrEq(modelName, "tower") ))
			{
				return 1;
			}
		}
		else
		{
			strcpy( teamName, (g_engfuncs.pfnInfoKeyValue( infobuffer, "team" )) );
			for (i=0; i<valveTeamNumber; i++) 
				if (stricmp( teamName, valveTeamList[i] ) == 0) return i;
		}
		debugMsg( "ERROR: Team not found!\n" );
		return 0;
	case AG_DLL:
	//case FRONTLINE_DLL:
	case TFC_DLL:
		return pEntity->v.team - 1;  // teams are 1-4 based
	
	case CSTRIKE_DLL:
		infobuffer = (*g_engfuncs.pfnGetInfoKeyBuffer)( pEntity );
		strcpy(modelName, (g_engfuncs.pfnInfoKeyValue(infobuffer, "model")));
		
		if ((FStrEq( modelName, "terror"   ) ) ||  // Phoenix Connektion
			(FStrEq( modelName, "arab"     ) ) ||    // Old L337 Krew
			(FStrEq( modelName, "leet") ) ||    // L337 Krew
			(FStrEq( modelName, "arctic"   ) ) ||  // Artic Avenger
			(FStrEq( modelName, "guerilla" ) ))  // Gorilla Warfare
		{
			return 0;
		}
		else if ((FStrEq( modelName, "urban" ) ) ||  // Seal Team 6
				 (FStrEq( modelName, "gsg9"  ) ) ||   // German GSG-9
				 (FStrEq( modelName, "sas"   ) ) ||    // UK SAS
				 (FStrEq( modelName, "gign"  ) ) ||   // French GIGN
				 (FStrEq( modelName, "vip"   ) ))      // VIP
		{
			return 1;
		}

		debugMsg( "UTIL_GetTeam: Unknown model = %s\n", modelName );
		return -1;  // return -1 if team is unknown
	
	default:
		int team = pEntity->v.team;  // assume all others are 0-3 based
		
		if ((team < 0) || (team > 3)) {
			debugMsg( "UTIL_GetTeam: Unknown team code = %i\n", team ); 
			team = -1;
		}			
		return team;
	}
}


int UTIL_GetBotIndex( edict_t *pEdict )
{
	for (int i=0; i<32; i++)
	{
		if (bots[i].is_used && bots[i].pEdict == pEdict) return i;
	}
	return -1;
}


bot_t* UTIL_GetBotPointer( edict_t *pEdict )
{ 
	for (int i=0; i<32; i++)
	{
		if (bots[i].is_used && bots[i].pEdict == pEdict) return (&bots[i]);
	}
	return 0;
}



bool UTIL_ButtonTriggers( edict_t *button, edict_t *target )
{
	const char *targetName = STRING( target->v.targetname );
	const char *buttonTarget = STRING( button->v.target );
	if ( FStrEq( buttonTarget, targetName ) ) return true;
	// multimanager in between?
	edict_t *bTarget = FIND_ENTITY_BY_TARGETNAME( 0, buttonTarget );
	if (!bTarget) return false;
	if ( FStrEq( STRING( bTarget->v.classname ), "multi_manager" ) ) {
		string_t *szTargetName = (string_t *)( (char *)bTarget->pvPrivateData + 272 );
		// check all multimanager targets:
		for ( ; *szTargetName; szTargetName++ ) {
			if ( FStrEq(targetName, STRING(*szTargetName)) )        return true;
		}
	}
	return false;
}


void UTIL_SelectItem(edict_t *pEdict, char *item_name)
{
   FakeClientCommand(pEdict, item_name, NULL, NULL);
}


void UTIL_ShowMenu( edict_t *pEdict, int slots, int displaytime, bool needmore, const char *pText )
{
   if (gmsgShowMenu == 0)
      gmsgShowMenu = REG_USER_MSG( "ShowMenu", -1 );

   MESSAGE_BEGIN( MSG_ONE, gmsgShowMenu, NULL, pEdict );

   WRITE_SHORT( slots );
   WRITE_CHAR( displaytime );
   WRITE_BYTE( needmore );
   WRITE_STRING( pText );

   MESSAGE_END();
}

#ifdef DEBUG
	edict_t *DBG_EntOfVars( const entvars_t *pev )
	{
		if (pev->pContainingEntity != NULL)	return pev->pContainingEntity;
		ALERT(at_console, "entvars_t pContainingEntity is NULL, calling into engine");
		edict_t* pent = (*g_engfuncs.pfnFindEntityByVars)((entvars_t*)pev);
		if (pent == NULL) ALERT(at_console, "DAMN!  Even the engine couldn't FindEntityByVars!");
		((entvars_t *)pev)->pContainingEntity = pent;
		return pent;
	}


	void DBG_AssertFunction( BOOL fExpr, const char *szExpr, const char *szFile, int szLine, const char *szMessage )
	{
		return;
		if (fExpr) return;
		char szOut[512];
		if (szMessage) sprintf( szOut, "ASSERT FAILED:\n %s \n(%s@%d)\n%s", szExpr, szFile, szLine, szMessage );
		else		   sprintf( szOut, "ASSERT FAILED:\n %s \n(%s@%d)", szExpr, szFile, szLine );
		ALERT( at_console, szOut );
	}

#endif	//DEBUG

