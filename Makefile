main: main.cpp hitablelist.hpp aabb.hpp camera.hpp hitable.hpp material.hpp ray.hpp sphere.hpp utils.hpp
	g++ -o $@ $< -I ../HConLib/include -L ../HConLib/lib -lFlatAlg  -l png -lpthread -O3 -std=c++1z -Wall -I . 
