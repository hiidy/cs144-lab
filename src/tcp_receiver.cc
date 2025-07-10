#include "tcp_receiver.hh"
#include "debug.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if ( message.RST ) {
    const_cast<Writer&>( reassembler_.writer() ).set_error();
    return;
  }

  if ( message.SYN && !has_syn_ ) {
    isn_ = message.seqno;
    has_syn_ = true;
  }

  uint64_t abs_seqno = message.seqno.unwrap( isn_, writer().bytes_pushed() + 1 );
  uint64_t stream_index;

  if ( message.SYN ) {
    stream_index = abs_seqno;
  } else {
    stream_index = abs_seqno - 1;
  }

  if ( !message.payload.empty() || message.FIN ) {
    reassembler_.insert( stream_index, message.payload, message.FIN );
  }
}

TCPReceiverMessage TCPReceiver::send() const
{
  TCPReceiverMessage msg;
  if ( has_syn_ ) {
    uint64_t next_seq_num = writer().bytes_pushed() + 1;
    if ( writer().is_closed() ) {
      next_seq_num++;
    }
    auto ackno = Wrap32::wrap( next_seq_num, isn_ );
    msg.ackno = ackno;
  }

  if ( writer().has_error() ) {
    msg.RST = true;
  }
  msg.window_size
    = static_cast<uint16_t>( min( writer().available_capacity(), static_cast<size_t>( UINT16_MAX ) ) );

  return msg;
}
