#pragma once
#include <vector>
#include <string>
#include <fstream>


/**
 * Format specification:
 * http://www.neuronland.org/NLMorphologyConverter/MorphologyFormats/SWC/Spec.html
 */
struct neuron{
	std::int64_t sample;
	std::int64_t parent;
	double   x;
	double   y;
	double   z;
	double   radius;
    std::int8_t  type;
};

/**
 *
 * @param filename
 * @return
 */
std::vector<neuron*> parseFile(std::string filename);

/**
 *
 * @param input
 * @return
 */
neuron* getNeuron(std::istream& input);
