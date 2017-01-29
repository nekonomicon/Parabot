/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

typedef int BOOL;
#define TRUE	1	
#define FALSE	0

// hack into header files that we can ship
typedef int qboolean;
typedef unsigned char byte;
#include "mathlib.h"
#include "const.h"
#include "edict.h"
#include "eiface.h"

#include "studio.h"

#ifndef ACTIVITY_H
#include "activity.h"
#endif

#ifndef ANIMATION_H
#include "animation.h"
#endif

#ifndef ENGINECALLBACK_H
#include "enginecallback.h"
#endif

#pragma warning( disable : 4244 )
int LookupActivity( void *pmodel, entvars_t *pev, int activity )
{
	studiohdr_t *pstudiohdr;

	pstudiohdr = (studiohdr_t *)pmodel;
	if( !pstudiohdr )
		return 0;

	mstudioseqdesc_t *pseqdesc;

	pseqdesc = (mstudioseqdesc_t *)( (byte *)pstudiohdr + pstudiohdr->seqindex );

	int weighttotal = 0;
	int seq = ACTIVITY_NOT_AVAILABLE;
	for( int i = 0; i < pstudiohdr->numseq; i++ )
	{
		if( pseqdesc[i].activity == activity )
		{
			weighttotal += pseqdesc[i].actweight;
			if( !weighttotal || RANDOM_LONG( 0, weighttotal - 1 ) < pseqdesc[i].actweight )
				seq = i;
		}
	}

	return seq;
}

