#define BOOST_TEST_MODULE json_collection

#include <iostream>

#include <nx/unit_test.hpp>

#include <nx/nx.hpp>
#include <nx/utils.hpp>

#include "../json/person.hpp"

// Configure a deadline timer for each test
const std::size_t test_count = 4;

nx::timer deadline;
nx::cond_var cv(test_count);

BOOST_AUTO_TEST_CASE(httpd_json_collection)
{
    using namespace nx;

    deadline(10.0) = [&](nx::timer& t, int events) {
        t.stop();
        cv.notify_all();
    };

    deadline.start();

    using collection_type = json_collection<test::person>;
    using persons = collection_type::values_type;

    // Register test::person type
    add_json_format<test::person>(test::person_fmt);
    add_json_format<persons>(collection_type::format());

    collection_type coll("persons");

    httpd hd;

    // Register collection in httpd
    hd << coll;

    // Start server
    auto sep = hd(endpoint("127.0.0.1"));

    httpc hc;

    bool item_not_found = false;

    hc(GET, sep) / "persons/1234" = [&](const reply& rep, buffer& data) {
        item_not_found = (rep == NotFound);

        cv.notify();
    };

    bool got_collection = false;
    bool empty_collection = false;

    hc(GET, sep) / "persons" = [&](const reply& rep, buffer& data) {
        got_collection = (rep == OK);
        persons p;

        json(data) >> p;

        empty_collection = p.empty();

        cv.notify();
    };

    bool item_created = false;
    bool item_has_id = false;
    bool item_with_id_found = false;

    hc(POST, sep)
        / "persons"
        << json(test::person{ 42, "Bart Simpson", 15 })
        = [&](const reply& rep, buffer& data) {
            item_created = (rep == Created);

            auto parts = split("/", rep.h(location));

            if (!parts.empty()) {
                item_has_id = true;

                hc(GET, sep) / "persons" / parts.back() =
                    [&](const reply& rep, buffer& data) {
                        item_with_id_found = (rep == OK);
                        test::person p;

                        json(data) >> p;

                        cv.notify();
                    };
                }

                cv.notify();
        };

    cv.wait();
    nx::stop();

    BOOST_CHECK_MESSAGE(
        item_not_found,
        "non-existent item was not found"
    );
    BOOST_CHECK_MESSAGE(
        got_collection,
        "got a collection"
    );
    BOOST_CHECK_MESSAGE(
        empty_collection,
        "intial collection is empty"
    );
    BOOST_CHECK_MESSAGE(
        item_created,
        "a new item was created"
    );
    BOOST_CHECK_MESSAGE(
        item_has_id,
        "new item has an id"
    );
    BOOST_CHECK_MESSAGE(
        item_with_id_found,
        "new item with id was found"
    );
}
