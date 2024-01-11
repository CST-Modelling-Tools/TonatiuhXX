#include "MatrixNR.h"

#include <Eigen/Dense>

#include <algorithm>

namespace sp {

MatrixNR::MatrixNR(int rows, int columns):
    Matrix<double>(rows, columns)
{

}

QVector<double> MatrixNR::multiply(const QVector<double>& vector)
{
    Q_ASSERT(m_columns == vector.size());

    QVector<double> ans(m_rows);
    for (int r = 0; r < m_rows; ++r) {
        double temp = 0.;
        for (int c = 0; c < m_columns; ++c)
            temp += (*this)(r, c)*vector[c];
        ans[r] = temp;
    }

    return ans;
}

QVector<double> MatrixNR::solve(const QVector<double>& vector)
{
    using namespace Eigen;

    int size = m_rows;
    MatrixXd mat(size, size);
    VectorXd vec(size);

    for (int r = 0; r < size; ++r)
        for (int c = 0; c < size; ++c)
            mat(r, c) = (*this)(r, c);

    for (int r = 0; r < size; ++r)
        vec[r] = vector[r];

    vec = mat.colPivHouseholderQr().solve(vec);

    QVector<double> ans(size);
    for (int r = 0; r < size; ++r)
        ans[r] = vec[r];
    return ans;
}

void MatrixNR::invert()
{
    using namespace Eigen;

    int size = m_rows;
    MatrixXd mat(size, size);

    for (int r = 0; r < size; r++)
        for (int c = 0; c < size; c++)
            mat(r, c) = (*this)(r, c);

//    mat = mat.inverse(); // worse
    mat = mat.colPivHouseholderQr().solve(MatrixXd::Identity(size, size)); // better

    for (int r = 0; r < size; r++)
        for (int c = 0; c < m_columns; c++)
            (*this)(r, c) = mat(r, c);
}

} // namespace sp
