/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:50:21 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 12:50:30 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

#include <unistd.h>

#include <sstream>

#include "Http.hpp"

HttpResponse::HttpResponse()
    : HttpPacket(),
      status_code_(Http::Status::OK),
      content_type_(Http::ContentType::OCTET_STREAM),
      location_("") {}

HttpResponse::HttpResponse(Http::Status::Code status_code, const std::string& body)
    : HttpPacket(),
      status_code_(status_code),
      content_type_(Http::ContentType::OCTET_STREAM),
      location_("") {
    setBody(body);
}

HttpResponse::HttpResponse(Http::Status::Code status_code, const std::string& body,
                           Http::ContentType::Type content_type)
    : HttpPacket(), status_code_(status_code), content_type_(content_type), location_("") {
    setBody(body);
}

HttpResponse::HttpResponse(const HttpResponse& other)
    : HttpPacket(other),
      status_code_(other.status_code_),
      content_type_(other.content_type_),
      location_(other.location_) {}

HttpResponse& HttpResponse::operator=(const HttpResponse& other) {
    if (this != &other) {
        HttpPacket::operator=(other);

        status_code_  = other.status_code_;
        content_type_ = other.content_type_;
        location_     = other.location_;
    }
    return *this;
}

std::string HttpResponse::startLine() const {
    std::ostringstream oss;
    oss << version_ << " " << static_cast<int>(status_code_) << " "
        << Http::reasonPhrase(status_code_);
    return oss.str();
}
/*
// Prepares headers before sending the response
static void prepareHeaders(HttpResponse& res) {
    if (res.getStatusCode() == Http::Status::MovedPermanently) {
        res.setHeader("Connection", "close");
        res.setHeader("Content-Length", "0");
        res.setHeader("Location", res.getLocation());
        return;
    }
    std::stringstream body_size;
    body_size << res.getBody().size();

    res.setHeader("Connection", "close");
    res.setHeader("Content-Type", Http::contentTypeToString(res.getContentType()));
    res.setHeader("Content-Length", body_size.str());
}
*/
void HttpResponse::prepare()
{
    std::ostringstream ss;

    //  STATUS LINE
    ss << version_
       << " "
       << status_code_
       << " "
       << Http::reasonPhrase(status_code_)
       << "\r\n";

    //  REQUIRED HEADERS
    ss << "Content-Length: " << body_.size() << "\r\n";

    if (!hasHeader("Content-Type"))
        ss << "Content-Type: " << Http::contentTypeToString(content_type_) << "\r\n";
    else
        ss << "Content-Type: " << getHeader("Content-Type") << "\r\n";

    ss << "Connection: keep-alive\r\n";

    //  WRITE OTHER HEADERS
    for (std::map<std::string,std::string>::iterator it = headers_.begin();
         it != headers_.end(); ++it)
    {
        if (it->first != "Content-Type" && it->first != "Content-Length" && it->first != "Connection")
            ss << it->first << ": " << it->second << "\r\n";
    }

    //  HEADER TERMINATOR 
    ss << "\r\n";

    // convert header stream into string
    serialized_ = ss.str();

    // APPEND BODY BYTES (binary safe)
    serialized_.append(body_.begin(), body_.end());
}
/*
ssize_t HttpResponse::sendResponse(int client_fd, HttpResponse& res) {
    prepareHeaders(res);
    const std::string fullPacket = res.fullPacket();

    ssize_t      totalSent = 0;
    const size_t totalSize = fullPacket.size();
    const char*  data      = fullPacket.c_str();

    while (totalSent < static_cast<ssize_t>(totalSize)) {
        ssize_t sent = ::write(client_fd, data + totalSent, totalSize - totalSent);
        if (sent <= 0) return -1;
        totalSent += sent;
    }

    return totalSent;
}
*/