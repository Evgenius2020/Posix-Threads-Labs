build_sem:
	mkdir -p out
	gcc ./src/*.c ./src/sem_implementation/*.c -pthread -o ./out/a.out -D SEM_IMPEMENTATION

build_cond:
	mkdir -p out
	gcc ./src/*.c ./src/cond_implementation/*.c -pthread -o ./out/a.out

run:
	./out/a.out