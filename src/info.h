
/** \file info.h
 *  \brief Header: panel managing
 */

#ifndef MC_INFO_H
#define MC_INFO_H

struct WInfo;
typedef struct WInfo WInfo;

WInfo *info_new (int y, int x, int lines, int cols);

#endif /* MC_INFO_H */
