#pragma once
#include "K-d_tree.hpp"

typedef int (*heuristic_func)(neuron net, int depth);

int default_compare(neuron net, int depth);
int surface_area(neuron net, int depth);
int curve_complexity(neuron net, int depth);

extern const heuristic_func heuristic_funcs[];