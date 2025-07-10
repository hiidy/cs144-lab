#include "reassembler.hh"
#include "debug.hh"

#include <iostream>

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  uint64_t first_unassembled_index = output_.writer().bytes_pushed();

  if ( is_last_substring ) {
    eof_seen_ = true;
    eof_index_ = first_index + data.size();
  }

  uint64_t first_unacceptable_index = first_unassembled_index + output_.writer().available_capacity();

  if ( first_index >= first_unacceptable_index ) {
    return;
  }

  if ( first_index < first_unassembled_index ) {
    size_t size = min( first_unassembled_index, first_index + data.size() ) - first_index;
    data = data.substr( size );
    first_index = first_unassembled_index;
  }

  if ( first_index + data.size() > first_unacceptable_index ) {
    size_t size = first_index + data.size() - first_unacceptable_index;
    data = data.substr( 0, data.size() - size );
  }

  // merge
  for ( auto it = unassembled_chunks_.begin(); it != unassembled_chunks_.end(); ) {
    uint64_t chunk_idx = it->first;
    auto chunk_data = it->second;
    uint64_t chunk_end = chunk_idx + chunk_data.size();
    uint64_t data_end = first_index + data.size();

    if ( first_index <= chunk_idx && chunk_end <= data_end ) {
      it = unassembled_chunks_.erase( it );
      continue;
    }

    if ( chunk_idx < first_index && chunk_end > first_index ) {
      size_t overlap = chunk_end - first_index;
      if ( chunk_end >= data_end ) {
        data = chunk_data;
      } else {
        data = chunk_data.substr( 0, chunk_data.size() - overlap ) + data;
      }
      first_index = chunk_idx;
      it = unassembled_chunks_.erase( it );
    } else if ( data_end > chunk_idx && data_end < chunk_end ) {
      size_t overlap = data_end - chunk_idx;
      data = data + chunk_data.substr( overlap );
      it = unassembled_chunks_.erase( it );
    } else {
      ++it;
    }
  }

  unassembled_chunks_[first_index] = data;

  while ( true ) {
    auto pair = unassembled_chunks_.find( first_unassembled_index );
    if ( pair == unassembled_chunks_.end() ) {
      break;
    }
    output_.writer().push( pair->second );
    first_unassembled_index += pair->second.size();
    unassembled_chunks_.erase( pair );
  }

  if ( eof_seen_ && output_.writer().bytes_pushed() == eof_index_ ) {
    output_.writer().close();
  }
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  uint64_t total = 0;
  for ( auto it : unassembled_chunks_ ) {
    total += it.second.size();
  }
  return total;
}
