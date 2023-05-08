#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>
#include <iostream>
// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _in_fligth_bytes(0)
    , _initial_retransmission_timeout(retx_timeout)
    , _retransmission_timeout(retx_timeout)
    , _retry_times(0)
    , _timer(false)
    , _tim(0)    
    , _window_size(1)
    , _stream(capacity)
    , _now_ack(0)
    , _send_syn(false)
    , _send_fin(false) {}

uint64_t TCPSender::bytes_in_flight() const { return _in_fligth_bytes; }

void TCPSender::fill_window() 
{
    uint64_t _end_seq = _now_ack + _window_size;
    while(_next_seqno < _end_seq)
    {
        TCPSegment data;
        data.header().seqno = wrap(_next_seqno, _isn);
        
        size_t len = TCPConfig::MAX_PAYLOAD_SIZE;
        
        if(_end_seq - _next_seqno < len) len = _end_seq - _next_seqno;

        if(_send_syn == false && len > 0) 
        {
            _send_syn = true;
            data.header().syn = true;
            -- len;
        }
        string read_data = _stream.read(len);
        len -= read_data.length();
        data.payload() = Buffer(std::move(read_data));
        if(_send_fin == false && _stream.eof() && len > 0)
        {
            _send_fin = true;
            data.header().fin = true;
            -- len;
        }
        
        if(data.length_in_sequence_space() > 0)
        {
            _segments_out.push(data);
            _segments_outstanding.push(data);
            _next_seqno += data.length_in_sequence_space();
            _in_fligth_bytes += data.length_in_sequence_space();
        }

        if(len > 0) break;
    }
    _retransmission_timeout = _initial_retransmission_timeout;
    _retry_times = 0;   
    if(_segments_outstanding.empty() == false)
    {
        _timer = true;
        _tim = 0;
    }
    else{
        _timer = false;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
//! \returns `false` if the ackno appears invalid (acknowledges something the TCPSender hasn't sent yet)
bool TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    
    uint64_t _ack = unwrap(ackno, _isn, _next_seqno);
   
    if(_ack > _next_seqno) return false;
    _window_size = window_size;
    if(_ack <= _now_ack) return true;

    while(_segments_outstanding.empty() == false)
    {
        TCPSegment tmp = _segments_outstanding.front();
        
        uint64_t now = unwrap(tmp.header().seqno, _isn, _next_seqno);

        if(now >= _ack) break;
        _in_fligth_bytes -= tmp.length_in_sequence_space();
        _segments_outstanding.pop();
    }
    _now_ack = _ack;
    fill_window();
    return true;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) 
{
    if(_timer == false) return ;
    _tim += ms_since_last_tick;
    if(_tim >= _retransmission_timeout && _segments_outstanding.empty() == false) 
    {
        _retransmission_timeout <<= 1;
        _retry_times = _retry_times + 1;
        _tim = 0;
        _segments_out.push(_segments_outstanding.front());
    }
    return ;
}

unsigned int TCPSender::consecutive_retransmissions() const { return _retry_times; }

void TCPSender::send_empty_segment() 
{
    TCPSegment data;
    data.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(data);
}
