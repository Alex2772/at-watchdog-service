#pragma once

#include <chrono>

namespace config {
    using namespace std::chrono_literals;

    static constexpr auto CHECK_INTERVAL = 10s;
    static constexpr auto CHECK_URL = "https://alex2772.ru";
    static constexpr auto CHECK_KEYWORDS = {"утюги", "Проекты"};
    static constexpr auto CHAT_ID = "625207005";
}