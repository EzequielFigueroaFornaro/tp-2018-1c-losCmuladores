#include "logging.h"

void init_logger() {
	logger = log_create("ise.log", "ise", 1, LOG_LEVEL_INFO);
}
