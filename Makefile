CFLAGS = -g -Wall
LDFLAGS = -lcglm `pkg-config --static --libs glfw3` -lvulkan

bean: *.c
	gcc $(CFLAGS) -o bean *.c $(LDFLAGS)

.PHONY: test clean

test: bean
	./bean

clean:
	rm -f bean