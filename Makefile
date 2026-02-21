NAME=a.out

CC      := clang
CFLAGS  := -Wall -Wextra -Werror -O0 -MMD -MP -Isrc -std=c23 -pedantic-errors -g

SRC_DIR := src
OBJ_DIR := build

SRCS := $(shell find $(SRC_DIR) -type f -name '*.c')
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

$(NAME): $(OBJS)
	$(CC) $(OBJS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR) $(NAME)

rebuild: clean $(NAME)




