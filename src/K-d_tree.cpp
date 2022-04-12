#include "K-d_tree.hpp"
#include <omp.h>
#include <algorithm>
#include <iostream>
#include <string>


node* build_serial(std::vector<neuron*> net, int depth, node* parent){
    if(net.empty()) return nullptr;

    node* elem = new node();
    elem-> root = parent;
    if(net.size() == 1){
        elem->data = net[0];
        return elem;
    }

    auto m = net.begin() + net.size()/2;
    std::nth_element(net.begin(), m, net.end(), [depth](neuron* lhs, neuron* rhs){
        switch(depth%3){
            case 0:
                return lhs->x < rhs->x;
            case 1:
                return lhs->y < rhs->y;
            case 2:
                return lhs->z < rhs->z;
        }
        return false;
    });
    elem->data = net[net.size()/2];
    std::vector<neuron*> left(net.begin(),net.begin()+net.size()/2);
    std::vector<neuron*> right(net.begin()+net.size()/2+1, net.end());
    elem->left = build_serial(left,depth+1,elem);
    elem->right = build_serial(right,depth+1,elem);
    return elem;
}

node* __build_parallel(std::vector<neuron*> net, int depth, node* parent, bool (*heuristic)(neuron* lhs, neuron* rhs, int depth, node* parent)){
    if(net.empty()) return nullptr;
    node* elem = new node();
    elem-> root = parent;

    auto m = net.begin() + net.size()/2;
    std::nth_element(net.begin(), m, net.end(), [depth, elem, heuristic](neuron* lhs, neuron* rhs){
        return heuristic(lhs, rhs, depth, elem);
    });

    elem->data = *m;

    //std::cout << "Thread " + std::to_string(omp_get_thread_num()) + ": " + std::to_string(elem->data.sample) + "\n" ;
    std::vector<neuron*> left(net.begin(),net.begin()+net.size()/2);
    std::vector<neuron*> right(net.begin()+net.size()/2+1, net.end());
    #pragma omp task default(none) shared(net,elem, depth, std::cout, heuristic) private(left)
    {
        elem->left = __build_parallel(left,depth+1,elem, heuristic);
    }
    #pragma omp task  default(none) shared(net,elem, depth, std::cout, heuristic) private(right)
    {
        elem->right = __build_parallel(right,depth+1,elem, heuristic);
    }

    return elem;
}

node* build_parallel(std::vector<neuron*> net, int depth, node* parent, bool (*heuristic)(neuron* lhs, neuron* rhs, int depth, node* parent)) {
    if(net.empty()) return nullptr;

    if(net.size() == 1){
        node* elem = new node();
        elem-> root = parent;
        elem->data = net[0];
        return elem;
    }

    //if( (end - begin) < 200 ){
    //    return build_serial(net, begin, end, depth, parent);
    //} else{
    node* res;
    #pragma omp parallel default(none) shared(net,depth,parent,res, heuristic)
    {
        #pragma omp single
        {
            #pragma omp task default(none) shared(net,depth,parent,res, heuristic)
            {
                res = __build_parallel(net, depth, parent, heuristic);
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