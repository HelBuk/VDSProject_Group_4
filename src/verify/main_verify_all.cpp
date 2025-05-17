#include <iostream>
#include <filesystem>
#include <string>
#include <unistd.h>  // for readlink
#include <limits.h>  // for PATH_MAX

namespace fs = std::filesystem;

std::string get_executable_dir() {
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	if (count == -1) {
		throw std::runtime_error("Failed to get executable path");
	}
	std::string path(result, count);
	return fs::path(path).parent_path().string();
}

int main() {
	std::string actual_dir = "results_c3540/txt";
	std::string expected_dir = "results/results_c3540/txt";

	std::string bin_dir = get_executable_dir();
	std::string verify_bin = bin_dir + "/VDSProject_verify";

	for (const auto& entry : fs::directory_iterator(actual_dir)) {
		if (entry.path().extension() == ".txt") {
			std::string filename = entry.path().filename().string();
			std::string actual_path = entry.path().string();
			std::string expected_path = expected_dir + "/" + filename;

			std::cout << "Verifying: " << filename << std::endl;

			if (fs::exists(expected_path)) {
				std::string command = "\"" + verify_bin + "\" \"" + actual_path + "\" \"" + expected_path + "\"";
				int ret = std::system(command.c_str());
				if (ret != 0) {
					std::cerr << "Not equivalent: " << filename << "\n";
				}
			} else {
				std::cerr << "Missing expected file: " << expected_path << "\n";
			}
		}
	}

	return 0;
}

