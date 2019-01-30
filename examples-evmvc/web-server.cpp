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

#include "evmvc/evmvc.h"




int main(int argc, char** argv)
{
    struct event_base* _ev_base = event_base_new();
    
    evmvc::sp_app srv = std::make_shared<evmvc::app>("./");
    
    srv->get("/test",
    [](const evmvc::request& req, evmvc::response& res, auto nxt){
        res.status(evmvc::status::ok).send(
            req.query_param_as<std::string>("val", "testing 1, 2...")
        );
        nxt(nullptr);
    });
    
    srv->get("/download-file/:[filename]",
    [](const evmvc::request& req, evmvc::response& res, auto nxt){
        res.status(evmvc::status::ok)
            .download("the file path",
            req.route_param_as<std::string>("filename", "test.txt"));
        nxt(nullptr);
    });
    
    srv->get("/echo/:val",
    [](const evmvc::request& req, evmvc::response& res, auto nxt){
        res.status(evmvc::status::ok).send(
            req.route_param_as<std::string>("val")
        );
        nxt(nullptr);
    });
    
    srv->listen(_ev_base);
    
    event_base_loop(_ev_base, 0);
    return 0;
}