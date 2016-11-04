#if !defined( MARKER_H )
#define MARKER_H

#pragma warning( disable : 4786 )	// disable warnings

#define MAX_MARKER 128


#include "pb_global.h"
#include <map>


typedef struct {
	Vector	pos;
	int		type;
} tWPMark;

class CMarker
{

public:

	CMarker();

	int newMarker( Vector pos, int type );
	// adds a new marker at pos, returns id

	bool setPos( int id, Vector pos );
	// sets position of marker id

	bool setType( int id, int type );
	// sets type of marker id

	bool deleteMarker( int id );
	// deletes marker id

	void deleteAll();
	// deletes all markers

	void drawMarkers();
	// must be called each frame to draw markers


private:

	typedef std::map<int, tWPMark> tMarker;
	typedef tMarker::value_type mPair;
	
	int markerId;		// idNumber
	tMarker marker;

};

#endif