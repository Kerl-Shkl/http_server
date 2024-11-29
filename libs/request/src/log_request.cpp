#include "log_request.hpp"

std::string methodToString(HttpMethod method)
{
    switch (method) {
    case HttpMethod::GET:
        return "GET";
    case HttpMethod::HEAD:
        return "HEAD";
    case HttpMethod::POST:
        return "POST";
    case HttpMethod::PUT:
        return "PUT";
    case HttpMethod::DELETE:
        return "DELETE";
    case HttpMethod::CONNECT:
        return "CONNECT";
    case HttpMethod::OPTIONS:
        return "OPTIONS";
    case HttpMethod::TRACE:
        return "TRACE";
    case HttpMethod::PATCH:
        return "PATCH";
    }
    return "INCORRECT";
}

std::ostream& operator<<(std::ostream& out, const HttpRequest& request)
{
    out << methodToString(request.getMethod()) << " " << request.getTarget() << " " << request.getProtocol()
        << "\n";
    out << "parameters: ";
    for (const auto& [name, value] : request.getParameters()) {
        out << name << " = " << value << "; ";
    }
    out << "\n";

    out << "queries: ";
    for (const auto& [name, value] : request.getQueries()) {
        out << name << " = " << value << "; ";
    }
    out << "\n";

    out << "headers: ";
    for (const auto& [name, value] : request.getHeaders()) {
        out << name << " = " << value << "; ";
    }
    out << "\n";

    out << "body: " << request.getBody();

    return out;
}
