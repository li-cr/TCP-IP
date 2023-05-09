#include "stream_reassembler.hh"
#include <iostream>
#include <unistd.h>
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    :  _bitmap(vector<bool>(capacity)),  _unassembled(0),_assembled(0), _fin_index(0), _fin(false), _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const uint64_t index, const bool eof) {

    {
        int be = -1;
        size_t end_i = 0;
        string tmp = "";
        for(size_t i = 0;i < data.length();i ++, end_i = i)
        {
            if(i+index < _assembled || i+index >= _assembled + _capacity - _output.buffer_size()) continue;
            if(_fin && i + index >= _fin_index) continue;

            int now = (i+index) % _capacity;
            if(_bitmap[now] == true)
            {
                if(be == -1) continue;
                _data[be] = {tmp, i == data.length() ? eof : false};
                tmp = "";  be = -1;
            }
            else{
                _bitmap[now] = true;
                ++ _unassembled;
                tmp.push_back(data[i]);
                if(be == -1) be = i + index;
            }
        }
        if(be != -1) 
            _data[be] = {tmp, false};
        if(end_i == data.length() && eof)
        {
            _fin_index = index + data.length();
            _bitmap[_fin_index] = true;
            _fin = true;
        }
    }

    while(_bitmap[mod(_assembled)] == true)
    {
        if(_fin && _assembled == _fin_index)
        {
            _output.end_input();
            break;
        }
        
        string str = _data[_assembled].first;
        bool eo = _data[_assembled].second;
        
        size_t len = _output.write(str);
       
        _unassembled -= len;
        _data.erase(_assembled);
        size_t new_assembled = _assembled + len;
        
        while(_assembled != new_assembled)
            _bitmap[mod(_assembled)] = false,
            ++ _assembled;

        if(len < str.length())
            _data[_assembled] = {str.substr(len), eo};

    }
}   

size_t StreamReassembler::unassembled_bytes() const { return _unassembled; }
size_t StreamReassembler::assembled_bytes() const { return _assembled; }

bool StreamReassembler::empty() const { return _unassembled == 0;}
