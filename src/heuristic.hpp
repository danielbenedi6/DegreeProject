#pragma once
#include "K-d_tree.hpp"

typedef bool (*heuristic_func)(compartment* lhs, compartment* rhs, int depth, node* parent);

bool default_compare(compartment* lhs, compartment* rhs, int depth, node* parent);
bool surface_area(compartment* lhs, compartment* rhs, int depth, node* parent);
bool curve_complexity(compartment* lhs, compartment* rhs, int depth, node* parent);

extern const heuristic_func heuristic_funcs[];