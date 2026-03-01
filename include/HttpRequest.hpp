/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:46:10 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 12:46:25 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "Http.hpp"
#include "HttpPacket.hpp"

class HttpRequest : public HttpPacket {
   private:
    Http::Method::Type method_;
    std::string        path_;

   public:
    HttpRequest();
    HttpRequest(const HttpRequest& other);
    HttpRequest& operator=(const HttpRequest& other);
    ~HttpRequest(){};

    std::string startLine() const;

    /* Getters/Setters */

    const std::string& getPath() const { return path_; }
    Http::Method::Type getMethod() const { return method_; }

    void setMethod(Http::Method::Type m) { method_ = m; }
    void setPath(const std::string& p) { path_ = p; }
};

#endif  // HTTPREQUEST_HPP
