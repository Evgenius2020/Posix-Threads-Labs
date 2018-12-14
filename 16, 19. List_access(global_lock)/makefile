build:
	mkdir -p out
	gcc ./src/main.c ./src/list.c -pthread -o ./out/a.out

build_rwlock:
	mkdir -p out
	gcc ./src/main.c ./src/list.c -pthread -o ./out/a.out -DRWLOCK

run:
	./out/a.out