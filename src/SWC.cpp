#include "SWC.hpp"
#include <regex>
#include <sstream>
#include <iostream>

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

const std::regex regex_comment("#.*");
const std::regex regex_trimming("(^\\s+)|(\\s+$)");


std::vector<neuron> parseFile(std::string filename){
	std::ifstream file;
    file.open(filename);

    if(!file){
        std::cerr << "Error: File " << filename << " not found." << std::endl;
        exit(1);
    }

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
        line = std::regex_replace(line, regex_comment, "");
        line = std::regex_replace(line, regex_trimming, "");
		if(input.eof()) return nullptr;
	}while(line.size() == 0);

	std::stringstream buff(line);
	neuron* res = new neuron();

	buff >> res->sample;
	buff >> res->type;
	buff >> res->x;
	buff >> res->y;
	buff >> res->z;
	buff >> res->radius;
	buff >> res->parent;

	return res;
}
