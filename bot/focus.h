#pragma once
#if !defined(PB_FOCUS_H)
#define PB_FOCUS_H
void focus_adddir(Vec3D *dir, SECTOR *sectors);
float focus_fordir(Vec3D *dir, SECTOR *sectors);
int focus_cellsfordir(Vec3D *dir, SECTOR *sectors);

#endif
