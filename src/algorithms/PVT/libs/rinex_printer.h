/*!
 * \file rinex_printer.h
 * \brief Interface of a RINEX 2.11 / 3.01 printer
 * See http://igscb.jpl.nasa.gov/igscb/data/format/rinex301.pdf
 *
 * Receiver Independent EXchange Format (RINEX):
 * The first proposal for the Receiver Independent Exchange Format RINEX
 * was developed by the Astronomical Institute of the University of Berne
 * for the easy exchange of the GPS data to be collected during the large
 * European GPS campaign EUREF 89, which involved more than 60 GPS receivers
 * of 4 different manufacturers.
 * The governing aspect during the development was the fact that most geodetic
 * processing software for GPS data use a well-defined set of observables:
 * 1) The carrier-phase measurement at one or both carriers (actually being a
 * measurement on the beat frequency between the received carrier of the
 * satellite signal and a receiver-generated reference frequency).
 * 2) The pseudorange (code) measurement , equivalent to the difference
 * of the time of reception (expressed in the time frame of the receiver)
 * and the time of transmission (expressed in the time frame of the satellite)
 * of a distinct satellite signal.
 * 3) The observation time being the reading of the receiver clock at the
 * instant of validity of the carrier-phase and/or the code measurements.
 * Note: A collection of the formats currently used by the IGS can be found
 * here: http://igscb.jpl.nasa.gov/components/formats.html
 * \author Carles Fernandez Prades, 2011. cfernandez(at)cttc.es
 * -------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2014  (see AUTHORS file for a list of contributors)
 *
 * GNSS-SDR is a software defined Global Navigation
 *          Satellite Systems receiver
 *
 * This file is part of GNSS-SDR.
 *
 * GNSS-SDR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GNSS-SDR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNSS-SDR. If not, see <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------
 */

#ifndef GNSS_SDR_RINEX_PRINTER_H_
#define	GNSS_SDR_RINEX_PRINTER_H_

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>  // for stringstream
#include <iomanip>  // for setprecision
#include <map>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "gps_navigation_message.h"
#include "galileo_navigation_message.h"
#include "sbas_telemetry_data.h"
#include "GPS_L1_CA.h"
#include "Galileo_E1.h"
#include "gnss_synchro.h"

class Sbas_Raw_Msg;

/*!
 * \brief Class that handles the generation of Receiver
 * INdependent EXchange format (RINEX) files
 */
class Rinex_Printer
{
public:
    /*!
     * \brief Default constructor. Creates GPS Navigation and Observables RINEX files and their headers
     */
    Rinex_Printer();

    /*!
     * \brief Default destructor. Closes GPS Navigation and Observables RINEX files
     */
    ~Rinex_Printer();

    std::ofstream obsFile ; //<! Output file stream for RINEX observation file
    std::ofstream navFile ; //<! Output file stream for RINEX navigation data file
    std::ofstream sbsFile ; //<! Output file stream for RINEX SBAS raw data file
    std::ofstream navGalFile ; //<! Output file stream for RINEX Galileo navigation data file
    std::ofstream navMixFile ; //<! Output file stream for RINEX Mixed navigation data file

    /*!
     *  \brief Generates the GPS Navigation Data header
     */
    void rinex_nav_header(std::ofstream& out, Gps_Iono iono, Gps_Utc_Model utc_model);

    /*!
     *  \brief Generates the Galileo Navigation Data header
     */
    void rinex_nav_header(std::ofstream& out, Galileo_Iono iono, Galileo_Utc_Model utc_model, Galileo_Almanac galileo_almanac);

    /*!
     *  \brief Generates the Mixed (GPS/Galileo) Navigation Data header
     */
    void rinex_nav_header(std::ofstream& out, Gps_Iono gps_iono, Gps_Utc_Model gps_utc_model, Galileo_Iono galileo_iono, Galileo_Utc_Model galileo_utc_model, Galileo_Almanac galileo_almanac);

    /*!
     *  \brief Generates the GPS Observation data header
     */
    void rinex_obs_header(std::ofstream& out, Gps_Ephemeris eph, double d_TOW_first_observation);

    /*!
     *  \brief Generates the Galileo Observation data header
     */
    void rinex_obs_header(std::ofstream& out, Galileo_Ephemeris eph, double d_TOW_first_observation);

    /*!
     *  \brief Generates the Mixed (GPS/Galileo) Observation data header
     */
    void rinex_obs_header(std::ofstream& out, Gps_Ephemeris gps_eph, Galileo_Ephemeris galileo_eph, double d_TOW_first_observation);

    /*!
     *  \brief Generates the SBAS raw data header
     */
    void rinex_sbs_header(std::ofstream& out);

    /*!
     *  \brief Computes the UTC time and returns a boost::posix_time::ptime object
     */
    boost::posix_time::ptime compute_UTC_time(Gps_Navigation_Message nav_msg);

    /*!
     *  \brief Computes the GPS time and returns a boost::posix_time::ptime object
     */
    boost::posix_time::ptime compute_GPS_time(Gps_Ephemeris eph, double obs_time);

    /*!
     *  \brief Computes the Galileo time and returns a boost::posix_time::ptime object
     */
    boost::posix_time::ptime compute_Galileo_time(Galileo_Ephemeris eph, double obs_time);

    /*!
     *  \brief Writes data from the GPS navigation message into the RINEX file
     */
    void log_rinex_nav(std::ofstream& out, std::map<int,Gps_Ephemeris> eph_map);

    /*!
     *  \brief Writes data from the Galileo navigation message into the RINEX file
     */
    void log_rinex_nav(std::ofstream& out, std::map<int, Galileo_Ephemeris> eph_map);

    /*!
     *  \brief Writes data from the Mixed (GPS/Galileo) navigation message into the RINEX file
     */
    void log_rinex_nav(std::ofstream& out, std::map<int, Gps_Ephemeris> gps_eph_map, std::map<int, Galileo_Ephemeris> galileo_eph_map);

    /*!
     *  \brief Writes GPS observables into the RINEX file
     */
    void log_rinex_obs(std::ofstream& out, Gps_Ephemeris eph, double obs_time, std::map<int,Gnss_Synchro> pseudoranges);

    /*!
     *  \brief Writes Galileo observables into the RINEX file
     */
    void log_rinex_obs(std::ofstream& out, Galileo_Ephemeris eph, double obs_time, std::map<int,Gnss_Synchro> pseudoranges);

    /*!
     *  \brief Writes Galileo observables into the RINEX file
     */
    void log_rinex_obs(std::ofstream& out, Gps_Ephemeris gps_eph, Galileo_Ephemeris galileo_eph, double gps_obs_time, std::map<int,Gnss_Synchro> pseudoranges);

    /*!
     * \brief Represents GPS time in the date time format. Leap years are considered, but leap seconds are not.
     */
    void to_date_time(int gps_week, int gps_tow, int &year, int &month, int &day, int &hour, int &minute, int &second);

    /*!
     *  \brief Writes raw SBAS messages into the RINEX file
     */
    void log_rinex_sbs(std::ofstream& out, Sbas_Raw_Msg sbs_message);

    std::map<std::string,std::string> satelliteSystem; //<! GPS, GLONASS, SBAS payload, Galileo or Compass
    std::map<std::string,std::string> observationType; //<! PSEUDORANGE, CARRIER_PHASE, DOPPLER, SIGNAL_STRENGTH
    std::map<std::string,std::string> observationCode; //<! GNSS observation descriptors
    std::string stringVersion; //<! RINEX version (2.10/2.11 or 3.01)

private:
    int version ;  // RINEX version (2 for 2.10/2.11 and 3 for 3.01)
    int numberTypesObservations; // Number of available types of observable in the system. Should be public?
    /*
     * Generation of RINEX signal strength indicators
     */
    int signalStrength(double snr);

    /* Creates RINEX file names according to the naming convention
     *
     * See http://igscb.jpl.nasa.gov/igscb/data/format/rinex301.pdf
     * Section 4, page 6
     *
     * \param[in] type of RINEX file. Can be:
     * "RINEX_FILE_TYPE_OBS" - Observation file.
     * "RINEX_FILE_TYPE_GPS_NAV" - GPS navigation message file.
     * "RINEX_FILE_TYPE_MET" - Meteorological data file.
     * "RINEX_FILE_TYPE_GLO_NAV" - GLONASS navigation file.
     * "RINEX_FILE_TYPE_GAL_NAV"  - Galileo navigation message file.
     * "RINEX_FILE_TYPE_MIXED_NAV" - Mixed GNSS navigation message file.
     * "RINEX_FILE_TYPE_GEO_NAV" - SBAS Payload navigation message file.
     * "RINEX_FILE_TYPE_SBAS" - SBAS broadcast data file.
     * "RINEX_FILE_TYPE_CLK" - Clock file.
     */
    std::string createFilename(std::string type);

    std::string navfilename;
    std::string obsfilename;
    std::string sbsfilename;
    std::string navGalfilename;
    std::string navMixfilename;

    /*
     * Generates the data for the PGM / RUN BY / DATE line
     */
    std::string getLocalTime();

    /*
     *  Checks that the line is 80 characters length
     */
    void lengthCheck(std::string line);

    /*
     * If the string is bigger than length, truncate it from the right.
     * otherwise, add pad characters to its right.
     *
     * Left-justifies the input in a string of the specified
     * length. If the new length (\a length) is larger than the
     * current length, the string is extended by the pad
     * character (\a pad). The default pad character is a
     * blank.
     * \param[in] s string to be modified.
     * \param[in] length new desired length of string.
     * \param[in] pad character to pad string with (blank by default).
     * \return a reference to \a s.  */
    inline std::string& leftJustify(std::string& s,
            const std::string::size_type length,
            const char pad = ' ');

    /*
     * If the string is bigger than length, truncate it from the right.
     * otherwise, add pad characters to its right.
     *
     * Left-justifies the receiver in a string of the specified
     * length (const version). If the new length (\a length) is larger
     * than the current length, the string is extended by the pad
     * character (\a pad). The default pad character is a
     * blank.
     * \param[in] s string to be modified.
     * \param[in] length new desired length of string.
     * \param[in] pad character to pad string with (blank by default).
     * \return a reference to \a s.  */
    inline std::string leftJustify(const std::string& s,
            const std::string::size_type length,
            const char pad = ' ')
    { std::string t(s); return leftJustify(t, length, pad); }


    /*
     * Right-justifies the receiver in a string of the specified
     * length. If the receiver's data is shorter than the
     * requested length (\a length), it is padded on the left with
     * the pad character (\a pad). The default pad
     * character is a blank. */
    inline std::string& rightJustify(std::string& s,
            const std::string::size_type length,
            const char pad = ' ');

    /*
     * Right-justifies the receiver in a string of the specified
     * length (const version). If the receiver's data is shorter than the
     * requested length (\a length), it is padded on the left with
     * the pad character (\a pad). The default pad
     * character is a blank.*/
    inline std::string rightJustify(const std::string& s,
            const std::string::size_type length,
            const char pad = ' ')
    { std::string t(s); return rightJustify(t, length, pad); }



    /*
     * Convert a double to a scientific notation number.
     * @param d the double to convert
     * @param length length (in characters) of output, including exponent
     * @param expLen length (in characters) of the exponent, with sign
     * @param showSign if true, reserves 1 character for +/- sign
     * @param checkSwitch if true, keeps the exponential sanity check for
     * exponentials above three characters in length.  If false, it removes
     * that check.
     */
    inline std::string doub2sci(const double& d,
            const std::string::size_type length,
            const std::string::size_type expLen,
            const bool showSign = true,
            const bool checkSwitch = true);




    /*
     * Convert scientific notation to FORTRAN notation.
     * As an example, the string "1.5636E5" becomes " .15636D6".
     * Note that the first character of the string will be '-' if
     * the number is negative or ' ' if the first character is positive.
     * @param aStr string with number to convert
     * @param startPos start position of number in string
     * @param length length (in characters) of number, including exponent.
     * @param expLen length (in characters of exponent, not including sign.
     * @param checkSwitch will keep the method running as originally programmed
     * when set to true.  If false, the method will always resize exponentials,
     * produce an exponential with an E instead of a D, and always have a leading
     * zero.  For example -> 0.87654E-0004 or -0.1234E00005.
     */
    inline std::string& sci2for(std::string& aStr,
            const std::string::size_type startPos = 0,
            const std::string::size_type length = std::string::npos,
            const std::string::size_type expLen = 3,
            const bool checkSwitch = true);





    /*
     * Convert double precision floating point to a string
     * containing the number in FORTRAN notation.
     * As an example, the number 156360 becomes ".15636D6".
     * @param d number to convert.
     * @param length length (in characters) of number, including exponent.
     * @param expLen length (in characters of exponent, including sign.
     * @param checkSwitch if true, keeps the exponential sanity check for
     * exponentials above three characters in length.  If false, it removes
     * that check.
     * @return a string containing \a d in FORTRAN notation.
     */
    inline std::string doub2for(const double& d,
            const std::string::size_type length,
            const std::string::size_type expLen,
            const bool checkSwitch = true);


    /*
     * Convert a string to a double precision floating point number.
     * @param s string containing a number.
     * @return double representation of string.
     */
    inline double asDouble(const std::string& s)
    { return strtod(s.c_str(), 0); }


    inline int toInt(std::string bitString, int sLength);

    /*
     * Convert a string to an integer.
     * @param s string containing a number.
     * @return long integer representation of string.
     */
    inline long asInt(const std::string& s)
    { return strtol(s.c_str(), 0, 10); }


    /*
     * Convert a double to a string in fixed notation.
     * @param x double.
     * @param precision the number of decimal places you want displayed.
     * @return string representation of \a x.
     */
    inline std::string asString(const double x,
            const std::string::size_type precision = 17);


    /*
     * Convert a long double to a string in fixed notation.
     * @param x long double.
     * @param precision the number of decimal places you want displayed.
     * @return string representation of \a x.
     */
    inline std::string asString(const long double x,
            const std::string::size_type precision = 21);

    /*
     * Convert any old object to a string.
     * The class must have stream operators defined.
     * @param x object to turn into a string.
     * @return string representation of \a x.
     */
    template <class X>
    inline std::string asString(const X x);
    inline std::string asFixWidthString(const int x, const int width, char fill_digit);
};



// Implementation of inline functions (modified versions from GPSTk http://www.gpstk.org)

inline std::string& Rinex_Printer::leftJustify(std::string& s,
        const std::string::size_type length,
        const char pad)
{

    if(length < s.length())
        {
            s = s.substr(0, length);
        }
    else
        {
            s.append(length-s.length(), pad);
        }
    return s;
}


// if the string is bigger than length, truncate it from the left.
// otherwise, add pad characters to its left.
inline std::string& Rinex_Printer::rightJustify(std::string& s,
        const std::string::size_type length,
        const char pad)
{
    if(length < s.length())
        {
            s = s.substr(s.length()-length, std::string::npos);
        }
    else
        {
            s.insert((std::string::size_type)0, length-s.length(), pad);
        }
    return s;
}





inline std::string Rinex_Printer::doub2for(const double& d,
        const std::string::size_type length,
        const std::string::size_type expLen,
        const bool checkSwitch)
{

    short exponentLength = expLen;

    /* Validate the assumptions regarding the input arguments */
    if (exponentLength < 0) exponentLength = 1;
    if (exponentLength > 3 && checkSwitch) exponentLength = 3;

    std::string toReturn = doub2sci(d, length, exponentLength, true, checkSwitch);
    sci2for(toReturn, 0, length, exponentLength, checkSwitch);

    return toReturn;

}


inline std::string Rinex_Printer::doub2sci(const double& d,
        const std::string::size_type length,
        const std::string::size_type expLen,
        const bool showSign,
        const bool checkSwitch)
{
    std::string toReturn;
    short exponentLength = expLen;

    /* Validate the assumptions regarding the input arguments */
    if (exponentLength < 0) exponentLength = 1;
    if (exponentLength > 3 && checkSwitch) exponentLength = 3;

    std::stringstream c;
    c.setf(std::ios::scientific, std::ios::floatfield);

    // length - 3 for special characters ('.', 'e', '+' or '-')
    // - exponentlength (e04)
    // - 1 for the digit before the decimal (2.)
    // and if showSign == true,
    //    an extra -1 for '-' or ' ' if it's positive or negative
    int expSize = 0;
    if (showSign)
        expSize = 1;
    c.precision(length - 3 - exponentLength - 1 - expSize);
    c << d;
    c >> toReturn;
    return toReturn;
}

inline std::string& Rinex_Printer::sci2for(std::string& aStr,
        const std::string::size_type startPos,
        const std::string::size_type length,
        const std::string::size_type expLen,
        const bool checkSwitch)
{
    std::string::size_type idx = aStr.find('.', startPos);
    int expAdd = 0;
    std::string exp;
    long iexp;
    //If checkSwitch is false, always redo the exponential. Otherwise,
    //set it to false.
    bool redoexp=!checkSwitch;

    // Check for decimal place within specified boundaries
    if ((idx == 0) || (idx >= (startPos + length - expLen - 1)))
        {
            //StringException e("sci2for: no decimal point in string");
        }

    // Here, account for the possibility that there are
    // no numbers to the left of the decimal, but do not
    // account for the possibility of non-scientific
    // notation (more than one digit to the left of the
    // decimal)
    if (idx > startPos)
        {
            redoexp = true;
            // Swap digit and decimal.
            aStr[idx] = aStr[idx-1];
            aStr[idx-1] = '.';
            // Only add one to the exponent if the number is non-zero
            if (asDouble(aStr.substr(startPos, length)) != 0.0)
                expAdd = 1;
        }

    idx = aStr.find('e', startPos);
    if (idx == std::string::npos)
        {
            idx = aStr.find('E', startPos);
            if (idx == std::string::npos)
                {
                    //StringException e("sci2for:no 'e' or 'E' in string");
                    //GPSTK_THROW(e);
                }
        }

    // Change the exponent character to D normally, or E of checkSwitch is false.
    if (checkSwitch)
        aStr[idx] = 'D';
    else
        aStr[idx] = 'E';

    // Change the exponent itself
    if (redoexp)
        {
            exp = aStr.substr(idx + 1, std::string::npos);
            iexp = asInt(exp);
            iexp += expAdd;

            aStr.erase(idx + 1);
            if (iexp < 0)
                {
                    aStr += "-";
                    iexp -= iexp*2;
                }
            else
                aStr += "+";
            aStr += Rinex_Printer::rightJustify(asString(iexp),expLen,'0');
        }

    // if the number is positive, append a space
    // (if it's negative, there's a leading '-'
    if (aStr[0] == '.')
        {
            aStr.insert((std::string::size_type)0, 1, ' ');
        }

    //If checkSwitch is false, add on one leading zero to the string
    if (!checkSwitch)
        {
            aStr.insert((std::string::size_type)1, 1, '0');
        }

    return aStr;
}  // end sci2for



inline std::string asString(const long double x, const std::string::size_type precision)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << x ;
    return ss.str();
}




inline std::string Rinex_Printer::asString(const double x, const std::string::size_type precision)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << x;
    return ss.str();
}


inline std::string Rinex_Printer::asFixWidthString(const int x, const int width, char fill_digit)
{
    std::ostringstream ss;
    ss << std::setfill(fill_digit) << std::setw(width) << x;
    //std::cout << "asFixWidthString(): x=" << x << " width=" << width << " fill_digit=" << fill_digit << " ss=" << ss.str() << std::endl;
    return ss.str().substr(ss.str().size() - width);
}

inline long asInt(const std::string& s)
    { return strtol(s.c_str(), 0, 10); }


inline int Rinex_Printer::toInt(std::string bitString, int sLength)
{
    int tempInt;
    int num = 0;
    for(int i=0; i < sLength; i++)
    {
        tempInt = bitString[i]-'0';
        num |= (1 << (sLength-1-i)) * tempInt;
    }
    return num;
}


template<class X>
inline std::string Rinex_Printer::asString(const X x)
{
    std::ostringstream ss;
    ss << x;
    return ss.str();
}


#endif
