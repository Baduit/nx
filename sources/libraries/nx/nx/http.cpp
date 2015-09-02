#include <nx/http.hpp>

namespace nx {

bool
http::request_parsed()
{
    if (!parsed_) {
        parsed_ = req_.parse(rbuf());
    }

    return parsed_;
}

bool
http::reply_parsed()
{
    if (!parsed_) {
        parsed_ = rep_.parse(rbuf());
    }

    return parsed_;
}

void
http::process_request()
{
    try {
        if (!request_parsed() || rbuf().size() < req_.content_length()) {
            // Wait until request is complete
            return;
        }

        if (req_.is_form()) {
            // Decode additional variables from body
            // TODO: implement iterator based function to avoid copy
            std::string body;
            rbuf() >> body;
            req_ << attributes(body, '&');
        }

        // All data arrived, call upper handler
        request_cb_(req_, rbuf(), rep_);
    } catch (const http_status& s) {
        rep_ << s;
    } catch (const std::exception& e) {
        std::cout << "BadRequest by " << e.what() << std::endl;
        rep_ << BadRequest(e);
    }

    rep_.on_done() = [this]() {
        *this << rep_.content();
        close_after_write();
    };

    if (!rep_.postponed()) {
        rep_.done();
    }
}

void
http::process_reply()
{
    try {
        if (!reply_parsed() || rbuf().size() < rep_.content_length()) {
            // Wait until response is complete
            return;
        }
    } catch (const http_status& s) {
        rep_ << s;
    } catch (const std::exception& e) {
        rep_ << BadResponse(e);
    }

    // All data arrived, call upper handler
    reply_cb_(rep_, rbuf());
    close();
}

void
http::send_request()
{
    req_ << header("Host", local_str());
    *this << req_.content();
}

http&
http::operator<<(request_cb cb)
{
    request_cb_ = std::move(cb);

    return *this;
}

http&
http::operator<<(request req)
{
    req_ = std::move(req);

    return *this;
}

http&
http::operator<<(reply_cb cb)
{
    reply_cb_ = std::move(cb);

    return *this;
}

http&
http::operator<<(reply rep)
{
    rep_ = std::move(rep);

    return *this;
}

} // namespace nx

