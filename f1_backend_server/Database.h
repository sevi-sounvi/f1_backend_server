#pragma once
#include <string>;


// Объявляем функцию инициализации базы, чтобы main знал о её существовании
void initDatabase();

// Функция получения новостей
std::string getNewsFromDB();

// Функция записи новости (она возвращает bool и принимает три ссылки на строки)
bool addNewsToDB(const std::string& title, const std::string& content, const std::string& date);