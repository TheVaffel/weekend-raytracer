main: main.cpp
	g++ -o $@ $< -I ../HConLib/include -L ../HConLib/lib -lFlatAlg  -l png -lpthread -O3 -std=c++1z -Wall -I . 
