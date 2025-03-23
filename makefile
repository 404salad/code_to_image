CFLAGS += -Wall -Wextra -Wpedantic \
          -Wformat=2 -Wno-unused-parameter -Wshadow \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wredundant-decls -Wnested-externs -Wmissing-include-dirs

# GCC warnings that Clang doesn't provide:
ifeq ($(CC),gcc)
    CFLAGS += -Wjump-misses-init -Wlogical-op
endif

main: main.c
	$(CC) $(CFLAGS) main.c -o main 

run: main
	@echo "Enter input file path:" && read input_file && ./main Lat2-VGA16.psf output.ppm < $$input_file && open output.ppm

clean:
	rm -f main output.ppm
