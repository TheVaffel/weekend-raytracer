#ifndef INCLUDE_UTILS_HPP
#define INCLUDE_UTILS_HPP

#include <random>
#include <FlatAlg.hpp>

struct unidist {
  
  unidist(float a = 0.0, float b = 1.0);

  float get();
  
  std::default_random_engine gen;
  std::uniform_real_distribution<float> dist;
  
};

falg::Vec3 random_in_unit_sphere(unidist& dist);


#endif // INCLUDE_UTILS_HPP
