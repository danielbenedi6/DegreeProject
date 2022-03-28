#pragma once
#include <vector>
#include <string>
#include <fstream>


/**
 * Format specification:
 * http://www.neuronland.org/NLMorphologyConverter/MorphologyFormats/SWC/Spec.html
 */
struct neuron{
	std::uint64_t sample;
	std::uint64_t parent;
	std::uint8_t  type;
	double   x;
	double   y;
	double   z;
	double   radius;
};

std::vector<neuron> parseFile(std::string filename);

neuron* getNeuron(std::istream& input);
