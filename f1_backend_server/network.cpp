#include "network.h"
#include <windows.h>
#include <wininet.h>

using namespace std;

#pragma comment(lib, "wininet.lib") // открываем нужную библиотеку для отправки запроса в сеть

// Функция отправки запроса и получения обратного ответа
string makeHttpRequest(const wstring& host, const wstring& path) {
	string response = "";

	// 1. Октрываем интернет-сессию. Отправляем запрос на подключение.
	HINTERNET hInternet = InternetOpen(L"F1 - API - BACKEND", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!hInternet) return "Ошибка! Не удалось открыть интернет-ссесию";

	// 2. Подключение к серверу
	HINTERNET hConnect = InternetConnect(hInternet, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	if (!hConnect) {
		InternetCloseHandle(hInternet);
		return "Ошибка! Не удалось подключиться к серверу";
	}

	// 3. Создание запроса
	HINTERNET hRequest = HttpOpenRequest(hConnect, L"GET", path.c_str(), NULL, NULL, NULL, INTERNET_FLAG_SECURE, 0);
	if (!hRequest) {
		InternetCloseHandle(hInternet);
		InternetCloseHandle(hConnect);
		return "Ошибка! Не удалось создать HTTP-запрос";
	}
	
	// 4. Отправка запроса
	bool hSend = HttpSendRequest(hRequest, NULL, 0, NULL, 0);
	if (hSend) {
		char buffer[4096]; // буффер для скорости
		DWORD bytes_read = 0;

		while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytes_read) && bytes_read > 0) {
			buffer[bytes_read] = '\0';
			response += buffer;
		}
	}
	else {
		return "Ошибка! Запрос не отправился!";
	}

	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);

	return response;
}