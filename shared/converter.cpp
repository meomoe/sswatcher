//
//  converter.cpp
//  sswatcher
//
//  Created by Jason Zhong on 5/9/16.
//  Copyright © 2016 Meom.moe. All rights reserved.
//

#include "converter.hpp"
#include <cstdlib>
#include <sstream>
#include <iomanip>


vector<string> Converter::units = { "B", "KB", "MB", "GB", "TB" };

uint64_t Converter::parse_size(const char * size, bool usegig) {
    string sizestr(size);
    for (auto & c: sizestr)
        c = toupper(c);
    
    uint64_t mul = 1;
    if (usegig)
        mul = 1000ull * 1000 * 1000;
    
    
    if (sizestr.find("T", 0) != string::npos)
        mul = 1000ull * 1000 * 1000 * 1000;
    else if (sizestr.find("G", 0) != string::npos)
        mul = 1000ull * 1000 * 1000;
    else if (sizestr.find("M", 0) != string::npos)
        mul = 1000ull * 1000;
    else if (sizestr.find("K", 0) != string::npos)
        mul = 1000;
    else if (sizestr.find("B", 0) != string::npos)
        mul = 1;
    
    string numstr;
    for (auto & c: sizestr) {
        // isdigit returns non-zero values for digits
        if (isdigit(c) != 0)
            numstr += c;
        else if (c == ' ' || c == ',' || c == '_')
            continue;
        else
            break;
    }
    
    uint64_t cap = atoll(numstr.c_str());
    cap *= mul;
    
    return cap;
}

string Converter::format(double d) {
    if (d == 0)
        return "0";
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << d;
    
    return ss.str();
}

string Converter::convert_to_string(const uint64_t size) {
    double s = size;
    unsigned cnt = 0;
    double q = size;
    
    while (s >= 1) {
        s /= 1000;
        if (s >= 1) {
            cnt++;
            if (cnt <= 4)
                q = s;
            else {
                cnt--;
                break;
            }
        }
    }
    
    string unit = units[cnt];
    string res = format(q) + " " + unit;
    return res;
}