// PB_VisTable.h: interface for the PB_VisTable class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(PB_VISTABLE_H)
#define PB_VISTABLE_H
#include <stdio.h>

#define MAX_CELLS	8192

typedef struct vistable {
	int	*cells[MAX_CELLS];
	int	 numcells;
	int	 tracecell, tracebit;
} VISTABLE;

void	 vistable_clear(VISTABLE *vistable);
void	 vistable_load(VISTABLE *vistable, FILE *fp);
void	 vistable_save(VISTABLE *vistable, FILE *fp);

void	 vistable_addcell(VISTABLE *vistable);

// returns true and two cells that still have to be traced or false
bool	 vistable_needtrace(VISTABLE *vistable, int &cell1, int &cell2);

// sets the result for the next two cells (as returned by needTrace)
void	 vistable_addtrace(VISTABLE *vistable, bool visible);

bool	 vistable_isvisible(VISTABLE *vistable, int cell1, int cell2);

#endif 
