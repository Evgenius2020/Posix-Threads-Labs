build_select:
	mkdir -p out
	gcc ./src/main.c ./src/lib/console_app_tools.c ./src/receiving_routine_select.c -o ./out/a.out

build_aio:
	mkdir -p out
	gcc ./src/main.c ./src/lib/console_app_tools.c ./src/receiving_routine_aio.c -o ./out/a.out -lrt

build_pthread:
	mkdir -p out
	gcc ./src/main.c ./src/receiving_routine_pthread.c ./src/lib/*.c  -o ./out/a.out -pthread

run:
	./out/a.out swsoft.nsu.ru