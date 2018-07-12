all: server-temp server-led server-websocket

server-temp: server-temp.c mongoose.c humidity.c
	gcc $^ -o $@ -g

server-led: server-led.c mongoose.c sensehat.c
	gcc $^ -o $@ -g

server-websocket: server-websocket.c mongoose.c sensehat.c
	gcc $^ -o $@ -g

clean:
	rm -rf *.o *.dSYM server-temp server-led server-websocket

