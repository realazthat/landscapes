#ifndef SVO_TREE_RAYMARCH_H
#define SVO_TREE_RAYMARCH_H 1

#include "cubelib/cubelib.h"
#include "svo_inttypes.h"
#include "opencl.shim.h"
#include "common.math.cl.h"
#include "svo_tree.capi.h"

#define MAXIMUM_TREE_DEPTH 30

#ifdef __OPENCL_VERSION__


#else
#include <cassert>


#include <vector>
#include <tuple>
#include "landscapes/svo_formatters.hpp"
#include "format.h"
#include <bitset>

#ifndef NDEBUG
#define RAYMARCHASSERTS
#endif

#endif



#define RAYMARCHDEBUG 1
//#define RAYMARCHDEBUGT0 1
//#define RAYMARCHDEBUGT1 1
//#define RAYMARCHDEBUGSELECTFIRSTNODE 1
//#define RAYMARCHDEBUGLOCATENODE 1
//#define RAYMARCHDEBUGRAYMARCH 1
//#define RAYMARCHDEBUGVOXELPIXELERROR 1
//#define RAYMARCHDEBUGTRACE 1






//#define RAYMARCHDEBUGRAYMARCHLOWER 1


typedef goffset_t node_descriptor_t;

typedef struct node_info_t{
    node_descriptor_t parent;
    corner_t corner;

#ifdef RAYMARCHDEBUGRAYMARCHLOWER
    float3_t lower;
#endif
} node_info_t;




GLOBAL_STATIC_CONST node_info_t null_node = (node_info_t){
      (goffset_t)0, NULL_CORNER
#ifdef RAYMARCHDEBUGRAYMARCHLOWER
    , make_float3(0,0,0)
#endif
};

static inline node_info_t make_node_info(node_descriptor_t parent, corner_t corner)
{
    return (node_info_t){parent, corner};
}



static inline bool svo_tree_has_children(const uint8_t* address_space, node_info_t node);
static inline bool svo_tree_has_nonempty_voxel(const uint8_t* address_space, node_info_t node);
static inline node_info_t svo_tree_get_child(const uint8_t* address_space, node_info_t node, corner_t corner);
static inline bool svo_tree_is_null(node_info_t node);
static inline bool svo_tree_isleaf_f3(const uint8_t* address_space, node_info_t node);
static inline bool svo_tree_voxelexists(const uint8_t* address_space, node_info_t node);


static inline bool svo_tree_is_null(node_info_t node)
{
    bool is_null = node.parent == invalid_goffset;
    ///if its null, the corner should also be null
    ///null(parent) => null(corner)
    assert(!is_null || is_null_corner(node.corner));
    
    ///null(corner) => null(parent)
    assert(!is_null_corner(node.corner) || is_null);
    
    return is_null;
}


static inline
bool svo_tree_has_children(const uint8_t* address_space, node_info_t node)
{
    const child_descriptor_t* pcd = svo_cget_cd(address_space, node.parent);
    
    ccurve_t ccurve = corner2ccurve(node.corner);
    
    assert(svo_get_valid_bit(pcd, ccurve));
    
    if (!svo_get_nonleaf_bit(pcd, ccurve))
    {
        assert(svo_get_leaf_bit(pcd,ccurve));
        return false;
    }
    assert(!svo_get_leaf_bit(pcd,ccurve));
    
    ///TODO: the rest of this stuff might be unecessary, if we assume that every CD
    /// only exists if it has some valid children
    
    
    goffset_t cd_goffset = svo_get_child_cd_goffset(address_space, node.parent, pcd, ccurve);
    
    assert(cd_goffset != invalid_goffset);
    
    const child_descriptor_t* cd = svo_cget_cd(address_space, cd_goffset);
    
    return svo_get_cd_valid_count(cd) > 0;
}

static inline
bool svo_tree_has_nonempty_voxel(const uint8_t* address_space, node_info_t node)
{
    ///TODO: FIXME: not sure what this function is supposed to do; if we have the node of something,
    /// it is non-empty, *shrug*
    return true;
}

static inline
node_info_t svo_tree_get_child(const uint8_t* address_space, node_info_t node, corner_t corner)
{
    const child_descriptor_t* pcd = svo_cget_cd(address_space, node.parent);
    assert( svo_get_cd_nonleaf_count(pcd) > 0 );
    assert( svo_get_valid_bit(pcd, corner2ccurve(node.corner)) );
    
    goffset_t cd_goffset = svo_get_child_cd_goffset(address_space, node.parent, pcd, corner2ccurve(node.corner));
    
    assert(cd_goffset != invalid_goffset);
    
    ///note this corner might not actually be a valid child of the CD
    return make_node_info(cd_goffset, corner);
}


static inline
bool svo_tree_voxelexists(const uint8_t* address_space, node_info_t node)
{
    
    if (node.parent == invalid_goffset)
        return false;
    
    
    const child_descriptor_t* pcd = svo_cget_cd(address_space, node.parent);
    
    return svo_get_valid_bit(pcd, corner2ccurve(node.corner));
}

static inline
bool svo_tree_isleaf_f3(const uint8_t* address_space, node_info_t node)
{
    return !svo_tree_has_children(address_space,node);
}

static inline
bool svo_intersects_voxel_data_f3(const uint8_t* address_space, node_info_t node, float3_t raypos, float3_t raydir)
{
    return svo_tree_voxelexists(address_space,node);
}



static inline node_info_t tree_null()
{
    return make_node_info(invalid_goffset, null_corner);
}





typedef struct svo_stack_t{

    node_info_t ptr[MAXIMUM_TREE_DEPTH];
    size_t size;
    size_t capacity;
} svo_stack_t;

static inline void svo_stack_push(svo_stack_t* stack_ptr, const node_info_t* value0);
static inline void svo_stack_pop(svo_stack_t* stack_ptr);
static inline node_info_t svo_stack_back(svo_stack_t* stack_ptr);
static inline float3_t calculate_abs_position_f3(const svo_stack_t* stack, corner_t current_corner);

static inline void svo_stack_push(svo_stack_t* stack_ptr, const node_info_t* value0)
{
    assert(stack_ptr->size < stack_ptr->capacity);

    stack_ptr->ptr[stack_ptr->size] = *value0;
    
    stack_ptr->size++;
    assert(stack_ptr->size < stack_ptr->capacity);
    assert(stack_ptr->size > 0);
}

static inline void svo_stack_pop(svo_stack_t* stack_ptr)
{
    assert(stack_ptr->size < stack_ptr->capacity);
    assert(stack_ptr->size > 0);
    stack_ptr->size--;
    assert(stack_ptr->size < stack_ptr->capacity);
}

static inline node_info_t svo_stack_back(svo_stack_t* stack_ptr)
{
    assert(stack_ptr->size < stack_ptr->capacity);
    assert(stack_ptr->size > 0);
    return stack_ptr->ptr[stack_ptr->size - 1];
}

















typedef struct dir_bounds_t{
    float3_t lower;
    float3_t upper;
} dir_bounds_t;

static inline dir_bounds_t make_dir_bounds(float3_t lower, float3_t upper)
{
    return (dir_bounds_t){lower,upper};
}

/**
 * The position of the lower,upper corner of the cube; where "upper" is in the direction of the ray's exit.
 * 
 * @param lower
 *          The lower corner of the cube.
 * @param upper
 *          The upper corner of the cube.
 * @param raydir
 *          The direction of the ray.
 */
static inline
dir_bounds_t svo_calculate_dir_bounds_f3(float3_t lower, float3_t upper, float3_t raydir)
{
    /*
    float3_t dir_lower;
    float3_t dir_upper;
    for (int i = 0; i < 3; ++i)
    {
        dir_upper[i] = (raydir[i] < 0) ? lower[i] : upper[i];
        dir_lower[i] = (raydir[i] < 0) ? upper[i] : lower[i];
    }
     */
    
    float3_t dir_upper = glm_select(upper, lower, glm_lt(raydir,(make_float3(0))));
    float3_t dir_lower = glm_select(lower, upper, glm_lt(raydir,(make_float3(0))));
    //float3_t dir_upper = (raydir < make_float3(0)) ? lower : upper;
    //float3_t dir_lower = (raydir < make_float3(0)) ? upper : lower;
    return make_dir_bounds(dir_lower, dir_upper);
}














typedef struct cube_hit_t{
    float3_t position;
    face_t face;
} cube_hit_t;

static inline cube_hit_t make_cube_hit(float3_t position, face_t face)
{
    return (cube_hit_t){position, face};
}


/**
 * Calculates the point of exit of a ray and a cube.
 *
 * @param raypos
 *          Origin of the ray.
 * @param raydir
 *          The direction of the ray.
 * @param dir_upper
 *          The position of the upper corner of the cube; where "upper" is in the direction of the ray's exit.
 */
static inline
cube_hit_t calculate_t1_f3( float3_t raypos
                 , float3_t raydir
                 , float3_t dir_upper)
{
#ifdef __cplusplus
    using namespace svo;
#endif
    ///Basically, we project the 3 "upper" faces of the cube as infinite planes.
    ///Then we intersect each of these planes with the ray.
    ///The one with the shortest distance along the ray to the intersection is the
    /// intersection we seek.

    for (int i = 0; i < 3; ++i)
    {
        assert( !std::isnan(raypos[i]) );
        assert( !std::isinf(raypos[i]) );
    }

    ///FIXME: account for NaN and Inf
    //float QARRAY[3];
    //float& Qx = QARRAY[0];
    //float& Qy = QARRAY[1];
    //float& Qz = QARRAY[2];

    float3_t Qv = (dir_upper - raypos) / raydir;

    //Qx = std::max(Qx, -Qx);
    //Qy = std::max(Qy, -Qy);
    //Qz = std::max(Qz, -Qz);

    ///We can early return here in some cases, if any of them are nan, and this will give us a good
    /// point on the surface, but it might be very far from dir_upper, so let's not.
    /*
    if (std::isnan(Qx))
        return std::make_tuple( raypos, raydir.x < 0 ? negxface : posxface );
    else if (std::isnan(Qy))
        return std::make_tuple( raypos, raydir.y < 0 ? negyface : posyface );
    else if (std::isnan(Qz))
        return std::make_tuple( raypos, raydir.z < 0 ? negzface : poszface );
    */

    ///anything that is nan should be discarded
    Qv = coalescenanf3(Qv,make_float3(fposinf));

    ///anything that is -/+inf should be discarded
    Qv = coalesceinff3(Qv,make_float3(fposinf));

#ifdef RAYMARCHDEBUGT1
        std::cout
            << "  calculate_t1:"
            << "  raypos: " << raypos
            << ", raydir: " << raydir
            << ", dir_upper: " << dir_upper
            << ", Qi: " << " (" << Q.x << "," << Q.y << "," << Q.z << ")"

            << std::endl;
#endif

    if (glm_any(glm_lt(Qv, make_float3(0))))
    {
        assert(Qv.x < 0 || Qv.y < 0 || Qv.z < 0);
        
        return make_cube_hit(make_float3(fnan), null_face);
    } else {
        assert(!(Qv.x < 0 || Qv.y < 0 || Qv.z < 0));
    }
    
    assert (!(Qv.x < 0));
    assert (!(Qv.y < 0));
    assert (!(Qv.z < 0));

    size_t i = mincomponentf3index(Qv);
    float Q = getcomponentf3(Qv,i);

    assert(!glm_isnan(Q));
    assert(!glm_isinf(Q));

    float3_t t1 = raypos + raydir*Q;

#if !defined(NDEBUG) && !defined(__OPENCL_VERSION__)
    if(!is_on_surface(t1, dir_upper))
    {
        throw std::runtime_error(fmt::format(  "t1 not on dir_upper surface" " raypos: {}, raydir: {}, dir_upper: {}" "{}"
                , tostr(raypos), tostr(raydir), tostr(dir_upper)
                , ( ", Qx: " + tostr(Qv.x) + ", Qy: " + tostr(Qv.y) + ", Qz: " + tostr(Qv.z)
                         + ", t1: " + tostr(t1) + ", Q: " + tostr(Q))));
    }
    
#endif
    ///clamp the result to the inside of the box
    //for (int i = 0; i < 3; ++i)
        //t1[i] = raydir[i] < 0 ? std::max(t1[i], dir_upper[i]) : std::min(t1[i], dir_upper[i]);

    face_t outfaces[3][2] = { {negxface, posxface}, {negyface, posyface}, {negzface, poszface} };
    
    
    face_t outface = outfaces[i][ getcomponentf3(raydir,i) < 0 ? 0 : 1 ];
    setcomponentf3(&t1, i, getcomponentf3(dir_upper, i));
    
    ///TODO bound t1 to the cube.

    /*
    if (i == 0) {
        outface = raydir.x < 0 ? negxface : posxface;
        t1.x = dir_upper.x;
    } else if (i == 1) {
        outface = raydir.y < 0 ? negyface : posyface;
        t1.y = dir_upper.y;
    } else {
        PPK_ASSERT( i == 2
                    , "cannot calculate t1" " raypos: %s, raydir: %s, dir_upper: %s" "%s"
                    , tostr(raypos).c_str(), tostr(raydir).c_str(), tostr(dir_upper).c_str()
                    , ( ", Qx: " + tostr(Qv.x) + ", Qy: " + tostr(Qv.y) + ", Qz: " + tostr(Qv.z)
                         + ", t1: " + tostr(t1) + ", Q: " + tostr(Q)).c_str());

        outface = raydir.z < 0 ? negzface : poszface;
        t1.z = dir_upper.z;
    }
    */
    
    /*
    std::cout
        << "  calculate_t1:"
        << "  raypos: " << raypos
        << ", raydir: " << raydir
        << ", dir_upper: " << dir_upper
        << ", Qi: " << " (" << Qx << "," << Qy << "," << Qz << ")" << ", (" << tohex(Qx) << "," << tohex(Qy) << "," << tohex(Qz) << ")"
        << ", Q: " << Q << ", " << tohex(Q)
        << ", i: " << i

        << std::endl;
    */
    
    assert( !is_null_face(outface) );

#if !defined(NDEBUG) && !defined(__OPENCL_VERSION__)
    if (!fiszerof1(glm_length(glm_normalize(t1 - raypos) - glm_normalize(raydir) )))
    {
        throw std::runtime_error( fmt::format("t1 not in the right direction" " raypos: {}, raydir: {}, dir_upper: {}"
                                            //, ", t1: %s, glm::normalize(t1 - raypos) - glm::normalize(raydir): %s"
                                            , tostr(raypos).c_str(), tostr(raydir).c_str(), tostr(dir_upper).c_str()
                                            //, tostr(t1).c_str(), tostr(glm::normalize(t1 - raypos) - glm::normalize(raydir)).c_str()
        ));
    }
#endif

    return make_cube_hit(t1, outface);
}

static inline
cube_hit_t _calculate_t0_f3( float3_t raypos
                 , float3_t raydir
                 , float3_t dir_lower)
{

    ///its easier to calculate t1, so we reposition the origin of the ray to the opposite side
    /// of the cube, and reflect the raydir, and calculate the t1 from the opposite direction
    float3_t raypos1 = dir_lower + (dir_lower - raypos);
    float3_t raydir1 = -raydir;

#ifdef RAYMARCHDEBUGT0
        std::cout
            << "  calculate_t0: "
            << "  raypos: " << raypos
            << ", raydir: " << raydir
            << ", dir_lower: " << dir_lower
            << ", raypos1: " << raypos1
            << ", raydir1: " << raydir1
            << std::endl;
#endif

    cube_hit_t cube_hit = calculate_t1_f3(raypos1, raydir1, dir_lower);
    float3_t t0 = cube_hit.position;
    face_t inface = cube_hit.face;

    if (is_null_face(inface))
        return make_cube_hit(-t0, inface); 
    return make_cube_hit(-t0, inface);
}

static inline
cube_hit_t calculate_t0_f3( float3_t raypos
                 , float3_t raydir
                 , float3_t dir_lower)
{
#ifdef __cplusplus
    using namespace svo;
#endif
    float3_t Qv;

    Qv = (dir_lower - raypos) / raydir;
    
    ///anything that is nan should be discarded
    Qv = coalescenanf3(Qv,make_float3(fneginf));
    
    ///TODO, what about inf?
    

#ifdef RAYMARCHDEBUGT0
        std::cout
            << "  calculate_t0: "
            << "  raypos: " << raypos
            << ", raydir: " << raydir
            << ", dir_lower: " << dir_lower
            << ", Qx: " << Qx
            << ", Qy: " << Qy
            << ", Qz: " << Qz
            << std::endl;
#endif

    /*
    ///check if the raypos starts on the surface.
    /// If so, we have a choice; we can early terminate here and return raypos.
    if (std::isnan(Qx))
        return std::make_tuple( raypos, raydir.x < 0 ? posxface : negxface );
    else if (std::isnan(Qy))
        return std::make_tuple( raypos, raydir.y < 0 ? posyface : negyface );
    else if (std::isnan(Qz))
        return std::make_tuple( raypos, raydir.z < 0 ? poszface : negzface );
    */
    
    ///TODO: why is this && ?
    //if (Qv.x < 0 && Qv.y < 0 && Qv.z < 0)
    
    if (glm_all(glm_lt(Qv, make_float3(0))))
    {
        assert((Qv.x < 0 && Qv.y < 0 && Qv.z < 0));
        return make_cube_hit(make_float3(fnan), null_face);
    }
    
    assert(!(Qv.x < 0 && Qv.y < 0 && Qv.z < 0));
    
    //assert (!(Qx < 0));
    //assert (!(Qy < 0));
    //assert (!(Qz < 0));


    size_t i = maxcomponentf3index(Qv);
    assert( i < 3 );
    //std::cout << "i: " << i << std::endl;

    float Q = getcomponentf3(Qv, i);

    float3_t t0 = raypos + raydir*Q;

#if !defined(NDEBUG) && !defined(__OPENCL_VERSION__)
    if( !(is_on_surface(t0, dir_lower)) )
    {
        throw std::runtime_error( fmt::format("t0 not on dir_lower surface" " raypos: {}, raydir: {}, dir_lower: {}" "{}"
                , tostr(raypos).c_str(), tostr(raydir).c_str(), tostr(dir_lower).c_str()
                , ( ", Qx: " + tostr(Qv.x) + ", Qy: " + tostr(Qv.y) + ", Qz: " + tostr(Qv.z)
                         + ", t0: " + tostr(t0) + ", Q: " + tostr(Q)).c_str()));
    }
#endif
    ///clamp the result to the inside of the box
    //for (int i = 0; i < 3; ++i)
        //t0[i] = raydir[i] < 0 ? std::min(t0[i], dir_lower[i]) : std::max(t0[i], dir_lower[i]);

    LOCAL_STATIC_CONST face_t infaces[3][2] = { {posxface, negxface}, {posyface, negyface}, {poszface, negzface} };
    face_t inface = infaces[i][ getcomponentf3(raydir, i) < 0 ? 0 : 1 ];
    setcomponentf3(&t0, i, getcomponentf3( dir_lower, i));
    
    /*
    face_t inface = null_face;
    if (i == 0) {
        inface = raydir.x < 0 ? posxface : negxface;
        t0.x = dir_lower.x;
    } else if (i == 1) {
        inface = raydir.y < 0 ? posyface : negyface;
        t0.y = dir_lower.y;
    } else if (i == 2) {
        inface = raydir.z < 0 ? poszface : negzface;
        t0.z = dir_lower.z;
    } else
        PPK_ASSERT( false
                    , "cannot calculate t0" " raypos: %s, raydir: %s, dir_lower: %s" "%s"
                    , tostr(raypos).c_str(), tostr(raydir).c_str(), tostr(dir_lower).c_str()
                    , ( ", Qx: " + tostr(Qx) + ", Qy: " + tostr(Qy) + ", Qz: " + tostr(Qz)
                         + ", t0: " + tostr(t0) + ", Q: " + tostr(Q)).c_str());
    */

    assert( !is_null_face(inface) );


    return make_cube_hit(t0,inface);

}
























typedef struct first_node_t
{
    node_info_t node;
    float3_t lower;
} first_node_t;

static inline first_node_t make_first_node(node_info_t node, float3_t lower)
{
    return (first_node_t){node,lower};
}

static inline
first_node_t svo_select_first_node_f3(goffset_t root_cd_goffset
    , float3_t lower, float scale
    , float3_t raypos, float3_t raydir, float3_t raydirinv)
{

    corner_t best_corner = null_corner;
    float best_distance = (float)(fposinf);
    float3_t best_child_lower = lower;
    for (size_t corneri = 0; corneri < 8; ++corneri)
    {
        corner_t child_corner = all_corners[corneri];
    
        float child_scale = scale / 2;

        float3_t corner_offset = make_float3(  get_corner_unitx(child_corner)
                                            , get_corner_unity(child_corner)
                                            , get_corner_unitz(child_corner) );
        float3_t child_lower = lower + corner_offset*child_scale;
        float3_t child_upper = child_lower+child_scale;

        if (!svo_fast_forward_intersects_f3(child_lower, child_upper, raypos, raydirinv))
            continue;

        ///Point of exit
        //float3_t dir_lower, dir_upper, t1;
        //face_t outface = null_face;

        //std::tie(dir_lower,dir_upper) = calculate_dir_bounds(child_lower,child_lower+child_scale, raydir);
        dir_bounds_t dir_bounds = svo_calculate_dir_bounds_f3(child_lower,child_lower+child_scale, raydir);
        float3_t dir_upper = dir_bounds.upper;
        //std::tie(t1, outface) = calculate_t1_f3(raypos, raydir, dir_upper);
        cube_hit_t cube_hit = calculate_t1_f3(raypos, raydir, dir_upper);
        float3_t t1 = cube_hit.position;
        face_t outface = cube_hit.face;
        
#ifdef RAYMARCHDEBUGSELECTFIRSTNODE
        float3_t dir_lower = dir_bounds.lower
        std::cout
            << "select_first_node.lower: " << lower
            << ", scale: " << scale
            << ", child_lower: " << child_lower
            << ", child_scale: " << child_scale
            << ", dir_lower: " << dir_lower
            << ", dir_upper: " << dir_upper
            << ", t1: " << t1
            << ", outface: " << outface
            << std::endl;
#endif
        if (is_null_face(outface))
            continue;

        float distance = sdistancef3(raypos, t1);
        if (distance < best_distance)
        {
            best_corner = child_corner;
            best_distance = distance;
            best_child_lower = child_lower;
        }
    }


#ifdef RAYMARCHDEBUGSELECTFIRSTNODE
        std::cout
            << "select_first_node.best_corner: " << best_corner
            << ", best_distance: " << best_distance
            << ", best_child_lower: " << best_child_lower
            << std::endl;
#endif
    return make_first_node(make_node_info(root_cd_goffset,best_corner),best_child_lower);
}





















typedef struct located_node_t{
    node_info_t node;
    float3_t lower;
} located_node_t;

static inline located_node_t make_located_node(node_info_t node, float3_t lower)
{
    return (located_node_t){node, lower};
}

static inline
located_node_t
svo_locate_node(svo_stack_t* stack
    , const uint8_t* address_space
    , float3_t root_lower, float root_scale
    , float3_t raypos, float3_t raydir, float3_t raydirinv
    , goffset_t root_cd_goffset
    , int max_depth)
{
#ifdef __cplusplus
    using namespace svo;
#endif
    float3_t pos = raypos;
    float3_t lower = make_float3(0,0,0);
    float3_t upper = make_float3(1,1,1);
    float3_t center = make_float3(.5,.5,.5);

    if (!containsf3(lower,upper, pos))
        return make_located_node( null_node, make_float3(0,0,0));
    float scale = root_scale;
    
    
    scale = scale / 2;
    first_node_t first_node = svo_select_first_node_f3(root_cd_goffset, root_lower, root_scale, raypos, raydir, raydirinv);
    node_info_t current_node = first_node.node;
    lower = first_node.lower;
    upper = lower + make_float3(scale);
    center = (upper + lower) / make_float3(2);
    
    
    //node_descriptor_t parent = root_node;
    //corner_t corner = root_corner;
    int depth = 0;

#if !defined(NDEBUG) && !defined(__OPENCL_VERSION__)
    if(lower != calculate_abs_position_f3(stack,current_node.corner))
        throw std::runtime_error(fmt::format( "lower: {}, upper: {}, calculate_abs_position_f3(stack,current.corner): {}"
                                                ", raypos: {}"
                                                , tostr(lower).c_str(), tostr(upper).c_str()
                                                , tostr(calculate_abs_position_f3(stack,current_node.corner)).c_str()
                                                , tostr(pos).c_str() ));
#endif
    while (max_depth == 0 || depth < max_depth)
    {

#ifdef RAYMARCHDEBUGLOCATENODE
        std::cout << "locate_node.loop, depth: " << depth
            << ", lower: " << lower << ", upper: " << upper << ", center: " << center
            << std::endl
            << "  containsf3(lower,upper, pos): " << (containsf3(lower,upper, pos) ? "true" : "false")
            << std::endl
            << "  current_node.corner: " << current_node.corner
            << std::endl
            << "  node path: " << node_stack_ostr(stack)
            << std::endl;
#endif


#if !defined(NDEBUG) && !defined(__OPENCL_VERSION__)
        if (lower != calculate_abs_position_f3(stack,current_node.corner))
            throw std::runtime_error(fmt::format("lower != calculate_abs_position_f3(stack,current_node.corner)"
                                                 ", lower: {}, upper: {}, calculate_abs_position_f3(stack,current.corner): {}"
                                                 ", raypos: {}"
                    , tostr(lower).c_str(), tostr(upper).c_str()
                    , tostr(calculate_abs_position_f3(stack,current_node.corner)).c_str()
                    , tostr(pos).c_str() ));
#endif

        int xcorneridx = (pos.x < center.x) ? 0 : 1;
        int ycorneridx = (pos.y < center.y) ? 0 : 1;
        int zcorneridx = (pos.z < center.z) ? 0 : 1;

        uint3_t cornerindices = make_uint3(xcorneridx,ycorneridx,zcorneridx);

        
        corner_t child_corner = get_corner_by_int3(xcorneridx, ycorneridx, zcorneridx);
        
        if (!svo_tree_has_children(address_space, current_node))
            break;
        
        node_info_t child = svo_tree_get_child(address_space, current_node, child_corner);

        svo_stack_push(stack, &current_node);
#ifdef RAYMARCHDEBUGRAYMARCHLOWER
        stack_back(stack).lower = lower;
#endif

        depth += 1;
        current_node = child;
        
        //for (int i = 0; i < 3; ++i)
        //    upper[i] = cornerindices[i] ? upper[i] : center[i];
        //for (int i = 0; i < 3; ++i)
        //    lower[i] = cornerindices[i] ? center[i] : lower[i];
        ///upper = cornerindices ? upper : center;
        upper = glm_select(center, upper, cornerindices);
        //lower = cornerindices ? center : lower;
        lower = glm_select(lower, center, cornerindices);

        //for (int i = 0; i < 3; ++i)
        //    center[i] = (lower[i] + upper[i]) / 2;
        center = (lower + upper) / make_float3(2);

    }
    
#if !defined(NDEBUG) && !defined(__OPENCL_VERSION__)
    if (lower != calculate_abs_position_f3(stack,current_node.corner))
        throw std::runtime_error(fmt::format("lower != calculate_abs_position_f3(stack,current_node.corner)"
                                             ", lower: {}, upper: {}, calculate_abs_position_f3(stack,current.corner): {}"
                                             ", raypos: {}"
                , tostr(lower).c_str(), tostr(upper).c_str()
                , tostr(calculate_abs_position_f3(stack,current_node.corner)).c_str()
                , tostr(pos).c_str() ));
#endif
    return make_located_node(current_node,lower);
}








static inline float3_t calculate_abs_position_f3(const svo_stack_t* stack, corner_t current_corner)
{

    float3_t pos = make_float3(0,0,0);

    size_t level = 0;

    for (uint32_t i = 0; i < stack->size; ++i)
    {
        node_info_t node_info = stack->ptr[i];

        if (is_null_corner(node_info.corner))
        {
            level++;
            continue;
        }

        float scale = 1.0/(1<<level);

        pos += make_float3( get_corner_unitx(node_info.corner), get_corner_unity(node_info.corner), get_corner_unitz(node_info.corner))*scale;

        level++;
    }

    if (!is_null_corner(current_corner))
    {
        float scale = 1.0/(1<<level);

        pos += make_float3( get_corner_unitx(current_corner), get_corner_unity(current_corner), get_corner_unitz(current_corner))*scale;
    }

    return pos;
}




static inline
bool svo_voxelpixelerror(float distance2, float rayScale2, uint32_t level){





    ///Given <A as the horizontal FOV. Given width as the number of pixels across the horizontal view.
    ///We want to know the distance a q-sized square has to be in the frustum
    /// to take up less than a pixel.
    ///We project a frustum/ray through the pixel, which has <a = (<A / width) as the FOV.
    ///Let the 2D cross-section of the projection be a triangle with <a on the left, and let it project to infinity.
    ///We place a q-length vertical line segment (representing the square) in the way of the projection,
    /// at a distance where the line segment intersection the projection is exactly length q.
    /// Let BC = q, represent the line segment.
    /// Let D be the midpoint of the line segment.
    /// Let AD be the distance from the origin to the midpoint of the line segment.
    ///We want to calculate AD in terms of q. We then assign q the size of this voxel; if this voxel
    /// at this distance is larger than AD, then this voxel will appear smaller than the FOV projection,
    /// and thus smaller than a pixel.
    ///
    ///Note that <ADB forms a right triangle, with one side being AD, and another side being q/2.
    ///
    ///Using TOA, we get tan(<a/2) = (q/2)/AD. Thus AD = q/(2 tan(<a/2)).
    ///
    ///Now we have AD in terms of q.
    ///
    ///We can determine q as 1/(2^{level}), using the unit cube as
    /// worldspace for the octree.
    ///We can store 1/(2 tan(<a/2)) as the "rayScale" parameters, and determine the appropriate AD for
    /// this voxel size. If the voxel is farther than AD, we can render it directly instead of going deeper
    /// in the hierarchy.


    ///Size of cube
    float q = 1.0 / (1 << level);
    float q2 = q*q;
    float AD2 = (q2*rayScale2);

#ifdef RAYMARCHDEBUGVOXELPIXELERROR
    std::cout << "-"
        << "  voxelpixelerror: "
        << "    distance: " << std::sqrt(distance2)
        << std::endl
        << "    rayScale2: " << rayScale2
        << std::endl
        << "    level: " << level
        << std::endl
        << "    scale2: " << q2
        << std::endl
        << "    AD: " << std::sqrt(AD2)
        << std::endl
        << "    q: " << (1.0 / (1 << level))
        << std::endl
        << "    2*tan(<a/2): " << std::sqrt(rayScale2)
        << std::endl
        << "    1/[2*tan(<a/2)]: " << 1.0/std::sqrt(rayScale2)
        << std::endl
        << "    q/[2*tan(<a/2)]: " << q*std::sqrt(rayScale2)
        << std::endl;
#endif

    return AD2 > distance2;
}

static inline
corner_t svo_push_calculate_next_corner_f3(float3_t cube_normalized_dir, float3_t dir_lower, float scale, float3_t t1)
{
    float3_t dir_center = dir_lower + (cube_normalized_dir*(scale/2));
    corner_t next_corner = get_corner_by_float3( t1.x - dir_center.x, t1.y - dir_center.y, t1.z - dir_center.z );
    return next_corner;
}




static inline
bool svo_tree_raymarch(const uint8_t* address_space, goffset_t root_cd_goffset
    , float3_t raypos, float3_t raydir, float rayScale2
    , float3_t* out_normal
    , float* out_t
#ifdef RAYMARCH_COMPUTE_ITERATIONS
    , uint32_t* out_iterations
#endif
#ifdef RAYMARCH_COMPUTE_LEVELS
    , uint32_t* out_levels
#endif
    )
{
#ifdef __cplusplus
    using namespace svo;
#endif
#ifdef RAYMARCH_COMPUTE_ITERATIONS
        uint32_t iterations = 0;
#endif


    
    
    float3_t raydirinv = make_float3(1.0) / raydir;

#ifdef RAYMARCHDEBUGRAYMARCH
    std::cout
        << "raymarch.raypos: " << raypos
        << ", raymarch.raydir: " << raydir
        << ", raymarch.rayScale2: " << rayScale2
        << std::endl;
#endif



    //corner_t raydir_corner = get_corner_by_float3(raydir.x, raydir.y, raydir.z);

    //std::vector< node_info_t > stack;


    svo_stack_t stack;
    stack.size = 0;
    stack.capacity = MAXIMUM_TREE_DEPTH;


    float root_scale = 1.0;

    float3_t root_lower = make_float3(0,0,0);
    float3_t root_upper = make_float3(1,1,1);

    ///the direction normalized as a vector of 3 integers \in {-1,1}, xyz,
    /// where +1 is above the axis, and -1 is below the axis.
    float3_t cube_normalized_dir = make_float3( raydir.x < 0 ? -1 : 1, raydir.y < 0 ? -1 : 1, raydir.z < 0 ? -1 : 1);


    float minraylength2 = 0;

    //float3_t dir_lower,dir_upper;
    //std::tie(dir_lower,dir_upper) = svo_calculate_dir_bounds_f3(root_lower,root_upper, raydir);


    node_info_t current = tree_null();
    face_t outface = null_face;
    bool rayisoutside = false;
    float3_t t1 = make_float3(0,0,0);
    float3_t lower = make_float3(0,0,0);
#ifdef RAYMARCHDEBUGRAYMARCHLOWER
    current.lower = lower;

    float3_t lastt1 = raypos;
#endif

#ifdef RAYMARCHDEBUGTRACE
    std::vector<float3_t> t1s;
#endif



    if (!svo_fast_forward_intersects_f3(root_lower, root_upper, raypos, raydirinv))
    {
#ifdef RAYMARCHDEBUGRAYMARCH
    std::cout << "!fintersects<float>(root_lower, root_upper, raypos, raydir)" << std::endl;
#endif

        return false;
    }
    if (containsf3(root_lower, root_upper, raypos))
    {
#ifdef RAYMARCHDEBUGRAYMARCH
        std::cout << "origin is *inside* of the root cube" << std::endl;
#endif
        ///the starting position is inside the root node

        located_node_t located_node = svo_locate_node(&stack, address_space, root_lower, root_scale, raypos, raydir, raydirinv, root_cd_goffset, MAXIMUM_TREE_DEPTH);
        //std::tie(current.parent,current.corner,lower) = locate_node(stack, tree, raypos, tree.root(), null_corner);
        current = located_node.node;
        lower = located_node.lower;


        ///this tree had better have at least 8 children from the root!
        assert (current.parent != 0);

        uint32_t level = stack.size;
        float scale = 1.0 / (1 << level);
        //std::tie(dir_lower,dir_upper) = svo_calculate_dir_bounds_f3(lower,lower+scale, raydir);
        dir_bounds_t dir_bounds = svo_calculate_dir_bounds_f3(lower,lower+scale, raydir);
        float3_t dir_lower = dir_bounds.lower, dir_upper = dir_bounds.upper;
        ///Point of exit
        
        cube_hit_t t1_result = calculate_t1_f3(raypos, raydir, dir_upper);
        t1 = t1_result.position;
        outface = t1_result.face;

#if !defined(NDEBUG) && !defined(__OPENCL_VERSION__)
        if( !containsf3(lower,lower+scale, t1) )
            throw std::runtime_error(fmt::format("lower: {}, upper: {}, t1: {}" ", raypos: {}, raydir: {}, dir_upper: {}"
                    , tostr(lower).c_str(), tostr(lower+scale).c_str(), tostr(t1).c_str()

                    , tostr(raypos).c_str(), tostr(raydir).c_str(), tostr(dir_upper).c_str() ));
#endif

#ifdef RAYMARCHDEBUGRAYMARCHLOWER
        current.lower = lower;
#endif

#ifdef RAYMARCHDEBUGRAYMARCH
        std::cout << "starting in root decendant " << node_stack_ostr(stack)
            << ", exiting at face " << outface << " at position " << t1 << std::endl;
#endif
#if !defined(NDEBUG) && !defined(__OPENCL_VERSION__)
        if (lower != calculate_abs_position_f3(&stack,current.corner))
            throw std::runtime_error(fmt::format("lower: {}, calculate_abs_position_f3(stack,current.corner): {}"
                                                 ", raypos: {}, raydir: {}"
                    , tostr(lower).c_str(), tostr(calculate_abs_position_f3(&stack,current.corner)).c_str()
                    , tostr(raypos).c_str(), tostr(raydir).c_str() ));
#endif
    } else {
#ifdef RAYMARCHDEBUGRAYMARCH
        std::cout << "origin is *outside* of the root cube" << std::endl;
        std::cout << "!containsf3(root_lower, root_upper, raypos)" << std::endl;
#endif

        ///the ray is sourced outside the root node.


        svo_stack_push(&stack, &current);

        ///find the child of the root that it intersects with.
        //std::tie(current.corner, lower) = select_first_node(root_lower, root_scale, raypos, raydir, raydirinv);
        //current.parent = tree.root();
        first_node_t first_node = svo_select_first_node_f3(root_cd_goffset, root_lower, root_scale, raypos, raydir, raydirinv);
        current = first_node.node;
        lower = first_node.lower;

#ifdef RAYMARCHDEBUGRAYMARCH
        std::cout
            << "current.corner: " << current.corner
            << ", lower: " << lower
            << std::endl;
#endif
        ///if it doesn't intersect with any children of the root
        if (is_null_corner(current.corner))
        {
#ifdef RAYMARCHDEBUGRAYMARCH
        std::cout << "ray does not intersect with any children of the root (an error? it intersects with the root, but not the children?)"
             << std::endl;
#endif
            ///no hit
            return false;
        }

        uint32_t level = stack.size;
        float scale = 1.0 / (1 << level);

        float3_t upper = lower+scale;


        dir_bounds_t dir_bounds = svo_calculate_dir_bounds_f3(lower,upper, raydir);
        float3_t dir_lower = dir_bounds.lower, dir_upper = dir_bounds.upper;
        //std::tie(dir_lower,dir_upper) = svo_calculate_dir_bounds_f3(lower,upper, raydir);

        face_t inface = null_face;
        ///Point of entry
        cube_hit_t t0_hit = calculate_t0_f3(raypos, raydir, dir_lower);
        t1 = t0_hit.position;
        inface = t0_hit.face;


#if !defined(NDEBUG) && !defined(__OPENCL_VERSION__)
        if( !svo_fast_forward_intersects_f3(lower, upper, raypos, raydirinv))
            throw std::runtime_error(fmt::format("lower: {}, upper: {}" ", raypos: {}, raydir: {}, dir_upper: {}"
                    , tostr(lower).c_str(), tostr(upper).c_str()

                    , tostr(raypos).c_str(), tostr(raydir).c_str(), tostr(dir_upper).c_str() ));
#endif

#ifdef RAYMARCHDEBUGRAYMARCHLOWER
        current.lower = lower;
#endif

#ifdef RAYMARCHDEBUGRAYMARCH
        /*
        std::cout
            << "*child_scale: " << child_scale
            << ", dir_lower: " << dir_lower
            << ", dir_upper: " << dir_upper
            << std::endl
            << "  raypos: " << raypos
            << ", raydir: " << raydir
            << ", t1: " << t1
            << ", inface: " << inface
            << std::endl;
        */
        std::cout << "ray intersects with child of root: " << current.corner
            << ", lower: " << lower
            << ", point of entry: " << t1
            << ", on face " << inface << std::endl;
#endif

#ifdef RAYMARCHASSERTS
        if (!containsf3(lower,upper, t1))
        {
            throw std::runtime_error( fmt::format("lower: {}={}, upper: {}={}, t1: {}={}"
                                            ", raypos: {}={}, raydir: {}={}, dir_upper: {}={}"
                                            , tostr(lower), tohex(lower)
                                            , tostr(upper), tohex(upper)
                                            , tostr(t1), tohex(t1)
                                            , tostr(raypos), tohex(raypos)
                                            , tostr(raydir), tohex(raydir)
                                            , tostr(dir_upper), tohex(dir_upper) ));
        }
        assert(!is_null_face(inface));
#endif
        outface = opposite_face(inface);

    }




    ///<realz> it can go to the neighbor in the same parent
    ///<realz> or it can go deeper into its current node
    ///<realz> or it can go up to the parent to get out of the node

    while (stack.size > 0)
    {
#ifdef RAYMARCHDEBUGTRACE
        t1s.push_back(t1);
#endif
        {
#ifdef RAYMARCHDEBUGRAYMARCH
            std::cout << "-----------" << std::endl;

            uint32_t level = stack.size;
            float scale = 1.0 / (1 << level);

            bool is_a_valid = svo_tree_is_null(current) && !is_null_corner(current.corner);
            bool is_a_leaf = is_a_valid && svo_tree_isleaf_f3(address_space,current);
            bool is_a_voxel = is_a_valid && svo_tree_voxelexists(address_space,current);
            bool is_a_parent = is_a_valid && svo_tree_has_children(address_space, current);
            std::cout << "loop: "
                << "  path: [" << node_stack_ostr(stack) << "]"
                << std::endl
                << "  level: " << stack.size
                << ", current.parent: " << current.parent
                << ", current.corner: " << current.corner
                << ", lower: " << lower
#ifdef RAYMARCHDEBUGRAYMARCHLOWER
                << ", current.lower: " << current.lower
#endif
                << std::endl
                << "  fast_forward_intersects<float>(lower, upper, raypos, raydirinv): "
                    << (svo_fast_forward_intersects_f3(lower, lower+scale, raypos, raydirinv) ? "true" : "false")
                //<< ", level:" << level
                << std::endl
                << "  outface: " << outface
                << ", t1: " << t1
#ifdef RAYMARCHDEBUGRAYMARCHLOWER
                << ", last t1: " << lastt1
#endif
                << std::endl
                << "  raypos: " << raypos
                << ", raydir: " << raydir
                << std::endl
                << "  stack.top().parent: " << svo_stack_back(&stack).parent
                << ", stack.top().corner: " << svo_stack_back(&stack).corner
                << std::endl
                << "  rayisoutside: " << (rayisoutside ? "true" : "false")
                << std::endl
                << "  is_a_valid: " << (is_a_valid ? "true" : "false")
                << std::endl
                << "  svo_tree_isleaf_f3(tree,current_node): " << (!is_a_valid ? "invalid node" : (is_a_leaf ? "true" : "false"))
                << std::endl
                << "  voxelexists(tree,current_node): " << (!is_a_valid ? "invalid node" : (is_a_voxel ? "true" : "false"))
                << std::endl
                << "  is_a_parent: " << (!is_a_valid ? "invalid node" : (is_a_parent ? "true" : "false"))
                << std::endl
                << "  calculate_abs_position_f3(stack): " << calculate_abs_position_f3(&stack,current.corner)
                << std::endl
#ifdef RAYMARCHDEBUGRAYMARCHLOWER
                << "  tdir: " << (t1-lastt1) << ", delta: " << (glm::normalize(raydir) - glm::normalize(t1-lastt1))
                << std::endl
                << "  tdir0: " << (t1-raypos) << ", delta: " << (glm::normalize(raydir) - glm::normalize(t1-raypos))
                << std::endl
#endif
#ifdef RAYMARCHDEBUGTRACE

                << "  t1s: " << t1s_ostr(t1s)
                << std::endl
                << "  dir(t1s): " << dir_t1s_ostr(t1s)
                << std::endl
#endif
                << std::endl;
#endif
#ifdef RAYMARCHDEBUGRAYMARCHLOWER
#if !defined(NDEBUG) && !defined(__OPENCL_VERSION__)
            if(current.lower != lower)
                throw std::runtime_error(fmt::format( "current.lower: {}, lower: {}" ", raypos: {}, raydir: {}"
                        , tostr(current.lower).c_str(), tostr(lower).c_str()
                        , tostr(raypos).c_str(), tostr(raydir).c_str()  ));
#endif

            /*
            PPK_ASSERT(glm::length(lastt1 - t1) == 0 || fiszero<float>(glm::normalize(raydir) - glm::normalize(t1-lastt1), .001)
                        , "raypos: %s, raydir: %s, lastt1 - t1: %s, len(lastt1 - t1): %s, delta: %s, n(delta): %s"
                        , tostr(raypos).c_str(), tostr(raydir).c_str()
                        , tostr(lastt1 - t1).c_str()
                        , tostr(glm::length(lastt1 - t1)).c_str()
                        , tostr(glm::normalize(raydir) - glm::normalize(t1-lastt1)).c_str()
                        , tostr(glm::normalize(glm::normalize(raydir) - glm::normalize(t1-lastt1))).c_str()
                          );
            */
            lastt1 = t1;

#endif
#if !defined(NDEBUG) && !defined(__OPENCL_VERSION__)
            if (lower != calculate_abs_position_f3(&stack,current.corner))
                throw std::runtime_error(fmt::format("lower: {}, calculate_abs_position_f3(stack,current.corner): {}"
                                                     ", raypos: {}, raydir: {}"
                        , tostr(lower).c_str(), tostr(calculate_abs_position_f3(&stack,current.corner)).c_str()
                        , tostr(raypos).c_str(), tostr(raydir).c_str() ));
            assert(current.parent);
#endif
        }
        //assert(!is_null_corner(current.corner));

        uint32_t level = stack.size;

#ifdef RAYMARCH_COMPUTE_ITERATIONS
        iterations++;
#endif

        ///if the corner is invalid, it means we are outside of the parent.
        if (rayisoutside)
        {
            ///POP
#ifdef RAYMARCHDEBUGRAYMARCH
            std::cout << "POP" << std::endl;
#endif
            assert(!is_null_corner(current.corner));
            assert(!is_null_face(outface));




            node_info_t next = svo_stack_back(&stack);
            assert(stack.size > 0);
            svo_stack_pop(&stack);


            {
                float scale = 1.0 / (1 << level);
                float3_t corner_increment0 = make_float3(  get_corner_unitx(current.corner)
                                                        , get_corner_unity(current.corner)
                                                        , get_corner_unitz(current.corner) );
                lower -= corner_increment0*scale;
            }

#ifdef RAYMARCHDEBUGRAYMARCHLOWER
#if !defined(NDEBUG) && !defined(__OPENCL_VERSION__)
            PPK_ASSERT(next.lower == lower, "next.lower: %s, lower: %s", tostr(next.lower).c_str(), tostr(lower).c_str() );
#endif
#endif

            corner_t next_corner0 = next.corner;

            ///we POPed, now we *try to* move to the next corner in the @c outface direction.
            corner_t next_corner1 = move_corner(next_corner0, outface);
#ifdef RAYMARCHDEBUGRAYMARCH
            std::cout << "  parent.corner:" << next.corner << std::endl;
            std::cout << "  next_corner1:" << next_corner1 << std::endl;
#endif

            if (is_null_corner(next_corner1))
            {
                ///we need to POP again
#ifdef RAYMARCHDEBUGRAYMARCH
            std::cout << "    we need to pop again" << std::endl;
#endif

                rayisoutside = true;
                current.parent = next.parent;
                current.corner = next.corner;
#ifdef RAYMARCHDEBUGRAYMARCHLOWER
                current.lower = next.lower;
#endif
            } else {
                ///we POPed, now we move to the next corner in the @c outface direction.
                rayisoutside = false;
                current.parent = next.parent;
                current.corner = next_corner1;

                level = stack.size;
                float scale = 1.0 / (1 << level);

                ///undo the current corner.
                {
                    float3_t corner_increment0 = make_float3(  get_corner_unitx(next_corner0)
                                                            , get_corner_unity(next_corner0)
                                                            , get_corner_unitz(next_corner0) );
                    lower -= corner_increment0*scale;
                }

                ///redo the current corner, moved in the @c outface direction.
                {
                    float3_t corner_increment1 = make_float3(  get_corner_unitx(next_corner1)
                                                            , get_corner_unity(next_corner1)
                                                            , get_corner_unitz(next_corner1) );
                    lower += corner_increment1*scale;
                }
#ifdef RAYMARCHDEBUGRAYMARCHLOWER
                current.lower = lower;
#endif
            }


#ifdef RAYMARCHDEBUGRAYMARCHLOWER
#if !defined(NDEBUG) && !defined(__OPENCL_VERSION__)
            PPK_ASSERT(current.lower == lower, "current.lower: %s, lower: %s", tostr(current.lower).c_str(), tostr(lower).c_str() );
#endif
#endif

#if !defined(NDEBUG) && !defined(__OPENCL_VERSION__)
            if (lower != calculate_abs_position_f3(&stack,current.corner))
                throw std::runtime_error(fmt::format("current.lower: {}, calculate_abs_position_f3(stack,current.corner): {}"
                                                     ", raypos: {}, raydir: {}"
                        , tostr(lower).c_str(), tostr(calculate_abs_position_f3(&stack,current.corner)).c_str()
                        , tostr(raypos).c_str(), tostr(raydir).c_str() ));
#endif

            continue;
        }

        //node_descriptor_t current_node = svo_tree_get_child(address_space, current.parent,current.corner);

        if (svo_tree_voxelexists(address_space,current))
        {

            if (svo_intersects_voxel_data_f3(address_space, current, raypos, raydir))
            {
                float scale = 1.0 / (1 << level);
                dir_bounds_t dir_bounds = svo_calculate_dir_bounds_f3(lower, lower+scale, raydir);
                float3_t dir_lower = dir_bounds.lower;
                //std::tie(dir_lower, std::ignore) = svo_calculate_dir_bounds_f3(lower, lower+scale, raydir);

                {
                    if (!svo_voxelpixelerror(sdistancef3(raypos, dir_lower), rayScale2, level))
                    {
#ifdef RAYMARCHDEBUGRAYMARCH
                        std::cout << "!voxelpixelerror" << std::endl;
#endif
#ifdef RAYMARCH_COMPUTE_ITERATIONS
                        *out_iterations = iterations;
#endif
#ifdef RAYMARCH_COMPUTE_LEVELS
                        *out_levels = level;
#endif
                        *out_t = minraylength2 + sdistancef3(raypos, dir_lower);
                        face_t inface = opposite_face(outface);
                        //float shade = .5;
                        //normal = compressMaterial(Vec3(get_direction_x(inface), get_direction_y(inface), get_direction_z(inface)), shade);

                        *out_normal = glm_normalize(make_float3(get_direction_x(inface), get_direction_y(inface), get_direction_z(inface)));

                        return true;
                    }
                }

                if (svo_tree_isleaf_f3(address_space,current))
                {
#ifdef RAYMARCHDEBUGRAYMARCH
                    std::cout << "svo_tree_isleaf_f3()" << std::endl;
#endif
#ifdef RAYMARCH_COMPUTE_ITERATIONS
                    *out_iterations = iterations;
#endif
#ifdef RAYMARCH_COMPUTE_LEVELS
                    *out_levels = level;
#endif
                    face_t inface = opposite_face(outface);
                    //float shade = .5;
                    *out_normal = glm_normalize(make_float3(get_direction_x(inface), get_direction_y(inface), get_direction_z(inface)));

                    *out_t = minraylength2 + sdistancef3(raypos, dir_lower);
                    return true;
                }

                if (svo_tree_has_children(address_space,current))
                {
                    ///PUSH
                    ///Go deeper in the hierarchy; the node's voxel value intersects with the ray
                    /// and it has children, and could use more resolution.

                    uint32_t next_level = stack.size + 1;
                    float scale = 1.0 / (1 << level);
                    float child_scale = 1.0 / (1 << next_level);

                    svo_stack_push(&stack,&current);


                    ///calculate next corner corner

                    //float3_t dir_lower, dir_upper;
                    //std::tie(dir_lower,dir_upper) = svo_calculate_dir_bounds_f3(lower,lower + scale, raydir);
                    dir_bounds_t dir_bounds = svo_calculate_dir_bounds_f3(lower,lower + scale, raydir);
                    float3_t dir_lower = dir_bounds.lower, dir_upper = dir_bounds.upper;

                    corner_t next_corner = svo_push_calculate_next_corner_f3(cube_normalized_dir, dir_lower,scale, t1);

                    float3_t corner_increment = make_float3( get_corner_unitx(next_corner)
                                                          , get_corner_unity(next_corner)
                                                          , get_corner_unitz(next_corner));
                    float3_t next_lower = lower + (corner_increment * child_scale);

#ifdef RAYMARCHDEBUGRAYMARCH
                    std::cout << "PUSH: "
                        << std::endl
                        << "  current lower: " << lower
                        << std::endl
                        << "  current dir_lower: " << dir_lower << " dir_upper: " << dir_upper
                        << std::endl
                        << "  level: " << level
                        << std::endl
                        << "  scale: " << scale
                        << std::endl
                        << "  next level: " << next_level
                        << std::endl
                        << "  next scale: " << child_scale
                        << std::endl
                        << "  next corner: " << next_corner
                        << std::endl
                        << "  next lower: " << next_lower

                        << std::endl;
#endif

                    
                    current = svo_tree_get_child(address_space, current, next_corner);
                    //current.parent = current_node;
                    //current.corner = next_corner;
                    lower = next_lower;
#ifdef RAYMARCHDEBUGRAYMARCHLOWER
                    current.lower = lower;
#endif

                    continue;
                }

            }

        }

        ///FIXME: is this correct?
        /*
        if (!voxelpixelerror(compute_square_distance(raypos, dir_lower), rayScale, level))
        {
            std::cout << "!voxelpixelerror" << std::endl;
            return false;
        }
        */

        float scale = 1.0 / (1 << level);

        ///Point of exit
        //float3_t dir_lower, dir_upper;
        //std::tie(dir_lower,dir_upper) = svo_calculate_dir_bounds_f3(lower,lower + scale, raydir);
        dir_bounds_t dir_bounds = svo_calculate_dir_bounds_f3(lower,lower + scale, raydir);
        float3_t dir_lower = dir_bounds.lower, dir_upper = dir_bounds.upper;


        //std::tie(t1, outface) = calculate_t1_f3(raypos, raydir, dir_upper);
        cube_hit_t cube_hit = calculate_t1_f3(raypos, raydir, dir_upper);
        t1 = cube_hit.position;
        outface = cube_hit.face;

        assert(!is_null_face(outface));

        ///Corner of the next node, within the parent node; can be null_corner if it goes outside of the parent
        corner_t next_corner = move_corner(current.corner, outface);
        

        ///ADVANCE or POP (if corner is a null_corner, it will pop in the beginning of the loop)

        if (!is_null_corner(next_corner))
        {
            assert(!is_corner_equal(current.corner,next_corner));

            ///ADVANCE
#ifdef RAYMARCHDEBUGRAYMARCH
            std::cout << "ADVANCE" << std::endl;
#endif

            float3_t corner_increment0 = make_float3(  get_corner_unitx(current.corner)
                                                    , get_corner_unity(current.corner)
                                                    , get_corner_unitz(current.corner));
            float3_t corner_increment1 = make_float3(  get_corner_unitx(next_corner)
                                                    , get_corner_unity(next_corner)
                                                    , get_corner_unitz(next_corner));

            float3_t next_lower = lower + ((corner_increment1 - corner_increment0) * scale);

#ifdef RAYMARCHDEBUGRAYMARCH
            std::cout << "lower: " << lower << ", " << tohex(lower) << std::endl;
            std::cout << "upper: " << (lower+scale) << ", " << tohex(lower+scale) << std::endl;
            std::cout << "corner_increment0: " << corner_increment0 << std::endl;
            std::cout << "outface: " << outface << std::endl;
            std::cout << "corner_increment1: " << corner_increment1 << std::endl;
            std::cout << "next_lower: " << next_lower << std::endl;
#endif

            lower = next_lower;
            current.parent = current.parent;
            current.corner = next_corner;
#ifdef RAYMARCHDEBUGRAYMARCHLOWER
            current.lower = lower;
#if !defined(NDEBUG) && !defined(__OPENCL_VERSION__)
            if (current.lower != lower)
                throw std::runtime_error(fmt::format("current.lower: {}, lower: {}"
                                                    , tostr(current.lower).c_str(), tostr(lower).c_str() ));
#endif
#endif
            continue;
        } else {
            ///POP, will complete at the top of the loop
#ifdef RAYMARCHDEBUGRAYMARCH
            std::cout << "POP0" << std::endl;
#endif

            float3_t corner_increment0 = make_float3(  get_corner_unitx(current.corner)
                                                    , get_corner_unity(current.corner)
                                                    , get_corner_unitz(current.corner));

            rayisoutside = true;
#ifdef RAYMARCHDEBUGRAYMARCHLOWER
#if !defined(NDEBUG) && !defined(__OPENCL_VERSION__)
            PPK_ASSERT(current.lower == lower, "current.lower: %s, lower: %s", tostr(current.lower).c_str(), tostr(lower).c_str() );
#endif
#endif
            continue;
        }
    }

    return false;
}


#endif
