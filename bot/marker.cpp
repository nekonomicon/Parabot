#include "marker.h"

extern int wpSpriteTexture;
extern int wpSprite2Texture;
extern int wpBeamTexture;
extern edict_t *playerEnt;

void WaypointDrawBeam(edict_t *pEntity, Vector start, Vector end, int width,
        int noise, int red, int green, int blue, int brightness, int speed)
{
	static int frameStart = 0;
	static int frameRate = 40;
	static int life = 1;

   MESSAGE_BEGIN(MSG_ONE, SVC_TEMPENTITY, NULL, pEntity);
   WRITE_BYTE( TE_BEAMPOINTS);
   WRITE_COORD(start.x);
   WRITE_COORD(start.y);
   WRITE_COORD(start.z);
   WRITE_COORD(end.x);
   WRITE_COORD(end.y);
   WRITE_COORD(end.z);
   WRITE_SHORT( wpBeamTexture );
   WRITE_BYTE( frameStart ); // framestart
   WRITE_BYTE( frameRate ); // framerate
   //WRITE_BYTE( 10 ); // life in 0.1's
   WRITE_BYTE( life ); // life in 0.1's
   WRITE_BYTE( width ); // width
   WRITE_BYTE( noise );  // noise

   WRITE_BYTE( red );   // r, g, b
   WRITE_BYTE( green );   // r, g, b
   WRITE_BYTE( blue );   // r, g, b

   WRITE_BYTE( brightness );   // brightness
   WRITE_BYTE( speed );    // speed
   MESSAGE_END();
}


void drawCube( edict_t *ent, Vector pos, float size )
{
	int red = 0;
	int green = 0;
	int blue = 255;
	// upper sqare
	WaypointDrawBeam( ent, pos+Vector( size, size, size), pos+Vector( size,-size, size), 20, 0, red, green, blue, 250, 5 );
	WaypointDrawBeam( ent, pos+Vector( size,-size, size), pos+Vector(-size,-size, size), 20, 0, red, green, blue, 250, 5 );
	WaypointDrawBeam( ent, pos+Vector(-size,-size, size), pos+Vector(-size, size, size), 20, 0, red, green, blue, 250, 5 );
	WaypointDrawBeam( ent, pos+Vector(-size, size, size), pos+Vector( size, size, size), 20, 0, red, green, blue, 250, 5 );
	// lower sqare
	WaypointDrawBeam( ent, pos+Vector( size, size,-size), pos+Vector( size,-size,-size), 20, 0, red, green, blue, 250, 5 );
	WaypointDrawBeam( ent, pos+Vector( size,-size,-size), pos+Vector(-size,-size,-size), 20, 0, red, green, blue, 250, 5 );
	WaypointDrawBeam( ent, pos+Vector(-size,-size,-size), pos+Vector(-size, size,-size), 20, 0, red, green, blue, 250, 5 );
	WaypointDrawBeam( ent, pos+Vector(-size, size,-size), pos+Vector( size, size,-size), 20, 0, red, green, blue, 250, 5 );
	// vertical lines
	WaypointDrawBeam( ent, pos+Vector( size, size, size), pos+Vector( size, size,-size), 20, 0, red, green, blue, 250, 5 );
	WaypointDrawBeam( ent, pos+Vector( size,-size, size), pos+Vector( size,-size,-size), 20, 0, red, green, blue, 250, 5 );
	WaypointDrawBeam( ent, pos+Vector(-size,-size, size), pos+Vector(-size,-size,-size), 20, 0, red, green, blue, 250, 5 );
	WaypointDrawBeam( ent, pos+Vector(-size, size, size), pos+Vector(-size, size,-size), 20, 0, red, green, blue, 250, 5 );
}


void drawParticles( edict_t *pEntity, Vector start, int color )
{
   MESSAGE_BEGIN(MSG_ONE, SVC_TEMPENTITY, NULL, pEntity);
   WRITE_BYTE( TE_PARTICLEBURST);
   
   WRITE_COORD(start.x);
   WRITE_COORD(start.y);
   WRITE_COORD(start.z);
   
   WRITE_SHORT( 10 );

   WRITE_BYTE( color );   // color
   
   WRITE_BYTE( 30 );	// duration
   
   MESSAGE_END();
}


void drawMarker( edict_t *pEntity, Vector start, Vector end )
{
   MESSAGE_BEGIN(MSG_ONE, SVC_TEMPENTITY, NULL, pEntity);
   WRITE_BYTE( TE_BEAMTORUS);
   
   WRITE_COORD(start.x);
   WRITE_COORD(start.y);
   WRITE_COORD(start.z);
   WRITE_COORD(end.x);
   WRITE_COORD(end.y);
   WRITE_COORD(end.z);
   
   WRITE_SHORT( wpBeamTexture );
   WRITE_BYTE( 1 ); // framestart
   WRITE_BYTE( 10 ); // framerate
   WRITE_BYTE( 30 ); // life in 0.1's
   WRITE_BYTE( 10 ); // width
   WRITE_BYTE( 0 );  // noise

   WRITE_BYTE( 0 );   // r, g, b
   WRITE_BYTE( 0 );   // r, g, b
   WRITE_BYTE( 255 );   // r, g, b

   WRITE_BYTE( 250 );   // brightness
   WRITE_BYTE(5 );    // speed
   
   MESSAGE_END();
}


void drawSprite( edict_t *pEntity, Vector start, int type )
{
   MESSAGE_BEGIN(MSG_ONE, SVC_TEMPENTITY, NULL, pEntity);
   WRITE_BYTE( TE_SPRITE);
   
   WRITE_COORD(start.x);
   WRITE_COORD(start.y);
   WRITE_COORD(start.z);
   
   if (type==1) {
	   WRITE_SHORT( wpSpriteTexture );
	   WRITE_BYTE( 2 );   // scale
   }
   else {
	   WRITE_SHORT( wpSprite2Texture );
	   WRITE_BYTE( 4 );   // scale
   }
   
   WRITE_BYTE( 128 );	// brightness
   
   MESSAGE_END();
}


CMarker::CMarker()
{
	markerId = 0;
}


int CMarker::newMarker( Vector pos, int type )
// adds a new marker at pos, returns id
{
	markerId++;
	tWPMark m;	m.pos = pos;	m.type = type;
	marker.insert( mPair(markerId, m) );
	//if (playerEnt) drawSprite( playerEnt, pos+Vector(0,0,50) );
		//drawParticles( playerEnt, pos, color );
		//drawMarker( playerEnt, pos, pos+Vector(0,0,30), color );
	//	WaypointDrawBeam( playerEnt, pos, pos+Vector(0,0,30), 20, 0, 0, 0, 255, 250, 5 );
	//else debugMsg( "Marker: no playerEnt!\n" );
	return markerId;
}


bool CMarker::setPos( int id, Vector pos )
// sets position of marker id
{
	tMarker::iterator m = marker.find( id );
	if (m != marker.end()) {
		m->second.pos = pos;
		return true;
	}
	else return false;
}


bool CMarker::setType( int id, int type )
// sets position of marker id
{
	tMarker::iterator m = marker.find( id );
	if (m != marker.end()) {
		m->second.type = type;
		return true;
	}
	else return false;
}

bool CMarker::deleteMarker( int id )
// deletes marker id
{
	tMarker::iterator m = marker.find( id );
	if (m != marker.end()) {
		marker.erase( m );
		return true;
	}
	else return false;
}


void CMarker::deleteAll()
// deletes all markers
{
	marker.clear();
}


void CMarker::drawMarkers()
{
	//return;
	if (!playerEnt) return;

	tMarker::iterator m = marker.begin();
	while (m != marker.end()) {
		drawSprite( playerEnt, m->second.pos, m->second.type );
		m++;
	}
}