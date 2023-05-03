#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

bool TCPReceiver::segment_received(const TCPSegment &seg) {
    bool syn = seg.header().syn;
    bool fin = seg.header().fin;
    WrappingInt32 seq = seg.header().seqno;
    if(syn)
    {
        _syn = true;
        _isn = seg.header().seqno;
        _checkpoint = 0;
    }
    
    if(_syn == false) return false;
    uint64_t be = unwrap(seq, _isn, _checkpoint);
    uint64_t en = be + seg.length_in_sequence_space();


    if(en <= _reassembler.assembled_bytes()) return false;
    if(be >= _reassembler.assembled_bytes() + window_size()) return false;
    

    uint64_t index = unwrap(seq, _isn, _checkpoint);
    size_t str_len = seg.payload().copy().length();
    if(syn == true)
    {
        _reassembler.push_substring("\0", index, false);
        ++ index;
    }
    
    if(str_len > 0 || (syn == false && fin == false))
    {
        _reassembler.push_substring(seg.payload().copy(), index, false);
        
        if(str_len == 0) index ++;
        else index += str_len;
    }
    if(fin == true)
        _reassembler.push_substring("\0", index, fin);
    _checkpoint = index;

    return true;
}

optional<WrappingInt32> TCPReceiver::ackno() const 
{ 
    if(_syn == false) return {};
    return wrap(_reassembler.assembled_bytes(), _isn); }

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size() ; }
