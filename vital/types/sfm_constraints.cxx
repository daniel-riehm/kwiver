#include <vital/types/sfm_constraints.h>

#ifndef M_PI
// Source: http://www.geom.uiuc.edu/~huberty/math5337/groupe/digits.html
#define M_PI 3.141592653589793238462643383279502884197169399375105820974944592307816406
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD (M_PI/180.0)
#endif

#ifndef RAD_TO_DEG
#define RAD_TO_DEG (180.0/M_PI)
#endif

namespace kwiver {
namespace vital {

/// Private implementation class
class sfm_constraints::priv
{
public:
  /// Constructor
  priv();

  /// Destructor
  ~priv();

  struct im_data {
    int width;
    int height;

    im_data() :
      width(-1),
      height(-1)
    {

    }
    im_data(int w_, int h_):
      width(w_),
      height(h_)
    {

    }
    im_data(const im_data& other):
      width(other.width),
      height(other.height)
    {

    }

  };

  metadata_map_sptr m_md;
  local_geo_cs m_lgcs;
  std::map<frame_id_t, im_data> m_image_data;
};

sfm_constraints::priv
::priv()
{

}

sfm_constraints::priv
::~priv()
{

}

sfm_constraints
::sfm_constraints(const sfm_constraints& other)
  : m_priv(new priv)
{
  m_priv->m_lgcs = other.m_priv->m_lgcs;
  m_priv->m_md = other.m_priv->m_md;
  m_priv->m_image_data = other.m_priv->m_image_data;
}

sfm_constraints
::sfm_constraints()
  : m_priv(new priv)
{

}

sfm_constraints
::sfm_constraints( metadata_map_sptr md,
                   local_geo_cs const& lgcs)
  :m_priv(new priv)
{
  m_priv->m_md = md;
  m_priv->m_lgcs = lgcs;
}

sfm_constraints
::~sfm_constraints()
{

}

metadata_map_sptr
sfm_constraints
::get_metadata()
{
  return m_priv->m_md;
}

void
sfm_constraints
::set_metadata(metadata_map_sptr md)
{
  m_priv->m_md = md;
}

local_geo_cs
sfm_constraints
::get_local_geo_cs()
{
  return m_priv->m_lgcs;
}

void
sfm_constraints
::set_local_geo_cs(local_geo_cs const& lgcs)
{
  m_priv->m_lgcs = lgcs;
}

bool
sfm_constraints
::get_focal_length_prior(frame_id_t fid, float &focal_length) const
{
  if (!m_priv->m_md)
  {
    return false;
  }

  auto &md = *m_priv->m_md;

  std::set<frame_id_t> frame_ids_to_try;

  int image_width = -1;
  if (!get_image_width(fid, image_width))
  {
    return false;
  }

  if (fid >= 0)
  {
    frame_ids_to_try.insert(fid);
  }
  else
  {
    frame_ids_to_try = md.frames();
  }

  for (auto test_fid : frame_ids_to_try)
  {

    double hfov;

    if (md.get_horizontal_field_of_view(test_fid, hfov))
    {
      focal_length = static_cast<float>((image_width*0.5) / tan(0.5*hfov*DEG_TO_RAD));
      return true;
    }

    double target_width, slant_range;
    bool has_target_width = md.get_target_width(test_fid, target_width);
    bool has_slant_range = md.get_slant_range(test_fid, slant_range);

    if (has_target_width && has_slant_range)
    {
      focal_length = static_cast<float>(image_width * slant_range / target_width);

      return true;
    }
  }

  return false;
}

bool
sfm_constraints
::get_camera_orientation_prior_local(frame_id_t fid,
                                     rotation_d &R_loc) const
{
  if (m_priv->m_lgcs.origin().is_empty())
  {
    return false;
  }

  if (!m_priv->m_md)
  {
    return false;
  }

  auto &md = *m_priv->m_md;

  double platform_heading, platform_roll,   platform_pitch;
  double sensor_rel_az,    sensor_rel_roll, sensor_rel_el;

  if (md.get_platform_heading_angle(fid, platform_heading) &&
      md.get_platform_roll_angle(   fid, platform_roll) &&
      md.get_platform_pitch_angle(  fid, platform_pitch) &&
      md.get_sensor_rel_az_angle(   fid, sensor_rel_az) &&
      md.get_sensor_rel_el_angle(   fid, sensor_rel_el))
  {
    if (!md.get_sensor_rel_roll_angle(fid, sensor_rel_roll))
    {
      sensor_rel_roll = 0;
    }

    if (std::isnan(platform_heading) || std::isnan(platform_pitch) || std::isnan(platform_roll) ||
        std::isnan(sensor_rel_az) || std::isnan(sensor_rel_el) || std::isnan(sensor_rel_roll))
    {
      return false;
    }

    R_loc = m_priv->m_lgcs.compose_rotation(platform_heading, platform_pitch, platform_roll,
                                            sensor_rel_az, sensor_rel_el, sensor_rel_roll);

    return true;
  }

  return false;
}


bool
sfm_constraints
::get_camera_position_prior_local(frame_id_t fid,
                                  vector_3d &pos_loc) const
{
  if (m_priv->m_lgcs.origin().is_empty())
  {
    return false;
  }

  if (!m_priv->m_md)
  {
    return false;
  }

  kwiver::vital::geo_point gloc;
  double alt;
  if (!m_priv->m_md->get_sensor_location(fid, gloc) ||
      !m_priv->m_md->get_sensor_altitude(fid, alt))
  {
    return false;
  }

  auto geo_origin = m_priv->m_lgcs.origin();
  vector_2d loc = gloc.location(geo_origin.crs());
  loc -= geo_origin.location();
  alt -= m_priv->m_lgcs.origin_altitude();

  pos_loc[0] = loc.x();
  pos_loc[1] = loc.y();
  pos_loc[2] = alt;

  return true;
}

sfm_constraints::position_map
sfm_constraints
::get_camera_position_priors() const
{
  position_map local_positions;

  auto md = m_priv->m_md->metadata();

  for (auto mdv : md)
  {
    auto fid = mdv.first;

    vector_3d loc;
    if (!get_camera_position_prior_local(fid, loc))
    {
      continue;
    }
    if (local_positions.empty())
    {
      local_positions[fid] = loc;
    }
    else
    {
      auto last_loc = local_positions.crbegin()->second;
      if (loc == last_loc)
      {
        continue;
      }
      local_positions[fid] = loc;
    }
  }
  return local_positions;
}

void
sfm_constraints
::store_image_size(frame_id_t fid, int image_width, int image_height)
{
  m_priv->m_image_data[fid] = priv::im_data(image_width, image_height);
}

bool
sfm_constraints
::get_image_height(frame_id_t fid, int &image_height) const
{
  if (fid >= 0)
  {
    auto data_it = m_priv->m_image_data.find(fid);
    if (data_it == m_priv->m_image_data.end())
    {
      return false;
    }
    image_height = data_it->second.height;
    return true;
  }
  else
  {
    if (m_priv->m_image_data.empty())
    {
      return false;
    }
    image_height = m_priv->m_image_data.begin()->second.height;
    return true;
  }
}

bool
sfm_constraints
::get_image_width(frame_id_t fid, int &image_width) const
{
  if (fid >= 0)
  {
    auto data_it = m_priv->m_image_data.find(fid);
    if (data_it == m_priv->m_image_data.end())
    {
      return false;
    }
    image_width = data_it->second.width;
    return true;
  }
  else
  {
    if (m_priv->m_image_data.empty())
    {
      return false;
    }
    image_width = m_priv->m_image_data.begin()->second.width;
    return true;
  }
}


}
}