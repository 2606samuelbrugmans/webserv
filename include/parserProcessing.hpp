/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parserProcessing.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:47:32 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 13:13:30 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_PROCESSING_HPP
#define PARSER_PROCESSING_HPP

#include <vector>
#include <string>
#include "webserv.hpp"

void	processServerDirectives(const std::vector<std::string>& directives, ServerConfig& serverConfig, size_t& server_max_body_size);
void	processLocationDirectives(std::vector<std::string> directives, Location& loc);
void	processGlobalDirectives(const std::vector<std::string>& directives, size_t& global_max_body_size);
void	processCgiDirectives(const std::vector<std::string>& directives, ServerConfig& serverConfig);

#endif
