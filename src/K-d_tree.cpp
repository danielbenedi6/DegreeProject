#include "K-d_tree.hpp"
#include <algorithm>

node* build_serial(std::vector<neuron> net, int depth, node* parent){
	if(net.size() == 0) return nullptr;
	
	node* elem = new node();
	elem-> root = parent;
	if(net.size() == 1){
		elem->data = net[0];
		return elem;
	}
	
	auto m = net.begin() + net.size()/2;
	std::nth_element(net.begin(), m, net.end(), [depth](neuron lhs, neuron rhs){
						switch(depth%3){
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
