
BIN	= gen
SRCS	= gen.c
OBJS	= $(SRCS:.c=.o)
DEPS	= $(SRCS:.c=.d)

CFLAGS	= -Wall -O -MMD
CLINK	= $(CC) -o
STRIP	= strip

default: $(BIN) ## Build gen executable

clean: ## Clean up build artifacts
	-@rm -f $(BIN) $(OBJS) $(DEPS)

install: $(BIN) ## Install gen executable to $HOME/bin
	cp $(BIN) $(HOME)/bin
	$(STRIP) $(HOME)/bin/$(BIN)

$(BIN):	$(OBJS) ## Build executable "$(BIN)"
	$(CLINK) $(BIN) $(OBJS)

help:   ## Show a list of targets
	@grep -h -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

-include $(DEPS)
