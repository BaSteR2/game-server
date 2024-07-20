# Учебный проект: Игровой Сервер(backend)
# Описание проекта:
Многопользовательская игра ```Dog Story```. 
Игрок управляет псом, задача которого — находить потерянные вещи и относить их в ближайшее бюро находок. 
Цель игры — набрать как можно больше игровых очков, которые начисляются за доставленные в бюро вещи.

# Требования:
Для корректной сборки и запуска проекта потребуется:
- ```Ubuntu 22.04.4 LTS```
- ```CMake 3.22.1```
- ```GCC 11.4.0```
- ```conan 1.62.0```
- ```docker 27.0.3```
- ```PostgreSQL для Ubuntu```
	
# Разворачиваем базу данных:
Создать контейнер с базой данных ```PosgresSQL```:
```
sudo docker run -d --name postgres-container -p 30432:5432 -e TZ=UTC -e POSTGRES_PASSWORD=Mys3Cr3t ubuntu/postgres:14-22.04_beta
```
Запустить контейнер:
```
sudo docker container start postgres-container
```
Переходим в контейнер PostgreSQL командой:
```
psql postgres://postgres:Mys3Cr3t@localhost:30432/
```
Создаем базу данных для игры и выходим из контейнера:
```
CREATE DATABASE game_db;
exit
```
# Установка: 
В директории проекта создать папку ```build``` и перейти в нее:
```
mkdir build && cd build
```
Установка необходимых библиотек с помощью ```conan``` и ```conanfile.txt```:
```
conan install --build missing .. -s compiler.libcxx=libstdc++11 -s build_type=Debug
```
Сборка проекта:
```
cmake .. -DCMAKE_BUILD_TYPE=Debug && cmake --build .
```
Передаем в переменную окружения```GAME_DB_URL``` url базы данных:
```
export GAME_DB_URL="postgres://postgres:Mys3Cr3t@localhost:30432/game_db"
```
Используйте флаг ```--help``` для просмотра параметров комендной строки.

Пример запуска исполняемого файла: 
```
./game_server -t 50 -c ../data/config.json -w ../static
```
Можно запустить тесты, проверяющие правильность работы некоторого функционала:
```
./game_server_tests
```
# Использование:
В случае успешного запуска, в терминале будет похожий вывод:
``` 
{"timestamp":"2024-07-19T23:37:43.320084","data":{"port":8080,"address":"0.0.0.0"},"message":"server started"}
```
В режиме ожидания, сервер готов принимать запросы. Для тестирования, откройте в браузере страницу ```http://127.0.0.1:8080```

Управление псом производится клавишами : ```left```,  ```right```, ``` up```, ```down```
# Технологии:
- C++17 STL
- библиотека boost 1.78.0 (.Asio, .Beast, .Log, .Serialization, .Json, .ProgramOptions)
- libpqxx 7.7.4
- catch2 3.1.0 (тесты)

# Планы по доработке:
- Покрыть код тестами
- Развертывание сервера в Docker-контейнере
