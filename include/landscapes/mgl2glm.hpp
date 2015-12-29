#ifndef SVO_MGL2GLM_HPP
#define SVO_MGL2GLM_HPP


#include "MathGeoLib.h"
#include "landscapes/opencl.shim.h"

namespace svo{
    
static inline glm::vec3 toglm(float3 v)
{
    return float3_t(v.x,v.y,v.z);
}

static inline glm::vec4 toglm(float4 v)
{
    return glm::vec4(v[0],v[1],v[2],v[3]);
}


} // namespace svo

#endif

