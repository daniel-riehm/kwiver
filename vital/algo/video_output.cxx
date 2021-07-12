// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// video_output algorithm definition instantiation.

#include <vital/algo/video_output.h>

#include <vital/algo/algorithm.txx>

namespace kwiver {

namespace vital {

namespace algo {

// ----------------------------------------------------------------------------
const algorithm_capabilities::capability_name_t
video_output::SUPPORTS_METADATA( "supports-metadata" );

// ----------------------------------------------------------------------------
video_output
::video_output()
{
  attach_logger( "algo.video_output" );
}

// ----------------------------------------------------------------------------
video_output
::~video_output()
{
}

} // namespace algo

} // namespace vital

} // namespace kwiver

/// \cond DoxygenSuppress
INSTANTIATE_ALGORITHM_DEF( kwiver::vital::algo::video_output );
/// \endcond
