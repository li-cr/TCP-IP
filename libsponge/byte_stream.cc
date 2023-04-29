#include "byte_stream.hh"

#include <algorithm>
#include <iterator>
#include <stdexcept>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t caacity)
    : _queue(), capacity(caacity), _eof(false), in_num(0), out_num(0), _error(false)
        {}

size_t ByteStream::write(const string &data) {

    if(this->_eof) 
        return 0;

    size_t num = min(data.size(), this->remaining_capacity());
    for(size_t i = 0;i < num;i ++)
            _queue.push_back(data[i]);

    this->in_num += num;

    return num;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    
    size_t num = min(len, this->buffer_size());
    return string(_queue.begin(), _queue.begin() + num);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) 
{
    size_t num = min(len, this->buffer_size());
    string ans;
    for(size_t i = 0;i < num;i ++)
        _queue.pop_front();
    this->out_num += num;
    return ;
}

void ByteStream::end_input() { this->_eof = true;}

bool ByteStream::input_ended() const { return this->_eof; }

size_t ByteStream::buffer_size() const { return _queue.size(); }

bool ByteStream::buffer_empty() const { return _queue.empty(); }

bool ByteStream::eof() const { return this->_eof && this->buffer_empty(); }

size_t ByteStream::bytes_written() const { return this->in_num; }

size_t ByteStream::bytes_read() const { return this->out_num; }

size_t ByteStream::remaining_capacity() const { return capacity - _queue.size(); }
