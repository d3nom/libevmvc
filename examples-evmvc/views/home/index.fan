@*
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
*@
@ns     
@name   
@layout _layout
@inherits{
    public @utils = helpers/utils
}

@set("title", "index - web-server example")

In index.fan page <br/>

Message: @get("msg")
<br/>

@{
    @utils::util_func01();
    std::string attr_tst("data-test-a=\"attribute test\"");
    std::string attr_val_tst("attribute value");
}


<a href="/test" @::attr_tst;>test</a><br/>
<a href="/html/login.html" data-value="@:attr_val_tst;">login</a><br/>
<a href="/download-file/">/download-file/:[filename]</a><br/>
<a href="/echo/blabla">/echo/:val</a><br/>
<a href="/send-json">/send-json</a><br/>
<a href="/send-file?path=xyz">/send-file?path=xyz</a><br/>
<a href="/cookies/set/">/cookies/set/:[name]/:[val]/:[path]</a><br/>
<a href="/cookies/get/">/cookies/get/:[name]</a><br/>
<a href="/cookies/clear/">/cookies/clear/:[name]/:[path]</a><br/>
<a href="/error">/error</a><br/>
<a href="/exit">/exit</a><br/>
<a href="/set_timeout/">/set_timeout/:[timeout(\\d+)]</a><br/>
<a href="/set_interval/">/set_interval/:[name]/:[interval(\\d+)]</a><br/>
<a href="/clear_interval/">/clear_interval/:[name]</a><br/>


@header{
    int j;
    
}

@add-section(section_a){
    <div>section a</div>
}

@write-section( missing_section ){
    <div>if section is missing</div>
}

@write-section(section_a);

@write-section(section_a){
    <div>section_a is missing</div>
}
