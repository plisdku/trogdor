/*--------------------------------------------------------------------------*\
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
 |                                                                          |
 |            ___    ____  ___  __  __        ___    ____  ___  __  __      |
 |           /   \  /     /   \  \  /        /   \  /     /   \  \  /       |
 |          /____/ /__   /____/   \/        /____/ /__   /____/   \/        |
 |         /   \  /     /   \     /        /   \  /     /   \     /         |
 |        /____/ /____ /    /    /        /____/ /____ /    /    /          |
 |                                                                          |
 |      Enrico Bertolazzi                                                   |
 |      Dipartimento di Ingegneria Meccanica e Strutturale                  |
 |      Universita` degli Studi di Trento                                   |
 |      Via Mesiano 77, I-38050 Trento, Italy                               |
 |                                                                          |
\*--------------------------------------------------------------------------*/

/**
 * 
 * @mainpage Calculator: an Expression Evaluator Class
 * @date     August 11, 2004
 * @version  4.7
 * @note     first release May 22, 1999
 *
 * @author   Enrico Bertolazzi
 *
 * @par      Affiliation:
 *           Department of Mechanics and Structures Engineering <br>
 *           University of Trento <br>
 *           via Mesiano 77, I -- 38050 Trento, Italy <br>
 *           enrico.bertolazzi@ing.unitn.it
 *
 * @par Preface
 * This code is inspired by the ``Expression Evaluator'' of
 * Mark Morley (c) 1992
 *      - morley@camosun.bc.ca <br>
 *        Mark Morley <br>
 *        3889 Mildred Street <br>
 *        Victoria, BC  Canada <br>
 *        V8Z 7G1 <br>
 *        (604) 479-7861
 * @par Abstract
 * This document presents a C++ class for run-time evaluation of
 * simple symbolic expressions.  This is particularly suitable to
 * facilitates the input process from free format files.  The C++
 * compiler must support templates, namespaces and exceptions.
 */
 
 /**
 * @page 1 Usage 
 * @par Including the class Calculator<...>
 * The software consists of a single header file @c calc.hh.
 * There is no @c .a or @c .so files to be linked.
 * The @c calc.hh facilities are available by the following
 * inclusion statement:
 *
 * @code
 * # include "calc.hh"
 * using namespace calc_load ;
 * @endcode
 *
 * @par Instantiating an expression evaluator.
 * The instantiation process of an expression evaluator requires
 * the specification of its template type. The statement
 *
 * @code
 * Calculator<double> ee ;
 * @endcode
 *
 * instantiates the object @c ee which in this case is an 
 * expression evaluator operating on @c double's.
 *
 */

/** @page 2 Parsing a string
 * The object @c ee can now be used to parse simple
 * symbolic expressions input as strings,
 *
 * @code
 * bool err = ee . parse("1+sin(3.5)/4") ;
 * @endcode
 *
 * the boolean variable @c err is set @c true when a parsing 
 * error is found. The function @c get_value() returns the value 
 * resulting from the parsing process:
 *
 * @code
 * double res = ee . get_value() ;
 * @endcode
 *
 * When an error is produced, an error report explaining what went wrong
 * can be print out by the class method @c report_error, as
 * for example in the following piece of source:
 *
 * @code
 * ee . report_error(cout) ;
 * @endcode
 *
 * The input string to the method @c parse can contain 
 * more than one expression. Individual expressions must be 
 * separated by the end-of-statement separator @c ;.
 * 
 * @code
 * err = ee . parse("a=1+sin(3) ; b=1-sin(3) ; sqrt(a*b)") ;
 * @endcode
 *
 * In the case of a multiple expression parsing, the class method 
 * @c get_value() returns only the last evaluated value.
 * In the previous example this is  @c a*b.
 * 
 * Notice also that the assignement symbol @c = makes possible the 
 * memorization of intermediate expression values in other variables.
 */
 
/** @page 3 Operators
 * 
 * The expression evaluator uses a fixed number of operators, that are
 * listed in order of precedence as follows
 * 
 *    - @c #
 *      the rest of the string is a comment (and clearly ignored);
 *    - @c +, @c -
 *      binary addition and subtraction, e.g.
 *      @code
 *        10+2; 3-4.2;
 *      @endcode
 *    - @c *, @c /
 *      binary multiplication and division, e.g.
 *      @code
 *        2.3*4.9; 2/4
 *      @endcode
 *    - @c ^
 *      power, e.g.
 *      @code
 *        10^4 # (results 10000);
 *      @endcode
 *    - @c +, @c -
 *      unary @c + and @c -, e.g.
 *      @code
 *        +120; 12+-12 ;
 *      @endcode
 *    - @c (, @c )
 *      parenthesis are use to change operator precedence;
 *      for example the expression @c 12-(2-2)
 *      evaluates to @c 12 while @c 12-2-2 evaluates to @c 8;
 *    - @c ;
 *      expression separator;
 *    - @c =
 *      assignement operator.
 */
 
/** @page 4 Symbolic Constants
 *
 * Two symbolic constants are available whose value
 * is assigned by default:
 *
 *    - @c e  = 2.71828182845904523536
 *    - @c pi = 3.14159265358979323846
 *
 * They can be used in symbolic expressions like
 * the following one:
 *
 * @code
 * e + sin(pi*0.5) ;
 * @endcode
 *
 */
 
/** @page 5 Predefined Functions
 *
 * In the previous section we used the function @c sin.
 * There are a number of predefined functions which can
 * be used in symbolic expressions. In the following
 * we lists them.
 *
 *    - @c abs(x)
 *      absolute value of @c x
 *    - @c pos(x)
 *      positive part of @c x
 *    - @c neg(x)
 *      negative part of @c x
 *    - @c cos(x)
 *      cosine of @c x
 *    - @c sin(x)
 *      sine of @c x
 *    - @c tan(x)
 *      tangent of @c x
 *    - @c asin(x)
 *      arcsin of @c x
 *    - @c acos(x)
 *      arccos of @c x
 *    - @c atan(x)
 *      arctan of @c x
 *    - @c cosh(x)
 *      hyperbolic cosine of @c x
 *    - @c sinh(x)
 *      hyperbolic sine of @c x
 *    - @c tanh(x)
 *      hyperbolic tangent of @c x
 *    - @c exp(x)
 *      exponential of @c x
 *    - @c log(x)
 *      natural logarithm of @c x
 *    - @c log10(x)
 *      base @c 10 logarithm of @c x
 *    - @c sqrt(x)
 *      square root of of @c x
 *    - @c ceil(x)|
 *      least integer over @c x
 *    - @c floor(x)
 *      great integer under @c x
 *
 *    - @c max(x,y)
 *      maximum of @c {x,y}
 *    - @c min(x,y)
 *      minimum of @c {x,y}
 *    - @c atan2(x,y)
 *      arctan of @c y/x
 *    - @c pow(x,y)
 *      power @f$x^{y}@f$
 */
 
/** @page 6 Defining new functions
 * 
 * A new function can be introduced into the expression
 * evaluator by defining it as static and then 
 * passing the evaluator its name and address pointer
 * by using the two evaluator facilities @c set_unary_fun 
 * and @c set_binary_fun. 
 * 
 * The following example illustrates the mechanism. Let us first
 * define the two static functions:
 *
 * @code
 * static double power2(double const & a)
 * { return a*a ; }
 * 
 * static double add(double const & a, double const & b)
 * { return a+b ; }
 * @endcode
 *
 * Then let us add @c power2 and @c add to the current
 * expression evaluator as follows:
 *
 * @code
 * ee . set_unary_fun("power2",power2) ;
 * ee . set_binary_fun("add",add) ;
 * @endcode
 *
 * These new functions can now be invoked in symbolic expressions 
 * as the predefined ones:
 *
 *
 * @code
 * err = ee . parse("power2(add(2,e))") ;
 * @endcode
 *
 * The expression evaluator is capable of handling only unary and 
 * binary functions, i.e. functions with one or two arguments.
 */
 
/** @page 7 Defining new variables
 * 
 * New variables can be introduced into the expression evaluator by using
 * the method @c set or the assignement operator.
 * For example, the following piece of source code defines the new variable 
 * @c abc and initialize it to the value @c 1/3: 
 *
 * @code
 * err = ee . parse("abc = 1/3") ;
 * ee . set("abc",1.0/3.0) ;
 * @endcode
 *
 * The first statements uses the parse method and the assignement operator
 * @c = of the expression evaluator. The parse method evaluates the 
 * expression on the right of @c = and then assigns the parsing result 
 * to the variable on the right. 
 *
 * If the variable should not exist it would be created and assigned.

 * The second statement creates - if needed - and assigns directly the 
 * variable. Once created and initilized, the variable can be used in 
 * the next operations; for example
 *
 * @code
 * err = ee . parse("zz = abc*sin(3)/(1+abc)") ;
 * @endcode
 *
 * In this case the new variable @c zz is also created.
 * A variable is a string which always begins with a letter and may be
 * followed by any sequence of alphanumeric characters, such as numbers,
 * letters or the underscore symbols like @c _.
 *
 * The @c exist return true if its argument is a 
 * defined variable, as in
 *
 * @code
 * bool ex1 = ee . exist("abc") ;
 * bool ex2 = ee . exist("pippo") ;
 * @endcode
 *
 * In this case @c ex1 is set to true and @c ex2 to false.
 * It is possible to get out the value of a variable, 
 * 
 *
 * @code
 * double val1 = ee . get("abc") ;
 * double val2 = ee . get("pippo") ;
 * @endcode
 *
 * The value of @c val1 is @c 0.333333 while @c val2 is null 
 * because the variable @c pippo does not exist.
 */
 
/** @page 8 Parsing a file
 *
 * The expression evaluator can be used to parse a complete
 * file. The parsing process proceeds by reading the file 
 * one line at a time and parsing it. Use the method
 *
 * @code
 * ee . parse_file("filename", true) ;
 * @endcode
 *
 * The boolean @c true in the second entry asks the 
 * expression evaluator for proceeding in verbose mode, 
 * that is for printing out on @c cerr input errors
 * when detected. If the flag was set to @c false,
 * reading would proceed silently and errors ignored.
 * 
 * For example, consider the following input file:
 * 
 *
 @verbatim
 # this is a comment line
 gamma = 1.4
 # set left state
 rin = 1 # density
 vin = 0 # velocity
 pin = 1 # pressure
 ein = rin*vin*vin/2+pin/(gamma-1)
 
 # set right state
 rout = 0.125
 vout = 0
 pout = 0.1
 eout = rin*vin*vin/2+pout/(gamma-1)
 @endverbatim
 *
 * If a program needs as input parameters @c rin, @c vin,
 * @c ein, @c rout, @c vout, @c eout the following 
 * piece of code
 *
 * @code
 * ee . parse_file("file.data", true) ;
 * double rin  = ee . get("rin") ;
 * double vin  = ee . get("vin") ;
 * double ein  = ee . get("ein") ;
 * double rout = ee . get("rout") ;
 * double vout = ee . get("vout") ;
 * double eout = ee . get("eout") ;
 * @endcode
 * 
 * does the work. The advantages of using expression
 * evaluators in reading input files are multiples:
 * 
 *    - a free input format is easily usable;
 *    - comments can be added everywhere therein;
 *    - simple computations may be inserted as part of an
 *      input file.
 */

# ifndef CALC_HH
# define CALC_HH

# ifdef __DECCXX
  # include <math.h>
# else
  # include <cmath>
# endif

// standard includes I/O
# include <iostream>
# include <iomanip>
# include <fstream>

# ifdef USE_OLD_STRSTREAM
# include <strstream>
# else
# include <sstream>
# endif

// STL lib
# include <string>
# include <map>

/**
 * This namespace is used to shield the class definitions
 */
namespace calc_defs {
  
  using namespace ::std ;
   
  /**
   * This class implement the expression evaluator
   */
  template <typename T_type = double>
  class Calculator {
  
    // Calculator type
  public:
  
    typedef T_type              value_type ;
    typedef value_type*         pointer;
    typedef const value_type*   const_pointer ;
    typedef value_type&         reference ;
    typedef const value_type&   const_reference ;
  
    typedef Calculator<value_type> CALCULATOR ;
  
    typedef value_type (*Func1)(const_reference) ;
    typedef value_type (*Func2)(const_reference,const_reference) ;
  
    typedef map<string,Func1>      map_fun1 ;
    typedef map<string,Func2>      map_fun2 ;
    typedef map<string,value_type> map_real ;
  
    typedef typename map_fun1::iterator       map_fun1_iterator ;
    typedef typename map_fun1::const_iterator map_fun1_const_iterator ;
    typedef typename map_fun2::iterator       map_fun2_iterator ;
    typedef typename map_fun2::const_iterator map_fun2_const_iterator ;
    typedef typename map_real::iterator       map_real_iterator ;
    typedef typename map_real::const_iterator map_real_const_iterator ;
  
  private:

    static value_type internal_abs   (const_reference x) { return x > 0 ? x : - x ; }
    static value_type internal_pos   (const_reference x) { return x > 0 ? x : 0 ; }
    static value_type internal_neg   (const_reference x) { return x > 0 ? 0 : x ; }
  
    static value_type internal_cos   (const_reference x) { return cos(x) ; }
    static value_type internal_sin   (const_reference x) { return sin(x) ; }
    static value_type internal_tan   (const_reference x) { return tan(x) ; }
  
    static value_type internal_acos  (const_reference x) { return acos(x) ; }
    static value_type internal_asin  (const_reference x) { return asin(x) ; }
    static value_type internal_atan  (const_reference x) { return atan(x) ; }
  
    static value_type internal_cosh  (const_reference x) { return cosh(x) ; }
    static value_type internal_sinh  (const_reference x) { return sinh(x) ; }
    static value_type internal_tanh  (const_reference x) { return tanh(x) ; }
  
    static value_type internal_exp   (const_reference x) { return exp(x)   ; }
    static value_type internal_log   (const_reference x) { return log(x)   ; }
    static value_type internal_log10 (const_reference x) { return log10(x) ; }
    static value_type internal_sqrt  (const_reference x) { return sqrt(x)  ; }
    static value_type internal_ceil  (const_reference x) { return ceil(x)  ; }
    static value_type internal_floor (const_reference x) { return floor(x) ; }
  
    static value_type internal_atan2 (const_reference x, const_reference y) { return atan2(x,y) ; }
    static value_type internal_pow   (const_reference x, const_reference y) { return pow(x,y) ; }
    static value_type internal_max   (const_reference a, const_reference b) { return a > b ? a : b ; }
    static value_type internal_min   (const_reference a, const_reference b) { return a < b ? a : b ; }
  
    map_fun1 unary_fun ;
    map_fun2 binary_fun ;
    map_real variables ;
  
    typedef enum {
      Number, Variable, Parameter,
      Plus, Minus, Times, Divide, Power,
      OpenPar, ClosePar,
      Assign, Comma, Unrecognized,
      EndOfExpression, EndOfString
    } Token_Type ;
  
    typedef enum {
      No_Error,
      Divide_By_Zero,
      Expected_OpenPar, Expected_ClosePar, Expected_Comma,
      Unknown_Variable, Bad_Position, Unknown_Error
    } ErrorCode ;
  
  private:
    // error handling
    ErrorCode  error_found ;
  
    // token management
    Token_Type token_type     ;
    string     token_string   ;
    value_type last_evaluated ;
  
    // internal use
    char const * string_in ;
    char const * ptr ;
  
    char const & get(void)       { return *ptr++ ; }
    char const & see(void) const { return *ptr ; }
    int          pos(void) const { return static_cast<int>(ptr - string_in) ; }
  
    void get_number(value_type & res, char const * const s) const
    # ifdef USE_OLD_STRSTREAM
    { istrstream str(s) ; str >> res ; }
    # else
    { stringstream str(s) ; str >> res ; }
    # endif
    
    CALCULATOR const & operator = (CALCULATOR const &) ;
    // { init() ; return *this ; }

    Calculator(CALCULATOR const &) ;
    // { init() ; }
  
  public:
  
    Calculator(void) 
      : unary_fun(),
        binary_fun(),
        variables(),
        error_found(),
        token_type(),
        token_string(),
        last_evaluated(0),
        string_in(0),
        ptr(0)
    { init() ; } ;

    ~Calculator(void) { } ;
  
    void init(void) ;
    
    /**
     *  This method do a parsing of an input string
     *  @param str the input string to be parsed
     *  @return @a true if no error parsing errors are found
     */
    bool parse(char const str[]) ;
    bool parse(string const & str) { return parse(str . c_str()) ; }

    /**
     *  This method do a parsing of a whole file
     *  @param name     the name of the input file
     *  @param show_err true if you wand message on error parsing
     */
    void parse_file(char const name[], bool const show_err) ;
    void parse_file(string const & name, bool const show_err)
    { parse_file( name . c_str(), show_err) ; }
  
    /**
     *  Print the last error found, noe if no error are found
     *  @param s the object stream of output
     */
    void report_error(ostream & s) ;

    /**
     *  @return the value of the last evaluated expression
     */
    const_reference get_value(void) const { return last_evaluated ; } ;

    /**
     *  Set or define a variable
     *  @param name the name of the variable
     *  @param val the value to be stored
     */
    void set(char const name[],   const_reference val) { variables[name] = val ; }
    void set(string const & name, const_reference val) { variables[name] = val ; }
  
    /**
     *  Look if a variable exists
     *  @param name the name of the variable
     *  @return @a true if the variable exists
     */
    bool exist(char const name[]) const
    { return variables . find(name) != variables . end() ; }
    bool exist(string const & name) const
    { return variables . find(name) != variables . end() ; }
  
    /**
     *  @param name the name of the variable
     *  @return the value of the variable @a name
     */
    value_type get(char const name[]) {
      map_real_const_iterator ii = variables . find(name) ;
      value_type res = 0 ;
      if ( ii != variables . end() ) res = ii -> second ;
      return res ;
    }
    value_type get(string const & name) { return get(name . c_str()) ; }

    /**
     *  Print internal status of the parser: the list of the variables
     *  constants and functions
     *  @param s the object stream of output
     */
    void print(ostream & s) const ;
  
    /**
     *  Add unary function to the parser
     *  @param f_name function name
     *  @param f_ptr  pointer to the function routine
     */
    void set_unary_fun(char const f_name[], Func1 f_ptr)
    { unary_fun[f_name] = f_ptr ; }
    void set_unary_fun(string const & f_name, Func1 f_ptr)
    { unary_fun[f_name] = f_ptr ; }
  
    /**
     *  Add binary function to the parser
     *  @param f_name function name
     *  @param f_ptr  pointer to the function routine
     */
    void set_binary_fun(char const f_name[], Func2 f_ptr)
    { binary_fun[f_name] = f_ptr ; }
    void set_binary_fun(string const & f_name, Func2 f_ptr)
    { binary_fun[f_name] = f_ptr ; }

    /**
     *  @return @a true if no error found
     */
    bool no_error(void) const { return No_Error == error_found ; }

    /**
     *  @return a reference of the variables map.
     */
    map_real const & variables_map() const { return variables ; }
    
    /**
     *  @return a reference of the variables map.
     */
    void variables_merge(map_real const & ee_vars) {
      for ( map_real_const_iterator ii = ee_vars . begin() ;
            ii != ee_vars . end() ; ++ii ) {
        set( ii -> first, ii -> second ) ;
      }
    }

  private:
  
    value_type G0(void) ;
    value_type G1(void) ;
    value_type G2(void) ;
    value_type G3(void) ;
    value_type G4(void) ;
    value_type G5(void) ;
    void Next_Token(void) ;
  
  } ;
  
  template <typename T_type>
  inline
  void
  Calculator<T_type>::init() {
    last_evaluated      = 0 ;
    unary_fun["cos"]    = internal_cos ;
    unary_fun["sin"]    = internal_sin ;
    unary_fun["tan"]    = internal_tan ;
  
    unary_fun["acos"]   = internal_acos ;
    unary_fun["asin"]   = internal_asin ;
    unary_fun["atan"]   = internal_atan ;
  
    unary_fun["cosh"]   = internal_cosh ;
    unary_fun["sinh"]   = internal_sinh ;
    unary_fun["tanh"]   = internal_tanh ;
  
    unary_fun["exp"]    = internal_exp ;
    unary_fun["log"]    = internal_log ;
    unary_fun["log10"]  = internal_log10 ;
    unary_fun["sqrt"]   = internal_sqrt ;
    unary_fun["ceil"]   = internal_ceil ;
    unary_fun["floor"]  = internal_floor ;
  
    binary_fun["atan2"] = internal_atan2 ;
    binary_fun["pow"]   = internal_pow ;
    binary_fun["max"]   = internal_max ;
    binary_fun["min"]   = internal_min ;
  
    // predefined variables
    variables["pi"]     = 3.14159265358979323846 ;
    variables["e"]      = 2.71828182845904523536 ;
  }
  
  template <typename T_type>
  bool
  Calculator<T_type>::parse(char const str[]) {
    string_in = ptr = str ;
    error_found = No_Error ;
    try {
      do { Next_Token() ; } while ( token_type == EndOfExpression ) ;
      while ( token_type != EndOfString && error_found == No_Error ) {
        last_evaluated = G0() ;
        while ( token_type == EndOfExpression ) Next_Token() ;
      }
    }
    catch (ErrorCode err) {
      error_found = err ;
    }
    catch (...) {
      error_found = Unknown_Error ;
    }
    return error_found != No_Error ;
  }
  
  template <typename T_type>
  void
  Calculator<T_type>::parse_file(char const name[],
                                 bool const show_err=false) {
    string        line ;
    unsigned long nline = 0 ;
    ifstream      in(name) ;
    while ( in . good() ) {
      ++nline ;
      getline( in, line) ;
      if ( parse(line.c_str()) && show_err ) {
        cerr << "in line " << nline << " found an error\n" ;
        report_error(cerr) ;
      }
    }
    in . close() ;
  }
  
  template <typename T_type>
  void
  Calculator<T_type>::report_error(ostream & s) {
    switch (error_found) {
    case No_Error:
      s << "No error found\n" ;
      return ;
  
    case Divide_By_Zero:
      s << "divide by 0\n" ;
      break;
  
    case Expected_OpenPar:
      s << "expect ``('' found ``" << token_string << "''\n" ;
      break;
  
    case Expected_ClosePar:
      s << "expect ``)'' found ``" << token_string << "''\n" ;
      break;
  
    case Expected_Comma:
      s << "expect ``,'' found ``" << token_string << "''\n" ;
      break;
  
    case Unknown_Variable:
      s << "unknown variable: " << token_string << "\n" ;
      break ;
  
    case Bad_Position:
      s << "bad position for token: ``" << token_string << "''\n" ;
      break;

    case Unknown_Error:
      s << "Unknown error for token: ``" << token_string << "''\n" ;
      break;
    }
    s << "\t: " << string_in << "\n"
      << "\t: " << setfill('-') << setw(pos()-1) << "^\n" ;
  }
  
  template <typename T_type>
  void
  Calculator<T_type>::print(ostream & s) const {
    map_real_const_iterator ii ;
    map_fun1_const_iterator f1 ;
    map_fun2_const_iterator f2 ;
  
    s << "\nUNARY FUNCTIONS\n" ;
    for ( f1 = unary_fun . begin() ; f1 != unary_fun . end() ; ++f1 )
      s << f1 -> first << ", " ;
  
    s << "\n\nBINARY FUNCTIONS\n" ;
    for ( f2 = binary_fun . begin() ; f2 != binary_fun . end() ; ++f2 )
      s << f2 -> first << ", " ;
  
    s << "\n\nVARIABLES\n" ;
    for ( ii = variables . begin() ; ii != variables . end() ; ++ii )
      s << ii -> first << " = " << ii -> second << "\n" ;
  
    s << "END LIST\n" ;
  }
  
  template <typename T_type>
  typename Calculator<T_type>::value_type
  Calculator<T_type>::G0() {
  
    if ( token_type == EndOfExpression || token_type == EndOfString ) return 0 ;
  
    char const * const bf_ptr = ptr ; // save pointer
    string     name  = token_string ;
    Token_Type token = token_type ;
  
    if ( token_type == Variable ) {
      Next_Token() ;
      if ( token_type == Assign ) { // handle assign
        Next_Token() ;              // eat assign
        value_type res = G1() ;
        variables[name] = res ;
        return res ; // assign value
      }
    }
    
    ptr = bf_ptr ; // restore pointer
    token_string = name ;
    token_type   = token ;
  
    return G1() ; // handle immediate evaluation
  }
  
  // handle binary + and -
  template <typename T_type>
  typename Calculator<T_type>::value_type
  Calculator<T_type>::G1() {
    value_type lval = G2(), rval ;
    while ( token_type == Plus || token_type == Minus ) {
      bool do_plus = token_type == Plus ;
      Next_Token() ;
      rval = G2() ;
      if ( do_plus ) lval += rval ;
      else           lval -= rval ;
    }  ;
    return lval ;
  }
  
  // handles * and /
  template <typename T_type>
  typename Calculator<T_type>::value_type
  Calculator<T_type>::G2() {
    value_type lval = G3(), rval ;
    while ( token_type == Times || token_type == Divide ) {
      bool do_times = token_type == Times ;
      Next_Token() ;
      rval = G3() ;
      if ( do_times ) lval *= rval ;
      else {
        if ( rval == 0 ) throw Divide_By_Zero ;
        lval /= rval ;
      }
    } ;
    return lval ;
  }
  
  // handles ^ operator
  template <typename T_type>
  typename Calculator<T_type>::value_type
  Calculator<T_type>::G3() {
    value_type lval = G4() ;
    if ( token_type == Power )
      { Next_Token() ; lval = pow(lval,G4()) ; }
    return lval ;
  }
  
  // handles any unary + or - signs
  template <typename T_type>
  typename Calculator<T_type>::value_type
  Calculator<T_type>::G4() {
    if ( token_type == Minus ) { Next_Token() ; return - G5() ; }
    if ( token_type == Plus  ) Next_Token() ;
    return G5() ;
  }
  
  // handles numbers, variables, functions and parentesis
  template <typename T_type>
  typename Calculator<T_type>::value_type
  Calculator<T_type>::G5() {
    if ( token_type == OpenPar ) { // handle ( ... )
      Next_Token() ; // eat (
      value_type val = G0() ;
      if ( token_type != ClosePar ) throw Expected_ClosePar ;
      Next_Token() ; // eat )
      return val ;
    }
  
    if ( token_type == Number ) {
      value_type val ;
      get_number(val,token_string.c_str()) ;
      Next_Token() ;
      return val ;
    }
  
    if ( token_type == Variable ) {
  
      map_real_iterator iv = variables . find(token_string) ;
      if ( iv != variables . end() ) {
        Next_Token() ;
        return iv -> second ;
      }
  
      map_fun1_iterator f1 = unary_fun . find(token_string) ;
      if ( f1 != unary_fun . end() ) {
        Next_Token() ; // expect (
        if ( token_type != OpenPar ) throw Expected_OpenPar ;
        Next_Token() ; // eat (
        value_type v1 = G0() ;
        if ( token_type != ClosePar ) throw Expected_ClosePar ;
        Next_Token() ; // eat )
        return f1 -> second(v1) ;
      }
  
      map_fun2_iterator f2 = binary_fun . find(token_string) ;
      if ( f2 != binary_fun . end() ) {
        Next_Token() ; // expect (
        if ( token_type != OpenPar ) throw Expected_OpenPar ;
        Next_Token() ; // eat (
        value_type v1 = G0() ;
        if ( token_type != Comma ) throw Expected_Comma ;
        Next_Token() ; // eat ,
        value_type v2 = G0() ;
        if ( token_type != ClosePar ) throw Expected_ClosePar ;
        Next_Token() ; // eat )
        return f2 -> second(v1,v2) ;
      }
  
      throw Unknown_Variable ;
    }
    throw Bad_Position ;
  }
  
  template <typename T_type>
  void
  Calculator<T_type>::Next_Token() { // eat separators
    token_type   = EndOfExpression ;
    token_string = "" ;
  
    // EAT SEPARATORS
    while ( isspace(see()) ) get() ;
  
    // SKIP COMMENTS
    if ( see() == '#' ) {
      while ( see() != '\n' && see() != '\0' ) get() ;
      return ;
    }
  
    if ( isalpha(see()) ) {
      token_type = Variable ;
      do { token_string += get() ; } while ( isalnum(see()) || see() == '_' ) ;
      return ;
    }
    
    if ( isdigit(see()) ) {
      token_type = Number ;
      do { token_string += get() ; } while ( isdigit(see()) ) ;
      if ( see() == '.' ) {
        do { token_string += get() ; } while ( isdigit(see()) ) ;
      }
      if ( see() == 'e' || see() == 'E' ) {
        token_string += get() ;
        if ( see() == '+' || see() == '-' ) token_string += get() ;
        if ( isdigit(see()) ) {
          do { token_string += get() ; } while ( isdigit(see()) ) ;
        } else {
          token_type = Unrecognized ;
        }
      }
      return ;
    }
  
    token_string = get() ;
    switch (token_string[0]) {
    case '+' : token_type = Plus ;
               break ;
    case '-' : token_type = Minus ;
               break ;
    case '*' : token_type = Times ;
               break ;
    case '/' : token_type = Divide ;
               break ;
    case '^' : token_type = Power ;
               break ;
    case '(' : token_type = OpenPar ;
               break ;
    case ')' : token_type = ClosePar ;
               break ;
    case '=' : token_type = Assign ;
               break ;
    case ',' : token_type = Comma ;
               break ;
    case '\0': token_type   = EndOfString     ;
               token_string = "EndOfString"   ;
               break ;
    case ';' : token_type   = EndOfExpression ;
               token_string = "EndOfExpression" ;
               break ;
    default  : token_type   = Unrecognized ;
               token_string = "Unrecognized" ;
               break ;
    }
  }
  
  // end class Calculator

} // end namespace


namespace calc_load {
  using calc_defs::Calculator ;
}

# endif

// end of file: calc.hh
