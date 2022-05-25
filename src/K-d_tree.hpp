#pragma once
#include "SWC.hpp"
#include "heuristic.hpp"
#include <vector>
#include <limits>

struct node{
	compartment *data;
	node *root, *left, *right;
    int index;
};


/** Builds a K-d tree from a vector of compartments.
 *
 *
 * @param net
 * @param depth
 * @param parent
 * @return
 */
node* build_serial(neuron& net, int depth, node* parent);

/** Builds a kd-tree from a vector of compartments using the given heuristic.
 *
 * @param net The vector of compartments to build the tree from
 * @param depth The actual depth of the tree from parent
 * @param parent The parent node of the tree
 * @return
 */
node* build_parallel(neuron& net, int depth, node* parent, heuristic_func heuristic);

/** Frees the memory used by the tree
 *
 * @param root The root of the tree
 */
void free(node* root);

compartment* find_nearest(node* root, const compartment* target, double dist);


void range_query(node* root, const compartment* target, double dist, std::vector<compartment*>& result);
void range_query_parallel(node* root, const compartment* target, double dist, std::vector<compartment*>& result);