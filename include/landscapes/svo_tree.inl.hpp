#ifndef SVO_TREE_INL_HPP
#define SVO_TREE_INL_HPP 1

#include "debug_macro.h"


//#include "prettyprint.hpp"

namespace svo{



/**
 * See http://cs.stackexchange.com/q/49319/2755
 */
template<typename metadata_t, typename visitor_f>
inline void z_preorder_traverse_block_cds0(const byte_t* address_space, const svo_block_t* block, metadata_t metadata0, visitor_f visitor)
{
    assert(address_space);
    assert(block);

    if (!(block->root_valid_bit))
        return;


    if (block->root_leaf_bit)
    {
        assert(block->root_ccurve < 8);
        visitor(invalid_goffset /*current_parent_goffset*/, invalid_goffset /*current_goffset*/, block->root_ccurve, metadata0);
        return;
    }

    typedef std::vector< std::tuple<goffset_t, goffset_t, ccurve_t, metadata_t> > stack_t;
    stack_t stack
    {
        std::make_tuple(invalid_goffset, block->root_shadow_cd_goffset, block->root_ccurve, metadata0)
    };

    while (stack.size() > 0)
    {
        goffset_t current_parent_goffset; goffset_t current_goffset; ccurve_t current_ccurve; metadata_t current_metadata;
        std::tie(current_parent_goffset, current_goffset, current_ccurve, current_metadata) = stack.back();stack.pop_back();

        assert(current_ccurve < 8);


        metadata_t next_metadata = visitor(current_parent_goffset, current_goffset, current_ccurve, current_metadata);

        if (current_goffset == invalid_goffset)
            continue;
        
        assert(current_goffset);
        assert(current_goffset != invalid_goffset);
        assert(block->is_in_block(current_goffset));

        const child_descriptor_t* cd = svo_cget_cd(address_space, current_goffset);
        assert(cd);

        auto valid_mask = svo_get_valid_mask(cd);
        auto leaf_mask = svo_get_leaf_mask(cd);

        auto child_base_goffset = svo_get_child_ptr_goffset(address_space, current_goffset, cd);

        
        ///if the child ptr is 0
        if (child_base_goffset == current_goffset)
        {
            /// then we expect no nonleaf children, and nothing more to visit here.
            assert(svo_get_cd_nonleaf_count(cd) == 0);
        }



        //if (!svo_is_goffset_in_block(block,child_base_goffset))
        //    continue;


        ///we are going to collect all the children first
        stack_t sibling_stack;

        auto current_child_goffset = child_base_goffset;

        for (ccurve_t child_ccurve = 0; child_ccurve < 8; ++child_ccurve)
        {
            assert(child_ccurve < 8);

            bool valid_bit = (valid_mask >> child_ccurve) & 1;
            bool leaf_bit = (leaf_mask >> child_ccurve) & 1;

            if (valid_bit)
            {
                if (!leaf_bit)
                {
                    assert(block->is_in_block(current_child_goffset));

                    sibling_stack.push_back( std::make_tuple(current_goffset, current_child_goffset, child_ccurve, next_metadata) );
                    current_child_goffset += sizeof(child_descriptor_t);
                } else {
                    ///add a leaf voxel; current cd will be the parent, and no cd is specified.
                    sibling_stack.push_back( std::make_tuple(current_goffset, invalid_goffset, child_ccurve, next_metadata) );
                }
            }

        }

        ///add the siblings to the stack, in reverse order, because we want them to be popped
        /// off the stack in the correct order.
        stack.insert(stack.end(),    sibling_stack.rbegin(), sibling_stack.rend() );
    }

}


/**
 * See http://cs.stackexchange.com/q/49319/2755
 * See https://en.wikipedia.org/wiki/Tree_traversal#Pre-order_2
 * See https://gist.github.com/realazthat/cf17d44d94133b129e56#file-z_preorder_traverse-exp-py
 *      see z_preorder_traverse2()
 * See https://repl.it/B1dq/37
 *
 * This implementation of z-preorder traversal is an iterative pre-order,
 * modified to {visit the children of a node} instead of the normal {visit(node)}.
 * This ensures that the children are visited in a group at a time.
 */
template<typename metadata_t, typename visitor_f>
inline void z_preorder_traverse_block_cds(const byte_t* address_space, const svo_block_t* block
                                        , metadata_t metadata0, visitor_f visitor, bool debug)
{
    assert(address_space);
    assert(block);
    assert(block->tree);


    if (!(block->root_valid_bit)) {
        return;
    }

    assert(block->root_shadow_cd_goffset != invalid_goffset);
    assert(block->is_valid_cd_goffset(block->root_shadow_cd_goffset));

    assert(block->root_ccurve < 8);


    auto root_meta_data = visitor(  invalid_goffset /*current_parent_goffset*/
                                  , block->root_shadow_cd_goffset /*current_goffset*/
                                  , block->root_ccurve
                                  , metadata0);
    
    ///no children, nothing else to visit.
    if (block->root_leaf_bit) {
        const auto* root_shadow_cd = svo_cget_cd(block->tree->address_space, block->root_shadow_cd_goffset);

        assert(svo_get_valid_mask(root_shadow_cd) == 0);
        assert(svo_get_leaf_mask(root_shadow_cd) == 0);

        return;
    }

    ///dunno how we got here if the root is not a leaf it should have children, but
    /// apparently it does not have a CD, and therefore has no children.
    if (block->root_shadow_cd_goffset == invalid_goffset)
        return;

    typedef std::tuple<goffset_t, goffset_t, ccurve_t, metadata_t> node_t;
    typedef std::vector< node_t > stack_t;

    ///put the root node into the parent stack, we already visited it.
    stack_t parent_stack{ std::make_tuple(invalid_goffset, block->root_shadow_cd_goffset, block->root_ccurve, root_meta_data) };

    node_t node;
    bool node_valid = false;


    while (parent_stack.size() > 0 || node_valid)
    {
        if (node_valid)
        {
            auto cd_goffset = std::get<1>(node);
            auto metadata = std::get<3>(node);

            SCAFFOLDING{
                if (debug)
                    std::cout << "cd_goffset: " << cd_goffset << std::endl;
            }

            assert(cd_goffset != 0 && cd_goffset != invalid_goffset);
            assert(block->is_valid_cd_goffset(cd_goffset));
            const auto* cd = svo_get_cd(block->tree->address_space, cd_goffset);
            
            
            assert((((~svo_get_valid_mask(cd)) & svo_get_leaf_mask(cd)) == 0));
            
            if (svo_get_cd_valid_count(cd) == 0) {
                node_valid = false;

                SCAFFOLDING{
                    if (debug)
                        std::cout << "nope, no children, moving on" << std::endl;
                }
                
                continue;
            } else {

                SCAFFOLDING{
                    if (debug)
                        std::cout << "ok going through the children" << std::endl;
                }
                
                stack_t sibling_stack;
                for (ccurve_t child_ccurve = 0; child_ccurve < 8; ++child_ccurve)
                {
                    assert(child_ccurve < 8);

                    bool valid_bit = svo_get_valid_bit(cd, child_ccurve);
                    bool leaf_bit = svo_get_leaf_bit(cd, child_ccurve);

                    SCAFFOLDING{
                        if (debug)
                            std::cout << "valid_bit: " << (valid_bit ? "true" : "false")
                                      << ", leaf_bit: " << (leaf_bit ? "true" : "false") << std::endl;
                    }
                    if (valid_bit)
                    {
                        if (!leaf_bit)
                        {
                            goffset_t child_cd_goffset = svo_get_child_cd_goffset(block->tree->address_space, cd_goffset, cd, child_ccurve);
                            assert(child_cd_goffset != 0 && child_cd_goffset != invalid_goffset);

                            if (!(block->is_valid_cd_goffset(child_cd_goffset)))
                            {
                                continue;
                            }

                            auto child_meta_data = visitor(cd_goffset, child_cd_goffset, child_ccurve, metadata);
                            sibling_stack.push_back( std::make_tuple(cd_goffset, child_cd_goffset, child_ccurve, child_meta_data) );
                        } else {
                            ///add a leaf voxel; current cd will be the parent, and no cd is specified.

                            auto child_meta_data = visitor(cd_goffset, invalid_goffset, child_ccurve, metadata);
                            UNUSED(child_meta_data);
                            //sibling_stack.push_back( std::make_tuple(cd_goffset, invalid_goffset, child_ccurve, child_meta_data) );
                        }
                    }

                }

                ///put the children in the stack in the right order.
                parent_stack.insert(parent_stack.end(), sibling_stack.rbegin(), sibling_stack.rend());
                node_valid = false;
            }
        } else {
            node = parent_stack.back(); parent_stack.pop_back();
            node_valid = true;
        }
    }

}

template<typename metadata_t, typename visitor_f>
inline void preorder_traverse_block_cds(const byte_t* address_space, const svo_block_t* block, metadata_t metadata0, visitor_f visitor)
{
    return z_preorder_traverse_block_cds(address_space, block, metadata0, visitor);
}


template<typename MetaDataT, typename VisitorF>
inline void levelorder_traverse_block_cds(byte_t* address_space, svo_block_t* block, MetaDataT metadata0, VisitorF visitor)
{

    ///visitor(null,root,null)
    ///
    ///parent_stack = [(root, 0)]
    ///
    ///
    ///current_level = 1
    ///
    ///while stack:
    ///
    ///  parent,ccurve = parent_stack.pop()
    ///
    ///  if len(parent_stack) == current_level:
    ///    visitor(parent,ccurve)
    ///
    ///    ccurve1 = parent.next_child_curve(ccurve)
    ///    if ccurve1 is not None:
    ///      parent_stack += [ (parent, ccurve1) ]
    ///    else: dir = "up"
    ///    continue
    ///  else:
    ///    assert len(parent_stack) < current_level
    ///    
    ///    if dir == "up":
    ///      ccurve1 = parent.next_child_curve(ccurve)
    ///      if ccurve1 is not None:
    ///        parent_stack += [ (parent, ccurve1) ]
    ///        dir = "down"
    ///    else:

    ///  assert False and "TODO"





    assert(false && "TODO");
}



template<typename svo_block_type, typename metadata_t, typename visitor_f>
inline void preorder_traverse_blocks(svo_block_type* block0, metadata_t metadata0, visitor_f visitor)
{

    {
        std::vector< std::tuple<svo_block_type*, svo_block_type*, metadata_t> > stack{ std::make_tuple(nullptr, block0, metadata0) };
        while (stack.size() > 0)
        {
            svo_block_type* parent_block;
            svo_block_type* current_block;
            metadata_t current_metadata;
            std::tie(parent_block, current_block, current_metadata) = stack.back();stack.pop_back();
            assert(current_block);

            metadata_t next_metadata = visitor(parent_block, current_block, current_metadata);

            assert(current_block->child_blocks);

            auto wend = current_block->child_blocks->crend();
            //for ( svo_slice_t* child : *current_slice->children )
            for (auto w = current_block->child_blocks->rbegin(); w != wend; ++w)
            {
                svo_block_type* child = *w;

                assert(child);
                assert(child->parent_block == current_block);

                stack.push_back( std::make_tuple(parent_block, child, next_metadata) );
            }


        }
    }
}



template<typename svo_slice_type, typename metadata_t, typename visitor_f>
inline void preorder_traverse_slices(svo_slice_type* root_slice, metadata_t metadata0, visitor_f visitor)
{

    {
        std::vector< std::tuple<svo_slice_type*, metadata_t> > stack{ std::make_tuple(root_slice, metadata0) };
        while (stack.size() > 0)
        {
            svo_slice_type* current_slice; metadata_t current_metadata;
            std::tie(current_slice, current_metadata) = stack.back();stack.pop_back();
            assert(current_slice);

            metadata_t next_metadata = visitor(current_slice, current_metadata);

            assert(current_slice->children);

            auto wend = current_slice->children->crend();
            //for ( svo_slice_type* child : *current_slice->children )
            for (auto w = current_slice->children->rbegin(); w != wend; ++w)
            {
                svo_slice_type* child = *w;

                assert(child);
                assert(child->parent_slice == current_slice);

                stack.push_back( std::make_tuple(child, next_metadata) );
            }


        }
    }
}


template<typename svo_slice_type, typename visitor_f>
inline void preorder_traverse_slices(svo_slice_type* root_slice, visitor_f visitor)
{
    auto visitor_internal = [visitor](svo_slice_type* slice, const int& noop)
    {
        visitor(slice);
        return noop;
    };

    preorder_traverse_slices(root_slice, 0, visitor_internal);
}

template<typename PostVisitorF>
inline void postorder_traverse_slices_experimental0(svo_slice_t* root_slice, PostVisitorF postorder_visitor)
{

    ///current node, current parent, current sibling index.
    std::vector< std::tuple<svo_slice_t*, svo_slice_t*, std::size_t> > stack = { std::make_tuple(root_slice, (svo_slice_t*)NULL, 0) };

    enum class traversal_dir_t{
          UP
        , DOWN
    };

    traversal_dir_t traversal_dir = traversal_dir_t::DOWN;

    std::size_t current_sibling_index = 0;

    while (stack.size() > 0)
    {
        svo_slice_t* current_node; svo_slice_t* parent_node; std::size_t current_sibling_index;
        std::tie(current_node, parent_node, current_sibling_index) = stack.back();
        stack.pop_back();

        assert(current_node);
        assert(current_node->children);
        ///if the parent node is null, it means we are at the root.
        assert(!!parent_node || stack.size() == 0);

        if (traversal_dir == traversal_dir_t::DOWN)
        {
            if (current_node->children->size() == 0)
            {
                ///going down, but have no children, time to visit this current_node; go UP, and UP routine will
                /// visit this current node.
                traversal_dir = traversal_dir_t::UP;
                continue;

            } else {
                ///going down, have children. Visit first one.

                ///push the current node back onto the stack.
                stack.push_back(std::make_tuple(current_node, parent_node, current_sibling_index));
                ///push the first child onto the stack.
                stack.push_back(std::make_tuple((*current_node->children)[0], current_node, 0));
                traversal_dir = traversal_dir_t::DOWN;
                continue;
            }
        } else {
            ///traversal dir is up.

            ///going up, that means we finished visiting all the children, now we can visit this node.
            postorder_visitor(current_node);


            ///next, go right.
            if (!parent_node)
            {
                ///we are the root node; there is no "right". We are done.
                break;
            } else {
                ///there are no nodes to the right.
                if (!(current_sibling_index + 1 < parent_node->children->size()))
                {
                    traversal_dir = traversal_dir_t::UP;
                    continue;
                }

                ///there are nodes to the right.
                ///go to the next node.
                ++current_sibling_index;
                ///set the current node to the one to the right.
                current_node = (*parent_node->children)[current_sibling_index];
                ///push the new current node onto the stack.
                stack.push_back( std::make_tuple(current_node, parent_node, current_sibling_index) );
                traversal_dir = traversal_dir_t::DOWN;
                continue;
            }
        }

    }
}

template<typename svo_slice_type, typename visitor_f, typename metadata_t>
inline metadata_t
postorder_traverse_slices_recursive(svo_slice_type* node, metadata_t metadata0, visitor_f visitor)
{
    if (!node)
        return std::vector<metadata_t>();


    assert(node);
    assert(node->children);
    auto& children = *node->children;


    std::vector<metadata_t> metadatas;
    for (svo_slice_type* child : children)
    {
        auto metadata = postorder_traverse_slices_recursive(child, metadata0, visitor);
        metadatas.push_back(metadata);
    }

    auto metadata = visitor(node, metadatas);

    return metadata;
}

/**
 * See https://gist.github.com/realazthat/cf17d44d94133b129e56#file-postorder_traverse-exp-py
 * See https://repl.it/B908/15
 *
 */
template<typename metadata_t, typename svo_slice_type, typename visitor_f>
inline metadata_t
postorder_traverse_slices(svo_slice_type* node0, visitor_f visitor)
{
    assert(node0);
    
    typedef std::vector< svo_slice_type* > stack_t;
    stack_t stack = { node0 };
    
    ///{node => completed metadata}
    std::map<svo_slice_type*, metadata_t> finished_node_metadata;
    
    ///{node => completed child count}
    std::map<svo_slice_type*, std::size_t> node_children_completed;
    while (stack.size())
    {
        svo_slice_type* node = stack.back(); stack.pop_back();

        assert(node);
        assert(node->children);
        assert(finished_node_metadata.count(node) == 0);
        
        auto& children = *node->children;
        
        if (children.size() == 0)
        {
            assert(node_children_completed.count(node) == 0);
            
            metadata_t result = visitor(node, std::vector<metadata_t>());
            finished_node_metadata[node] = result;
            node_children_completed[node->parent_slice] += 1;
            assert(node_children_completed.count(node) == 0);
            continue;
        }
        
        assert(children.size() > 0);
        
        ///this node has not been visited before
        if (node_children_completed.count(node) == 0)
        {
            
            node_children_completed[node] = 0;
            
            ///put the node back on the stack.
            stack.push_back(node);
            
            
            ///put the children on the stack
            {
                stack_t siblings;
                for (svo_slice_type* child : children)
                {
                    siblings.push_back(child);
                }
                
                ///put them on in reverse order; so that they are processed in order.
                stack.insert(stack.end(), siblings.rbegin(), siblings.rend());
            }
            
            continue;
        }
        
        assert(children.size() > 0);
        assert(node_children_completed.count(node) > 0);
        assert(node_children_completed[node] == children.size());
        
        std::vector<metadata_t> metadatas;
        ///collect children's metadata
        /// and also erase them from the map.
        for (svo_slice_type* child : children)
        {
            assert(node_children_completed.count(child) == 0);
            assert(finished_node_metadata.count(child) == 1);
            
            auto w = finished_node_metadata.find(child);
            assert(w != finished_node_metadata.end());
            
            const auto& child_metadata = w->second;
            metadatas.push_back(child_metadata);
            finished_node_metadata.erase(w);
        }
        
        ///viit this node.
        metadata_t result = visitor(node, metadatas);
        
        ///remember the metadata result
        finished_node_metadata[node] = result;
        
        ///let the parent know this child completed.
        if (node->parent_slice)
            node_children_completed[node->parent_slice] += 1;
        
        ///stop tracking the children's visit status.
        {
            auto w = node_children_completed.find(node);
            assert(w != node_children_completed.end());
            node_children_completed.erase(w);
        }
        assert(node_children_completed.count(node) == 0);
        
        
    }
    
    
    assert(node0);
    assert(node0->children);
    assert(node_children_completed.size() == 0);
    assert(finished_node_metadata.size() == 1);
    assert(finished_node_metadata.count(node0) == 1);
    
    metadata_t result = finished_node_metadata[node0];
    return result;
}


inline std::size_t print_cd_tree(svo_tree_t* tree, goffset_t pcd_goffset, goffset_t cd_goffset, ccurve_t cd_ccurve, std::size_t level)
{
    std::string symbol = "**";
    std::string indent = "  ";
    assert(pcd_goffset != 0);
    assert(cd_goffset != 0);
    if (pcd_goffset == invalid_goffset)
    {
        assert(level == 0);
        symbol = ">*";
    } else if (cd_goffset == invalid_goffset) {
        symbol = "*@";
    }

    for (std::size_t i = 0; i < level; ++i)
    {
        std::cout << indent;
    }
    std::cout << symbol;

    if (cd_goffset != invalid_goffset)
    {
        auto* cd = svo_get_cd(tree->address_space, cd_goffset);

        std::cout
            << " level: " << level
            << ", corner: " << ccurve2corner(cd_ccurve)
            << ", ccurve: " << cd_ccurve
            << ", valid_mask: " << std::bitset<8>(svo_get_valid_mask(cd))
            << ", leaf_mask: " << std::bitset<8>(svo_get_leaf_mask(cd))
            << std::endl;
    } else {
        std::cout
            << " level: " << level
            << ", corner: " << ccurve2corner(cd_ccurve)
            << ", ccurve: " << cd_ccurve
            << std::endl;
    }

    return level + 1;
}



} //namespace svo

#endif
