#pragma once

#include "CoreMinimal.h"
#include "box2d/box2d.h"

// UE 5.8 FVector2D uses double, b2Vec2 uses float
static inline b2Vec2 ToB2(const FVector2D& V) { return { (float)V.X, (float)V.Y }; }
static inline FVector2D FromB2(b2Vec2 V) { return FVector2D(V.x, V.y); }
static inline b2Pos ToB2Pos(const FVector2D& V) { return { (float)V.X, (float)V.Y }; }
