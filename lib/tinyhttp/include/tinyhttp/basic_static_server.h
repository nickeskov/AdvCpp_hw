#ifndef TINYHTTP_TINYHTTP_STATIC_SERVER_H
#define TINYHTTP_TINYHTTP_STATIC_SERVER_H

#include "tinyhttp/server.h"

#include <string>
#include <string_view>

namespace tinyhttp {

class BasicStaticServer final : public Server {
  public:

    BasicStaticServer(std::string_view ip, uint16_t port, trivilog::BaseLogger &logger,
                      std::string_view document_root);

    HttpResponse on_request(const HttpRequest &request) override;

    ~BasicStaticServer() override = default;

  private:
    std::string document_root_;

};

}


#endif //TINYHTTP_TINYHTTP_STATIC_SERVER_H
