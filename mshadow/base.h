#ifndef MSHADOW_BASE_H_
#define MSHADOW_BASE_H_
/*!
 * \file base.h
 * \brief definitions of base types, operators, macros functions
 *
 * \author Bing Xu, Tianqi Chen
 */
#include <cmath>
#include <cstdio>
#include <cfloat>
#include <climits>
#include <algorithm>
// macro defintiions
/*!\brief if this macro is define to be 1, mshadow should compile without any of other libs */
#ifndef MSHADOW_STAND_ALONE
#define MSHADOW_STAND_ALONE 1
#endif
/*! \brief whether do padding during allocation */
#ifndef MSHADOW_ALLOC_PAD
#define MSHADOW_ALLOC_PAD true
#endif
/*! 
 * \brief x dimension of data must be bigger pad_size * ratio to be alloced padded memory, otherwise use tide allocation 
 *        for example, if pad_ratio=2, GPU memory alignement size is 32, then we will only allocate padded memory if x dimension > 64
 *        set it to 0 then we will always allocate padded memory
 */
#ifndef MSHADOW_MIN_PAD_RATIO
  #define MSHADOW_MIN_PAD_RATIO 2
#endif

#if MSHADOW_STAND_ALONE
  #define MSHADOW_USE_CBLAS 0
  #define MSHADOW_USE_MKL   0
  #define MSHADOW_USE_CUDA  0
#endif

/*! \brief use CBLAS for CBLAS */
#ifndef MSHADOW_USE_CBLAS
  #define MSHADOW_USE_CBLAS 0
#endif
/*! \brief use MKL for BLAS */
#ifndef MSHADOW_USE_MKL
  #define MSHADOW_USE_MKL   0
#endif
/*!
 * \brief use CUDA support, must ensure that the cuda include path is correct,
 * or directly compile using nvcc
 */
#ifndef MSHADOW_USE_CUDA
  #define MSHADOW_USE_CUDA   1
#endif
/*! \brief use single precition float */
#ifndef MSHADOW_SINGLE_PRECISION
  #define MSHADOW_SINGLE_PRECISION 1
#endif
/*! \brief whether use SSE */
#ifndef MSHADOW_USE_SSE
  #define MSHADOW_USE_SSE 0
#endif
/*! \brief whether use NVML to get dynamic info */
#ifndef MSHADOW_USE_NVML
  #define MSHADOW_USE_NVML 0
#endif
// SSE is conflict with cudacc
#ifdef __CUDACC__
  #undef MSHADOW_USE_SSE
  #define MSHADOW_USE_SSE 0
#endif

#if MSHADOW_USE_CBLAS
extern "C" {
    #include <cblas.h>
}
#elif MSHADOW_USE_MKL
  #include <mkl.h>
  #include <mkl_cblas.h>
  #include <mkl_vsl.h>
  #include <mkl_vsl_functions.h>
#endif

#if MSHADOW_USE_CUDA
  #include <cublas.h>
  #include <curand.h>
#endif

#if MSHADOW_USE_NVML
  #include <nvml.h>
#endif
// --------------------------------
// MSHADOW_XINLINE is used for inlining template code for both CUDA and CPU code.
#ifdef MSHADOW_XINLINE
  #error "MSHADOW_XINLINE must not be defined"
#endif
#ifdef __CUDACC__
  #define MSHADOW_XINLINE inline __attribute__((always_inline)) __device__ __host__
#else
  #define MSHADOW_XINLINE inline __attribute__((always_inline))
#endif
/*! \brief cpu force inline */
#define MSHADOW_CINLINE inline __attribute__((always_inline))

#if defined(__GXX_EXPERIMENTAL_CXX0X) ||\
    defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
  #define MSHADOW_CONSTEXPR constexpr
#else
  #define MSHADOW_CONSTEXPR const
#endif

/*! \brief namespace for mshadow */
namespace mshadow {
/*! \brief buffer size for each random number generator */
const unsigned kRandBufferSize = 1000000;
/*! \brief pi  */
const float kPi = 3.1415926f;
/*! \brief type that will be used for index */
typedef unsigned index_t;
/*! \brief float point type that will be used in default by mshadow */
typedef float default_real_t;

/*! \brief namespace for operators */
namespace op {
// binary operator
/*! \brief mul operator */
struct mul{
  /*! \brief map a, b to result using defined operation */
  template<typename DType>
  MSHADOW_XINLINE static DType Map(DType a, DType b) {
    return a * b;
  }
};
/*! \brief plus operator */
struct plus {
  /*! \brief map a, b to result using defined operation */
  template<typename DType>
  MSHADOW_XINLINE static DType Map(DType a, DType b) {
    return a + b;
  }
};
/*! \brief minus operator */
struct minus {
  /*! \brief map a, b to result using defined operation */
  template<typename DType>
  MSHADOW_XINLINE static DType Map(DType a, DType b) {
    return a - b;
  }
};
/*! \brief divide operator */
struct div {
  /*! \brief map a, b to result using defined operation */
  template<typename DType>
  MSHADOW_XINLINE static DType Map(DType a, DType b) {
    return a / b;
  }
};
/*! \brief get rhs */
struct right {
  /*! \brief map a, b to result using defined operation */
  template<typename DType>
  MSHADOW_XINLINE static DType Map(DType a, DType b) {
    return b;
  }
};
// unary operator/ function: example
// these operators can be defined by user, in the same style as binary and unary operator
// to use, simply write F<op::identity>( src )
/*! \brief identity function that maps a real number to it self */
struct identity{
  /*! \brief map a to result using defined operation */
  template<typename DType>
  MSHADOW_XINLINE static DType Map(DType a) {
    return a;
  }
};
}  // namespace op
/*! \brief namespace for savers */
namespace sv {
/*! \brief save to saver: = */
struct saveto {
  /*! \brief save b to a using save method */
  template<typename DType>
  MSHADOW_XINLINE static void Save(DType &a, DType b) {
    a = b;
  }
  /*! \brief helper constant to use BLAS, alpha */  
  inline static default_real_t AlphaBLAS(void) { return 1.0f; }
  /*! \brief helper constant to use BLAS, beta */
  inline static default_real_t BetaBLAS(void) { return 0.0f; }
  /*! \brief corresponding binary operator type */
  typedef op::right OPType;
};
/*! \brief save to saver: += */
struct plusto {
  /*! \brief save b to a using save method */
  template<typename DType>
  MSHADOW_XINLINE static void Save(DType &a, DType b) {
    a += b;
  }
  /*! \brief helper constant to use BLAS, alpha */
  inline static default_real_t AlphaBLAS(void) { return 1.0f; }
  /*! \brief helper constant to use BLAS, beta */
  inline static default_real_t BetaBLAS(void) { return 1.0f; }
  /*! \brief corresponding binary operator type */
  typedef op::plus OPType;
};
/*! \brief minus to saver: -= */
struct minusto {
  /*! \brief save b to a using save method */
  template<typename DType>
  MSHADOW_XINLINE static void Save(DType &a, DType b) {
    a -= b;
  }
  /*! \brief helper constant to use BLAS, alpha */
  inline static default_real_t AlphaBLAS(void) { return -1.0f; }
  /*! \brief helper constant to use BLAS, beta */
  inline static default_real_t BetaBLAS(void) { return 1.0f; }
  /*! \brief corresponding binary operator type */
  typedef op::minus OPType;
};
/*! \brief multiply to saver: *= */
struct multo {
  /*! \brief save b to a using save method */
  template<typename DType>
  MSHADOW_XINLINE static void Save(DType &a, DType b) {
    a *= b;
  }
  /*! \brief corresponding binary operator type */
  typedef op::mul OPType;
};
/*! \brief divide to saver: /= */
struct divto {
  /*! \brief save b to a using save method */
  template<typename DType>
  MSHADOW_XINLINE static void Save(DType& a, DType b) {
    a /= b;
  }
  /*! \brief corresponding binary operator type */
  typedef op::div OPType;
};
}  // namespace sv
/*! \brief namespace for potential reducer operations */
namespace red {
/*! \brief sum reducer */
struct sum {
  /*! \brief do reduction into dst */
  template<typename DType>
  MSHADOW_XINLINE static void Reduce(volatile DType& dst,  volatile DType src) {
    dst += src;
  }
  /*! 
   *\brief calculate gradient of redres with respect to redsrc,
   * redres: reduced result, redsrc: one of reduction element   
   */
  template<typename DType>
  MSHADOW_XINLINE static DType PartialGrad(DType redres, DType redsrc) {
    return 1;
  }
  template<typename DType>
  MSHADOW_XINLINE static DType InitValue(void) {
    return 0;
  }
};
/*! \brief helper namespace to get the limits */
namespace limits {
  template<typename DType>
  MSHADOW_XINLINE DType MinValue(void);
  template<>
  MSHADOW_XINLINE float MinValue<float>(void) {
    return -FLT_MAX;
  }
  template<>
  MSHADOW_XINLINE double MinValue<double>(void) {
    return -DBL_MAX;
  }
  template<>
  MSHADOW_XINLINE int MinValue<int>(void) {
    return INT_MIN;
  }
}  // namespace limits
/*! \brief maximum reducer */
struct maximum {
  /*! \brief do reduction into dst */
  template<typename DType>
  MSHADOW_XINLINE static void Reduce(volatile DType& dst,  volatile DType src) {
    using std::max;
    dst = max(dst, src);
  }
  /*!
   * \brief calculate gradient of redres with respect to redsrc,
   * redres: reduced result, redsrc: one of reduction element
   */
  template<typename DType>
  MSHADOW_XINLINE static DType PartialGrad(DType redres, DType redsrc) {
    return redres == redsrc ? 1: 0;
  }
  template<typename DType>
  MSHADOW_XINLINE static DType InitValue(void) {
    return limits::MinValue<DType>();
  }
};
}  // namespace red
}  // namespace mshadow
#endif  // MSHADOW_BASE_H_
