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

int gmsgParabot2dMsg = 0;
int gmsgParabot3dMsg = 0;

bool dbgFile = true;



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
	CBaseEntity *pOther = NULL;
	Vector p;
	bool found = false;

	while ( (pOther = UTIL_FindEntityByClassname (pOther, classname)) != NULL) {
		p = (pOther->pev->absmax + pOther->pev->absmin) * 0.5;
/*		float d = (pos-p).Length();
		if ( strcmp( STRING(pOther->pev->classname), "func_train" )==0 ) {
			debugMsg( "train\n" );
		}*/
		if (p==pos) { found=true; break; }
	}
	if (found) return pOther->edict();
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


void checkForBreakpoint( int reason )
{
#ifdef _DEBUG
	if ( (botNr==activeBot) && (botHalt==reason) ) {
		debugMsg( "Breakpoint reached\n" );
		botHalt = 0;
	}
#endif
}


void pb2dMsg( int x, int y, const char *msg )
{
/*#ifdef _DEBUG
	if (gmsgParabot2dMsg == 0)
		gmsgParabot2dMsg = REG_USER_MSG( "Pb2dMsg", -1 );
	
	if (gmsgParabot2dMsg > 0) {
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


void pb3dMsg( Vector pos, const char *msg )
{
/*#ifdef _DEBUG
	if (gmsgParabot3dMsg == 0)
		gmsgParabot3dMsg = REG_USER_MSG( "Pb3dMsg", -1 );
	
	if (gmsgParabot3dMsg > 0) {
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


extern int wpBeamTexture;

void debugBeam( Vector start, Vector end, int life, int color )
{
#ifdef _DEBUG
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
#endif
}


void debugMarker( Vector pos, int life )
{
#ifdef _DEBUG
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
#endif
}


void debugFile( char *msg )
{
#ifdef _DEBUG
	if (!dbgFile) return;
	FILE *fp=fopen( "parabot/debug.txt", "a" ); 
	fprintf( fp, msg ); 
	fclose( fp );
#endif
}


void debugMsg( const char *str1, const char *str2, const char *str3, const char *str4 )
{
//#ifdef _DEBUG
	if (botNr != activeBot) return;
	char buffer[256];
	
	strcpy( buffer, str1 );
	if (str2) {
		strcat( buffer, str2 );
		if (str3) {
			strcat( buffer, str3 );
			if (str4) strcat( buffer, str4 );
		}
	}
	if (IS_DEDICATED_SERVER()) printf( buffer );
	else ALERT( at_console, buffer );
//#endif
}


void errorMsg( const char *str1, const char *str2, const char *str3, const char *str4 )
{
	char buffer[256];
	
	//strcpy( buffer, "Parabot - " );
	strcpy( buffer, str1 );
	if (str2) {
		strcat( buffer, str2 );
		if (str3) {
			strcat( buffer, str3 );
			if (str4) strcat( buffer, str4 );
		}
	}
	//ALERT( at_error, buffer );
	MessageBox( NULL, buffer, "Parabot", MB_OK );
	//PostQuitMessage(0);	
}


void infoMsg( const char *str1, const char *str2, const char *str3, const char *str4 ) 
{
	char buffer[256];
	
	strcpy( buffer, str1 );
	if (str2) {
		strcat( buffer, str2 );
		if (str3) {
			strcat( buffer, str3 );
			if (str4) strcat( buffer, str4 );
		}
	}
	if (IS_DEDICATED_SERVER()) printf( buffer );
	else ALERT( at_console, buffer );
}


void debugMsg( const char *str1, int data1, int data2, int data3 )
{
#ifdef _DEBUG
	if (botNr != activeBot) return;
	ALERT ( at_console, (char*)str1, data1, data2, data3 );
#endif
}


void debugMsg( const char *str1, float data1, float data2, float data3 )
{
#ifdef _DEBUG
	if (botNr != activeBot) return;
	ALERT ( at_console, (char*)str1, data1, data2, data3 );
#endif
}


void debugSound( edict_t *recipient, const char *sample )
{
#ifdef _DEBUG
	pfnEmitSound( recipient, CHAN_BODY, sample, 1.0, ATTN_NORM, 0, 100 );
#endif
}