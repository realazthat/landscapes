
#include <iomanip>
#include <algorithm>


#include <cassert>

inline
std::ostream& operator<<(std::ostream& out, const glm::vec3& v)
{
    out << std::setprecision(std::numeric_limits<float>::digits10+1) << "(" << v.x << "," << v.y << "," << v.z << ")";
    return out;
}


inline
std::ostream& operator<<(std::ostream& out, const glm::uvec3& v)
{
    out << std::setprecision(std::numeric_limits<float>::digits10+1) << "(" << v.x << "," << v.y << "," << v.z << ")";
    return out;
}


inline
std::ostream& operator<<(std::ostream& out, const glm::ivec3& v)
{
    out << "(" << v.x << "," << v.y << "," << v.z << ")";
    return out;
}









namespace svo{

///hax because operator<<(ostream&, glm::vec3) doesn't seem to be catching it
inline
std::ostream& outputvec3(std::ostream& out, const glm::vec3& v)
{
    out << std::setprecision(std::numeric_limits<float>::digits10+1) << "(" << v.x << "," << v.y << "," << v.z << ")";
    return out;
}
template<typename T>
inline std::string tostr(T v)
{
    std::ostringstream out;
    out.flush() << v;
    return out.str();
}


template<>
inline std::string tostr<glm::vec3>(glm::vec3 v)
{
    std::ostringstream out;
    outputvec3(out.flush(), v);
    return out.str();
}



inline std::string quote(const std::string& in)
{
    std::size_t count = std::count(in.begin(), in.end(), '"');

    std::vector<char> out;
    out.reserve(count+2);
    out.push_back('"');

    for (auto c : in)
    {
        if (c == '"')
            out.push_back('\\');

        out.push_back(c);
    }

    out.push_back('"');

    return std::string(out.data(), out.size());
}




constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

inline unsigned char _hextonibble(char c)
{
    if (c >= '0' && c <= '9')
        return (unsigned char)(c - '0');
    else if (c >= 'a' && c <= 'f')
        return (unsigned char)(c - 'a') + 10;
    else if (c >= 'A' && c <= 'F')
        return (unsigned char)(c - 'A') + 10;
    else
        assert(false);
    return 0;
}

inline unsigned char hextonibble(char c)
{
    unsigned char nibble = _hextonibble(c);
    assert(nibble < 16);
                //, "c: %s, (int)nibble: %s"
                //, tostr(c).c_str(), tostr((int)nibble).c_str());
    assert(hexmap[nibble] == c);
                //, "c: %s, (int)nibble: %s"
                //, tostr(c).c_str(), tostr((int)nibble).c_str());
    return nibble;
}
inline std::string tohex(const unsigned char *data, int len)
{
    assert(sizeof(unsigned char) == 1);

    ///http://codereview.stackexchange.com/a/78539/16444
    std::string s(len * 2, ' ');
    for (int i = 0; i < len; ++i) {
        unsigned char lownibble = (data[i] & 0x0F);
        unsigned char hinibble = (data[i] & 0xF0) >> 4;

        char h0 = hexmap[lownibble];
        char h1 = hexmap[hinibble];
        s[2 * i]     = h0;
        s[2 * i + 1] = h1;
        //std::cout << (int)((data[i]))  << ",";
        //std::cout << s[2*i] << s[2*i+1];
        //std::cout << (int)((data[i] & 0xF0) >> 4) << "," << (int)(data[i] & 0x0F) << ",";
        //std::cout << (unsigned char)(data[i]) << "," << (unsigned char)(data[i]) << ",";
        //assert(((data[i] & 0xF0) >> 4) == fromhex(s[2 * i]));
        //assert((data[i] & 0x0F) == fromhex(s[2 * i + 1]));
    }
    //std::cout << std::endl;


    //std::cout << "nibbles: ";
    for (int i = 0; i < len; ++i) {
    //    std::cout << (int)hextonibble(s[2*i]) << "," << (int)hextonibble(s[2*i+1]) << ",";
    }

    //std::cout << std::endl;
    return s;
}

inline std::string tohex(const float v)
{
    return tohex(reinterpret_cast<const unsigned char*>(&v), sizeof(v));
}

template<typename float_t>
inline float_t fromhex(const std::string& hex)
{
    assert(hex.size() % 2 == 0);
    assert(hex.size() / 2 == sizeof(float_t));

    float v = 0;
    unsigned char* vp = reinterpret_cast<unsigned char*>(&v);

    //std::cout << "bytes: ";
    for (std::size_t i = 0; i < hex.size() / 2; ++i)
    {
        unsigned char byte = 0;
        auto c0 = (hex[2*i]);
        auto c1 = (hex[2*i+1]);
        auto n0 = hextonibble(c0);
        auto n1 = hextonibble(c1);
        assert(sizeof(char) == 1);

        assert( n0 < 16 );
        assert( n1 < 16 );
        assert( c0 == hexmap[(int)n0] );
        assert( c1 == hexmap[(int)n1] );

        byte = (n0) | (n1 << 4);
        *vp++ = byte;
        //std::cout << (int)v0 << "," << (int)v1 << ",";
        //std::cout << (int)c0 << "," << (int)c1 << ",";
        //std::cout << c0 << "," << c1 << ",";
        //std::cout << (int)n0 << "," << (int)n1 << ",";
        //std::cout << (int)byte << ",";
    }

    //std::cout << std::endl;
    return v;
}

inline std::string tohex(const glm::vec3 v)
{
    std::ostringstream out;
    out << "(" << tohex(v.x) << "," << tohex(v.y) << "," << tohex(v.z) << ")";
    return out.str();
}


} // namespace svo




















