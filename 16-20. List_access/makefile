build_global_mutex:
	mkdir -p out
	gcc ./src/global_lock/main.c -pthread -o ./out/a.out

build_global_rwlock:
	mkdir -p out
	gcc ./src/global_lock/main.c -pthread -o ./out/a.out -DRWLOCK

build_local_rwlock:
	mkdir -p out
	gcc ./src/local_lock/main.c -pthread -o ./out/a.out -DRWLOCK

run:
	./out/a.out