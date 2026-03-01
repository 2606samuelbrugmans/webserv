/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:50:02 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 12:50:13 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequestParser.hpp"

#include <cstdlib>  // atoi
#include <deque>
#include <sstream>

#include "Http.hpp"
#include "HttpPacket.hpp"
#include "HttpRequest.hpp"

/**
 * @brief Normalize a given path, resolving '.' and '..' components
 *
 * @param request_path input path to normalize
 * @return std::string normalized path
 */
static std::string normalizePath(const std::string& request_path) {
    std::stringstream       ss(request_path);
    std::string             item;
    std::deque<std::string> parts;

    // Split by '/' and add parts to deque
    while (std::getline(ss, item, '/')) {
        if (item.empty() || item == ".") continue;
        if (item == "..") {
            if (!parts.empty()) parts.pop_back();
            continue;
        }
        parts.push_back(item);
    }

    std::stringstream normalized;
    normalized << "/";

    // Reconstruct normalized path
    for (size_t i = 0; i < parts.size(); ++i) {
        normalized << parts[i];
        if (i + 1 < parts.size()) normalized << "/";
    }

    // Preserve trailing slash if present in original path (except for root)
    if (*request_path.rbegin() == '/' && normalized.str() != "/") normalized << "/";
    return normalized.str();
}

// Trim spaces from both sides of a string
static void trim(std::string& s) {
    while (!s.empty() && std::isspace(s[0])) s.erase(s.begin());
    while (!s.empty() && std::isspace(s[s.size() - 1])) s.erase(s.end() - 1);
}

HttpRequest HttpRequestParser::parse(const std::string& raw) {
    std::istringstream ss(raw);
    HttpRequest        req;

    std::string method, path, version;
    ss >> method >> path >> version;

    req.setMethod(Http::methodFromString(method));
    req.setPath(normalizePath(path));
    req.setVersion(version);

    std::string line;
    std::getline(ss, line);  // consume leftover \r\n after start line
    while (std::getline(ss, line) && line != "\r" && !line.empty()) {
        if (*line.rbegin() == '\r') line.erase(line.size() - 1);  // Remove \r at the end

        size_t sep = line.find(":");
        if (sep == std::string::npos) continue;

        std::string key = line.substr(0, sep);
        std::string val = line.substr(sep + 1);
        trim(key);
        trim(val);
        req.setHeader(key, val);
    }

    std::string body_str;
    if (req.hasHeader("Content-Length")) {
        size_t len = std::atoi(req.getHeader("Content-Length").c_str());
        body_str.resize(len);
        ss.read(&(body_str[0]), len);
        req.setBody(body_str);
    }

    return req;
}
