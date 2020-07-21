# messenger_server
Данный проект предсталяет собой однопоточный асинхронный сервер для мессенджера, с графическим интерфейсом. Сервер обеспечивает адресную передачу текстовых сообщений и файлов между пользователями.
Возможности сервера:
	1)Регистрация и авторизация пользователей на сервере.
	2)Передача текстовых сообщений между авторизованными пользователями, подключенными к серверу
	3)Передача файлов между авторизованными пользователями, подключенными к серверу

В качестве СУБД для сервера используется SQLite. 
Текст SQL-запроса для создания таблицы в БД содержится в файле create_table.sql