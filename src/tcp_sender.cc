#include "tcp_sender.hh"
#include "debug.hh"
#include "tcp_config.hh"

using namespace std;

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return next_seqno_ - last_acked_;
}

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::consecutive_retransmissions() const
{
  return consecutive_retransmissions_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  TCPSenderMessage msg;
  uint16_t window_size;
  string payload;
  if ( window_size_ == 0 ) {
    window_size = 1;
  } else {
    window_size = window_size_;
  }

  if ( !sync_sent_ ) {
    msg.SYN = true;
    sync_sent_ = true;
    msg.seqno = isn_;
    msg.payload = payload;
    next_seqno_++;

    if ( reader().is_finished() && !fin_sent_ && next_seqno_ < last_acked_ + window_size ) {
      msg.FIN = true;
      fin_sent_ = true;
      next_seqno_ += 1;
    }

    outstanding_segments_.push_back( msg );
    if ( msg.sequence_length() != 0 && !timer_.running() ) {
      timer_.start();
    }
    transmit( msg );
    return;
  }

  if ( !fin_sent_ && next_seqno_ < last_acked_ + window_size && reader().is_finished() ) {
    msg.FIN = true;
    fin_sent_ = true;
    msg.seqno = Wrap32::wrap( next_seqno_, isn_ );
    next_seqno_ += 1;
    outstanding_segments_.push_back( msg );
    if ( msg.sequence_length() != 0 && !timer_.running() ) {
      timer_.start();
    }
    transmit( msg );
    return;
  }

  while ( next_seqno_ < last_acked_ + window_size ) {
    string_view data = reader().peek();
    size_t window_remain = ( window_size + last_acked_ ) - next_seqno_;
    size_t max_send = min( min( data.size(), window_remain ), TCPConfig::MAX_PAYLOAD_SIZE );

    if ( max_send == 0 ) {
      return;
    }

    msg.payload = string( data.substr( 0, max_send ) );
    msg.seqno = Wrap32::wrap( next_seqno_, isn_ );
    reader().pop( msg.payload.size() );
    next_seqno_ += msg.payload.size();
    if ( reader().is_finished() && next_seqno_ < last_acked_ + window_size ) {
      msg.FIN = true;
      fin_sent_ = true;
      next_seqno_ += 1;
    }
    outstanding_segments_.push_back( msg );
    if ( msg.sequence_length() != 0 && !timer_.running() ) {
      timer_.start();
    }
    transmit( msg );
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage msg;
  msg.seqno = Wrap32::wrap( next_seqno_, isn_ );
  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  window_size_ = msg.window_size;
  if ( !msg.ackno.has_value() ) {
    return;
  }

  uint64_t rcv_last_acked_ = msg.ackno->unwrap( isn_, last_acked_ );
  if ( rcv_last_acked_ < last_acked_ || rcv_last_acked_ > next_seqno_ ) {
    return;
  }

  if ( rcv_last_acked_ > last_acked_ ) {
    timer_.reset_timeout( initial_RTO_ms_ );
    if ( !outstanding_segments_.empty() ) {
      timer_.restart();
    }
    consecutive_retransmissions_ = 0;
  }

  last_acked_ = rcv_last_acked_;
  while ( !outstanding_segments_.empty() ) {
    const auto& seg = outstanding_segments_.front();
    uint64_t seg_start = seg.seqno.unwrap( isn_, last_acked_ );
    uint64_t seg_end = seg_start + seg.sequence_length();

    if ( last_acked_ >= seg_end ) {
      outstanding_segments_.pop_front();
    } else {
      break;
    }
  }

  if ( outstanding_segments_.empty() ) {
    timer_.stop();
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  if ( outstanding_segments_.empty() && !timer_.running() )
    return;

  timer_.tick( ms_since_last_tick );

  if ( timer_.expired() ) {
    if ( !outstanding_segments_.empty() ) {
      const auto& msg = outstanding_segments_.front();
      transmit( msg );

      if ( window_size_ > 0 ) {
        consecutive_retransmissions_++;
        timer_.double_timeout();
      }
      timer_.start();
    }
  }
}
