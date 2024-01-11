#pragma once

#include "SunPath/math/math.h"

#include <QVector>

namespace sp {

template <class T>
class Matrix
{
public:
    // constructors
    Matrix(): m_rows(0), m_columns(0) {}
    Matrix(int rows, int columns) {resize(rows, columns);}

    // components
    inline int rows() const {return m_rows;}
    inline int columns() const {return m_columns;}
    void resize(int rows, int columns);

    QVector<T>& data() {return m_data;}
    const QVector<T>& data() const {return m_data;}
    void setData(const QVector<T>& data) {m_data = data;}

    int index(int r, int c) const {return r*m_columns + c;}
    T& operator()(int r, int c) {return m_data[index(r, c)];}
    const T& operator()(int r, int c) const {return m_data[index(r, c)];}

    // functions
    bool isEmpty() const {return m_data.isEmpty();}
    void fill(const T& value) {m_data.fill(value);}

    bool operator==(const Matrix<T>& other) const;
    Matrix<T>& operator+=(const Matrix<T>& other);

protected:
    int m_rows;
    int m_columns;
    QVector<T> m_data;
};


template<class T>
void Matrix<T>::resize(int rows, int columns)
{
    Q_ASSERT(rows >= 0);
    Q_ASSERT(columns >= 0);
    m_rows = rows;
    m_columns = columns;
    m_data.resize(m_rows*m_columns);
}

template<class T>
bool Matrix<T>::operator==(const Matrix<T>& other) const
{
    if (m_rows != other.m_rows) return false;
    if (m_columns != other.m_columns) return false;
    return m_data == other.m_data;
}

template<class T>
Matrix<T>& Matrix<T>::operator+=(const Matrix<T>& other)
{
    Q_ASSERT(m_rows == other.m_rows);
    Q_ASSERT(m_columns == other.m_columns);
    for (int n = 0; n < m_data.size(); ++n)
        m_data[n] += other.m_data[n];
    return *this;
}

} // namespace sp
