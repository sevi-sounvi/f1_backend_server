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
#include "f1_structures.h"
// Подключаем JSON и Crow
#include <nlohmann/json.hpp> 
#include "crow_all.h"



using namespace std;
using json = nlohmann::json;



int main() {
    // Настраиваем консоль Windows, чтобы она корректно отображала UTF-8 (текст на английском/русском)
    SetConsoleOutputCP(CP_UTF8);

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
                json race;;

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
            res.write(responseJson.dump()); // dump() превращает JSON-объект обратно в строку для отправки
            return res;
        }
        catch (const exception& e) {
            crow::response res(500); // Ошибка сервера
            res.write("Internal Server Error: " + std::string(e.what()));
            return res;
        }
    });
    cout << "Сервер успешно запущен на порту 18080!" << endl;
    cout << "Для проверки открой в браузере: http://localhost:18080/api/calendar" << endl;

    // Запускаем сервер на порту 18080. 
    // Программа замрет на этой строчке и будет бесконечно слушать сеть.
    app.port(18080).multithreaded().run();

    return 0;
}