#include <nx/json_collection.hpp>

namespace nx {

json_collection_base::json_collection_base(const std::string& path)
: path_(clean_path(path))
{}

json_collection_base::~json_collection_base()
{}

const std::string&
json_collection_base::path() const
{ return path_; }

json_collection::json_collection(const std::string& path)
: json_collection_base(path),
id_(0)
{}

json_collection::values_type&
json_collection::get()
{ return c_; }

const json_collection::values_type&
json_collection::get() const
{ return c_; }

route_cb
json_collection::GET(const collection_tag& t)
{
    return [&](const request& req, buffer& data, reply& rep) {
        rep << "[";

        auto it = c_.begin();

        if (it != c_.end()) {
            rep << it->second.as_string();
        }

        while (++it != c_.end()) {
            rep << "," << it->second.as_string();
        }

        rep << "]";
    };
}

route_cb
json_collection::PUT(const collection_tag& t)
{
    return [&](const request& req, buffer& data, reply& rep) {
        if (data.empty()) {
            throw BadRequest;
        }

        c_.clear();

        json arr(data);

        for (auto& v : arr.value().as_array()) {
            auto it = v.find("id");

            if (it == v.end_object()) {
                continue;
            }

            auto id = nx::to_num<id_type>(
                jsonv::extract<std::string>(it->second)
            );

            c_[id] = v;
        }
    };
}

route_cb
json_collection::POST(const collection_tag& t)
{
    return [&](const request& req, buffer& data, reply& rep) {
        if (data.empty()) {
            throw BadRequest;
        }

        auto id = next_id_(id_);
        json item(data);

        item.value()["id"] = id;
        c_.emplace(id, item.value());

        rep
            << Created
            << header{ location, join("/", path(), id) }
            ;
    };
}

route_cb
json_collection::DELETE(const collection_tag& t)
{
    return [&](const request& req, buffer& data, reply& rep) {
        c_.clear();
    };
}

route_cb
json_collection::GET(const item_tag& t)
{
    return [&](const request& req, buffer& data, reply& rep) {
        auto id = nx::to_num<id_type>(req.a("id"));
        auto it = c_.find(id);

        if (it != c_.end()) {
            rep << it->second.as_string();
        } else {
            rep << NotFound;
        }
    };
}

route_cb
json_collection::PUT(const item_tag& t)
{
    return [&](const request& req, buffer& data, reply& rep) {
        auto id = nx::to_num<id_type>(req.a("id"));

        json item(data);

        item.value()["id"] = id;
        c_[id] = item.value();
    };
}

route_cb
json_collection::DELETE(const item_tag& t)
{
    return [&](const request& req, buffer& data, reply& rep) {
        auto id = nx::to_num<id_type>(req.a("id"));

        c_.erase(id);
    };
}

} // namespace nx
