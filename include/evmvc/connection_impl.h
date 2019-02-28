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

#include "app.h"
#include "connection.h"

namespace evmvc { 


sp_http_worker connection::get_worker() const
{
    return _worker.lock();
}

void connection::close()
{
    if(_closed)
        return;
    _closed = true;
    EVMVC_TRACE(this->_log, "closing\n{}", this->debug_string());
    
    _parser.reset();
    if(_resume_ev){
        event_free(_resume_ev);
        _resume_ev = nullptr;
    }
    if(_bev){
        if(_ssl){
            SSL_set_shutdown(_ssl, SSL_RECEIVED_SHUTDOWN);
            SSL_shutdown(_ssl);
            _ssl = nullptr;
        }
        bufferevent_flush(_bev, EV_READ | EV_WRITE, BEV_FINISHED);
        bufferevent_free(_bev);
        ::close(_sock_fd);
        _sock_fd = -1;
        _bev = nullptr;
    }
    
    if(auto w = _worker.lock())
        w->remove_connection(this->_id);
}

void connection::_send_file_chunk_start()
{
    _file->res->headers().remove("Content-Length");
    _file->res->headers().set("Transfer-Encoding", "chunked");
    
    _file->res->_prepare_headers();
    _file->res->_started = true;
}

void connection::_send_chunk(struct evbuffer* chunk)
{
    if(evbuffer_get_length(chunk) == 0)
        return;
    
    struct evbuffer* out = bev_out();
    evbuffer_add_printf(out, "%x\r\n", (unsigned)evbuffer_get_length(out));
    bufferevent_write_buffer(_bev, chunk);
    evbuffer_add(out, "\r\n", 2);
    bufferevent_flush(_bev, EV_WRITE, BEV_FLUSH);
}

void connection::_send_file_chunk_end()
{
    evbuffer_add(bev_out(), "0\r\n\r\n", 5);
    bufferevent_flush(_bev, EV_WRITE, BEV_FLUSH);
    unset_conn_flag(conn_flags::sending_file);
}


evmvc::status connection::_send_file_chunk()
{
    char buf[EVMVC_READ_BUF_SIZE];
    size_t bytes_read;
    
    /* try to read EVMVC_READ_BUF_SIZE bytes from the file pointer */
    bytes_read = fread(buf, 1, sizeof(buf), _file->file_desc);
    
    if(bytes_read > 0){
        // verify if we need to compress data
        if(_file->zs){
            _file->zs->next_in = (Bytef*)buf;
            _file->zs->avail_in = bytes_read;
            
            // retrieve the compressed bytes blockwise.
            int ret;
            char zbuf[EVMVC_READ_BUF_SIZE];
            int flush_mode = feof(_file->file_desc) ?
                Z_FINISH : Z_SYNC_FLUSH;
            bytes_read = 0;
            
            _file->zs->next_out = reinterpret_cast<Bytef*>(zbuf);
            _file->zs->avail_out = sizeof(zbuf);
            
            ret = deflate(_file->zs, flush_mode);
            if(_file->zs_size < _file->zs->total_out){
                bytes_read += _file->zs->total_out - _file->zs_size;
                evbuffer_add(
                    _file->buffer,
                    zbuf,
                    _file->zs->total_out - _file->zs_size
                );
                _file->zs_size = _file->zs->total_out;
            }
            
            if(ret != Z_OK && ret != Z_STREAM_END){
                _file->res->log()->error(
                    "zlib deflate function returned: '{}'",
                    ret
                );
                this->_send_file_chunk_end();
                return evmvc::status::internal_server_error;
            }
            
        }else{
            evbuffer_add(_file->buffer, buf, bytes_read);
        }
        
        if(bytes_read > 0){
            EVMVC_TRACE(_file->res->log(),
                "Sending {} bytes for '{}'",
                bytes_read, _file->res->req()->uri().to_string()
            );
            
            _send_chunk(_file->buffer);
            evbuffer_drain(_file->buffer, bytes_read);
        }
    }
    
    if(feof(_file->file_desc)){
        EVMVC_TRACE(_file->res->log(),
            "Sending last chunk for '{}'",
            _file->res->req()->uri().to_string()
        );
        
        this->_send_file_chunk_end();
    }
    
    return evmvc::status::ok;
}






void connection::on_connection_resume(int /*fd*/, short /*events*/, void* arg)
{
    connection* c = (connection*)arg;
    EVMVC_DBG(c->_log, "resuming");
    
    c->unset_conn_flag(conn_flags::paused);
    
    if(c->flag_is(conn_flags::wait_release)){
        c->close();
        return;
    }
    
    if(evbuffer_get_length(c->bev_out())){
        EVMVC_DBG(c->_log, "SET WAITING");

        c->set_conn_flag(conn_flags::waiting);
        if(!(bufferevent_get_enabled(c->_bev) & EV_WRITE)){
            EVMVC_DBG(c->_log, "ENABLING EV_WRITE");
            bufferevent_enable(c->_bev, EV_WRITE);
        }
        
        return;
    }else{
        EVMVC_DBG(c->_log, "SET READING");
        if(!(bufferevent_get_enabled(c->_bev) & EV_READ)){
            EVMVC_DBG(c->_log, "ENABLING EV_READ | EV_WRITE");
            bufferevent_enable(c->_bev, EV_READ | EV_WRITE);
        }
        
        if(evbuffer_get_length(c->bev_in())){
            EVMVC_DBG(c->_log, "data available calling on_connection_read");
            on_connection_read(c->_bev, arg);
        }
        
        return;
    }
    
    if(c->_parser->completed())
        c->_parser->exec();
}

void connection::on_connection_read(struct bufferevent* /*bev*/, void* arg)
{
    connection* c = (connection*)arg;
    EVMVC_TRACE(c->_log, "on_connection_read\n{}", c->debug_string());
    
    size_t ilen = evbuffer_get_length(c->bev_in());
    if(ilen == 0){
        return;
    }
    
    if(c->flag_is(conn_flags::paused))
        return EVMVC_DBG(c->log(), "Connection is paused, do nothing!");
    
    if(c->_parser->completed() && c->_parser->_res && c->_parser->_res->ended())
        c->_parser->reset();
    
    void* buf = evbuffer_pullup(c->bev_in(), ilen);
    
    cb_error ec;
    size_t nread = c->_parser->parse((const char*)buf, ilen, ec);
    if(nread > 0)
        evbuffer_drain(c->bev_in(), nread);
    EVMVC_TRACE(c->_log, "Parsing done, nread: {}", nread);
    
    if(ec){
        c->log()->error("Parse error:\n{}", ec);
        c->set_conn_flag(conn_flags::error);
        return;
    }
    
    if(c->_parser->_res && c->_parser->_res->paused())
        return;
    
    if(c->_parser->completed())
        c->_parser->exec();
    
    if(nread < ilen){
        c->set_conn_flag(conn_flags::waiting);
        c->resume();
    }
}

void connection::on_connection_write(struct bufferevent* /*bev*/, void* arg)
{
    connection* c = (connection*)arg;
    EVMVC_TRACE(c->_log, "on_connection_write\n{}", c->debug_string());
    
    if(c->flag_is(conn_flags::paused))
        return;
    
    if(c->flag_is(conn_flags::waiting)){
        c->unset_conn_flag(conn_flags::waiting);
        if(!(bufferevent_get_enabled(c->_bev) & EV_READ))
            bufferevent_enable(c->_bev, EV_READ);
        
        if(evbuffer_get_length(c->bev_in())){
            on_connection_read(c->_bev, arg);
            return;
        }
    }
    
    if(c->flag_is(conn_flags::sending_file))
        c->_send_file_chunk();
    
    if(c->_parser->_res && !c->_parser->_res->ended())
        return;
    
    // still have data to send
    if(evbuffer_get_length(c->bev_out()))
        return;
    
    // request completed
    if(c->flag_is(conn_flags::keepalive)){
        c->_parser->reset();
    }else{
        c->close();
    }
}

void connection::on_connection_event(
    struct bufferevent* /*bev*/, short events, void* arg)
{
    connection* c = (connection*)arg;
    
    EVMVC_TRACE(c->_log,
        "conn: {}\n"
        "events: {}{}{}{}",
        c->debug_string(),
        events & BEV_EVENT_CONNECTED ? "connected " : "",
        events & BEV_EVENT_ERROR     ? "error "     : "",
        events & BEV_EVENT_TIMEOUT   ? "timeout "   : "",
        events & BEV_EVENT_EOF       ? "eof"       : ""
    );
    
    if((events & BEV_EVENT_TIMEOUT)){
        c->close();
        return;
    }
    
    if((events & BEV_EVENT_CONNECTED)){
        EVMVC_DBG(c->_log, "CONNECTED");
        return;
    }
    
    if(c->_ssl && !(events & BEV_EVENT_EOF)){
        // ssl error
        c->set_conn_flag(conn_flags::error);
    }
    
    if(events == (BEV_EVENT_EOF | BEV_EVENT_READING)){
        EVMVC_DBG(c->_log, "EOF | READING");
        if(errno == EAGAIN){
            EVMVC_DBG(c->_log, "errno EAGAIN");
            
            if(!(bufferevent_get_enabled(c->_bev) & EV_READ))
                bufferevent_enable(c->_bev, EV_READ);
            errno = 0;
            return;
        }
    }
    
    c->set_conn_flag(conn_flags::error);
    c->unset_conn_flag(conn_flags::connected);
    
    if(c->flag_is(conn_flags::paused))
        c->set_conn_flag(conn_flags::wait_release);
    else
        c->close();
}

}//::evmvc