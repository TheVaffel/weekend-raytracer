#ifndef INCLUDE_UTILS_HPP
#define INCLUDE_UTILS_HPP


struct unidist {
  
  unidist(float a = 0.0, float b = 1.0) : gen(), dist(a, b) {}
  
  float get() {
    return (this->dist)(this->gen);
  }
  
  std::default_random_engine gen;
  std::uniform_real_distribution<float> dist;
  
};

vec3 random_in_unit_sphere(unidist& dist) {
  vec3 p;
  do {
    p = 2.0 * vec3(dist.get(), dist.get(), dist.get()) - vec3(1, 1, 1);
  } while (p.sqNorm() >= 1.0);
  return p;
}


#endif // INCLUDE_UTILS_HPP
