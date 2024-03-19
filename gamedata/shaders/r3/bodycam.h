#include "common.h"

/*
shader_param_6.x = xx.xyyzz 
                   x.xx = bodycam_zoom (1.00)
                   yy.  = bodycam_circle_ratio_x (10)
                   zz.  = bodycam_circle_ratio_y (10)

shader_param_6.y = xx.xyyy
                   x.xx = bodycam_distortion (1.00)
                   y.yy = bodycam_distortion_cubic (1.00)

shader_param_6.z = x.xx
                   x.xx = bodycam_circle_radius (1.00)

shader_param_6.w = x.xx
                   x.xx = bodycam_circle_blur (1.00)
*/
uniform float4 shader_param_6;

#define float3_color_black float3 (0.0,0.0,0.0)

float2 get_center_uv()
{
  float2 result = (0.5);
  return result;
}

float3 bodycam(float3 image, float2 texcoord)
{
  //BodyCamParam
  float4 lua_param = shader_param_6;
  
  float bodycam_zoom             = 1.0 / (floor(lua_param.x * 10.0) * 0.01);
  float bodycam_circle_ratio_x   = floor(frac(lua_param.x * 10.0) * 100.0);
  float bodycam_circle_ratio_y   = frac(frac(lua_param.x * 10.0) * 100.0) * 100.0;
  float bodycam_distortion       = floor(lua_param.y * 10.0) * 0.01;
  float bodycam_distortion_cubic = frac(lua_param.y * 10.0) * 100.0;
  float bodycam_circle_radius    = lua_param.z;
  float bodycam_circle_blur      = lua_param.w;

  //Center
  float2 center = get_center_uv();
  float2 bodycam_uv = texcoord.xy;
  float2 uv_center = bodycam_uv - center;
  
  //BodyCamDistortion
  float r = (bodycam_distortion_cubic == 0.0) ? (bodycam_uv.y - center.y) : (length(bodycam_uv - center));
  float f = 1.0 + r * r * (bodycam_distortion + bodycam_distortion_cubic);

  bodycam_uv = (uv_center * 0.5) * f * bodycam_zoom + center;

  //IsOutDisplay?
  bool is_out_display = (bodycam_uv.x > 1.0 || bodycam_uv.x < 0.0) || (bodycam_uv.y > 1.0 || bodycam_uv.y < 0.0);
  
  //IsOutCircle?
  float2 circle_center = (texcoord.xy - center);
  circle_center.x /= (bodycam_circle_ratio_y / bodycam_circle_ratio_x);

  float circle_radius = length(circle_center);
  bool is_out_circle = circle_radius > bodycam_circle_radius;

  //Blur
  bool is_out_of_render = (is_out_display || is_out_circle);
  float blur = (circle_radius - bodycam_circle_radius + bodycam_circle_blur) / bodycam_circle_blur;
  blur = clamp(blur , 0.0 , 1.0);
  blur = blur * blur * blur * blur * blur;
  float blur_render_param = is_out_of_render ? 0.0 : 1.0 - blur;

  //Result
  float3 result = lerp(float3_color_black , s_image.SampleLevel(smp_rtlinear, bodycam_uv, 0.0).xyz , blur_render_param);

  return result;
}