#pragma once
#include <vector>
#include <string>
#include <fstream>


/**
 * Format specification:
 * http://www.neuronland.org/NLMorphologyConverter/MorphologyFormats/SWC/Spec.html
 */
struct compartment{
	std::int64_t sample;
	std::int64_t parent;
	double   x;
	double   y;
	double   z;
	double   radius;
    std::int8_t  type;
};

using neuron = std::vector<compartment*>;

/** It gets a file name and returns the neuron contained in swc format.
 *
 * The swc file follows the standard format that it is:
 * In each line there are 7 values separated by spaces representing
 * a compartment of the neuron. The first value is the sample number,
 * the second value is the type, the third value is the x coordinate,
 * the fourth value is the y coordinate, the fifth value is the z coordinate,
 * the sixth value is the radius and the seventh value is the parent sample number.
 * @param filename The name of the file to be read.
 * @return A neuron structure containing the information of the neuron.
 */
neuron parseSWC(const std::string& filename);

/** Reads one compartment from the input stream.
 *
 * There are 7 values separated by spaces representing a compartment of the
 * neuron. The first value is the sample number, the second value is the type,
 * the third value is the x coordinate, the fourth value is the y coordinate,
 * the fifth value is the z coordinate, the sixth value is the radius and the
 * seventh value is the parent sample number.
 * @param input The input stream to be read.
 * @return
 */
compartment* getCompartment(std::istream& input);

/** Replicates the given neuron according with the input file.
 *
 * The replication file follow the following format:
 * Each line represents a new neuron and contains the operations applied to
 * the input neuron. The operations are applied to all the compartments of the
 * input neuron. One operation is defined by three values separated by spaces.
 * The first number is the type of the operation, being 0 translation and 1 rotation.
 * The second number is the dimension of the operation, being 0 x, 1 y and 2 z.
 * The third number is the value of the operation in radians for rotation and
 * micrometers for translation.
 * @param filename The name of the file to be read.
 * @param n The neuron to be replicated.
 * @return The neuronal network replicated according with the input file.
 */
std::vector<neuron> parseRPL(const std::string& filename, const neuron& n);
