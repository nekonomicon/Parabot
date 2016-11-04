#if !defined( PB_SECTORS_H )
#define PB_SECTORS_H


#include "pb_global.h"



class PB_Sectors
{

#define NUM_SECTORS 4


protected:

	int getSector( Vector dir );

};

#endif