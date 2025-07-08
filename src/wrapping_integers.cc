#include "wrapping_integers.hh"
#include "debug.hh"

#include <iostream>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  debug( "unimplemented wrap( {}, {} ) called", n, zero_point.raw_value_ );
  uint32_t result = (zero_point.raw_value_ + n % (1ULL << 32));
  return Wrap32( result );
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  debug( "unimplemented unwrap( {}, {} ) called", zero_point.raw_value_, checkpoint );
  return {};
}
