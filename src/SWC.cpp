#include "SWC.hpp"
#include <regex>
#include <sstream>
#include <iostream>
#include <cmath>

/*
struct compartment{
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


neuron parseSWC(const std::string& filename){
	std::ifstream file;
    file.open(filename);

    if(!file){
        std::cerr << "Error: File " << filename << " not found." << std::endl;
        exit(1);
    }

	std::vector<compartment*> output;

	while(file){
        compartment* n = getCompartment(file);
		if(n != nullptr){
			output.push_back(n);
		}
	}

	return output;
}

compartment* getCompartment(std::istream& input){
	std::string line;
	do{
		getline(input, line);
        line = std::regex_replace(line, regex_comment, "");
        line = std::regex_replace(line, regex_trimming, "");
		if(input.eof()) return nullptr;
	}while(line.size() == 0);

	std::stringstream buff(line);
    compartment* res = new compartment();

	buff >> res->sample;
	buff >> res->type;
	buff >> res->x;
	buff >> res->y;
	buff >> res->z;
	buff >> res->radius;
	buff >> res->parent;

	return res;
}


std::vector<neuron> parseRPL(const std::string& filename, const neuron& n){
    std::ifstream file;
    file.open(filename);

    if(!file){
        std::cerr << "Error: File " << filename << " not found." << std::endl;
        exit(1);
    }

    std::vector<neuron> output;
    std::string line;
    getline(file, line);

    while(file){
        std::stringstream buff(line);
        neuron res = neuron();

        // Deep copy of the neuron
        for(compartment* c : n){
            res.push_back(new compartment(*c));
        }

        // Read the operations
        while(buff){
            std::int8_t op, ax;
            double val;

            buff >> op;
            buff >> ax;
            buff >> val;


            if(op == 0){ //Translation
                for(compartment* c : res){
                    if(ax == 0){
                        c->x += val;
                    }else if(ax == 1){
                        c->y += val;
                    }else if(ax == 2){
                        c->z += val;
                    }
                }
            }else if(op == 1){ // Rotation
                double cosVal = cos(val), sinVal = sin(val);
                for(compartment* c : res){
                    if(ax == 0) { // Rotation axis is x
                        c->y = c->y * cosVal - c->z * sinVal;
                        c->z = c->y * sinVal + c->z * cosVal;
                    } else if( ax == 1){ // Rotation axis is y
                        c->x = c->x * cosVal - c->z * sinVal;
                        c->z = c->x * sinVal + c->z * cosVal;
                    } else if( ax == 2){ // Rotation axis is z
                        c->x = c->x * cosVal - c->y * sinVal;
                        c->y = c->x * sinVal + c->y * cosVal;
                    }
                }
            }
        }


        getline(file, line);
    }

    return output;
}
