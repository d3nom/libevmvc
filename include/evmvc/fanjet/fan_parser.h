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

#ifndef _libevmvc_fanjet_parser_h
#define _libevmvc_fanjet_parser_h

#include "../stable_headers.h"
#include "fan_common.h"
#include "fan_tokenizer.h"
#include "fan_ast.h"

namespace evmvc { namespace fanjet {

class parser
{
    parser() = delete;
public:
    
    static ast::root_node parse(const std::string& text)
    {
        ast::token root_token = ast::tokenizer::tokenize(text);
        ast::root_node r = ast::parse(root_token);
        
        return r;
    }
    
    
    
private:

};

}}//::evmvc::fanjet
#endif //_libevmvc_fanjet_parser_h
