#include "K-d_tree.hpp"
#include <algorithm>
#include <iostream>

#define DIMENSION 3

node* build_serial(std::vector<neuron> net, int depth, node* parent){
	if(net.empty()) return nullptr;
	
	node* elem = new node();
	elem-> root = parent;
	if(net.size() == 1){
		elem->data = net[0];
		return elem;
	}
	
	auto m = net.begin() + net.size()/2;
	std::nth_element(net.begin(), m, net.end(), [depth](neuron lhs, neuron rhs){
						switch(depth%DIMENSION){
							case 0:
								return lhs.x < rhs.x;
							case 1:
								return lhs.y < rhs.y;
							case 2:
								return lhs.z < rhs.z;
						}
						return false;
					});
	elem->data = net[net.size()/2];
	std::vector<neuron> left(net.begin(),net.begin()+net.size()/2);
	std::vector<neuron> right(net.begin()+net.size()/2+1, net.end());
	elem->left = build_serial(left,depth+1,elem);
	elem->right = build_serial(right,depth+1,elem);
	return elem;
}

node* build_parallel(std::vector<neuron> net, int depth, node* parent){
    if(net.empty()){
        return nullptr;
    }

    node* elem = new node();
    elem-> root = parent;
    if(net.size() == 1){
        elem->data = net[0];
        return elem;
    }

    auto m = net.begin() + net.size()/2;
    std::nth_element(net.begin(), m, net.end(), [depth](neuron lhs, neuron rhs){
        switch(depth%DIMENSION){
            case 0:
                return lhs.x < rhs.x;
            case 1:
                return lhs.y < rhs.y;
            case 2:
                return lhs.z < rhs.z;
        }
        return false;
    });

    elem->data = net[net.size()/2];
    #pragma omp task default(none) shared(net,elem, depth)
    {
        std::vector<neuron> left(net.begin(),net.begin()+net.size()/2);
        elem->left = build_serial(left,depth+1,elem);
    }
    #pragma omp task  default(none) shared(net,elem, depth)
    {
        std::vector<neuron> right(net.begin()+net.size()/2+1, net.end());
        elem->right = build_serial(right,depth+1,elem);
    }

    #pragma omp taskwait
    return elem;
}


void free(node* root){
    if(root == nullptr) return;
    free(root->left);
    free(root->right);
    delete root;
}