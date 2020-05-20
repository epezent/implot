#ifndef _INC_TIMEFORMATTER_HPP_
#define _INC_TIMEFORMATTER_HPP_
#include <iostream>
#include <ctime>
#include <sstream>
#include <iomanip>

class TimeFormatter {
    public:
    TimeFormatter(double microSecondTimeStamp) {
        // Don't keep decimals
        uint64_t USTimeStamp = microSecondTimeStamp;
        s = USTimeStamp / US_IN_SEC;
        _us = USTimeStamp - s * US_IN_SEC ;
    }

    std::string getRangeFormatterPrefixString(double range, int divisions , int subdivisions) {
        double us = range;
        double ms = us/1000;
        double s = ms/1000;
        double min=s/60;
        double hr=min/60;
        double day= hr/24;
        double month = day/30;
        double div = 5;
        std::string p = "" ;//std::to_string(getIntergral());

         if (day>div) {
            p= toString("%Y");
        } else if (hr>div) {
            p=  toString("%Y:%m");
        } else if (min>div) {
            p= toString("%m:%e");
        } else if (s>div) {
            p = toString("%e");
        } else if (ms>div) {
            p =  toString("%e:%I");
        } { // Last Prefix Granularity
            p =  toString("%I:%M:%S");
        }
        return  p ;

    }

    std::string toString(std::string &&format) {
        char buf[80];
        strftime(buf, 80, format.c_str(),std::localtime(&s));
        return std::string(buf);
    }
    
    std::string getPaddedMilliSeconds() {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(3) << getMilliSeconds();
        return oss.str();
    }
    
    std::string getPaddedMicroSeconds() {
        
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(6) << getMicroSeconds();
        return oss.str();
    }
    std::string getFullFormattedString() {
        std::string p =  toString("%Y/%m/%e %I:%M:%S.") + getPaddedMicroSeconds();
        return p;
    }

    std::string getRangeFormattedString(double range, uint64_t divisions, uint64_t subdivisions) {
        double us = range/1000;
        double ms = us/1000;
        double s = ms/1000;
        double min=s/60;
        double hr=min/60;
        double day= hr/24;
        double month = day/30;
        double div = 5;
        std::string p = std::to_string(getIntergral());

        if (month>div) {
            p= toString("%Y/%m/%e");
        } else if (day>div) {
            p= toString("%m/%e");
        } else if (hr>div) {
            p= toString("%e:%I");
        } else if (min>div) {
            p= toString("%I:%M");
        } else if (s>div) {
            p = toString("%I:%M:%S");
        } else if (ms>div) {
            p = toString("%M:%S.")+getPaddedMilliSeconds();
        } else {    // Last Granularity
            p = toString("%S.")+getPaddedMicroSeconds();
        }
        return  p ;
    }
    uint64_t getIntergral() {
        return s * US_IN_SEC + _us;
    }
    uint64_t getSeconds(){
        return s;
    }
    /*uint64_t getNanoSeconds() {
        return ns;
    }*/
    uint64_t getMicroSeconds() {
        return _us ;
    }
    uint64_t getMilliSeconds() {
        return getMicroSeconds() / 1000;
    }

    std::time_t s;
    uint64_t _us;
    static const uint64_t US_IN_SEC = 1000000;

};
#endif
