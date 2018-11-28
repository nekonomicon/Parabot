#include "parabot.h"
#include "meta_api.h"

extern "C" DLLEXPORT void Meta_Init()
{
	com.gamedll_flags |= GAMEDLL_METAMOD;
}

extern "C" DLLEXPORT bool Meta_Query(char *ifvers, PLUGIN_INFO **pluginfo, META_UTIL_FUNCS *metautilfuncs)
{
	// this function is the first function ever called by metamod in the plugin DLL. Its purpose
	// is for metamod to retrieve basic information about the plugin, such as its meta-interface
	// version, for ensuring compatibility with the current version of the running metamod.

	// keep track of the pointers to metamod function tables metamod gives us
	com.metamod.utilfuncs =			metautilfuncs;
	com.metamod.plugin_info =		(PLUGIN_INFO *)malloc(sizeof(PLUGIN_INFO));
	com.metamod.plugin_info->ifvers =	META_INTERFACE_VERSION;
	com.metamod.plugin_info->name =		"Parabot";
	com.metamod.plugin_info->version =	"0.92.1";
	com.metamod.plugin_info->date =		__DATE__;
	com.metamod.plugin_info->author =	"Andrey Akhmichin, Tobias Heimann & Bots-United";
	com.metamod.plugin_info->url =		"https://github.com/nekonomicon/Parabot";
	com.metamod.plugin_info->logtag =	"PARABOT";
	com.metamod.plugin_info->loadable =	PT_CHANGELEVEL;
	com.metamod.plugin_info->unloadable =	PT_ANYTIME;
	*pluginfo =				com.metamod.plugin_info;

	return true; // tell metamod this plugin looks safe
}

extern "C" DLLEXPORT bool Meta_Attach(PLUG_LOADTIME now, META_FUNCS *functiontable, META_GLOBALS *metaglobals, GAMEDLLFUNCS *gamedllfuncs)
{
	// this function is called when metamod attempts to load the plugin. Since it's the place
	// where we can tell if the plugin will be allowed to run or not, we wait until here to make
	// our initialization stuff, like registering CVARs and dedicated server commands.

	// keep track of the pointers to engine function tables metamod gives us
	com.metamod.globals = metaglobals;
	// memcpy(pFunctionTable, &gMetaFunctionTable, sizeof (META_FUNCTIONS));
	functiontable->pfnGetEntityAPI = GetEntityAPI;
	functiontable->pfnGetNewDLLFunctions = GetNewDLLFunctions;
	functiontable->pfnGetEngineFunctions = GetEngineFunctions;

	com.gamedll = gamedllfuncs;

	return true; // returning 1 enables metamod to attach this plugin
}

extern "C" DLLEXPORT bool Meta_Detach(PLUG_LOADTIME now, void *reason)
{
	// this function is called when metamod unloads the plugin. A basic check is made in order
	// to prevent unloading the plugin if its processing should not be interrupted.

	return true; // returning 1 enables metamod to unload this plugin
}
