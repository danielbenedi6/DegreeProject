#pragma once
#include "K-d_tree.hpp"

typedef bool (*heuristic_func)(neuron* lhs, neuron* rhs, int depth, node* parent);

bool default_compare(neuron* lhs, neuron* rhs, int depth, node* parent);
bool surface_area(neuron* lhs, neuron* rhs, int depth, node* parent);
bool curve_complexity(neuron* lhs, neuron* rhs, int depth, node* parent);

extern const heuristic_func heuristic_funcs[];