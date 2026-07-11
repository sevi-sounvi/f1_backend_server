#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN // Отключаем старый WinSock из windows.h, убирая конфликт!
#define NOMINMAX        // ОТКЛЮЧАЕТ конфликтный макрос max из windows.h!
#define CROW_USE_BOOST  // Говорим Crow работать через движок Boost!

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "Advapi32.lib")

#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include "network.h"
#include "sqlite3.h"
#include "f1_structures.h"
// Подключаем JSON, Crow, SQLite
#include <nlohmann/json.hpp> 
#include "crow_all.h"
#include "Database.h"


using namespace std;
using json = nlohmann::json;



int main() {
    // Настраиваем консоль Windows, чтобы она корректно отображала UTF-8 (текст на английском/русском)
    SetConsoleOutputCP(CP_UTF8);

    // Запуск БД
    initDatabase();

    cout << "=== ИНИЦИАЛИЗАЦИЯ F1 BACKEND-СЕРВЕРА ===" << endl;

    // Создаем объект приложения Crow
    crow::SimpleApp app;

    // Настраиваем сетевой маршрут (роут) /api/calendar
    // Когда клиент (браузер или приложение) перейдет по этому адресу, выполнится код ниже
    CROW_ROUTE(app, "/api/calendar")([]() {
        wstring host = L"api.openf1.org";
        wstring path = L"/v1/meetings?year=2026";

        // Скачиваем сырые данные из интернета с помощью нашей функции WinINet
        string rawJsonData = makeHttpRequest(host, path);

        try {
            json parsedJson = json::parse(rawJsonData); // Парсим сырую строку в объект библиотеки json
            json responseJson = json::array(); // Создаем чистый массив JSON для ответа клиенту

            // API возвращает массив гонок. Проходим по каждому элементу массива циклом
            for (const auto& item : parsedJson) {
                json race;

                if (!item.is_object()) continue; // На всякий случай защищаем и календарь

                // Достаем данные из JSON по ключам (названия ключей мы берем из документации OpenF1 API)
                race["id"] = item.value("meeting_key", 0);
                race["name"] = item.value("meeting_official_name", "Unknow Race");
                race["country"] = item.value("country_name", "Unknown Country");
                race["date"] = item.value("date_start", "No date");

                // Добавляем Гран-При в календарь
                responseJson.push_back(race);
            }

            // Возвращаем красивый отфильтрованный JSON клиенту
            // Задаем тип контента application/json, чтобы браузер или приложение понимали формат
            crow::response res;
            res.set_header("Content-Type", "application/json");
            // ВОТ ЭТА КРИТИЧЕСКИ ВАЖНАЯ СТРОЧКА ДЛЯ КЛИЕНТА (СНИМАЕТ CORS ОШИБКУ!)
            res.set_header("Access-Control-Allow-Origin", "*");
            res.write(responseJson.dump()); // dump() превращает JSON-объект обратно в строку для отправки
            return res;
        }
        catch (const exception& e) {
            crow::response res(500); // Ошибка сервера
            res.write("Internal Server Error: " + string(e.what()));
            return res;
        }
    });

    CROW_ROUTE(app, "/api/driverstandings")([]() {
        wstring host = L"api.jolpi.ca";
        wstring path = L"/ergast/f1/2026/driverstandings.json"; 

        string rawJsonData = makeHttpRequest(host, path);

        try {
            json parsedJson = json::parse(rawJsonData);
            json responseJson = json::array();

            auto standingsLists = parsedJson["MRData"]["StandingsTable"]["StandingsLists"];

            // Проверяем, что этот список не пустой
            if (!standingsLists.empty()) {

                // Берем самый первый элемент массива StandingsLists (индекс 0) 
                // и заходим в массив "DriverStandings" [0.1]
                auto driverStandings = standingsLists[0]["DriverStandings"];

                for (const auto& item : driverStandings) {
                    json driver;

                    // Если это не объект (а строка), просто пропускаем этот элемент и идем дальше
                    if (!item.is_object()) continue; 
                    
                    // Открываем внутреннюю папку "Driver", чтобы забрать имя и номер
                    auto driverInfo = item["Driver"];
                    driver["number"] = driverInfo.value("permanentNumber", "0"); // Номер пилота

                    // Собираем полное имя (Имя + Пробел + Фамилия) [0.1]
                    string firstName = driverInfo.value("givenName", "");
                    string lastName = driverInfo.value("familyName", "");
                    driver["full_name"] = firstName + " " + lastName;

                    // Забираем очки
                    string pointsStr = item.value("points", "0");
                    driver["points"] = stof(pointsStr); ;

                    responseJson.push_back(driver);
                }

                crow::response res;
                res.set_header("Content-Type", "application/json");
                res.set_header("Access-Control-Allow-Origin", "*");
                res.write(responseJson.dump());
                return res;
            }
        }
        catch (const exception& e) {
            crow::response res(500);
            res.write("Internal Server Error (Teams): " + string(e.what()));
            return res;
        }
    });

    CROW_ROUTE(app, "/api/constructorStandings") ([]() {
        const wstring host = L"api.jolpi.ca";
        const wstring path = L"/ergast/f1/2026/constructorStandings.json";

        string rawJsonData = makeHttpRequest(host, path);

        try {
            json parsedJson = json::parse(rawJsonData);
            json responseJson = json::array();

            auto standingsLists = parsedJson["MRData"]["StandingsTable"]["StandingsLists"];
            
            if (!standingsLists.empty()) {
                auto constructorStandings = standingsLists[0]["ConstructorStandings"];

                for (const auto& item : constructorStandings) {
                    json constructor;

                    if (!item.is_object()) continue;

                    auto constructorInfo = item["Constructor"];
                    constructor["name"] = constructorInfo.value("name", "Unknown name");
                    string pointStr = item.value("points", "0");
                    constructor["points"] = stof(pointStr);

                    responseJson.push_back(constructor);
                }

                crow::response res;
                res.set_header("Content-Type", "application/json");
                res.set_header("Access-Control-Allow-Origin", "*");
                res.write(responseJson.dump());
                return res;
            }
        }
        catch (const exception& e) {
            crow::response res(500);
            res.write("Internal Server Error: " + string(e.what()));
            return res;
        }
    });

    CROW_ROUTE(app, "/api/news") ([]() {
        // Вызов функциb из database.h, которая возвращает string с JSON-массивом новостей
        string newsJsonString = getNewsFromDB();

        crow::response res;

        // Указываем заголовок
        res.set_header("Content-Type", "application/json");
        // Добавляем разрешение CORS
        res.set_header("Access-Control-Allow-Origin", "*");

        // Записываем JSON-строку в ответ
        res.write(newsJsonString);
        return res;
    });

    // РОУТ 5: АДМИНКА ДЛЯ ДОБАВЛЕНИЯ НОВОСТИ В БАЗУ ДАННЫХ
    CROW_ROUTE(app, "/api/news/add").methods(crow::HTTPMethod::Post)([](const crow::request& req) {
        try {
            // Парсим JSON-данные, которые нам прислал клиент внутри тела запроса (req.body)
            auto incomingJson = json::parse(req.body);

            // Вытаскиваем из присланного JSON заголовок, текст и дату статьи
            string title = incomingJson.value("title", "No title");
            string content = incomingJson.value("content", "No content");
            string date = incomingJson.value("date", "09.07.2026");

            // Вызов функции записи в БД
            bool success = addNewsToDB(title, content, date);

            crow::response res;
            res.set_header("Content-Type", "application/json");
            res.set_header("Access-Control-Allow-Origin", "*");
            
            if (success) {
                res.write("{\"success\": \"success\", \"message\": \"Новость успешно добавлена!\"}");
            }
            else {
                res.code = 500;
                res.write("{\"status\": \"error\", \"message\": \"Не удалось записать в БД\"}");
            }
            return res;
        }
        catch (const exception& e) {
            crow::response res(400); // Ошибка неверного запроса
            res.write("Bad Request: " + string(e.what()));
            return res;
        }
    });

    cout << "Сервер успешно запущен на порту 18080!" << endl;
    cout << "Для проверки календаря открой в браузере: http://localhost:18080/api/calendar" << endl;
    cout << "Для проверки гонщиков открой в браузере: http://localhost:18080/api/driverstandings" << endl;
    cout << "Для проверки конструкторов открой в браузере: http://localhost:18080/api/constructorStandings" << endl;
    cout << "Для проверки новостей открой в браузере: http://localhost:18080/api/news" << endl;
    cout << "Для проверки новостей открой в браузере: http://localhost:18080/api/news/add" << endl;




    // Запускаем сервер на порту 18080. 
    // Программа замрет на этой строчке и будет бесконечно слушать сеть.
    app.port(18080).multithreaded().run();

    return 0;
}

/* curl -X POST http://localhost:18080/api/news/add 
-H "Content-Type: application/json" -d "
{\"title\":\"Итоги бешеной квалификации в Сильверстоуне!\",
\"content\":\"Борьба за поул-позицию Гран-при Великобритании завершилась! 
Пилоты показали феноменальный темп, а трибуны ревели от восторга. 
Наша база данных SQLite успешно зафиксировала этот исторический заезд.\",
\"date\":\"11.07.2026\"}" */