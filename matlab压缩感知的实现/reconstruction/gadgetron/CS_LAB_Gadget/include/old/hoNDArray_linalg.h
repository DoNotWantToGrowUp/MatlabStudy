#pragma once

#include "CS_LAB_export.h"
#include "hoNDArray.h"

#ifdef USE_ARMADILLO
    #include "hoArmadillo.h"
#endif // USE_ARMADILLO

#ifndef lapack_int
    #define lapack_int int
#endif // lapack_int

/// ----------------------------------------------------------------------
/// the fortran interface of lapack and blas functions are called
/// ----------------------------------------------------------------------

namespace Gadgetron
{

// following matrix computation calls lapacke functions

/// C = A*B for complex float
EXPORTCSLAB void gemm(hoNDArray< std::complex<float> >& C, const hoNDArray< std::complex<float> >& A, const hoNDArray< std::complex<float> >& B);
/// if transA==true, C = A'*B
/// if transB==true, C=A*B'
/// if both are true, C=A'*B'
template<typename T> EXPORTCSLAB
void gemm(hoNDArray<T>& C, 
        const hoNDArray<T>& A, bool transA, 
        const hoNDArray<T>& B, bool transB);

/// perform a symmetric rank-k update (no conjugated).
template<typename T> EXPORTCSLAB 
void syrk(hoNDArray<T>& C, const hoNDArray<T>& A, char uplo, bool isATA);

/// perform a Hermitian rank-k update.
template<typename T> EXPORTCSLAB 
void herk(hoNDArray<T>& C, const hoNDArray<T>& A, char uplo, bool isAHA);

/// compute the Cholesky factorization of a real symmetric positive definite matrix A
template<typename T> EXPORTCSLAB 
void potrf(hoNDArray<T>& A, char uplo);

/// compute all eigenvalues and eigenvectors of a Hermitian matrix A
template<typename T> EXPORTCSLAB 
void heev(hoNDArray<T>& A, hoNDArray<typename realType<T>::Type>& eigenValue);

template<typename T> EXPORTCSLAB
void heev(hoNDArray< std::complex<T> >& A, hoNDArray<  std::complex<T> >& eigenValue);

/// compute inverse of a symmetric (Hermitian) positive-definite matrix A
template<typename T> EXPORTCSLAB 
void potri(hoNDArray<T>& A);

/// compute the inverse of a triangular matrix A
template<typename T> EXPORTCSLAB 
void trtri(hoNDArray<T>& A, char uplo);

/// solve Ax=b, a symmetric or Hermitian positive-definite matrix A and multiple right-hand sides b
/// b is replaced with x
template<typename T> EXPORTCSLAB
void posv(hoNDArray<T>& A, hoNDArray<T>& b);

/// solve Ax=b, a square symmetric / hermitian matrix A and multiple right-hand sides b
/// for float and double, A is a symmetric matrix
/// for complex type, A is a hermitian matrix
/// b is replaced with x
template<typename T> EXPORTCSLAB
void hesv(hoNDArray<T>& A, hoNDArray<T>& b);

/// solve Ax=b, a square matrix A and multiple right-hand sides b
/// b is replaced with x
template<typename T> EXPORTCSLAB
void gesv(hoNDArray<T>& A, hoNDArray<T>& b);

/// solve Ax=b with Tikhonov regularization
//template<typename T> EXPORTCSLAB
//void SolveLinearSystem_Tikhonov(hoNDArray<T>& A, hoNDArray<T>& b, hoNDArray<T>& x, double lamda);

/// Computes the LU factorization of a general m-by-n matrix
/// this function is called by general matrix inversion
template<typename T> EXPORTCSLAB 
void getrf(hoNDArray<T>& A, hoNDArray<lapack_int>& ipiv);

/// Computes the inverse of an LU-factored general matrix
template<typename T> EXPORTCSLAB 
void getri(hoNDArray<T>& A);

}


/// compute the Cholesky factorization of a real symmetric positive definite matrix A
template<typename T> EXPORTCSLAB 
void potrf(hoNDArray<T>& A, char uplo);

/// compute all eigenvalues and eigenvectors of a Hermitian matrix A
template<typename T> EXPORTCSLAB 
void heev(hoNDArray<T>& A, hoNDArray<typename realType<T>::Type>& eigenValue);

template<typename T> EXPORTCSLAB
void heev(hoNDArray< std::complex<T> >& A, hoNDArray<  std::complex<T> >& eigenValue);

/// compute inverse of a symmetric (Hermitian) positive-definite matrix A
template<typename T> EXPORTCSLAB 
void potri(hoNDArray<T>& A);

/// compute the inverse of a triangular matrix A
template<typename T> EXPORTCSLAB 
void trtri(hoNDArray<T>& A, char uplo);

/// solve Ax=b, a symmetric or Hermitian positive-definite matrix A and multiple right-hand sides b
/// b is replaced with x
template<typename T> EXPORTCSLAB
void posv(hoNDArray<T>& A, hoNDArray<T>& b);

/// solve Ax=b, a square symmetric / hermitian matrix A and multiple right-hand sides b
/// for float and double, A is a symmetric matrix
/// for complex type, A is a hermitian matrix
/// b is replaced with x
template<typename T> EXPORTCSLAB
void hesv(hoNDArray<T>& A, hoNDArray<T>& b);

/// solve Ax=b, a square matrix A and multiple right-hand sides b
/// b is replaced with x
template<typename T> EXPORTCSLAB
void gesv(hoNDArray<T>& A, hoNDArray<T>& b);

/// solve Ax=b with Tikhonov regularization
//template<typename T> EXPORTCSLAB
//void SolveLinearSystem_Tikhonov(hoNDArray<T>& A, hoNDArray<T>& b, hoNDArray<T>& x, double lamda);

/// Computes the LU factorization of a general m-by-n matrix
/// this function is called by general matrix inversion
template<typename T> EXPORTCSLAB 
void getrf(hoNDArray<T>& A, hoNDArray<lapack_int>& ipiv);

/// Computes the inverse of an LU-factored general matrix
template<typename T> EXPORTCSLAB 
void getri(hoNDArray<T>& A);

}
