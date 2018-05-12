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
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnPrecacheModel: %s\n",s); fclose(fp); }
#endif
   return (*g_engfuncs.pfnPrecacheModel)(s);
}

int pfnPrecacheSound( const char *s )
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnPrecacheSound: %s\n",s); fclose(fp); }
#endif
   return (*g_engfuncs.pfnPrecacheSound)(s);
}

void pfnSetModel(edict_t *e, const char *m)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetModel: edict=%p %s\n",e,m); fclose(fp); }
#endif
   (*g_engfuncs.pfnSetModel)(e, m);
}

int pfnModelIndex(const char *m)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnModelIndex: %s\n",m); fclose(fp); }
#endif
   return (*g_engfuncs.pfnModelIndex)(m);
}

int pfnModelFrames(int modelIndex)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnModelFrames:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnModelFrames)(modelIndex);
}

void pfnSetSize(edict_t *e, const float *rgflMin, const float *rgflMax)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetSize: %p\n",e); fclose(fp); }
#endif
   (*g_engfuncs.pfnSetSize)(e, rgflMin, rgflMax);
}

void pfnGetSpawnParms(edict_t *ent)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetSpawnParms:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnGetSpawnParms)(ent);
}

void pfnSaveSpawnParms(edict_t *ent)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSaveSpawnParms:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnSaveSpawnParms)(ent);
}

float pfnVecToYaw(const float *rgflVector)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnVecToYaw:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnVecToYaw)(rgflVector);
}

void pfnVecToAngles(const float *rgflVectorIn, float *rgflVectorOut)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnVecToAngles:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnVecToAngles)(rgflVectorIn, rgflVectorOut);
}

void pfnMoveToOrigin(edict_t *ent, const float *pflGoal, float dist, int iMoveType)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnMoveToOrigin:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnMoveToOrigin)(ent, pflGoal, dist, iMoveType);
}

void pfnChangeYaw(edict_t* ent)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnChangeYaw:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnChangeYaw)(ent);
}

void pfnChangePitch(edict_t* ent)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnChangePitch:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnChangePitch)(ent);
}

int pfnGetEntityIllum(edict_t* pEnt)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetEntityIllum:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnGetEntityIllum)(pEnt);
}

edict_t* pfnFindEntityInSphere(edict_t *pEdictStartSearchAfter, const float *org, float rad)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnFindEntityInSphere:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnFindEntityInSphere)(pEdictStartSearchAfter, org, rad);
}

edict_t* pfnFindClientInPVS(edict_t *pEdict)
{
#ifdef _DEBUG
   //if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnFindClientInPVS:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnFindClientInPVS)(pEdict);
}

edict_t* pfnEntitiesInPVS(edict_t *pplayer)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnEntitiesInPVS:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnEntitiesInPVS)(pplayer);
}

void pfnMakeVectors(const float *rgflVector)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnMakeVectors:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnMakeVectors)(rgflVector);
}

void pfnAngleVectors(const float *rgflVector, float *forward, float *right, float *up)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnAngleVectors:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnAngleVectors)(rgflVector, forward, right, up);
}

edict_t* pfnCreateEntity(void)
{
   edict_t *pent = (*g_engfuncs.pfnCreateEntity)();
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCreateEntity: %p\n",pent); fclose(fp); }
#endif
   return pent;
}

void pfnRemoveEntity(edict_t* e)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnRemoveEntity: %p\n",e); fclose(fp); }
   if (debug_engine)
   {
      FILE *fp = UTIL_OpenDebugLog();
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
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCreateNamedEntity: edict=%p name=%s\n",pent,STRING(className)); fclose(fp); }
#endif
   return pent;
}

void pfnMakeStatic(edict_t *ent)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnMakeStatic:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnMakeStatic)(ent);
}

int pfnEntIsOnFloor(edict_t *e)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnEntIsOnFloor:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnEntIsOnFloor)(e);
}

int pfnDropToFloor(edict_t* e)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnDropToFloor:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnDropToFloor)(e);
}

int pfnWalkMove(edict_t *ent, float yaw, float dist, int iMode)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnWalkMove:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnWalkMove)(ent, yaw, dist, iMode);
}

void pfnSetOrigin(edict_t *e, const float *rgflOrigin)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetOrigin:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnSetOrigin)(e, rgflOrigin);
}

void pfnTraceLine(const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnTraceLine:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnTraceLine)(v1, v2, fNoMonsters, pentToSkip, ptr);
}

void pfnTraceToss(edict_t* pent, edict_t* pentToIgnore, TraceResult *ptr)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnTraceToss:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnTraceToss)(pent, pentToIgnore, ptr);
}

int pfnTraceMonsterHull(edict_t *pEdict, const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnTraceMonsterHull:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnTraceMonsterHull)(pEdict, v1, v2, fNoMonsters, pentToSkip, ptr);
}

void pfnTraceHull(const float *v1, const float *v2, int fNoMonsters, int hullNumber, edict_t *pentToSkip, TraceResult *ptr)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnTraceHull:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnTraceHull)(v1, v2, fNoMonsters, hullNumber, pentToSkip, ptr);
}

void pfnTraceModel(const float *v1, const float *v2, int hullNumber, edict_t *pent, TraceResult *ptr)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnTraceModel:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnTraceModel)(v1, v2, hullNumber, pent, ptr);
}

const char *pfnTraceTexture(edict_t *pTextureEntity, const float *v1, const float *v2 )
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnTraceTexture:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnTraceTexture)(pTextureEntity, v1, v2);
}

void pfnTraceSphere(const float *v1, const float *v2, int fNoMonsters, float radius, edict_t *pentToSkip, TraceResult *ptr)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnTraceSphere:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnTraceSphere)(v1, v2, fNoMonsters, radius, pentToSkip, ptr);
}

void pfnGetAimVector(edict_t* ent, float speed, float *rgflReturn)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetAimVector:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnGetAimVector)(ent, speed, rgflReturn);
}

void pfnServerExecute()
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnServerExecute:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnServerExecute)();
}

void pfnParticleEffect(const float *org, const float *dir, float color, float count)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnParticleEffect:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnParticleEffect)(org, dir, color, count);
}

void pfnLightStyle(int style, const char* val)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnLightStyle:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnLightStyle)(style, val);
}

int pfnDecalIndex(const char *name)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnDecalIndex:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnDecalIndex)(name);
}

int pfnPointContents(const float *rgflVector)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnPointContents:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnPointContents)(rgflVector);
}
void pfnCVarRegister(cvar_t *pCvar)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCVarRegister:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnCVarRegister)(pCvar);
}

float pfnCVarGetFloat(const char *szVarName)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCVarGetFloat: %s\n",szVarName); fclose(fp); }
#endif
   return (*g_engfuncs.pfnCVarGetFloat)(szVarName);
}

const char* pfnCVarGetString(const char *szVarName)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCVarGetString:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnCVarGetString)(szVarName);
}

void pfnCVarSetFloat(const char *szVarName, float flValue)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCVarSetFloat:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnCVarSetFloat)(szVarName, flValue);
}

void pfnCVarSetString(const char *szVarName, const char *szValue)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCVarSetString:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnCVarSetString)(szVarName, szValue);
}

void pfnEngineFprintf( FILE *pfile, char *szFmt, ... )
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnEngineFprintf:\n"); fclose(fp); }
#endif
   va_list argptr;
   static char string[1024];

   va_start (argptr, szFmt);
   vsprintf (string, szFmt, argptr);
   va_end (argptr);

   (*g_engfuncs.pfnEngineFprintf) (pfile, string);
}

void pfnAlertMessage( ALERT_TYPE atype, char *szFmt, ... )
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnAlertMessage:\n"); fclose(fp); }
#endif
   va_list argptr;
   static char string[1024];

   va_start (argptr, szFmt);
   vsprintf (string, szFmt, argptr);
   va_end (argptr);

   (*g_engfuncs.pfnAlertMessage)(atype, string);
}

void* pfnPvAllocEntPrivateData(edict_t *pEdict, int cb)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnPvAllocEntPrivateData:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnPvAllocEntPrivateData)(pEdict, cb);
}

void* pfnPvEntPrivateData(edict_t *pEdict)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnPvEntPrivateData:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnPvEntPrivateData)(pEdict);
}

void pfnFreeEntPrivateData(edict_t *pEdict)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnFreeEntPrivateData:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnFreeEntPrivateData)(pEdict);
}

const char* pfnSzFromIndex(int iString)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSzFromIndex:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnSzFromIndex)(iString);
}

int pfnAllocString(const char *szValue)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnAllocString:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnAllocString)(szValue);
}

entvars_t* pfnGetVarsOfEnt(edict_t *pEdict)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetVarsOfEnt:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnGetVarsOfEnt)(pEdict);
}

edict_t* pfnPEntityOfEntOffset(int iEntOffset)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnPEntityOfEntOffset:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnPEntityOfEntOffset)(iEntOffset);
}

int pfnEntOffsetOfPEntity(const edict_t *pEdict)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnEntOffsetOfPEntity: %p\n",pEdict); fclose(fp); }
#endif
   return (*g_engfuncs.pfnEntOffsetOfPEntity)(pEdict);
}

int pfnIndexOfEdict(const edict_t *pEdict)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnIndexOfEdict: %p\n",pEdict); fclose(fp); }
#endif
   return (*g_engfuncs.pfnIndexOfEdict)(pEdict);
}

edict_t* pfnPEntityOfEntIndex(int iEntIndex)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnPEntityOfEntIndex:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnPEntityOfEntIndex)(iEntIndex);
}

edict_t* pfnFindEntityByVars(entvars_t* pvars)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnFindEntityByVars:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnFindEntityByVars)(pvars);
}

void* pfnGetModelPtr(edict_t* pEdict)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetModelPtr: %p\n",pEdict); fclose(fp); }
#endif
   return (*g_engfuncs.pfnGetModelPtr)(pEdict);
}

void pfnAnimationAutomove(const edict_t* pEdict, float flTime)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnAnimationAutomove:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnAnimationAutomove)(pEdict, flTime);
}

void pfnGetBonePosition(const edict_t* pEdict, int iBone, float *rgflOrigin, float *rgflAngles )
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetBonePosition:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnGetBonePosition)(pEdict, iBone, rgflOrigin, rgflAngles);
}

unsigned long pfnFunctionFromName( const char *pName )
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnFunctionFromName:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnFunctionFromName)(pName);
}

const char *pfnNameForFunction( unsigned long function )
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnNameForFunction:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnNameForFunction)(function);
}

void pfnClientPrintf( edict_t* pEdict, PRINT_TYPE ptype, const char *szMsg )
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnClientPrintf:\n"); fclose(fp); }
#endif
   if (!(pEdict->v.flags & FL_FAKECLIENT)) (*g_engfuncs.pfnClientPrintf)(pEdict, ptype, szMsg);
}

void pfnServerPrint( const char *szMsg )
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnServerPrint:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnServerPrint)(szMsg);
}

void pfnGetAttachment(const edict_t *pEdict, int iAttachment, float *rgflOrigin, float *rgflAngles )
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetAttachment:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnGetAttachment)(pEdict, iAttachment, rgflOrigin, rgflAngles);
}

void pfnCRC32_Init(CRC32_t *pulCRC)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCRC32_Init:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnCRC32_Init)(pulCRC);
}

void pfnCRC32_ProcessBuffer(CRC32_t *pulCRC, void *p, int len)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCRC32_ProcessBuffer:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnCRC32_ProcessBuffer)(pulCRC, p, len);
}

void pfnCRC32_ProcessByte(CRC32_t *pulCRC, unsigned char ch)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCRC32_ProcessByte:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnCRC32_ProcessByte)(pulCRC, ch);
}

CRC32_t pfnCRC32_Final(CRC32_t pulCRC)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCRC32_Final:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnCRC32_Final)(pulCRC);
}

int pfnRandomLong(int lLow, int lHigh)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnRandomLong: lLow=%d lHigh=%d\n",lLow,lHigh); fclose(fp); }
#endif
   return (*g_engfuncs.pfnRandomLong)(lLow, lHigh);
}

float pfnRandomFloat(float flLow, float flHigh)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnRandomFloat:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnRandomFloat)(flLow, flHigh);
}

void pfnSetView(const edict_t *pClient, const edict_t *pViewent )
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetView:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnSetView)(pClient, pViewent);
}

float pfnTime( void )
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnTime:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnTime)();
}

void pfnCrosshairAngle(const edict_t *pClient, float pitch, float yaw)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCrosshairAngle:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnCrosshairAngle)(pClient, pitch, yaw);
}

byte *pfnLoadFileForMe(const char *filename, int *pLength)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnLoadFileForMe: filename=%s\n",filename); fclose(fp); }
#endif
   return (*g_engfuncs.pfnLoadFileForMe)(filename, pLength);
}

void pfnFreeFile(void *buffer)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnFreeFile:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnFreeFile)(buffer);
}

void pfnEndSection(const char *pszSectionName)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnEndSection:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnEndSection)(pszSectionName);
}

int pfnCompareFileTime(const char *filename1, const char *filename2, int *iCompare)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCompareFileTime:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnCompareFileTime)(filename1, filename2, iCompare);
}

void pfnGetGameDir(const char *szGetGameDir)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetGameDir:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnGetGameDir)(szGetGameDir);
}

void pfnCvar_RegisterVariable(cvar_t *variable)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCvar_RegisterVariable:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnCvar_RegisterVariable)(variable);
}

void pfnFadeClientVolume(const edict_t *pEdict, int fadePercent, int fadeOutSeconds, int holdTime, int fadeInSeconds)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnFadeClientVolume:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnFadeClientVolume)(pEdict, fadePercent, fadeOutSeconds, holdTime, fadeInSeconds);
}

edict_t * pfnCreateFakeClient(const char *netname)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCreateFakeClient:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnCreateFakeClient)(netname);
}

void pfnRunPlayerMove(edict_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, byte impulse, byte msec )
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnRunPlayerMove: impulse=%i\n", impulse); fclose(fp); }
#endif
   (*g_engfuncs.pfnRunPlayerMove)(fakeclient, viewangles, forwardmove, sidemove, upmove, buttons, impulse, msec);
}

int pfnNumberOfEntities(void)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnNumberOfEntities:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnNumberOfEntities)();
}

const char* pfnGetInfoKeyBuffer(edict_t *e)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetInfoKeyBuffer:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnGetInfoKeyBuffer)(e);
}

const char* pfnInfoKeyValue(const char *infobuffer, const char *key)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnInfoKeyValue: %s %s\n",infobuffer,key); fclose(fp); }
#endif
   return (*g_engfuncs.pfnInfoKeyValue)(infobuffer, key);
}

void pfnSetKeyValue(const char *infobuffer, const char *key, const char *value)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetKeyValue: %s %s\n",key,value); fclose(fp); }
#endif
   (*g_engfuncs.pfnSetKeyValue)(infobuffer, key, value);
}

int pfnIsMapValid(const char *filename)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnIsMapValid:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnIsMapValid)(filename);
}

void pfnStaticDecal( const float *origin, int decalIndex, int entityIndex, int modelIndex )
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnStaticDecal:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnStaticDecal)(origin, decalIndex, entityIndex, modelIndex);
}

int pfnPrecacheGeneric(const char* s)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnPrecacheGeneric: %s\n",s); fclose(fp); }
#endif
   return (*g_engfuncs.pfnPrecacheGeneric)(s);
}

int pfnGetPlayerUserId(edict_t *e )
{
   if (gpGlobals->deathmatch)
   {
#ifdef _DEBUG
      if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetPlayerUserId: %p\n",e); fclose(fp); }
#endif
   }

   return (*g_engfuncs.pfnGetPlayerUserId)(e);
}

void pfnBuildSoundMsg(edict_t *entity, int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch, int msg_dest, int msg_type, const float *pOrigin, edict_t *ed)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnBuildSoundMsg:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnBuildSoundMsg)(entity, channel, sample, volume, attenuation, fFlags, pitch, msg_dest, msg_type, pOrigin, ed);
}

int pfnIsDedicatedServer(void)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnIsDedicatedServer:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnIsDedicatedServer)();
}

cvar_t* pfnCVarGetPointer(const char *szVarName)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCVarGetPointer: %s\n",szVarName); fclose(fp); }
#endif
   return (*g_engfuncs.pfnCVarGetPointer)(szVarName);
}

unsigned int pfnGetPlayerWONId(edict_t *e)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetPlayerWONId: %p\n",e); fclose(fp); }
#endif
   return (*g_engfuncs.pfnGetPlayerWONId)(e);
}

// new stuff for SDK 2.0

void pfnInfo_RemoveKey(const char *s, const char *key)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnInfo_RemoveKey:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnInfo_RemoveKey)(s, key);
}

const char *pfnGetPhysicsKeyValue(const edict_t *pClient, const char *key)
{
	const char *res = (*g_engfuncs.pfnGetPhysicsKeyValue)(pClient, key);
#ifdef _DEBUG
	if (debug_engine) { 
		FILE *fp = UTIL_OpenDebugLog(); 
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
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetPhysicsKeyValue:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnSetPhysicsKeyValue)(pClient, key, value);
}

const char *pfnGetPhysicsInfoString(const edict_t *pClient)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetPhysicsInfoString:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnGetPhysicsInfoString)(pClient);
}

unsigned short pfnPrecacheEvent(int type, const char *psz)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnPrecacheEvent:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnPrecacheEvent)(type, psz);
}

void pfnPlaybackEvent(int flags, const edict_t *pInvoker, unsigned short eventindex, float delay,
   float *origin, float *angles, float fparam1,float fparam2, int iparam1, int iparam2, int bparam1, int bparam2)
{
#ifdef _DEBUG
	if (debug_engine) { 
		FILE *fp = UTIL_OpenDebugLog(); 
		fprintf(fp,"pfnPlaybackEvent(flags=%i,index=%i, delay=%.2f)\n", flags, eventindex, delay); 
		fclose(fp); 
	}
#endif
   (*g_engfuncs.pfnPlaybackEvent)(flags, pInvoker, eventindex, delay, origin, angles, fparam1, fparam2, iparam1, iparam2, bparam1, bparam2);
}

unsigned char *pfnSetFatPVS(float *org)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetFatPVS:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnSetFatPVS)(org);
}

unsigned char *pfnSetFatPAS(float *org)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetFatPAS:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnSetFatPAS)(org);
}

int pfnCheckVisibility(const edict_t *entity, unsigned char *pset)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCheckVisibility:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnCheckVisibility)(entity, pset);
}

void pfnDeltaSetField(struct delta_s *pFields, const char *fieldname)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnDeltaSetField:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnDeltaSetField)(pFields, fieldname);
}

void pfnDeltaUnsetField(struct delta_s *pFields, const char *fieldname)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnDeltaUnsetField:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnDeltaUnsetField)(pFields, fieldname);
}

void pfnDeltaAddEncoder(const char *name, void (*conditionalencode)( struct delta_s *pFields, const unsigned char *from, const unsigned char *to))
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnDeltaAddEncoder:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnDeltaAddEncoder)(name, conditionalencode);
}

int pfnGetCurrentPlayer(void)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetCurrentPlayer: "); fclose(fp); }
#endif
   return (*g_engfuncs.pfnGetCurrentPlayer)();
}

int pfnCanSkipPlayer(const edict_t *player)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCanSkipPlayer:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnCanSkipPlayer)(player);
}

int pfnDeltaFindField(struct delta_s *pFields, const char *fieldname)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnDeltaFindField:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnDeltaFindField)(pFields, fieldname);
}

void pfnDeltaSetFieldByIndex(struct delta_s *pFields, int fieldNumber)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnDeltaSetFieldByIndex:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnDeltaSetFieldByIndex)(pFields, fieldNumber);
}

void pfnDeltaUnsetFieldByIndex(struct delta_s *pFields, int fieldNumber)
{
#ifdef _DEBUG
//   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnDeltaUnsetFieldByIndex:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnDeltaUnsetFieldByIndex)(pFields, fieldNumber);
}

void pfnSetGroupMask(int mask, int op)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSetGroupMask:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnSetGroupMask)(mask, op);
}

int pfnCreateInstancedBaseline(int classname, struct entity_state_s *baseline)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCreateInstancedBaseline:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnCreateInstancedBaseline)(classname, baseline);
}

void pfnCvar_DirectSet(struct cvar_s *var, const char *value)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnCvar_DirectSet:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnCvar_DirectSet)(var, value);
}

void pfnForceUnmodified(FORCE_TYPE type, float *mins, float *maxs, const char *filename)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnForceUnmodified:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnForceUnmodified)(type, mins, maxs, filename);
}

void pfnGetPlayerStats(const edict_t *pClient, int *ping, int *packet_loss)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetPlayerStats:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnGetPlayerStats)(pClient, ping, packet_loss);
}

void pfnAddServerCommand( const char *cmd_name, void (*function) (void) )
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnAddServerCommand:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnAddServerCommand)( cmd_name, function );
}

qboolean pfnVoice_GetClientListening(int iReceiver, int iSender)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnVoice_GetClientListening:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnVoice_GetClientListening)(iReceiver, iSender);
}

qboolean pfnVoice_SetClientListening(int iReceiver, int iSender, qboolean bListen)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnVoice_SetClientListening:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnVoice_SetClientListening)(iReceiver, iSender, bListen);
}

const char *pfnGetPlayerAuthId( edict_t *e )
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetPlayerAuthId:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnGetPlayerAuthId)(e);
}

void* pfnSequenceGet( const char* fileName, const char* entryName )
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSequenceGet:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnSequenceGet)(fileName, entryName);
}

void* pfnSequencePickSentence( const char* groupName, int pickMethod, int *picked )
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnSequencePickSentence:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnSequencePickSentence)(groupName, pickMethod, picked);
}

int pfnGetFileSize( char *filename )
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetFileSize:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnGetFileSize)(filename);
}

unsigned int pfnGetApproxWavePlayLen(const char *filepath)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetApproxWavePlayLen:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnGetApproxWavePlayLen)(filepath);
}

int pfnIsCareerMatch( void )
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnIsCareerMatch:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnIsCareerMatch)();
}

int pfnGetLocalizedStringLength(const char *label)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetLocalizedStringLength:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnGetLocalizedStringLength)(label);
}

void pfnRegisterTutorMessageShown(int mid)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnRegisterTutorMessageShown:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnRegisterTutorMessageShown)(mid);
}

int pfnGetTimesTutorMessageShown(int mid)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnGetTimesTutorMessageShown:\n"); fclose(fp); }
#endif
   return (*g_engfuncs.pfnGetTimesTutorMessageShown)(mid);
}

void pfnProcessTutorMessageDecayBuffer(int *buffer, int bufferLength)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnProcessTutorMessageDecayBuffer:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnProcessTutorMessageDecayBuffer)(buffer, bufferLength);
}

void pfnConstructTutorMessageDecayBuffer(int *buffer, int bufferLength)
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnConstructTutorMessageDecayBuffer:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnConstructTutorMessageDecayBuffer)(buffer, bufferLength);
}

void pfnResetTutorMessageDecayData( void )
{
#ifdef _DEBUG
   if (debug_engine) { FILE *fp = UTIL_OpenDebugLog(); fprintf(fp,"pfnResetTutorMessageDecayData:\n"); fclose(fp); }
#endif
   (*g_engfuncs.pfnResetTutorMessageDecayData)();
}

#endif //_DEBUG
