//This file is licensed under MIT-0
//Copyright 2026 Ribbon-otter
#include <expected>
#include <string>
#include <string_view>

namespace evalmath {
    std::expected<double, std::string> eval(std::string_view formula);
}
