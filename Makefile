HEADERS = hitablelist.hpp aabb.hpp camera.hpp hitable.hpp material.hpp ray.hpp sphere.hpp utils.hpp texture.hpp perlin.hpp transforms.hpp volume.hpp triangles.hpp

SOURCES = main.cpp triangles.cpp aabb.cpp utils.cpp

main: $(SOURCES) $(HEADERS)
	g++ -o $@ $(SOURCES) -I ../HConLib/include -L ../HConLib/lib -lFlatAlg -lpthread -Wall -I . -lOpenImageIO -std=c++17 -lpng -O3 -mavx
