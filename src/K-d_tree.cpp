#include "K-d_tree.hpp"
#include <omp.h>
#include <algorithm>
#include <iostream>
#include <string>


node* build_serial(neuron net, int depth, node* parent){
    if(net.empty()) return nullptr;

    node* elem = new node();
    elem-> root = parent;
    if(net.size() == 1){
        elem->data = net[0];
        return elem;
    }

    auto m = net.begin() + net.size()/2;
    std::nth_element(net.begin(), m, net.end(), [depth](compartment* lhs, compartment* rhs){
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
    std::vector<compartment*> left(net.begin(), net.begin() + net.size() / 2);
    std::vector<compartment*> right(net.begin() + net.size() / 2 + 1, net.end());
    elem->left = build_serial(left,depth+1,elem);
    elem->right = build_serial(right,depth+1,elem);
    return elem;
}

node* __build_parallel(neuron net, int depth, node* parent, bool (*heuristic)(compartment* lhs, compartment* rhs, int depth, node* parent)){
    if(net.empty()) return nullptr;
    node* elem = new node();
    elem-> root = parent;

    auto m = net.begin() + net.size()/2;
    std::nth_element(net.begin(), m, net.end(), [depth, elem, heuristic](compartment* lhs, compartment* rhs){
        return heuristic(lhs, rhs, depth, elem);
    });

    elem->data = *m;

    //std::cout << "Thread " + std::to_string(omp_get_thread_num()) + ": " + std::to_string(elem->data.sample) + "\n" ;
    neuron left(net.begin(), net.begin() + net.size() / 2);
    neuron right(net.begin() + net.size() / 2 + 1, net.end());
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

node* build_parallel(neuron net, int depth, node* parent, bool (*heuristic)(compartment* lhs, compartment* rhs, int depth, node* parent)) {
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

inline double dist2(const compartment* lhs,const compartment* rhs){
    return (lhs->x - rhs->x)*(lhs->x - rhs->x) + (lhs->y - rhs->y)*(lhs->y - rhs->y) + (lhs->z - rhs->z)*(lhs->z - rhs->z);
}

compartment* find_nearest(node* root, const compartment* target, double dist){
    node* cur = root;
    int depth = 0;
    dist *= dist;

    while(cur != nullptr){
        if(dist2(cur->data, target) < dist){
            return cur->data;
        }else {
            if(depth == 0) cur = target->x < cur->data->x ? cur->left : cur->right ;
            else if(depth == 1) cur = target->y < cur->data->y ? cur->left : cur->right ;
            else if(depth == 2) cur = target->z < cur->data->z ? cur->left : cur->right ;
            depth = (depth+1)%3;
        }
    }

    return nullptr;
}