#ifndef ONENODE_HPP
#define ONENODE_HPP
#include<limits>

namespace lxj {
struct oneNode{
    short value;
    oneNode* left;
    oneNode* right;

    oneNode():value(std::numeric_limits<short>::min()),left(nullptr),right(nullptr){}
    oneNode(short v,oneNode* l=nullptr,oneNode*r=nullptr):
        value(v),left(l),right(r){}
    ~oneNode(){}
};
}

#endif // ONENODE_HPP
