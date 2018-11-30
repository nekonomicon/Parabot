#include "parabot.h"
#include "pb_global.h"
#include "sectors.h"
#include "pb_mapgraph.h"
#include "vistable.h"
#include "pb_mapcells.h"
#include "bot.h"


PB_MapGraph mapGraph;	// mapgraph for waypoints
PB_MapCells map;

int activeBot;			// bot that's thinking
extern int botNr;		// bot that's getting debugged
extern int botHalt;		// if set to >0, breaks at checkForBreakpoint()

int gmsgParabot2dMsg = 0;
int gmsgParabot3dMsg = 0;

bool LOSExists(const Vec3D *v1, const Vec3D *v2)
// traces line with ignore monster from v1 to v2
{
	TRACERESULT tr;

	trace_line(v1, v2, true, true, 0, &tr);
	if (tr.startsolid)
		return false;
	return (tr.fraction == 1.0);
}


EDICT* getEntity( const char *classname, Vec3D *pos )
// returns a pointer to edict at pos if it exists, else 0
{
	EDICT *pOther = NULL;
	Vec3D p;

	while ((pOther = find_entitybyclassname(pOther, classname))) {
		boxcenter(pOther, &p);
/*		float d = vlen(pos - p);
		if ( Q_STREQ( STRING(pOther->v.classname), "func_train" ) ) {
			DEBUG_MSG( "train\n" );
		}*/
		if (vcomp(&p, pos))
			return pOther;
	}
	return 0;
}


PB_Navpoint& getNavpoint( int index )
{
	assert( index >= 0 );
	assert( index < mapGraph.numberOfNavpoints() );
	if ( index < 0 || index >= mapGraph.numberOfNavpoints() ) {
		DEBUG_MSG("Navpoint-Index-ERROR!\n" );
		return mapGraph[0].first;
	}
	return mapGraph[index].first;
}


int getNavpointIndex( EDICT *entity )
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

#if _DEBUG
void checkForBreakpoint( int reason )
{
	if ( (botNr==activeBot) && (botHalt==reason) ) {
		DEBUG_MSG( "Breakpoint reached\n" );
		botHalt = 0;
	}
}

void pb2dMsg( int x, int y, const char *szFmt, ... )
{
/*#if _DEBUG
	if (gmsgParabot2dMsg == 0)
		gmsgParabot2dMsg = REG_USER_MSG( "Pb2dMsg", -1 );
	
	if (gmsgParabot2dMsg > 0) {
		va_list argptr;
		char msg[1024];

		va_start (argptr, szFmt);
		vsprintf (msg, szFmt, argptr);
		va_end (argptr);

		EDICT *player = edictofindex( 1 );
		entvars_t *client = &(player->v);

		MESSAGE_BEGIN( MSG_ONE, gmsgParabot2dMsg, NULL, client );
		
		WRITE_SHORT( x );
		WRITE_SHORT( y );
		WRITE_STRING( msg );
				
		MESSAGE_END();
	}
#endif*/
}

void pb3dMsg( Vec3D *pos, const char *szFmt, ... )
{
/*#if _DEBUG
	if (gmsgParabot3dMsg == 0)
		gmsgParabot3dMsg = REG_USER_MSG( "Pb3dMsg", -1 );
	
	if (gmsgParabot3dMsg > 0) {
		va_list argptr;
		char msg[1024];

		va_start (argptr, szFmt);
		vsprintf (msg, szFmt, argptr);
		va_end (argptr);

		EDICT *player = edictofindex( 1 );
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

bool isOnScreen( EDICT *ent, EDICT *player )
{
	TRACERESULT tr;
	Vec3D playerpos, pos, dir;
	float dist, dot;

	// check if in visible distance
	eyepos(player, &playerpos);
	eyepos(ent, &pos);
	vsub(&pos, &playerpos, &dir);
	dist = vlen(&dir);
	if (dist > 1500.0f) return false;

	// check if in viewcone
	normalize(&dir);
	dot = dotproduct(com.globals->fwd, &dir);
	if (dot > 0.7f) {
		trace_line(&playerpos, &pos, false, true, player, &tr);
		if ( (tr.fraction == 1.0) || (tr.hit == ent) ) {
			return true;
		}
	}
	return false;
}

void print3dDebugInfo()
{
/*#if _DEBUG
	// draw 3d msg
	EDICT *player = edictofindex( 1 );
	UTIL_MakeVectors( player->v.v_angle );
	for (int i=0; i < com.globals->maxClients; i++) {
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

#if _DEBUG
void debugBeam( Vec3D *start, Vec3D *end, int life, int color )
{
	if (botNr!=activeBot) return;

	EDICT *player = edictofindex( 1 );

	MESSAGE_BEGIN( MSG_ONE, SVC_TEMPITY, NULL, player );
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

void debugMarker( Vec3D *pos, int life )
{
	if (botNr!=activeBot) return;

	EDICT *player = edictofindex( 1 );

	MESSAGE_BEGIN( MSG_ONE, SVC_TEMPITY, NULL, player );
	WRITE_BYTE( TE_IMPLOSION);
	WRITE_COORD(pos.x);
	WRITE_COORD(pos.y);
	WRITE_COORD(pos.z);
	WRITE_BYTE( 32 ); // radius
	WRITE_BYTE( 16 ); // count
	WRITE_BYTE( life ); // life in 0.1's
	MESSAGE_END();
}
#endif
