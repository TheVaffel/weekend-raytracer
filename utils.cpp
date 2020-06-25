#include "utils.hpp"

unidist::unidist(float a, float b) : gen(), dist(a, b) {}

float unidist::get() {
  return (this->dist)(this->gen);
}

falg::Vec3 random_in_unit_sphere(unidist& dist) {
  falg::Vec3 p;
  do {
    p = 2.0 * falg::Vec3(dist.get(), dist.get(), dist.get()) - falg::Vec3(1, 1, 1);
  } while (p.sqNorm() >= 1.0);
  return p;
}
