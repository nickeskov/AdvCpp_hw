#include "tinyhttp/constants.h"

#include <unordered_map>

namespace tinyhttp::constants {

namespace {

const std::unordered_map<http_response_status, std::string_view> http_response_status_text = {
        {http_response_status::Continue,                      "Continue"},
        {http_response_status::SwitchingProtocols,            "Switching Protocols"},
        {http_response_status::Processing,                    "Processing"},
        {http_response_status::EarlyHints,                    "Early Hints"},

        {http_response_status::OK,                            "OK"},
        {http_response_status::Created,                       "Created"},
        {http_response_status::Accepted,                      "Accepted"},
        {http_response_status::NonAuthoritativeInfo,          "Non-Authoritative Information"},
        {http_response_status::NoContent,                     "No Content"},
        {http_response_status::ResetContent,                  "Reset Content"},
        {http_response_status::PartialContent,                "Partial Content"},
        {http_response_status::MultiStatus,                   "Multi-Status"},
        {http_response_status::AlreadyReported,               "Already Reported"},
        {http_response_status::IMUsed,                        "IM Used"},

        {http_response_status::MultipleChoices,               "Multiple Choices"},
        {http_response_status::MovedPermanently,              "Moved Permanently"},
        {http_response_status::Found,                         "Found"},
        {http_response_status::SeeOther,                      "See Other"},
        {http_response_status::NotModified,                   "Not Modified"},
        {http_response_status::UseProxy,                      "Use Proxy"},
        {http_response_status::TemporaryRedirect,             "Temporary Redirect"},
        {http_response_status::PermanentRedirect,             "Permanent Redirect"},

        {http_response_status::BadRequest,                    "Bad Request"},
        {http_response_status::Unauthorized,                  "Unauthorized"},
        {http_response_status::PaymentRequired,               "Payment Required"},
        {http_response_status::Forbidden,                     "Forbidden"},
        {http_response_status::NotFound,                      "Not Found"},
        {http_response_status::MethodNotAllowed,              "Method Not Allowed"},
        {http_response_status::NotAcceptable,                 "Not Acceptable"},
        {http_response_status::ProxyAuthRequired,             "Proxy Authentication Required"},
        {http_response_status::RequestTimeout,                "Request Timeout"},
        {http_response_status::Conflict,                      "Conflict"},
        {http_response_status::Gone,                          "Gone"},
        {http_response_status::LengthRequired,                "Length Required"},
        {http_response_status::PreconditionFailed,            "Precondition Failed"},
        {http_response_status::RequestEntityTooLarge,         "Request Entity Too Large"},
        {http_response_status::RequestURITooLong,             "Request URI Too Long"},
        {http_response_status::UnsupportedMediaType,          "Unsupported Media Type"},
        {http_response_status::RequestedRangeNotSatisfiable,  "Requested Range Not Satisfiable"},
        {http_response_status::ExpectationFailed,             "Expectation Failed"},
        {http_response_status::Teapot,                        "I'm a teapot"},
        {http_response_status::MisdirectedRequest,            "Misdirected Request"},
        {http_response_status::UnprocessableEntity,           "Unprocessable Entity"},
        {http_response_status::Locked,                        "Locked"},
        {http_response_status::FailedDependency,              "Failed Dependency"},
        {http_response_status::TooEarly,                      "Too Early"},
        {http_response_status::UpgradeRequired,               "Upgrade Required"},
        {http_response_status::PreconditionRequired,          "Precondition Required"},
        {http_response_status::TooManyRequests,               "Too Many Requests"},
        {http_response_status::RequestHeaderFieldsTooLarge,   "Request Header Fields Too Large"},
        {http_response_status::UnavailableForLegalReasons,    "Unavailable For Legal Reasons"},

        {http_response_status::InternalServerError,           "Internal Server Error"},
        {http_response_status::NotImplemented,                "Not Implemented"},
        {http_response_status::BadGateway,                    "Bad Gateway"},
        {http_response_status::ServiceUnavailable,            "Service Unavailable"},
        {http_response_status::GatewayTimeout,                "Gateway Timeout"},
        {http_response_status::HTTPVersionNotSupported,       "HTTP Version Not Supported"},
        {http_response_status::VariantAlsoNegotiates,         "Variant Also Negotiates"},
        {http_response_status::InsufficientStorage,           "Insufficient Storage"},
        {http_response_status::LoopDetected,                  "Loop Detected"},
        {http_response_status::NotExtended,                   "Not Extended"},
        {http_response_status::NetworkAuthenticationRequired, "Network Authentication Required"},
};

const std::unordered_map<http_method, std::string_view> http_method_text = {
        {http_method::GET,     "GET"},
        {http_method::HEAD,    "HEAD"},
        {http_method::POST,    "POST"},
        {http_method::PUT,     "PUT"},
        {http_method::DELETE,  "DELETE"},
        {http_method::CONNECT, "CONNECT"},
        {http_method::OPTIONS, "OPTIONS"},
        {http_method::TRACE,   "TRACE"},
};

const std::unordered_map<std::string_view, http_method> http_method_code = {
        {"GET",     http_method::GET},
        {"HEAD",    http_method::HEAD,},
        {"POST",    http_method::POST},
        {"PUT",     http_method::PUT},
        {"DELETE",  http_method::DELETE},
        {"CONNECT", http_method::CONNECT},
        {"OPTIONS", http_method::OPTIONS},
        {"TRACE",   http_method::TRACE},
};

constexpr std::string_view http_version_V0_9_text = "0.9";
constexpr std::string_view http_version_V1_0_text = "1.0";
constexpr std::string_view http_version_V1_1_text = "1.1";

}

std::string_view get_http_response_status_text(http_response_status status) {
    return http_response_status_text.at(status);
}

std::string_view get_http_version_text(http_version version) noexcept {
    std::string_view version_text;

    switch (version) {
        case http_version::V1_1: {
            version_text = http_version_V1_1_text;
            break;
        }
        case http_version::V1_0: {
            version_text = http_version_V1_0_text;
            break;
        }
        case http_version::V0_9: {
            version_text = http_version_V0_9_text;
            break;
        }
        case http_version::UNSUPPORTED_: // fallthrough
        default: {
            break;
        }
    }

    return version_text;
}

http_version get_http_version_code(std::string_view version_text) noexcept {
    http_version version = http_version::UNSUPPORTED_;

    if (version_text == http_version_V1_1_text) {
        version = http_version::V1_1;
    } else if (version_text == http_version_V1_0_text) {
        version = http_version::V1_0;
    } else if (version_text == http_version_V0_9_text) {
        version = http_version::V0_9;
    }

    return version;
}

std::string_view get_http_method_text(http_method method) {
    return http_method_text.at(method);
}

http_method get_http_method_code(std::string_view method_text) noexcept {
    http_method method = http_method::UNSUPPORTED_;

    if (http_method_code.count(method_text) > 0) {
        method = http_method_code.at(method_text);
    }

    return method;
}

}
