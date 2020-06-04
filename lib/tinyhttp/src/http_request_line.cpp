#include "tinyhttp/http_request_line.h"
#include "tinyhttp/constants.h"
#include "tinyhttp/utils.h"
#include "tinyhttp/errors.h"

#include <algorithm>

namespace tinyhttp {

HttpRequestLine::HttpRequestLine(std::string_view request_line) {
    request_line = request_line.substr(0, request_line.find(constants::strings::newline));

    auto space_pos = request_line.find(constants::strings::space);
    if (space_pos == std::string_view::npos
        || space_pos + constants::strings::space.size() == request_line.size()) {
        throw errors::HttpInvalidRequestLine();
    }

    method_ = request_line.substr(0, space_pos);
    request_line.remove_prefix(space_pos + constants::strings::space.size());

    space_pos = request_line.find(constants::strings::space);
    if (space_pos == std::string_view::npos
        || space_pos + constants::strings::space.size() == request_line.size()) {
        throw errors::HttpInvalidRequestLine();
    }

    url_ = utils::decode_url(request_line.substr(0, space_pos));
    request_line.remove_prefix(space_pos + constants::strings::space.size());

    auto question_pos = url_.find(constants::strings::question);
    if (question_pos != std::string::npos
        && question_pos + space_pos + constants::strings::question.size() < url_.size()) {
        std::string_view query_string_view = url_;
        query_string_view = query_string_view.substr(
                question_pos + constants::strings::question.size());

        query_params_ = HttpQueryParameters(query_string_view);

        url_ = url_.substr(0, question_pos);
    }

    auto slash_pos = request_line.find(constants::strings::slash);
    if (slash_pos == std::string_view::npos
        || slash_pos + constants::strings::slash.size() == request_line.size()) {
        throw errors::HttpInvalidRequestLine();
    }

    version_ = request_line.substr(slash_pos + constants::strings::slash.size());
}

const std::string &HttpRequestLine::get_version() const {
    return version_;
}

const std::string &HttpRequestLine::get_method() const {
    return method_;
}

const std::string &HttpRequestLine::get_url() const {
    return url_;
}

const HttpQueryParameters &HttpRequestLine::get_query_string() const {
    return query_params_;
}

std::string HttpRequestLine::to_string() const {
    std::string buf;

    buf += method_;

    buf += constants::strings::space;

    buf += url_;

    if (!query_params_.empty()) {
        buf += constants::strings::question;
        buf += query_params_.to_string();
    }

    buf += constants::strings::space;

    buf += constants::strings::http_upper;
    buf += constants::strings::slash;
    buf += version_;

    buf += constants::strings::newline;

    return buf;
}

}
