/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpPacket.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:49:12 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 12:49:26 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpPacket.hpp"

#include <map>
#include <sstream>
#include <string>

std::string HttpPacket::toString() const {
    std::ostringstream ss;
    ss << startLine() << "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = headers_.begin();
         it != headers_.end(); ++it) {
        ss << it->first << ": " << it->second << "\r\n";
    }
    ss << "\r\n";
    return ss.str();
}

std::string HttpPacket::getHeader(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = headers_.find(key);
    return (it != headers_.end()) ? it->second : "";
}
std::string HttpPacket::headersToString() const {
    std::ostringstream ss;
    for (std::map<std::string, std::string>::const_iterator it = headers_.begin();
         it != headers_.end(); ++it)
        ss << it->first << ": " << it->second << "\r\n";
    ss << "\r\n";
    return ss.str();
}

std::string HttpPacket::fullPacket() const {
    std::string out = this->toString();
    out.insert(out.end(), body_.begin(), body_.end());
    return out;
}
