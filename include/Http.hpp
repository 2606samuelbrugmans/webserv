/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Http.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:45:05 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 12:45:30 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_HPP
#define HTTP_HPP

#include <string>

namespace Http {

// HTTP methods
struct Method {
    enum Type { GET, POST, DELETE, UNKNOWN };
};

// Status codes
struct Status {
    enum Code {
        OK                  = 200,
        Created             = 201,
        NoContent           = 204,
        MovedPermanently    = 301,
        BadRequest          = 400,
        Unauthorized        = 401,
        Forbidden           = 403,
        NotFound            = 404,
        MethodNotAllowed    = 405,
        Conflict            = 409,
        PayloadTooLarge     = 413,
        InternalServerError = 500,
        NotImplemented      = 501,
        BadGateway          = 502,
        ServiceUnavailable  = 503
    };
};

// Content types
struct ContentType {
    enum Type {
        TEXT_HTML,
        TEXT_PLAIN,
        TEXT_CSS,
        APPLICATION_JSON,
        APPLICATION_JAVASCRIPT,
        VIDEO_MP4,
        IMAGE_PNG,
        IMAGE_JPEG,
        IMAGE_GIF,
        IMAGE_ICON,
        OCTET_STREAM
    };
};

/**
 * @brief Convert Method enum to string
 *
 * @param m Method enum value
 * @return const char* string representation of the method
 */
inline const char* methodToString(const Http::Method::Type& m) {
    switch (m) {
        case Http::Method::GET: return "GET";
        case Http::Method::POST: return "POST";
        case Http::Method::DELETE: return "DELETE";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Convert string to Method enum
 *
 * @param str string value
 * @return const Method enum representation of the method
 */
inline Http::Method::Type methodFromString(const std::string& str) {
    std::string upper(str);

    for (size_t i = 0; i < str.size(); i++) upper[i] = std::toupper(upper[i]);

    if (upper == "GET")
        return Http::Method::GET;
    else if (upper == "POST")
        return Http::Method::POST;
    else if (upper == "DELETE")
        return Http::Method::DELETE;
    return Http::Method::UNKNOWN;
}

/**
 * @brief Get Reason phrase from status code
 *
 * @param code HTTP status code
 * @return const char* standard reason phrase according to RFC 7231
 */
inline const char* reasonPhrase(unsigned int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 409: return "Conflict";
        case 413: return "Payload Too Large";
        case 415: return "Unsupported Media Type";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        default: return "";
    }
}

/**
 * @brief Get ContentType from file extension
 *
 * @param filename filename or path
 * @return Response::ContentType::Type type of content
 */
inline Http::ContentType::Type contentTypeFromFile(const std::string& filename) {
    std::size_t dotPos = filename.rfind('.');
    if (dotPos == std::string::npos) return Http::ContentType::OCTET_STREAM;

    std::string ext = filename.substr(dotPos);

    if (ext == ".html" || ext == ".htm") return ContentType::TEXT_HTML;
    if (ext == ".txt") return ContentType::TEXT_PLAIN;
    if (ext == ".css") return ContentType::TEXT_CSS;
    if (ext == ".json") return ContentType::APPLICATION_JSON;
    if (ext == ".js") return ContentType::APPLICATION_JAVASCRIPT;
    if (ext == ".mp4") return ContentType::VIDEO_MP4;
    if (ext == ".png") return ContentType::IMAGE_PNG;
    if (ext == ".jpg" || ext == ".jpeg") return ContentType::IMAGE_JPEG;
    if (ext == ".ico") return ContentType::IMAGE_ICON;
    if (ext == ".gif") return ContentType::IMAGE_GIF;

    return ContentType::OCTET_STREAM;
}

/**
 * @brief Get string representation of ContentType
 *
 * @param type ContentType enum value
 * @return const char* string representation of ContentType
 */
inline const char* contentTypeToString(ContentType::Type type) {
    switch (type) {
        case Http::ContentType::TEXT_HTML: return "text/html";
        case Http::ContentType::TEXT_PLAIN: return "text/plain";
        case Http::ContentType::TEXT_CSS: return "text/css";
        case Http::ContentType::APPLICATION_JSON: return "application/json";
        case Http::ContentType::APPLICATION_JAVASCRIPT: return "application/javascript";
        case Http::ContentType::VIDEO_MP4: return "video/mp4";
        case Http::ContentType::IMAGE_PNG: return "image/png";
        case Http::ContentType::IMAGE_JPEG: return "image/jpeg";
        case Http::ContentType::IMAGE_GIF: return "image/gif";
        case Http::ContentType::IMAGE_ICON: return "image/x-icon";
        default: return "application/octet-stream";
    }
}

/**
 * @brief Get ContentType enum from string
 *
 * @param str string representation of ContentType
 * @return ContentType::Type enum value
 */
inline ContentType::Type contentTypeFromString(const std::string& str) {
    if (str == "text/html") return Http::ContentType::TEXT_HTML;
    if (str == "text/plain") return Http::ContentType::TEXT_PLAIN;
    if (str == "text/css") return Http::ContentType::TEXT_CSS;
    if (str == "application/json") return Http::ContentType::APPLICATION_JSON;
    if (str == "application/javascript") return Http::ContentType::APPLICATION_JAVASCRIPT;
    if (str == "video/mp4") return Http::ContentType::VIDEO_MP4;
    if (str == "image/png") return Http::ContentType::IMAGE_PNG;
    if (str == "image/jpeg" || str == "image/jpg") return Http::ContentType::IMAGE_JPEG;
    if (str == "image/gif") return Http::ContentType::IMAGE_GIF;
    if (str == "image/x-icon" || str == "image/vnd.microsoft.icon")
        return Http::ContentType::IMAGE_ICON;

    return Http::ContentType::OCTET_STREAM;
}

}  // namespace Http

#endif  // HTTP_HPP
