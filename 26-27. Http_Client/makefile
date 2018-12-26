build:
	mkdir -p out
	gcc ./src/main.c ./src/console_app_tools.c ./src/receiving_routine.c -o ./out/a.out

build_aio:
	mkdir -p out
	gcc ./src/main.c ./src/console_app_tools.c ./src/receiving_routine_aio.c -o ./out/a.out -lrt

run:
	./out/a.out swsoft.nsu.ru