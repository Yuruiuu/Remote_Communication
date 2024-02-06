OBJECT=main.o chat_server.o chat_list.o chat_database.o chat_thread.o

main:$(OBJECT)
	g++ $(OBJECT) -o main -levent -lmysqlclient -lpthread -std=c++11 -ljsoncpp

clean:
	rm -f *.o main
