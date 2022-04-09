#include "K-d_tree.hpp"
#include <omp.h>
#include <algorithm>
#include <iostream>
#include <string>

#define DIMENSION 3

node* build_serial(std::vector<neuron> net, std::vector<neuron>::iterator begin, std::vector<neuron>::iterator end, int depth, node* parent){
	if((begin-end) == 0) return nullptr;
	
	node* elem = new node();
	elem-> root = parent;
	if((end-begin) == 1){
		elem->data = net[0];
		return elem;
	}
	
	auto m = begin + (end - begin)/2;
	std::nth_element(begin, m, end, [depth](neuron lhs, neuron rhs){
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
	elem->data = *m;
	elem->left = build_serial(net, begin, m,depth+1,elem);
	elem->right = build_serial(net, m+1, end,depth+1,elem);
	return elem;
}

node* __build_parallel(std::vector<neuron> net, std::vector<neuron>::iterator begin, std::vector<neuron>::iterator end, int depth, node* parent) {
    if((end-begin) == 0) return nullptr;
    node* elem = new node();
    elem-> root = parent;

    auto m = begin + (end - begin)/2;
    std::nth_element(begin, m, end, [depth](neuron lhs, neuron rhs){
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

    elem->data = *m;

    //std::cout << "Thread " + std::to_string(omp_get_thread_num()) + ": " + std::to_string(elem->data.sample) + "\n" ;
    #pragma omp task default(none) shared(net,elem, depth, begin, end, m, std::cout)
    {
        elem->left = __build_parallel(net, begin, m,depth+1,elem);
    }
    #pragma omp task  default(none) shared(net,elem, depth, begin, end, m, std::cout)
    {
        elem->right = __build_parallel(net, m+1, end,depth+1,elem);
    }
    #pragma omp taskwait

    return elem;
}

node* build_parallel(std::vector<neuron> net, std::vector<neuron>::iterator begin, std::vector<neuron>::iterator end, int depth, node* parent) {
    if((end-begin) == 0) return nullptr;

    if((end-begin) == 1){
        node* elem = new node();
        elem-> root = parent;
        elem->data = net[0];
        return elem;
    }

    //if( (end - begin) < 200 ){
    //    return build_serial(net, begin, end, depth, parent);
    //} else{
    node* res;
    #pragma omp parallel default(none) shared(net,begin,end,depth,parent,res)
    {
        #pragma omp single
        {
            #pragma omp task default(none) shared(net,begin,end,depth,parent,res)
            {
                res = __build_parallel(net, begin, end, depth, parent);
                #pragma omp taskwait
            }
        }
    }
    return res;
}


void free(node* root){
    if(root == nullptr) return;
    free(root->left);
    free(root->right);
    delete root;
}