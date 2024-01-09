#pragma once

#include <vector>

#include "Triangle.h"



class BVHNode
{
public:
    BVHNode();
    ~BVHNode();

    void setBox(const Box3D& bbox) {m_box = bbox;}
    Box3D box() const {return m_box;}

    void makeLeaf(uint index, uint size);
    bool isLeaf() const {return m_isLeaf;}
    int index() const {return m_index;}
    int triangles() const {return m_size;}

    void setNodeLeft(BVHNode* node) {m_nodeLeft = node;}
    BVHNode* nodeLeft() const {return m_nodeLeft;}
    void setNodeRight(BVHNode* node) {m_nodeRight = node;}
    BVHNode* nodeRight() const {return m_nodeRight;}

private:
    Box3D m_box;
    bool m_isLeaf; // end of tree, contains the starting index and size for triangles
    int m_index;
    int m_size;

    BVHNode* m_nodeLeft;
    BVHNode* m_nodeRight;
};



class BVH
{
public:
    BVH(std::vector<Triangle*>* triangles, int leafSize = 1);
    ~BVH();

    Box3D box() const;
    bool intersect(const Ray& ray, double* tHit, DifferentialGeometry* dg) const;
    bool intersect(BVHNode* node, const Ray& ray, double *tHit, DifferentialGeometry* dg) const;

private:
    void build(int left_index, int right_index, BVHNode* parent, int depth);
    void sortTrinagles(int indexStart, int indexEnd, int dimension);

    std::vector<Triangle*>* m_triangles;
    int m_leafSize;
    int m_nNodes;
    int m_nLeafs;
    BVHNode* m_nodeRoot;
};
