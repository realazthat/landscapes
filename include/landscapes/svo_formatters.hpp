#ifndef SVO_FORMATTERS_HPP
#define SVO_FORMATTERS_HPP 1

#include "glm/glm.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include "landscapes/svo_tree.fwd.hpp"
#include "landscapes/svo_buffer.fwd.hpp"
#include "landscapes/svo_camera_mapping.cl.fwd.h"

#include "cubelib/formatters.hpp"

struct direction_t;
struct corner_t;
struct child_descriptor_t;
//struct svo_block_t;

std::ostream& operator<<(std::ostream& out, const glm::vec3& v);
std::ostream& operator<<(std::ostream& out, const glm::uvec3& v);
std::ostream& operator<<(std::ostream& out, const glm::ivec3& v);
std::ostream& operator<<(std::ostream& out, const glm::vec2& v);
std::ostream& operator<<(std::ostream& out, const glm::uvec2& v);
std::ostream& operator<<(std::ostream& out, const glm::ivec2& v);

std::ostream& operator<<(std::ostream& out, const child_descriptor_t& cd);

std::ostream& operator<<(std::ostream& out, const svo::svo_semantic_t& semantic);
std::ostream& operator<<(std::ostream& out, const svo::svo_data_type_t& data_type);
std::ostream& operator<<(std::ostream& out, const std::vector<svo::svo_element_t>& elements);
std::ostream& operator<<(std::ostream& out, const svo::svo_element_t& element);
std::ostream& operator<<(std::ostream& out, const svo::svo_declaration_t& declaration);
std::ostream& operator<<(std::ostream& out, const std::vector<svo::svo_declaration_t>& declarations);

std::ostream& operator<<(std::ostream& out, const svo_camera_mapping_t& camera_mapping);

std::ostream& operator<<(std::ostream& out, const svo_camera_quad_t& camera_quad);




namespace svo{

    
::std::ostream& operator<<(::std::ostream& out, const svo_error_t& error);
    
template<typename T>
std::string tostr(T v);

inline unsigned char hextonibble(char c);
inline std::string tohex(const unsigned char *data, int len);
inline std::string tohex(const float v);

template<typename float_t> inline float_t fromhex(const std::string& hex);
inline std::string tohex(const glm::vec3 v);
inline std::string tohex(const glm::vec3 v);

void pprint_announce_msg(std::ostream& out, const std::string& announce_msg, std::size_t width);
void pprint_block(std::ostream& out, const std::string& announce_msg, const svo_block_t* block);




inline std::string quote(const std::string& in);

















namespace detail{
    template<typename T>
    struct stl_node_stack_container_t{
        stl_node_stack_container_t(const T& stack) : stack(stack){}
        const T& stack;
    };
    template<typename T>
    struct node_stack_container_t{
        node_stack_container_t(const T& stack) : stack(stack){}
        const T& stack;
    };
}

template<typename T>
inline
std::ostream& operator<<(std::ostream& out, const detail::stl_node_stack_container_t<T>& node_stack_container)
{

    bool first = true;
    for (const auto& node_level : node_stack_container.stack)
    {
        if (!first)
            out << ".";
        else
            first = false;
        out << node_level.corner;
    }

    return out;
}

template<typename T>
inline
std::ostream& operator<<(std::ostream& out, const detail::node_stack_container_t<T>& node_stack_container)
{

    bool first = true;
    for (uint32_t i = 0; i < node_stack_container.stack.size; ++i)
    {
        const auto& node_level = node_stack_container.stack.ptr[i];

        if (!first)
            out << ".";
        else
            first = false;
        out << node_level.corner;
    }

    return out;
}

template<typename T>
inline
detail::node_stack_container_t<T> stl_node_stack_ostr(const T& stack)
{
    return detail::stl_node_stack_container_t<T>(stack);
}

template<typename T>
inline
detail::node_stack_container_t<T> node_stack_ostr(const T& stack)
{
    return detail::node_stack_container_t<T>(stack);
}




} //namespace svo



#include "landscapes/svo_formatters.inl.hpp"


#endif
