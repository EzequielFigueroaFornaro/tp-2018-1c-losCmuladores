#ifndef ESI_STRUCTURE_H_
#define ESI_STRUCTURE_H_

typedef struct {
	int id;
	int tiempo_de_entrada;
	int cantidad_de_instrucciones;
	int instrucction_pointer;
}__attribute((packed)) esi;

#endif

