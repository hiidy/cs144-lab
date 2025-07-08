#include "wrapping_integers.hh"
#include "debug.hh"

#include <iostream>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  debug( "unimplemented wrap( {}, {} ) called", n, zero_point.raw_value_ );
  uint32_t result = zero_point.raw_value_ + static_cast<uint32_t>(n);
  return Wrap32( result );
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint32_t rel_seq = this->raw_value_ - zero_point.raw_value_;
  uint64_t base = checkpoint & ~((1ULL << 32) - 1);
  uint64_t first = base + rel_seq;
  uint64_t second = base + rel_seq + (1ULL << 32);
  uint64_t third = (base + rel_seq >= (1ULL << 32)) ? base + rel_seq - (1ULL << 32) : first;
  uint64_t best = first;
  if (abs_diff( best, checkpoint ) > abs_diff( second, checkpoint )) {
    best = second;
  }
  if (abs_diff( best, checkpoint ) > abs_diff( third, checkpoint )) {
    best = third;
  }
  return best;
}

uint64_t Wrap32::abs_diff(uint64_t a, uint64_t b) {
  return a > b ? a - b : b - a;
}
