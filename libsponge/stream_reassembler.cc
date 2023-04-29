#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _data(), _output(0), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    _unit this_data = _unit{-index, {data, eof}};
    _data.push(this_data);
    while(_data.empty() == false)
    {
        _unit tmp = _data.top();
        if( -tmp.first > _output.buffer_size()) break;
        std::string str = tmp.second.first;
        bool eo = tmp.second.second;
        _output.write(str);
        if(eo) 
            _output.eof();
        _data.pop();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return {}; }

bool StreamReassembler::empty() const { return {}; }
