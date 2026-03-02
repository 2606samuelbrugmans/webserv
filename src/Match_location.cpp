#include "webserv.hpp"


int match_location(const ServerConfig &config, std::string request_path) {
	int best_match_index = -1;
	size_t best_match_length = 0;
	//classic searching algorithm for longest prefix match
	// we want to find the location with the longest path that is a prefix of the request<
	for (size_t i = 0; i < config.locations.size(); ++i) {
		const Location &loc = config.locations[i];
		if (request_path.compare(0, loc.path.length(), loc.path) == 0) {
			if (loc.path.length() > best_match_length) {
				best_match_length = loc.path.length();
				best_match_index = static_cast<int>(i);
			}
		}
	}
	return best_match_index;
}