//
//  converter.hpp
//  sswatcher
//
//  Created by Jason Zhong on 5/9/16.
//  Copyright © 2016 Meom.moe. All rights reserved.
//

#ifndef converter_hpp
#define converter_hpp

#include <vector>
#include <string>

using std::vector;
using std::string;

class Converter {
public:
    static uint64_t parse_size(const char * str, const bool usegig = false);
    static string convert_to_string(const uint64_t size);
    
private:
    static vector<string> units;
    static string format(double d);
    
};

#endif /* converter_hpp */
