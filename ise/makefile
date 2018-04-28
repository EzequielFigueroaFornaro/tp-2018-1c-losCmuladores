print-%  : 
	@echo $* = $($*)
	
TARGET = ise 
LIBS = -lcommons -pthread -lcommons-library	 
CFLAGS = -g -Wall
LIBPATH = -L"$(PWD)/../commons-library"
INCLUDES = -I"$(PWD)/../commons-library"

.PHONY: default all clean
# Esta regla .PHONY es para decirle a make que si hay archivos con nombres similares
# a alguna regla, que entonces no asuma que el argumento es el archivo, sino que es la regla
# dado que va a tratar de tomar el archivo y no la regla por defecto

default: $(TARGET)
all: default

# Definamos un par de constantes mas, OBJECTS (que vamos a usar para generar los .o)
# y HEADERS, para incluir los headers que definamos.
# Nota: wildcard implica que seleccione todos los que cumplen con la regex (osea todos los que terminen con .h)

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS) # Las variables que le pasamos a la tarea, le decimos que use todos los .c y todos los .h
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@ # Creamos los objects sin linkear

.PRECIOUS: $(TARGET) $(OBJECTS) # Nos permite mantener los archivos temporales terminados si matan a make con Cntl-C

$(TARGET): $(OBJECTS) # Le pasamos a la tarea los objects ya compilados
	$(CC) $(LIBPATH) $(OBJECTS) $(CFLAGS) $(LIBS) -o $@ # Usamos el compilador para linkear los objects a las bibliotecas y tener nuestro binario

clean: # Finalmente creamos una tarea para eliminar todos los .o y el binario
	-rm -f *.o
	-rm -f $(TARGET)