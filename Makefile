


web:
#	emcc -o game.html main.c -s USE_SDL=2 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["FS"]' \
#        -I/path/to/lua/include -L/path/to/lua/lib -llua
	emcc -o game.html src/main.c -s USE_SDL=2 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["FS"]' \
        -I/usr/include/lua5.3 -L/usr/lib/x86_64-linux-gnu -llua5.3 \
        --preload-file scripts/enemy_ai.lua
