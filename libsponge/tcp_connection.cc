#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_recv_segment; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    bool rst = seg.header().rst;
    bool ack = seg.header().ack;
    bool syn = seg.header().syn;
    _time_since_last_recv_segment = 0;

    if (TCPState::state_summary(_sender) == TCPSenderStateSummary::SYN_SENT) {
        if (ack && !syn && !rst)  return;
        if (!ack && rst)          return;
    }

    if (rst == true) {
        _rst = true;
        set_rst_state(false);
    }
    if (_rst == true || _is_live == false)
        return;

    bool need_to_send_empty_seg = false;

   if( _receiver.segment_received(seg) == false && _receiver.ackno().has_value() && seg.payload().size())
        need_to_send_empty_seg = true;

    if (ack == true && _sender.next_seqno_absolute() > 0) {
        if (_sender.ack_received(seg.header().ackno, seg.header().win) == false)
            need_to_send_empty_seg = true;
    }
    
    if (TCPState::state_summary(_receiver) == TCPReceiverStateSummary::SYN_RECV &&
        TCPState::state_summary(_sender) == TCPSenderStateSummary::CLOSED) {
        connect();
        return;
    }

    if (TCPState::state_summary(_receiver) == TCPReceiverStateSummary::FIN_RECV &&
        TCPState::state_summary(_sender) == TCPSenderStateSummary::SYN_ACKED)
        _linger_after_streams_finish = false;

    if (TCPState::state_summary(_receiver) == TCPReceiverStateSummary::FIN_RECV &&
        TCPState::state_summary(_sender) == TCPSenderStateSummary::FIN_ACKED && !_linger_after_streams_finish)
        _is_live = false;
   
    if(seg.length_in_sequence_space() > 0 && _receiver.ackno().has_value())
        need_to_send_empty_seg = true;

    if (need_to_send_empty_seg && _is_live)
        _sender.send_empty_segment();
    sender_to_this();
}

bool TCPConnection::active() const { return _is_live; }

size_t TCPConnection::write(const string &data) { return _sender.stream_in().write(data); }

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    _sender.tick(ms_since_last_tick);

    if (_rst)
        return;
    if (_sender.consecutive_retransmissions() >= TCPConfig::MAX_RETX_ATTEMPTS) {
        set_rst_state(true);
        return;
    }
    sender_to_this();
    _time_since_last_recv_segment += ms_since_last_tick;

    if (TCPState::state_summary(_receiver) == TCPReceiverStateSummary::FIN_RECV &&
        TCPState::state_summary(_sender) == TCPSenderStateSummary::FIN_ACKED && _linger_after_streams_finish == true &&
        time_since_last_segment_received() >= 10 * _cfg.rt_timeout) {
        _is_live = false;
    }
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();
    sender_to_this();
}
void TCPConnection::set_rst_state(bool to_send_segment) {
    _rst = true;
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    _linger_after_streams_finish = false;
    _is_live = false;
    if (to_send_segment) {
        TCPSegment data;
        data.header().rst = true;
        data.header().seqno = _sender.next_seqno();
        _segments_out.push(data);
    }
}
void TCPConnection::connect() {
    _sender.fill_window();
    sender_to_this();
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
            if (_rst == false) {
                _rst = true;
                set_rst_state(true);
            }
            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

void TCPConnection::sender_to_this() {
    while (_sender.segments_out().empty() == false) {
        TCPSegment data = _sender.segments_out().front();
        _sender.segments_out().pop();
        if (_receiver.ackno().has_value() == true) {
            data.header().ack = true;
            data.header().ackno = _receiver.ackno().value();
            data.header().win = _receiver.window_size();
        }
        _segments_out.push(data);
    }
    return;
}