/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectoryListing.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 07:26:09 by akostian          #+#    #+#             */
/*   Updated: 2026/02/14 12:49:02 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <dirent.h>
#include <sys/stat.h>

#include <algorithm>  // std::sort
#include <ctime>      // std::localtime, std::strftime
#include <iomanip>    // std::setprecision
#include <sstream>
#include <string>
#include <vector>

#include "webserv.hpp"

struct DirEntry {
    std::string name;
    bool        is_dir;
    std::string time_str;
    std::string size_str;
};

static std::string formatTime(const struct stat &statbuf) {
    char     timebuf[32];
    std::tm *tm_info = localtime(&statbuf.st_mtime);

    std::strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", tm_info);
    return std::string(timebuf);
}

static std::string formatSize(off_t size) {
    const char *units[]      = {"B", "KB", "MB", "GB", "TB"};
    short       unit_index   = 0;
    double      display_size = static_cast<double>(size);

    while (display_size >= 1024 && unit_index < 4) {
        display_size /= 1024;
        unit_index++;
    }

    std::ostringstream oss;

    if (unit_index == 0)
        oss << size << " B";
    else
        oss << std::setprecision(2) << display_size << " " << units[unit_index];
    return oss.str();
}

static bool sortEntries(const DirEntry &a, const DirEntry &b) {
    // Sort: directories first, then alphabetically
    if (a.is_dir != b.is_dir) return a.is_dir > b.is_dir;  // directories first
    return a.name < b.name;                                // then alphabetically
}

std::string DirectoryListing(const std::string &request_path, const std::string &resposne_path) {
    const char *head =
        "<head>"
        "<meta charset=\"UTF-8\">"
        "<title>Directory Listing</title>"
        "<style>"
        "  a { text-decoration: none; color: #0077cc; }"
        "  a:hover { text-decoration: underline; color: #005fa3; }"
        "  table, td, th{ padding: 4px 10px; border-collapse: collapse;}"
        "  td:not(:nth-child(1)), th:not(:nth-child(1)) { border-left: 1px solid #ddd; }"
        "  @media (prefers-color-scheme: dark) { "
        "    body { background-color: #333; color: #eee; }"
        "    a { color: #66aaff; }"
        "  }"
        "</style>"
        "</head>";

    std::ostringstream page;

    page << "<html>" << head << "<body>";

    DIR *dir = opendir(resposne_path.c_str());

    if (!dir) {
        page << "Directory not found</body></html>";
        return page.str();
    }

    std::vector<DirEntry> entries;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        struct stat stat_result;

        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;

        if (stat((resposne_path + name).c_str(), &stat_result)) continue;

        DirEntry dirEntry = {name, (entry->d_type == DT_DIR), formatTime(stat_result),
                             formatSize(stat_result.st_size)};
        entries.push_back(dirEntry);
    }
    closedir(dir);

    std::sort(entries.begin(), entries.end(), sortEntries);

    if (request_path != "/") {
        DirEntry _ = {"..", true, "", ""};
        entries.insert(entries.begin(), _);
    }
    page << "<h1>Index of " << request_path << "</h1>";

    page << "<table>";
    page << "<thead><tr><th>Name</th><th>Last modified</th><th>Size</th></tr></thead>";
    page << "<tbody>";
    for (std::vector<DirEntry>::iterator it = entries.begin(); it != entries.end(); ++it) {
        page << "<tr>";
        page << "<td><a href=\"" << it->name << (it->is_dir ? "/" : "") << "\">" << it->name
             << (it->is_dir ? "/" : "") << "</a></td>";
        page << "<td>" << it->time_str << "</td>";
        page << "<td>" << (it->is_dir ? "-" : it->size_str) << "</td>";
        page << "</tr>";
    }
    page << "</tbody>";
    page << "</table>";

    page << "</body>";
    page << "</html>";

    return page.str();
}
