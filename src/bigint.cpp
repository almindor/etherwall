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
//  FILE     : bigInt.cpp
//
//  DESCRIPTION : Implementation of the classes
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

// include section
#include "bigint.h"

// Errors
static QString lastError = "";

// =================
// Constants
// =================
static const BigInt::Rossi RossiZero (0);
static const BigInt::Rossi RossiOne  (1);
static const BigInt::Rossi RossiTwo  (2);
static const BigInt::Rossi RossiThree (3);
// static const BigInt::Rossi RossiTen  = BigInt::Rossi(BigInt::toString(std::numeric_limits<BigInt::Ulong>::max()), BigInt::DEC_DIGIT) + RossiOne;

static const BigInt::Vin VinZero (0);
static const BigInt::Vin VinOne  (1);

const std::string BigInt::s_strHelp("help");
const std::string BigInt::s_strHellow("hellow");


static const std::pair<char, std::size_t> s_hex2dec[] =
{
    std::make_pair ('0', 0),
    std::make_pair ('1', 1),
    std::make_pair ('2', 2),
    std::make_pair ('3', 3),
    std::make_pair ('4', 4),
    std::make_pair ('5', 5),
    std::make_pair ('6', 6),
    std::make_pair ('7', 7),
    std::make_pair ('8', 8),
    std::make_pair ('9', 9),
    std::make_pair ('a', 10),
    std::make_pair ('A', 10),
    std::make_pair ('b', 11),
    std::make_pair ('B', 11),
    std::make_pair ('c', 12),
    std::make_pair ('C', 12),
    std::make_pair ('d', 13),
    std::make_pair ('D', 13),
    std::make_pair ('e', 14),
    std::make_pair ('E', 14),
    std::make_pair ('f', 15),
    std::make_pair ('F', 15)
};

std::map<char, std::size_t> BigInt::BaseBigInt::ms_hex2dec (array2map(s_hex2dec));

// =================
// Macros
// =================

#define BIGINT_TEST_COMPUTE_BINARY_OP(outstream, type, args, op) \
	  type z = args.first op args.second; \
	  outstream << args.first \
	            << " " \
	            << #op \
	            << " " \
                << args.second \
	            << " = " \
	            << z \
	            << std::endl

#define BIGINT_TEST_COMPUTE_BINARY_ULONG_OP(outstream, type, args, op) \
	  type z = args.first op args.second; \
	  outstream << args.first \
	            << " " \
	            << #op \
	            << " " \
                << std::hex \
                << std::showbase \
                << args.second \
                << std::dec \
	            << " = " \
	            << z \
	            << std::endl

#define BIGINT_TEST_COMPARE_BINARY_OP(outstream, args, op) \
	  const bool z = args.first op args.second; \
	  outstream << args.first \
	            << " " \
	            << #op \
	            << " " \
                << args.second \
	            << " = " \
	            << (z ? "TRUE" : "FALSE") \
	            << std::endl

#define BIGINT_TEST_COMPUTE_UNARY_OP(outstream, args, op) \
      outstream << args.first; \
	  args.first op args.second; \
	  outstream << "" \
	            << " " \
	            << #op \
	            << " " \
                << args.second \
	            << " = " \
	            << args.first \
	            << std::endl


#define ROSSI_TEST_COMPUTE_BINARY_OP(outstream, args, op)          BIGINT_TEST_COMPUTE_BINARY_OP(outstream, BigInt::Rossi, args, op)
#define ROSSI_TEST_COMPUTE_BINARY_ULONG_OP(outstream, args, op)    BIGINT_TEST_COMPUTE_BINARY_ULONG_OP(outstream, BigInt::Rossi, args, op)

#define VIN_TEST_COMPUTE_BINARY_OP(outstream, args, op)            BIGINT_TEST_COMPUTE_BINARY_OP(outstream, BigInt::Vin, args, op)
#define VIN_TEST_COMPUTE_BINARY_ULONG_OP(outstream, args, op)      BIGINT_TEST_COMPUTE_BINARY_ULONG_OP(outstream, BigInt::Vin, args, op)



////////////
// FUNCTIONS
////////////
// ===============================
void BigInt::Run::showHelp(std::ostream& o_stream, const std::vector<std::string>& i_args)
{
    std::ostringstream oss;

    const std::string exeFileName (i_args.empty() ? "<exeFileName>" : i_args[0]);

    oss << ""
        << std::endl

        << "USAGE : " 
		<< std::endl
		
		<< "      : " 
        << exeFileName
		<< " "
		<< BigInt::s_strHelp
        << std::endl

		<< "      : " 
        << exeFileName
		<< " "
		<< BigInt::s_strHellow
        << std::endl

		<< "      : " 
        << exeFileName 
        << std::endl

        << "      : " 
        << exeFileName 
        << " <first-arg-in-decimal> <binary-operation> <second-arg-in-decimal>" 
        << std::endl

        << std::endl;

    oss << ""
        << std::endl

        << "SAMPLE: " 
		<< std::endl
        
		<< "      : " 
        << exeFileName
		<< " "
		<< BigInt::s_strHelp
        << std::endl

		<< "      : " 
        << exeFileName
		<< " "
		<< BigInt::s_strHellow
        << std::endl

		<< "      : " 
        << exeFileName 
        << std::endl

        << "      : " 
        << exeFileName 
        << " 1123581321345589 + 123456789" 
        << std::endl

        << std::endl;

    o_stream << oss.str();
}


// ===============================
void BigInt::Run::showHelp(std::ostream& o_stream, const std::string& i_exeFileName)
{
	std::ostringstream oss;

	std::vector<std::string> args;

	if (!i_exeFileName.empty())
	{
		args.push_back(i_exeFileName);
	}

	showHelp(o_stream, args);

	o_stream << oss.str();
}


// ===============================
std::map<std::string, std::string> BigInt::Run::getSampleAllowedOperation()
{
    static std::map<std::string, std::string> allowedOperations;

    if (allowedOperations.empty())
    {
        allowedOperations["+"] = "Addition";
        allowedOperations["-"] = "Subtraction";
        allowedOperations["x"] = "Multiplication";
        allowedOperations["/"] = "Division";
        allowedOperations["%"] = "Reminder";
    }

    return allowedOperations;

   
}


// ===============================
bool BigInt::Run::checkCommandLine(std::ostream& o_stream, const std::vector<std::string>& i_args)
{
    const std::map<std::string, std::string> allowedOperations = getSampleAllowedOperation();

    switch (i_args.size())
    {
        case 0:
        case 1:
            showHelp (o_stream, i_args);
            return true;
            break;  // unused

        case 4:
            if (!allowedOperations.count (i_args[2]))
            {
                std::ostringstream oss;
                oss << ""
                    << "Illegal operation in argv[2] : " 
                    << i_args[2] 
                    << std::endl;
                oss << "Allowed operations are as follows: ";
                for (std::map<std::string, std::string>::const_iterator posIter = allowedOperations.begin();  
                     posIter != allowedOperations.end(); 
                     posIter++
                    )
                {
                    oss << posIter->first 
                        << " ";  
                }
                oss << std::endl
                    << std::endl;
                o_stream << oss.str();
                return false;
            }
            return true;
            break;  // unused

        default:
            showHelp (o_stream, i_args);
            return false;
            break;  // unused
    }
}

// ===============================
int BigInt::Run::mainBigInt(int argc, char** argv)
{
    std::vector<std::string> i_args;

    for (int i = 0; i < argc; i++)
    {
        i_args.push_back(argv[i]);
    }

    const int result = mainBigInt(std::cout, i_args);

    return result;
}

// ===============================
int BigInt::Run::mainBigInt(std::ostream& o_stream, const std::vector<std::string>& i_args)
{

    BigInt::showVersion(o_stream);

    BigInt::assertCheck();

    applicationSimpleSample(std::cout);
    applicationSimpleSample(o_stream);
	
    if (!checkCommandLine (o_stream, i_args)) 
    {
        return 1;
    }

	
    if (i_args.size() <= 1)
    {

        int retValue = 0;
        try
        {

            BigInt::TestVin::testAll(o_stream);
            BigInt::TestRossi::testAll(o_stream);
            BigInt::TestRossi::testTryCatch(o_stream);     
        }
        catch (std::exception& i_e)
        {
            std::ostringstream oss;
            std::ostringstream ossFatal;
            ossFatal    << ""
                        << "EXCEPTION ===> "
                        << i_e.what() 
                        << std::endl;
            FATAL_MSG (oss, ossFatal.str());

            o_stream << oss.str();
          
            retValue = 1;
        }
        catch(...)
        {
            std::ostringstream oss;
            std::ostringstream ossFatal;
            ossFatal    << ""
                        << "UNKNOWN EXCEPTION"
                        << std::endl;
            FATAL_MSG (oss, ossFatal.str());

            o_stream << oss.str();
          
            retValue = 1;
        }
        showHelp (o_stream, i_args);
        return retValue;
    }

    // ----------------------
    ASSERTION (i_args.size() == 4);

    runRossiInteractiveSample(o_stream, i_args);

    return 0;
}

// ==================================
void BigInt::showVersion(std::ostream& o_stream)
{
    std::ostringstream oss;
    oss << std::endl

        << "BigInt - C++ class for computation with arbitrary large integers" 
        << std::endl

        << "Version " 
        << BIG_INT_MAJOR_VERSION 
        << "." 
        << BIG_INT_MINOR_VERSION 
        << std::endl

        << "Compiled in " 
        << (sizeof(void*) * CHAR_BIT) 
        << "-bit mode" 
        << std::endl

        << std::endl

        << "Copyright (C) 1995       William A. Rossi - class BigInt::Rossi" 
        << std::endl
        << "Copyright (C) 1999-2015  Alex Vinokur - class BigInt, class BigInt::BaseBigInt, class BigInt::Vin, upgrading class BigInt::Rossi, class BigInt::Run, class BigInt::Test, class BigInt::TestVin, class BigInt::TestRossi" 
        << std::endl

        << std::endl

        << "This is free software; see the source for copying conditions." 
        << std::endl
        << "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." 
        << std::endl

        << std::endl;

    o_stream << std::flush << oss.str() << std::flush;
}




// ==================================
void BigInt::Run::applicationSimpleSample(std::ostream& o_stream)
{
    std::ostringstream oss;

	SET_START_TEST_NAME(oss);

    // ---------------------------
    BigInt::Rossi rossi1 (100);
    BigInt::Rossi rossi2 ("100", DEC_DIGIT);
    BigInt::Rossi rossi3 ("100", HEX_DIGIT);
    BigInt::Rossi rossi4 ("123456789", DEC_DIGIT);
    BigInt::Rossi rossi5 ("123456789ABCDEF", HEX_DIGIT);
    BigInt::Rossi rossi6 ("99999999999999999999999999999999", DEC_DIGIT);
	BigInt::Rossi rossi7 ("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", HEX_DIGIT);

    std::vector<BigInt::Rossi> rossiArgs;
    rossiArgs.push_back(rossi1);
    rossiArgs.push_back(rossi2);
    rossiArgs.push_back(rossi3);
    rossiArgs.push_back(rossi4);
    rossiArgs.push_back(rossi5);
    rossiArgs.push_back(rossi6);
    rossiArgs.push_back(rossi7);


    const std::string strRossiDec ("[Rossi: Dec] ");
    const std::string strRossiHex ("[Rossi: Hex] ");

    // ---------------------------
    BigInt::Vin vin1 (100);
    BigInt::Vin vin2 ("100", DEC_DIGIT);
    BigInt::Vin vin3 ("100", HEX_DIGIT);
    BigInt::Vin vin4 ("123456789", DEC_DIGIT);
    BigInt::Vin vin5 ("123456789ABCDEF", HEX_DIGIT);
    BigInt::Vin vin6 ("99999999999999999999999999999999", DEC_DIGIT);
	BigInt::Vin vin7 ("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", HEX_DIGIT);

    std::vector<BigInt::Vin> vinArgs;
    vinArgs.push_back(vin1);
    vinArgs.push_back(vin2);
    vinArgs.push_back(vin3);
    vinArgs.push_back(vin4);
    vinArgs.push_back(vin5);
    vinArgs.push_back(vin6);
    vinArgs.push_back(vin7);


    const std::string strVinDec ("[Vin: Dec] ");
    const std::string strVinHex ("[Vin: Hex] ");


    // -----------------------------------------------
    for (std::size_t i = 0; i < rossiArgs.size(); i++)
    {
        for (std::size_t k = 0; k < rossiArgs.size(); k++)
        {
            oss << std::endl;

            BigInt::Rossi rossiArg1 = rossiArgs[i];
            BigInt::Rossi rossiArg2 = rossiArgs[k];
            BigInt::Rossi rossiResult;

            std::string theOperation;
            // ---------------------------------------------
            theOperation = "+";
            rossiResult = rossiArg1 + rossiArg2;

            oss << ""
                << strRossiHex
                << rossiArg1 
                << " " 
                << theOperation 
                << " " 
                << rossiArg2 
                << " = " 
                << rossiResult 
                << std::endl;

            oss << ""
                << strRossiDec
                << rossiArg1.toStrDec()  
                << " " 
                << theOperation 
                << " " 
                << rossiArg2.toStrDec()  
                << " = " 
                << rossiResult.toStrDec() 
                << std::endl;

            oss << std::endl;

            // ---------------------------------------------
            if (rossiArg1 >= rossiArg2)
            {
                theOperation = "-";
                rossiResult = rossiArg1 - rossiArg2;

                oss << ""
                    << strRossiHex
                    << rossiArg1 
                    << " " 
                    << theOperation 
                    << " " 
                    << rossiArg2 
                    << " = " 
                    << rossiResult 
                    << std::endl;

                oss << ""
                    << strRossiDec
                    << rossiArg1.toStrDec()  
                    << " " 
                    << theOperation
                    << " " 
                    << rossiArg2.toStrDec()  
                    << " = " 
                    << rossiResult.toStrDec() 
                    << std::endl;

                oss << std::endl;
            }

            // ---------------------------------------------
            theOperation = "*";
            rossiResult = rossiArg1 * rossiArg2;

            oss << ""
                << strRossiHex
                << rossiArg1 
                << " " 
                << theOperation
                << " " 
                << rossiArg2 
                << " = " 
                << rossiResult 
                << std::endl;

            oss << ""
                << strRossiDec
                << rossiArg1.toStrDec()  
                << " " 
                << theOperation 
                << " " 
                << rossiArg2.toStrDec()  
                << " = " 
                << rossiResult.toStrDec() 
                << std::endl;

            oss << std::endl;


            // ---------------------------------------------
            theOperation = "/";
            rossiResult = rossiArg1 / rossiArg2;

            oss << ""
                << strRossiHex
                << rossiArg1 
                << " " 
                << theOperation
                << " " 
                << rossiArg2 
                << " = " 
                << rossiResult 
                << std::endl;

            oss << ""
                << strRossiDec
                << rossiArg1.toStrDec()  
                << " " 
                << theOperation 
                << " " 
                << rossiArg2.toStrDec()  
                << " = " 
                << rossiResult.toStrDec() 
                << std::endl;

            oss << std::endl;


            // ---------------------------------------------
            theOperation = "%";
            rossiResult = rossiArg1 % rossiArg2;

            oss << ""
                << strRossiHex
                << rossiArg1 
                << " " 
                << theOperation
                << " " 
                << rossiArg2 
                << " = " 
                << rossiResult 
                << std::endl;

            oss << ""
                << strRossiDec
                << rossiArg1.toStrDec()  
                << " " 
                << theOperation 
                << " " 
                << rossiArg2.toStrDec()  
                << " = " 
                << rossiResult.toStrDec() 
                << std::endl;

            oss << std::endl;

        } // for (std::size_t k = 0; k < rossiArgs.size(); k++)
    } // for (std::size_t i = 0; i < rossiArgs.size(); i++)

    oss << std::endl;
    // -----------------------------------------------
    for (std::size_t i = 0; i < vinArgs.size(); i++)
    {
        for (std::size_t k = 0; k < vinArgs.size(); k++)
        {
            BigInt::Vin vinArg1 = vinArgs[i];
            BigInt::Vin vinArg2 = vinArgs[k];
            BigInt::Vin vinResult;

            std::string theOperation;
            // ---------------------------------------------
            theOperation = "+";
            vinResult = vinArg1 + vinArg2;

            oss << ""
                << strVinHex
                << vinArg1.toStr0xHex() 
                << " " 
                << theOperation 
                << " " 
                << vinArg2.toStr0xHex() 
                << " = " 
                << vinResult.toStr0xHex() 
                << std::endl;

            oss << ""
                << strVinDec
                << vinArg1  
                << " " 
                << theOperation 
                << " " 
                << vinArg2 
                << " = " 
                << vinResult
                << std::endl;

            oss << std::endl;

        } // for (std::size_t k = 0; k < rossiArgs.size(); k++)
    } // for (std::size_t i = 0; i < rossiArgs.size(); i++)


	SET_FINISH_TEST_NAME(oss);

    o_stream << std::flush << oss.str() << std::flush;
}



// ==================================
void BigInt::Run::runRossiInteractiveSample(std::ostream& o_stream, const std::vector<std::string>& i_args)
{
    std::ostringstream oss;

	SET_START_TEST_NAME(oss);

    ASSERTION (i_args.size() == 4);

    const std::map<std::string, std::string> allowedOperations = getSampleAllowedOperation();
    ASSERTION(!allowedOperations.empty());

    // ---------------------------
    BigInt::Rossi rossiArg1 (std::string(i_args[1]), DEC_DIGIT);
    std::string   op (i_args[2]);
    BigInt::Rossi rossiArg2 (std::string(i_args[3]), DEC_DIGIT);

    BigInt::Rossi rossiResult;

    ASSERTION (allowedOperations.count(op));

    BigInt::Vin vinArg1(rossiArg1);
    BigInt::Vin vinArg2(rossiArg2);

    // ---------------------------
    std::ostringstream ossFailure;
    if (op == "+") 
    {
        rossiResult = rossiArg1 + rossiArg2;
    }

    if (op == "-") 
    {
        if (rossiArg2 <= rossiArg1)
        {
            rossiResult = rossiArg1 - rossiArg2;
        }
        else
        {
            ossFailure << ""
                       << "Arguments are inconsistent for operator"
                       << op
                       << " : "
                       << " minuend = "
                       << vinArg1
                       << ", "
                       << "subtracter = "
                       << vinArg2
                       << ", i.e. minuend < subtracter";        
        }
    }

    if (op == "x") 
    {
        rossiResult = rossiArg1 * rossiArg2;
    }

    if (op == "/") 
    {
        rossiResult = rossiArg1 / rossiArg2;
    }

    if (op == "%") 
    {
        rossiResult = rossiArg1 % rossiArg2;
    }

    // ---------------------------
    BigInt::Vin vinResult(rossiResult);

    if (ossFailure.str().empty())
    {
        oss << ""
            << "[Dec] "
            << vinArg1 
            << " " 
            << op 
            << " " 
            << vinArg2 
            << " = " 
            << vinResult 
            << std::endl;

        oss << ""
            << "[Hex] "
            << rossiArg1 
            << " " 
            << op 
            << " " 
            << rossiArg2 
            << " = " 
            << rossiResult 
            << std::endl;

    }
    else
    {
        oss << ""
            << ossFailure.str()
            << std::endl;
    }

	SET_FINISH_TEST_NAME(oss);

    o_stream << std::flush << oss.str() << std::flush;
}


// ==========================================
void BigInt::BaseBigInt::showUnits(std::ostream& o_stream) const
{
    std::ostringstream oss;

    const std::string::size_type widthSpaces = static_cast<std::string::size_type>(BigInt::toString(m_units.size()).size());

    oss << "--- Units: BEGIN ---" 
        << std::endl;
    for (std::size_t i = 0; i < m_units.size(); i++)
    {
        oss << "m_units[" 
            << i 
            << "]"
            << std::string (widthSpaces, ' ')
            << ": " 
            << std::hex
            << std::showbase
            << m_units[i] 
            << std::dec
            << " = "
            << m_units[i] 
            << std::endl;
    }
    oss << "---- Units: END ----" 
        << std::endl;

    
    o_stream << std::flush << oss.str() << std::flush;
}



    


// ================
// class BigInt::BaseBigInt
// ================
// ------------------
bool BigInt::BaseBigInt::isEmpty() const
{
    return m_units.empty();
}



// ------------------
void BigInt::BaseBigInt::maximize()
{
    m_units.clear();

    const BigInt::Ulong maxUlong = std::numeric_limits<BigInt::Ulong>::max();

    while (true)
    {
        try
        {
            m_units.push_back(maxUlong);
        }
        catch(...)
        {
            // Do nothing
            break;
        }
    }
}



// ------------------
std::size_t BigInt::BaseBigInt::getUnitsSize() const
{
    return m_units.size();
}


// ------------------
BigInt::BaseBigInt::~BaseBigInt()
{
}



// =================
// class BigInt::Vin
// =================
// ----------------------------
BigInt::Ulong BigInt::Vin::s_carry = 0;
// ----------------------------
BigInt::Ulong BigInt::Vin::addUnits(BigInt::Ulong n1, BigInt::Ulong n2)
{
    n1 += (n2 + s_carry);
    s_carry = (OVER_BASE2 & n1) || (n1 > SUB_BASE2);
    return (s_carry ? (n1 - BASE2) : n1);
}



// ------------------
// Constructor-0
BigInt::Vin::Vin () 
{
}


// ------------------
// Constructor-1
BigInt::Vin::Vin (BigInt::Ulong i_unit) 
{
    if (!(i_unit < BASE2))
    {
        const std::size_t witdh1 = 19;

        std::ostringstream ossErr;
        ossErr  << ""
                << "BigInt::Ulong value too big: "
                << std::endl

                << std::setw (witdh1)
                << std::left
                << "i_unit"
                << std::right
                << " = " 
                << std::dec 
                << i_unit 
                << ", " 
                << std::hex 
                << std::showbase
                << i_unit 
                << std::dec
                << std::endl

                << std::setw (witdh1)
                << std::left
                << "BASE2"
                << std::right
                << " = "
                << BASE2 
                << ", " 
                << std::hex 
                << std::showbase
                << BASE2 
                << std::dec 
                << std::endl

                << std::setw (witdh1)
                << std::left
                << "MAX_UNIT_VALUE" 
                << std::right
                << " = "
                << MAX_UNIT_VALUE 
                << ", " 
                << std::hex 
                << std::showbase
                << MAX_UNIT_VALUE 
                << std::dec
                << std::endl
                << std::endl;

        ERR_MSG (std::cerr, ossErr.str());
       
    }
    ASSERTION (i_unit < BASE2);

    
    // ---------------------
    try
    {
        m_units.reserve(m_units.size() + 1);
    }
    catch(...)
    {
        std::ostringstream ossThrow;
        std::ostringstream ossErr;
        ossErr  << ""
                << "Unable to to do m_units.reserve("
                << (m_units.size() + 1)
                << ")"
                << std::endl;

        ERR_MSG (std::cerr, ossErr.str());
        ERR_MSG (ossThrow, ossErr.str());
        throw std::runtime_error (ossThrow.str().c_str());
    }

    
    // ---------------------
    try
    {
        m_units.push_back (i_unit);
    }
    catch(...)
    {
        std::ostringstream ossThrow;
        std::ostringstream ossErr;
        ossErr  << ""
                << "Unable to to do m_units.push_back()"
                << "; "
                << "m_units.size() = "
                << m_units.size()
                << std::endl;

        ERR_MSG (std::cerr, ossErr.str());
        ERR_MSG (ossThrow, ossErr.str());
        throw std::runtime_error (ossThrow.str().c_str());
    }

}



// ------------------
// Constructor-2
BigInt::Vin::Vin (const std::string& i_arg, std::size_t i_digitBase)
{
    bool rc = initViaString (i_arg, i_digitBase);
    ASSERTION (rc);
}

// ------------------
// Constructor-3
BigInt::Vin::Vin (const BigInt::Rossi& i_arg)
{
    const std::string str (i_arg.toStrHex ());
    bool rc = initViaString (str, HEX_DIGIT);
    ASSERTION (rc);
}



// ------------------
long double BigInt::Vin::toDouble () const
{
   if (m_units.empty()) 
    {
		return static_cast<long double>(0);
    }

    ASSERTION (!m_units.empty());

    // --------------
	double ret = 0.0;
    for (std::size_t i1 = (m_units.size() - 1); i1; i1--)
    {
		const std::size_t i = i1 - 1;
		ret = ret * static_cast<long double>(std::numeric_limits<Ulong>::max());
        ret = ret + static_cast<long double>(m_units [i]);
    }
    return ret;
}


// -----------------------
BigInt::Vin BigInt::Vin::fromDouble (const long double& i_double)
{
	BigInt::Vin ret;
    long double majorPart = i_double;
	long double minorPart = 0.0;

	const long double epsilon = 0.7;
	
	while (true)
	{
		minorPart = fmod (majorPart, std::numeric_limits<Ulong>::max());
		ret.m_units.push_back(Ulong(minorPart));

		majorPart = majorPart / std::numeric_limits<Ulong>::max();

		if (majorPart < epsilon)
		{
			break;
		}
	}

	return ret;

}

// ------------------
BigInt::Ulong BigInt::Vin::toUlong () const
{
    ASSERTION (!m_units.empty());
    if (m_units.size() > 1)
    {
        std::ostringstream ossErr;
        ossErr  << ""
                << "BigInt::Ulong can't be obtained: m_units.size() too big"
                << std::endl
                << "m_units.size() = " 
                << m_units.size() 
                << std::endl
                << "Here m_units.size() must be = 1"
                << std::endl
                << std::endl;

        ERR_MSG (std::cerr, ossErr.str());
        ASSERTION (m_units.size() == 1);
    }
    ASSERTION (m_units.size() == 1);
    return m_units.front();
}

// ------------------
// BigInt::Vin::operator BigInt::Ulong () const
// {
//  return toUlong();
// }


// ------------------
bool BigInt::Vin::initViaString (const std::string& i_arg, std::size_t i_digitBase)
{
    ASSERTION (m_units.empty());
    ASSERTION ((i_digitBase == BigInt::HEX_DIGIT) || (i_digitBase == BigInt::DEC_DIGIT));

    m_units.push_back(0);

    for (std::size_t i = 0; i < i_arg.size(); i++)
    {
        switch (i_digitBase)
        {
            case DEC_DIGIT:
                if (!isdigit (i_arg[i])) 
                {
                    std::ostringstream ossErr;
                    ossErr  << ""
                            << "std::string contains non-decimal digit"
                            << std::endl
                            << "std::string = " 
                            << i_arg 
                            << std::endl
                            << i 
                            << "-th char = " 
                            << i_arg[i] 
                            << std::endl
                            << std::endl;

                    ERR_MSG (std::cerr, ossErr.str());
              
                    ASSERTION (0);
                    return false; // unused
                }
                break;

            case HEX_DIGIT:
                if (!isxdigit (i_arg[i])) 
                {
                    std::ostringstream ossErr;
                    ossErr  << ""
                            << "std::string contains non-hexadecimal digit"
                            << std::endl
                            << "std::string = " 
                            << i_arg 
                            << std::endl
                            << i 
                            << "-th char = " 
                            << i_arg[i] 
                            << std::endl
                            << std::endl;

                    ERR_MSG (std::cerr, ossErr.str());
                    ASSERTION (0);
                    return false; // unused
                }
                break;

            default:
                ASSERTION (0);
                return false;
                break; // unused
        }  // switch (i_digitBase)
    } // for (std::size_t i = 0; i < i_arg.size(); i++)

    for (std::size_t i = 0; i < i_arg.size(); i++)
    {
        *this = (*this) * static_cast<BigInt::Ulong>(i_digitBase)
                + 
                BigInt::Vin (static_cast<BigInt::Ulong>(ms_hex2dec[i_arg[i]]));
    }
    return true;
}



// ------------------
BigInt::Vin BigInt::Vin::operator+ (const BigInt::Vin& i_arg) const
{
    BigInt::Vin ret;
    const std::size_t maxSize (std::max (m_units.size (), i_arg.m_units.size ()));

    std::vector<BigInt::Ulong> u1 (m_units);
    std::vector<BigInt::Ulong> u2 (i_arg.m_units);

    
    // ---------------------
    try
    {
        u1.reserve(maxSize);
    }
    catch(...)
    {
        std::ostringstream ossThrow;
        std::ostringstream ossErr;
        ossErr  << ""
                << "Unable to to do u1.reserve("
                << maxSize
                << ")"
                << std::endl;

        ERR_MSG (std::cerr, ossErr.str());
        ERR_MSG (ossThrow, ossErr.str());
        throw std::runtime_error (ossThrow.str().c_str());
    }

    // ---------------------
    try
    {
        u2.reserve(maxSize);
    }
    catch(...)
    {
        std::ostringstream ossThrow;
        std::ostringstream ossErr;
        ossErr  << ""
                << "Unable to to do u2.reserve("
                << maxSize
                << ")"
                << std::endl;

        ERR_MSG (std::cerr, ossErr.str());
        ERR_MSG (ossThrow, ossErr.str());
        throw std::runtime_error (ossThrow.str().c_str());
    }

    
    // ---------------------
    try
    {
        ret.m_units.reserve(maxSize + 1);
    }
    catch(...)
    {
        std::ostringstream ossThrow;
        std::ostringstream ossErr;
        ossErr  << ""
                << "Unable to to do ret.m_units.reserve("
                << (maxSize + 1)
                << ")"
                << std::endl;

        ERR_MSG (std::cerr, ossErr.str());
        ERR_MSG (ossThrow, ossErr.str());
        throw std::runtime_error (ossThrow.str().c_str());
    }

    // ---------------------
    try
    {
        u1.resize(maxSize);
    }
    catch(...)
    {
        std::ostringstream ossThrow;
        std::ostringstream ossErr;
        ossErr  << ""
                << "Unable to to do u1.resize("
                << maxSize
                << ")"
                << std::endl;

        ERR_MSG (std::cerr, ossErr.str());
        ERR_MSG (ossThrow, ossErr.str());
        throw std::runtime_error (ossThrow.str().c_str());
    }

    // ---------------------
    try
    {
        u2.resize(maxSize);
    }
    catch(...)
    {
        std::ostringstream ossThrow;
        std::ostringstream ossErr;
        ossErr  << ""
                << "Unable to to do u2.resize("
                << maxSize
                << ")"
                << std::endl;

        ERR_MSG (std::cerr, ossErr.str());
        ERR_MSG (ossThrow, ossErr.str());
        throw std::runtime_error (ossThrow.str().c_str());
    }

    // ---------------------
    try
    {
        ret.m_units.resize(maxSize);
    }
    catch(...)
    {
        std::ostringstream ossThrow;
        std::ostringstream ossErr;
        ossErr  << ""
                << "Unable to to do ret.m_units.resize("
                << maxSize
                << ")"
                << std::endl;

        ERR_MSG (std::cerr, ossErr.str());
        ERR_MSG (ossThrow, ossErr.str());
        throw std::runtime_error (ossThrow.str().c_str());
    }



    s_carry = 0;
    std::transform (&u1[0], &u1[0] + maxSize, &u2[0], &ret.m_units[0], BigInt::Vin::addUnits);
  
    if (s_carry) 
    {
        try
        {
            ret.m_units.push_back (s_carry);
        }
        catch(...)
        {
            std::ostringstream ossThrow;
            std::ostringstream ossErr;
            ossErr  << ""
                    << "Unable to to do ret.m_units.push_back()"
                    << "; "
                    << "ret.m_units.size() = "
                    << ret.m_units.size()
                    << std::endl;

            ERR_MSG (std::cerr, ossErr.str());
            ERR_MSG (ossThrow, ossErr.str());
            throw std::runtime_error (ossThrow.str().c_str());
        }

    }

    return ret;

}


// ------------------
BigInt::Vin BigInt::Vin::operator* (BigInt::Ulong i_arg) const
{
    BigInt::Vin ret (0);
    for (std::size_t i = 0; i < i_arg; i++) 
    {
        ret = ret + (*this);
    }
    return ret;
}


// ------------------
bool BigInt::Vin::operator< (const BigInt::Vin& i_arg) const
{
    if (m_units.size() < i_arg.m_units.size())
    {
        return true;
    }
    if (i_arg.m_units.size() < m_units.size())
    {
        return false;
    }

    ASSERTION (m_units.size() == i_arg.m_units.size());
    for (std::size_t i = (m_units.size() - 1); i > 0; i--)
    {
        if (m_units[i] < i_arg.m_units[i])
        {
            return true;
        }

        if (i_arg.m_units[i] < m_units[i])
        {
            return false;
        }
    }
    return (m_units[0] < i_arg.m_units[0]);
}


// ------------------
bool BigInt::Vin::operator<= (const BigInt::Vin& i_arg) const
{
    if (*this < i_arg) 
    {
        return true;
    }

    if (*this == i_arg) 
    {
        return true;
    }

    return false;
}



// ------------------
bool BigInt::Vin::operator> (const BigInt::Vin& i_arg) const
{
    return (!(*this <= i_arg));
}


// ------------------
bool BigInt::Vin::operator>= (const BigInt::Vin& i_arg) const
{
    return (!(*this < i_arg));
}

// ------------------
bool BigInt::Vin::operator== (const BigInt::Vin& i_arg) const
{
    if (*this < i_arg)
    {
        return false;
    }

    if (i_arg < *this) 
    {
        return false;
    }

    return true;
}

// ------------------
bool BigInt::Vin::operator!= (const BigInt::Vin& i_arg) const
{
    return (!(*this == i_arg));
}


// ------------------
std::string BigInt::Vin::toStrHex (const std::string& i_prefix) const
{
    std::ostringstream oss;

    BigInt::Rossi rossi (toStrDec(), BigInt::DEC_DIGIT);

    oss << i_prefix
        << rossi.toStrHex();

    return oss.str();
}


// ------------------
std::string BigInt::Vin::toStr0xHex () const
{
    return toStrHex("0x");
}


// ------------------
std::string BigInt::Vin::toStrDec (const std::string& i_prefix) const
{
    std::ostringstream oss;

    if (isEmpty ()) 
    {
        return "isEmpty";
    }

    oss << i_prefix;

    for (std::size_t i = (m_units.size() - 1); i; --i) 
    {
        oss << m_units [i] 
            << std::setw (BigInt::BASE1 - 1) 
            << std::setfill ('0');
    }
    oss << m_units [0];

    return oss.str();
}


// ------------------
std::ostream& operator<< (std::ostream& o_os, const BigInt::Vin& i_ins)
{
    return o_os << i_ins.toStrDec();
}


// ==========================================
void BigInt::Vin::showUnits(std::ostream& o_stream) const
{
    std::ostringstream oss;

    BigInt::Rossi rossi(*this);

    oss << std::endl;
    oss << ""
        << "BigInt::Vin value"
        << ": "
        << rossi.toStr0xHex()
        << " = "       
        << this->toStrDec()
        << std::endl;

    BigInt::BaseBigInt::showUnits(oss);

    oss << std::endl;

    o_stream << std::flush << oss.str() << std::flush;
}


// ===================
// class BigInt::Rossi
// ===================


// ------------------
// Constructor-0
BigInt::Rossi::Rossi () 
{
    ASSERTION(isEmpty());
}


// ------------------
// Constructor-1
BigInt::Rossi::Rossi (BigInt::Ulong i_unit) 
{
    ASSERTION (m_units.empty());
    m_units.push_back (i_unit);
}


// ------------------
// Constructor-2
BigInt::Rossi::Rossi (const std::string& i_arg, std::size_t i_digitBase) 
{
    const bool rc = initViaString (i_arg, i_digitBase);
    ASSERTION (rc);
}



// ------------------
// Constructor-3
BigInt::Rossi::Rossi (const BigInt::Vin& i_arg)
{
    std::ostringstream oss;
    oss << i_arg;
    const bool rc = initViaString (oss.str(), DEC_DIGIT);
    ASSERTION (rc);
}


// ------------------
// Constructor-4
BigInt::Rossi::Rossi (const std::size_t i_noOfUnits, BigInt::Ulong i_internalUnits, BigInt::Ulong i_backUnit, const std::string& i_msg)
{
    ASSERTION (m_units.empty());

    try
    {
        m_units.resize (i_noOfUnits, i_internalUnits);      
    }
    catch(...)
    {
        std::ostringstream ossThrow;
        std::ostringstream ossErr;
        ossErr  << ""
                << "Unable to do m_units.resize("
                << i_noOfUnits
                << ")"
				<< i_msg
                << std::endl;

            ERR_MSG (std::cerr, ossErr.str());
            ERR_MSG (ossThrow, ossErr.str());
            throw std::runtime_error (ossThrow.str().c_str());
    }

    m_units.back() = i_backUnit;
}


// ------------------
long double BigInt::Rossi::toDouble () const
{
    if (m_units.empty()) 
    {
		return static_cast<long double>(0);
    }

    ASSERTION (!m_units.empty());

    // --------------
	long double ret = 0.0;
    for (std::size_t i1 = m_units.size(); i1; i1--)
    {
		ret = ret * static_cast<long double>(std::numeric_limits<Ulong>::max());
        ret = ret + static_cast<long double>(m_units [i1 - 1]);
    }

    return ret;
}


// --------------------------------
BigInt::Rossi BigInt::Rossi::fromDouble (const long double& i_double)
{
	BigInt::Rossi ret;
    long double majorPart = i_double;
	long double minorPart = 0.0;

	const long double epsilon = 0.7;

	
	while (true)
	{
		minorPart = fmod (majorPart, std::numeric_limits<Ulong>::max());
		ret.m_units.push_back(Ulong(minorPart));

		majorPart = majorPart / std::numeric_limits<Ulong>::max();

		if (majorPart < epsilon)
		{
			break;
		}
	}

	ret.smartTruncateUnits();
	return ret;
}


// ------------------
BigInt::Ulong BigInt::Rossi::toUlong () const
{
    ASSERTION (!m_units.empty());
    if (m_units.size() > 1)
    {
        std::ostringstream ossErr;
        ossErr  << ""
                << "BigInt::Ulong can't be obtained: m_units.size() too big"
                << std::endl
                << "m_units.size() = " 
                << m_units.size() 
                << std::endl
                << "Hex value = " 
                << toStrHex() 
                << std::endl
                << "Here m_units.size() must be = 1"
                << std::endl
                << std::endl;

        ERR_MSG (std::cerr, ossErr.str());

        ASSERTION (m_units.size() == 1);
    }
    ASSERTION (m_units.size() == 1);
    return m_units.front();
}

// ------------------
// BigInt::Rossi::operator BigInt::Ulong () const
// {
//  return toUlong();
// }


// ------------------
bool BigInt::Rossi::initViaString (const std::string& i_arg, std::size_t i_digitBase)
{
    ASSERTION (m_units.empty());
    ASSERTION ((i_digitBase == BigInt::HEX_DIGIT) || (i_digitBase == BigInt::DEC_DIGIT));

    m_units.push_back(0);

    for (std::size_t i = 0; i < i_arg.size(); i++)
    {
        switch (i_digitBase)
        {
            case DEC_DIGIT:
                if (!isdigit (i_arg[i])) 
                {
                    std::ostringstream ossErr;
                    ossErr  << ""
                            << "std::string contains non-decimal digit"
                            << std::endl
                            << "std::string = " 
                            << i_arg 
                            << std::endl
                            << i 
                            << "-th char = " 
                            << i_arg[i] 
                            << std::endl
                            << std::endl;

                    ERR_MSG (std::cerr, ossErr.str());

                    ASSERTION (0);
                    return false; // unused
                }
                break;

            case HEX_DIGIT:
                if (!isxdigit (i_arg[i])) 
                { 
                    std::ostringstream ossErr;
                    ossErr  << ""
                            << "std::string contains non-hexadecimal digit"
                            << std::endl
                            << "std::string = " 
                            << i_arg 
                            << std::endl
                            << i 
                            << "-th char = " 
                            << i_arg[i] 
                            << std::endl
                            << std::endl;

                    ERR_MSG (std::cerr, ossErr.str());
                    ASSERTION (0);
                    return false; // unused
                }
            break;

        default:
            ASSERTION (0);
            return false;
            break; // unused

        }
    }

    for (std::size_t i = 0; i < i_arg.size(); i++)
    {
        *this = (*this) * static_cast<BigInt::Ulong>(i_digitBase)
                + 
                BigInt::Rossi (static_cast<BigInt::Ulong>(ms_hex2dec[i_arg[i]]));
    }
    return true;
}


// ------------------
void BigInt::Rossi::resizeUnits(std::size_t i_size)
{
    m_units.resize(i_size);
}

// ------------------
void BigInt::Rossi::truncateUnits()
{
    while ((m_units.size() > 1) && (m_units.back() == 0))
    {
        m_units.pop_back();
    } 
    ASSERTION(!isEmpty());

}


// ------------------
void BigInt::Rossi::smartTruncateUnits()
{
    if (backUnitIsNull ())
    { 
  	    truncateUnits ();
    }
}

// ------------------
bool BigInt::Rossi::backUnitIsNull () const
{
    ASSERTION (!m_units.empty());
    if (m_units.size() == 1) 
    {
        return false;
    }
    return (m_units.back() == 0);
}



// ------------------
// BigInt::Rossi& BigInt::Rossi::operator= (BigInt::Ulong i_arg)
// {
//  *this = BigInt::Rossi (i_arg);
//  return *this;
// }

// ------------------
BigInt::Rossi BigInt::Rossi::operator+ (const BigInt::Rossi& i_arg)
{
    BigInt::Rossi ret (0);
    BigInt::Rossi arg (i_arg);
    BigInt::Ulong carry = 0;

    const std::size_t maxSize (std::max (getUnitsSize(), arg.getUnitsSize()));

    resizeUnits (maxSize + 1);
    arg.resizeUnits (maxSize + 1);
    ret.resizeUnits (maxSize + 1);

    for (std::size_t i = 0; i < m_units.size(); i++)
    {  	 
        ret.m_units[i] = m_units[i] + arg.m_units[i] + carry;
        if (carry == 0)
        {
            carry = ((ret.m_units[i] < m_units[i] || ret.m_units[i] < arg.m_units[i]) ? 1 : 0);
        }
        else
        {
            carry = ((ret.m_units[i] <= m_units[i] || ret.m_units[i] <= arg.m_units[i]) ? 1 : 0);
        }
    }

    smartTruncateUnits();
    ret.smartTruncateUnits();

    return ret;

}

// ------------------
BigInt::Rossi BigInt::Rossi::operator+ (BigInt::Ulong i_arg)
{
    return (*this + BigInt::Rossi (i_arg));
}


// ------------------
bool BigInt::Rossi::operator< (const BigInt::Rossi& i_arg) const
{
    if (m_units.size() < i_arg.m_units.size()) 
    {
        return true;
    }
    if (i_arg.m_units.size() < m_units.size())
    {
        return false;
    }

    ASSERTION (m_units.size() == i_arg.m_units.size());
    for (std::size_t i = (m_units.size() - 1); i > 0; i--)
    {
        if (m_units[i] < i_arg.m_units[i]) 
        {
            return true;
        }

        if (i_arg.m_units[i] < m_units[i]) 
        {
            return false;
        }
    }
    return (m_units[0] < i_arg.m_units[0]);
}


// ------------------
bool BigInt::Rossi::operator<= (const BigInt::Rossi& i_arg) const
{

    if (*this < i_arg) 
    {
        return true;
    }
    if (*this == i_arg) 
    {
        return true;
    }

    return false;
}



// ------------------
bool BigInt::Rossi::operator> (const BigInt::Rossi& i_arg) const
{
  return (!(*this <= i_arg));
}


// ------------------
bool BigInt::Rossi::operator>= (const BigInt::Rossi& i_arg) const
{
    return (!(*this < i_arg));
}

// ------------------
bool BigInt::Rossi::operator== (const BigInt::Rossi& i_arg) const
{
    if (*this < i_arg) 
    {
        return false;
    }

    if (i_arg < *this) 
    {
        return false;
    }

    return true;
}


// ------------------
bool BigInt::Rossi::operator!= (const BigInt::Rossi& i_arg) const
{
    return (!(*this == i_arg));
}



// ------------------
BigInt::Rossi BigInt::Rossi::operator/ (const BigInt::Rossi& i_arg) const
{
    return divide(*this, i_arg, NULL);
}


// ------------------
BigInt::Rossi BigInt::Rossi::operator% (const BigInt::Rossi& i_arg) const
{
    BigInt::Rossi ret;
    divide(*this, i_arg, &ret);
    return ret;
}



// ------------------
BigInt::Rossi BigInt::Rossi::operator>> (std::size_t i_shift)
{
    BigInt::Rossi tmp;
    BigInt::Rossi ret;

    tmp = *this;

    ret.resizeUnits (m_units.size());

    ASSERTION (ret.getUnitsSize() == tmp.getUnitsSize());

    for (std::size_t i = 0; i < i_shift; i++)
    {
        ret.m_units.back() = (tmp.m_units.back() >> 1);

        for (std::size_t j1 = tmp.m_units.size(); j1 ; j1--)
        {
            const std::size_t j = j1 -1;
            ret.m_units[j] = (tmp.m_units[j] >> 1);

            if ((tmp.m_units[j + 1] & 1) != 0)
            {
                ret.m_units[j] |= ULONG_MSB;      // Set MSB bit
            }
        }
        tmp = ret;
    }

    smartTruncateUnits();
    ret.smartTruncateUnits();

    return ret;
}


// ------------------
BigInt::Rossi BigInt::Rossi::operator<< (std::size_t i_shift)
{
    BigInt::Rossi tmp;
    BigInt::Rossi ret;

    tmp = *this;

    ret.resizeUnits (m_units.size() + 1);

    ASSERTION ((ret.getUnitsSize() + 1) == tmp.getUnitsSize());

    for (std::size_t i = 0; i < i_shift; i++)
    {
        ret.m_units.front() = (tmp.m_units.front() << 1);
        for (std::size_t j = 1; j < tmp.m_units.size(); j++)
        {
            ret.m_units[j] = (tmp.m_units[j] << 1);
            if ((tmp.m_units[j-1] & ULONG_MSB) != 0)
            {
                ret.m_units[j] |= 1;     // Set MSB bit
            }
        }
        if ((tmp.m_units.back() & ULONG_MSB) != 0)
        {
            ret.m_units.back() |= 1;   // Set MSB bit
        }
        tmp = ret;
    }

    smartTruncateUnits();
    ret.smartTruncateUnits();

    return ret;
}



// ------------------
BigInt::Rossi& BigInt::Rossi::operator>>= (std::size_t i_shift)
{
    BigInt::Ulong carry;
    m_units.push_back(0);

    for (std::size_t i = 0; i < i_shift; i++)
    {
        carry = m_units.back() & 1;
        m_units.back() >>= 1;

        for (std::size_t j1 = m_units.size(); j1; j1--)
        {
            const std::size_t j = j1 -1;
            if (carry)
            {
                carry = m_units[j] & 1;
                m_units[j] >>= 1;
                m_units[j] |= ULONG_MSB;
            }
            else
            {
                carry = m_units[j] & 1;
                m_units[j] >>= 1;
            }
        }
    }

    smartTruncateUnits();

    return *this;
}



// ------------------
BigInt::Rossi& BigInt::Rossi::operator<<= (std::size_t i_shift)
{
    BigInt::Ulong carry;

    const std::size_t pushBackSize (i_shift/(sizeof (std::size_t) * CHAR_BIT) + 1);

    for (std::size_t i = 0; i < (pushBackSize + 1); i++)
    {
        try
        {
            m_units.push_back(0);        
        }
        catch(...)
        {
            std::ostringstream ossThrow;
            std::ostringstream ossErr;
            ossErr  << ""
                    << "Unable to to do m_units.push_back()"
                    << "; "
                    << "m_units.size() = "
                    << m_units.size()
                    << std::endl;

            ERR_MSG (std::cerr, ossErr.str());
            ERR_MSG (ossThrow, ossErr.str());
            throw std::runtime_error (ossThrow.str().c_str());
        }
    }

    for (std::size_t i = 0; i < i_shift; i++)
    {
        carry = m_units.front() & ULONG_MSB;
        m_units.front() <<= 1;

        for (std::size_t j = 1; j < m_units.size(); j++)
        {
            if (carry)
            {
                carry = m_units[j] & ULONG_MSB;
                m_units[j] <<= 1;
                m_units[j] |= 1;
            }
            else
            {
                carry = m_units[j] & ULONG_MSB;
                m_units[j] <<= 1;
            }
        }
    }

    smartTruncateUnits();

    return *this;
}



// ------------------
BigInt::Rossi& BigInt::Rossi::operator+=(const BigInt::Rossi& i_arg)
{
    BigInt::Ulong carry = 0;
    BigInt::Ulong prevDigit;
    BigInt::Rossi arg (i_arg);

    const std::size_t maxSize (std::max (getUnitsSize(), arg.getUnitsSize()));

    resizeUnits (maxSize + 1);
    arg.resizeUnits (maxSize + 1);

    for (std::size_t i = 0; i < m_units.size(); i++)
    {
        prevDigit = m_units[i];
        m_units[i] = m_units[i] + arg.m_units[i] + carry;
        if (carry == 0)
        {
            carry = ((m_units[i] < prevDigit || m_units[i] < arg.m_units[i]) ? 1 : 0);
        }
        else
        {
        carry = ((m_units[i] <= prevDigit || m_units[i] <= arg.m_units[i]) ? 1 : 0);
        }
    }

    smartTruncateUnits();
    return *this;
}


// ------------------
BigInt::Rossi& BigInt::Rossi::operator++()  // Pre Increment operator -- faster than add
{
    m_units.push_back(0);

    m_units.front()++;
    for (std::size_t i = 1; i < m_units.size(); i++)
    {
        if (m_units[i-1]) 
        {
            break;
        }
        m_units[i]++;
    }

    smartTruncateUnits();

    return *this;
}


// ------------------
BigInt::Rossi BigInt::Rossi::operator++ (int)  // Post Increment operator -- faster than add
{
    BigInt::Rossi tmp (*this);
    ++*this;
    return tmp;
}




// ------------------
BigInt::Rossi BigInt::Rossi::operator- ()  // Negates a number
{
    BigInt::Rossi ret;
    ret.resizeUnits(m_units.size() + 1);

    for (std::size_t i = 0; i < m_units.size(); i++)
    {
        ret.m_units[i] = ~m_units[i];
    }

    ret = ret + RossiOne;

    smartTruncateUnits();
    ret.smartTruncateUnits();

    return ret;
}



// ------------------
BigInt::Rossi BigInt::Rossi::operator-(const BigInt::Rossi& i_arg)
{
    BigInt::Rossi ret (RossiZero);
    BigInt::Rossi arg (i_arg);
    BigInt::Ulong borrow = 0;

    const std::size_t maxSize (std::max (getUnitsSize(), arg.getUnitsSize()));

    resizeUnits (maxSize + 1);
    arg.resizeUnits (maxSize + 1);
    ret.resizeUnits (maxSize + 1);

    if (*this < arg)
    {
        std::ostringstream ossErr;
        ossErr  << ""
                << "minuend < subtracter"
                << std::endl
                << "minuend    = " 
                << this->toStrHex() 
				<< ", "
                << "subtracter = " 
                << arg.toStrHex() 
                << std::endl
                << std::endl;

        ERR_MSG (std::cerr, ossErr.str());
        ASSERTION (0);
    }

    for (std::size_t i = 0; i < m_units.size(); i++)
    {
        ret.m_units[i] = m_units[i] - arg.m_units[i] - borrow;
    
        if (borrow == 0)
        {
            borrow = ((m_units[i] < arg.m_units[i]) ? 1 : 0);
        }
        else
        {
        borrow = ((m_units[i] <= arg.m_units[i]) ? 1 : 0);
        }
    }

    smartTruncateUnits();
    ret.smartTruncateUnits();

    return ret;
}


// ------------------
BigInt::Rossi& BigInt::Rossi::operator-= (const BigInt::Rossi& i_arg)
{
    BigInt::Ulong borrow = 0;
    BigInt::Rossi arg (i_arg);
    BigInt::Ulong prevDigit;

    const std::size_t maxSize (std::max (getUnitsSize(), arg.getUnitsSize()));
	resizeUnits (maxSize + 1);
    arg.resizeUnits (maxSize + 1);
  
    if (*this < arg)
    {
        std::ostringstream ossErr;
        ossErr  << ""
                << "minuend < subtracter"
                << std::endl
                << "minuend    = " 
                << this->toStrHex() 
                << ", "
				<< "subtracter = " 
                << arg.toStrHex() 
                << std::endl
                << std::endl;

        ERR_MSG (std::cerr, ossErr.str());
        ASSERTION (0);
    }

    for (std::size_t i = 0; i < m_units.size(); i++)
    {
        prevDigit = m_units[i];
        m_units[i] = m_units[i] - arg.m_units[i] - borrow;

        if (borrow == 0)
        {
            borrow = ((prevDigit < arg.m_units[i]) ? 1 : 0);
        }
        else
        {
            borrow = ((prevDigit <= arg.m_units[i]) ? 1 : 0);
        }
    }

    smartTruncateUnits();

    return *this;
}




// ------------------
BigInt::Rossi& BigInt::Rossi::operator--()  // Pre Decrement operator -- faster than add
{
    m_units.front()--;
    for (std::size_t i = 1; i < m_units.size(); i++)
    {
        if (m_units[i-1] != ULONG_MAX) 
        {
            break;
        }
        ASSERTION (m_units[i-1] < ULONG_MAX);

        m_units[i]--;
    }

    smartTruncateUnits();
    return *this;
}


// ------------------
BigInt::Rossi BigInt::Rossi::operator-- (int)  // Post Decrement operator -- faster than add
{
    BigInt::Rossi tmp (*this);
    --*this;
    return tmp;
}



// ------------------
BigInt::Rossi BigInt::Rossi::operator& (const BigInt::Rossi& i_arg)
{
    const std::size_t maxSize (std::max (getUnitsSize (), i_arg.getUnitsSize ()));

    BigInt::Rossi ret;
    BigInt::Rossi arg (i_arg);

    ret.resizeUnits(maxSize);
    arg.resizeUnits(maxSize);
    resizeUnits(maxSize);

    for (std::size_t i1 = m_units.size(); i1; i1--)
    {
        const std::size_t i = i1 - 1;
        ret.m_units[i] = m_units[i] & arg.m_units[i];
    }

    smartTruncateUnits();
    ret.smartTruncateUnits();

    return ret;
}


// ------------------
BigInt::Rossi BigInt::Rossi::operator| (const BigInt::Rossi& i_arg)
{
    const std::size_t maxSize (std::max (getUnitsSize (), i_arg.getUnitsSize ()));

    BigInt::Rossi ret;
    BigInt::Rossi arg (i_arg);

    ret.resizeUnits(maxSize);
    arg.resizeUnits(maxSize);
    resizeUnits(maxSize);

    for (std::size_t i1 = m_units.size(); i1; i1--)
    {
        const std::size_t i = i1 - 1;
        ret.m_units[i] = m_units[i] | arg.m_units[i];
    }

   
    smartTruncateUnits();
    ret.smartTruncateUnits();

    return ret;
}



// ------------------
BigInt::Rossi BigInt::Rossi::operator^ (const BigInt::Rossi& i_arg)
{
    const std::size_t maxSize (std::max (getUnitsSize (), i_arg.getUnitsSize ()));

    BigInt::Rossi ret;
    BigInt::Rossi arg (i_arg);

    ret.resizeUnits(maxSize);
    arg.resizeUnits(maxSize);
    resizeUnits(maxSize);

    for (std::size_t i1 = m_units.size(); i1; i1--)     
    {
        const std::size_t i = i1 - 1;
        ret.m_units[i] = m_units[i] ^ arg.m_units[i];
    }

   
    smartTruncateUnits();
    ret.smartTruncateUnits();

    return ret;

}


// ------------------
BigInt::Rossi BigInt::Rossi::operator~ ()
{
    BigInt::Rossi ret;

    ret.resizeUnits(getUnitsSize());

    for (std::size_t i1 = m_units.size(); i1; i1--)
    {
        const std::size_t i = i1 - 1;
        ret.m_units[i] = ~m_units[i];
    }

   
    smartTruncateUnits();
    ret.smartTruncateUnits();

    return ret;

}



// ------------------
BigInt::Rossi& BigInt::Rossi::operator&= (const BigInt::Rossi& i_arg)
{
    const std::size_t maxSize (std::max  (getUnitsSize (), i_arg.getUnitsSize ()));

    BigInt::Rossi arg (i_arg);

    arg.resizeUnits(maxSize);
    resizeUnits(maxSize);

    for (std::size_t i1 = m_units.size(); i1; i1--) 
    {
        const std::size_t i = i1 - 1; 
        m_units[i] = m_units[i] & arg.m_units[i];
    }

    smartTruncateUnits();
 
    return *this;

}


// ------------------
BigInt::Rossi& BigInt::Rossi::operator|=(const BigInt::Rossi& i_arg)
{
    const std::size_t maxSize (std::max (getUnitsSize (), i_arg.getUnitsSize ()));

    BigInt::Rossi arg (i_arg);

    arg.resizeUnits(maxSize);
    resizeUnits(maxSize);

    for (std::size_t i1 = m_units.size(); i1; i1--)
    {
        const std::size_t i = i1 - 1; 
        m_units[i] = m_units[i] | arg.m_units[i];
    }

    smartTruncateUnits();

    return *this;

}


// ------------------
BigInt::Rossi& BigInt::Rossi::operator^=(const BigInt::Rossi& i_arg)
{
    const std::size_t maxSize (std::max  (getUnitsSize (), i_arg.getUnitsSize ()));

    BigInt::Rossi arg (i_arg);

    arg.resizeUnits(maxSize);
    resizeUnits(maxSize);

    for (std::size_t i1 = m_units.size(); i1; i1--)
    {
        const std::size_t i = i1 - 1; 
        m_units[i] = m_units[i] ^ arg.m_units[i];
    }

    smartTruncateUnits();
    return *this;

}




// ------------------
BigInt::Rossi BigInt::Rossi::operator* (BigInt::Rossi i_arg) const
{
    BigInt::Rossi tmp;
    BigInt::Rossi ret;

 
    tmp = *this;
    ret = RossiZero;
    // ret.resizeUnits (getUnitsSize () + i_arg.getUnitsSize ());
  
    do
    {	
        if ((i_arg.m_units.front() & 1) != 0) 
        {
            ret += tmp;
        }
        i_arg >>= 1;
        tmp <<= 1;
    } while (i_arg != RossiZero);

    const_cast<BigInt::Rossi*>(this)->smartTruncateUnits();
    ret.smartTruncateUnits();

    return ret;
}


// ------------------
BigInt::Rossi BigInt::Rossi::operator* (BigInt::Ulong i_arg) const
{
    return ((*this) * BigInt::Rossi (i_arg));
}

				 

// ------------------
BigInt::Rossi BigInt::Rossi::divide (
		                    const BigInt::Rossi& i_dividend, 
		                    const BigInt::Rossi& i_divisor, 
		                    BigInt::Rossi*       o_remainder
		                    ) const
{
    BigInt::Rossi dividend (i_dividend); 
    BigInt::Rossi divisor (i_divisor); 


    long shiftcnt (0);

    // --- Check for attempt to divide by zero ---
    if (divisor == RossiZero)
    {
        ERR_MSG (std::cerr, std::string("divisor == Zero"));
        ASSERTION (0 && "divisor == Zero");

        shiftcnt = 1 / shiftcnt;  // Force a divide by zero exception. (shiftcnt=0)
    }

    BigInt::Rossi quotient (RossiZero);

    quotient.resizeUnits (dividend.getUnitsSize ());

    if (o_remainder != NULL)
    {
        o_remainder->resizeUnits (dividend.getUnitsSize ());
    }


    // --- Left shift divisor until it is greater than or equal to dividend ---
    // while ( divisor < dividend && ((divisor.m_units.back() & ULONG_MSB) == 0))
    while ( divisor < dividend)
    { 	
        divisor <<= 1;
        shiftcnt++;
    }

    if (divisor > dividend)      // If divisor is greater than dividend, right shift divisor
    {
        divisor >>= 1;
        shiftcnt--;   
    }

    if (shiftcnt >= 0)
    { 	
        for(long i = 0; i <= shiftcnt; i++)
        {    	
            if ( divisor <= dividend)  // If divisor is greater than dividend, then the bit is a 1 
            {       	
                dividend -= divisor;     // Subtract divisor from dividend 
                divisor  >>= 1;          // Right shift divisor 
                quotient <<= 1;          // Left shift quotient
                quotient++;              // Increment quotient           // Increment quotient 
            }
            else
            {      	
                divisor >>= 1;             // Bit is 0, right shift divisor, left shift quotient
                quotient <<= 1;
            }
        }
    }

    BigInt::Rossi remainder (dividend); 
    remainder.smartTruncateUnits ();	

    if (o_remainder != NULL) 
    {
        *o_remainder = remainder;
        o_remainder->smartTruncateUnits ();
    }

    quotient.smartTruncateUnits ();

    ASSERTION ((quotient * i_divisor + remainder) == i_dividend);
    return quotient;
}


BigInt::Rossi BigInt::Rossi::naive_sqrt()		// Returns the square root of this
{
	if (*this == RossiZero)
	{
		return RossiZero;
	}

	if (*this == RossiOne)
	{
		return RossiOne;
	}

	if (*this == RossiTwo)
	{
		return RossiOne;
	}

	// -----------------------
	BigInt::Rossi result;

	const long double doubleValue = toDouble();

	const BigInt::Rossi reverseRossi = fromDouble(doubleValue); 
	// ASSERTION (m_units.size() == reverseRossi.m_units.size());
	// ASSERTION (*this == reverseRossi);

	const long double doubleSquare = std::sqrt(doubleValue);

	BigInt::Rossi reverse = fromDouble(doubleSquare);
	BigInt::Rossi curCandidate = reverse;

	while (true)
	{
		if ((curCandidate * curCandidate) <= *this)
		{
			break;
		}
		curCandidate = curCandidate / RossiTwo;
	}

    const std::size_t startFactor = 4096 * 4096;
	for (std::size_t factor = startFactor; factor > 1; factor/=2)
	{
		const BigInt::Rossi theFactor (factor);
		BigInt::Rossi delta (1); 
		// -----------------------
		while (true)
		{
			if (!((curCandidate * curCandidate) <= *this))
			{
				break;
			}
			delta = delta * theFactor;	
			curCandidate = curCandidate + delta;
		}
        ASSERTION ((curCandidate * curCandidate) > *this);
		curCandidate = curCandidate - delta;
		ASSERTION ((curCandidate * curCandidate) <= *this);
	}
	

	// ----------------------------------
	
	while ((curCandidate * curCandidate) <= *this)
	{
		curCandidate++;
	}
    ASSERTION ((curCandidate * curCandidate) > *this);
	curCandidate--;
	ASSERTION ((curCandidate * curCandidate) <= *this);

	return curCandidate;

	// =============================================
	if (*this == RossiZero)
	{
		return RossiZero;
	}

	if (*this == RossiOne)
	{
		return RossiOne;
	}

	if (*this == RossiTwo)
	{
		return RossiOne;
	}

	BigInt::Rossi ret = RossiTwo;

	BigInt::Rossi prev_ret = RossiZero;
    while (true)
	{
	   BigInt::Rossi tmp = ret * ret;
	   if (tmp > *this)
	   {
		   break;
	   }
	   prev_ret = ret;
       ret = tmp;
    }
	ASSERTION ((ret * ret) > *this);
	ASSERTION ((prev_ret * prev_ret) <= *this);


	// ------------------------------
	BigInt::Rossi delta2 = RossiOne;

	ret = prev_ret;
	if (ret == RossiZero)
	{
		ret = RossiOne;
	}

	while (true)
	{
		BigInt::Rossi tmp = ret * ret;
		if (tmp > *this)
		{
		   break;
		}
		ret = ret * delta2;
		delta2 = delta2 * RossiTwo;
		 
	}
	ASSERTION ((ret * ret) > *this);
	ret = ret / delta2;
	ASSERTION ((ret * ret) <= *this);


	// ------------------------------
	while (true)
	{
		BigInt::Rossi tmp = ret * ret;
		if (tmp > *this)
		{
		   break;
		}
		ret++;
	}
	ASSERTION ((ret * ret) > *this);
	ret--;
	ASSERTION ((ret * ret) <= *this);

	// ---------------------------
    return ret;
}


// ------------------
BigInt::Rossi BigInt::Rossi::sqrt()		// Returns the square root of this
{
	return this->naive_sqrt();

	// --------------
    
	if (*this == RossiZero)
	{
		return RossiZero;
	}

	BigInt::Rossi ret;
    BigInt::Rossi tmp;
    BigInt::Rossi delta;

    BigInt::Rossi mask (RossiTwo);

    tmp = *this;
    mask = -mask;

    std::size_t i = 0;
    ret = tmp; 
    for (i = 0; ret != RossiZero; ret >>= 1, i++)
    {
        // Do nothing
    }

    ret = tmp; 
    for (std::size_t j = 0; j < std::size_t(i >> 1); ret >>= 1, j++)
    {
        // Do nothing
    }

    do
    {
        // -----------------------------------------------
        // We are really performing the fuction:
        //	 delta = (tmp/ret - ret) / 2;
        //   below, but since these are unsigned numbers,
        //   we MUST do the subtraction last in order for
        //   the ret = ret + delta;  equation to work properly.
        // -----------------------------------------------
	
        // delta = ( tmp >> BigInt::Ulong(1)) / ret - (ret >> BigInt::Ulong(1));

		// -----------------------------------
		BigInt::Rossi tmp1 = (( tmp >> BigInt::Ulong(1)) / ret);
		BigInt::Rossi tmp2 = (ret >> BigInt::Ulong(1));
		// -----------------------------------

		if (tmp1 <= tmp2)
		{

			delta = tmp2 - tmp1;
			//ASSERTION (delta <= ret);

			if (delta <= ret)
			{
				ret = ret - delta;
			}
			else
			{
				return this->naive_sqrt();
			}

		}
		else
		{
			delta = tmp1 - tmp2;
			ret = ret + delta;
		}

    } while ((delta &= mask) != RossiZero);

	if ((ret * ret) > *this)
	{
		ret--;
	}
	ASSERTION (((ret + RossiOne) * (ret  + RossiOne)) > *this);
	ASSERTION ((ret * ret) <= *this);

    return ret;
}




// ------------------
std::string BigInt::Rossi::toStrHex (const std::string& i_prefix) const
{
    const std::size_t HEX_SETW = sizeof(BigInt::Ulong) * 2;
    std::ostringstream oss;

    if (m_units.empty()) 
    {
        oss << i_prefix
            << std::hex
            << static_cast<BigInt::Ulong>(0)
            << std::dec;

        return oss.str();
    }

    ASSERTION (!m_units.empty());

    // --------------

    oss << i_prefix
        << std::hex
        << m_units.back();
    for (std::size_t i1 = (m_units.size() - 1); i1; i1--)
    {
        const std::size_t i = i1 - 1;
        oss << std::setw (HEX_SETW) 
            << std::setfill ('0') 
            << m_units [i];
    }
    oss << std::dec;
    return oss.str();
}


// ------------------
std::string BigInt::Rossi::toStr0xHex () const
{
    return toStrHex("0x");
}


// ------------------
std::string BigInt::Rossi::toStrDec () const
{
    std::ostringstream oss;

    BigInt::Vin vin (toStrHex(), BigInt::HEX_DIGIT);

    oss << vin;

    return oss.str();
}


// ==========================================
void BigInt::Rossi::showUnits(std::ostream& o_stream) const
{
    std::ostringstream oss;

    oss << std::endl;
    oss << ""
        << "BigInt::Rossi value"
        << ": "
        << this->toStr0xHex()      
        << " = "
        << this->toStrDec()
        << std::endl;


    BigInt::BaseBigInt::showUnits(oss);

    oss << std::endl;

    o_stream << std::flush << oss.str() << std::flush;
}



// ------------------
std::ostream& operator<< (std::ostream& o_os, const BigInt::Rossi& i_ins)
{
    // return o_os << i_ins.toStrDec();
    return o_os << i_ins.toStr0xHex();
}


// ------------------
void BigInt::TestVin::testMaxUnits(std::ostream& o_stream)
{
    std::ostringstream oss;

    SET_START_TEST_NAME(oss);

    BigInt::Vin bigInt = VinZero;
    bigInt.maximize();

    oss << "[maximize] Max BigInt::Vin has "
        << bigInt.getUnitsSize()
        << " units"
        << std::endl;

	SET_FINISH_TEST_NAME(oss);

    // ------------------
    o_stream<< std::flush << oss.str() << std::flush;

}


// ------------------
void BigInt::TestRossi::testMaxUnits(std::ostream& o_stream)
{
    std::ostringstream oss;

    SET_START_TEST_NAME(oss);

    BigInt::Rossi bigInt = RossiZero;
    bigInt.maximize();

    oss << "[maximize] Max BigInt::Rossi has "
        << bigInt.getUnitsSize()
        << " units"
        << std::endl;

	SET_FINISH_TEST_NAME(oss);

    // ------------------
    o_stream << std::flush << oss.str() << std::flush;

}

// ------------------
void BigInt::TestRossi::testMaxMultiplication(std::ostream& o_stream)
{
    std::ostringstream oss;

    SET_START_TEST_NAME(oss);

    BigInt::Rossi RossiTen (BigInt::toString(std::numeric_limits<BigInt::Ulong>::max()), BigInt::DEC_DIGIT);
    RossiTen = RossiTen + RossiOne;

    // -----------------------------
    BigInt::Rossi bigInt = RossiOne;
    while (true)
    {
        try
        {
            bigInt = bigInt * RossiTen;
        }
        catch(...)
        {
            // Do nothing
			SET_FINISH_TEST_NAME(oss);
            break;
        }

    }

    oss << "[multiplication] Max BigInt::Rossi has "
        << bigInt.getUnitsSize()
        << " units"
        << std::endl;


	SET_FINISH_TEST_NAME(oss);

    // ------------------
    o_stream << std::flush << oss.str() << std::flush;

}


// ------------------
void BigInt::TestRossi::testTryCatch(std::ostream& o_stream)
{
    std::ostringstream oss;

    oss << ""
        << std::endl
        << std::endl;

    SET_START_TEST_NAME(oss);

    o_stream << std::flush << oss.str() << std::flush;
    oss.str("");

    // -------------------------------
    const BigInt::Rossi rossi(std::numeric_limits<std::size_t>::max(), 0, 1, " - It is OK, it is a stress test");
    // -------------------------------

	SET_FINISH_TEST_NAME(oss);

    // ------------------
    o_stream << std::flush << oss.str() << std::flush;

}



// ------------------
void BigInt::TestRossi::testSqrt(std::ostream& o_stream)
{
    SET_START_TEST_NAME(o_stream);

	std::ostringstream oss;

    BigInt::Rossi result;

    // ------------------------

	for (BigInt::Ulong i = 0; i < 1025; i++)
	{
       BigInt::Rossi arg1(i);
	
	   result = arg1.sqrt();
	   oss << "[Hex] sqrt(" << arg1 << ") " << " = " << result << std::endl;
	   oss << "[Dec] sqrt(" << arg1.toStrDec() << ") " << " = " << result.toStrDec() << std::endl;
	   oss << std::endl;
    }


    const BigInt::Ulong delta = 10;
	for (BigInt::Ulong i = std::numeric_limits<BigInt::Ulong>::max() - delta; i < std::numeric_limits<BigInt::Ulong>::max(); i++)
	{
       BigInt::Rossi arg1(i);
	
	   result = arg1.sqrt();
	   oss << "[Hex] sqrt(" << arg1 << ") " << " = " << result << std::endl;
	   oss << "[Dec] sqrt(" << arg1.toStrDec() << ") " << " = " << result.toStrDec() << std::endl;
	   oss << std::endl;
    }


	BigInt::Rossi arg2 ("99999999999999999999999999999999", BigInt::DEC_DIGIT);
	for (std::size_t i = 0; i < 5; i++, arg2++)
	{
	   result = arg2.sqrt();

	   oss << "[Hex] sqrt(" << arg2 << ") " << " = " << result << std::endl;
	   oss << "[Dec] sqrt(" << arg2.toStrDec() << ") " << " = " << result.toStrDec() << std::endl;
	   oss << std::endl;

    }

	o_stream << oss.str();
	SET_FINISH_TEST_NAME(o_stream);

}



// ------------------
void BigInt::TestRossi::testOperatorAdd(std::ostream& o_stream)
{
    SET_START_TEST_NAME(o_stream);

    std::vector<std::pair<BigInt::Rossi, BigInt::Rossi> > vtest = BigInt::TestRossi::fillTestInputPairsRossiRossi();

    // --- test ---
    for (std::size_t i = 0; i < vtest.size(); i++)
    {
        ROSSI_TEST_COMPUTE_BINARY_OP(o_stream, vtest[i], +);
    }

	SET_FINISH_TEST_NAME(o_stream);

}


// ------------------
void BigInt::TestRossi::testOperatorAddAssign(std::ostream& o_stream)
{
    SET_START_TEST_NAME(o_stream);

    std::vector<std::pair<BigInt::Rossi, BigInt::Rossi> > vtest = BigInt::TestRossi::fillTestInputPairsRossiRossi();

    // --- test ---
    for (std::size_t i = 0; i < vtest.size(); i++)
    {
        BIGINT_TEST_COMPUTE_UNARY_OP(o_stream, vtest[i], +=);
    }

    SET_FINISH_TEST_NAME(o_stream);

}



// ------------------
void BigInt::TestRossi::testOperatorSubtraction(std::ostream& o_stream)
{
    SET_START_TEST_NAME(o_stream);

    std::vector<std::pair<BigInt::Rossi, BigInt::Rossi> > vtest = BigInt::TestRossi::fillTestInputPairsRossiRossi();
 
    // --- test ---
    for (std::size_t i = 0; i < vtest.size(); i++)
    {
        if (vtest[i].first < vtest[i].second)
        {
            continue;
        }
        ROSSI_TEST_COMPUTE_BINARY_OP(o_stream, vtest[i], -);
    }

	SET_FINISH_TEST_NAME(o_stream);

}


// ------------------
void BigInt::TestRossi::testOperatorSubtractionAssign(std::ostream& o_stream)
{
    SET_START_TEST_NAME(o_stream);

    std::vector<std::pair<BigInt::Rossi, BigInt::Rossi> > vtest = BigInt::TestRossi::fillTestInputPairsRossiRossi();

 
    // --- test ---
    for (std::size_t i = 0; i < vtest.size(); i++)
    {
        if (vtest[i].first < vtest[i].second)
        {
            continue;
        }
        BIGINT_TEST_COMPUTE_UNARY_OP(o_stream, vtest[i], -=);
    }

	SET_FINISH_TEST_NAME(o_stream);

}


// ------------------
void BigInt::TestRossi::testOperatorMultiplication1(std::ostream& o_stream)
{
    SET_START_TEST_NAME(o_stream);

    std::vector<std::pair<BigInt::Rossi, BigInt::Rossi> > vtest = BigInt::TestRossi::fillTestInputPairsRossiRossi();

 
    // --- test ---
    for (std::size_t i = 0; i < vtest.size(); i++)
    {
        ROSSI_TEST_COMPUTE_BINARY_OP(o_stream, vtest[i], *);
    }

	SET_FINISH_TEST_NAME(o_stream);

}


// ------------------
void BigInt::TestRossi::testOperatorMultiplication2(std::ostream& o_stream)
{
    SET_START_TEST_NAME(o_stream);

    std::vector<std::pair<BigInt::Rossi, BigInt::Ulong> > vtest = BigInt::TestRossi::fillTestInputPairsRossiUlong();

 
    // --- test ---
    for (std::size_t i = 0; i < vtest.size(); i++)
    {
        ROSSI_TEST_COMPUTE_BINARY_ULONG_OP(o_stream, vtest[i], *);
    }

	SET_FINISH_TEST_NAME(o_stream);

}


// ------------------
void BigInt::TestRossi::testOperatorDivision(std::ostream& o_stream)
{
    SET_START_TEST_NAME(o_stream);

    std::vector<std::pair<BigInt::Rossi, BigInt::Rossi> > vtest = BigInt::TestRossi::fillTestInputPairsRossiRossi();

 
    // --- test ---
    for (std::size_t i = 0; i < vtest.size(); i++)
    {
        if (vtest[i].second == RossiZero)
        {
            continue;
        }
        ROSSI_TEST_COMPUTE_BINARY_OP(o_stream, vtest[i], /);
    }

	SET_FINISH_TEST_NAME(o_stream);

}


// ------------------
void BigInt::TestRossi::testOperatorReminder(std::ostream& o_stream)
{
    SET_START_TEST_NAME(o_stream);

    std::vector<std::pair<BigInt::Rossi, BigInt::Rossi> > vtest = BigInt::TestRossi::fillTestInputPairsRossiRossi();

 
    // --- test ---
    for (std::size_t i = 0; i < vtest.size(); i++)
    {
        if (vtest[i].second == RossiZero)
        {
            continue;
        }
        ROSSI_TEST_COMPUTE_BINARY_OP(o_stream, vtest[i], %);
    }

	SET_FINISH_TEST_NAME(o_stream);

}


// ------------------
void BigInt::TestRossi::testOperatorLess(std::ostream& o_stream)
{
    SET_START_TEST_NAME(o_stream);

    std::vector<std::pair<BigInt::Rossi, BigInt::Rossi> > vtest = BigInt::TestRossi::fillTestInputPairsRossiRossi();

 
    // --- test ---
    for (std::size_t i = 0; i < vtest.size(); i++)
    {
        BIGINT_TEST_COMPARE_BINARY_OP(o_stream, vtest[i], <);
    }

	SET_FINISH_TEST_NAME(o_stream);

}



    
// ------------------
void BigInt::TestRossi::testAll(std::ostream& o_stream)
{
    testOperatorAdd(o_stream);
    testOperatorAddAssign(o_stream);
    testOperatorSubtraction(o_stream);
    testOperatorSubtractionAssign(o_stream);
    testOperatorMultiplication1(o_stream);
    testOperatorMultiplication2(o_stream);
    testOperatorDivision(o_stream);
    testOperatorReminder(o_stream);
	testSqrt(o_stream);
    testOperatorLess(o_stream);
}



// ------------------
void BigInt::TestVin::testOperatorAdd(std::ostream& o_stream)
{
    SET_START_TEST_NAME(o_stream);

    std::vector<std::pair<BigInt::Vin, BigInt::Vin> > vtest = BigInt::TestVin::fillTestInputPairsVinVin();

 
    // --- test ---
    for (std::size_t i = 0; i < vtest.size(); i++)
    {
        VIN_TEST_COMPUTE_BINARY_OP(o_stream, vtest[i], +);
    }

	SET_FINISH_TEST_NAME(o_stream);

}



// ------------------
void BigInt::TestVin::testOperatorMultiplication(std::ostream& o_stream)
{
    SET_START_TEST_NAME(o_stream);

    std::vector<std::pair<BigInt::Vin, BigInt::Ulong> > vtest = BigInt::TestVin::fillTestInputPairsVinUlong();

 
    // --- test ---
    for (std::size_t i = 0; i < vtest.size(); i++)
    {
        VIN_TEST_COMPUTE_BINARY_ULONG_OP(o_stream, vtest[i], *);
    }

	SET_FINISH_TEST_NAME(o_stream);

}


// ------------------
void BigInt::TestVin::testOperatorLess(std::ostream& o_stream)
{
    SET_START_TEST_NAME(o_stream);

    std::vector<std::pair<BigInt::Vin, BigInt::Vin> > vtest = BigInt::TestVin::fillTestInputPairsVinVin();

 
    // --- test ---
    for (std::size_t i = 0; i < vtest.size(); i++)
    {
        BIGINT_TEST_COMPARE_BINARY_OP(o_stream, vtest[i], <);
    }

	SET_FINISH_TEST_NAME(o_stream);

}


    
// ------------------
void BigInt::TestVin::testAll(std::ostream& o_stream)
{
    testOperatorAdd(o_stream);
    // testOperatorMultiplication();
    testOperatorLess(o_stream);
}



// --------------------------
void BigInt::assertCheck()
{
    // ------------------------------
    ASSERTION (BigInt::SUB_BASE2 == 999999999);
    ASSERTION (!(BigInt::BASE2 >= BigInt::MAX_UNIT_VALUE));
    ASSERTION (BigInt::BASE1 * (BigInt::BASE2/BASE1 + 1) < BigInt::MAX_UNIT_VALUE);
    ASSERTION (!(BigInt::BASE2 != (BigInt::SUB_BASE2 + 1)));
    ASSERTION (BigInt::OVER_BASE2 > BigInt::SUB_BASE2);

    ASSERTION(
            (sizeof(BigInt::Ulong) == 4) && (((BigInt::ULONG_MSB == static_cast<BigInt::Ulong>(0x80000000))) || (sizeof(BigInt::Ulong) == 8)) && ((BigInt::ULONG_MSB == ((static_cast<BigInt::Ulong>(0x80000000) << 31) << 1)))
          );
}  
	

// ------------------
void BigInt::Test::setTestName(std::ostream& o_stream, const std::string& i_txt, const std::string& i_funcName, const std::size_t i_lineNo)
{
    std::ostringstream oss;
	std::ostringstream ossNameLineNo;

	ossNameLineNo << i_txt
		          << ": "
		          << i_funcName
		          << "; "
				  << "lineNo = "
		          << i_lineNo;

	const std::size_t nameLineNoSize = ossNameLineNo.str().size();
   
    oss << std::endl
        << std::endl
        << std::endl

        << std::string (nameLineNoSize, '-') 
        << std::endl

        << ossNameLineNo.str()
        << std::endl

        << std::string (nameLineNoSize, '-')
        << std::endl

        << std::endl;

    o_stream << std::flush << oss.str() << std::flush;
}



// ------------------
void BigInt::Test::testDisplayBigInts(std::ostream& o_stream)
{
    std::ostringstream oss;

    SET_START_TEST_NAME(oss);

    const std::string arrayHexStrNumbers[] = 
    {
        BigInt::toString(0),
        BigInt::toString(1),
        BigInt::toString(2),
        BigInt::toString(9),
        BigInt::toString(10),
        BigInt::toString(11),
        BigInt::toString(15),
        BigInt::toString(16),
        BigInt::toString(17),
        BigInt::toString(std::numeric_limits<BigInt::Ulong>::max() - 1),
        BigInt::toString(std::numeric_limits<BigInt::Ulong>::max())
       
    };

    const std::vector<std::string> vectorHexStrNumbers (array2vector (arrayHexStrNumbers));

    // ----------------------------------
    std::set<BigInt::Vin> vinBigInts;

    for (std::size_t i = 0; i < vectorHexStrNumbers.size(); i++)
    {
        for (std::size_t k = 0; k < vectorHexStrNumbers.size(); k++)
        {
            BigInt::Vin value1 (vectorHexStrNumbers[i], DEC_DIGIT);
            BigInt::Vin value2 (vectorHexStrNumbers[k], DEC_DIGIT);
            BigInt::Vin sum = value1 + value2; 
           
            vinBigInts.insert(sum);
        }
    }


    // -----------------------------------------------
    std::set<BigInt::Vin>::const_iterator iter;
    for (iter = vinBigInts.begin();
         iter != vinBigInts.end();
         iter++
         )
    {
        const BigInt::Vin&  curVin = *iter;
        const BigInt::Rossi curRossi (curVin);

        ASSERTION (curVin.toStrDec() == curRossi.toStrDec());

        oss << ""
            << curRossi.toStrDec()
            << " = "
            << curRossi.toStr0xHex()
            << std::endl;
    }

	SET_FINISH_TEST_NAME(oss);
    // ------------------------------
    o_stream << std::flush << oss.str() << std::flush;
}


// ------------------
void BigInt::Test::testMain(std::ostream& o_stream, const std::vector<std::string>& i_args)
{
    std::ostringstream oss;

    SET_START_TEST_NAME(oss);

    BigInt::Run::mainBigInt(oss, i_args);

	SET_FINISH_TEST_NAME(oss);

    // ------------------------------
    o_stream << std::flush << oss.str() << std::flush;

}




// ------------------
std::vector<std::string> BigInt::Test::fillTestInputHexStr()
{
    // --- hexStrNumbers ---

    const std::string arrayHexStrNumbers[] = 
    {
        "0",
        "0",
        "1",
        "1",
        "2",
        "3",
        "100",
        "ABC",
        "100000000"
        "123456789ABCDEF",
        "F000000000000000",
        "FEDCBA9876543210",
        "10000000000000000",
        "1000000000000000000000000",
        "111222333444555666777888999AAABBBCC"
    };

    const std::vector<std::string> retVector (array2vector (arrayHexStrNumbers));

    return retVector;

}


// ------------------
std::vector<BigInt::Ulong> BigInt::TestRossi::fillTestInputUlong()
{
    // --- ulongNumbers ---
    const BigInt::Ulong arrayUlongNumbers[] = 
    {
        0, 
        0, 
        1, 
        1, 
        2, 
        3, 
        9, 
        10, 
        11, 
        15, 
        16, 
        17, 
    };
    const std::vector<BigInt::Ulong> retVector (array2vector (arrayUlongNumbers));
   
    return retVector;

}


// ------------------
std::vector<BigInt::Ulong> BigInt::TestVin::fillTestInputUlong()
{
    // --- ulongNumbers ---
    const BigInt::Ulong arrayUlongNumbers[] = 
    {
        0, 
        0, 
        1, 
        1, 
        2, 
        3, 
        9, 
        10, 
        11, 
        15, 
        16, 
        17, 
        999999998, 
        999999999
    };
    const std::vector<BigInt::Ulong> retVector (array2vector (arrayUlongNumbers));
   
    return retVector;

}





// ------------------
std::vector<std::pair<BigInt::Rossi, BigInt::Rossi> > BigInt::TestRossi::fillTestInputPairsRossiRossi()
{
    std::vector<std::pair<BigInt::Rossi, BigInt::Rossi> > retVector;

    
    std::vector<BigInt::Ulong> ulongNumbers(BigInt::TestRossi::fillTestInputUlong());
    std::vector<std::string>   hexStrNumbers(BigInt::Test::fillTestInputHexStr());
    // ------------------------------------

    for (std::size_t i = 0; i < ulongNumbers.size(); i++)
    {
        for (std::size_t k = 0; k < ulongNumbers.size(); k++)
        {
            retVector.push_back(
                std::make_pair(
                        BigInt::Rossi(ulongNumbers[i]), 
                        BigInt::Rossi(ulongNumbers[k])
                        ));
        }
    }

    for (std::size_t i = 0; i < hexStrNumbers.size(); i++)
    {
        for (std::size_t k = 0; k < hexStrNumbers.size(); k++)
        {
            retVector.push_back(
                std::make_pair(
                        BigInt::Rossi(hexStrNumbers[i], BigInt::HEX_DIGIT), 
                        BigInt::Rossi(hexStrNumbers[k], BigInt::HEX_DIGIT)
                        ));
        }
    }


    return retVector;


}

// ------------------
std::vector<std::pair<BigInt::Rossi, BigInt::Ulong> > BigInt::TestRossi::fillTestInputPairsRossiUlong()
{
    std::vector<std::pair<BigInt::Rossi, BigInt::Ulong> > retVector;
    
    std::vector<BigInt::Ulong> ulongNumbers(BigInt::TestRossi::fillTestInputUlong());
    // ------------------------------------

    for (std::size_t i = 0; i < ulongNumbers.size(); i++)
    {
        for (std::size_t k = 0; k < ulongNumbers.size(); k++)
        {
            retVector.push_back(
                std::make_pair(
                        BigInt::Rossi(ulongNumbers[i]), 
                        ulongNumbers[k]
                        ));
        }
    }

    return retVector;

}


// ------------------
std::vector<std::pair<BigInt::Vin, BigInt::Vin> > BigInt::TestVin::fillTestInputPairsVinVin()
{
    std::vector<std::pair<BigInt::Vin, BigInt::Vin> > retVector;

    
    std::vector<BigInt::Ulong> ulongNumbers (BigInt::TestVin::fillTestInputUlong());
    std::vector<std::string>   hexStrNumbers (BigInt::Test::fillTestInputHexStr());
    // ------------------------------------

    for (std::size_t i = 0; i < ulongNumbers.size(); i++)
    {
        for (std::size_t k = 0; k < ulongNumbers.size(); k++)
        {
            retVector.push_back(
                std::make_pair(
                        BigInt::Vin(ulongNumbers[i]), 
                        BigInt::Vin(ulongNumbers[k])
                        ));
        }
    }

    for (std::size_t i = 0; i < hexStrNumbers.size(); i++)
    {
        for (std::size_t k = 0; k < hexStrNumbers.size(); k++)
        {
            retVector.push_back(
                std::make_pair(
                        BigInt::Vin(hexStrNumbers[i], BigInt::HEX_DIGIT), 
                        BigInt::Vin(hexStrNumbers[k], BigInt::HEX_DIGIT)
                        ));
        }
    }


    return retVector;


}

// ------------------
std::vector<std::pair<BigInt::Vin, BigInt::Ulong> > BigInt::TestVin::fillTestInputPairsVinUlong()
{
    std::vector<std::pair<BigInt::Vin, BigInt::Ulong> > retVector;

    
    std::vector<BigInt::Ulong> ulongNumbers (BigInt::TestVin::fillTestInputUlong());
    // ------------------------------------

    for (std::size_t i = 0; i < ulongNumbers.size(); i++)
    {
        for (std::size_t k = 0; k < ulongNumbers.size(); k++)
        {
            retVector.push_back(
                std::make_pair(
                        BigInt::Vin(ulongNumbers[i]), 
                        ulongNumbers[k]
                        ));
        }
    }

    return retVector;

}

