#pragma once
#include "SWC.hpp"
#include <vector>

struct node{
	compartment *data;
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
node* build_serial(std::vector<compartment*> net, int depth, node* parent);

/**
 *
 * @param net
 * @param depth
 * @param parent
 * @return
 */
node* build_parallel(std::vector<compartment*> net, int depth, node* parent, bool (*heuristic)(compartment* lhs, compartment* rhs, int depth, node* parent));

void free(node* root);