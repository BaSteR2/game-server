# Учебный проект: Игровой Сервер(backend)
# Описание проекта:
Многопользовательская игрой “Dog Story”. Игрок управляет псом, задача которого — находить потерянные вещи и относить их в ближайшее бюро находок. Цель игры — набрать как можно больше игровых очков, которые начисляются за доставленные в бюро находок вещи.

# Требования:
Для установки необходимых библиотек понадобиться ```conan```

Для развертывания базы данных потребуется ```doker```
	
# Разворачиваем базу данных:
Создать контейнер с базой данных:
```
sudo docker run -d --name postgres-container -p 30432:5432 -e TZ=UTC -e POSTGRES_PASSWORD=Mys3Cr3t ubuntu/postgres:14-22.04_beta
```
Запустить созданный контейнер:
```
sudo docker container start postgres-container
```
Передаем в глобальную переменную url базы данных:
```
export GAME_DB_URL="postgres://postgres:Mys3Cr3t@localhost:30432/game_db"
```
# Инструкция по установке: 
Создание папки ```build``` и переход в нее:
```
mkdir build && cd build
```
Установка необходимых библиотек (```conanfile.txt``` уже есть):
```
conan install --build missing .. -s compiler.libcxx=libstdc++11 -s build_type=Debug
```
Сборка проекта:
```
cmake .. -DCMAKE_BUILD_TYPE=Debug && cmake --build .
```
Используйте флаг ```--help``` для просмотра возможных параметров:
Пример запуска: 
```
./game_server -t 50 -c ../data/config.json -w ../static
```

# Использованные технологии:
linux, c++17 STL, gcc 11.4.0, библиотека boost(.Asio, .Beast, .Log, .Test, .Ser, .Json, командная строка), СУБД PostgresSQL

# Планы на будущее:
развертывание сервера в Doker-контейнере

прочие плюшки
