// Based on:
//
// HPB bot - botman's High Ping Bastard bot
//
// (http://planethalflife.com/botman/)
//
// engine.cpp
//

#ifdef _DEBUG
#include "extdll.h"
#include "dllapi.h"
#include "meta_api.h"
#include "entity_state.h"
#include "bot.h"
#include "engine.h"

///////////////////////////////////////////////////////////////////////////////////
//
//  FORWARD ENGINE FUNCTIONS...
//
///////////////////////////////////////////////////////////////////////////////////
int pfnPrecacheModel(const char *s)
{
   debugFile("pfnPrecacheModel: %s\n",s);
   return (*g_engfuncs.pfnPrecacheModel)(s);
}

int pfnPrecacheSound( const char *s )
{
   debugFile("pfnPrecacheSound: %s\n",s);
   return (*g_engfuncs.pfnPrecacheSound)(s);
}

void pfnSetModel(edict_t *e, const char *m)
{
   debugFile("pfnSetModel: edict=%p %s\n",e,m);
   (*g_engfuncs.pfnSetModel)(e, m);
}

int pfnModelIndex(const char *m)
{
//   debugFile("pfnModelIndex: %s\n",m);
   return (*g_engfuncs.pfnModelIndex)(m);
}

int pfnModelFrames(int modelIndex)
{
   debugFile("pfnModelFrames:\n");

   return (*g_engfuncs.pfnModelFrames)(modelIndex);
}

void pfnSetSize(edict_t *e, const float *rgflMin, const float *rgflMax)
{
   debugFile("pfnSetSize: %p\n",e);
   (*g_engfuncs.pfnSetSize)(e, rgflMin, rgflMax);
}

void pfnGetSpawnParms(edict_t *ent)
{
   debugFile("pfnGetSpawnParms:\n");
   (*g_engfuncs.pfnGetSpawnParms)(ent);
}

void pfnSaveSpawnParms(edict_t *ent)
{
   debugFile("pfnSaveSpawnParms:\n");
   (*g_engfuncs.pfnSaveSpawnParms)(ent);
}

float pfnVecToYaw(const float *rgflVector)
{
//   debugFile("pfnVecToYaw:\n");
   return (*g_engfuncs.pfnVecToYaw)(rgflVector);
}

void pfnVecToAngles(const float *rgflVectorIn, float *rgflVectorOut)
{
//   debugFile("pfnVecToAngles:\n");
   (*g_engfuncs.pfnVecToAngles)(rgflVectorIn, rgflVectorOut);
}

void pfnMoveToOrigin(edict_t *ent, const float *pflGoal, float dist, int iMoveType)
{
   debugFile("pfnMoveToOrigin:\n");
   (*g_engfuncs.pfnMoveToOrigin)(ent, pflGoal, dist, iMoveType);
}

void pfnChangeYaw(edict_t* ent)
{
//   debugFile("pfnChangeYaw:\n");
   (*g_engfuncs.pfnChangeYaw)(ent);
}

void pfnChangePitch(edict_t* ent)
{
//   debugFile( "pfnChangePitch:\n");
   (*g_engfuncs.pfnChangePitch)(ent);
}

int pfnGetEntityIllum(edict_t* pEnt)
{
   debugFile("pfnGetEntityIllum:\n");
   return (*g_engfuncs.pfnGetEntityIllum)(pEnt);
}

edict_t* pfnFindEntityInSphere(edict_t *pEdictStartSearchAfter, const float *org, float rad)
{
   debugFile("pfnFindEntityInSphere:\n");
   return (*g_engfuncs.pfnFindEntityInSphere)(pEdictStartSearchAfter, org, rad);
}

edict_t* pfnFindClientInPVS(edict_t *pEdict)
{
   //debugFile( "pfnFindClientInPVS:\n");
   return (*g_engfuncs.pfnFindClientInPVS)(pEdict);
}

edict_t* pfnEntitiesInPVS(edict_t *pplayer)
{
   debugFile( "pfnEntitiesInPVS:\n");
   return (*g_engfuncs.pfnEntitiesInPVS)(pplayer);
}

void pfnMakeVectors(const float *rgflVector)
{
//   debugFile( "pfnMakeVectors:\n");
   (*g_engfuncs.pfnMakeVectors)(rgflVector);
}

void pfnAngleVectors(const float *rgflVector, float *forward, float *right, float *up)
{
//   debugFile( "pfnAngleVectors:\n");
   (*g_engfuncs.pfnAngleVectors)(rgflVector, forward, right, up);
}

edict_t* pfnCreateEntity(void)
{
   edict_t *pent = (*g_engfuncs.pfnCreateEntity)();
   debugFile( "pfnCreateEntity: %p\n",pent);
   return pent;
}

void pfnRemoveEntity(edict_t* e)
{
//   debugFile("pfnRemoveEntity: %p\n",e);
      debugFile("pfnRemoveEntity: %p\n",e);
      if (e->v.model != 0)
         debugFile( " model=%s\n", STRING(e->v.model));
   (*g_engfuncs.pfnRemoveEntity)(e);
}

edict_t* pfnCreateNamedEntity(int className)
{
   edict_t *pent = (*g_engfuncs.pfnCreateNamedEntity)(className);
   debugFile( "pfnCreateNamedEntity: edict=%p name=%s\n",pent,STRING(className));
   return pent;
}

void pfnMakeStatic(edict_t *ent)
{
   // debugFile( "pfnMakeStatic:\n");
   (*g_engfuncs.pfnMakeStatic)(ent);
}

int pfnEntIsOnFloor(edict_t *e)
{
   debugFile("pfnEntIsOnFloor:\n");
   return (*g_engfuncs.pfnEntIsOnFloor)(e);
}

int pfnDropToFloor(edict_t* e)
{
    debugFile("pfnDropToFloor:\n");
   return (*g_engfuncs.pfnDropToFloor)(e);
}

int pfnWalkMove(edict_t *ent, float yaw, float dist, int iMode)
{
    debugFile( "pfnWalkMove:\n");
   return (*g_engfuncs.pfnWalkMove)(ent, yaw, dist, iMode);
}

void pfnSetOrigin(edict_t *e, const float *rgflOrigin)
{
    debugFile("pfnSetOrigin:\n");
   (*g_engfuncs.pfnSetOrigin)(e, rgflOrigin);
}

void pfnTraceLine(const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr)
{
//   debugFile( "pfnTraceLine:\n");
   (*g_engfuncs.pfnTraceLine)(v1, v2, fNoMonsters, pentToSkip, ptr);
}

void pfnTraceToss(edict_t* pent, edict_t* pentToIgnore, TraceResult *ptr)
{
//   debugFile( "pfnTraceToss:\n");
   (*g_engfuncs.pfnTraceToss)(pent, pentToIgnore, ptr);
}

int pfnTraceMonsterHull(edict_t *pEdict, const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr)
{
//   debugFile( "pfnTraceMonsterHull:\n");
   return (*g_engfuncs.pfnTraceMonsterHull)(pEdict, v1, v2, fNoMonsters, pentToSkip, ptr);
}

void pfnTraceHull(const float *v1, const float *v2, int fNoMonsters, int hullNumber, edict_t *pentToSkip, TraceResult *ptr)
{
//   debugFile("pfnTraceHull:\n");
   (*g_engfuncs.pfnTraceHull)(v1, v2, fNoMonsters, hullNumber, pentToSkip, ptr);
}

void pfnTraceModel(const float *v1, const float *v2, int hullNumber, edict_t *pent, TraceResult *ptr)
{
//   debugFile("pfnTraceModel:\n");
   (*g_engfuncs.pfnTraceModel)(v1, v2, hullNumber, pent, ptr);
}

const char *pfnTraceTexture(edict_t *pTextureEntity, const float *v1, const float *v2 )
{
//   debugFile( "pfnTraceTexture:\n");
   return (*g_engfuncs.pfnTraceTexture)(pTextureEntity, v1, v2);
}

void pfnTraceSphere(const float *v1, const float *v2, int fNoMonsters, float radius, edict_t *pentToSkip, TraceResult *ptr)
{
//   debugFile("pfnTraceSphere:\n");
   (*g_engfuncs.pfnTraceSphere)(v1, v2, fNoMonsters, radius, pentToSkip, ptr);
}

void pfnGetAimVector(edict_t* ent, float speed, float *rgflReturn)
{
//   debugFile("pfnGetAimVector:\n");
   (*g_engfuncs.pfnGetAimVector)(ent, speed, rgflReturn);
}

void pfnServerExecute()
{
   debugFile("pfnServerExecute:\n");
   (*g_engfuncs.pfnServerExecute)();
}

void pfnParticleEffect(const float *org, const float *dir, float color, float count)
{
//   debugFile("pfnParticleEffect:\n");
   (*g_engfuncs.pfnParticleEffect)(org, dir, color, count);
}

void pfnLightStyle(int style, const char* val)
{
//   debugFile("pfnLightStyle:\n");
   (*g_engfuncs.pfnLightStyle)(style, val);
}

int pfnDecalIndex(const char *name)
{
//   debugFile("pfnDecalIndex:\n");
   return (*g_engfuncs.pfnDecalIndex)(name);
}

int pfnPointContents(const float *rgflVector)
{
//   debugFile("pfnPointContents:\n");
   return (*g_engfuncs.pfnPointContents)(rgflVector);
}
void pfnCVarRegister(cvar_t *pCvar)
{
   debugFile( "pfnCVarRegister:\n");
   (*g_engfuncs.pfnCVarRegister)(pCvar);
}

float pfnCVarGetFloat(const char *szVarName)
{
//   debugFile( "pfnCVarGetFloat: %s\n",szVarName);
   return (*g_engfuncs.pfnCVarGetFloat)(szVarName);
}

const char* pfnCVarGetString(const char *szVarName)
{
//   debugFile("pfnCVarGetString:\n");
   return (*g_engfuncs.pfnCVarGetString)(szVarName);
}

void pfnCVarSetFloat(const char *szVarName, float flValue)
{
//   debugFile("pfnCVarSetFloat:\n");
   (*g_engfuncs.pfnCVarSetFloat)(szVarName, flValue);
}

void pfnCVarSetString(const char *szVarName, const char *szValue)
{
//   debugFile("pfnCVarSetString:\n");
   (*g_engfuncs.pfnCVarSetString)(szVarName, szValue);
}

void pfnEngineFprintf( FILE *pfile, char *szFmt, ... )
{
   debugFile("pfnEngineFprintf:\n");
   va_list argptr;
   char string[1024];

   va_start (argptr, szFmt);
   vsprintf (string, szFmt, argptr);
   va_end (argptr);

   (*g_engfuncs.pfnEngineFprintf) (pfile, string);
}

void pfnAlertMessage( ALERT_TYPE atype, char *szFmt, ... )
{
   debugFile("pfnAlertMessage:\n");
   va_list argptr;
   static char string[1024];

   va_start (argptr, szFmt);
   vsprintf (string, szFmt, argptr);
   va_end (argptr);

   (*g_engfuncs.pfnAlertMessage)(atype, string);
}

void* pfnPvAllocEntPrivateData(edict_t *pEdict, int cb)
{
   debugFile("pfnPvAllocEntPrivateData:\n");
   return (*g_engfuncs.pfnPvAllocEntPrivateData)(pEdict, cb);
}

void* pfnPvEntPrivateData(edict_t *pEdict)
{
   debugFile("pfnPvEntPrivateData:\n");
   return (*g_engfuncs.pfnPvEntPrivateData)(pEdict);
}

void pfnFreeEntPrivateData(edict_t *pEdict)
{
   debugFile("pfnFreeEntPrivateData:\n");
   (*g_engfuncs.pfnFreeEntPrivateData)(pEdict);
}

const char* pfnSzFromIndex(int iString)
{
   debugFile("pfnSzFromIndex:\n");
   return (*g_engfuncs.pfnSzFromIndex)(iString);
}

int pfnAllocString(const char *szValue)
{
   debugFile("pfnAllocString:\n");
   return (*g_engfuncs.pfnAllocString)(szValue);
}

entvars_t* pfnGetVarsOfEnt(edict_t *pEdict)
{
   debugFile("pfnGetVarsOfEnt:\n");
   return (*g_engfuncs.pfnGetVarsOfEnt)(pEdict);
}

edict_t* pfnPEntityOfEntOffset(int iEntOffset)
{
//   debugFile("pfnPEntityOfEntOffset:\n");
   return (*g_engfuncs.pfnPEntityOfEntOffset)(iEntOffset);
}

int pfnEntOffsetOfPEntity(const edict_t *pEdict)
{
//   debugFile("pfnEntOffsetOfPEntity: %p\n",pEdict);
   return (*g_engfuncs.pfnEntOffsetOfPEntity)(pEdict);
}

int pfnIndexOfEdict(const edict_t *pEdict)
{
//   debugFile("pfnIndexOfEdict: %p\n",pEdict);
   return (*g_engfuncs.pfnIndexOfEdict)(pEdict);
}

edict_t* pfnPEntityOfEntIndex(int iEntIndex)
{
//   debugFile("pfnPEntityOfEntIndex:\n");
   return (*g_engfuncs.pfnPEntityOfEntIndex)(iEntIndex);
}

edict_t* pfnFindEntityByVars(entvars_t* pvars)
{
//   debugFile("pfnFindEntityByVars:\n");
   return (*g_engfuncs.pfnFindEntityByVars)(pvars);
}

void* pfnGetModelPtr(edict_t* pEdict)
{
//   debugFile("pfnGetModelPtr: %p\n",pEdict);
   return (*g_engfuncs.pfnGetModelPtr)(pEdict);
}

void pfnAnimationAutomove(const edict_t* pEdict, float flTime)
{
//   debugFile("pfnAnimationAutomove:\n");
   (*g_engfuncs.pfnAnimationAutomove)(pEdict, flTime);
}

void pfnGetBonePosition(const edict_t* pEdict, int iBone, float *rgflOrigin, float *rgflAngles )
{
//   debugFile("pfnGetBonePosition:\n");
   (*g_engfuncs.pfnGetBonePosition)(pEdict, iBone, rgflOrigin, rgflAngles);
}

unsigned long pfnFunctionFromName( const char *pName )
{
   debugFile("pfnFunctionFromName:\n");
   return (*g_engfuncs.pfnFunctionFromName)(pName);
}

const char *pfnNameForFunction( unsigned long function )
{
   debugFile("pfnNameForFunction:\n");
   return (*g_engfuncs.pfnNameForFunction)(function);
}

void pfnClientPrintf( edict_t* pEdict, PRINT_TYPE ptype, const char *szMsg )
{
   debugFile("pfnClientPrintf:\n");
   if (!(pEdict->v.flags & FL_FAKECLIENT)) (*g_engfuncs.pfnClientPrintf)(pEdict, ptype, szMsg);
}

void pfnServerPrint( const char *szMsg )
{
   debugFile("pfnServerPrint:\n");
   (*g_engfuncs.pfnServerPrint)(szMsg);
}

void pfnGetAttachment(const edict_t *pEdict, int iAttachment, float *rgflOrigin, float *rgflAngles )
{
//   debugFile("pfnGetAttachment:\n");
   (*g_engfuncs.pfnGetAttachment)(pEdict, iAttachment, rgflOrigin, rgflAngles);
}

void pfnCRC32_Init(CRC32_t *pulCRC)
{
   debugFile("pfnCRC32_Init:\n");
   (*g_engfuncs.pfnCRC32_Init)(pulCRC);
}

void pfnCRC32_ProcessBuffer(CRC32_t *pulCRC, void *p, int len)
{
   debugFile("pfnCRC32_ProcessBuffer:\n");
   (*g_engfuncs.pfnCRC32_ProcessBuffer)(pulCRC, p, len);
}

void pfnCRC32_ProcessByte(CRC32_t *pulCRC, unsigned char ch)
{
   debugFile("pfnCRC32_ProcessByte:\n");
   (*g_engfuncs.pfnCRC32_ProcessByte)(pulCRC, ch);
}

CRC32_t pfnCRC32_Final(CRC32_t pulCRC)
{
   debugFile("pfnCRC32_Final:\n");
   return (*g_engfuncs.pfnCRC32_Final)(pulCRC);
}

int pfnRandomLong(int lLow, int lHigh)
{
//   debugFile("pfnRandomLong: lLow=%d lHigh=%d\n",lLow,lHigh);
   return (*g_engfuncs.pfnRandomLong)(lLow, lHigh);
}

float pfnRandomFloat(float flLow, float flHigh)
{
//   debugFile("pfnRandomFloat:\n");
   return (*g_engfuncs.pfnRandomFloat)(flLow, flHigh);
}

void pfnSetView(const edict_t *pClient, const edict_t *pViewent )
{
   debugFile("pfnSetView:\n");
   (*g_engfuncs.pfnSetView)(pClient, pViewent);
}

float pfnTime()
{
   debugFile("pfnTime:\n");
   return (*g_engfuncs.pfnTime)();
}

void pfnCrosshairAngle(const edict_t *pClient, float pitch, float yaw)
{
   debugFile("pfnCrosshairAngle:\n");
   (*g_engfuncs.pfnCrosshairAngle)(pClient, pitch, yaw);
}

byte *pfnLoadFileForMe(const char *filename, int *pLength)
{
   debugFile("pfnLoadFileForMe: filename=%s\n",filename);
   return (*g_engfuncs.pfnLoadFileForMe)(filename, pLength);
}

void pfnFreeFile(void *buffer)
{
   debugFile("pfnFreeFile:\n");
   (*g_engfuncs.pfnFreeFile)(buffer);
}

void pfnEndSection(const char *pszSectionName)
{
   debugFile("pfnEndSection:\n");
   (*g_engfuncs.pfnEndSection)(pszSectionName);
}

int pfnCompareFileTime(const char *filename1, const char *filename2, int *iCompare)
{
   debugFile("pfnCompareFileTime:\n");
   return (*g_engfuncs.pfnCompareFileTime)(filename1, filename2, iCompare);
}

void pfnGetGameDir(const char *szGetGameDir)
{
   debugFile("pfnGetGameDir:\n");
   (*g_engfuncs.pfnGetGameDir)(szGetGameDir);
}

void pfnCvar_RegisterVariable(cvar_t *variable)
{
   debugFile("pfnCvar_RegisterVariable:\n");
   (*g_engfuncs.pfnCvar_RegisterVariable)(variable);
}

void pfnFadeClientVolume(const edict_t *pEdict, int fadePercent, int fadeOutSeconds, int holdTime, int fadeInSeconds)
{
   debugFile("pfnFadeClientVolume:\n");
   (*g_engfuncs.pfnFadeClientVolume)(pEdict, fadePercent, fadeOutSeconds, holdTime, fadeInSeconds);
}

edict_t * pfnCreateFakeClient(const char *netname)
{
   debugFile("pfnCreateFakeClient:\n");
   return (*g_engfuncs.pfnCreateFakeClient)(netname);
}

void pfnRunPlayerMove(edict_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, byte impulse, byte msec )
{
   debugFile("pfnRunPlayerMove: impulse=%i\n", impulse);
   (*g_engfuncs.pfnRunPlayerMove)(fakeclient, viewangles, forwardmove, sidemove, upmove, buttons, impulse, msec);
}

int pfnNumberOfEntities(void)
{
   debugFile("pfnNumberOfEntities:\n");
   return (*g_engfuncs.pfnNumberOfEntities)();
}

const char* pfnGetInfoKeyBuffer(edict_t *e)
{
   debugFile("pfnGetInfoKeyBuffer:\n");
   return (*g_engfuncs.pfnGetInfoKeyBuffer)(e);
}

const char* pfnInfoKeyValue(const char *infobuffer, const char *key)
{
   debugFile("pfnInfoKeyValue: %s %s\n",infobuffer,key);
   return (*g_engfuncs.pfnInfoKeyValue)(infobuffer, key);
}

void pfnSetKeyValue(const char *infobuffer, const char *key, const char *value)
{
   debugFile("pfnSetKeyValue: %s %s\n",key,value);
   (*g_engfuncs.pfnSetKeyValue)(infobuffer, key, value);
}

int pfnIsMapValid(const char *filename)
{
   debugFile("pfnIsMapValid:\n");
   return (*g_engfuncs.pfnIsMapValid)(filename);
}

void pfnStaticDecal( const float *origin, int decalIndex, int entityIndex, int modelIndex )
{
   debugFile("pfnStaticDecal:\n");
   (*g_engfuncs.pfnStaticDecal)(origin, decalIndex, entityIndex, modelIndex);
}

int pfnPrecacheGeneric(const char* s)
{
   debugFile("pfnPrecacheGeneric: %s\n",s);
   return (*g_engfuncs.pfnPrecacheGeneric)(s);
}

int pfnGetPlayerUserId(edict_t *e )
{
      debugFile( "pfnGetPlayerUserId: %p\n",e);

   return (*g_engfuncs.pfnGetPlayerUserId)(e);
}

void pfnBuildSoundMsg(edict_t *entity, int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch, int msg_dest, int msg_type, const float *pOrigin, edict_t *ed)
{
   debugFile("pfnBuildSoundMsg:\n");
   (*g_engfuncs.pfnBuildSoundMsg)(entity, channel, sample, volume, attenuation, fFlags, pitch, msg_dest, msg_type, pOrigin, ed);
}

int pfnIsDedicatedServer(void)
{
   debugFile("pfnIsDedicatedServer:\n");
   return (*g_engfuncs.pfnIsDedicatedServer)();
}

cvar_t* pfnCVarGetPointer(const char *szVarName)
{
   debugFile("pfnCVarGetPointer: %s\n",szVarName);
   return (*g_engfuncs.pfnCVarGetPointer)(szVarName);
}

unsigned int pfnGetPlayerWONId(edict_t *e)
{
   debugFile("pfnGetPlayerWONId: %p\n",e);
   return (*g_engfuncs.pfnGetPlayerWONId)(e);
}

// new stuff for SDK 2.0

void pfnInfo_RemoveKey(const char *s, const char *key)
{
   debugFile("pfnInfo_RemoveKey:\n");
   (*g_engfuncs.pfnInfo_RemoveKey)(s, key);
}

const char *pfnGetPhysicsKeyValue(const edict_t *pClient, const char *key)
{
	const char *res = (*g_engfuncs.pfnGetPhysicsKeyValue)(pClient, key);
	debugFile("pfnGetPhysicsKeyValue: key=%s, result=%s\n", key, res);
	//int ir = (int) res[0];
	//debugMsg( "PK=%i\n", ir );
	return res;
}

void pfnSetPhysicsKeyValue(const edict_t *pClient, const char *key, const char *value)
{
   debugFile("pfnSetPhysicsKeyValue:\n");
   (*g_engfuncs.pfnSetPhysicsKeyValue)(pClient, key, value);
}

const char *pfnGetPhysicsInfoString(const edict_t *pClient)
{
   debugFile("pfnGetPhysicsInfoString:\n");
   return (*g_engfuncs.pfnGetPhysicsInfoString)(pClient);
}

unsigned short pfnPrecacheEvent(int type, const char *psz)
{
   debugFile("pfnPrecacheEvent:\n");
   return (*g_engfuncs.pfnPrecacheEvent)(type, psz);
}

void pfnPlaybackEvent(int flags, const edict_t *pInvoker, unsigned short eventindex, float delay,
   float *origin, float *angles, float fparam1,float fparam2, int iparam1, int iparam2, int bparam1, int bparam2)
{
	debugFile("pfnPlaybackEvent(flags=%i,index=%i, delay=%.2f)\n", flags, eventindex, delay);
   (*g_engfuncs.pfnPlaybackEvent)(flags, pInvoker, eventindex, delay, origin, angles, fparam1, fparam2, iparam1, iparam2, bparam1, bparam2);
}

unsigned char *pfnSetFatPVS(float *org)
{
   debugFile("pfnSetFatPVS:\n");
   return (*g_engfuncs.pfnSetFatPVS)(org);
}

unsigned char *pfnSetFatPAS(float *org)
{
   debugFile("pfnSetFatPAS:\n");
   return (*g_engfuncs.pfnSetFatPAS)(org);
}

int pfnCheckVisibility(const edict_t *entity, unsigned char *pset)
{
//   debugFile("pfnCheckVisibility:\n");
   return (*g_engfuncs.pfnCheckVisibility)(entity, pset);
}

void pfnDeltaSetField(struct delta_s *pFields, const char *fieldname)
{
//   debugFile("pfnDeltaSetField:\n");
   (*g_engfuncs.pfnDeltaSetField)(pFields, fieldname);
}

void pfnDeltaUnsetField(struct delta_s *pFields, const char *fieldname)
{
//   debugFile("pfnDeltaUnsetField:\n");
   (*g_engfuncs.pfnDeltaUnsetField)(pFields, fieldname);
}

void pfnDeltaAddEncoder(const char *name, void (*conditionalencode)( struct delta_s *pFields, const unsigned char *from, const unsigned char *to))
{
//   debugFile("pfnDeltaAddEncoder:\n");
   (*g_engfuncs.pfnDeltaAddEncoder)(name, conditionalencode);
}

int pfnGetCurrentPlayer()
{
	debugFile("pfnGetCurrentPlayer: ");
   return (*g_engfuncs.pfnGetCurrentPlayer)();
}

int pfnCanSkipPlayer(const edict_t *player)
{
   debugFile("pfnCanSkipPlayer:\n");
   return (*g_engfuncs.pfnCanSkipPlayer)(player);
}

int pfnDeltaFindField(struct delta_s *pFields, const char *fieldname)
{
//   debugFile("pfnDeltaFindField:\n");
   return (*g_engfuncs.pfnDeltaFindField)(pFields, fieldname);
}

void pfnDeltaSetFieldByIndex(struct delta_s *pFields, int fieldNumber)
{
   debugFile("pfnDeltaSetFieldByIndex:\n");
   (*g_engfuncs.pfnDeltaSetFieldByIndex)(pFields, fieldNumber);
}

void pfnDeltaUnsetFieldByIndex(struct delta_s *pFields, int fieldNumber)
{
//   debugFile("pfnDeltaUnsetFieldByIndex:\n");
   (*g_engfuncs.pfnDeltaUnsetFieldByIndex)(pFields, fieldNumber);
}

void pfnSetGroupMask(int mask, int op)
{
   debugFile("pfnSetGroupMask:\n");
   (*g_engfuncs.pfnSetGroupMask)(mask, op);
}

int pfnCreateInstancedBaseline(int classname, struct entity_state_s *baseline)
{
   debugFile("pfnCreateInstancedBaseline:\n");
   return (*g_engfuncs.pfnCreateInstancedBaseline)(classname, baseline);
}

void pfnCvar_DirectSet(struct cvar_s *var, const char *value)
{
   debugFile("pfnCvar_DirectSet:\n");
   (*g_engfuncs.pfnCvar_DirectSet)(var, value);
}

void pfnForceUnmodified(FORCE_TYPE type, float *mins, float *maxs, const char *filename)
{
   debugFile("pfnForceUnmodified:\n");
   (*g_engfuncs.pfnForceUnmodified)(type, mins, maxs, filename);
}

void pfnGetPlayerStats(const edict_t *pClient, int *ping, int *packet_loss)
{
   debugFile("pfnGetPlayerStats:\n");
   (*g_engfuncs.pfnGetPlayerStats)(pClient, ping, packet_loss);
}

void pfnAddServerCommand( const char *cmd_name, void (*function) (void) )
{
   debugFile("pfnAddServerCommand:\n");
   (*g_engfuncs.pfnAddServerCommand)( cmd_name, function );
}

qboolean pfnVoice_GetClientListening(int iReceiver, int iSender)
{
   debugFile("pfnVoice_GetClientListening:\n");
   return (*g_engfuncs.pfnVoice_GetClientListening)(iReceiver, iSender);
}

qboolean pfnVoice_SetClientListening(int iReceiver, int iSender, qboolean bListen)
{
   debugFile("pfnVoice_SetClientListening:\n");
   return (*g_engfuncs.pfnVoice_SetClientListening)(iReceiver, iSender, bListen);
}

const char *pfnGetPlayerAuthId( edict_t *e )
{
   debugFile("pfnGetPlayerAuthId:\n");
   return (*g_engfuncs.pfnGetPlayerAuthId)(e);
}

void* pfnSequenceGet( const char* fileName, const char* entryName )
{
   debugFile("pfnSequenceGet:\n");
   return (*g_engfuncs.pfnSequenceGet)(fileName, entryName);
}

void* pfnSequencePickSentence( const char* groupName, int pickMethod, int *picked )
{
   debugFile("pfnSequencePickSentence:\n");
   return (*g_engfuncs.pfnSequencePickSentence)(groupName, pickMethod, picked);
}

int pfnGetFileSize( char *filename )
{
   debugFile("pfnGetFileSize:\n");
   return (*g_engfuncs.pfnGetFileSize)(filename);
}

unsigned int pfnGetApproxWavePlayLen(const char *filepath)
{
   debugFile("pfnGetApproxWavePlayLen:\n");
   return (*g_engfuncs.pfnGetApproxWavePlayLen)(filepath);
}

int pfnIsCareerMatch( void )
{
   debugFile( "pfnIsCareerMatch:\n");
   return (*g_engfuncs.pfnIsCareerMatch)();
}

int pfnGetLocalizedStringLength(const char *label)
{
   debugFile("pfnGetLocalizedStringLength:\n");
   return (*g_engfuncs.pfnGetLocalizedStringLength)(label);
}

void pfnRegisterTutorMessageShown(int mid)
{
   debugFile("pfnRegisterTutorMessageShown:\n");
   (*g_engfuncs.pfnRegisterTutorMessageShown)(mid);
}

int pfnGetTimesTutorMessageShown(int mid)
{
   debugFile("pfnGetTimesTutorMessageShown:\n");
   return (*g_engfuncs.pfnGetTimesTutorMessageShown)(mid);
}

void pfnProcessTutorMessageDecayBuffer(int *buffer, int bufferLength)
{
   debugFile("pfnProcessTutorMessageDecayBuffer:\n");
   (*g_engfuncs.pfnProcessTutorMessageDecayBuffer)(buffer, bufferLength);
}

void pfnConstructTutorMessageDecayBuffer(int *buffer, int bufferLength)
{
	debugFile("pfnConstructTutorMessageDecayBuffer:\n");
   (*g_engfuncs.pfnConstructTutorMessageDecayBuffer)(buffer, bufferLength);
}

void pfnResetTutorMessageDecayData()
{
   debugFile("pfnResetTutorMessageDecayData:\n");
   (*g_engfuncs.pfnResetTutorMessageDecayData)();
}

#endif //_DEBUG
