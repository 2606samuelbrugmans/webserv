/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:44:49 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 13:05:07 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "webserv.hpp"

class ConfigParser {
public:
	static std::vector<ServerConfig> parse(const std::string& filepath); //throw
	static std::vector<ServerConfig> parseString(const std::string& conf);
};

#endif
