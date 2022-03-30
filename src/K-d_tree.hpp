#pragma once
#include "SWC.hpp"
#include <vector>

struct node{
	neuron data;
	node *root, *left, *right;
};

/**
 *
 *
 * @param net
 * @param depth
 * @param parent
 * @return
 */
node* build_serial(std::vector<neuron> net, int depth, node* parent);

/**
 *
 * @param net
 * @param depth
 * @param parent
 * @return
 */
node* build_parallel(std::vector<neuron> net, int depth, node* parent);

void free(node* root);