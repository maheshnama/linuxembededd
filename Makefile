all:
	gcc socketServer.c database.c -o taxiMeter `mysql_config --cflags --libs`
