// ==============================================================
//
//  Copyright (C) 1995  William A. Rossi
//                       class RossiBigInt
// 
//  Copyright (C) 1999-2015  Alex Vinokur
//                           class BigInt 
//                           class BigInt::BaseBigInt 
//                           class BigInt::Vin
//                           upgrading class BigInt::Rossi
//                           class BigInt::Run
//                           class BigInt::Test
//                           class BigInt::TestVin
//                           class BigInt::TestRossi
//
//  ------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//  ------------------------------------------------------------
// 
//  mailto:alex DOT vinokur AT gmail DOT com
//  http://sourceforge.net/users/alexvn/
//
// ==============================================================


// ##############################################################
//
//  SOFTWARE : Class BigInt
//  FILE     : bigInt.h
//
//  DESCRIPTION : Declarations of the classes
//								* BigInt 
//								* BigInt::BaseBigInt 
//								* BigInt::Vin
//								* BigInt::Rossi
//								* BigInt::Run
//								* BigInt::Test
//								* BigInt::TestVin
//								* BigInt::TestRossi
//
// ##############################################################


#ifndef   __BIG_INT_H__
#define   __BIG_INT_H__

// include section
#include <cassert>
#include <climits>
#include <cmath>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <iomanip>
#include <utility>
#include <limits>
#include <algorithm>
#include <stdexcept>
#include <QString>


// ========================================
// BOOST
// --------------------------
// #define BIG_INT_USES_BOOST
// --------------------------
#ifdef BIG_INT_USES_BOOST
#include "boost/static_assert.hpp"
#endif
// ========================================





// =================================================
// ========== FUNCTION_NAME MACROS (BEGIN) =========


#ifdef FUNCTION_NAME
#undef FUNCTION_NAME
#endif

#ifdef UNDECORATED_FUNCTION_NAME
#undef UNDECORATED_FUNCTION_NAME
#endif

// --------------------------------------
// See
// http://predef.sourceforge.net/precomp.html
// --------------------------------------
#if defined _MSC_VER	// Windows
// #define FUNCTION_NAME __FUNCSIG__  // too long and not friendly representation
#define FUNCTION_NAME               __FUNCTION__
#define UNDECORATED_FUNCTION_NAME   __FUNCTION__
#elif defined __GNUC__	// g++ GNU C++
#define FUNCTION_NAME               __PRETTY_FUNCTION__
#define UNDECORATED_FUNCTION_NAME   __FUNCTION__
#elif defined __HP_aCC	// aCC on HP-UX
#define FUNCTION_NAME               __PRETTY_FUNCTION__
#define UNDECORATED_FUNCTION_NAME   __FUNCTION__
#elif defined __xlC__	// xlC on IBM AIX
#define FUNCTION_NAME               __FUNCTION__
#define UNDECORATED_FUNCTION_NAME   __func__
#elif defined __SUNPRO_CC	// SUN CC
#define FUNCTION_NAME               BOOST_CURRENT_FUNCTION  // Must be compiled with option  -features=extensions
#define UNDECORATED_FUNCTION_NAME   __func__                // Must be compiled with option  -features=extensions
#elif defined __INTEL_COMPILER	// Intel C/C++
#define FUNCTION_NAME               __PRETTY_FUNCTION__
#define UNDECORATED_FUNCTION_NAME   __FUNCTION__
#else
#define FUNCTION_NAME               "Unable to detect FUNCTION_NAME"
#define UNDECORATED_FUNCTION_NAME   "Unable to detect UNDECORATED_FUNCTION_NAME"
#endif


// -----------------------------------------------
#define SET_START_TEST_NAME(outstream) BigInt::Test::setTestName(outstream,  "START ", FUNCTION_NAME, __LINE__)
#define SET_FINISH_TEST_NAME(outstream) BigInt::Test::setTestName(outstream, "FINISH", FUNCTION_NAME, __LINE__)


// ========== FUNCTION_NAME MACROS (END) ===========
// =================================================


// ===========================================
// ========== DISPLAY MACROS (BEGIN) =========

#define PRE_MSG(s, p)       s << p << "[" <<  __FILE__ << ", #" << std::dec << std::setw(3) << __LINE__ << "; " << UNDECORATED_FUNCTION_NAME << "] " << std::flush
#define SCREEN_MSG(s,p,x,t) { lastError = QString(t.c_str()); std::ostringstream oo_ss_ss; PRE_MSG(oo_ss_ss,p) << x << t << std::endl; s << std::flush << oo_ss_ss.str() << std::flush; }
#define FATAL_MSG(s, t)     SCREEN_MSG (s, "FATAL   ", "", t)
#define ERR_MSG(s, t)       SCREEN_MSG (s, "ERROR   ", "", t)
#define WARN_MSG(s, t)      SCREEN_MSG (s, "WARNING ", "", t)
#define INFO_MSG(s, t)      SCREEN_MSG (s, "INFO    ", "", t)
#define SUCCESS_MSG(s, t)   SCREEN_MSG (s, "SUCCESS ", "", t)

// ========== DISPLAY MACROS (END) ===========
// ===========================================



// ===========================================
// ========= ASSERTION MACROS (BEGIN) ========
#define ASSERTION(x)    if (!(x)) throw QString("BigInt assertion failed: " + lastError)
// ========== ASSERTION MACROS (END) =========
// ===========================================


struct BigInt
{
    // -------------------
    // FORWARD DECLARATION
    // -------------------
    class Rossi;
 

    // --------
    // TYPEDEFS
    // --------
    typedef unsigned long Ulong;


    // ----------------
    // STATIC CONSTANTS
    // ----------------
    static const std::size_t BIG_INT_MAJOR_VERSION = 6;
    static const std::size_t BIG_INT_MINOR_VERSION = 0;

    static const std::size_t DEC_DIGIT          = 10;
    static const std::size_t HEX_DIGIT          = 16;
    static const std::size_t NIBBLE_BIT         = 4;
    static const std::size_t ULONG_HEX_DIGITS   = ((sizeof (Ulong) * CHAR_BIT)/NIBBLE_BIT);


    static const Ulong MAX_UNIT_VALUE   = (ULONG_MAX >> 2);
    static const Ulong ULONG_MSB        = static_cast<Ulong>(1) << (sizeof(Ulong) * CHAR_BIT - 1);

    static const Ulong BASE1        = 10;
    static const Ulong BASE2        = 1000000000;  // // BASE1 ** (BASE1 - 1)
    static const Ulong SUB_BASE2    = BASE2 - 1;  // 999999999
    static const Ulong OVER_BASE2   = 0xc0000000;  // OVER_BASE2 > SUB_BASE2

	static const std::string s_strHelp;
	static const std::string s_strHellow;

    // -----------------------------------------
#ifdef BIG_INT_USES_BOOST
    BOOST_STATIC_ASSERT (SUB_BASE2 == 999999999);
    BOOST_STATIC_ASSERT (!(BASE2 >= MAX_UNIT_VALUE));
    BOOST_STATIC_ASSERT (BASE1 * (BASE2/BASE1 + 1) < MAX_UNIT_VALUE);
    BOOST_STATIC_ASSERT (!(BASE2 != (SUB_BASE2 + 1)));
    BOOST_STATIC_ASSERT (OVER_BASE2 > SUB_BASE2);

    BOOST_STATIC_ASSERT(
                            (sizeof(Ulong) == 4) && ((ULONG_MSB == static_cast<Ulong>(0x80000000)))
                            ||
                            (sizeof(Ulong) == 8) && ((ULONG_MSB == ((static_cast<Ulong>(0x80000000) << 31) << 1)))
                       );
#endif
    // -----------------------------------------
    // Public Methods
    static void assertCheck();
    static void showVersion(std::ostream& o_stream);

    // -----------------------------------
    template <typename T>
    static std::string toString(const T& t)
    {
        std::ostringstream oss;
        oss << t;
        return oss.str();
    }
    
     // -------------------------------
    template<typename T, std::size_t N> 
    static std::size_t getArrayElems(const T(&)[N]) 
    { 
        return N; 
    } 

    // -------------------------------
    template<typename T, std::size_t N> 
    static std::size_t getArrayElems(T(&)[N]) 
    { 
        return N; 
    } 

    // -------------------------------
    template<typename T, std::size_t N> 
    static std::size_t getArraySizeInBytes(const T(&a)[N]) 
    { 
        return getArrayElems(a) * sizeof (T); 
    }

    // -------------------------------
    template<typename T, std::size_t N> 
    static std::size_t getArraySizeInBytes(T(&a)[N]) 
    { 
        return getArrayElems(a) * sizeof (T); 
    } 


    // -------------------------------
    template<typename T, std::size_t N> 
    static std::vector<T> array2vector(const T(&a)[N]) 
    { 
        return std::vector<T> (a, a + getArrayElems (a));
    } 

    // --------------------------------
    template<typename T, std::size_t N> 
    static std::vector<T> array2vector(T(&a)[N]) 
    { 
        return std::vector<T> (a, a + getArrayElems (a));
    } 

    // ------------------------------------
    template<typename T> 
    static std::vector<T> elem2vector(const T& e) 
    { 
        return std::vector<T> (&e, &e + 1);
    } 

    // ------------------------------------
    template<typename T> 
    static std::vector<T> elem2vector(T& e) 
    { 
        return std::vector<T> (&e, &e + 1);
    } 

    // ---------------------------------------------------------
    template<typename K, typename T, std::size_t N> 
    static std::map<K, T> array2map(const std::pair<K,T>(&a)[N]) 
    { 
        return std::map<K, T> (a, a + getArrayElems (a));
    } 

    // ---------------------------------------------------------
    template<typename K, typename T, std::size_t N> 
    static std::map<K, T> array2map(std::pair<K,T>(&a)[N]) 
    { 
        return std::map<K, T> (a, a + getArrayElems (a));
    } 

    // ------------------------------------
    template<typename K, typename T>
    static std::map<K, T> pair2map(const std::pair<K,T>& e) 
    { 
        return std::map<K, T> (&e, &e + 1);
    } 



// ==============
class BaseBigInt
// ==============
{

public:
    // --- Public Methods 
    std::size_t getUnitsSize () const;
    void        maximize ();

protected :
    // --- Protected Data Members ---
    std::vector<Ulong>                  m_units;
    static std::map<char, std::size_t>  ms_hex2dec;

    // --- Protected Methods 
    void showUnits (std::ostream& o_stream) const;
    virtual ~BaseBigInt () = 0;


public :
    // --- Public Methods ---
    bool  isEmpty () const;

};



// ==============
class Vin : public BaseBigInt
// ==============
{
private :
    // -------
    // FRIENDS
    // -------
    friend std::ostream& operator<< (std::ostream& o_os, const Vin& i_ins);


private :
    // --- Private Data Members ---
    static Ulong s_carry;

    // --- Private Methods ---
    static Ulong    addUnits(Ulong n1, Ulong n2);
    bool            initViaString (const std::string& i_arg, std::size_t i_digitBase);

public :
    // ----------------------
    // --- Public Methods ---
    // ----------------------
    // --- Constructors ---
    explicit Vin ();
    explicit Vin (Ulong i_unit);
    explicit Vin (const std::string& i_arg, std::size_t i_digitBase);
    explicit Vin (const Rossi& i_arg);

    // --- Aux methods  ---
    Ulong toUlong () const;
    // operator Ulong () const;
	long double toDouble () const;
	static Vin fromDouble (const long double& i_double);

    // --- Show functions ---
    std::string toStrHex (const std::string& i_prefix = "") const;
    std::string toStr0xHex () const;
    std::string toStrDec (const std::string& i_prefix = "") const;


    // --- General purpose mathematical methods ---
    Vin operator+ (const Vin& i_arg) const;
    Vin operator* (Ulong i_arg) const;


    // --- Comparison operators ---
    bool operator== (const Vin& i_arg) const;
    bool operator!= (const Vin& i_arg) const;
    bool operator<  (const Vin& i_arg) const;
    bool operator>  (const Vin& i_arg) const;
    bool operator<= (const Vin& i_arg) const;
    bool operator>= (const Vin& i_arg) const;


    // ---- Service methods ---
    void showUnits(std::ostream& o_stream) const;

};


// ============================
class Rossi : public BaseBigInt
// ============================
{
private :
    // -------
    // FRIENDS
    // -------
    friend std::ostream& operator<< (std::ostream& o_os, const Rossi& i_ins);

private :
    // --- Private Methods ---
    void  resizeUnits (std::size_t i_size);
    void  truncateUnits ();
    void  smartTruncateUnits ();
    bool  backUnitIsNull () const;
    bool  initViaString (const std::string& i_arg, std::size_t i_digitBase);


public :
    // ----------------------
    // --- Public Methods ---
    // ----------------------
    // --- Constructors ---
    explicit Rossi ();
    explicit Rossi (Ulong i_unit);
    explicit Rossi (const std::string& i_arg, std::size_t i_digitBase);
    explicit Rossi (const Vin& i_arg);
	explicit Rossi (const std::size_t i_noOfUnits, Ulong i_internalUnits, Ulong i_backUnit, const std::string& i_msg = std::string());


    // --- Aux methods ---
    Ulong toUlong () const;
    // operator Ulong () const;
	long double toDouble () const;
	static Rossi fromDouble (const long double& i_double);



    // --- General purpose mathematical methods ---

    // Rossi& operator= (Ulong i_arg);
    Rossi  operator+ (const Rossi& i_arg);
    Rossi  operator+ (Ulong i_arg);
    Rossi  operator* (Rossi i_arg) const;
    Rossi  operator* (Ulong i_arg) const;
    //  Rossi& Rossi::operator*= (Rossi i_arg);
    Rossi  operator/ (const Rossi& i_arg) const;
    Rossi  operator% (const Rossi& i_arg) const;
    Rossi  divide(const Rossi& i_dividend, const Rossi& i_divisor, Rossi* o_remainder) const;
    Rossi& operator+= (const Rossi& i_arg);
    Rossi  operator++ (int); // Post increment operator
    Rossi& operator++ ();    // Pre increment operator
    Rossi  operator-  (const Rossi& i_arg);
    Rossi  operator-  ();
    Rossi  operator-- (int); // Post decrement operator
    Rossi& operator-- ();    // Pre decrement operator
    Rossi& operator-= (const Rossi& i_arg);
    Rossi  sqrt();
	Rossi  naive_sqrt();


    // --- Bitwise boolean operators ---
    Rossi  operator&   (const Rossi& i_arg);
    Rossi  operator|   (const Rossi& i_arg);
    Rossi  operator^   (const Rossi& i_arg);
    Rossi& operator&=  (const Rossi& i_arg);
    Rossi& operator|=  (const Rossi& i_arg);
    Rossi& operator^=  (const Rossi& i_arg);
    Rossi  operator~   ();
    Rossi  operator>>  (std::size_t i_shift);
    Rossi  operator<<  (std::size_t  i_shift);
    Rossi& operator>>= (std::size_t  i_shift);
    Rossi& operator<<= (std::size_t  i_shift);



    // --- Comparison operators ---
    bool operator== (const Rossi& i_arg) const;
    bool operator!= (const Rossi& i_arg) const;
    bool operator<  (const Rossi& i_arg) const;
    bool operator>  (const Rossi& i_arg) const;
    bool operator<= (const Rossi& i_arg) const;
    bool operator>= (const Rossi& i_arg) const;

    // --- Show functions ---
    std::string toStrHex (const std::string& i_prefix = "") const;
    std::string toStr0xHex () const;
    std::string toStrDec () const;

    // ---- Service methods ---
    void showUnits(std::ostream& o_stream) const;


};


// ============================
struct Run
{
    static int  mainBigInt(int argc, char** argv); 
    static int  mainBigInt(std::ostream& o_stream, const std::vector<std::string>& i_args); 
	static void showHelp(std::ostream& o_stream, const std::vector<std::string>& i_args);
	static void showHelp(std::ostream& o_stream, const std::string& i_exeFileName = std::string()); 
    static bool checkCommandLine(std::ostream& o_stream, const std::vector<std::string>& i_args); 
    static std::map<std::string, std::string> getSampleAllowedOperation(); 
    static void runRossiInteractiveSample(std::ostream& o_stream, const std::vector<std::string>& i_args); 
    static void applicationSimpleSample(std::ostream& o_stream);  
};


// ============================
struct Test
{
    static std::vector<std::string> fillTestInputHexStr();

	static void setTestName(std::ostream& o_stream, const std::string& i_txt, const std::string& i_funcName, const std::size_t i_lineNo);

    static void testDisplayBigInts (std::ostream& o_stream);
    static void testMain (std::ostream& o_stream, const std::vector<std::string>& i_args = std::vector<std::string>());

};

struct TestRossi : public Test
{
    static std::vector<BigInt::Ulong> fillTestInputUlong();

    static std::vector<std::pair<BigInt::Rossi, BigInt::Rossi> > fillTestInputPairsRossiRossi();
    static std::vector<std::pair<BigInt::Rossi, BigInt::Ulong> > fillTestInputPairsRossiUlong();

    // ---------------------------------
    static void testAll (std::ostream& o_stream);
    static void testOperatorAdd (std::ostream& o_stream);
    static void testOperatorAddAssign (std::ostream& o_stream);
    static void testOperatorSubtraction (std::ostream& o_stream);
    static void testOperatorSubtractionAssign (std::ostream& o_stream);
    static void testOperatorMultiplication1 (std::ostream& o_stream);
    static void testOperatorMultiplication2 (std::ostream& o_stream);
    static void testOperatorDivision (std::ostream& o_stream);
    static void testOperatorReminder (std::ostream& o_stream);
    static void testOperatorLess (std::ostream& o_stream);
	static void testSqrt (std::ostream& o_stream);
    static void testMaxUnits (std::ostream& o_stream);
    static void testMaxMultiplication (std::ostream& o_stream);
    static void testTryCatch (std::ostream& o_stream);
    
};


struct TestVin : public Test
{
    static std::vector<BigInt::Ulong> fillTestInputUlong();

    static std::vector<std::pair<BigInt::Vin, BigInt::Vin> > fillTestInputPairsVinVin();
    static std::vector<std::pair<BigInt::Vin, BigInt::Ulong> > fillTestInputPairsVinUlong();


    // ---------------------------------
    static void testAll (std::ostream& o_stream);
    static void testOperatorAdd (std::ostream& o_stream);
    static void testOperatorMultiplication (std::ostream& o_stream);
    static void testOperatorLess (std::ostream& o_stream);
    static void testMaxUnits (std::ostream& o_stream);
    
};


}; // class BigInt



#endif    // __BIG_INT_H__
