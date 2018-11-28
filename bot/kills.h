#pragma once
#if !defined(PB_KILLS_H)
#define PB_KILLS_H

void kills_adddir(Vec3D *dir, SECTOR *sectors);
short kills_fordir(Vec3D *dir, SECTOR *sectors);
#endif
