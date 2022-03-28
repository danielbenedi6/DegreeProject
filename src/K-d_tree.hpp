#pragma once
#include "SWC.hpp"
#include <vector>

struct node{
	neuron data;
	node *root, *left, *right;
};

node* build_serial(std::vector<neuron> net, int depth, node* parent);
