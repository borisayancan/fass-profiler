#ifndef MATH_TIPOS_H
#define MATH_TIPOS_H
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>


#ifndef u8
typedef unsigned char u8;
#endif

#ifndef byte
typedef unsigned char byte;
#endif

#ifndef u16
typedef unsigned short u16;
#endif

#ifndef u32
typedef uint32_t u32;
#endif

#ifndef u64
typedef uint64_t u64;
#endif


/***********************************************************************
 *
 *  Vector generico de doubles
 *
 * ********************************************************************/
class Vector
{
private:
    double* m_d;
    int m_elems;

public:

    // Construir con dimensiones para alocar espacio
    explicit Vector(int elems)
    {
        m_elems = elems;
        m_d = (double*)malloc(m_elems*sizeof(double));
    }

    Vector()
    {
        m_elems = 0;
        m_d = NULL;
    }

    Vector(const Vector& other)
    {
        m_elems = other.lenght();
        m_d = (double*)malloc(m_elems*sizeof(double));
        memcpy(m_d, other.data(),m_elems*sizeof(double));
    }

    // Liberar memoria al destruir
    ~Vector()
    {
        if(m_d!=NULL)
            free(m_d);
        m_d=NULL;
        m_elems = 0;
    }

    double& operator [](const int idx)
    {
        return m_d[idx];
    }

    Vector& operator=(const Vector& other)
    {
        m_elems = other.lenght();
        if(m_d) free(m_d);
        m_d = (double*)malloc(m_elems*sizeof(double));
        memcpy(m_d, other.data(),m_elems*sizeof(double));
        return *this;
    }

    Vector& operator/=(double divisor)
    {
        for(int i=0; i<m_elems; i++)
            m_d[i] /= divisor;
        return *this;
    }

    Vector& operator+=(const Vector& other)
    {
        const double* src = other.data();
        for(int i=0; i<m_elems; i++)
            m_d[i] += src[i];
        return *this;
    }

    Vector& operator-(const Vector& other)
    {
        const double* src = other.data();
        for(int i=0; i<m_elems; i++)
            m_d[i] -= src[i];
        return *this;
    }

    void set(double d)
    {
        for(int i=0; i<m_elems; i++)
            m_d[i] = d;
    }

    void truncate(int len)
    {
        if(len>=m_elems)
            return;

        if(len<=0)
        {
            m_elems=0;
            free(m_d);
        }
        else
        {
            m_elems=len;
            m_d = (double*)realloc(m_d, sizeof(double)*m_elems);
        }
    }

    void clear()
    {
        if(m_d!=NULL) free(m_d);
        m_d=NULL;
        m_elems = 0;
    }

    const int& lenght()  const { return m_elems; }
    double* data() const { return m_d; }
};


/***********************************************************************
 *
 *  Matriz generica de doubles
 *
 * ********************************************************************/
class Matrix
{
private:
    int m_elems;
    int m_row, m_col;
    double* m_d;

public:

    // Construir con dimensiones para alocar espacio
    explicit Matrix(int row, int col)
    {
        m_elems = row*col;
        m_row=row;
        m_col=col;
        m_d = (double*)malloc(m_elems*sizeof(double));
    }

    Matrix()
    {
        m_elems = 0;
        m_row=0;
        m_col=0;
        m_d = NULL;
    }

    Matrix(const Matrix& other)
    {
        m_elems = other.cols()*other.rows();
        m_row=other.rows();
        m_col=other.cols();
        m_d = (double*)malloc(m_elems*sizeof(double));
        memcpy(m_d, other.data(),m_elems*sizeof(double));
    }

    // Liberar memoria al destruir
    ~Matrix()
    {
        if(m_d!=NULL)
            free(m_d);
        m_d=NULL;
        m_elems = 0;
        m_row=0;
        m_col=0;
    }

    // Cagar la matriz con un array de uint16
    void load(u16* d)
    {
        for(int i=0; i<m_elems; i++)
            m_d[i] = d[i];
    }

    void set(double d)
    {
        for(int i=0; i<m_elems; i++)
            m_d[i] = d;
    }

    double* operator [](const int row)
    {
        return &m_d[row*m_col];
    }

    Matrix& operator=(const Matrix& other)
    {
        m_elems = other.cols()*other.rows();
        m_row=other.rows();
        m_col=other.cols();
        if(m_d) free(m_d);
        m_d = (double*)malloc(m_elems*sizeof(double));
        memcpy(m_d, other.data(),m_elems*sizeof(double));
        return *this;
    }

    Matrix& t()
    {
        Matrix tmp = *this;
        for(int row=0; row<m_row; row++)
            for(int col=0; col<m_col; col++)
                m_d[row*m_col+col] = tmp[col][row];

        return *this;
    }

    void clear()
    {
        if(m_d!=NULL) free(m_d);
        m_d=NULL;
        m_elems = 0;
        m_row=0;
        m_col=0;
    }

    Vector getRow(int row)
    {
        Vector d(m_col);
        memcpy(d.data(), &m_d[row*m_col], sizeof(double)*m_col);
        return d;
    }

    bool isEmpty()    { return m_col==0 || m_row==0; }
    const int& rows() const { return m_row; }
    const int& cols() const { return m_col; }
    double* data()    const { return m_d; }
};


// Helpers
#define rangeok(A,B,C) (A>=B && A<=C)
#define fix(A) ((int)A)

#endif // MATH_TIPOS_H
