# Compilateur
CC = gcc

# Options de compilation
CFLAGS = -Wall -DGL_SILENCE_DEPRECATION
LDFLAGS = -framework OpenGL -framework GLUT

# Nom de l'exécutable
TARGET = planetrack

# Fichiers sources
SRC = main.c volume.c k-arbre.c

# Fichiers objets
OBJ = $(SRC:.c=.o)

# Règle principale
all: $(TARGET)

# Création de l'exécutable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

# Compilation des fichiers .c en .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyer les fichiers objets
clean:
	rm -f $(OBJ)

# Nettoyer tout
fclean: clean
	rm -f $(TARGET)

# Recompiler tout
re: fclean all

.PHONY: all clean fclean re
