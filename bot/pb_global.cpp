#include "pb_global.h"
#include "pb_mapgraph.h"
#include "pb_mapcells.h"
#include "enginecallback.h"
#include "bot.h"


PB_MapGraph mapGraph;	// mapgraph for waypoints
PB_MapCells map;

int activeBot;			// bot that's thinking
extern int botNr;		// bot that's getting debugged
extern int botHalt;		// if set to >0, breaks at checkForBreakpoint()
extern char mod_name[32];

int gmsgParabot2dMsg = 0;
int gmsgParabot3dMsg = 0;

void pfnEmitSound( edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch );
// from engine.h



bool LOSExists( Vector v1, Vector v2 )
// traces line with ignore monster from v1 to v2
{
	TraceResult tr;

	UTIL_TraceLine( v1, v2, ignore_monsters, ignore_glass, 0, &tr);
	if (tr.fStartSolid) return false;
	return (tr.flFraction == 1.0);
}


edict_t* getEntity( const char *classname, Vector pos )
// returns a pointer to edict at pos if it exists, else 0
{
	edict_t *pOther = NULL;
	Vector p;
	bool found = false;

	while ( !FNullEnt(pOther = FIND_ENTITY_BY_CLASSNAME (pOther, classname))) {
		p = (pOther->v.absmax + pOther->v.absmin) * 0.5;
/*		float d = (pos-p).Length();
		if ( FStrEq( STRING(pOther->v.classname), "func_train" ) ) {
			debugMsg( "train\n" );
		}*/
		if (p==pos) { found=true; break; }
	}
	if (found) return pOther;
	else return 0;
}


PB_Navpoint& getNavpoint( int index )
{
	assert( index >= 0 );
	assert( index < mapGraph.numberOfNavpoints() );
	if ( index < 0 || index >= mapGraph.numberOfNavpoints() ) {
		debugMsg("Navpoint-Index-ERROR!\n" );
		return mapGraph[0].first;
	}
	return mapGraph[index].first;
}


int getNavpointIndex( edict_t *entity )
// returns the index of navpoint with given entity or -2 if not found
{
	for (int i=0; i<mapGraph.numberOfNavpoints(); i++)
		if (mapGraph[i].first.entity() == entity) return i;
	return -2;
}


PB_Path* getPath( int pathId )
{
	return mapGraph.findPath( pathId );
}


int getTotalAttempts()
{
	return mapGraph.getPassCount();
}


void incTotalAttempts()
{
	mapGraph.incPasses();
}

#ifdef _DEBUG
void checkForBreakpoint( int reason )
{
	if ( (botNr==activeBot) && (botHalt==reason) ) {
		debugMsg( "Breakpoint reached\n" );
		botHalt = 0;
	}
}

void pb2dMsg( int x, int y, const char *szFmt, ... )
{
/*#ifdef _DEBUG
	if (gmsgParabot2dMsg == 0)
		gmsgParabot2dMsg = REG_USER_MSG( "Pb2dMsg", -1 );
	
	if (gmsgParabot2dMsg > 0) {
		va_list argptr;
		char msg[1024];

		va_start (argptr, szFmt);
		vsprintf (msg, szFmt, argptr);
		va_end (argptr);

		edict_t *player = INDEXENT( 1 );
		entvars_t *client = &(player->v);

		MESSAGE_BEGIN( MSG_ONE, gmsgParabot2dMsg, NULL, client );
		
		WRITE_SHORT( x );
		WRITE_SHORT( y );
		WRITE_STRING( msg );
				
		MESSAGE_END();
	}
#endif*/
}

void pb3dMsg( Vector pos, const char *szFmt, ... )
{
/*#ifdef _DEBUG
	if (gmsgParabot3dMsg == 0)
		gmsgParabot3dMsg = REG_USER_MSG( "Pb3dMsg", -1 );
	
	if (gmsgParabot3dMsg > 0) {
		va_list argptr;
		char msg[1024];

		va_start (argptr, szFmt);
		vsprintf (msg, szFmt, argptr);
		va_end (argptr);

		edict_t *player = INDEXENT( 1 );
		entvars_t *client = &(player->v);

		MESSAGE_BEGIN( MSG_ONE, gmsgParabot3dMsg, NULL, client );
		
		WRITE_COORD( pos.x );
		WRITE_COORD( pos.y );
		WRITE_COORD( pos.z );
		WRITE_STRING( msg );
				
		MESSAGE_END();
	}
#endif*/
}

bool isOnScreen( edict_t *ent, edict_t *player )
{
	TraceResult tr;

	// check if in visible distance
	Vector playerpos = player->v.origin + player->v.view_ofs;
	Vector pos = ent->v.origin + ent->v.view_ofs;
	float dist = (pos - playerpos).Length();
	if (dist > 1500) return false;

	// check if in viewcone
	Vector dir = (pos - playerpos).Normalize();
	float dot = DotProduct( gpGlobals->v_forward, dir );
	if (  dot > 0.7 ) {
		UTIL_TraceLine( playerpos, pos, dont_ignore_monsters, ignore_glass, player, &tr);	
		if ( (tr.flFraction == 1.0) || (tr.pHit == ent) ) {
			return true;
		}
	}
	return false;
}

void print3dDebugInfo()
{
/*#ifdef _DEBUG
	// draw 3d msg
	edict_t *player = INDEXENT( 1 );
	UTIL_MakeVectors( player->v.v_angle );
	for (int i=0; i < gpGlobals->maxClients; i++) {
		if (bots[i].is_used && bots[i].respawn_state==RESPAWN_IDLE) {
			if (isOnScreen( bots[i].pEdict, player )) {
				char buffer[256];
				strcpy( buffer, STRING(bots[i].pEdict->v.netname) );
				strcat( buffer, "\n" );
				strcat( buffer, bots[i].parabot->goalMove );
				strcat( buffer, "\n" );
				strcat( buffer, bots[i].parabot->goalView );
				strcat( buffer, "\n" );
				strcat( buffer, bots[i].parabot->goalAct );
				pb3dMsg( bots[i].pEdict->v.origin+bots[i].pEdict->v.view_ofs, buffer );
			}
		}
	}
#endif*/
}
#endif

extern int wpBeamTexture;

#ifdef _DEBUG
void debugBeam( Vector start, Vector end, int life, int color )
{
	if (botNr!=activeBot) return;

	edict_t *player = INDEXENT( 1 );

	MESSAGE_BEGIN( MSG_ONE, SVC_TEMPENTITY, NULL, player );
	WRITE_BYTE( TE_BEAMPOINTS);
	WRITE_COORD(start.x);
	WRITE_COORD(start.y);
	WRITE_COORD(start.z);
	WRITE_COORD(end.x);
	WRITE_COORD(end.y);
	WRITE_COORD(end.z);
	WRITE_SHORT( wpBeamTexture );
	WRITE_BYTE( 0 ); // framestart
	WRITE_BYTE( 0 ); // framerate
	WRITE_BYTE( life ); // life in 0.1's
	WRITE_BYTE( 10 ); // width
	WRITE_BYTE( 0 );  // noise
	
	switch( color ) {
	case 0:	WRITE_BYTE( 255 );  WRITE_BYTE( 0 );  WRITE_BYTE( 0 );
			break;
	case 1:	WRITE_BYTE( 0 );  WRITE_BYTE( 255 );  WRITE_BYTE( 0 );
			break;
	case 2:	WRITE_BYTE( 0 );  WRITE_BYTE( 0 );  WRITE_BYTE( 255 );
			break;
	}
	WRITE_BYTE( 255 );   // brightness
	WRITE_BYTE( 5 );    // speed
	MESSAGE_END();
}

void debugMarker( Vector pos, int life )
{
	if (botNr!=activeBot) return;

	edict_t *player = INDEXENT( 1 );

	MESSAGE_BEGIN( MSG_ONE, SVC_TEMPENTITY, NULL, player );
	WRITE_BYTE( TE_IMPLOSION);
	WRITE_COORD(pos.x);
	WRITE_COORD(pos.y);
	WRITE_COORD(pos.z);
	WRITE_BYTE( 32 ); // radius
	WRITE_BYTE( 16 ); // count
	WRITE_BYTE( life ); // life in 0.1's
	MESSAGE_END();
}

void debugFile( const char *szFmt, ... )
{
	char logfile[64];
	va_list argptr;
	char string[1024];

	if (!FBitSet(g_uiGameFlags, GAME_DEBUG)) return;
	sprintf( logfile, "%s/addons/parabot/log/debug.txt", mod_name );
	FILE *fp = fopen( logfile, "a" ); 
      
	va_start (argptr, szFmt);
	vsprintf (string, szFmt, argptr);
	va_end (argptr);

	fprintf( fp, "%s", string );
	fclose( fp );
}

void debugMsg( const char *szFmt, ... )
{
	va_list argptr;
	char string[1024];

	if (botNr != activeBot) return;

	va_start (argptr, szFmt);
	vsprintf (string, szFmt, argptr);
	va_end (argptr);

	if (IS_DEDICATED_SERVER()) printf( "%s", string );
	else ALERT( at_console, string );
}
#endif

void errorMsg( const char *szFmt, ... )
{
	va_list argptr;
	char string[1024];

	va_start (argptr, szFmt);
	vsprintf (string, szFmt, argptr);
	va_end (argptr);
#ifdef _WIN32
	MessageBox( NULL, string, "Parabot", MB_OK );
#else
	ALERT( at_error, string );
#endif
}

void infoMsg( const char *szFmt, ... ) 
{
	va_list argptr;
	char string[1024];

	va_start (argptr, szFmt);
	vsprintf (string, szFmt, argptr);
	va_end (argptr);

	if (IS_DEDICATED_SERVER()) printf( "%s", string );
	else ALERT( at_console, string );
}

#ifdef _DEBUG
void debugSound( edict_t *recipient, const char *sample )
{
	pfnEmitSound( recipient, CHAN_BODY, sample, 1.0, ATTN_NORM, 0, 100 );
}
#endif
