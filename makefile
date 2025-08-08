CFLAGS += -Wall -Wextra -Wpedantic \
          -Wformat=2 -Wno-unused-parameter -Wshadow \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wredundant-decls -Wnested-externs -Wmissing-include-dirs

# GCC warnings that Clang doesn't provide:
ifeq ($(CC),gcc)
    CFLAGS += -Wjump-misses-init -Wlogical-op
endif

main: main.c
	$(CC) $(CFLAGS) main.c -o main -ljpeg

run: main
	@echo "Enter input file path:" && \
	read input_file && \
	./main Lat2-VGA16.psf output.jpg < "$$input_file" && \
	{ \
		if command -v open >/dev/null 2>&1; then open output.jpg; \
		elif command -v xdg-open >/dev/null 2>&1; then xdg-open output.jpg; \
		else echo "Please open output.jpg manually"; \
		fi \
	}

clean:
	rm -f main output.jpg

.PHONY: run clean
