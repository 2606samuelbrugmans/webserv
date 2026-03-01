/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParser.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:46:32 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 12:46:41 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUESTPARSER_HPP
#define HTTPREQUESTPARSER_HPP

#include "HttpRequest.hpp"

class HttpRequestParser {
   public:
    static HttpRequest parse(const std::string& raw);
};

#endif  // HTTPREQUESTPARSER_HPP
