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

#ifndef _libevmvc_response_h
#define _libevmvc_response_h

#include "stable_headers.h"
#include "logging.h"
#include "connection.h"
#include "statuses.h"
#include "headers.h"
#include "fields.h"
#include "mime.h"
#include "cookies.h"
#include "request.h"

#include <boost/filesystem.hpp>


namespace evmvc {

namespace _internal {
    struct file_reply {
        sp_response res;
        evhtp_request_t* request;
        FILE* file_desc;
        struct evbuffer* buffer;
        z_stream* zs;
        uLong zs_size;
        evmvc::async_cb cb;
        evmvc::sp_logger log;
    };
    
    static evhtp_res send_file_chunk(evhtp_connection_t* conn, void* arg);
    static evhtp_res send_file_fini(
        struct evhtp_connection* c, void* arg);
}


class response
    : public std::enable_shared_from_this<response>
{
    friend evmvc::sp_response _internal::create_http_response(
        sp_logger log,
        evhtp_request_t* ev_req,
        sp_route rt,
        const std::vector<std::shared_ptr<evmvc::http_param>>& params
        );
    
public:
    
    response(
        uint64_t id,
        const sp_route& rt,
        evmvc::sp_logger log,
        evhtp_request_t* ev_req,
        const sp_http_cookies& http_cookies)
        : _id(id), _rt(rt),
        _log(log->add_child(
            "res-" + evmvc::num_to_str(id, false) + 
            ev_req->uri->path->full
        )),
        _ev_req(ev_req), 
        // _headers(std::make_shared<response_headers>(
        //     ev_req->headers_out
        // )),
        _cookies(http_cookies),
        _started(false), _ended(false),
        _status(-1), _type(""), _enc(""), _paused(false)
    {
        EVMVC_DEF_TRACE(
            "response '{}' created", this->id()
        );
    }
    
    ~response()
    {
        EVMVC_DEF_TRACE(
            "response '{}' released", this->id()
        );
    }
    
    static sp_response null(wp_app a, evhtp_request_t* ev_req);
    
    uint64_t id() const { return _id;}
    
    
    sp_connection connection() const;
    bool secure() const;
    
    evmvc::sp_app get_app() const;
    evmvc::sp_router get_router()const;
    evmvc::sp_route get_route()const { return _rt;}
    evmvc::sp_logger log() const { return _log;}
    
    evhtp_request_t* evhtp_request(){ return _ev_req;}
    evmvc::response_headers& headers() const { return *(_headers.get());}
    http_cookies& cookies() const { return *(_cookies.get());}
    evmvc::sp_request req() const { return _req;}
    
    bool paused() const { return _paused;}
    void pause()
    {
        if(_paused)
            return;
        this->log()->debug("Connection paused");
        _paused = true;
        evhtp_request_pause(_ev_req);
    }
    
    void resume()
    {
        if(!_paused)
            return;
        evhtp_request_resume(_ev_req);
        this->log()->debug("Connection resumed");
        _paused = false;
    }
    
    bool started(){ return _started;};
    bool ended(){ return _ended;}
    void end()
    {
        if(!_started)
            this->encoding("utf-8").type("txt")._start_reply();
        
        evhtp_send_reply_end(_ev_req);
        _ended = true;
    }
    

    
    int16_t get_status()
    {
        return _status == -1 ? (int16_t)evmvc::status::ok : _status;
    }
    
    response& status(evmvc::status code)
    {
        _status = (uint16_t)code;
        return *this;
    }
    
    response& status(uint16_t code)
    {
        _status = code;
        return *this;
    }
    
    bool has_encoding(){ return !_enc.empty();}
    evmvc::string_view encoding(){ return _enc;}
    response& encoding(evmvc::string_view enc)
    {
        _enc = enc.to_string();
        return *this;
    }
    
    bool has_type(){ return !_type.empty();}
    evmvc::string_view get_type()
    {
        return _type.empty() ? "" : _type;
    }
    
    response& type(evmvc::string_view type, evmvc::string_view enc = "")
    {
        _type = evmvc::mime::get_type(type);
        if(_type.empty())
            _type = type.data();
        if(enc.size() > 0)
            _enc = enc.to_string();
        
        std::string ct = _type;
        if(!_enc.empty())
            ct += "; charset=" + _enc;
        this->headers().set(evmvc::field::content_type, ct);
        
        return *this;
    }
    
    void send_status(evmvc::status code)
    {
        this->status((uint16_t)code)
            .send(evmvc::statuses::status((uint16_t)code));
    }
    
    void send_status(uint16_t code)
    {
        this->status(code);
        this->send(evmvc::statuses::status(code));
    }
    
    void send(evmvc::string_view body)
    {
        this->resume();
        if(_type.empty())
            this->type("txt", "utf-8");
        
        struct evbuffer* b = evbuffer_new();
        
        int len = evbuffer_add_printf(b, "%s", body.data());
        this->headers().set(
            evmvc::field::content_length, evmvc::num_to_str(len)
        );
        
        _start_reply();
        evhtp_send_reply_body(_ev_req, b);
        
        evbuffer_free(b);
        
        this->end();
    }
    
    void html(evmvc::string_view html_val)
    {
        this->encoding("utf-8").type("html").send(html_val);
    }
    
    template<
        typename T, 
        typename std::enable_if<
            std::is_same<T, evmvc::json>::value,
            int32_t
        >::type = -1
    >
    void send(const T& json_val)
    {
        this->encoding("utf-8").type("json").send(json_val.dump());
    }
    
    void json(const evmvc::json& json_val)
    {
        this->encoding("utf-8").type("json").send(json_val.dump());
    }
    
    void jsonp(
        const evmvc::json& json_val,
        bool escape = false,
        evmvc::string_view cb_name = "callback"
        )
    {
        if(!this->has_encoding())
            this->encoding("utf-8");
        if(!this->has_type()){
            this->headers().set("X-Content-Type-Options", "nosniff");
            this->type("text/javascript");
        }
        
        std::string sv = json_val.dump();
        evmvc::replace_substring(sv, "\u2028", "\\u2028");
        evmvc::replace_substring(sv, "\u2029", "\\u2029");
        
        sv = "/**/ typeof " + cb_name.to_string() + " === 'function' && " +
            cb_name.to_string() + "(" + sv + ");";
        this->send(sv);
    }
    
    void download(const bfs::path& filepath,
        evmvc::string_view filename = "",
        const evmvc::string_view& enc = "utf-8", 
        async_cb cb = evmvc::noop_cb)
    {
        this->headers().set(
            evmvc::field::content_disposition,
            fmt::format(
                "attachment; filename={}",
                filename.size() == 0 ?
                    filepath.filename().c_str() :
                    filename.data()
            )
        );
        this->send_file(filepath, enc, cb);
    }
    
    void send_file(
        const bfs::path& filepath,
        const evmvc::string_view& enc = "utf-8", 
        async_cb cb = evmvc::noop_cb);
    
    void error(const cb_error& err)
    {
        this->error(evmvc::status::internal_server_error, err);
    }
    void error(evmvc::status err_status, const cb_error& err);
    
    void redirect(evmvc::string_view new_location,
        evmvc::status redir_status = evmvc::status::found)
    {
        switch (redir_status){
            // Permanent redirections
            case evmvc::status::moved_permanently:
            case evmvc::status::permanent_redirect:

            // Temporary redirections
            case evmvc::status::found:
            case evmvc::status::see_other:
            case evmvc::status::temporary_redirect:

            // Special redirections
            case evmvc::status::multiple_choices:
            case evmvc::status::not_modified:
                
                this->headers().set(evmvc::field::location, new_location);
                this->send_status(redir_status);
                break;
            
            default:
                throw EVMVC_ERR(
                    "Invalid redirection status!\nSee 'https://developer.mozilla.org/en-US/docs/Web/HTTP/Redirections' for valid statuses!"
                );
        }
    }
    
private:

    void _prepare_headers()
    {
        if(_started)
            throw std::runtime_error(
                "unable to prepare headers after response is started!"
            );
        
        // look for keepalive
        evhtp_kv_t* header = nullptr;
        if((header = evhtp_headers_find_header(
            _ev_req->headers_in, "Connection"
        )) != nullptr){
            std::string con_val(header->val);
            evmvc::trim(con_val);
            evmvc::lower_case(con_val);
            if(con_val == "keep-alive"){
                this->headers().set(evmvc::field::connection, "keep-alive");
                evhtp_request_set_keepalive(_ev_req, 1);
            } else
                evhtp_request_set_keepalive(_ev_req, 0);
        } else if(evhtp_request_get_proto(_ev_req) == EVHTP_PROTO_10)
            evhtp_request_set_keepalive(_ev_req, 0);
    }
    
    void _start_reply()
    {
        _prepare_headers();
        _started = true;
        evhtp_send_reply_start(_ev_req, _status);
    }
    
    uint64_t _id;
    wp_connection _conn;
    
    sp_route _rt;
    evmvc::sp_logger _log;
    evhtp_request_t* _ev_req;
    evmvc::sp_response_headers _headers;
    evmvc::sp_request _req;
    sp_http_cookies _cookies;
    bool _started;
    bool _ended;
    int16_t _status;
    std::string _type;
    std::string _enc;
    bool _paused;
};

namespace _internal {
    static evhtp_res send_file_chunk(evhtp_connection_t* conn, void* arg)
    {
        struct file_reply* reply = (struct file_reply*)arg;
        char buf[EVMVC_READ_BUF_SIZE];
        size_t bytes_read;
        
        /* try to read EVMVC_READ_BUF_SIZE bytes from the file pointer */
        bytes_read = fread(buf, 1, sizeof(buf), reply->file_desc);
        
        if(bytes_read > 0){
            // verify if we need to compress data
            if(reply->zs){
                reply->zs->next_in = (Bytef*)buf;
                reply->zs->avail_in = bytes_read;
                
                // retrieve the compressed bytes blockwise.
                int ret;
                char zbuf[EVMVC_READ_BUF_SIZE];
                int flush_mode = feof(reply->file_desc) ?
                    Z_FINISH : Z_SYNC_FLUSH;
                bytes_read = 0;
                
                reply->zs->next_out = reinterpret_cast<Bytef*>(zbuf);
                reply->zs->avail_out = sizeof(zbuf);
                
                ret = deflate(reply->zs, flush_mode);
                if(reply->zs_size < reply->zs->total_out){
                    bytes_read += reply->zs->total_out - reply->zs_size;
                    evbuffer_add(
                        reply->buffer,
                        zbuf,
                        reply->zs->total_out - reply->zs_size
                    );
                    reply->zs_size = reply->zs->total_out;
                }
                
                if(ret != Z_OK && ret != Z_STREAM_END){
                    reply->res->log()->error(
                        "zlib deflate function returned: '{}'",
                        ret
                    );
                    evhtp_send_reply_chunk_end(reply->request);
                    return EVHTP_RES_SERVERR;
                }
                
            }else{
                /* add our data we read from the file into our reply buffer */
                evbuffer_add(reply->buffer, buf, bytes_read);
            }
            
            if(bytes_read > 0){
                EVMVC_TRACE(reply->res->log(),
                    "Sending {} bytes for '{}'",
                    bytes_read, reply->request->uri->path->full
                );
                
                /* send the reply buffer as a http chunked message */
                evhtp_send_reply_chunk(reply->request, reply->buffer);
                
                /* we can now drain our reply buffer as to not be a resource
                * hog.
                */
                evbuffer_drain(reply->buffer, bytes_read);
            }
        }
        
        /* check if we have read everything from the file */
        if(feof(reply->file_desc)){
            EVMVC_TRACE(reply->res->log(),
                "Sending last chunk for '{}'",
                reply->request->uri->path->full
            );
            
            /* now that we have read everything from the file, we must
            * first unset our on_write hook, then inform evhtp to send
            * this message as the final chunk.
            */
            evhtp_connection_unset_hook(conn, evhtp_hook_on_write);
            evhtp_send_reply_chunk_end(reply->request);

            /* we can now free up our little reply_ structure */
            {
                if(reply->zs){
                    deflateEnd(reply->zs);
                    free(reply->zs);
                    reply->zs = nullptr;
                }
                
                fclose(reply->file_desc);
                
                evhtp_safe_free(reply->buffer, evbuffer_free);
                //evhtp_safe_free(reply, free);
                delete reply;
                
                // no need for the connection fini hook anymore
                evhtp_connection_unset_hook(
                    conn, evhtp_hook_on_connection_fini
                );
            }
        }
        
        return EVHTP_RES_OK;
    }
    
    static evhtp_res send_file_fini(
        struct evhtp_connection* c, void* arg)
    {
        struct file_reply* reply = (struct file_reply*)arg;
        
        if(reply->zs){
            deflateEnd(reply->zs);
            free(reply->zs);
            reply->zs = nullptr;
        }
        
        fclose(reply->file_desc);
        evhtp_safe_free(reply->buffer, evbuffer_free);
        //evhtp_safe_free(reply, free);
        delete reply;
        
        return EVHTP_RES_OK;
    }
}


} //ns evmvc
#endif //_libevmvc_response_h
