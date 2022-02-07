/*
	Copyright (c) 2020 tevador <tevador@gmail.com>
	All rights reserved.
*/

#include <monero_seed/monero_seed.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <cstring>

static inline void read_string_option(const char* option, int argc,
	const char** argv, const char** out, const char* def_val = nullptr) {
	for (int i = 0; i < argc - 1; ++i) {
		if (strcmp(argv[i], option) == 0) {
			*out = argv[i + 1];
			return;
		}
	}
	*out = def_val;
}

static inline void read_option(const char* option, int argc, const char** argv,
	bool& out) {
	for (int i = 0; i < argc; ++i) {
		if (strcmp(argv[i], option) == 0) {
			out = true;
			return;
		}
	}
	out = false;
}

static time_t parse_date(const char* s) {
	std::istringstream iss(s);
	char delimiter;
	int day, month, year;
	if (iss >> year >> delimiter >> month >> delimiter >> day) {
		struct tm t = { 0 };
		t.tm_mday = day;
		t.tm_mon = month - 1;
		t.tm_year = year - 1900;
		t.tm_isdst = -1;

		time_t dt = mktime(&t);
		if (dt != -1) {
			return dt;
		}
	}
	throw std::runtime_error("invalid date");
}

void print_seed(const monero_seed& seed, const char* coin, bool phrase) {
	if (!seed.correction().empty()) {
		std::cout << "Warning: corrected erasure: " << monero_seed::erasure << " -> " << seed.correction() << std::endl;
	}
	if (phrase) {
		std::cout << "Mnemonic phrase: " << seed << std::endl;
	}
	std::cout << "- coin: " << coin << std::endl;
	std::cout << "- private key: " << seed.key() << std::endl;
	auto created_on = seed.date();
	std::tm tm = *std::localtime(&created_on);
	std::cout << "- created on or after: " << std::put_time(&tm, "%d/%b/%Y") << std::endl;
}

int main(int argc, const char* argv[]) {
	bool create;
	const char* create_date;
	const char* coin;
	const char* restore;
	read_option("--create", argc, argv, create);
	read_string_option("--date", argc, argv, &create_date);
	read_string_option("--coin", argc, argv, &coin, "monero");
	read_string_option("--restore", argc, argv, &restore);

	try {
		if (create) {
			time_t time;
			if (create_date != nullptr) {
				time = parse_date(create_date);
			}
			else {
				time = std::time(nullptr);
			}
			monero_seed seed(time, coin);
			print_seed(seed, coin, true);
		}
		else if (restore != nullptr) {
			monero_seed seed(restore, coin);
			print_seed(seed, coin, false);
		}
		else {
			std::cout << "Monero 14-word mnemonic seed proof of concept" << std::endl;
			std::cout << "Usage: " << std::endl;
			std::cout << argv[0] << " --create [--date <yyyy-MM-dd>] [--coin <monero|aeon|wownero>]" << std::endl;
			std::cout << argv[0] << " --restore \"<14-word seed>\" [--coin <monero|aeon|wownero>]" << std::endl;
		}
	}
	catch (const std::exception & ex) {
		std::cout << "ERROR: " << ex.what() << std::endl;
		return 1;
	}
	return 0;
}
