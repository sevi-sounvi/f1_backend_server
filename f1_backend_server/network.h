#pragma once
#include <string>

// Просто объявляем функцию, чтобы другие файлы знали о её существовании
std:: string makeHttpRequest(const std::wstring& host, const std::wstring& path);
