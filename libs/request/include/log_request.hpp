#pragma once

#include <ostream>
#include <request.hpp>

std::ostream& operator<<(std::ostream& out, const HttpRequest& request);
std::string methodToString(HttpMethod method);
