#include "tinyhttp/basic_static_server.h"
#include "coroutine/coroutine.h"
#include "unixprimwrap/descriptor.h"

#include <filesystem>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <cstring>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
}

namespace tinyhttp {

using namespace std::literals::string_literals;

namespace fs = std::filesystem;

namespace {

const std::unordered_map<std::string_view, std::string_view> mime_type_by_extension = {
        {".html", "text/html"},
        {".css",  "text/css"},
        {".js",   "application/javascript"},
        {".jpg",  "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".png",  "image/png"},
        {".gif",  "image/gif"},
        {".swf",  "application/x-shockwave-flash"},
};

constexpr size_t MAX_WRITE_BYTES_PER_CALL = 16384;
constexpr std::string_view mime_octet_stream = "application/octet-stream";

inline std::string_view get_mime_type(std::string_view extension) {
    if (mime_type_by_extension.count(extension) > 0) {
        return mime_type_by_extension.at(extension);
    }
    return mime_octet_stream;
}


}

BasicStaticServer::BasicStaticServer(std::string_view ip, uint16_t port, trivilog::BaseLogger &logger,
                                     std::string_view document_root) : Server(
        ip, port, logger) {


    fs::path document_path = fs::canonical(document_root); // nickeskov: if no such directory throw exception
    document_root_ = document_path;

    if (!fs::is_directory(document_path)) {
        throw std::runtime_error("document_root=" + document_root_ + " is not a directory");
    }

}

HttpResponse BasicStaticServer::on_request(const HttpRequest &request) {
    auto http_version = request.get_request_line().get_version();
    auto method = request.get_request_line().get_method();

    if (method != constants::http_method::HEAD
        && method != constants::http_method::GET) {
        return HttpResponse(constants::http_response_status::MethodNotAllowed, http_version);
    }

    fs::path file_path = document_root_ + request.get_request_line().get_url();

    std::error_code error_code;

    if (!fs::exists(file_path, error_code)) { // nickeskov: if exists file
        if (error_code && error_code.value() == EACCES) {
            return HttpResponse(constants::http_response_status::Forbidden, http_version);
        }
        return HttpResponse(constants::http_response_status::NotFound, http_version);
    }

    file_path = fs::canonical(file_path);

    if (file_path.native().find(document_root_) != 0) { // nickeskov: check if file not in document root
        return HttpResponse(constants::http_response_status::BadRequest, http_version);
    }

    if (fs::is_directory(file_path)) { // nickeskov: display index file
        file_path = file_path / "index.html";
        if (!fs::exists(file_path, error_code)) { // nickeskov: check if index file exists
            if (error_code) {
                return HttpResponse(constants::http_response_status::InternalServerError, http_version);
            }
            return HttpResponse(constants::http_response_status::Forbidden, http_version);
        }
    }

    if (!fs::exists(file_path, error_code)) { // nickeskov: check if index file exists
        if (error_code && error_code.value() == EACCES) {
            return HttpResponse(constants::http_response_status::Forbidden, http_version);
        }
        return HttpResponse(constants::http_response_status::NotFound, http_version);
    }

    auto content_length = fs::file_size(file_path);
    auto mime_type = get_mime_type(file_path.extension().string());

    HttpResponse response(constants::http_response_status::OK, http_version);

    auto &headers = response.get_headers();

    headers.emplace(constants::headers::content_length, std::to_string(content_length));
    headers.emplace(constants::headers::content_type, mime_type);

    if (method == constants::http_method::HEAD) {
        return response; // nickeskov: if head request response must have empty body
    }

    auto sender = [file_path, content_length](Connection &connection, HttpResponse &http_response) {
        auto fd_file = unixprimwrap::Descriptor(open(file_path.c_str(), O_RDONLY));
        if (!fd_file.is_valid()) {
            http_response.reset_headers();
            http_response.get_response_line().set_response_status(
                    constants::http_response_status::InternalServerError);
        }

        connection.get_io_buffer().clear();
        connection.get_io_buffer() += http_response.to_string();

        auto write_size = connection.get_io_buffer().size();
        for (size_t written = 0; written < write_size;) {
            auto bytes = connection.write_from_io_buff(MAX_WRITE_BYTES_PER_CALL);

            if (bytes < 0) {
                coroutine::yield();
                continue;
            }

            written += bytes;

            coroutine::yield(); // nickeskov: using level triggered mode
        }

        if (fd_file.is_valid()) {
            write_size = content_length;
            for (size_t written = 0; written < write_size;) {
                // nickeskov: send maximum of possible bytes per call
                auto bytes = sendfile(connection.get_io_service().data(), fd_file.data(),
                                      nullptr, MAX_WRITE_BYTES_PER_CALL);

                if (bytes < 0) {
                    if (errno != EAGAIN) {
                        throw std::runtime_error("sendfile error: "s + (std::strerror(errno)));
                    }
                    coroutine::yield();
                    continue;
                }

                written += bytes;

                coroutine::yield(); // nickeskov: using level triggered mode
            }
        }
    };

    response.set_sender(std::move(sender));

    return response;
}

}
