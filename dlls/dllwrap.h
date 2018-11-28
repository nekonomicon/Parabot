#pragma once
#if !defined(DLL_WRAP_H)
#define DLL_WRAP_H
inline int
clientconnect(EDICT *e, const char *name, const char *address, void *rejectreason)
{
	return com.gamedll->funcs->ClientConnect(e, name, address, rejectreason);
}

inline void
clientdisconnect(EDICT *e)
{
	com.gamedll->funcs->ClientDisconnect(e);
}

inline void
clientcommand(EDICT *e)
{
	com.gamedll->funcs->ClientCommand(e);
}

inline void
clientputinserver(EDICT *e)
{
	com.gamedll->funcs->ClientPutinServer(e);
}

inline signed char
find_texturetype(const char *name)
{
	return com.gamedll->funcs->PM_FindTextureType(name);
}

inline void
CMD_Start(EDICT *p, void *cmd, unsigned int random_seed)
{
	com.gamedll->funcs->CmdStart(p, cmd, random_seed);
}

inline void
CMD_End(EDICT *p)
{
	com.gamedll->funcs->CmdEnd(p);
}

inline void
gameinit()
{
	com.gamedll->funcs->GameInit();
}

inline void
startframe()
{
	com.gamedll->funcs->StartFrame();
}

inline int
spawn(EDICT *e)
{
	return com.gamedll->funcs->Spawn(e);
}

inline void
keyvalue(EDICT *e, KEYVALUE *pkvd)
{
	com.gamedll->funcs->KeyValue(e, pkvd);
}

inline void
serverdeactivate()
{
	com.gamedll->funcs->ServerDeactivate();
}

#endif // DLL_WRAP_H
