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
    : _data(), _unassembled(0), _assembled(0), _output(capacity), _capacity(capacity), _bitmap(vector<bool>(capacity)) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    
    if(index > _capacity) return ;

    {
        if(data.length() == 0)
            _data.push({_capacity - index, {data, eof}});
        size_t len = min(_capacity - index, data.length());
        int be = -1;
        for(size_t i = 0;i < len;i ++)
            if(_bitmap[i+index] == true)
            {
                if(be == -1) continue;
                _data.push({_capacity - be - index, {data.substr(be, i-be+1), false}});
                be = -1;
            }
            else {
                ++ _unassembled;
                _bitmap[i+index] = true;
                if(be == -1) be = i;
            }
        if(be != -1)
            _data.push({_capacity - be - index, {data.substr(be), eof}});
    }
    
    while(_data.empty() == false)
    {
        _unit tmp = _data.top();
        size_t str_be = static_cast<size_t>(_capacity - tmp.first);
        size_t str_en = str_be + tmp.second.first.length();

        // std::cout<<static_cast<size_t>(_capacity - tmp.first)<<' '<<str_en<<' '<<str_be<<' '<<tmp.second.first<<' '<<_data.size()<<' '<<_assembled<<endl;
        if(str_be > _assembled) break;
        _data.pop();        
        if(str_en < _assembled) continue;


        std::string &str = tmp.second.first;
        bool eo = tmp.second.second;

        _assembled += _output.write(str.substr(_assembled - str_be));

        // sleep(1); 
        if(eo) 
            _output.end_input();
    }

}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled - _assembled; }

bool StreamReassembler::empty() const { return _data.empty(); }
