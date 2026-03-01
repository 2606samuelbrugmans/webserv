/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:49:35 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 12:49:54 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

HttpRequest::HttpRequest() : HttpPacket(), method_(Http::Method::GET), path_("/") {}

HttpRequest::HttpRequest(const HttpRequest& other)
    : HttpPacket(other), method_(other.method_), path_(other.path_) {}

HttpRequest& HttpRequest::operator=(const HttpRequest& other) {
    if (this != &other) {
        HttpPacket::operator=(other);

        this->method_ = other.method_;
        this->path_   = other.path_;
    }
    return *this;
}

std::string HttpRequest::startLine() const {
    std::ostringstream out;

    out << Http::methodToString(method_) << " " << path_ << " " << version_;
    return out.str();
}
