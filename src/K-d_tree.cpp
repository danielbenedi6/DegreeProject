#include "K-d_tree.hpp"
#include "heuristic.hpp"
#include <algorithm>
#include <iostream>

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
        return lhs->get(depth%3) < rhs->get(depth%3) || (lhs->get(depth%3) == rhs->get(depth%3) && lhs->sample < rhs->sample);
    });
    m = net.begin() + net.size()/2;
    elem->data = *m;
    neuron left(net.begin(), m);
    neuron right(m + 1, net.end());
    elem->left = build_serial(left,depth+1,elem);
    elem->right = build_serial(right,depth+1,elem);
    elem->index = depth%3;
    return elem;
}

node* build_parallel_tasks(neuron& net, int depth, node* parent, heuristic_func heuristic){
    if(net.empty()){
        return nullptr;
    }
    node* elem = new node();
    elem-> root = parent;

    auto [dimension, split, left, right] = *heuristic(net, depth);

    elem->data = split;
    elem->index = dimension;

    //std::cout << "Thread " + std::to_string(omp_get_thread_num()) + ": " + std::to_string(elem->data.sample) + "\n" ;

#pragma omp task default(none) shared(net, elem, depth, heuristic, left)
        {
            elem->left = build_parallel_tasks(left, depth + 1, elem, heuristic);
        }
#pragma omp task  default(none) shared(net, elem, depth, heuristic, right)
        {
            elem->right = build_parallel_tasks(right, depth + 1, elem, heuristic);
        }
#pragma omp taskwait

    return elem;
}

node* build_parallel(neuron& net, int depth, node* parent, heuristic_func heuristic) {
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
        res = build_parallel_tasks(net, depth, parent, heuristic);
        #pragma omp taskwait
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

compartment* find_nearest(node* root, const compartment* target, double dist, double& best_dist){
    if(root == nullptr) return nullptr;

    double d = dist2(root->data, target);

    if ((d - dist*dist) <= 1e-6){
        //Touch point found
        best_dist = d;
        return root->data;
    }else{
        if(target->get(root->index) < root->data->get(root->index)){
            if((target->get(root->index) - best_dist) <= root->data->get(root->index)){
                compartment* res = find_nearest(root->left, target, dist, best_dist);
                if(res != nullptr) return res;
            }
            if((target->get(root->index) + best_dist) > root->data->get(root->index)){
                compartment* res = find_nearest(root->right, target, dist, best_dist);
                if(res != nullptr) return res;
            }
        }else{
            if((target->get(root->index) + best_dist) > root->data->get(root->index)){
                compartment* res = find_nearest(root->right, target, dist, best_dist);
                if(res != nullptr) return res;
            }
            if((target->get(root->index) - best_dist) <= root->data->get(root->index)){
                compartment* res = find_nearest(root->left, target, dist, best_dist);
                if(res != nullptr) return res;
            }
        }
    }
    return nullptr;

    /*
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
     */
}