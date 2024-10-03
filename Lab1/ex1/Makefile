.PHONY: all
all: check gen_numbers.out select_arg.out gen_in_range.out

%.out: %.c utils.c
	gcc -o $@ $^
	
check:
	@if [ "$$(uname)" != "Linux" ]; then \
		echo "Sorry, I prefer Linux"; \
		exit 1; \
	fi

.PHONY: clean
clean:
	rm -f *.out

.PHONY: test
test: *.out
	@echo "Running tests..."
	@for exec in *.out; do \
		if [ "$$exec" = "gen_in_range.out" ]; then \
			./$$exec 1 2; \
		elif [ "$$exec" = "gen_numbers.out" ]; then \
			./$$exec 1; \
		elif [ "$$exec" = "select_arg.out" ]; then \
			./$$exec 1 2 3; \
		fi; \
		if [ $$? -ne 0 ]; then \
			echo "Error: $$exec did not return 0"; \
			exit 1; \
		fi; \
	done 
	echo "All tests passed!"
