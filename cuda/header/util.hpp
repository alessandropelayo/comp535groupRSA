#pragma once

#include <string_view>

extern bool v_flag;

void error(const std::string_view &err_msg);
void verbose(const std::string_view &msg);
