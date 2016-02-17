#ifndef __NX_REQUEST_H__
#define __NX_REQUEST_H__

#include <string>
#include <ostream>
#include <sstream>
#include <memory>
#include <type_traits>

#include <nx/picohttpparser.h>

#include <nx/config.h>
#include <nx/buffer.hpp>
#include <nx/methods.hpp>
#include <nx/headers.hpp>
#include <nx/attributes.hpp>
#include <nx/uri.hpp>
#include <nx/json.hpp>
#include <nx/file.hpp>
#include <nx/socket_base.hpp>

namespace nx {

class NX_API request
{
public:
    request();
    request(const nx::method& m);
    request(const std::string& method);
    request(const std::string& method, const std::string& path);
    request(const request& other) = delete;
    request(request&& other);
    virtual ~request();

    request& operator=(request&& other);

    bool parse(buffer& b);

    const std::string& method() const;
    const std::string& path() const;

    std::size_t content_length() const;

    std::string& h(const std::string& name);
    const std::string& h(const std::string& name) const;
    bool has(const header& h) const;
    bool has(const std::string& name) const;

    std::string& a(const std::string& name);
    const std::string& a(const std::string& name) const;

    bool operator==(const nx::method& m) const;
    bool operator!=(const nx::method& m) const;

    request& operator/(const std::string& path);

    request& operator<<(const nx::method& m);
    request& operator<<(const header& h);
    request& operator<<(const headers& h);
    request& operator<<(const attribute& a);
    request& operator<<(const attributes& a);
    request& operator<<(const json& js);
    request& operator<<(const jsonv::value& v);
    request& operator<<(const file& f);

    template <typename T>
    request& operator<<(const T& v)
    {
        data_ << v;

        return *this;
    }

    bool is_form() const;

private:
    using raw_headers_ptr = std::unique_ptr<phr_header[]>;
    static const std::size_t max_headers = 128;

    raw_headers_ptr raw_headers_ptr_;
    headers headers_;
    std::string method_;
    std::string path_;
    std::size_t content_length_;
    attributes attrs_;
    std::ostringstream data_;
    std::string empty_;

    const char *raw_method_;
    std::size_t raw_method_len_;
    const char *raw_path_;
    std::size_t raw_path_len_;
    int minor_version_;
    std::size_t num_headers_;
    std::size_t prev_buf_len_ = 0;
};

template <
    typename Socket,
    typename = std::enable_if_t<
        std::is_base_of<socket_base, Socket>::value
    >
>
Socket&
operator<<(Socket& s, )


} // namespace nx

#endif // __NX_REQUEST_H__
