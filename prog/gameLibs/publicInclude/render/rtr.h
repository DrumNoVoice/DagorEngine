//
// Dagor Engine 6.5 - Game Libraries
// Copyright (C) Gaijin Games KFT.  All rights reserved.
//
#pragma once

#include <drv/3d/dag_resId.h>

class Point2;
class Point3;
class TMatrix;
class TMatrix4;
class BaseTexture;
typedef BaseTexture Texture;

namespace bvh
{
struct Context;
using ContextId = Context *;
} // namespace bvh

namespace rtr
{

void initialize(bool half_res, bool checkerboard_);
void teardown();

void set_performance_mode(bool performance_mode);

void set_water_params();
void set_rtr_hit_distance_params();

void render(bvh::ContextId context_id, const TMatrix4 &proj_tm, bool rt_shadow, bool csm_shadow, bool high_speed_mode,
  TEXTUREID half_depth);

void render_validation_layer();

void turn_off();

} // namespace rtr