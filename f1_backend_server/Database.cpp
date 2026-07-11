#include "Database.h"
#include "sqlite3.h"
#include <iostream>
#include <string>
#include <nlohmann/json.hpp> 


using namespace std;
using json = nlohmann::json;

sqlite3* GLOBAL_DB = nullptr;

// Функция подключения базы данных
void initDatabase() {

	// Открываем или создаем файл f1_news.db
	// Маркер ":memory:" заставляет базу создаваться в оперативной памяти
	int exit = sqlite3_open(":memory:", &GLOBAL_DB);

	if (exit != SQLITE_OK) {
		cout << "[БАЗА ДАННЫХ] Ошибка при открытии файла" << endl;
		return;
	}

	// SQL-запрос для создания таблицы новостей
	string sql = "CREATE TABLE IF NOT EXISTS news ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT, "
		"title TEXT NOT NULL, "
		"content TEXT NOT NULL, "
		"date TEXT NOT NULL);";

	char* messageError;

	// Посылаем команду внутрь файла на ЖД
	exit = sqlite3_exec(GLOBAL_DB, sql.c_str(), NULL, 0, &messageError);

	if (exit != SQLITE_OK) {
		cout << "[БАЗА ДАННЫХ] Ошибка создания базы! Причина: " << sqlite3_errmsg(GLOBAL_DB) << endl;
		return;
	} else {
		cout << "[БАЗА ДАННЫХ] Файл базы подключен, таблица новостей готова!" << endl;
		return;
	}

	// ВНИМАНИЕ: sqlite3_close здесь НЕ вызываем, чтобы база не стерлась!
	//sqlite3_close(GLOBAL_DB);
}

// Функция возвращает строку в формате JSON, содержащую все новости
string getNewsFromDB() {
	if (GLOBAL_DB == nullptr) return "[]";
	
	// SQL-запрос: "Выбрать ВСЕ столбцы из таблицы news, сортируя от новых к старым (по id)"
	string sql = "SELECT id, title, content, date FROM news ORDER BY id DESC;";

	// Специальный объект-заявление для хранения результатов поиска
	sqlite3_stmt* stmt;

	// Подготавливаем запрос. Если всё ок, stmt готов к чтению
	if (sqlite3_prepare_v2(GLOBAL_DB, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
		return "[]"; // Пустой массив
	};

	// Создаем коробку-массив для новостей
	json newsArray = json::array();

	// Запускаем цикл шагов. Функция sqlite3_step переходит на новую строчку таблицы.
	// Пока строки не кончились (возвращается SQLITE_ROW), мы читаем данные!
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		json item;

		// Читаем столбцы по их номерам (индексам): id=0, title=1, content=2, date=3
		item["id"] = sqlite3_column_int(stmt, 0);
		// sqlite3 возвращает текст как unsigned char*, переводим его в понятный C++ string
		item["title"] = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
		item["content"] = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
		item["date"] = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));

		newsArray.push_back(item);
	}

	// Финализируем сканнер и закрываем базу, чтобы не копить мусор в памяти
	sqlite3_finalize(stmt);

	// Переводим JSON-массив в обычную строку битов и возвращаем её
	return newsArray.dump();
}

// Функция принимает заголовок, текст статьи, дату и записывает их в базу
bool addNewsToDB(const string& title, const string& content, const string& date) {
	if (GLOBAL_DB == nullptr) return false;

	// SQL-запрос с "знаками вопроса" ?. Это называется плейсхолдерами. 
	// Защищает базу данных от хакерских атак (SQL-инъекций)
	string sql = "INSERT INTO news(title, content, date) VALUES(? , ? , ? );";

	sqlite3_stmt* stmt;

	// Подготавливаем запрос. Если всё ок, stmt готов к чтению
	if (sqlite3_prepare_v2(GLOBAL_DB, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
		return "[]"; // Пустой массив
	};

	// Привязываем (bind) наши реальные C++ строки вместо знаков вопроса
	sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, content.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_TRANSIENT);

	// Выполняем запрос (делаем физическую запись в файл)
	int rc = sqlite3_step(stmt);

	sqlite3_finalize(stmt);

	// Если база ответила SQLITE_DONE — значит запись прошла успешно
	return (rc == SQLITE_DONE);
}