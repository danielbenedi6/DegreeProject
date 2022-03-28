#include "SWC.hpp"
#include <sstream>

/*
struct neuron{
	std::uint64_t sample;
	std::uint64_t parent;
	double   x;
	double   y;
	double   z;
	double   radius;
	std::uint8_t  type;
}
*/

std::vector<neuron> parseFile(std::string filename){
	std::ifstream file(filename);
	std::vector<neuron> output;

	while(!file.eof()){
		neuron* n = getNeuron(file);
		if(n != nullptr){
			output.push_back(*n);
		}
	}

	return output;
}

neuron* getNeuron(std::istream& input){
	std::string line;	
	do{
		getline(input, line);
		if(input.eof()) return nullptr;
	}while(line.at(0) == '#');

	std::stringstream buff(line);
	neuron* res = new neuron();

	buff >> res->sample;
	buff >> res->type;
	buff >> res->x;
	buff >> res->y;
	buff >> res->z;
	buff >> res->radius;
	buff >> res->parent;

	if(buff.eof()){
		delete res;
		return nullptr;
	}

	return res;
}
