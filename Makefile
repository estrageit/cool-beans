CFLAGS = -O2
LDFLAGS = -lcglm -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

bean: main.c
	gcc $(CFLAGS) -o bean main.c $(LDFLAGS)

.PHONY: test clean

test: bean
	./bean

clean:
	rm -f bean