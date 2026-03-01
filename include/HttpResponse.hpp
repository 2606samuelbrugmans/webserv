/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:46:48 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 12:47:14 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <sys/types.h>  //ssize_t

#include "Http.hpp"
#include "HttpPacket.hpp"

class HttpResponse : public HttpPacket {
   private:
    Http::Status::Code      status_code_;
    Http::ContentType::Type content_type_;
    std::string             location_;
    std::string serialized_;

   public:
    HttpResponse();
    HttpResponse(Http::Status::Code status_code, const std::string& body);
    HttpResponse(Http::Status::Code status_code, const std::string& body,
                 Http::ContentType::Type content_type);
    HttpResponse(const HttpResponse& other);
    HttpResponse& operator=(const HttpResponse& other);
    ~HttpResponse(){};

    virtual std::string startLine() const;
    void                prepare();

    static ssize_t sendResponse(int client_fd, HttpResponse& res);

    /* Getters/Setters */

    Http::ContentType::Type getContentType() const { return content_type_; }
    const std::string&      getLocation() const { return location_; }
    Http::Status::Code      getStatusCode() const { return status_code_; }
    const std::string&      getSerialized() const { return serialized_; }

    void setContentType(Http::ContentType::Type type) { content_type_ = type; }
    void setLocation(const std::string& location) { location_ = location; }
    void setStatusCode(Http::Status::Code code) { status_code_ = code; }
    void setSerialized(const std::string& serialized) { serialized_ = serialized; }
};

#endif  // HTTPRESPONSE_HPP
