// vi: set ts=4 sw=4 :
// vim: set tw=75 :

// meta_api.h - description of metamod's DLL interface

/*
 * Copyright (c) 2001-2006 Will Day <willday@hpgx.net>
 *
 *    This file is part of Metamod.
 *
 *    Metamod is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU General Public License as published by the
 *    Free Software Foundation; either version 2 of the License, or (at
 *    your option) any later version.
 *
 *    Metamod is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Metamod; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#pragma once
#ifndef META_API_H
#define META_API_H

#define META_INTERFACE_VERSION "5:13"

// Flags for plugin to indicate when it can be be loaded/unloaded.
// NOTE: order is crucial, as greater/less comparisons are made.
typedef enum {
	PT_NEVER,
	PT_STARTUP,		// should only be loaded/unloaded at initial hlds execution
	PT_CHANGELEVEL,		// can be loaded/unloaded between maps
	PT_ANYTIME,		// can be loaded/unloaded at any time
	PT_ANYPAUSE		// can be loaded/unloaded at any time, and can be "paused" during a map
} PLUG_LOADTIME;

// Information plugin provides about itself.
typedef struct plugin_info {
	const char	*ifvers;	// meta_interface version
	const char	*name;		// full name of plugin
	const char	*version;	// version
	const char	*date;		// date
	const char	*author;	// author name/email
	const char	*url;		// URL
	const char	*logtag;	// log message prefix (unused right now)
	PLUG_LOADTIME	 loadable;	// when loadable
	PLUG_LOADTIME	 unloadable;	// when unloadable
} PLUGIN_INFO;

// Plugin identifier, passed to all Meta Utility Functions.
typedef PLUGIN_INFO* plid_t;
#define PLID	com.metamod.plugin_info

// Meta Utility Function table type.
typedef struct meta_util_funcs {
	void		 (*pfnLogConsole)		(plid_t, const char *, ...);
	void		 (*pfnLogMessage)		(plid_t, const char *, ...);
	void		 (*pfnLogError)			(plid_t, const char *, ...);
	void		 (*pfnLogDeveloper)		(plid_t, const char *, ...);
	void		 (*pfnCenterSay)		(plid_t, const char *, ...);
	void		 (*pfnCenterSayParms)		(plid_t, void *, const char *, ...);
	void		 (*pfnCenterSayVarargs)		(plid_t, void *, const char *, va_list);
	qboolean	 (*pfnCallGameEntity)		(plid_t, const char *, ENTVARS *);
	int		 (*pfnGetUserMsgID)		(plid_t, const char *, int *);
	const char	*(*pfnGetUserMsgName)		(plid_t, int, int *);
	const char	*(*pfnGetPluginPath)		(plid_t);
	const char	*(*pfnGetGameInfo)		(plid_t, void *);

	int		 (*pfnLoadPlugin)		(plid_t, const char *, PLUG_LOADTIME, void **);
	int		 (*pfnUnloadPlugin)		(plid_t, const char *, PLUG_LOADTIME, void *);
	int		 (*pfnUnloadPluginByHandle)	(plid_t, void *, PLUG_LOADTIME, void *);

	const char	*(*pfnIsQueryingClientCvar)	(plid_t, const EDICT *);

	int		 (*pfnMakeRequestID)		(plid_t);

	void		 (*pfnGetHookTables)		(plid_t, ENGINEAPI **, SERVERFUNCS **, SERVERFUNCS2 **);
} META_UTIL_FUNCS;

// Flags returned by a plugin's api function.
// NOTE: order is crucial, as greater/less comparisons are made.
typedef enum {
	MRES_UNSET = 0,
	MRES_IGNORED,		// plugin didn't take any action
	MRES_HANDLED,		// plugin did something, but real function should still be called
	MRES_OVERRIDE,		// call real function, but use my return value
	MRES_SUPERCEDE		// skip real function; use my return value
} META_RES;

// Variables provided to plugins.
typedef struct meta_globals {
	META_RES	 mres;			// writable; plugin's return flag
	META_RES	 prev_mres;		// readable; return flag of the previous plugin called
	META_RES	 status;		// readable; "highest" return flag so far
	void		*orig_ret;		// readable; return value from "real" function
	void		*override_ret;		// readable; return value from overriding/superceding plugin
} META_GLOBALS;

// Table of getapi functions, retrieved from each plugin.
typedef struct meta_funcs {
	int (*pfnGetEntityAPI)(SERVERFUNCS *, int);
	int (*pfnGetEntityAPI_Post)(SERVERFUNCS *, int);
	int (*pfnGetEntityAPI2)(SERVERFUNCS *, int *);
	int (*pfnGetEntityAPI2_Post)(SERVERFUNCS *, int *);
	int (*pfnGetNewDLLFunctions)(SERVERFUNCS2 *, int *);
	int (*pfnGetNewDLLFunctions_Post)(SERVERFUNCS2 *, int *);
	int (*pfnGetEngineFunctions)(ENGINEAPI *, int *);
	int (*pfnGetEngineFunctions_Post)(ENGINEAPI *, int *);
} META_FUNCS;

extern "C" EXPORT int	GetEntityAPI(SERVERFUNCS *functiontable, int interfaceversion);
extern "C" EXPORT int	GetNewDLLFunctions(SERVERFUNCS2 *functiontable, int *interfaceversion);
extern "C" EXPORT int	GetEngineFunctions(ENGINEAPI *engfuncs, int *interfaceVersion);

// Pair of function tables provided by game DLL.
typedef struct gamedllfuncs {
	SERVERFUNCS	*funcs;
	SERVERFUNCS2	*newfuncs;
} GAMEDLLFUNCS;

typedef struct meta_api_funcs {
	PLUGIN_INFO	*plugin_info;
	META_GLOBALS	*globals;
	META_UTIL_FUNCS	*utilfuncs;
} META_API_FUNCS;

#define SET_META_RESULT(result)			com.metamod.globals->mres=result
#define RETURN_META(result) \
	do { com.metamod.globals->mres = result; return; } while(0)
#define RETURN_META_VALUE(result, value) \
	do { com.metamod.globals->mres = result; return(value); } while(0)
#define META_RESULT_STATUS			com.metamod.globals->status
#define META_RESULT_PREVIOUS			com.metamod.globals->prev_mres
#define META_RESULT_ORIG_RET(type)		*(type *)com.metamod.globals->orig_ret
#define META_RESULT_OVERRIDE_RET(type)		*(type *)com.metamod.globals->override_ret

// Convenience macros for MetaUtil functions
#define CALL_GAME_ENTITY(entname, vars)	com.metamod.utilfuncs->pfnCallGameEntity(PLID, entname, vars)
#endif /* META_API_H */
