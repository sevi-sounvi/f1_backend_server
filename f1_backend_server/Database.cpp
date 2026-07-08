#include "Database.h"
#include "sqlite3.h"
#include <iostream>
#include <string>

using namespace std;

void initDatabase() {
	sqlite3* DB;

	// Открываем или создаем файл f1_news.db
	int exit = sqlite3_open("f1_news.db", &DB);

	if (exit != SQLITE_OK) {
		cout << "[БАЗА ДАННЫХ] Ошибка при открытии файла" << endl;
		return;
	}

	// SQL-запрос для создания таблицы новостей
	string sql = "CREATE TABLE IF NOT EXITS news ("
		"id INTEGER PRIMERY KEY AUTOINCREMENT"
		"title TEXT NOT NULL"
		"content TEXT NOT NULL"
		"date TEXT NOT NULL);";

	char* messageError;

	// Посылаем команду внутрь файла на ЖД
	exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messageError);

	if (exit != SQLITE_OK) {
		cout << "[БАЗА ДАННЫХ] Ошибка создания таблицы" << endl;
		return; 
	} else {
		cout << "[БАЗА ДАННЫХ] Файл базы подключен, таблица новостей готова!" << endl;
		return;
	}

	sqlite3_close(DB);
}