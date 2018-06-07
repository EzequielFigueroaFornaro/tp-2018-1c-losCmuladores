#ifndef ESI_STRUCTURE_H_
#define ESI_STRUCTURE_H_

typedef struct {
	long id;
	int estado;
	int tiempo_de_entrada;
	int cantidad_de_instrucciones;
	int instrucction_pointer;
}__attribute((packed)) esi;

enum tipo_de_esi {
	ESI_BLOQUEADO = -10
};

enum estados {
	NUEVO = 1,
	BLOQUEADO = 2,
	DESBLOQUEADO = 3,
	CORRIENDO = 4,
	FINALIZADO = 5

};

#endif

