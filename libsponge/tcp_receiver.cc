#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

bool TCPReceiver::segment_received(const TCPSegment &seg) {
    bool syn = seg.header().syn;
    // bool fin = seg.header().fin;
    WrappingInt32 seq = seg.header().seqno;
    if(syn)
    {
        if(_syn == true) return false;
        _syn = true;
        _isn = seg.header().seqno;
        _checkpoint = 0;
    }
    
    if(_syn == false) return false;
    uint64_t be = unwrap(seq, _isn, _checkpoint);
    uint64_t en = be + seg.length_in_sequence_space();

    size_t x =_reassembler.assembled_bytes() + 1 - syn + _reassembler.stream_out().input_ended();
    if(seg.length_in_sequence_space() == 0 && be == x) return true;
    if(en <= x) return false;
    if(be >= x + window_size()) return false;

    _checkpoint = be;
    
    _reassembler.push_substring(seg.payload().copy(), be - 1 + syn, seg.header().fin);

    return true;
}

std::optional<WrappingInt32> TCPReceiver::ackno() const 
{ 
    if(_syn == false) return {};

    return wrap(_reassembler.assembled_bytes() + 1 + _reassembler.stream_out().input_ended(), _isn); 
}

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size() ; }
