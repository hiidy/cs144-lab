#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

#include <functional>

class TCPSender
{
public:
  class Timer
  {
  public:
    Timer( uint64_t initial_timeout ) : RTO_timeout_( initial_timeout ), time_elapsed_( 0 ), running_( false ) {}

    void start()
    {
      running_ = true;
      time_elapsed_ = 0;
    }

    void stop() { running_ = false; }

    void restart()
    {
      time_elapsed_ = 0;
      running_ = true;
    }

    void tick( uint64_t time )
    {
      if ( running_ ) {
        time_elapsed_ += time;
      }
    }

    bool running() { return running_; }

    bool expired() { return running_ && RTO_timeout_ <= time_elapsed_; }

    void double_timeout() { RTO_timeout_ = RTO_timeout_ * 2; }

    void reset_timeout( uint64_t new_timeout ) { RTO_timeout_ = new_timeout; }

  private:
    uint64_t RTO_timeout_;
    uint64_t time_elapsed_;
    bool running_;
  };
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( ByteStream&& input, Wrap32 isn, uint64_t initial_RTO_ms )
    : input_( std::move( input ) )
    , isn_( isn )
    , initial_RTO_ms_( initial_RTO_ms )
    , outstanding_segments_()
    , timer_( initial_RTO_ms )
  {}

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage make_empty_message() const;

  /* Receive and process a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Type of the `transmit` function that the push and tick methods can use to send messages */
  using TransmitFunction = std::function<void( const TCPSenderMessage& )>;

  /* Push bytes from the outbound stream */
  void push( const TransmitFunction& transmit );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called */
  void tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit );

  // Accessors
  uint64_t sequence_numbers_in_flight() const;  // For testing: how many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // For testing: how many consecutive retransmissions have happened?
  const Writer& writer() const { return input_.writer(); }
  const Reader& reader() const { return input_.reader(); }
  Writer& writer() { return input_.writer(); }

private:
  Reader& reader() { return input_.reader(); }

  ByteStream input_;
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;
  uint16_t window_size_ = 1;
  bool sync_sent_ = false;
  bool fin_sent_ = false;
  std::deque<TCPSenderMessage> outstanding_segments_;
  uint64_t next_seqno_ = 0;
  uint64_t last_acked_ = 0;
  Timer timer_;
  uint64_t consecutive_retransmissions_ = 0;
};
