#pragma once
#if !defined(ENG_WRAP_H)
#define ENG_WRAP_H
inline qboolean
is_dedicatedserver()
{
	return com.engfuncs->IsDedicatedServer();
}

inline void
changelevel(const char *map, const char *landmark)
{
	com.engfuncs->Changelevel(map, landmark);
}

// precache funcs
inline int
precache_model(const char *name)
{
	return com.engfuncs->PrecacheModel(name);
}

inline int
precache_sound(const char *name)
{
	return com.engfuncs->PrecacheSound(name);
}

inline int
precache_file(const char *name)
{
	return com.engfuncs->PrecacheGeneric(name);
}

// cvar funcs
inline void
cvar_register(CVAR *cvar)
{
	com.engfuncs->CvarRegister(cvar);
}

inline void
cvar_registervariable(CVAR *variable)
{
	com.engfuncs->Cvar_RegisterVariable(variable);
}

inline float
cvar_getfloat(const char *name)
{
	return com.engfuncs->CvarGetFloat(name);
}

inline const char *
cvar_getstring(const char *name)
{
	return com.engfuncs->CvarGetString(name);
}

inline CVAR *
cvar_getpointer(const char *name)
{
	return com.engfuncs->CvarGetPointer(name);
}

inline void
cvar_setfloat(const char *name, float value)
{
	com.engfuncs->CvarSetFloat(name, value);
}

inline void
cvar_setstring(const char *name, const char *value)
{
	com.engfuncs->CvarSetString(name, value);
}

// entity funcs
inline int
indexofedict(EDICT *e)
{
	return com.engfuncs->IndexOfEdict(e);
}

inline EDICT *
edictofindex(int i)
{
	return com.engfuncs->PEntityOfEntIndex(i);
}

inline void
removeentity(EDICT *e)
{
	com.engfuncs->RemoveEntity(e);
}

inline EDICT *
find_entitybystring(EDICT *start, const char *key, const char *value)
{
	return com.engfuncs->FindEntityByString(start, key, value);
}

inline EDICT *
find_entityinsphere(EDICT *start, Vec3D *origin, float radius)
{
	return com.engfuncs->FindEntityInSphere(start, origin, radius);
}

inline EDICT *
find_entitybyclassname(EDICT *start, const char *name)
{
	return com.engfuncs->FindEntityByString(start, "classname", name);
}

inline EDICT *
find_entitybytargetname(EDICT *start, const char *name)
{
	return com.engfuncs->FindEntityByString(start, "targetname", name);
}

inline EDICT *
find_entitybytarget(EDICT *start, const char *name)
{
	return com.engfuncs->FindEntityByString(start, "target", name);
}

// network message funcs
inline int
RegUserMSG(const char *name, int size)
{
	return com.engfuncs->RegUserMsg(name, size);
}

inline void
MSG_Begin(int dest, int type, const Vec3D *origin, EDICT *e)
{
	com.engfuncs->MessageBegin(dest, type, origin, e);
}

inline void
MSG_End()
{
	com.engfuncs->MessageEnd();
}

inline void
MSG_WriteByte(int v)
{
	com.engfuncs->WriteByte(v);
}

inline void
MSG_WriteChar(int v)
{
	com.engfuncs->WriteChar(v);
}

inline void
MSG_WriteShort(int v)
{
	com.engfuncs->WriteShort(v);
}

inline void
MSG_WriteLong(int v)
{
	com.engfuncs->WriteLong(v);
}

inline void
MSG_WriteAngle(float v)
{
	com.engfuncs->WriteAngle(v);
}

inline void
MSG_WriteCoord(float v)
{
	com.engfuncs->WriteCoord(v);
}

inline void
MSG_WriteString(const char *str)
{
	com.engfuncs->WriteString(str);
}

inline void
MSG_WriteEntity(int ent)
{
	com.engfuncs->WriteEntity(ent);
}

// sound funcs
inline void
sound(EDICT *e, int channel, const char *sample, float vol, float attenuation, int flags, int pitch)
{
	com.engfuncs->EmitSound(e, channel, sample, vol, attenuation, flags, pitch);
}

inline void
debugsound(EDICT *e, const char *sample)
{
#if _DEBUG
	com.engfuncs->EmitSound(e, CHAN_BODY, sample, 1.0f, ATTN_NORM, 0, 100);
#endif
}

inline void
ambientsound(EDICT *e, Vec3D *pos, const char *sample, float vol, float attenuation, int flags, int pitch)
{
	com.engfuncs->EmitAmbientSound(e, pos, sample, vol, attenuation, flags, pitch);
}

// file i/o funcs
inline byte *
loadfileforme(const char *fname, int *len)
{
	return com.engfuncs->LoadFileForMe(fname, len);
}

inline void
freefile(byte *buf)
{
	com.engfuncs->FreeFile(buf);
}

inline int
comparefiletime(const char *name1, const char *name2, int *compare)
{
	return com.engfuncs->CompareFileTime(name1, name2, compare);
}

inline void
getgamedir(char *gamedir)
{
	com.engfuncs->GetGameDir(gamedir);
}

inline bool
is_mapvalid(const char *filename)
{
	return com.engfuncs->IsMapValid(filename);
}

#if _DEBUG
#define debugFile(...) \
	{ \
		char logfile[64]; \
		if (com.gamedll_flags & GAMEDLL_DEBUG) { \
			sprintf(logfile, "%s/addons/parabot/log/debug.txt", com.modname); \
			FILE *fp = fopen(logfile, "a"); \
			fprintf(fp, __VA_ARGS__); \
			fclose(fp); \
		} \
	}
#else
#define debugFile(...)	(void)0
#endif

// cmd funcs
inline const char *
cmd_args()
{
	return com.engfuncs->Cmd_Args();
}

inline const char *
cmd_argv(int i)
{
	return com.engfuncs->Cmd_Argv(i);
}

inline int
cmd_argc()
{
	return com.engfuncs->Cmd_Argc();
}

inline void
servercommand(const char *command)
{
	com.engfuncs->ServerCommand(command);
}

inline void
serverexecute()
{
	com.engfuncs->ServerExecute();
}

#define CLIENT_COMMAND(p, ...)	com.engfuncs->ClientCommand(p, __VA_ARGS__);

// math funcs
inline int
randomint(int low, int high)
{
	return com.engfuncs->RandomLong(low, high);
}

inline float
randomfloat(float low, float high)
{
	return com.engfuncs->RandomFloat(low, high);
}

inline void
setorigin(EDICT *e, Vec3D *origin)
{
	com.engfuncs->SetOrigin(e, origin);
}

inline float
vectoyaw(Vec3D *vec)
{
	return com.engfuncs->VecToYaw(vec);
}

inline void
vectoangles(Vec3D *in, Vec3D *out)
{
	com.engfuncs->VecToAngles(in, out);
}

inline void
movetoorigin(EDICT *e, Vec3D *goal, float dist, int movetype)
{
	com.engfuncs->MoveToOrigin(e, goal, dist, movetype);
}

inline void
ChangeYaw(EDICT *e)
{
	com.engfuncs->ChangeYaw(e);
}

inline void
ChangePitch(EDICT *e)
{
	com.engfuncs->ChangePitch(e);
}

inline void
makevectors(Vec3D *angle)
{
	com.engfuncs->MakeVectors(angle);
}

inline void
getright(Vec3D *in, Vec3D *out)
{
	vectoangles(in, out);
        makevectors(out);
        out = &com.globals->right;
}

inline void
anglevectors(Vec3D *angles, Vec3D *forward, Vec3D *right, Vec3D *up)
{
	com.engfuncs->AngleVectors(angles, forward, right, up);
}

inline void
aim(EDICT *e, float missilespeed, Vec3D *ret)
{
	com.engfuncs->GetAimVector(e, missilespeed, ret);
}

// save/restore funcs
inline void *
functionfromname(const char *name)
{
	return com.engfuncs->FunctionFromName(name);
}

inline const char *
nameforfunction(void *func)
{
	return com.engfuncs->NameForFunction(func);
}

// trace funcs
enum {
	NO_GLASS = BIT(5)
};

inline void
trace_line(const Vec3D *start, const Vec3D *end, qboolean nomonsters, qboolean noglass, EDICT *skip, TRACERESULT *tr)
{
	com.engfuncs->TraceLine(start, end, noglass ? (nomonsters | NO_GLASS) : nomonsters, skip, tr);
}

inline void
trace_hull(Vec3D *start, Vec3D *end, qboolean nomonsters, int hull, EDICT *skip, TRACERESULT *tr)
{
	com.engfuncs->TraceHull(start, end, nomonsters, hull, skip, tr);
}

inline const char *
trace_texture(EDICT *e, Vec3D *start, Vec3D *end)
{
	return com.engfuncs->TraceTexture(e, start, end);
}

inline int
pointcontents(Vec3D *origin)
{
	return com.engfuncs->PointContents(origin);
}

// client funcs
inline void
setclientmaxspeed(EDICT *player, float maxspeed)
{
	com.engfuncs->SetClientMaxspeed(player, maxspeed);
}

inline EDICT *
createfakeclient(const char *netname)
{
	return com.engfuncs->CreateFakeClient(netname);
}

inline void
runplayermove(EDICT *player, const Vec3D *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, byte impulse, byte msec)
{
	com.engfuncs->RunPlayerMove(player, viewangles, forwardmove, sidemove, upmove, buttons, impulse, msec);
}

inline const char *
getinfokeybuffer(EDICT *e)
{
	return com.engfuncs->GetInfoKeyBuffer(e);
}

inline const char *
getinfokeyvalue(const char *ib, const char *key)
{
	return com.engfuncs->InfoKeyValue(ib, key);
}

inline void
setinfokeyvalue(const char *ib, const char *key, const char *value)
{
	com.engfuncs->SetInfoKeyValue(ib, key, value);
}

inline void
setclientkeyvalue(int clientindex, const char *ib, const char *key, const char *value)
{
	com.engfuncs->SetClientKeyValue(clientindex, ib, key, value);
}

// private data funcs
inline void *
alloc_privatedata(EDICT *e, size_t size)
{
	return com.engfuncs->PvAllocEntPrivateData(e, size);
}

inline void
free_privatedata(EDICT *e)
{
	com.engfuncs->FreeEntPrivateData(e);
}

// printing funcs
#define INFO_MSG(...) \
	if (com.engfuncs->IsDedicatedServer()) \
		printf(__VA_ARGS__); \
	else \
		com.engfuncs->AlertMessage(AT_CONSOLE, __VA_ARGS__)
#if _WIN32
#define ERROR_MSG(...) \
	{ \
		char string[256]; \
		sprintf(string, __VA_ARGS__); \
		MessageBox( NULL, string, "Parabot", MB_OK ); \
	}
#else // _WIN32
#define ERROR_MSG(...) \
	com.engfuncs->AlertMessage(AT_ERROR, __VA_ARGS__)
#endif // _WIN32

#if _DEBUG
#define DEBUG_MSG(...)	INFO_MSG(__VA_ARGS__)
#else // _DEBUG
#define DEBUG_MSG(...)	(void)0
#endif // _DEBUG
#endif // ENG_WRAP_H
