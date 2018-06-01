#include "response_codes.h"

char* get_execution_result_description(int execution_result) {
	switch (execution_result) {
	case OK : return "Sentencia ejecutada";
	case KEY_TOO_LONG : return "Error de Tamano de Clave";
	case KEY_NOT_FOUND : return "Error de Clave no Identificada";
	case KEY_UNREACHABLE : return "Error de Clave Inaccesible";
	case KEY_LOCK_NOT_ACQUIRED : return "Error de Clave no Bloqueada";
	case KEY_BLOCKED : return "Clave bloqueada por otro proceso";
	case PARSE_ERROR : return "Error al intentar parsear sentencia";
	default : return string_from_format("Resultado desconocido (Codigo: %d)", execution_result);
	}
}
