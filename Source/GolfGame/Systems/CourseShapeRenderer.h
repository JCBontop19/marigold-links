// CourseShapeRenderer.h
// Draws every HoleCoverageZone with bDrawShape on into the shape-mask render target
// (R = fairway, G = green, B = rough). M_Turf reads that mask instead of the painted
// Fairway / Green landscape layers, so the edges are straight lines and half circles at
// the render target's resolution (about 15 cm per pixel) instead of the 1 m paint grid.
//
// It redraws on its own when a zone is loaded, moved, edited or deleted (editor and game).
// Console commands:  golf.RebuildShapes   golf.ShapeYawSign 1|-1

#pragma once

#include "CoreMinimal.h"

class UWorld;

class GOLFGAME_API FCourseShapeRenderer
{
public:
	// Redraw once, next frame. Many calls in one frame become a single redraw.
	static void RequestRebuild(UWorld* world);

	// Redraw right now.
	static void RebuildNow(UWorld* world);
};
