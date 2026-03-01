/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseFormData.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:51:08 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 12:51:15 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParseFormData.hpp"

#include <map>
#include <string>
#include <vector>

#include "HttpRequest.hpp"

static std::string extractBoundary(const std::string& contentType) {
    size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos) return "";

    pos += 9;  //  "boundary=" len
    size_t end = contentType.find(';', pos);
    if (end == std::string::npos) end = contentType.length();

    return contentType.substr(pos, end - pos);
}

static void parsePartHeaders(const std::string& headers, std::string& name, std::string& filename,
                             std::string& contentType) {
    size_t cdPos = headers.find("Content-Disposition:");
    if (cdPos != std::string::npos) {
        std::string cdLine = headers.substr(cdPos, headers.find("\r\n", cdPos) - cdPos);

        size_t namePos = cdLine.find("name=\"");
        if (namePos != std::string::npos) {
            namePos += 6;
            name = cdLine.substr(namePos, cdLine.find("\"", namePos) - namePos);
        }

        size_t fnPos = cdLine.find("filename=\"");
        if (fnPos != std::string::npos) {
            fnPos += 10;
            filename = cdLine.substr(fnPos, cdLine.find("\"", fnPos) - fnPos);
        }
    }

    size_t ctPos = headers.find("Content-Type:");
    if (ctPos != std::string::npos) {
        ctPos += 13;
        while (ctPos < headers.length() && headers[ctPos] == ' ') ctPos++;
        const size_t ctEnd = headers.find("\r\n", ctPos);
        contentType        = headers.substr(ctPos, ctEnd - ctPos);
    }
}

/**
 * @brief Parse multipart/form-data from HTTP POST request
 *
 * Extracts form fields and uploaded files from a multipart/form-data encoded request body.
 * The function parses the request body by splitting it into parts using the boundary
 * specified in the Content-Type header, then processes each part to extract either
 * a form field value or a file upload.
 *
 * @param request The HTTP request containing multipart/form-data
 * @return FormData Structure containing parsed form fields and uploaded files
 *
 * @note The request must have a Content-Type header with a valid boundary parameter
 * @note File data is stored as binary in UploadedFile::data (vector<unsigned char>)
 * @note Empty filename indicates a regular form field, not a file upload
 *
 * Example multipart format:
 * @code
 * ------WebKitFormBoundary
 * Content-Disposition: form-data; name="username"
 *
 * john_doe
 * ------WebKitFormBoundary
 * Content-Disposition: form-data; name="avatar"; filename="photo.jpg"
 * Content-Type: image/jpeg
 *
 * [binary data]
 * ------WebKitFormBoundary--
 * @endcode
 */
FormData parseMultipartFormData(const HttpRequest& request) {
    FormData result;

    std::string boundary = "--" + extractBoundary(request.getHeader("Content-Type"));

    const std::vector<unsigned char>& body = request.getBody();
    std::string                       bodyStr(body.begin(), body.end());

    size_t pos = 0;
    while (true) {
        pos = bodyStr.find(boundary, pos);
        if (pos == std::string::npos) break;

        pos += boundary.length();

        // Last boundary
        if (bodyStr.substr(pos, 2) == "--") break;

        if (bodyStr.substr(pos, 2) == "\r\n") pos += 2;

        size_t headersEnd = bodyStr.find("\r\n\r\n", pos);
        if (headersEnd == std::string::npos) break;

        // writing unparsed headers to headers, and skipping
        std::string headers = bodyStr.substr(pos, headersEnd - pos);
        pos                 = headersEnd + 4;

        size_t dataEnd = bodyStr.find(boundary, pos);
        if (dataEnd == std::string::npos) break;

        if (dataEnd >= 2 && bodyStr.substr(dataEnd - 2, 2) == "\r\n") dataEnd -= 2;

        std::string name, filename, partContentType;
        parsePartHeaders(headers, name, filename, partContentType);

        if (filename.empty()) {
            // form field found
            result.fields[name] = bodyStr.substr(pos, dataEnd - pos);
        } else {
            // form file found
            UploadedFile file;
            file.name        = name;
            file.filename    = filename;
            file.contentType = Http::contentTypeFromString(partContentType);
            file.data.assign(body.begin() + pos, body.begin() + dataEnd);
            result.files[name].push_back(file);
        }
        pos = dataEnd;
    }
    return result;
}
