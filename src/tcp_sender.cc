#include "tcp_sender.hh"
#include "debug.hh"
#include "tcp_config.hh"

using namespace std;

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  debug( "unimplemented sequence_numbers_in_flight() called" );
  return next_seqno_ - last_acked_;
}

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::consecutive_retransmissions() const
{
  debug( "unimplemented consecutive_retransmissions() called" );
  return {};
}

void TCPSender::push( const TransmitFunction& transmit )
{
  debug( "unimplemented push() called" );
  TCPSenderMessage msg;
  string payload;
  if ( !sync_sent_ ) {
    msg.SYN = true;
    sync_sent_ = true;
    msg.seqno = isn_;
    msg.payload = payload;
    next_seqno_++;
    outstanding_segments_.push_back( msg );
    transmit( msg );
    return;
  }

  if ( next_seqno_ < last_acked_ + window_size_ ) {
    string_view data = reader().peek();
    size_t window_remain = ( window_size_ + last_acked_ ) - next_seqno_;
    size_t max_send = min( min( data.size(), window_remain ), TCPConfig::MAX_PAYLOAD_SIZE );

    if ( max_send == 0 ) {
      return;
    }

    if ( reader().is_finished() ) {
      msg.FIN = true;
      next_seqno_++;
    }

    msg.payload = string( data.substr( 0, max_send ) );
    msg.seqno = Wrap32::wrap( next_seqno_, isn_ );
    reader().pop( msg.payload.size() );
    next_seqno_ += msg.payload.size();
    outstanding_segments_.push_back( msg );
    transmit( msg );
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  debug( "unimplemented make_empty_message() called" );
  TCPSenderMessage msg;
  msg.seqno = Wrap32::wrap( next_seqno_, isn_ );
  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  debug( "unimplemented receive() called" );
  window_size_ = msg.window_size;
  if ( !msg.ackno.has_value() ) {
    return;
  }
  last_acked_ = msg.ackno->unwrap( isn_, last_acked_ );

  while ( !outstanding_segments_.empty()
          && outstanding_segments_.front().seqno.unwrap( isn_, last_acked_ ) <= last_acked_ ) {
    outstanding_segments_.pop_front();
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  debug( "unimplemented tick({}, ...) called", ms_since_last_tick );
  (void)transmit;
}
