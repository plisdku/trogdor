/*--------------------------------------------------------------------------*\
 |                                                                          |
 |  VMLIB                                                                   |
 |                                                                          |
 |  date         : 4 october, 2003                                          |
 |  release      : 5.1                                                      |
 |  release_date : 1999, July 21                                            |
 |  file         : vmlib.hh                                                 |
 |  author       : Enrico Bertolazzi                                        |
 |  email        : enrico.bertolazzi@ing.unitn.it                           |
 |                                                                          |
 |  This program is free software; you can redistribute it and/or modify    |
 |  it under the terms of the GNU General Public License as published by    |
 |  the Free Software Foundation; either version 2, or (at your option)     |
 |  any later version.                                                      |
 |                                                                          |
 |  This program is distributed in the hope that it will be useful,         |
 |  but WITHOUT ANY WARRANTY; without even the implied warranty of          |
 |  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           |
 |  GNU General Public License for more details.                            |
 |                                                                          |
 |  You should have received a copy of the GNU General Public License       |
 |  along with this program; if not, write to the Free Software             |
 |  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.               |
 |                                                                          |
 |  Copyright (C) 1999                                                      |
 |            ___    ____  ___   _   _        ___    ____  ___   _   _      |
 |           /   \  /     /   \  \  /        /   \  /     /   \  \  /       |
 |          /____/ /__   /____/   \/        /____/ /__   /____/   \/        |
 |         /   \  /     /  \      /        /   \  /     /  \      /         |
 |        /____/ /____ /    \    /        /____/ /____ /    \    /          |
 |                                                                          |
 |      Enrico Bertolazzi                                                   |
 |      Dipartimento di Ingegneria                                          |
 |      Meccanica e Strutturale               tel: +39-461-882590           |
 |      Universita` degli Studi di Trento     fax: +39-461-882599           |
 |      Via Mesiano 77                                                      |
 |      I-38050 Trento, Italy        email: enrico.bertolazzi@ing.unitn.it  |
 |                                                                          |
\*--------------------------------------------------------------------------*/

//
// Possible customization
// # define VMLIB_DEBUG
// # define VMLIB_UNROLL
// # define VMLIB_NO_INLINE_HOUSEHOLDER
//

# ifndef VMLIB_HH
# define VMLIB_HH

# include <cmath>

// standard includes I/O
# include <iostream>
# include <iomanip>

// STL lib
# include <algorithm>

/*
// #    #    ##     ####   #####    ####    ####
// ##  ##   #  #   #    #  #    #  #    #  #
// # ## #  #    #  #       #    #  #    #   ####
// #    #  ######  #       #####   #    #       #
// #    #  #    #  #    #  #   #   #    #  #    #
// #    #  #    #   ####   #    #   ####    ####
*/

// loops
# ifndef VMLIB_UNROLL
  # define VMLIB_LOOP(N,DO) \
    { for ( index_type i = 0 ; i < N ; ++i ) { DO ; } }
# else
  # define VMLIB_LOOP(N,DO)                             \
    { index_type i = N ;                                \
      while ( i >= 4 )                                  \
        { --i ; DO ; --i ; DO ; --i ; DO ; --i ; DO ; } \
      while ( i-- > 0 ) { DO ; }                        \
    }
# endif

namespace vmlib_fun {
  static inline float       absval(float       const & a) { return a > 0 ? a : -a ; }
  static inline double      absval(double      const & a) { return a > 0 ? a : -a ; }
  static inline long double absval(long double const & a) { return a > 0 ? a : -a ; }
  static inline int			absval(int		   const & a) { return a > 0 ? a : -a ; }

  static inline float       const & maxval(float       const & a, float       const & b) { return a > b ? a : b ; }
  static inline double      const & maxval(double      const & a, double      const & b) { return a > b ? a : b ; }
  static inline long double const & maxval(long double const & a, long double const & b) { return a > b ? a : b ; }
  static inline int			const & maxval(int		   const & a, int		  const & b) { return a > b ? a : b ; }

  static inline float       const & minval(float       const & a, float       const & b) { return a < b ? a : b ; }
  static inline double      const & minval(double      const & a, double      const & b) { return a < b ? a : b ; }
  static inline long double const & minval(long double const & a, long double const & b) { return a < b ? a : b ; }
  static inline int		    const & minval(int         const & a, int         const & b) { return a < b ? a : b ; }

  using namespace ::std ;
}

namespace vmlib {

  //using namespace ::std ;
  using ::std::istream ;
  using ::std::ostream ;
  using ::std::cin  ;
  using ::std::cout ;
  using ::std::cerr ;
  //using ::std::setw ;
  //using ::std::endl ;

  /*
  // #     # #     # #         ###   ######
  // #     # ##   ## #          #    #     #
  // #     # # # # # #          #    #     #
  // #     # #  #  # #          #    ######
  //  #   #  #     # #          #    #     #
  //   # #   #     # #          #    #     #
  //    #    #     # #######   ###   ######

  //   ####    ####   #    #  #    #   ####   #    #
  //  #    #  #    #  ##  ##  ##  ##  #    #  ##   #
  //  #       #    #  # ## #  # ## #  #    #  # #  #
  //  #       #    #  #    #  #    #  #    #  #  # #
  //  #    #  #    #  #    #  #    #  #    #  #   ##
  //   ####    ####   #    #  #    #   ####   #    #
  */
  

  typedef unsigned            index_type ;
  typedef index_type*         index_pointer;
  typedef const index_type*   index_const_pointer ;
  typedef index_type&         index_reference ;
  typedef const index_type&   index_const_reference ;
  
  # define VMLIB_TYPES(T)                     \
  typedef T                 value_type ;      \
  typedef value_type*       pointer ;         \
  typedef const value_type* const_pointer ;   \
  typedef value_type&       reference ;       \
  typedef const value_type& const_reference
  
  # ifdef VMLIB_DEBUG
    static inline
    void test_ok(bool       ok,
                 char const method[],
                 char const msg[]) {
      using namespace ::std ;
      if ( !ok ) {
        cerr << "\n\nVMLIB in method: ``" << method << ")''"
             << "\nFatal error: " << msg << "\n\n" ;
        exit(0) ;
      }
    }

    static inline
    void test_ok(bool             ok,
                 char const       method[],
                 index_type const i,
                 char       const msg[]) {
      using namespace ::std ;
      if ( !ok ) {
        cerr << "\n\nVMLIB in method: ``" << method << "(" << i << ")''"
             << "\nFatal error: " << msg << "\n\n" ;
        exit(0) ;
      }
    }

    static void check_range(index_type const i,
                            index_type const imax,
                            char       const method[]) {
      using namespace ::std ;
      if ( i >= imax ) {
        cerr << "\n\nVMLIB in method: ``" << method << "[" << i << "]''"
             << "\nFatal error: local index out of range.\n\n" ;
        exit(0) ;
      }
    }
    static void check_drange(index_type const i, index_type const j,
                             index_type const imax,
                             char const method[]) {
      using namespace ::std ;
      if ( i >= imax || j >= imax) {
        cerr << "\n\nVMLIB in method: ``" << method << "(" << i << "," << j << ")''"
             << "\nFatal error: local index out of range.\n\n" ;
        exit(0) ;
      }
    }
  # else
    static inline
    void
    test_ok(bool, char const [], char const []) {}

    static inline
    void
    test_ok(bool, char const [], index_type const, char const []) {}

    static inline
    void
    check_range(index_type const, index_type const, char const []) {}

    static inline
    void
    check_drange(index_type const, index_type const,
                 index_type const, char const []) {}
  # endif

  template <unsigned DIM, typename T> class SVec ;
  template <unsigned DIM, typename T> class SMat ;
  template <unsigned DIM, typename T, typename R> class SVecE ;
  template <unsigned DIM, typename T, typename R> class SMatE ;

  template <unsigned DIM, typename T>
  struct VMLIB_mult {
    VMLIB_TYPES(T) ;
    
    static inline
    void 
    M_mul_V(pointer pr, const_pointer pa, const_pointer b) {
      for ( index_type i = 0 ; i < DIM ; ++i ) {
        value_type bf(0) ;
        const_pointer pb = b ;
        for ( index_type k = 0 ; k < DIM ; ++k )
          bf = bf + *pa++ * *pb++ ;
        *pr++ = bf ;
      }
    }

    static inline
    void
    M_mul_M(pointer pr, const_pointer a, const_pointer b) {
      for ( index_type i = 0 ; i < DIM ; ++i, a += DIM ) {
        const_pointer bb = b ;
        for ( index_type j = 0 ; j < DIM ; ++j) {
          value_type bf(0) ;
          const_pointer pa = a ;
          const_pointer pb = bb++ ;
          for ( index_type k = 0 ; k < DIM ; ++k, pb += DIM )
            bf += *pa++ * *pb ;
          *pr++ = bf ;
        }
      }
    }
  } ;

  template <typename T>
  struct VMLIB_mult<1,T> {
    VMLIB_TYPES(T) ;
    static inline
    void
    M_mul_V(pointer pr, const_pointer pa, const_pointer pb)
    { *pr = *pa * *pb ; }
    
    static inline
    void
    M_mul_M(pointer *pr, const_pointer pa, const_pointer pb)
    { *pr = *pa * *pb ; }
  } ;

  template <typename T>
  struct VMLIB_mult<2,T> {
    VMLIB_TYPES(T) ;
    static inline
    void
    M_mul_V(pointer pr, const_pointer pa, const_pointer pb) {
      pr[0] = pa[0] * pb[0] + pa[1] * pb[1] ;
      pr[1] = pa[2] * pb[0] + pa[3] * pb[1] ;
    }
    
    static inline
    void
    M_mul_M(pointer pr, const_pointer pa, const_pointer pb) {
      *pr++ = pa[0] * pb[0] + pa[1] * pb[2] ;
      *pr++ = pa[0] * pb[1] + pa[1] * pb[3] ;
      *pr++ = pa[2] * pb[0] + pa[3] * pb[2] ;
      *pr   = pa[2] * pb[1] + pa[3] * pb[3] ;
    }
  } ;

  template <typename T>
  struct VMLIB_mult<3,T> {
    VMLIB_TYPES(T) ;
    static inline
    void
    M_mul_V(pointer pr, const_pointer pa, const_pointer pb) {
      *pr++ = pa[0] * pb[0] + pa[1] * pb[1] + pa[2] * pb[2] ; pa += 3 ;
      *pr++ = pa[0] * pb[0] + pa[1] * pb[1] + pa[2] * pb[2] ; pa += 3 ;
      *pr   = pa[0] * pb[0] + pa[1] * pb[1] + pa[2] * pb[2] ;
    }
    
    static inline
    void
    M_mul_M(pointer pr, const_pointer pa, const_pointer pb) {
      for ( unsigned k = 0 ; k < 3 ; ++k ) {
        pr[0] = pa[0] * pb[0] + pa[1] * pb[3] + pa[2] * pb[6] ;
        pr[3] = pa[3] * pb[0] + pa[4] * pb[3] + pa[5] * pb[6] ;
        pr[6] = pa[6] * pb[0] + pa[7] * pb[3] + pa[8] * pb[6] ;
        ++pr ;
        ++pb ;
      }
    }
  } ;

  template <typename T>
  struct VMLIB_mult<4,T> {
    VMLIB_TYPES(T) ;
    static inline
    void
    M_mul_V(pointer pr, const_pointer pa, const_pointer pb) {
      *pr++ = pa[0]*pb[0] + pa[1]*pb[1] + pa[2]*pb[2] + pa[3]*pb[3] ; pa += 4 ;
      *pr++ = pa[0]*pb[0] + pa[1]*pb[1] + pa[2]*pb[2] + pa[3]*pb[3] ; pa += 4 ;
      *pr++ = pa[0]*pb[0] + pa[1]*pb[1] + pa[2]*pb[2] + pa[3]*pb[3] ; pa += 4 ;
      *pr   = pa[0]*pb[0] + pa[1]*pb[1] + pa[2]*pb[2] + pa[3]*pb[3] ;
    }
    
    static inline
    void
    M_mul_M(pointer pr, const_pointer pa, const_pointer pb) {
      for ( unsigned k = 0 ; k < 4 ; ++k ) {
        for ( unsigned i = 0 ; i < 4 ; ++i) {
          pr[0] = pa[0]*pb[0] + pa[1]*pb[4] + pa[2]*pb[8] + pa[3]*pb[12] ;
          pr += 4 ; pa += 4 ;
        }
        pr -= 15 ;
        pa -= 16 ;
        ++pb ;
      }
    }
  } ;

  template <typename T>
  struct VMLIB_mult<5,T> {
    VMLIB_TYPES(T) ;
    static inline
    void
    M_mul_V(pointer pr, const_pointer pa, const_pointer pb) {
      for ( unsigned k = 0 ; k < 5 ; ++k, pa += 5 )
        *pr++ = pa[0]*pb[0]+pa[1]*pb[1]+pa[2]*pb[2]+pa[3]*pb[3]+pa[4]*pb[4];
    }
    
    static inline
    void
    M_mul_M(pointer pr, const_pointer pa, const_pointer pb) {
      for ( unsigned k = 0 ; k < 5 ; ++k ) {
        for ( unsigned i = 0 ; i < 5 ; ++i) {
          pr[0] = pa[0]*pb[0]+pa[1]*pb[5]+pa[2]*pb[10]+pa[3]*pb[15]+pa[4]*pb[20];
          pr += 5 ; pa += 5 ;
        }
        pr -= 24 ;
        pa -= 25 ;
        ++pb ;
      }
    }
  } ;

  template <unsigned DIM, typename T>
  struct VMLIB_solve {
    VMLIB_TYPES(T) ;
    static void LinearSolver(SMat<DIM,T> const & A, SVec<DIM,T> & bx) ;
    static void LinearSolver(SMat<DIM,T> const & A, SMat<DIM,T> & BX) ;
  } ;

  /*
  //  #####  #     #                 #######
  // #     # #     #  ######   ####  #
  // #       #     #  #       #    # #
  //  #####  #     #  #####   #      #####
  //       #  #   #   #       #      #
  // #     #   # #    #       #    # #
  //  #####     #     ######   ####  #######
  */
  
  template <unsigned DIM, typename T, typename A>
  class SVecE {
  public:
    VMLIB_TYPES(T) ;

  private:
    A value ;
    
  public:
    SVecE(A const & a) : value(a) { }
    value_type operator [] (index_type i) const { return value[i]; }
  } ;

  /*
  //  #####  #     #
  // #     # #     #  ######   ####
  // #       #     #  #       #    #
  //  #####  #     #  #####   #
  //       #  #   #   #       #
  // #     #   # #    #       #    #
  //  #####     #     ######   ####
  */

  template <unsigned DIM, typename T>
  class SVec {
  public:
    VMLIB_TYPES(T) ;

    typedef T*                  iterator ;
    typedef const T*            const_iterator ;

    typedef SVec<DIM,T>         vector_type ;
    typedef vector_type*        vector_pointer;
    typedef const vector_type*  vector_const_pointer ;
    typedef vector_type&        vector_reference ;
    typedef const vector_type&  vector_const_reference ;

  private:
    class VListassign {
      index_type idx ;
      pointer    pv ;
    public:

      VListassign(pointer vv, const_reference val) : idx(1), pv(vv)
      { pv[0] = val ; }
    
      ~VListassign(void) {
        if ( idx == 1 ) {
          VMLIB_LOOP(DIM, pv[i] = pv[0]) ;
        } else {
          ::vmlib::test_ok(idx == DIM, "VListassign", "bad initialization list") ;
        }
      }
    
      VListassign & operator , (const_reference val) {
        ::vmlib::test_ok(idx < DIM, "VListassign", "list full") ;
        pv[idx++] = val ;
        return *this ;
      }

    } ;

    value_type _begin[DIM] ;

  public:

    SVec(void) { }
    ~SVec(void) { }

    const_reference operator[] (index_type i) const
    { ::vmlib::check_range(i,DIM,"SVec") ; return _begin[i] ; }

    reference operator[] (index_type i)
    { ::vmlib::check_range(i,DIM,"SVec") ; return _begin[i] ; }

    const_pointer begin(void) const { return _begin ; }
    pointer       begin(void)       { return _begin ; }

    const_pointer end(void)   const { return _begin + DIM ; }
    pointer       end(void)         { return _begin + DIM ; }

    VListassign operator = (const_reference val)
      { return VListassign(_begin, val) ; }
    SVec(vector_const_reference v) { VMLIB_LOOP(DIM, (*this)[i] = v[i]) ; }

    template <typename S>
    explicit SVec(S const & s)
      { VMLIB_LOOP(DIM, (*this)[i] = s) ; }

    template <typename R>
    SVec(SVecE<DIM,T,R> const & e)
      { VMLIB_LOOP(DIM, (*this)[i] = e[i]) ; }

    explicit SVec(const_reference s)
      { VMLIB_LOOP(DIM, (*this)[i] = s) ; }

    explicit SVec(const_reference a, const_reference b) {
      _begin[0] = a ;
      _begin[1] = b ;
    }
    explicit SVec(const_reference a, const_reference b, const_reference c) {
      _begin[0] = a ;
      _begin[1] = b ;
      _begin[2] = c ;
    }
    explicit SVec(const_reference a, const_reference b, const_reference c,
         const_reference d) {
      _begin[0] = a ;
      _begin[1] = b ;
      _begin[2] = c ;
      _begin[3] = d ;
    }
    explicit SVec(const_reference a, const_reference b, const_reference c,
                  const_reference d, const_reference e) {
      _begin[0] = a ;
      _begin[1] = b ;
      _begin[2] = c ;
      _begin[3] = d ;
      _begin[4] = e ;
    }

  # define SVEC_ASSIGN(OP)                                   \
    vector_const_reference                                   \
    operator OP (vector_const_reference v) {                 \
      VMLIB_LOOP(DIM, (*this)[i] OP v[i]) ;                  \
      return *this ;                                         \
    }                                                        \
    template <typename R> inline                             \
    vector_const_reference                                   \
    operator OP (SVecE<DIM,T,R> const & e) {                 \
      VMLIB_LOOP(DIM, (*this)[i] OP e[i]) ;                  \
      return *this ;                                         \
    }                                                        \

    SVEC_ASSIGN(=)
    SVEC_ASSIGN(+=)
    SVEC_ASSIGN(-=)
    SVEC_ASSIGN(*=)
    SVEC_ASSIGN(/=)
    
    vector_const_reference
    operator += (const_reference s)
    { VMLIB_LOOP(DIM, (*this)[i] += s) ; return *this ; }

    vector_const_reference
    operator -= (const_reference s)
    { VMLIB_LOOP(DIM, (*this)[i] -= s) ; return *this ; }

    vector_const_reference
    operator *= (const_reference s)
    { VMLIB_LOOP(DIM, (*this)[i] *= s) ; return *this ; }

    vector_const_reference
    operator /= (const_reference s)
    { VMLIB_LOOP(DIM, (*this)[i] /= s) ; return *this ; }

    vector_const_reference
    operator *= (SMat<DIM,T> const & a) {
      vector_type b(*this) ;
      VMLIB_mult<DIM,T>::M_mul_V(_begin, a.begin(), b.begin()) ;
      return *this ;
    }

    template <typename R>
    vector_const_reference
    operator *= (SMatE<DIM,T,R> const & a) {
      vector_type b(*this) ;
      VMLIB_mult<DIM,T>::M_mul_V(_begin,SMat<DIM,T>(a) . begin(), b.begin()) ;
      return *this ;
    }

    vector_const_reference
    operator /= (SMat<DIM,T> const & a) {
      VMLIB_solve<DIM,T>::LinearSolver(a,*this) ;
      return *this ;
    }

    template <typename R>
    vector_const_reference
    operator /= (SMatE<DIM,T,R> const & a) {
      VMLIB_solve<DIM,T>::LinearSolver(SMat<DIM,T>(a),*this) ;
      return *this ;
    }

  } ;

  /*
  //  #####  #     #
  // #     # #     #  ######   ####
  // #       #     #  #       #    #
  //  #####  #     #  #####   #
  //       #  #   #   #       #
  // #     #   # #    #       #    #
  //  #####     #     ######   ####
  //
  // #    #    ##     ####   #####    ####    ####
  // ##  ##   #  #   #    #  #    #  #    #  #
  // # ## #  #    #  #       #    #  #    #   ####
  // #    #  ######  #       #####   #    #       #
  // #    #  #    #  #    #  #   #   #    #  #    #
  // #    #  #    #   ####   #    #   ####    ####
  */

  # define SVEC_V(OP,OP_V)                                             \
  template<unsigned DIM, typename T, typename A>                       \
  class OP_V {                                                         \
  public:                                                              \
    VMLIB_TYPES(T) ;                                                   \
  private:                                                             \
    A const & a ;                                                      \
  public:                                                              \
    OP_V(A const & aa) : a(aa) { }                                     \
    value_type operator [] (index_type i) const { return OP a[i] ; }   \
  };                                                                   \
                                                                       \
  template<unsigned DIM, typename T> inline                            \
  SVecE<DIM,T,OP_V<DIM,T,SVec<DIM,T> > >                               \
  operator OP (SVec<DIM,T> const & a) {                                \
    typedef OP_V<DIM,T,SVec<DIM,T> > op ;                              \
    return SVecE<DIM,T,op>(op(a));                                     \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename A> inline               \
  SVecE<DIM,T,OP_V<DIM,T,SVecE<DIM,T,A> > >                            \
  operator OP (SVecE<DIM,T,A> const & a) {                             \
    typedef OP_V<DIM,T,SVecE<DIM,T,A> > op ;                           \
    return SVecE<DIM,T,op>(op(a));                                     \
  }

  # define SVEC_VV(OP,V_OP_V)                                          \
  template<unsigned DIM, typename T, typename A, typename B>           \
  class V_OP_V {                                                       \
  public:                                                              \
    VMLIB_TYPES(T) ;                                                   \
  private:                                                             \
    A const & a ;                                                      \
    B const & b ;                                                      \
  public:                                                              \
    V_OP_V(A const & aa, B const & bb) : a(aa), b(bb) { }              \
    value_type operator [] (index_type i) const                        \
      { return a[i] OP b[i] ; }                                        \
  } ;                                                                  \
                                                                       \
  template<unsigned DIM, typename T> inline                            \
  SVecE<DIM,T,V_OP_V<DIM,T,SVec<DIM,T>,SVec<DIM,T> > >                 \
  operator OP (SVec<DIM,T> const & a, SVec<DIM,T> const & b) {         \
    typedef V_OP_V<DIM,T,SVec<DIM,T>,SVec<DIM,T> > op ;                \
    return SVecE<DIM,T,op>(op(a,b)) ;                                  \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename A> inline               \
  SVecE<DIM,T,V_OP_V<DIM,T,SVecE<DIM,T,A>,SVec<DIM,T> > >              \
  operator OP (SVecE<DIM,T,A> const & a, SVec<DIM,T> const & b) {      \
    typedef V_OP_V<DIM,T,SVecE<DIM,T,A>,SVec<DIM,T> > op ;             \
    return SVecE<DIM,T,op>(op(a,b));                                   \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename B> inline               \
  SVecE<DIM,T,V_OP_V<DIM,T,SVec<DIM,T>,SVecE<DIM,T,B> > >              \
  operator OP (SVec<DIM,T> const & a, SVecE<DIM,T,B> const & b) {      \
    typedef V_OP_V<DIM,T,SVec<DIM,T>,SVecE<DIM,T,B> > op ;             \
    return SVecE<DIM,T,op>(op(a,b));                                   \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename A, typename B> inline   \
  SVecE<DIM,T,V_OP_V<DIM,T,SVecE<DIM,T,A>, SVecE<DIM,T,B> > >          \
  operator OP (SVecE<DIM,T,A> const & a, SVecE<DIM,T,B> const & b) {   \
    typedef V_OP_V<DIM,T,SVecE<DIM,T,A>,SVecE<DIM,T,B> > op ;          \
    return SVecE<DIM,T,op>(op(a,b));                                   \
  }                                                                    \

  # define SVEC_SV(OP,S_OP_V)                                          \
  template<unsigned DIM, typename T, typename S, typename B>           \
  class S_OP_V {                                                       \
  public:                                                              \
    VMLIB_TYPES(T) ;                                                   \
  private:                                                             \
    S const & a ;                                                      \
    B const & b ;                                                      \
  public:                                                              \
    S_OP_V(S const & aa, B const & bb) : a(aa), b(bb) { }              \
    value_type operator [] (index_type i) const                        \
      { return a OP b[i] ; }                                           \
  } ;                                                                  \
                                                                       \
  template <unsigned DIM, typename T, typename S> inline               \
  SVecE<DIM,T,S_OP_V<DIM,T,S,SVec<DIM,T> > >                           \
  operator OP (S const & s, SVec<DIM,T> const & v) {                   \
    typedef S_OP_V<DIM,T,S,SVec<DIM,T> > op ;                          \
    return SVecE<DIM,T,op>(op(s,v));                                   \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename S, typename A> inline   \
  SVecE<DIM,T,S_OP_V<DIM,T,S,SVecE<DIM,T,A> > >                        \
  operator OP (S const & s, SVecE<DIM,T,A> const & v) {                \
    typedef S_OP_V<DIM,T,S,SVecE<DIM,T,A> > op ;                       \
    return SVecE<DIM,T,op>(op(s,v));                                   \
  }                                                                    \

  # define SVEC_VS(OP,V_OP_S)                                          \
  template<unsigned DIM, typename T, typename S, typename A>           \
  class V_OP_S {                                                       \
  public:                                                              \
    VMLIB_TYPES(T) ;                                                   \
  private:                                                             \
    A const & a ;                                                      \
    S const & b ;                                                      \
  public:                                                              \
    V_OP_S(A const & aa, S const & bb) : a(aa), b(bb) { }              \
    value_type operator [] (index_type i) const                        \
      { return a[i] OP b ; }                                           \
  } ;                                                                  \
                                                                       \
  template <unsigned DIM, typename T, typename S> inline               \
  SVecE<DIM,T,V_OP_S<DIM,T,S,SVec<DIM,T> > >                           \
  operator OP (SVec<DIM,T> const & v, S const & s) {                   \
    typedef V_OP_S<DIM,T,S,SVec<DIM,T> > op ;                          \
    return SVecE<DIM,T,op>(op(v,s)) ;                                  \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename S, typename A> inline   \
  SVecE<DIM,T, V_OP_S<DIM,T,S,SVecE<DIM,T,A> > >                       \
  operator OP (SVecE<DIM,T,A> const & v, S const & s) {                \
    typedef V_OP_S<DIM,T,S,SVecE<DIM,T,A> > op ;                       \
    return SVecE<DIM,T,op>(op(v,s)) ;                                  \
  }                                                                    \


  SVEC_V(-,SVec_neg_V)

  SVEC_VV(+,SVec_V_sum_V)
  SVEC_VV(-,SVec_V_sub_V)
  SVEC_VV(*,SVec_V_mul_V)
  SVEC_VV(/,SVec_V_div_V)

  SVEC_SV(+,SVec_S_sum_V)
  SVEC_SV(-,SVec_S_sub_V)
  SVEC_SV(*,SVec_S_mul_V)
  SVEC_SV(/,SVec_S_div_V)

  SVEC_VS(+,SVec_V_sum_S)
  SVEC_VS(-,SVec_V_sub_S)
  SVEC_VS(*,SVec_V_mul_S)
  SVEC_VS(/,SVec_V_div_S)
  
  # define SVEC_F_V(FUN)                                               \
  template<unsigned DIM, typename T, typename A>                       \
  class SVec_##FUN {                                                   \
  public:                                                              \
    VMLIB_TYPES(T) ;                                                   \
  private:                                                             \
    A const & a ;                                                      \
  public:                                                              \
    SVec_##FUN(A const & aa) : a(aa) { }                               \
    value_type operator [] (index_type i) const                        \
      { return ::vmlib_fun::FUN(a[i]) ; }                              \
  } ;

  # define SVEC_F_V_TREE(FUN)                                          \
  template<unsigned DIM, typename T> inline                            \
  SVecE<DIM,T,SVec_##FUN<DIM,T,SVec<DIM,T> > >                         \
  FUN(SVec<DIM,T> const & a) {                                         \
    typedef SVec_##FUN<DIM,T,SVec<DIM,T> > op ;                        \
    return SVecE<DIM,T,op>(op(a));                                     \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename A> inline               \
  SVecE<DIM,T,SVec_##FUN<DIM,T,SVecE<DIM,T,A> > >                      \
  FUN(SVecE<DIM,T,A> const & a) {                                      \
    typedef SVec_##FUN<DIM,T,SVecE<DIM,T,A> > op ;                     \
    return SVecE<DIM,T,op>(op(a));                                     \
  }                                                                    \

  # define SVEC_F_VV(FUN)                                              \
  template<unsigned DIM, typename T, typename A, typename B>           \
  class SVec_##FUN {                                                   \
  public:                                                              \
    VMLIB_TYPES(T) ;                                                   \
  private:                                                             \
    A const & a ;                                                      \
    B const & b ;                                                      \
  public:                                                              \
    SVec_##FUN(A const & aa, B const & bb) : a(aa), b(bb) { }          \
    value_type operator [] (index_type i) const                        \
      { return ::vmlib_fun::FUN(a[i], b[i]) ; }                        \
  } ;

  # define SVEC_F_VV_TREE(FUN)                                         \
  template<unsigned DIM, typename T> inline                            \
  SVecE<DIM,T,SVec_##FUN<DIM,T,SVec<DIM,T>,SVec<DIM,T> > >             \
  FUN(SVec<DIM,T> const & a, SVec<DIM,T> const & b) {                  \
    typedef SVec_##FUN<DIM,T,SVec<DIM,T>,SVec<DIM,T> > op ;            \
    return SVecE<DIM,T,op>(op(a,b)) ;                                  \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename A> inline               \
  SVecE<DIM,T,SVec_##FUN<DIM,T,SVecE<DIM,T,A>,SVec<DIM,T> > >          \
  FUN(SVecE<DIM,T,A> const & a, SVec<DIM,T> const & b) {               \
    typedef SVec_##FUN<DIM,T,SVecE<DIM,T,A>,SVec<DIM,T> > op ;         \
    return SVecE<DIM,T,op>(op(a,b));                                   \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename B> inline               \
  SVecE<DIM,T,SVec_##FUN<DIM,T,SVec<DIM,T>,SVecE<DIM,T,B> > >          \
  FUN(SVec<DIM,T> const & a, SVecE<DIM,T,B> const & b) {               \
    typedef SVec_##FUN<DIM,T,SVec<DIM,T>,SVecE<DIM,T,B> > op ;         \
    return SVecE<DIM,T,op>(op(a,b));                                   \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename A, typename B> inline   \
  SVecE<DIM,T,SVec_##FUN<DIM,T,SVecE<DIM,T,A>, SVecE<DIM,T,B> > >      \
  FUN(SVecE<DIM,T,A> const & a, SVecE<DIM,T,B> const & b) {            \
    typedef SVec_##FUN<DIM,T,SVecE<DIM,T,A>,SVecE<DIM,T,B> > op ;      \
    return SVecE<DIM,T,op>(op(a,b));                                   \
  }

  SVEC_F_V(absval)
  SVEC_F_V(sin)
  SVEC_F_V(cos)
  SVEC_F_V(tan)
  SVEC_F_V(asin)
  SVEC_F_V(acos)
  SVEC_F_V(atan)
  SVEC_F_V(cosh)
  SVEC_F_V(sinh)
  SVEC_F_V(tanh)

  SVEC_F_V(sqrt)
  SVEC_F_V(ceil)
  SVEC_F_V(floor)
  SVEC_F_V(exp)
  SVEC_F_V(log)
  SVEC_F_V(log10)

  SVEC_F_VV(pow)
  SVEC_F_VV(atan2)
  SVEC_F_VV(maxval)
  SVEC_F_VV(minval)

  /*
  //
  //  #####  #     #                 #######    #
  // #     # #     #  ######   ####  #         ##
  // #       #     #  #       #    # #        # #
  //  #####  #     #  #####   #      #####      #
  //       #  #   #   #       #      #          #
  // #     #   # #    #       #    # #          #
  //  #####     #     ######   ####  #        #####
  */

  template<unsigned DIM, typename T, typename A>
  struct F1 {

    VMLIB_TYPES(T) ;
    
    static inline
    value_type sum(A const & a) {
      value_type res(0) ;
      VMLIB_LOOP( DIM, res += a[i] ) ; 
      return res ;
    }

    static inline
    value_type norm1(A const & a) {
      value_type res(0) ;
      VMLIB_LOOP( DIM, res += ::vmlib_fun::absval(a[i]) ) ; 
      return res ;
    }

    static inline
    value_type normi(A const & a) {
      value_type res(0) ;
      VMLIB_LOOP( DIM, value_type bf = ::vmlib_fun::absval(a[i]) ; if ( bf > res ) res = bf) ;
      return res ;
    }

    static inline
    value_type norm2(A const & a) {
      value_type res(0) ;
      VMLIB_LOOP( DIM, res = res + a[i] * a[i]) ;
      return ::vmlib_fun::sqrt(res) ;
    }
  
    static inline
    value_type normp(A const & a, const_reference p) {
      value_type res(0) ;
      VMLIB_LOOP( DIM, res = res + ::vmlib_fun::pow(::vmlib_fun::absval(a[i]),p) ) ;
      return ::vmlib_fun::pow(res,1/p) ;
    }

    static inline
    value_type maxval(A const & a) {
      value_type res = a[0] ;
      VMLIB_LOOP( DIM, if ( a[i] > res ) res = a[i] ) ;
      return res ;
    }

    static inline
    value_type minval(A const & a) {
      value_type res = a[0] ;
      VMLIB_LOOP( DIM, if ( a[i] < res ) res = a[i] ) ;
      return res ;
    }

  } ;

  /*
  //
  //  #####  #     #                 #######  #####
  // #     # #     #  ######   ####  #       #     #
  // #       #     #  #       #    # #             #
  //  #####  #     #  #####   #      #####    #####
  //       #  #   #   #       #      #       #
  // #     #   # #    #       #    # #       #
  //  #####     #     ######   ####  #       #######
  */

  template<unsigned DIM, typename T, typename A, typename B>
  struct F2 {
    VMLIB_TYPES(T) ;

    static inline
    value_type dot(A const & a, B const & b) {
      value_type res(0) ;
      VMLIB_LOOP( DIM, res += a[i] * b[i]) ;
      return res ;
    }

    static inline
    value_type dot_div(A const & a, B const & b) {
      value_type res(0) ;
      VMLIB_LOOP( DIM, res += a[i] / b[i]) ;
      return res ;
    }

    static inline
    value_type dist2(A const & a, B const & b) {
      value_type res(0) ;
      VMLIB_LOOP( DIM, value_type bf = a[i] - b[i] ; res += bf * bf) ;
      return res ;
    }

    static inline
    value_type dist(A const & a, B const & b) {
      value_type res(0) ;
      VMLIB_LOOP( DIM, value_type bf = a[i] - b[i] ; res += bf * bf) ;
      return ::vmlib_fun::sqrt(res) ;
    }

  } ;
  
  SVEC_F_V_TREE(absval)
  SVEC_F_V_TREE(sin)
  SVEC_F_V_TREE(cos)
  SVEC_F_V_TREE(tan)
  SVEC_F_V_TREE(asin)
  SVEC_F_V_TREE(acos)
  SVEC_F_V_TREE(atan)
  SVEC_F_V_TREE(cosh)
  SVEC_F_V_TREE(sinh)
  SVEC_F_V_TREE(tanh)

  SVEC_F_V_TREE(sqrt)
  SVEC_F_V_TREE(ceil)
  SVEC_F_V_TREE(floor)
  SVEC_F_V_TREE(exp)
  SVEC_F_V_TREE(log)
  SVEC_F_V_TREE(log10)

  SVEC_F_VV_TREE(pow)
  SVEC_F_VV_TREE(atan2)
  SVEC_F_VV_TREE(maxval)
  SVEC_F_VV_TREE(minval)

  # define VMLIB_UNARY_FUN(FUN)                                           \
  template <unsigned DIM, typename T> inline                              \
  T FUN(SVec<DIM,T> const & v)                                            \
  { return F1<DIM,T,SVec<DIM,T> >::FUN(v) ; }                             \
                                                                          \
  template <unsigned DIM, typename T, typename A> inline                  \
  T FUN(SVecE<DIM,T,A> const & v)                                         \
  { return F1<DIM,T,SVecE<DIM,T,A> >::FUN(v) ; }

  # define VMLIB_BINARY_FUN(FUN)                                          \
  template <unsigned DIM, typename T> inline                              \
  T FUN(SVec<DIM,T> const & a, SVec<DIM,T> const & b)                     \
  { return F2<DIM,T,SVec<DIM,T>,SVec<DIM,T> >::FUN(a,b) ; }               \
                                                                          \
  template <unsigned DIM, typename T, typename A> inline                  \
  T FUN(SVecE<DIM,T,A> const & a, SVec<DIM,T> const & b)                  \
  { return F2<DIM,T,SVecE<DIM,T,A>,SVec<DIM,T> >::FUN(a,b) ; }            \
                                                                          \
  template <unsigned DIM, typename T, typename A> inline                  \
  T FUN(SVec<DIM,T> const & a, SVecE<DIM,T,A> const & b)                  \
  { return F2<DIM,T,SVec<DIM,T>,SVecE<DIM,T,A> >::FUN(a,b) ; }            \
                                                                          \
  template <unsigned DIM, typename T, typename A, typename B>             \
  inline                                                                  \
  T FUN(SVecE<DIM,T,A> const & a, SVecE<DIM,T,B> const & b)               \
  { return F2<DIM,T,SVecE<DIM,T,A>,SVecE<DIM,T,B> >::FUN(a,b) ; }

  // S U M
  VMLIB_UNARY_FUN(sum)

  // N O R M 1
  VMLIB_UNARY_FUN(norm1)

  // N O R M 2
  VMLIB_UNARY_FUN(norm2)

  // N O R M I
  VMLIB_UNARY_FUN(normi)

  // N O R M P
  template <unsigned DIM, typename T, typename S> inline
  T normp(SVec<DIM,T> const & v, S const & p)
  { return F1<DIM,T,SVec<DIM,T> >::normp(v,T(p)) ; }

  template <unsigned DIM, typename T, typename A, typename S> inline
  T normp(SVecE<DIM,T,A> const & v, S const & p)
  { return F1<DIM,T,SVecE<DIM,T,A> >::normp(v,T(p)) ; }

  // M A X
  VMLIB_UNARY_FUN(maxval)

  // M I N
  VMLIB_UNARY_FUN(minval)

  // D O T
  VMLIB_BINARY_FUN(dot)

  // D O T _ D I V
  VMLIB_BINARY_FUN(dot_div)

  // D I S T
  VMLIB_BINARY_FUN(dist)

  // D I S T 2
  VMLIB_BINARY_FUN(dist2)

  /*
  //  #####  #     #                 #######
  // #     # ##   ##    ##     ##### #
  // #       # # # #   #  #      #   #
  //  #####  #  #  #  #    #     #   #####
  //       # #     #  ######     #   #
  // #     # #     #  #    #     #   #
  //  #####  #     #  #    #     #   #######
  */

  template <unsigned DIM, typename T, typename A>
  class SMatE {
    A value ;
  public:
    VMLIB_TYPES(T) ;
    SMatE(A const & a) : value(a) { }
    value_type operator [] (index_type i) const { return value[i]; }
  } ;

  /*
  //  #####  #     #
  // #     # ##   ##    ##     #####
  // #       # # # #   #  #      #
  //  #####  #  #  #  #    #     #
  //       # #     #  ######     #
  // #     # #     #  #    #     #
  //  #####  #     #  #    #     #
  */

  template <unsigned DIM, typename T>
  class SMat {
  public: 
    VMLIB_TYPES(T) ;

    typedef T*                  iterator ;
    typedef const T*            const_iterator ;

    typedef SMat<DIM,T>         matrix_type ;
    typedef matrix_type*        matrix_pointer;
    typedef const matrix_type*  matrix_const_pointer ;
    typedef matrix_type&        matrix_reference ;
    typedef const matrix_type&  matrix_const_reference ;
    
    static unsigned const SIZE = DIM * DIM ;

  private: 

    class MListassign {
      index_type idx ;
      pointer pv ;
    public:

      MListassign(pointer vv, const_reference val) : idx(1), pv(vv) {
        pv[0] = val ;
      }
    
      ~MListassign(void) {
        if ( idx == 1 ) {
          value_type bf = pv[0] ;
          VMLIB_LOOP(DIM * DIM, pv[i] = 0) ;
          VMLIB_LOOP(DIM,  pv[i*(DIM+1)] = bf) ;
        } else {
          ::vmlib::test_ok(idx == DIM * DIM, "MListassign", "bad initialization list") ;
        }
      }
    
      MListassign & operator , (const_reference val) {
        ::vmlib::test_ok(idx < DIM * DIM, "MListassign", "list full") ;
        pv[idx++] = val ;
        return *this ;
      }

    } ;
    
    static inline index_type addr(index_type i, index_type j) { return i*DIM+j ;}

    value_type _begin[SIZE] ;

  public: 

    SMat(void) { }
    ~SMat(void) { }

    const_reference operator() (index_type i, index_type j) const
    { ::vmlib::check_drange(i,j,DIM,"SMat") ; return _begin[addr(i,j)] ; }
    
    reference operator() (index_type i, index_type j)
    { ::vmlib::check_drange(i,j,DIM,"SMat") ; return _begin[addr(i,j)] ; }
    
    const_reference operator [] (index_type i) const
    { ::vmlib::check_range(i,SIZE,"SMat") ; return _begin[i] ; }

    reference operator [] (index_type i)
    { ::vmlib::check_range(i,SIZE,"SMat") ; return _begin[i] ; }

    const_pointer begin(void) const { return _begin ; }
    pointer       begin(void)       { return _begin ; }

    const_pointer end(void) const   { return _begin + SIZE ; }
    pointer       end(void)         { return _begin + SIZE ; }

    MListassign operator = (T const & val)
      { return MListassign(_begin, val) ; }
    
    explicit SMat(const_reference s) {
      VMLIB_LOOP(SIZE, (*this)[i] = 0 ) ;
      VMLIB_LOOP(DIM,  (*this)[i*(DIM+1)] = s) ;
    }

    SMat(matrix_const_reference v)
      { VMLIB_LOOP(SIZE, (*this)[i] = v[i]) ; }

    template <typename R> inline
    SMat(SMatE<DIM,T,R> const & e)
      { VMLIB_LOOP(SIZE, (*this)[i] = e[i]) ; }

  # define SMAT_ASSIGN(OP)                                   \
    matrix_const_reference                                   \
    operator OP (matrix_const_reference v) {                 \
      VMLIB_LOOP(SIZE, (*this)[i] OP v[i]) ;                 \
      return *this ;                                         \
    }                                                        \
    template <typename R> inline                             \
    matrix_const_reference                                   \
    operator OP (SMatE<DIM,T,R> const & e) {                 \
      VMLIB_LOOP(SIZE, (*this)[i] OP e[i]) ;                 \
      return *this ;                                         \
    }                                                        \

    SMAT_ASSIGN(=)
    SMAT_ASSIGN(+=)
    SMAT_ASSIGN(-=)

    matrix_const_reference
    operator *= (SMat<DIM,T> const & a) {
      matrix_type b = *this ;
      VMLIB_mult<DIM,T>::M_mul_M(_begin, a.begin(), b.begin()) ;
      return *this ;
    }

    template <typename R>
    matrix_const_reference
    operator *= (SMatE<DIM,T,R> const & a) {
      matrix_type b = *this ;
      VMLIB_mult<DIM,T>::M_mul_M(_begin, SMat<DIM,T>(a).begin(), b.begin()) ;
      return *this ;
    }

    matrix_const_reference
    operator /= (SMat<DIM,T> const & a) {
      VMLIB_solve<DIM,T>::LinearSolver(a,*this) ;
      return *this ;
    }

    template <typename R>
    matrix_const_reference
    operator /= (SMatE<DIM,T,R> const & a) {
      VMLIB_solve<DIM,T>::LinearSolver(SMat<DIM,T>(a),*this) ;
      return *this ;
    }
    
    template <typename S> inline
    matrix_const_reference
    operator += (S const & s)
    { VMLIB_LOOP(DIM, (*this)[i*(DIM+1)] += s) ; return *this ; }

    template <typename S> inline
    matrix_const_reference
    operator -= (S const & s)
    { VMLIB_LOOP(DIM, (*this)[i*(DIM+1)] -= s) ; return *this ; }

    template <typename S> inline
    matrix_const_reference
    operator *= (S const & s)
    { VMLIB_LOOP(SIZE, (*this)[i] *= s) ; return *this ; }

    template <typename S> inline
    matrix_const_reference
    operator /= (S const & s)
    { VMLIB_LOOP(SIZE, (*this)[i] /= s) ; return *this ; }

  } ;

  /*
  //  #####  #     #
  // #     # ##   ##    ##     #####
  // #       # # # #   #  #      #
  //  #####  #  #  #  #    #     #
  //       # #     #  ######     #
  // #     # #     #  #    #     #
  //  #####  #     #  #    #     #
  //
  //  #    #    ##     ####   #####    ####    ####
  //  ##  ##   #  #   #    #  #    #  #    #  #
  //  # ## #  #    #  #       #    #  #    #   ####
  //  #    #  ######  #       #####   #    #       #
  //  #    #  #    #  #    #  #   #   #    #  #    #
  //  #    #  #    #   ####   #    #   ####    ####
  */

  # define SMAT_UNARY(OP,OP_M)                                         \
  template<unsigned DIM, typename T, typename A>                       \
  class OP_M {                                                         \
  public:                                                              \
    VMLIB_TYPES(T) ;                                                   \
  private:                                                             \
    A const & a ;                                                      \
  public:                                                              \
    OP_M(A const & aa) : a(aa) { }                                     \
    value_type operator [] (index_type i) const { return OP a[i] ; }   \
  };                                                                   \
                                                                       \
  template<unsigned DIM, typename T> inline                            \
  SMatE<DIM,T,OP_M<DIM,T,SMat<DIM,T> > >                               \
  operator OP (SMat<DIM,T> const & a) {                                \
    typedef OP_M<DIM, T, SMat<DIM,T> > op ;                            \
    return SMatE<DIM,T,op>(op(a));                                     \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename A> inline               \
  SMatE<DIM,T,OP_M<DIM,T,SMatE<DIM,T,A> > >                            \
  operator OP (SMatE<DIM,T,A> const & a) {                             \
    typedef OP_M<DIM,T,SMatE<DIM,T,A> > op ;                           \
    return SMatE<DIM,T,op>(op(a));                                     \
  }

  # define SMAT_BINARY(OP,M_OP_M)                                      \
  template<unsigned DIM, typename T, typename A, typename B>           \
  class M_OP_M {                                                       \
  public:                                                              \
    VMLIB_TYPES(T) ;                                                   \
  private:                                                             \
    A const & a ;                                                      \
    B const & b ;                                                      \
  public:                                                              \
    M_OP_M(A const & aa, B const & bb) : a(aa), b(bb) { }              \
    value_type operator [] (index_type i) const                        \
      { return a[i] OP b[i] ; }                                        \
  } ;                                                                  \
                                                                       \
  template<unsigned DIM, typename T> inline                            \
  SMatE<DIM,T,M_OP_M<DIM,T,SMat<DIM,T>,SMat<DIM,T> > >                 \
  operator OP (SMat<DIM,T> const & a, SMat<DIM,T> const & b) {         \
    typedef M_OP_M<DIM,T,SMat<DIM,T>,SMat<DIM,T> > op ;                \
    return SMatE<DIM,T,op>(op(a,b)) ;                                  \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename A> inline               \
  SMatE<DIM,T,M_OP_M<DIM,T,SMatE<DIM,T,A>,SMat<DIM,T> > >              \
  operator OP (SMatE<DIM,T,A> const & a, SMat<DIM,T> const & b) {      \
    typedef M_OP_M<DIM,T,SMatE<DIM,T,A>,SMat<DIM,T> > op ;             \
    return SMatE<DIM,T,op>(op(a,b));                                   \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename B> inline               \
  SMatE<DIM,T,M_OP_M<DIM,T,SMat<DIM,T>,SMatE<DIM,T,B> > >              \
  operator OP (SMat<DIM,T> const & a, SMatE<DIM,T,B> const & b) {      \
    typedef M_OP_M<DIM,T,SMat<DIM,T>,SMatE<DIM,T,B> > op ;             \
    return SMatE<DIM,T,op>(op(a,b));                                   \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename A, typename B> inline   \
  SMatE<DIM,T,M_OP_M<DIM,T,SMatE<DIM,T,A>, SMatE<DIM,T,B> > >          \
  operator OP (SMatE<DIM,T,A> const & a, SMatE<DIM,T,B> const & b) {   \
    typedef M_OP_M<DIM,T,SMatE<DIM,T,A>,SMatE<DIM,T,B> > op ;          \
    return SMatE<DIM,T,op>(op(a,b));                                   \
  }

  # define SMAT_BINARY_SM(OP,S_OP_M)                                   \
  template <unsigned DIM, typename T, typename S, typename B>          \
  class S_OP_M {                                                       \
  public:                                                              \
    VMLIB_TYPES(T) ;                                                   \
  private:                                                             \
    S const & a ;                                                      \
    B const & b ;                                                      \
  public:                                                              \
    S_OP_M(S const & aa, B const & bb) : a(aa), b(bb) { }              \
    value_type operator [] (index_type i) const                        \
      { return ((i%(DIM+1))==0 ? a : 0) OP b[i] ; }                    \
  } ;                                                                  \
                                                                       \
  template <unsigned DIM, typename T, typename S> inline               \
  SMatE<DIM,T,S_OP_M<DIM,T,S,SMat<DIM,T> > >                           \
  operator OP (S const & s, SMat<DIM,T> const & v) {                   \
    typedef S_OP_M<DIM,T,S,SMat<DIM,T> > op ;                          \
    return SMatE<DIM,T,op>(op(s,v));                                   \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename S, typename A> inline   \
  SMatE<DIM,T,S_OP_M<DIM,T,S,SMatE<DIM,T,A> > >                        \
  operator OP (S const & s, SMatE<DIM,T,A> const & v) {                \
    typedef S_OP_M<DIM,T,S,SMatE<DIM,T,A> > op ;                       \
    return SMatE<DIM,T,op>(op(s,v));                                   \
  }                                                                    \

  # define SMAT_BINARY_MS(OP,M_OP_S)                                   \
  template <unsigned DIM, typename T, typename S, typename A>          \
  class M_OP_S {                                                       \
  public:                                                              \
    VMLIB_TYPES(T) ;                                                   \
  private:                                                             \
    A const & a ;                                                      \
    S const & b ;                                                      \
  public:                                                              \
    M_OP_S(A const & aa, S const & bb) : a(aa), b(bb) { }              \
    value_type operator [] (index_type i) const                        \
      { return a[i] OP ((i%(DIM+1))==0 ? b : 0) ; }                    \
  } ;                                                                  \
                                                                       \
  template <unsigned DIM, typename T, typename S> inline               \
  SMatE<DIM,T,M_OP_S<DIM,T,S,SMat<DIM,T> > >                           \
  operator OP (SMat<DIM,T> const & v, S const & s) {                   \
    typedef M_OP_S<DIM,T,S,SMat<DIM,T> > op ;                          \
    return SMatE<DIM,T,op>(op(v,s)) ;                                  \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename S, typename A> inline   \
  SMatE<DIM,T, M_OP_S<DIM,T,S,SMatE<DIM,T,A> > >                       \
  operator OP (SMatE<DIM,T,A> const & v, S const & s) {                \
    typedef M_OP_S<DIM,T,S,SMatE<DIM,T,A> > op ;                       \
    return SMatE<DIM,T,op>(op(v,s)) ;                                  \
  }                                                                    \

  # define SMAT_BINARY_FSM(OP,S_OP_M)                                  \
  template <unsigned DIM, typename T, typename S, typename B>          \
  class S_OP_M {                                                       \
  public:                                                              \
    VMLIB_TYPES(T) ;                                                   \
  private:                                                             \
    S const & a ;                                                      \
    B const & b ;                                                      \
  public:                                                              \
    S_OP_M(S const & aa, B const & bb) : a(aa), b(bb) { }              \
    value_type operator [] (index_type i) const                        \
      { return a OP b[i] ; }                                           \
  } ;                                                                  \
                                                                       \
  template <unsigned DIM, typename T, typename S> inline               \
  SMatE<DIM,T,S_OP_M<DIM,T,S,SMat<DIM,T> > >                           \
  operator OP (S const & s, SMat<DIM,T> const & v) {                   \
    typedef S_OP_M<DIM,T,S,SMat<DIM,T> > op ;                          \
    return SMatE<DIM,T,op>(op(s,v));                                   \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename S, typename A> inline   \
  SMatE<DIM,T,S_OP_M<DIM,T,S,SMatE<DIM,T,A> > >                        \
  operator OP (S const & s, SMatE<DIM,T,A> const & v) {                \
    typedef S_OP_M<DIM,T,S,SMatE<DIM,T,A> > op ;                       \
    return SMatE<DIM,T,op>(op(s,v));                                   \
  }                                                                    \

  # define SMAT_BINARY_MFS(OP,M_OP_S)                                  \
  template <unsigned DIM, typename T, typename S, typename A>          \
  class M_OP_S {                                                       \
  public:                                                              \
    VMLIB_TYPES(T) ;                                                   \
  private:                                                             \
    A const & a ;                                                      \
    S const & b ;                                                      \
  public:                                                              \
    M_OP_S(A const & aa, S const & bb) : a(aa), b(bb) { }              \
    value_type operator [] (index_type i) const                        \
      { return a[i] OP b ; }                                           \
  } ;                                                                  \
                                                                       \
  template <unsigned DIM, typename T, typename S> inline               \
  SMatE<DIM,T,M_OP_S<DIM,T,S,SMat<DIM,T> > >                           \
  operator OP (SMat<DIM,T> const & v, S const & s) {                   \
    typedef M_OP_S<DIM,T,S,SMat<DIM,T> > op ;                          \
    return SMatE<DIM,T,op>(op(v,s)) ;                                  \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename S, typename A> inline   \
  SMatE<DIM,T, M_OP_S<DIM,T,S,SMatE<DIM,T,A> > >                       \
  operator OP (SMatE<DIM,T,A> const & v, S const & s) {                \
    typedef M_OP_S<DIM,T,S,SMatE<DIM,T,A> > op ;                       \
    return SMatE<DIM,T,op>(op(v,s)) ;                                  \
  }                                                                    \


  SMAT_UNARY(-,SMat_neg_M)

  SMAT_BINARY(+,SMat_M_sum_M)
  SMAT_BINARY(-,SMat_M_sub_M)

  SMAT_BINARY_SM(+,SMat_S_sum_M)
  SMAT_BINARY_SM(-,SMat_S_sub_M)

  SMAT_BINARY_MS(+,SMat_M_sum_S)
  SMAT_BINARY_MS(-,SMat_M_sub_S)

  SMAT_BINARY_FSM(*,SMat_S_mul_M)
  SMAT_BINARY_MFS(*,SMat_M_mul_S)
  SMAT_BINARY_MFS(/,SMat_M_div_S)



  # define SMAT_FUN1(FUN)                                              \
  template <unsigned DIM, typename T, typename A>                      \
  class SMat_##FUN {                                                   \
  public:                                                              \
    VMLIB_TYPES(T) ;                                                   \
  private:                                                             \
    A const & a ;                                                      \
  public:                                                              \
    SMat_##FUN(A const & aa) : a(aa) { }                               \
    value_type operator [] (index_type i) const                        \
      { return ::vmlib_fun::FUN(a[i]) ; }                              \
  };
  
  # define SMAT_FUN1_TREE(FUN)                                         \
  template<unsigned DIM, typename T> inline                            \
  SMatE<DIM,T,SMat_##FUN<DIM,T,SMat<DIM,T> > >                         \
  FUN(SMat<DIM,T> const & a) {                                         \
    typedef SMat_##FUN<DIM,T,SMat<DIM,T> > op ;                        \
    return SMatE<DIM,T,op>(op(a));                                     \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename A> inline               \
  SMatE<DIM,T,SMat_##FUN<DIM,T,SMatE<DIM,T,A> > >                      \
  FUN(SMatE<DIM,T,A> const & a) {                                      \
    typedef SMat_##FUN<DIM,T,SMatE<DIM,T,A> > op ;                     \
    return SMatE<DIM,T,op>(op(a));                                     \
  }

  # define SMAT_FUN2(FUN)                                              \
  template<unsigned DIM, typename T, typename A, typename B>           \
  class SMat_##FUN {                                                   \
  public:                                                              \
    VMLIB_TYPES(T) ;                                                   \
  private:                                                             \
    A const & a ;                                                      \
    B const & b ;                                                      \
  public:                                                              \
    SMat_##FUN(A const & aa, B const & bb) : a(aa), b(bb) { }          \
    value_type operator [] (index_type i) const                        \
      { return ::vmlib_fun::FUN(a[i], b[i]) ; }                        \
  } ;
  
  # define SMAT_FUN2_TREE(FUN)                                         \
  template<unsigned DIM, typename T> inline                            \
  SMatE<DIM,T,SMat_##FUN<DIM,T,SMat<DIM,T>,SMat<DIM,T> > >             \
  FUN(SMat<DIM,T> const & a, SMat<DIM,T> const & b) {                  \
    typedef SMat_##FUN<DIM,T,SMat<DIM,T>,SMat<DIM,T> > op ;            \
    return SMatE<DIM,T,op>(op(a,b)) ;                                  \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename A> inline               \
  SMatE<DIM,T,SMat_##FUN<DIM,T,SMatE<DIM,T,A>,SMat<DIM,T> > >          \
  FUN(SMatE<DIM,T,A> const & a, SMat<DIM,T> const & b) {               \
    typedef SMat_##FUN<DIM,T,SMatE<DIM,T,A>,SMat<DIM,T> > op ;         \
    return SMatE<DIM,T,op>(op(a,b));                                   \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename B> inline               \
  SMatE<DIM,T,SMat_##FUN<DIM,T,SMat<DIM,T>,SMatE<DIM,T,B> > >          \
  FUN(SMat<DIM,T> const & a, SMatE<DIM,T,B> const & b) {               \
    typedef SMat_##FUN<DIM,T,SMat<DIM,T>,SMatE<DIM,T,B> > op ;         \
    return SMatE<DIM,T,op>(op(a,b));                                   \
  }                                                                    \
                                                                       \
  template <unsigned DIM, typename T, typename A, typename B> inline   \
  SMatE<DIM,T,SMat_##FUN<DIM,T,SMatE<DIM,T,A>, SMatE<DIM,T,B> > >      \
  FUN(SMatE<DIM,T,A> const & a, SMatE<DIM,T,B> const & b) {            \
    typedef SMat_##FUN<DIM,T,SMatE<DIM,T,A>,SMatE<DIM,T,B> > op ;      \
    return SMatE<DIM,T,op>(op(a,b));                                   \
  }

  SMAT_FUN1(absval)
  SMAT_FUN1(sin)
  SMAT_FUN1(cos)
  SMAT_FUN1(tan)
  SMAT_FUN1(asin)
  SMAT_FUN1(acos)
  SMAT_FUN1(atan)
  SMAT_FUN1(cosh)
  SMAT_FUN1(sinh)
  SMAT_FUN1(tanh)

  SMAT_FUN1(sqrt)
  SMAT_FUN1(ceil)
  SMAT_FUN1(floor)
  SMAT_FUN1(exp)
  SMAT_FUN1(log)
  SMAT_FUN1(log10)

  SMAT_FUN2(pow)
  SMAT_FUN2(atan2)
  SMAT_FUN2(maxval)
  SMAT_FUN2(minval)

  /*
  //  #####  #     #                 #######    #
  // #     # ##   ##    ##     ##### #         ##
  // #       # # # #   #  #      #   #        # #
  //  #####  #  #  #  #    #     #   #####      #
  //       # #     #  ######     #   #          #
  // #     # #     #  #    #     #   #          #
  //  #####  #     #  #    #     #   #        #####
  */
  template<unsigned DIM, typename T, typename A>
  class MF1 {
    static inline index_type addr(index_type i, index_type j) { return i*DIM+j ;}
  public:
    VMLIB_TYPES(T) ;

    static inline
    value_type sum(A const & a) {
      value_type res(0) ;
      VMLIB_LOOP( DIM*DIM, res += a[i] ) ; 
      return res ;
    }
   
    static inline
    value_type trace(A const & a) {
      value_type res(0) ;
      VMLIB_LOOP( DIM, res += a[addr(i,i)] ) ; 
      return res ;
    }

    static inline
    value_type sum_row(A const & a, index_type nr) {
      value_type res(0) ;
      VMLIB_LOOP( DIM, res += a[addr(nr,i)] ) ; 
      return res ;
    }

    static inline
    value_type sum_col(A const & a, index_type nc) {
      value_type res(0) ;
      VMLIB_LOOP( DIM, res += a[addr(i,nc)] ) ; 
      return res ;
    }

    static inline
    value_type sum_abs_row(A const & a, index_type nr) {
      value_type res(0) ;
      VMLIB_LOOP( DIM, res += ::vmlib_fun::absval(a(nr,i)) ) ; 
      return res ;
    }

    static inline
    value_type sum_abs_col(A const & a, index_type nc) {
      value_type res(0) ;
      VMLIB_LOOP( DIM, res += ::vmlib_fun::absval(a[addr(i,nc)]) ) ; 
      return res ;
    }
   
    static inline
    value_type norm1(A const & a) {
      value_type res(0) ;
      VMLIB_LOOP( DIM, value_type bf = sum_abs_col(a,i) ; if ( res < bf ) res = bf ) ; 
      return res ;
    }

    static inline
    value_type normi(A const & a) {
      value_type res(0) ;
      VMLIB_LOOP( DIM, value_type bf = sum_abs_row(a,i) ; if ( res < bf ) res = bf ) ; 
      return res ;
    }

    static inline
    value_type normf(A const & a) {
      value_type res(0) ;
      VMLIB_LOOP( DIM*DIM, value_type bf = a[i] ; res += bf * bf) ;
      return ::vmlib_fun::sqrt(res) ;
    }

    static inline
    value_type maxval(A const & a) {
      value_type res = a[0] ;
      VMLIB_LOOP( DIM*DIM, if ( a[i]  > res ) res = a[i]  ) ;
      return res ;
    }

    static inline
    value_type minval(A const & a) {
      value_type res = a[0] ;
      VMLIB_LOOP( DIM*DIM, if ( a[i] < res ) res = a[i] ) ;
      return res ;
    }

  } ;

  SMAT_FUN1_TREE(absval)
  SMAT_FUN1_TREE(sin)
  SMAT_FUN1_TREE(cos)
  SMAT_FUN1_TREE(tan)
  SMAT_FUN1_TREE(asin)
  SMAT_FUN1_TREE(acos)
  SMAT_FUN1_TREE(atan)
  SMAT_FUN1_TREE(cosh)
  SMAT_FUN1_TREE(sinh)
  SMAT_FUN1_TREE(tanh)

  SMAT_FUN1_TREE(sqrt)
  SMAT_FUN1_TREE(ceil)
  SMAT_FUN1_TREE(floor)
  SMAT_FUN1_TREE(exp)
  SMAT_FUN1_TREE(log)
  SMAT_FUN1_TREE(log10)

  SMAT_FUN2_TREE(pow)
  SMAT_FUN2_TREE(atan2)
  SMAT_FUN2_TREE(maxval)
  SMAT_FUN2_TREE(minval)
  
  // S U M
  template <unsigned DIM, typename T> inline
  T sum(SMat<DIM,T> const & v)
  { return MF1<DIM,T,SMat<DIM,T> >::sum(v) ; }

  template <unsigned DIM, typename T, typename A> inline
  T sum(SMatE<DIM,T,A> const & v)
  { return MF1<DIM,T,SMatE<DIM,T,A> >::sum(v) ; }

  // T R A C E
  template <unsigned DIM, typename T> inline
  T trace(SMat<DIM,T> const & v)
  { return MF1<DIM,T,SMat<DIM,T> >::trace(v) ; }

  template <unsigned DIM, typename T, typename A> inline
  T trace(SMatE<DIM,T,A> const & v)
  { return MF1<DIM,T,SMatE<DIM,T,A> >::trace(v) ; }

  // N O R M 1
  template <unsigned DIM, typename T> inline
  T norm1(SMat<DIM,T> const & v)
  { return MF1<DIM,T,SMat<DIM,T> >::norm1(v) ; }

  template <unsigned DIM, typename T, typename A> inline
  T norm1(SMatE<DIM,T,A> const & v)
  { return MF1<DIM,T,SMatE<DIM,T,A> >::norm1(v) ; }

  // N O R M F
  template <unsigned DIM, typename T> inline
  T normf(SMat<DIM,T> const & v)
  { return MF1<DIM,T,SMat<DIM,T> >::normf(v) ; }

  template <unsigned DIM, typename T, typename A> inline
  T normf(SMatE<DIM,T,A> const & v)
  { return MF1<DIM,T,SMatE<DIM,T,A> >::normf(v) ; }

  // N O R M I
  template <unsigned DIM, typename T> inline
  T normi(SMat<DIM,T> const & v)
  { return MF1<DIM,T,SMat<DIM,T> >::normi(v) ; }

  template <unsigned DIM, typename T, typename A> inline
  T normi(SMatE<DIM,T,A> const & v)
  { return MF1<DIM,T,SMatE<DIM,T,A> >::normi(v) ; }

  // N O R M P
  template <unsigned DIM, typename T, typename S> inline
  T normp(SMat<DIM,T> const & v, S const & p)
  { return MF1<DIM,T,SMat<DIM,T> >::normp(v,T(p)) ; }

  template <unsigned DIM, typename T, typename A, typename S> inline
  T normp(SMatE<DIM,T,A> const & v, S const & p)
  { return MF1<DIM,T,SMatE<DIM,T,A> >::normp(v,T(p)) ; }

  // M A X
  template <unsigned DIM, typename T> inline
  T maxval(SMat<DIM,T> const & v)
  { return MF1<DIM,T,SMat<DIM,T> >::maxval(v) ; }

  template <unsigned DIM, typename T, typename A> inline
  T maxval(SMatE<DIM,T,A> const & v)
  { return MF1<DIM,T,SMatE<DIM,T,A> >::maxval(v) ; }

  // M I N
  template <unsigned DIM, typename T> inline
  T minval(SMat<DIM,T> const & v)
  { return MF1<DIM,T,SMat<DIM,T> >::minval(v) ; }

  template <unsigned DIM, typename T, typename A> inline
  T minval(SMatE<DIM,T,A> const & v)
  { return MF1<DIM,T,SMatE<DIM,T,A> >::minval(v) ; }

  // M A T R I X -- V E C T O R
  template <unsigned DIM, typename T>
  class SVecM_mul_V : public VMLIB_mult<DIM,T>,
                      public VMLIB_solve<DIM,T>  {
  public:
    VMLIB_TYPES(T) ;

  private:
    SVec<DIM,T> res ;
    
  public:
    SVecM_mul_V(SMat<DIM,T> const & a, SVec<DIM,T> const & b)
      { VMLIB_mult<DIM,T>::M_mul_V(res.begin(), a.begin(), b.begin()) ; }
    const_reference operator [] (index_type i) const { return res[i] ; }
  } ;

  template <unsigned DIM, typename T> inline
  SVecE<DIM,T,SVecM_mul_V<DIM,T> >
  operator * (SMat<DIM,T> const & a, SVec<DIM,T> const & b) {
    typedef SVecM_mul_V<DIM,T> op ;
    return SVecE<DIM,T,op>(op(a,b)) ;
  }

  template <unsigned DIM, typename T, typename B> inline
  SVecE<DIM,T,SVecM_mul_V<DIM,T> >
  operator * (SMat<DIM,T> const & a, SVecE<DIM,T,B> const & b) {
    typedef SVecM_mul_V<DIM,T> op ;
    return SVecE<DIM,T,op>(op(a,SVec<DIM,T>(b))) ;
  }

  template <unsigned DIM, typename T, typename A> inline
  SVecE<DIM,T,SVecM_mul_V<DIM,T> >
  operator * (SMatE<DIM,T,A> const & a, SVec<DIM,T> const & b) {
    typedef SVecM_mul_V<DIM,T> op ;
    return SVecE<DIM,T,op>(op(SMat<DIM,T>(a),b)) ;
  }

  template <unsigned DIM, typename T, typename A, typename B> inline
  SVecE<DIM,T,SVecM_mul_V<DIM,T> >
  operator * (SMatE<DIM,T,A> const & a, SVecE<DIM,T,B> const & b) {
    typedef SVecM_mul_V<DIM,T> op ;
    return SVecE<DIM,T,op>(op(SMat<DIM,T>(a),SVec<DIM,T>(b))) ;
  }

  ////////////////////////////////

  template <unsigned DIM, typename T>
  class SVecV_div_M : public VMLIB_mult<DIM,T>,
                      public VMLIB_solve<DIM,T> {
  public:
    VMLIB_TYPES(T) ;
    
  private:
    SVec<DIM,T> res ;

  public:
    SVecV_div_M(SVec<DIM,T> const & a, SMat<DIM,T> const & b) {
      res = a ; VMLIB_solve<DIM,T>::LinearSolver(b,res) ;
    }
    const_reference operator [] (index_type i) const { return res[i] ; }
    
  } ;

  template <unsigned DIM, typename T> inline
  SVecE<DIM,T,SVecV_div_M<DIM,T> >
  operator / (SVec<DIM,T> const & a, SMat<DIM,T> const & b) {
    typedef SVecV_div_M<DIM,T> op ;
    return SVecE<DIM,T,op>(op(a,b)) ;
  }

  template <unsigned DIM, typename T, typename B> inline
  SVecE<DIM,T,SVecV_div_M<DIM,T> >
  operator / (SVec<DIM,T> const & a, SMatE<DIM,T,B> const & b) {
    typedef SVecV_div_M<DIM,T> op ;
    return SVecE<DIM,T,op>(op(a,SMat<DIM,T>(b))) ;
  }

  template <unsigned DIM, typename T, typename A> inline
  SVecE<DIM,T,SVecV_div_M<DIM,T> >
  operator / (SVecE<DIM,T,A> const & a, SMat<DIM,T> const & b) {
    typedef SVecV_div_M<DIM,T> op ;
    return SVecE<DIM,T,op>(op(SVec<DIM,T>(a),b)) ;
  }

  template <unsigned DIM, typename T, typename A, typename B> inline
  SVecE<DIM,T,SVecV_div_M<DIM,T> >
  operator / (SVecE<DIM,T,A> const & a, SMatE<DIM,T,B> const & b) {
    typedef SVecV_div_M<DIM,T> op ;
    return SVecE<DIM,T,op>(op(SVec<DIM,T>(a),SMat<DIM,T>(b))) ;
  }

  // M A T R I X -- M A T R I X
  template <unsigned DIM, typename T>
  class SMatM_mul_M : public VMLIB_mult<DIM,T>,
                      public VMLIB_solve<DIM,T> {
  public:
    VMLIB_TYPES(T) ;
    
  private:
    SMat<DIM,T> res ;

  public:
    SMatM_mul_M(SMat<DIM,T> const & a, SMat<DIM,T> const & b)
      { VMLIB_mult<DIM,T>::M_mul_M(res.begin(), a.begin(), b.begin()) ; }
    const_reference operator [] (index_type i) const { return res[i] ; }
  } ;

  template <unsigned DIM, typename T> inline
  SMatE<DIM,T,SMatM_mul_M<DIM,T> >
  operator * (SMat<DIM,T> const & a, SMat<DIM,T> const & b) {
    typedef SMatM_mul_M<DIM,T> op ;
    return SMatE<DIM,T,op>(op(a,b)) ;
  }

  template <unsigned DIM, typename T, typename B> inline
  SMatE<DIM,T,SMatM_mul_M<DIM,T> >
  operator * (SMat<DIM,T> const & a, SMatE<DIM,T,B> const & b) {
    typedef SMatM_mul_M<DIM,T> op ;
    return SMatE<DIM,T,op>(op(a,SMat<DIM,T>(b))) ;
  }

  template <unsigned DIM, typename T, typename A> inline
  SMatE<DIM,T,SMatM_mul_M<DIM,T> >
  operator * (SMatE<DIM,T,A> const & a, SMat<DIM,T> const & b) {
    typedef SMatM_mul_M<DIM,T> op ;
    return SMatE<DIM,T,op>(op(SMat<DIM,T>(a),b)) ;
  }

  template <unsigned DIM, typename T, typename A, typename B> inline
  SMatE<DIM,T,SMatM_mul_M<DIM,T> >
  operator * (SMatE<DIM,T,A> const & a, SMatE<DIM,T,B> const & b) {
    typedef SMatM_mul_M<DIM,T> op ;
    return SMatE<DIM,T,op>(op(SMat<DIM,T>(a),SMat<DIM,T>(b))) ;
  }

  /////////////////////////////////////////////////////////////////////////

  template <unsigned DIM, typename T>
  class SMatM_div_M : public VMLIB_mult<DIM,T>,
                      public VMLIB_solve<DIM,T> {
  public:
    VMLIB_TYPES(T) ;
    
  private:
    SMat<DIM,T> res ;
    
  public:
    SMatM_div_M(SMat<DIM,T> const & a, SMat<DIM,T> const & b) {
      res = a ; VMLIB_solve<DIM,T>::LinearSolver(b,res) ;
    }
    const_reference operator [] (index_type i) const { return res[i] ; }
  } ;

  template <unsigned DIM, typename T> inline
  SMatE<DIM,T,SMatM_div_M<DIM,T> >
  operator / (SMat<DIM,T> const & a, SMat<DIM,T> const & b) {
    typedef SMatM_div_M<DIM,T> op ;
    return SMatE<DIM,T,op>(op(a,b)) ;
  }

  template <unsigned DIM, typename T, typename B> inline
  SMatE<DIM,T,SMatM_div_M<DIM,T> >
  operator / (SMat<DIM,T> const & a, SMatE<DIM,T,B> const & b) {
    typedef SMatM_div_M<DIM,T> op ;
    return SMatE<DIM,T,op>(op(a,SMat<DIM,T>(b))) ;
  }

  template <unsigned DIM, typename T, typename A> inline
  SMatE<DIM,T,SMatM_div_M<DIM,T> >
  operator / (SMatE<DIM,T,A> const & a, SMat<DIM,T> const & b) {
    typedef SMatM_div_M<DIM,T> op ;
    return SMatE<DIM,T,op>(op(SMat<DIM,T>(a),b)) ;
  }

  template <unsigned DIM, typename T, typename A, typename B> inline
  SMatE<DIM,T,SMatM_div_M<DIM,T> >
  operator / (SMatE<DIM,T,A> const & a, SMatE<DIM,T,B> const & b) {
    typedef SMatM_div_M<DIM,T> op ;
    return SMatE<DIM,T,op>(op(SMat<DIM,T>(a),SMat<DIM,T>(b))) ;
  }

  template <unsigned DIM, typename T>
  class SMatS_div_M : public VMLIB_mult<DIM,T>,
                      public VMLIB_solve<DIM,T>  {
  public:
    VMLIB_TYPES(T) ;

  private:
    SMat<DIM,T> res ;
    
  public:
    SMatS_div_M(T const & s, SMat<DIM,T> const & b)
      { res = s ; VMLIB_solve<DIM,T>::LinearSolver(b,res) ; }
    const_reference operator [] (index_type i) const { return res[i] ; }
  } ;

  template <unsigned DIM, typename T, typename S> inline
  SMatE<DIM,T,SMatS_div_M<DIM,T> >
  operator / (S const & s, SMat<DIM,T> const & b) {
    typedef SMatS_div_M<DIM,T> op ;
    return SMatE<DIM,T,op>(op(s,b)) ;
  }

  template <unsigned DIM, typename T, typename B, typename S> inline
  SMatE<DIM,T,SMatS_div_M<DIM,T> >
  operator / (S const & s, SMatE<DIM,T,B> const & b) {
    typedef SMatS_div_M<DIM,T> op ;
    return SMatE<DIM,T,op>(op(s,SMat<DIM,T>(b))) ;
  }

  /*
  // ###         # #######
  //  #         #  #     #
  //  #        #   #     #
  //  #       #    #     #
  //  #      #     #     #
  //  #     #      #     #
  // ###   #       #######
  */
  
  template <typename A>
  inline
  void print_vector(ostream& s, A const & v, index_type DIM) {
    index_type sz1 = DIM - 1 ;
    s << "[" ;
    for ( index_type i = 0 ; i < sz1 ; ++i )
      s << v[i] << " , " ;
    s << v[sz1] << "]" ;
  }
    
  template <unsigned DIM, typename T>
  inline
  ostream& operator << (ostream& s, SVec<DIM,T> const & v) {
    print_vector(s,v,DIM) ;
    return s ;
   }

  template <unsigned DIM, typename T, typename A>
  inline
  ostream& operator << (ostream& s, SVecE<DIM,T,A> const & v) {
    print_vector(s,v,DIM) ;
    return s ;
  }

  template <unsigned DIM, typename T>
  inline
  istream& operator >> (istream& s, SVec<DIM,T> & v) {
    typename SVec<DIM,T>::pointer pv=v.begin() ;
    while ( pv != v.end() ) s >> *pv++ ;
    return s ;
  }

  template <typename A>
  inline
  void print_matrix(ostream& s, A const & m, index_type DIM) {
    s << DIM << " x " << DIM << "\n" ;
    for ( index_type i = 0 ; i < DIM ; ++i ) {
      for ( index_type j = 0 ; j < DIM ; ++j )
        { s.width(10) ; s << m(i,j) << " " ; }
      s << "\n" ;
    }
  }

  template <unsigned DIM, typename T>
  inline
  ostream& operator << (ostream& s, SMat<DIM,T> const & v) {
    print_matrix(s,v,DIM) ;
    return s ;
  }

  template <unsigned DIM, typename T, typename A>
  inline
  ostream& operator << (ostream& s, SMatE<DIM,T,A> const & v) {
    print_matrix(s,v,DIM) ;
    return s ;
  }

  template <unsigned DIM, typename T> inline
  istream& operator >> (istream& s, SMat<DIM,T> & v) {
    typename SMat<DIM,T>::pointer pv=v.begin() ;
    while ( pv != v.end() ) s >> *pv++ ;
    return s ;
  }
  
  /*
  //  #####  ######
  // #     # #     #
  // #     # #     #
  // #     # ######
  // #   # # #   #
  // #    #  #    #
  //  #### # #     #
  */
  
  template <unsigned DIM, typename T>
  void
  HouseHolderQR(SMat<DIM,T> & a,
                SVec<DIM,T> & d,
                SVec<DIM,T> & beta) {
    unsigned i,j=0,k ;
    do {
      T s2(0) ;
      for (i=j ; i<DIM ; ++i ) s2 += a(i,j)*a(i,j) ;
      // if s2 = 0 matrice singolare
      d[j] = ::vmlib_fun::sqrt(s2) ; if ( a(j,j) < 0 ) d[j] = - d[j] ;
      a(j,j) += d[j] ;
      beta[j] = 1/(a(j,j)*d[j]) ;
      for ( k = j+1 ; k < DIM ; ++k ) {
        T bf(0) ;
        for (i=j;i<DIM; ++i) bf = bf + a(i,j)*a(i,k);
        bf = bf * beta[j] ;
        for (i=j;i<DIM; ++i) a(i,k) -= a(i,j)*bf ;
      }
    } while ( ++j < DIM ) ;
  }

  template <unsigned DIM, typename T>
  void
  HouseHolderQprod(SMat<DIM,T> const & a,
                   SVec<DIM,T> const & beta,
                   SVec<DIM,T>       & b) {
    unsigned i,j ;
    for (j=0 ; j<DIM ; ++j ) {
      T bf(0) ;
      for (i=j ; i < DIM; ++i) bf = bf + a(i,j)*b[i] ;
      bf = bf * beta[j] ;
      for (i=j ; i < DIM; ++i) b[i] -= a(i,j)*bf ;
    }
  }

  template <unsigned DIM, typename T>
  void
  HouseHolderQprod(SMat<DIM,T> const & a,
                   SVec<DIM,T> const & beta,
                   SMat<DIM,T>       & b) {
    unsigned i,j,k ;
    for (j=0 ; j < DIM ; ++j ) {
      for ( k=0 ; k < DIM ; ++k ) {
        T bf(0) ;
        for (i=j ; i < DIM; ++i) bf = bf + a(i,j)*b(i,k) ;
        bf = bf * beta[j] ;
        for (i=j ; i < DIM; ++i) b(i,k) -= a(i,j)*bf ;
      }
    }
  }

  template <unsigned DIM, typename T>
  void
  HouseHolderRsolve(SMat<DIM,T> const & a,
                    SVec<DIM,T> const & beta,
                    SVec<DIM,T>       & b) {
   unsigned i = DIM ;
   do {
      --i ;
      for (unsigned j=i+1 ; j < DIM ; ++j) b[i] -= a(i,j)*b[j] ;
        b[i] /= -beta[i] ;
    } while ( i != 0 ) ;
  }

  template <unsigned DIM, typename T>
  void
  HouseHolderRsolve(SMat<DIM,T> const & a,
                    SVec<DIM,T> const & beta,
                    SMat<DIM,T>       & b) {
    unsigned k = 0 ;
    do {
      unsigned i= DIM ;
      do {
        --i ;
        for ( unsigned j=i+1 ; j < DIM ; ++j) b(i,k) -= a(i,j)*b(j,k) ;
        b(i,k) /= -beta[i] ;
      } while ( i != 0 ) ;
    } while ( ++k < DIM ) ;
  }

  template <unsigned DIM, typename T, unsigned J, unsigned K>
  class HouseHolder {
  public:

    VMLIB_TYPES(T) ;

    static inline
    void
    QR(SMat<DIM,T> & a, SVec<DIM,T> & d, SVec<DIM,T> & beta) {
      value_type s2=0 ;
      for (index_type i=J ; i < DIM ; ++i ) s2 += a(i,J)*a(i,J) ;
      // if sigma = 0 matrice singolare
      d[J] = ::vmlib_fun::sqrt(s2) ; if ( a(J,J) < 0 ) d[J] = - d[J] ;
      a(J,J) += d[J] ;
      beta[J] = 1/(a(J,J)*d[J]) ;
      HouseHolder<DIM,T,J,J+1>::QR1(a,d,beta) ;
      HouseHolder<DIM,T,J+1,J+1>::QR(a,d,beta) ;
    }

    static inline
    void
    QR1(SMat<DIM,T> & a, SVec<DIM,T> & d, SVec<DIM,T> & beta) {
      value_type bf(0) ;
      for (index_type i = J ; i < DIM ; ++i) bf = bf + a(i,J)*a(i,K);
      bf = bf * beta[J] ;
      for (index_type i = J ; i < DIM ; ++i) a(i,K) -= a(i,J)*bf ;
      HouseHolder<DIM,T,J,K+1>::QR1(a,d,beta) ;
    }
      
    static inline
    void
    Qprod(SMat<DIM,T> const & a, SVec<DIM,T> const & beta, SVec<DIM,T> & b) {
      value_type bf(0) ;
      for (index_type i = J ; i < DIM ; ++i) bf = bf + a(i,J) * b[i] ;
      bf = bf * beta[J] ;
      for (index_type i = J ; i < DIM ; ++i) b[i] -= bf * a(i,J);
      HouseHolder<DIM,T,J+1,J+1>::Qprod(a,beta,b) ;
    }

    static inline
    void
    Qprod(SMat<DIM,T> const & a, SVec<DIM,T> const & beta, SMat<DIM,T> & b) {
      for ( index_type k=0 ; k < DIM ; ++k ) {
        value_type bf(0) ;
        for (index_type i = J ; i < DIM; ++i) bf = bf + a(i,J)*b(i,k) ;
        bf = bf * beta[J] ;
        for (index_type i = J ; i < DIM; ++i) b(i,k) -= bf * a(i,J);
      }
      HouseHolder<DIM,T,J+1,J+1>::Qprod(a,beta,b) ;
    }

    static inline
    void
    Rsolve(SMat<DIM,T> const & a, SVec<DIM,T> const & beta, SVec<DIM,T> & b) {
      HouseHolder<DIM,T,J+1,J+1>::Rsolve(a,beta,b) ;
      for (index_type j = J+1 ; j < DIM ; ++j) b[J] -= a(J,j)*b[j] ;
      b[J] /= -beta[J] ;
    } ;

    static inline
    void
    Rsolve(SMat<DIM,T> const & a, SVec<DIM,T> const & beta, SMat<DIM,T> & b) {
      HouseHolder<DIM,T,J+1,J+1>::Rsolve(a,beta,b) ;
      for ( index_type k = 0 ; k < DIM ; ++k) {
        for (index_type j = J+1 ; j < DIM ; ++j) b(J,k) -= a(J,j)*b(j,k) ;
        b(J,k) /= -beta[J] ;
      }
    } ;

  } ;

  template <unsigned DIM, typename T, unsigned J>
  class HouseHolder<DIM,T,J,DIM> {
  public:
    static inline void QR (SMat<DIM,T> &, SVec<DIM,T> &, SVec<DIM,T> &) { }
    static inline void QR1(SMat<DIM,T> &, SVec<DIM,T> &, SVec<DIM,T> &) { }
      
    static inline
    void Qprod(SMat<DIM,T> const &, SVec<DIM,T> const &, SVec<DIM,T> &) { }
      
    static inline
    void Qprod(SMat<DIM,T> const &, SVec<DIM,T> const &, SMat<DIM,T> &) { }
      
    static inline
    void Rsolve(SMat<DIM,T> const &, SVec<DIM,T> const &, SVec<DIM,T> &) { }
      
    static inline
    void Rsolve(SMat<DIM,T> const &, SVec<DIM,T> const &, SMat<DIM,T> &) { }
  } ;

  # ifdef VMLIB_NO_INLINE_HOUSEHOLDER

  template <unsigned DIM, typename T>
  inline
  void
  VMLIB_solve<DIM,T>::LinearSolver(SMat<DIM,T> const & a, SVec<DIM,T> & x) {
    SVec<DIM,T> d, beta ;
    SMat<DIM,T> A(a) ;
    HouseHolderQR(A,d,beta) ;
    HouseHolderQprod(A,beta,x) ;
    HouseHolderRsolve(A,d,x) ;
  }

  template <unsigned DIM, typename T>
  inline
  void
  VMLIB_solve<DIM,T>::LinearSolver(SMat<DIM,T> const & a, SMat<DIM,T> & X) {
    SVec<DIM,T> d, beta ;
    SMat<DIM,T> A(a) ;
    HouseHolderQR(A,d,beta) ;
    HouseHolderQprod(A,beta,X) ;
    HouseHolderRsolve(A,d,X) ;
  }

  # else

  template <unsigned DIM, typename T>
  inline
  void
  VMLIB_solve<DIM,T>::LinearSolver(SMat<DIM,T> const & a, SVec<DIM,T> & x) {
    SVec<DIM,T> d, beta ;
    SMat<DIM,T> A(a) ;
    HouseHolder<DIM,T,0,0>::QR(A,d,beta) ;
    HouseHolder<DIM,T,0,0>::Qprod(A,beta,x) ;
    HouseHolder<DIM,T,0,0>::Rsolve(A,d,x) ;
  }

  template <unsigned DIM, typename T>
  inline
  void
  VMLIB_solve<DIM,T>::LinearSolver(SMat<DIM,T> const & a, SMat<DIM,T> & X) {
    SVec<DIM,T> d, beta ;
    SMat<DIM,T> A(a) ;
    HouseHolder<DIM,T,0,0>::QR(A,d,beta) ;
    HouseHolder<DIM,T,0,0>::Qprod(A,beta,X) ;
    HouseHolder<DIM,T,0,0>::Rsolve(A,d,X) ;
  }

  # endif

  template <typename T>
  class VMLIB_solve<1,T> {
  public:
    static inline void LinearSolver(SMat<1,T> const & A, SVec<1,T> & bx) {
      bx[0] /= A(0,0) ;
    }
    static inline void LinearSolver(SMat<2,T> const & A, SMat<2,T> & BX) {
      BX(0,0) /= A(0,0) ;
    }
  } ;

  template <typename T>
  class VMLIB_solve<2,T> {
  public:
    static inline void LinearSolver(SMat<2,T> const & A, SVec<2,T> & bx) {
      T det = A(0,0) * A(1,1) - A(0,1) * A(1,0) ;
      T bf = (bx[0] * A(1,1) - bx[1] * A(0,1)) / det ;
      bx[1] = (A(0,0) * bx[1] - A(1,0) * bx[0] ) / det ;
      bx[0] = bf ;
    }
    static inline void LinearSolver(SMat<2,T> const & A, SMat<2,T> & BX) {
      T det = A(0,0) * A(1,1) - A(0,1) * A(1,0) ;
      T bf = (BX(0,0) * A(1,1) - BX(1,0) * A(0,1)) / det ;
      BX(1,0) = (A(0,0) * BX(1,0) - A(1,0) * BX(0,0) ) / det ;
      BX(0,0) = bf ;
      bf = (BX(0,1) * A(1,1) - BX(1,1) * A(0,1)) / det ;
      BX(1,1) = (A(0,0) * BX(1,1) - A(1,0) * BX(0,1) ) / det ;
      BX(0,1) = bf ;
    }
    
  } ;
}


namespace vmlib_load {
  
  using ::vmlib::SVec ;
  using ::vmlib::SMat ;
  
  using ::vmlib::sin ;
  using ::vmlib::cos ;
  using ::vmlib::tan ;
  using ::vmlib::asin ;
  using ::vmlib::acos ;
  using ::vmlib::atan ;
  using ::vmlib::cosh ;
  using ::vmlib::sinh ;
  using ::vmlib::tanh ;

  using ::vmlib::sqrt ;
  using ::vmlib::ceil ;
  using ::vmlib::floor ;
  using ::vmlib::exp ;
  using ::vmlib::log ;
  using ::vmlib::log10 ;

  using ::vmlib::absval ;
  using ::vmlib::maxval ;
  using ::vmlib::minval ;
  using ::vmlib::pow ;
  using ::vmlib::atan2 ;

  using ::vmlib::sum ;
  using ::vmlib::norm1 ;
  using ::vmlib::norm2 ;
  using ::vmlib::normi ;
  using ::vmlib::normp ;
  using ::vmlib::maxval ;
  using ::vmlib::minval ;
  using ::vmlib::dot ;
  using ::vmlib::dot_div ;
  using ::vmlib::dist ;
  using ::vmlib::dist2 ;
  
  using ::vmlib::trace ;
  using ::vmlib::normf ;
  
  // I/O
  using ::vmlib::operator << ;
  using ::vmlib::operator >> ;

}

/*
// ####### ####### #######
// #       #     # #
// #       #     # #
// #####   #     # #####
// #       #     # #
// #       #     # #
// ####### ####### #
*/

# endif
