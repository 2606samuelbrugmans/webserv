/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpPacket.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:45:53 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 12:46:01 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPACKET_HPP
#define HTTPPACKET_HPP

#include <map>
#include <sstream>
#include <string>
#include <vector>

class HttpPacket {
   protected:
    std::string                        version_;
    std::map<std::string, std::string> headers_;
    std::vector<unsigned char>         body_;

   public:
    HttpPacket() : version_("HTTP/1.1") {}
    virtual ~HttpPacket() {}

    virtual std::string startLine() const = 0;

    virtual std::string toString() const;

    const std::string& getVersion() const { return version_; }
    void               setVersion(const std::string& v) { version_ = v; }

    /* Getters/Setters */

    const std::vector<unsigned char>& getBody() const { return body_; }
    size_t                            getContentLength() const { return body_.size(); }
    bool hasHeader(const std::string& key) const { return headers_.find(key) != headers_.end(); }
    std::string                               getHeader(const std::string& key) const;
    const std::map<std::string, std::string>& getHeaders() const { return headers_; }

    void setBody(const std::vector<unsigned char>& body) { body_ = body; }
    void setBody(const std::string& str) { body_.assign(str.begin(), str.end()); }
    void setHeader(const std::string& key, const std::string& value) { headers_[key] = value; }

    /* To String */

    std::string headersToString() const;
    std::string fullPacket() const;
};
#endif  // HTTPPACKET_HPP
