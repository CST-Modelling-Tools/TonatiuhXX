#include "Matrix4x4.h"

#include "gcf.h"


Matrix4x4::Matrix4x4()
{
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            m[i][j] = i == j ? 1. : 0.;
}

Matrix4x4::Matrix4x4(
    double t00, double t01, double t02, double t03,
    double t10, double t11, double t12, double t13,
    double t20, double t21, double t22, double t23,
    double t30, double t31, double t32, double t33
)
{
    m[0][0] = t00; m[0][1] = t01; m[0][2] = t02; m[0][3] = t03;
    m[1][0] = t10; m[1][1] = t11; m[1][2] = t12; m[1][3] = t13;
    m[2][0] = t20; m[2][1] = t21; m[2][2] = t22; m[2][3] = t23;
    m[3][0] = t30; m[3][1] = t31; m[3][2] = t32; m[3][3] = t33;
}

Matrix4x4::Matrix4x4(double array[4][4])
{
    memcpy(m, array, 16*sizeof(double));
}

Matrix4x4::Matrix4x4(const Matrix4x4& rhs)
{
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            m[i][j] = rhs.m[i][j];
}

bool Matrix4x4::operator==(const Matrix4x4& matrix) const
{
    if (this == &matrix) return true;

    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            if (!gcf::equals(m[i][j], matrix.m[i][j]))
                return false;

    return true;
}

std::shared_ptr<Matrix4x4> Matrix4x4::Transpose() const
{
    return std::make_shared<Matrix4x4>(
        m[0][0], m[1][0], m[2][0], m[3][0],
        m[0][1], m[1][1], m[2][1], m[3][1],
        m[0][2], m[1][2], m[2][2], m[3][2],
        m[0][3], m[1][3], m[2][3], m[3][3]
    );
}

std::shared_ptr<Matrix4x4> Matrix4x4::Inverse() const
{
    double det = m[0][1] * m[1][3] * m[2][2] * m[3][0] - m[0][1] * m[1][2] * m[2][3] * m[3][0] - m[0][0] * m[1][3] * m[2][2] * m[3][1] + m[0][0] * m[1][2] * m[2][3] * m[3][1]
                 - m[0][1] * m[1][3] * m[2][0] * m[3][2] + m[0][0] * m[1][3] * m[2][1] * m[3][2] + m[0][1] * m[1][0] * m[2][3] * m[3][2] - m[0][0] * m[1][1] * m[2][3] * m[3][2]
                 + m[0][3] * (m[1][2] * m[2][1] * m[3][0] - m[1][1] * m[2][2] * m[3][0] - m[1][2] * m[2][0] * m[3][1] + m[1][0] * m[2][2] * m[3][1] + m[1][1] * m[2][0] * m[3][2] - m[1][0] * m[2][1] * m[3][2])
                 + m[3][3] * (m[0][1] * m[1][2] * m[2][0] - m[0][0] * m[1][2] * m[2][1] - m[0][1] * m[1][0] * m[2][2] + m[0][0] * m[1][1] * m[2][2])
                 + m[0][2] * (-m[1][3] * m[2][1] * m[3][0] + m[1][1] * m[2][3] * m[3][0] + m[1][3] * m[2][0] * m[3][1] - m[1][0] * m[2][3] * m[3][1] - m[1][1] * m[2][0] * m[3][3] + m[1][0] * m[2][1] * m[3][3]);

    if (fabs(det) < gcf::Epsilon) gcf::SevereError("Singular matrix in Matrix4x4::Inverse()");
    double alpha = 1./det;

    double inv00 = (-m[1][3] * m[2][2] * m[3][1] + m[1][2] * m[2][3] * m[3][1] + m[1][3] * m[2][1] * m[3][2] - m[1][1] * m[2][3] * m[3][2] - m[1][2] * m[2][1] * m[3][3] + m[1][1] * m[2][2] * m[3][3]) * alpha;
    double inv01 = (m[0][3] * m[2][2] * m[3][1] - m[0][2] * m[2][3] * m[3][1] - m[0][3] * m[2][1] * m[3][2] + m[0][1] * m[2][3] * m[3][2] + m[0][2] * m[2][1] * m[3][3] - m[0][1] * m[2][2] * m[3][3]) * alpha;
    double inv02 = (-m[0][3] * m[1][2] * m[3][1] + m[0][2] * m[1][3] * m[3][1] + m[0][3] * m[1][1] * m[3][2] - m[0][1] * m[1][3] * m[3][2] - m[0][2] * m[1][1] * m[3][3] + m[0][1] * m[1][2] * m[3][3]) * alpha;
    double inv03 = (m[0][3] * m[1][2] * m[2][1] - m[0][2] * m[1][3] * m[2][1] - m[0][3] * m[1][1] * m[2][2] + m[0][1] * m[1][3] * m[2][2] + m[0][2] * m[1][1] * m[2][3] - m[0][1] * m[1][2] * m[2][3]) * alpha;
    double inv10 = (m[1][3] * m[2][2] * m[3][0] - m[1][2] * m[2][3] * m[3][0] - m[1][3] * m[2][0] * m[3][2] + m[1][0] * m[2][3] * m[3][2] + m[1][2] * m[2][0] * m[3][3] - m[1][0] * m[2][2] * m[3][3]) * alpha;
    double inv11 = (-m[0][3] * m[2][2] * m[3][0] + m[0][2] * m[2][3] * m[3][0] + m[0][3] * m[2][0] * m[3][2] - m[0][0] * m[2][3] * m[3][2] - m[0][2] * m[2][0] * m[3][3] + m[0][0] * m[2][2] * m[3][3]) * alpha;
    double inv12 = (m[0][3] * m[1][2] * m[3][0] - m[0][2] * m[1][3] * m[3][0] - m[0][3] * m[1][0] * m[3][2] + m[0][0] * m[1][3] * m[3][2] + m[0][2] * m[1][0] * m[3][3] - m[0][0] * m[1][2] * m[3][3]) * alpha;
    double inv13 = (-m[0][3] * m[1][2] * m[2][0] + m[0][2] * m[1][3] * m[2][0] + m[0][3] * m[1][0] * m[2][2] - m[0][0] * m[1][3] * m[2][2] - m[0][2] * m[1][0] * m[2][3] + m[0][0] * m[1][2] * m[2][3]) * alpha;
    double inv20 = (-m[1][3] * m[2][1] * m[3][0] + m[1][1] * m[2][3] * m[3][0] + m[1][3] * m[2][0] * m[3][1] - m[1][0] * m[2][3] * m[3][1] - m[1][1] * m[2][0] * m[3][3] + m[1][0] * m[2][1] * m[3][3]) * alpha;
    double inv21 = (m[0][3] * m[2][1] * m[3][0] - m[0][1] * m[2][3] * m[3][0] - m[0][3] * m[2][0] * m[3][1] + m[0][0] * m[2][3] * m[3][1] + m[0][1] * m[2][0] * m[3][3] - m[0][0] * m[2][1] * m[3][3]) * alpha;
    double inv22 = (-m[0][3] * m[1][1] * m[3][0] + m[0][1] * m[1][3] * m[3][0] + m[0][3] * m[1][0] * m[3][1] - m[0][0] * m[1][3] * m[3][1] - m[0][1] * m[1][0] * m[3][3] + m[0][0] * m[1][1] * m[3][3]) * alpha;
    double inv23 = (m[0][3] * m[1][1] * m[2][0] - m[0][1] * m[1][3] * m[2][0] - m[0][3] * m[1][0] * m[2][1] + m[0][0] * m[1][3] * m[2][1] + m[0][1] * m[1][0] * m[2][3] - m[0][0] * m[1][1] * m[2][3]) * alpha;
    double inv30 = (m[1][2] * m[2][1] * m[3][0] - m[1][1] * m[2][2] * m[3][0] - m[1][2] * m[2][0] * m[3][1] + m[1][0] * m[2][2] * m[3][1] + m[1][1] * m[2][0] * m[3][2] - m[1][0] * m[2][1] * m[3][2]) * alpha;
    double inv31 = (-m[0][2] * m[2][1] * m[3][0] + m[0][1] * m[2][2] * m[3][0] + m[0][2] * m[2][0] * m[3][1] - m[0][0] * m[2][2] * m[3][1] - m[0][1] * m[2][0] * m[3][2] + m[0][0] * m[2][1] * m[3][2]) * alpha;
    double inv32 = (m[0][2] * m[1][1] * m[3][0] - m[0][1] * m[1][2] * m[3][0] - m[0][2] * m[1][0] * m[3][1] + m[0][0] * m[1][2] * m[3][1] + m[0][1] * m[1][0] * m[3][2] - m[0][0] * m[1][1] * m[3][2]) * alpha;
    double inv33 = (-m[0][2] * m[1][1] * m[2][0] + m[0][1] * m[1][2] * m[2][0] + m[0][2] * m[1][0] * m[2][1] - m[0][0] * m[1][2] * m[2][1] - m[0][1] * m[1][0] * m[2][2] + m[0][0] * m[1][1] * m[2][2]) * alpha;

    return std::make_shared<Matrix4x4>(inv00, inv01, inv02, inv03, inv10, inv11, inv12, inv13, inv20, inv21, inv22, inv23, inv30, inv31, inv32, inv33);
}

std::shared_ptr<Matrix4x4> multiply(const Matrix4x4& m1, const Matrix4x4& m2)
{
    double r[4][4];
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            r[i][j] = m1.m[i][0] * m2.m[0][j] +
                      m1.m[i][1] * m2.m[1][j] +
                      m1.m[i][2] * m2.m[2][j] +
                      m1.m[i][3] * m2.m[3][j];
    return std::make_shared<Matrix4x4>(r);
}

std::ostream& operator<<(std::ostream& os, const Matrix4x4& matrix)
{
    for (int i = 0; i < 4; ++i)
    {
        os << "[ ";
        for (int j = 0; j < 4; ++j)
        {
            if (fabs(matrix.m[i][j]) < gcf::Epsilon) os << "0";
            else os << matrix.m[i][j];
            if (j != 3) os << ", ";
        }
        os << " ] " << std::endl;
    }
    return os;
}
