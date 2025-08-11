# =========================
#  Makefile — RevoluCIn
#  (coloque este arquivo em src/)
# =========================

# Nome do executável
PROJECT_NAME := RevoluCIn

# Detecta SO para extensão do binário
ifeq ($(OS),Windows_NT)
	EXT := .exe
else
	EXT :=
endif

# Compilador e flags
CC      := gcc
CSTD    := -std=c99
CFLAGS  := -Wall -Wno-missing-braces -D_DEFAULT_SOURCE -O1
INCLUDE := -I. -I/usr/local/include

# Tenta usar pkg-config do raylib
PKG_RL_FLAGS := $(shell pkg-config --cflags --libs raylib 2>/dev/null)

ifeq ($(strip $(PKG_RL_FLAGS)),)
	# Fallback manual
	ifeq ($(OS),Windows_NT)
		LDLIBS := -lraylib -lopengl32 -lgdi32 -lwinmm
	else
		# Ordem importa: libs no final; inclui -lm aqui também
		LDLIBS := -lraylib -lGL -lpthread -ldl -lrt -lX11
	endif
else
	RL_CFLAGS := $(shell pkg-config --cflags raylib)
	RL_LDLIBS := $(shell pkg-config --libs raylib)
	CFLAGS   += $(RL_CFLAGS)
	LDLIBS   += $(RL_LDLIBS)
endif

# Garante -lm sempre (para fminf, cosf, etc.)
LDLIBS += -lm

# Fontes do projeto (tudo que está em src/)
SOURCES := main.c assets.c player.c phase1.c phase2.c
OBJECTS := $(SOURCES:.c=.o)

# Alvos padrão
.PHONY: all run clean rebuild

all: $(PROJECT_NAME)$(EXT)

$(PROJECT_NAME)$(EXT): $(OBJECTS)
	$(CC) $(CSTD) $(OBJECTS) -o $@ $(CFLAGS) $(INCLUDE) $(LDLIBS)

# Regra genérica .c -> .o
%.o: %.c
	$(CC) $(CSTD) $(CFLAGS) $(INCLUDE) -c $< -o $@

run: all
	./$(PROJECT_NAME)$(EXT)

clean:
	$(RM) $(OBJECTS) $(PROJECT_NAME)$(EXT)

rebuild: clean all
