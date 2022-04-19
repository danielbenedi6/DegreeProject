#include "heuristic.hpp"
#define DIMENSION 3

const heuristic_func heuristics[] = {default_compare,surface_area,curve_complexity};

bool default_compare(compartment* lhs, compartment* rhs, int depth, node* parent){
    switch(depth%DIMENSION){
        case 0:
            return lhs->x < rhs->x;
        case 1:
            return lhs->y < rhs->y;
        case 2:
            return lhs->z < rhs->z;
    }
    return false;
}

bool surface_area(compartment* lhs, compartment* rhs, int depth, node* parent){
    return true;
}

bool curve_complexity(compartment* lhs, compartment* rhs, int depth, node* parent){
    return true;
}
