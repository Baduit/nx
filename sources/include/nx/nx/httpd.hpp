#ifndef __NX_HTTPD_H__
#define __NX_HTTPD_H__

#include <string>

#include <nx/config.h>
#include <nx/http.hpp>
#include <nx/methods.hpp>
#include <nx/route.hpp>

namespace nx {

class NX_API httpd
{
public:
    route& operator()(const method& m);

    const endpoint& operator()(const endpoint& ep);

private:
    void operator()(request& req, buffer& data, reply& rep);

    http s_;
    routes_map routes_map_;
};

} // namespace nx

#endif // __NX_HTTPD_H__