#ifndef __NX_JSON_H__
#define __NX_JSON_H__

#include <ostream>
#include <unordered_map>
#include <typeinfo>

#include <jsonv/parse.hpp>
#include <jsonv/serialization.hpp>
#include <jsonv/serialization_builder.hpp>

#include <nx/config.h>
#include <nx/buffer.hpp>

namespace nx {

class NX_API json_types
{
public:
    static json_types& get();

    template <typename T>
    void add(const jsonv::formats& fmt)
    {
        auto type_name = typeid(T).name();
        types_.emplace(type_name, fmt);
    }

    template <typename T>
    jsonv::value to_json(const T& t) const
    { return jsonv::to_json(t, get_formats<T>()); }

    template <typename T>
    void from_json(T& t, const jsonv::value& v) const
    { t = jsonv::extract<T>(v, get_formats<T>()); }

private:
    using type_format_map = std::unordered_map<std::string, jsonv::formats>;

    json_types();
    ~json_types();

    json_types(const json_types&) = delete;
    void operator=(const json_types&) = delete;

    template <typename T>
    jsonv::formats get_formats() const
    {
        auto type_name = typeid(T).name();
        auto it = types_.find(type_name);

        if (it == types_.end()) {
            throw std::logic_error("unknown type");
        }

        auto fmt = jsonv::formats::compose(
            { it->second, jsonv::formats::defaults() }
        );

        return fmt;
    }

    type_format_map types_;
};

template <typename T>
inline
void
add_json_format(const jsonv::formats& fmt)
{ json_types::get().add<T>(fmt); }

class NX_API json
{
public:
    json();
    json(buffer& b);

    template <typename T>
    json(const T& t)
    { v_ = json_types::get().to_json(t); }

    template <typename T>
    void operator>>(T& t) const
    { json_types::get().from_json(t, v_); }

    void operator()(std::ostream& os) const;

private:
    jsonv::value v_;
};

inline
std::ostream&
operator<<(std::ostream& os, const json& j)
{
    j(os);
    return os;
}

} // namespace nx

#endif // __NX_JSON_H__
