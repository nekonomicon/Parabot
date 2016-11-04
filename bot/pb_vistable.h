// PB_VisTable.h: interface for the PB_VisTable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined( PB_VISTABLE_H )
#define PB_VISTABLE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <stdio.h>



class PB_VisTable  
{

#define MAX_CELLS 8192


public:

	PB_VisTable();
	virtual ~PB_VisTable();
	void clear();
	void load( FILE *fp );
	void save( FILE *fp );

	void addCell();

	// returns true and two cells that still have to be traced or false
	bool needTrace( int &cell1, int &cell2 );	
	
	// sets the result for the next two cells (as returned by needTrace)
	void addTrace( bool visible );
	
	bool isVisible( int cell1, int cell2 );
	

private:

	int *visTable[MAX_CELLS];
	int bitMask[32];	// stores the bitmask for fast anding
	int numCells;
	int traceCell, traceBit;

	int* getMem( int numBits );
	void setVisibility( int cell1, int cell2, bool visible );

};

#endif 
