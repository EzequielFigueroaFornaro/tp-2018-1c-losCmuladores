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
	case NEED_COMPACTION : return "Se necesita compactacion";
	case START_COMPACTION : return "Inicia compactacion";
	case INSTANCE_AVAILABLE_FOR_KEY: return "Instancia encontrada para contener key solicitada";
	case NO_INSTANCE_AVAILABLE_FOR_KEY: return "No hay instancia que pueda contener la key solicitada";
	case KEY_FOUND : return "Clave encontrada en instancia";
	case KEY_VALUE_FOUND : return "Valor de clave encontrado";
	case KEY_VALUE_NOT_FOUND : return "Valor de clave no encontrado";
	default : return string_from_format("Resultado desconocido (Codigo: %d)", execution_result);
	}
}
