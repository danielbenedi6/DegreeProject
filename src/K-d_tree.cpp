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

compartment* find_nearest(node* root, const compartment* target, double dist){
    if(root == nullptr) return nullptr;

    double d = dist2(root->data, target);
    compartment* res = nullptr;

    if ((d - dist*dist) <= 1e-6){
        //Touch point found
        return root->data;
    }else{
        if(target->get(root->index) < root->data->get(root->index)){
            res = find_nearest(root->left, target, dist);
            if(res == nullptr && (root->data->get(root->index) - target->get(root->index)) <= dist){
                res = find_nearest(root->right, target, dist);
            }
        }else{
            res = find_nearest(root->right, target, dist);
            if(res == nullptr && (target->get(root->index) - root->data->get(root->index)) <= dist){
                res = find_nearest(root->left, target, dist);
            }
        }
    }
    return res;
}

void range_query(node* root, const compartment* target, double dist, std::vector<compartment*>& result){
    if(root == nullptr) return;
    double d = dist2(root->data, target);

    if((d - dist*dist) <= 1e-6 && target->neuron_id != root->data->neuron_id){
        //Touchpoint found
        result.push_back(root->data);
    }
    if(target->get(root->index) < root->data->get(root->index)){
        range_query(root->left, target, dist, result);
        if((root->data->get(root->index) - target->get(root->index)) <= dist){
            range_query(root->right, target, dist, result);
        }
    }else{
        range_query(root->right, target, dist, result);
        if((target->get(root->index) - root->data->get(root->index)) <= dist){
            range_query(root->left, target, dist, result);
        }
    }
}

void range_query_tasks(node* root, const compartment* target, double dist, std::vector<compartment*>& result){
    if(root == nullptr) return;

    #pragma omp task default(none) shared(root, target, dist, result)
    {
        double d = dist2(root->data, target);
        if((d - dist*dist) <= 1e-6 && target->neuron_id != root->data->neuron_id){
            //Touchpoint found
            result.push_back(root->data);
        }
    };
    if(target->get(root->index) < root->data->get(root->index)){
        #pragma omp task default(none) shared(root, target, dist, result)
        {
            range_query(root->left, target, dist, result);
        };
        #pragma omp task default(none) shared(root, target, dist, result)
        {
            if((root->data->get(root->index) - target->get(root->index)) <= dist){
                range_query(root->right, target, dist, result);
            }
        };
    }else {
        #pragma omp task default(none) shared(root, target, dist, result)
        {
            range_query(root->right, target, dist, result);
        };
        #pragma omp task default(none) shared(root, target, dist, result)
        {
            if((target->get(root->index) - root->data->get(root->index)) <= dist){
                range_query(root->left, target, dist, result);
            }
        };
    }
    #pragma omp taskwait
}

void range_query_parallel(node* root, const compartment* target, double dist, std::vector<compartment*>& result){
    if(root == nullptr) return;

    #pragma omp parallel default(none) shared(root,target,dist,result)
    #pragma omp single
    {
        range_query_tasks(root,target,dist,result);
        #pragma omp taskwait
    }
}
