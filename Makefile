main: main.cpp hitablelist.hpp aabb.hpp camera.hpp hitable.hpp material.hpp ray.hpp sphere.hpp utils.hpp texture.hpp perlin.hpp transforms.hpp volume.hpp
	g++ -o $@ $< -I ../HConLib/include -L ../HConLib/lib -lFlatAlg -lpthread -O3 -Wall -I . -mavx -lOpenImageIO
