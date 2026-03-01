#include "webserv.hpp"




Http::Method::Type stringToMethod(const std::string& m)
{
    if (m == "GET") return Http::Method::GET;
    if (m == "POST") return Http::Method::POST;
    if (m == "DELETE") return Http::Method::DELETE;
    return Http::Method::UNKNOWN;
}

HttpRequest buildRequest(const Client& client)
{
    HttpRequest req;

    req.setMethod(stringToMethod(client.request_line.method));
    req.setPath(client.request_line.url);
    req.setVersion(client.request_line.version);  // inherited from HttpPacket


    for (std::map<std::string, std::string>::const_iterator it = client.headers.begin();
         it != client.headers.end(); ++it)
    {
        req.setHeader(it->first, it->second);
    }


    if (!client.body.empty())
    {
        std::vector<unsigned char> b(client.body.begin(), client.body.end());
        req.setBody(b);
    }

    return req;
}