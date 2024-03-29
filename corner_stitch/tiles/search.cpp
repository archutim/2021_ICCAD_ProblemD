/*
 * search.c --
 *
 * Point searching.
 *
 *     ********************************************************************* 
 *     * Copyright (C) 1985, 1990 Regents of the University of California. * 
 *     * Permission to use, copy, modify, and distribute this              * 
 *     * software and its documentation for any purpose and without        * 
 *     * fee is hereby granted, provided that the above copyright          * 
 *     * notice appear in all copies.  The University of California        * 
 *     * makes no representations about the suitability of this            * 
 *     * software for any purpose.  It is provided "as is" without         * 
 *     * express or implied warranty.  Export of this software outside     * 
 *     * of the United States of America may require an export license.    * 
 *     *********************************************************************
 */

#include <iostream>
#include "../utils/magic.h"
#include "tile.h"

/*
 * --------------------------------------------------------------------
 *
 * TiSrPoint --
 *
 * Search for a point.
 *
 * Results:
 *	A pointer to the tile containing the point.
 *	The bottom and left edge of a tile are considered part of
 *	the tile; the top and right edge are not.
 *
 * Side effects:
 *	Updates the hint tile in the supplied plane to point
 *	to the tile found.
 *
 * --------------------------------------------------------------------
 */

Tile* TiSrPoint(Tile* hintTile, Plane* plane, Point* point){
    Tile *tp = (hintTile) ? hintTile : plane->pl_hint;

    GOTOPOINT(tp, point);
    plane->pl_hint = tp;
    return(tp);
}
