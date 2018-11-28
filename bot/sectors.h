#pragma once
#if !defined(SECTORS_H)
#define SECTORS_H

#define NUM_SECTORS	4

typedef struct sector {
	float focusvalue;
	short kills, cells;
} SECTOR;

int	getsector(Vec3D *dir);
#endif // SECTORS_H
