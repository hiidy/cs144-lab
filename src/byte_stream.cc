#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : buffer_(), capacity_( capacity ), closed_( false ), total_pushed_(), total_popped_()
{}

void Writer::push( string data )
{
  size_t size = std::min( data.size(), capacity_ - buffer_.size() );
  buffer_.append( data.substr( 0, size ) );
  total_pushed_ += size;
}

void Writer::close()
{
  closed_ = true;
}

bool Writer::is_closed() const
{
  return closed_;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - buffer_.size();
}

uint64_t Writer::bytes_pushed() const
{
  return total_pushed_;
}

string_view Reader::peek() const
{
  return { buffer_.data(), buffer_.size() };
}

void Reader::pop( uint64_t len )
{
  size_t size = std::min( len, buffer_.size() );
  buffer_.erase( 0, size );
  total_popped_ += size;
}

bool Reader::is_finished() const
{
  return closed_ && buffer_.empty();
}

uint64_t Reader::bytes_buffered() const
{
  return buffer_.size(); // Your code here.
}

uint64_t Reader::bytes_popped() const
{
  return total_popped_;
}
