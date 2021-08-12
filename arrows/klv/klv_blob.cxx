// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV blob class and associated functions.

#include "klv_blob.h"

#include <vital/exceptions/metadata.h>

#include <algorithm>
#include <deque>
#include <vector>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
klv_blob
::klv_blob()
{}

// ----------------------------------------------------------------------------
klv_blob
::klv_blob( klv_bytes_t const& bytes ) : bytes{ bytes }
{}

// ----------------------------------------------------------------------------
klv_bytes_t&
klv_blob
::operator*()
{
  return bytes;
}

// ----------------------------------------------------------------------------
klv_bytes_t const&
klv_blob
::operator*() const
{
  return bytes;
}

// ----------------------------------------------------------------------------
klv_bytes_t*
klv_blob
::operator->()
{
  return &bytes;
}

// ----------------------------------------------------------------------------
klv_bytes_t const*
klv_blob
::operator->() const
{
  return &bytes;
}

// ----------------------------------------------------------------------------
#define KLV_ASSERT_UINT8_ITERATOR( ITER )                         \
  static_assert(                                                  \
    std::is_same< typename std::decay< decltype( *ITER ) >::type, \
                  uint8_t >::value,                               \
    "iterator must point to uint8_t" )

// ----------------------------------------------------------------------------
template < class Iterator >
klv_blob
klv_read_blob( Iterator& data, size_t length )
{
  KLV_ASSERT_UINT8_ITERATOR( data );

  auto const begin = data;
  auto const end = ( data += length );
  return klv_bytes_t{ begin, end };
}

// ----------------------------------------------------------------------------
template < class Iterator >
void
klv_write_blob( klv_blob const& value, Iterator& data, size_t max_length )
{
  KLV_ASSERT_UINT8_ITERATOR( data );

  if( max_length < value->size() )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "writing blob overruns end of data buffer" );
  }

  data = std::copy( value->cbegin(), value->cend(), data );
}

// ----------------------------------------------------------------------------
size_t
klv_blob_length( klv_blob const& value )
{
  return value->size();
}

// ----------------------------------------------------------------------------
#define KLV_INSTANTIATE_ALL_CITER( INSTANTIATE )                  \
  INSTANTIATE( uint8_t const* );                                  \
  INSTANTIATE( typename std::vector< uint8_t >::const_iterator ); \
  INSTANTIATE( typename std::deque<  uint8_t >::const_iterator );

#define KLV_INSTANTIATE_ALL_WITER( INSTANTIATE )            \
  INSTANTIATE( uint8_t* );                                  \
  INSTANTIATE( typename std::vector< uint8_t >::iterator ); \
  INSTANTIATE( typename std::deque<  uint8_t >::iterator );

#define KLV_INSTANTIATE_ALL_ITER( ... )     \
  KLV_INSTANTIATE_ALL_CITER( __VA_ARGS__ ); \
  KLV_INSTANTIATE_ALL_WITER( __VA_ARGS__ )

#define KLV_INSTANTIATE_READ( ITERATOR )   \
  template KWIVER_ALGO_KLV_EXPORT klv_blob \
  klv_read_blob< ITERATOR >( ITERATOR&, size_t )

#define KLV_INSTANTIATE_WRITE( ITERATOR ) \
  template KWIVER_ALGO_KLV_EXPORT void    \
  klv_write_blob< ITERATOR >( klv_blob const&, ITERATOR&, size_t )

KLV_INSTANTIATE_ALL_ITER( KLV_INSTANTIATE_READ );
KLV_INSTANTIATE_ALL_WITER( KLV_INSTANTIATE_WRITE );

} // namespace klv

} // namespace arrows

} // namespace kwiver
