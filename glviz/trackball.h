#pragma once
#include <Eigen/Dense>

class Trackball {
public:
  Eigen::Quaternionf const& operator()(float u0_x, float u0_y, float u1_x, float u1_y);

protected:
  float project_to_sphere (float r, float x, float y) const;

  Eigen::Quaternionf rotation;
  };
