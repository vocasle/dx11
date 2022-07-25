#pragma once

#include "Math.h"

struct Color
{
  Color () : R{ 0 }, G{ 0 }, B{ 0 }, A{ 0 } {}
  Color (float r, float g, float b, float a) : R{ r }, G{ g }, B{ b }, A{ a }
  {
  }
  float R;
  float G;
  float B;
  float A;
};

inline Color
ColorFromRGBA (const float r, const float g, const float b, const float a)
{
  Color color = { r, g, b, a };
  return color;
}

struct DirectionalLight
{
  DirectionalLight () : Ambient{}, Diffuse{}, Specular{}, Direction{}, pad{ 0 }
  {
  }
  Color Ambient;
  Color Diffuse;
  Color Specular;
  Vec3D Direction;
  float pad;
};

struct PointLight
{
  PointLight ()
      : Ambient{}, Diffuse{}, Specular{}, Position{}, Range{ 0 }, Att{}, pad{
          0
        }
  {
  }
  Color Ambient;
  Color Diffuse;
  Color Specular;
  Vec3D Position;
  float Range;
  Vec3D Att;
  float pad;
};

struct SpotLight
{
  SpotLight ()
      : Ambient{}, Diffuse{}, Specular{}, Position{}, Range{ 0 }, Direction{},
        Spot{ 0 }, Att{}, pad{ 0 }
  {
  }
  Color Ambient;
  Color Diffuse;
  Color Specular;
  Vec3D Position;
  float Range;
  Vec3D Direction;
  float Spot;
  Vec3D Att;
  float pad;
};

struct Material
{
  Material () : Ambient{}, Diffuse{}, Specular{} {}
  Material (Color ambient, Color diffuse, Color specular)
      : Ambient (ambient), Diffuse (diffuse), Specular (specular)
  {
  }
  Color Ambient;
  Color Diffuse;
  Color Specular;
};
