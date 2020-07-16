# Como nos conviene armar un makefile?
# Primero podemos definir algunas constantes que vamos a usar muchas veces
# Como por ejemplo, bibliotecas, flags, el compilador (el "CC")

print-%  : 
	@echo $* = $($*)
	
# Definimos la tarea que va a ejecutar make por defecto Y el nombre del binario final
TARGET = instance
# Incluimos las bibliotecas que necesitamos, las commos y la biblio para el MD5
LIBS = -lcunit -lcommons -lcommons-library -lpthread
# Vamos a usar gcc como compilador, porque nos encanta!
CC = gcc
# Compilamos con los flags -g (con simbolos de debug) y -Wall (que nos muestre todos los warnings)
CFLAGS = -g -Wall
LIBPATH = -L"$(PWD)/../commons-library"
INCLUDES = -I"$(PWD)/../commons-library"

# Ahora creemos las tareas del makefile
# Cuando hagamos make sin argumentos, se ejecutará la tarea por default
# Mientras que si le pasamos un argumento, ejecutará la tarea con ese nombre
# como por ejemplo make all o make clean

.PHONY: default all clean
# Esta regla .PHONY es para decirle a make que si hay archivos con nombres similares
# a alguna regla, que entonces no asuma que el argumento es el archivo, sino que es la regla
# dado que va a tratar de tomar el archivo y no la regla por defecto

# la regla por defecto (tp0)
default: $(TARGET)
# Si ejecutan all, que ejecute la default
all: default

# Definamos un par de constantes mas, OBJECTS (que vamos a usar para generar los .o)
# y HEADERS, para incluir los headers que definamos.
# Nota: wildcard implica que seleccione todos los que cumplen con la regex (osea todos los que terminen con .h)

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c **/*.c **/**/*.c))
HEADERS = $(wildcard *.h **/*.h **/**/*.h)

# Las variables que le pasamos a la tarea, le decimos que use todos los .c y todos los .h
# Creamos los objects sin linkear
%.o: %.c $(HEADERS) 
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@ 

# Nos permite mantener los archivos temporales terminados si matan a make con Cntl-C
.PRECIOUS: $(TARGET) $(OBJECTS)

# Le pasamos a la tarea los objects ya compilados
# Usamos el compilador para linkear los objects a las bibliotecas y tener nuestro binario
$(TARGET): $(OBJECTS)
	$(CC) $(LIBPATH) $(OBJECTS) $(CFLAGS) $(LIBS) -o $@ 

# Finalmente creamos una tarea para eliminar todos los .o y el binario
clean:
	-rm -f *.o
	-rm -f $(TARGET)