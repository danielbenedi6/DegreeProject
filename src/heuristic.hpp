#pragma once
#include "SWC.hpp"
#include <tuple>

using partition = std::tuple<int, compartment*, neuron, neuron>*;
using heuristic_func = partition (*)(neuron& net, int depth);

partition default_compare(neuron& net, int depth);
partition surface_area(neuron& net, int depth);
partition curve_complexity(neuron& net, int depth);
partition variance_heuristic(neuron& net, int depth);
partition variance_ratio_heuristic(neuron& net, int depth);

extern const heuristic_func heuristic_funcs[];