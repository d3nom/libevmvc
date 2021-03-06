[TOC]

# About libevmvc

## Routing

* simple route that will match url "/abc/123" and "/abc/123/"
<br/>
/abc/123

* simple route that will match url's "/abc/123", "/abc/123/def", "/abc/123/456/" but not "/abc/123/def/other_path"
<br/>
/abc/123/*

* simple route that will match url's "/abc/123", "/abc/123/def", "/abc/123/456/" and any sub path "/abc/123/def/sub/path/..."
<br/>
/abc/123/**

* optional parameter can be defined by enclosing the parameter in square brackets
<br/>
/abc/123/:p1/:[p2]

* PCRE regex can be use to validate parameter by enclosing the rules inside parentheses following the parameter name
<br/>
/abc/123/:p1(\\d+)/:[p2]

* regex parameter can be optional as well
<br/>
/abc/123/:[p1(\\d+)]

* all parameters following an optional parameter must be optional
<br/>
/abc/123/:p1(\\d+)/:[p2]/:[p3]


## Fanjet language

### Fanjet engine directory structure

~~~
    .../views/
        ├─ layouts/ (default layout dir)
        │   ├─ *.fan (Fanjet root layout files, default layout is named '_layout.fan')
        │   ├─ subdir_a/
        │   │   └─ *.fan (Fanjet root layouts files)
        │   │   
        │   └─ subdir_b/
        │       └─ *.fan (Fanjet root layouts files)
        │       
        ├─ partials/ (partials can be rendered with the @>view/path/to/render; directive)
        │   └─ *.fan (Fanjet root partials files)
        │
        ├─ helpers/ (optional helper root directory)
        │   └─ *.fan (Fanjet root helper files)
        │
        ├─ usr_ns_a/ (user defined namespace)
        │   ├─ layouts/ (optional layouts dir)
        │   │   └─ *.fan (Fanjet 'usr_ns_a/' layout files, default layout is named '_layout.fan')
        │   │   
        │   ├─ partials/
        │   │   └─ *.fan (Fanjet 'usr_ns_a/' partials files)
        │   │
        │   ├─ helpers/
        │   │   └─ *.fan (Fanjet 'usr_ns_a/' helpers files)
        │   │
        │   └─ *.fan (Fanjet 'usr_ns_a/*' files)
        │
        ├─ usr_ns_b/ ... (same as '/usr_ns_a')
        │
        └─ *.fan (Fanjet root files)
        
~~~

### layouts directory

Files in the layouts directory are template used to render enclosing html
to render the body use the render body directive '@body', this will asynchrounously render the view

### partials directory

partials view can be rendered at any time using the render partial directive '@>...;'
the user must replace the '...' value with the partial view name to render.
the directive must be ended with a semicolon.

### helpers directory

Files in the helpers directory are automatically inherited by all file in the same directory
or in any subdirectory of the hierarchy.

### search paths

The search for a specific view is always done from current directory to top directory
e.g.: the user whant to render the following partial view '@>user_info;'
first the 'user_info' view is search in the current view 'partials' directory './partials/user_info'
if the view is not found than the parent directory will be probe '../partials/user_info' and so on,
until the view is found or the views 'root' directory is reached.

### Fanjet directives

| Start tag     | End tag   | comment |
| ---------     | --------- | ------- |
| @@            |           | escape sequence, will be processed as the '@' char. |
| @{            | }         | cpp code block. |
| @**           | \n        | line comment |
| @*            | *@        | multiline comment |
| @namespace    | \n        | view namespace to use the directory structure let the directive empty |
| @ns           | \n        | view namespace to use the directory structure let the directive empty |
| @name         | \n        | view name to use the filename without it's extension let the directive empty |
| @layout       | \n        | layout used with that view, to use the default layout let the directive empty |
| @header       | ;         | view header definition |
| @import       | ;         | inline the specified document |
| @inherits     | ;         | view inherits override settings |
| @region       | \n        | foldable region start |
| @endregion    | \n        | foldable region end |
| @::           | ;         | write encoded output |
| @:            | ;         | write non encoded output |
| @if           |           | start a cpp 'if' code block |
| @switch       |           | start a cpp 'switch' code block |
| @while        |           | start a cpp 'while' code block |
| @for          |           | start a cpp 'for' code block |
| @do           |           | start a cpp 'do' code block |
| @try          |           | start a cpp 'try' code block |
| @funi         | }         | start a cpp class declaration code block |
| @func         | }         | start a cpp class definition code block |
| @<            | cpp-type> | create an async cpp function |
| @await        |           | call an async cpp function |
| @(lang){      | }         | start a markup block, lang can be 'htm' or 'md' |
| @this         |           | will be converted to view's generated instance variable name |
| @cb           |           | will be converted to view's callback variable name |
| @body         |           | render the view content in a layout, layout can also be included in layout |
| @filename     |           | the fanjet filename with it's extension |
| @directory    |           | the fanjet file parent directory |
| @>            | ;         | render a partial view. when a view is rendered from that call, the layout directive will ignored |
| @set(n,v)     |           | set key value pair, key is 'n' and value is 'v' |
| @get(n,d)     |           | write encoded value matching key 'n' from store, if not found 'd' is returned, 'd' is optional |
| @fmt(f,v...)  |           | write encoded format output, 'f' is the format string and 'v...' is arguments |
| @get-raw(n,d) |           | write non encoded value matching key 'n' from store, if not found 'd' is returned, 'd' is optional |
| @fmt-raw(f,v...) |        | write non encoded format output, 'f' is the format string and 'v...' is arguments |
| @style(style-filename, attr...) |  | a style file that will be append to the 'evmvc::response' list of styles which can be output with the @styles tag |
| @script(javascript-filename) |  | a javascript file that will be append to the 'evmvc::response' list of scripts which can be output with the @scripts tag |


# benchmark

$ cat curl-format.txt 
    time_namelookup:  %{time_namelookup}\n
       time_connect:  %{time_connect}\n
    time_appconnect:  %{time_appconnect}\n
   time_pretransfer:  %{time_pretransfer}\n
      time_redirect:  %{time_redirect}\n
 time_starttransfer:  %{time_starttransfer}\n
                    ----------\n
         time_total:  %{time_total}\n

curl -w "@curl-format.txt" -o /dev/null -s "http://127.0.0.1:8080/views/index"
