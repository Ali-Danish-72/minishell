PARSER_SRCS := $(addprefix $(PARSER_DIR)/, create_cmd.c dollar.c parser.c \
				wildcard.c)
SRCS += $(PARSER_SRCS)
