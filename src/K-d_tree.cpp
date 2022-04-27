#include "K-d_tree.hpp"
#include "heuristic.hpp"
#include <omp.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <unistd.h>


node* build_serial(neuron &net, int depth, node* parent){
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
    neuron left(net.begin(), m);
    neuron right(m + 1, net.end());
    elem->left = build_serial(left,depth+1,elem);
    elem->right = build_serial(right,depth+1,elem);
    return elem;
}

node* __build_parallel(neuron net, int depth, node* parent, int (*heuristic)(neuron net, int depth)){
    if(net.empty()) return nullptr;
    node* elem = new node();
    elem-> root = parent;

    auto m = net.begin() + net.size()/2;
    int dimension = heuristic(net, depth);
    std::nth_element(net.begin(), m, net.end(), [dimension](compartment* lhs, compartment* rhs){
        return lhs->get(dimension) < rhs->get(dimension);
    });

    elem->data = *m;
    elem->index = dimension;

    //std::cout << "Thread " + std::to_string(omp_get_thread_num()) + ": " + std::to_string(elem->data.sample) + "\n" ;
    #pragma omp task default(none) shared(net,elem, depth, std::cout, heuristic)
    {
        neuron left(net.begin(), net.begin() + net.size() / 2);
        elem->left = __build_parallel(left,depth+1,elem, heuristic);
    }
    #pragma omp task  default(none) shared(net,elem, depth, std::cout, heuristic)
    {
        neuron right(net.begin() + net.size() / 2 + 1, net.end());
        elem->right = __build_parallel(right,depth+1,elem, heuristic);
    }
    #pragma omp taskwait

    return elem;
}

node* build_parallel(neuron net, int depth, node* parent, int (*heuristic)(neuron net, int depth)) {
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
    #pragma omp single
    {
        res = __build_parallel(net, depth, parent, heuristic);
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

compartment* find_nearest(node* root, const compartment* target, double dist, double best_dist){
    if(root == nullptr) return nullptr;

    double d = dist2(root->data, target);

    if( d < best_dist){
        best_dist = d;
        return root->data;
    }
    if(d < dist*dist) return root->data;
    double dx = root->data->get(root->index) - target->get(root->index);
    compartment* result = find_nearest(dx > 0 ? root->left : root->right, target, dist, best_dist);
    if(result != nullptr) return result;
    if(dx*dx > best_dist) return nullptr;
    return find_nearest(dx > 0 ? root->right : root->left, target, dist, best_dist);
}