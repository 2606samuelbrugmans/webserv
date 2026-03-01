/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseFormData.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:47:23 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 12:47:30 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSEFORMDATA_HPP
#define PARSEFORMDATA_HPP

#include <map>
#include <string>
#include <vector>

#include "HttpRequest.hpp"

struct UploadedFile {
    std::string                name;         // form field name
    std::string                filename;     // filename
    Http::ContentType::Type    contentType;  // file ContentType
    std::vector<unsigned char> data;         // file bytes
};

struct FormData {
    std::map<std::string, std::string>                fields;  // form fields
    std::map<std::string, std::vector<UploadedFile> > files;   // files
};

FormData parseMultipartFormData(const HttpRequest& request);

#endif  // PARSEFORMDATA_HPP
