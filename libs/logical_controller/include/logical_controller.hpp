#pragma once

#include "request.hpp"

class LogicalController
{
public:
    using HttpResponse = std::string;

    [[nodiscard]] HttpResponse process(HttpRequest request);
};
