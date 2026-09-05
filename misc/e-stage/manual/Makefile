all:
	gcc unspecified_behavior.c 2>/dev/null && ./a.out && rm a.out
	clang unspecified_behavior.c 2>/dev/null && ./a.out && rm a.out

	gcc undefined_behavior.c 2>/dev/null && ./a.out && rm a.out
	clang undefined_behavior.c 2>/dev/null && ./a.out && rm a.out
