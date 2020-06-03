#pragma once

#include "kernel/shape/ShapeRT.h"


class TONATIUH_KERNEL ShapeCube: public ShapeRT
{
    SO_NODE_HEADER(ShapeCube);

public:
    ShapeCube();
    static void initClass();

    BoundingBox getBox(ProfileRT* aperture) const;
    bool intersect(const Ray& ray, double* tHit, DifferentialGeometry* dg, ProfileRT* aperture) const;
    bool intersectP(const Ray& ray, ProfileRT* aperture) const;

    trt::TONATIUH_REAL xSize;
    trt::TONATIUH_REAL ySize;
    trt::TONATIUH_REAL zSize;

    NAME_ICON_FUNCTIONS("Cube", ":/ShapeCube.png")
//    void updateShapeGL(TShapeKit* parent);

protected:
    Vector3D getPoint(double u, double v) const;
    Vector3D getNormal(double u, double v) const;
};
