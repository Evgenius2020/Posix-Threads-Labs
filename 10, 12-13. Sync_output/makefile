build_mutex:
	mkdir -p out
	gcc ./src/main.c -pthread -o ./out/a.out -DMUTEX

build_cond:
	mkdir -p out
	gcc ./src/main.c -pthread -o ./out/a.out -DCOND

build_sem:
	mkdir -p out
	gcc ./src/main.c -pthread -o ./out/a.out -DSEM

run:
	./out/a.out