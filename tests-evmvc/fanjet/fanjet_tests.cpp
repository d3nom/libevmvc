/*
MIT License

Copyright (c) 2019 Michel Dénommée

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <gmock/gmock.h>
#include "evmvc/evmvc.h"
#include "evmvc/fanjet/fanjet.h"

namespace evmvc { namespace tests {


class fanjet_test: public testing::Test
{
public:
};


// TEST_F(fanjet_test, ast_test)
// {
//     try{
//         std::string fan_str;
        
//         std::vector<std::string> ml;
//         fanjet::ast::root_node r = 
//             fanjet::parser::parse(ml, fan_str);
        
//         md::log::info(
//             "debug ast output:\n{}",
//             r->dump()
//         );
        
//         md::log::info(
//             "ast src output:\n{}",
//             r->token_section_text(true)
//         );
        
//         ASSERT_EQ(fan_str, r->token_section_text());
        
//     }catch(const std::exception& err){
//         md::log::error(err.what());
//         FAIL();
//     }
// }


TEST_F(fanjet_test, ast_parser)
{
    try{
        // std::string fan_str(
        //     "@if(test_fn_a() && test_fn_b()){"
        //     "    std::cout << \"a\""
        //     "        << std::endl;"
        //     "} else if(1 != false){"
        //     "    auto i = test_fn_c();"
        //     "    std::cout << \"b\""
        //     "        << i"
        //     "        << std::endl;"
        //     "}else{"
        //     "    std::cout << \"cde\""
        //     "        << std::endl;"
        //     "}"
        // );
        
        // std::string fan_str(
        //     "@if(true){\n"
        //     "    return 1;\n"
        //     "}else if(std::string(\"abc\") == \"123\"){\n"
        //     "    @( md )   { in markdown literal! }\n"
        //     "    return -1;\n"
        //     "}else{\n"
        //     "    try{\n"
        //     "        return 2;\n"
        //     "    }catch(const std::exception& err){\n"
        //     "        throw err;\n"
        //     "    }\n"
        //     "}\n"
        //     "try{ inside literal }catch( n/a ){ in literal... }\n"
        // );        
        
        
        
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}


}} //ns evevmvc::tests