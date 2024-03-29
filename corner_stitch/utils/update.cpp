/*
 * update.c --
 *
 * tile manipulation for updating tile plane structures
 *
 *     ************************************************************************ 
 *     * Copyright (C) 2015 lionking, National Chiao Tung University, Taiwan. * 
 *     * Permission to use, copy, modify, and distribute this                 * 
 *     * software and its documentation for any purpose and without           * 
 *     * fee is hereby granted, provided that the above copyright             * 
 *     * notice appear in all copies.                                         * 
 *     ************************************************************************
 *
 */

#include "update.h"

static int checkTileType(Tile *tile, ClientData cdata)
{
	bool* is_all_space = (bool*)cdata;
	if (TiGetBody(tile) == SOLID_TILE) { *is_all_space = false; }
	return 0;
}


static bool checkValidInsertion(Rect *rect, Plane *plane)
{
	bool is_all_space = true;
	TiSrArea(nullptr, plane, rect, checkTileType, (ClientData)(&is_all_space));
	return is_all_space;
}


static bool canMergeVertical(const Tile *tile1, const Tile *tile2)
{
	if (TiGetBody(tile1) == SOLID_TILE) { return false; }
	if (TiGetBody(tile2) == SOLID_TILE) { return false; }
	if (LEFT(tile1) != LEFT(tile2)) { return false; }
	if (RIGHT(tile1) != RIGHT(tile2)) { return false; }
	return false;
}


static bool canMergeHorizontal(const Tile *tile1, const Tile *tile2)
{
	if (TiGetBody(tile1) == SOLID_TILE) { return false; }
	if (TiGetBody(tile2) == SOLID_TILE) { return false; }
	if (TOP(tile1) == TOP(tile2)) { return false; }
	if (BOTTOM(tile1) == BOTTOM(tile2)) { return false; }
	return true;
}


Tile* CreateTile(int x, int y)
{
	Tile *tile = TiAlloc();
	// specified lower-left corner
	LEFT(tile) = x;
	BOTTOM(tile) = y;
	// all the stitches will point to nullptr
	RT(tile) = nullptr;
	TR(tile) = nullptr;
	LB(tile) = nullptr;
	BL(tile) = nullptr;

	return tile;
}

 
Tile* InsertTile(Rect *rect, Plane *plane, int type, int id)
{
	// enumerate all the tiles which intersect with the given tiles
	// Then split those tiles.
	
	// Firstly, search the bottom-left corner of this tile
	Tile* tp = TiSrPoint(nullptr, plane, &(rect->r_ll));
	
	// check all the tiles overlapping with the desired region are all space tiles
	if (checkValidInsertion(rect, plane) == false) { return nullptr; }
	
	// If bottom of tp is less than the bottom of tile, we should split tp at the bottom of tile
	if ( BOTTOM(tp) < rect->r_ll.p_y ) {
		tp = TiSplitY( tp, rect->r_ll.p_y );
	}
	
	/* check whether a tile can merge with its bottom tile */
	#define DOWN_MERGE(tile)	\
	{	\
		Tile *btp = LB(tile);	\
		if ( canMergeVertical(tile, btp) ) { TiJoinY(tile, btp, plane); }	\
	}
	
	#define SPLIT_AND_MERGE(tile)	\
	{	\
		if ( LEFT(tile) < rect->r_ll.p_x ) { \
			Tile *old_tile = tile;	\
			tile = TiSplitX( tile, rect->r_ll.p_x );	\
			DOWN_MERGE(old_tile)	\
		}	\
		if ( RIGHT(tile) > rect->r_ur.p_x ) {	\
			Tile *new_tile = TiSplitX( tile, rect->r_ur.p_x );	\
			DOWN_MERGE(new_tile)	\
		}	\
		if (target != nullptr) {	\
			TiJoinY(tile, target, plane); \
		}	\
		target = tile;	\
	}
	
	// split tiles 
	Tile* target = nullptr;
	while ( TOP(tp) <= rect->r_ur.p_y ) {
		SPLIT_AND_MERGE(tp)
		tp = RT(tp);
	}
	
	// reach top.
	// If the top of tp is larger than the top of tile,
	// then split tp at the top of tile.
	if ( (BOTTOM(tp) < rect->r_ur.p_y) && (TOP(tp) > rect->r_ur.p_y) ) {
		TiSplitY( tp, rect->r_ur.p_y );
		SPLIT_AND_MERGE(tp)
	}
	
	#undef DOWN_MERGE
	#undef SPLIT_AND_MERGE
	
	TiSetBody(target, type);
	if(type == SOLID_TILE){
		TiSetClient(target, id);
	}
	return target;
}


void RemoveTile(Tile *tile, Plane *plane)
{
	// special case: target tile is a space tile
	if (TiGetBody(tile) != SOLID_TILE) { return ; }
	const Point hint_ll = tile->ti_ll;
	
	// Firstly, search the dead tile in the tile plane
	tile = TiSrPoint(nullptr, plane, &(tile->ti_ll));
	const double del_ytop = TOP(tile);
	const double del_ybot = BOTTOM(tile);
	
	// change the type of dead tile into space tile (initial value)
	TiSetBody(tile, 0);
	
	Tile *right_start = TR(tile);
	Tile *left_start = BL(tile);

	// special case: the top of right_start is higher than del_ytop and the type of tp is a space tile.
	// We should first split right_start so that right_start will share the common top edge with the dead tile. 
	// Note: this is possible when there is another solid tile right above the dead tile with common right edge.
	if ( (TiGetBody(right_start) != SOLID_TILE) && (TOP(right_start) > del_ytop) ) {
		TiSplitY(right_start, del_ytop);
	}
	
	// go through all tiles that adjoin the right edge of the dead tile in the order of top to bottom
	Tile *tp = nullptr;
	for (tp = right_start; BOTTOM(tp) >= del_ybot; ) {
		// store the tile that will be processed in next iteration
		// since the corner stitch structure will be modified
		// and we may not be able to find this tile through tp after this iteration.
		Tile *next = LB(tp);
		
		Tile *tmp = tile;
		if (BOTTOM(tp) > del_ybot) { tmp = TiSplitY(tile, BOTTOM(tp)); }
		if (TiGetBody(tp) == SOLID_TILE) { tp = next; continue; }
		else { TiJoinX(tp, tmp, plane); }
		
		// restore tile
		tp = next;
	}

	// special case: the top of tp is higher than del_ybot and the type of tp is a space tile.
	// We should also split tp so that tp will share the common bottom edge with the dead tile.
	// Note: this is possible when there is another solid tile right below the dead tile with common right edge.
	if ( (TiGetBody(tp) != SOLID_TILE) && (TOP(tp) > del_ybot) ) {
		tp = TiSplitY(tp, del_ybot);
		TiJoinX(tp, tile, plane);
	}
	
	// special case: 
	if ((TiGetBody(BL(RT(tp))) == SOLID_TILE) && canMergeVertical(tp, RT(tp)) ) { TiJoinY(tp, RT(tp), plane); }

	// special case: the bottom of left_start is lower than del_ybot and the type of left_start is a space tile.
	// We should first split left_start so that left_start will share the common bottom edge with the dead tile.
	// Note: this is possible when there is another solid tile right below the dead tile with common left edge.
	if ( (TiGetBody(left_start) != SOLID_TILE) && (BOTTOM(left_start) < del_ybot) ) {
		left_start = TiSplitY(left_start, del_ybot);
	}
	
	// go through all tiles that adjoin the left edge of the dead tile in the order of bottom to top
	for (tp = left_start; TOP(tp) <= del_ytop; ) {
		// store the tile that will be processed in next iteration
		// since the corner stitch structure will be modified
		// and we may not be able to find this tile through tp after this iteration.
		Tile *next = RT(tp);
		
		Tile *tmp = TR(tp);
		if (TOP(tp) < TOP(tmp)) { TiSplitY(tmp, TOP(tp)); }
		if (TiGetBody(tp) == SOLID_TILE) { tp = next; continue; }
		
		for (tmp = TR(tp); BOTTOM(tp) < BOTTOM(tmp); tmp = LB(tmp)) {
			TiSplitY(tp, BOTTOM(tmp));
		}
		next = RT(tp);
		TiJoinX(tp, tmp, plane);
		if ( canMergeVertical(tp, LB(tp)) ) { TiJoinY(tp, LB(tp), plane); }
		
		// restore tile
		tp = next;
	}

	// special case: the top of tp is higher than del_ytop and the type of tp is a space tile.
	// We should also split tp so that tp will share the common top edge with the dead tile.
	// Note: this is possible when there is another solid tile right above the dead tile with common left edge.
	if ( (TiGetBody(tp) != SOLID_TILE) && (BOTTOM(tp) > del_ytop) ) {
		Tile *next = TiSplitY(tp, del_ytop);
		TiJoinX(tp, TR(tp), plane);
		tp = next;
	}
	
	if ( canMergeVertical(tp, LB(tp)) ) { TiJoinY(tp, LB(tp), plane); }
	
	// reset hint tile
	plane->pl_hint = TiSrPoint(tp, plane, (Point*)&hint_ll);
}


Plane* CreateTilePlane()
{
	/*
	 ************************************************************************
	 * Important!!
	 ************************************************************************
	 * The coordinates of the initial tile cannot be MINFINITY and INFINITY
	 *  due to the search strategy of TiSrArea.
	 * It will cause core dump once we do so.
	 ************************************************************************
	 */
	Tile *init_tile = CreateTile(MINFINITY + 1, MINFINITY + 1);
	Plane *plane = TiNewPlane(init_tile);
	return plane;
}


static int deleteTile (Tile *tile, ClientData cdata)
{
	Plane *plane = (Plane*)cdata;
	if (TiGetBody(tile) == SOLID_TILE) { RemoveTile(tile, plane); }
	return 0;
}


void RemoveTilePlane(Plane *plane)
{
	Rect rect = { {MINFINITY + 1, MINFINITY + 1}, {INFINITY - 1, INFINITY - 1} };
	//Rect rect = { {0, 0}, {600, 400} };
	TiSrArea(nullptr, plane, &rect, deleteTile, (ClientData)plane);
	// TiFree(plane->pl_hint);
	TiFreePlane(plane);
}