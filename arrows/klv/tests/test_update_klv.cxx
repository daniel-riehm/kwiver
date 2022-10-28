// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test the update_klv filter.

#include "data_format.h"

#include <vital/plugin_loader/plugin_manager.h>

#include <arrows/klv/update_klv.h>
#include <arrows/klv/klv_metadata.h>
#include <arrows/klv/klv_1108.h>
#include <arrows/klv/klv_1108_metric_set.h>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  TEST_LOAD_PLUGINS();
  return RUN_ALL_TESTS();
}

namespace kv = kwiver::vital;
using namespace kwiver::arrows::klv;

// ----------------------------------------------------------------------------
// Ensure we can create the filter with the factory method.
TEST ( update_klv, create )
{
  EXPECT_NE( nullptr, kv::algo::metadata_filter::create( "update_klv" ) );
}

// ----------------------------------------------------------------------------
// No metadata given.
TEST ( update_klv, empty )
{
  update_klv filter;
  kv::metadata_vector input;
  auto const output = filter.filter( input, nullptr );
  EXPECT_EQ( input, output );
}

// ----------------------------------------------------------------------------
// Null metadata pointer.
TEST ( update_klv, null_metadata_sptr )
{
  update_klv filter;
  kv::metadata_vector input{ nullptr };
  auto const output = filter.filter( input, nullptr );
  EXPECT_EQ( input, output );
}

// ----------------------------------------------------------------------------
// Metadata objects with no KLV attached.
TEST ( update_klv, non_klv_metadata_sptr )
{
  update_klv filter;
  kv::metadata_vector input{
    std::make_shared< kv::metadata >(),
    std::make_shared< kv::metadata >(),
  };
  input[ 0 ]->add< kv::VITAL_META_UNIX_TIMESTAMP >( 0 );
  input[ 0 ]->add< kv::VITAL_META_AVERAGE_GSD >( 12.0 );
  input[ 1 ]->add< kv::VITAL_META_UNIX_TIMESTAMP >( 1 );
  auto const output = filter.filter( input, nullptr );
  ASSERT_EQ( 2, output.size() );
  EXPECT_EQ(
    0, output.at( 0 )->find( kv::VITAL_META_UNIX_TIMESTAMP ).as_uint64() );
  EXPECT_EQ(
    1, output.at( 1 )->find( kv::VITAL_META_UNIX_TIMESTAMP ).as_uint64() );
}

// ----------------------------------------------------------------------------
// Adding in a new ST1108 packet.
TEST( update_klv, add_st_1108 )
{
  update_klv filter;
  kv::metadata_vector input{ std::make_shared< klv_metadata >() };
  input[ 0 ]->add< kv::VITAL_META_VIDEO_DATA_STREAM_INDEX >( 1 );
  input[ 0 ]->add< kv::VITAL_META_UNIX_TIMESTAMP >( 1 );
  input[ 0 ]->add< kv::VITAL_META_AVERAGE_GSD >( 12.0 );
  input[ 0 ]->add< kv::VITAL_META_VNIIRS >( 5.0 );
  input[ 0 ]->add< kv::VITAL_META_VIDEO_BITRATE >( 500000 );
  input[ 0 ]->add< kv::VITAL_META_VIDEO_COMPRESSION_TYPE >( "H.264" );
  input[ 0 ]->add< kv::VITAL_META_VIDEO_COMPRESSION_PROFILE >( "Main" );
  input[ 0 ]->add< kv::VITAL_META_VIDEO_COMPRESSION_LEVEL >( "4.1" );
  input[ 0 ]->add< kv::VITAL_META_VIDEO_FRAME_RATE >( 30.0 );
  input[ 0 ]->add< kv::VITAL_META_IMAGE_WIDTH >( 1280 );
  input[ 0 ]->add< kv::VITAL_META_IMAGE_HEIGHT >( 720 );

  auto const output = filter.filter( input, nullptr );
  ASSERT_EQ( 1, output.size() );
  auto output_klv =
    dynamic_cast< klv_metadata const& >( *output[ 0 ] ).klv();

  // We have to treat the timestamp fields specially, since they should be the
  // current time and we can't hardcode the correct value for that.
  auto& set = output_klv[ 0 ].value.get< klv_local_set >();
  for( auto& entry : set.all_at( KLV_1108_METRIC_LOCAL_SET ) )
  {
    auto& metric_set = entry.second.get< klv_local_set >();
    ASSERT_EQ( 1, metric_set.count( KLV_1108_METRIC_SET_TIME ) );

    // Check that timestamp is sane - some time between when this was written
    // and 2100.
    auto const timestamp =
      metric_set.at( KLV_1108_METRIC_SET_TIME ).get< uint64_t >();
    EXPECT_GT( timestamp, 1670000000000000);
    EXPECT_LT( timestamp, 4102462800000000);

    // Remove the timestamp field
    metric_set.erase( KLV_1108_METRIC_SET_TIME );
  }

  std::vector< klv_packet > expected_klv { {
    { klv_1108_key(), klv_local_set{
      { KLV_1108_ASSESSMENT_POINT, KLV_1108_ASSESSMENT_POINT_ARCHIVE },
      { KLV_1108_METRIC_PERIOD_PACK, klv_1108_metric_period_pack{ 1, 33333 } },
      { KLV_1108_METRIC_LOCAL_SET, klv_local_set{
        { KLV_1108_METRIC_SET_NAME, std::string{ "GSD" } },
        { KLV_1108_METRIC_SET_VERSION, std::string{} },
        { KLV_1108_METRIC_SET_IMPLEMENTER,
          klv_1108_kwiver_metric_implementer() },
        { KLV_1108_METRIC_SET_PARAMETERS, std::string{} },
        { KLV_1108_METRIC_SET_VALUE, klv_lengthy< double >{ 12.0 } } } },
      { KLV_1108_METRIC_LOCAL_SET, klv_local_set{
        { KLV_1108_METRIC_SET_NAME, std::string{ "VNIIRS" } },
        { KLV_1108_METRIC_SET_VERSION, std::string{} },
        { KLV_1108_METRIC_SET_IMPLEMENTER,
          klv_1108_kwiver_metric_implementer() },
        { KLV_1108_METRIC_SET_PARAMETERS, std::string{} },
        { KLV_1108_METRIC_SET_VALUE, klv_lengthy< double >{ 5.0 } } } },
      { KLV_1108_COMPRESSION_TYPE, KLV_1108_COMPRESSION_TYPE_H264 },
      { KLV_1108_COMPRESSION_PROFILE, KLV_1108_COMPRESSION_PROFILE_MAIN },
      { KLV_1108_COMPRESSION_LEVEL, std::string{ "4.1" } },
      { KLV_1108_COMPRESSION_RATIO, klv_lengthy< double >{ 1327.104 } },
      { KLV_1108_STREAM_BITRATE, uint64_t{ 500 } },
      { KLV_1108_DOCUMENT_VERSION, uint64_t{ 3 } } } } } };

  EXPECT_EQ( expected_klv, output_klv );
}
