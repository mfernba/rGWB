#include "copiafor.h"

#include "cyassert.h"
#include "cypestr.h"

#include <stdio.h>
#include <stdarg.h>

// --------------------------------------------------------------------------------

char *copiafor_nousar(const char *formato, ...)
{
	int resultado;
	char buffer[10000];
	va_list argptr;
	
	va_start(argptr, formato);
	resultado = vsnprintf(buffer, sizeof(buffer) - 1, formato, argptr);
	assert(resultado >= 0);
	buffer[sizeof(buffer) - 1] = '\0';
	va_end(argptr);
	
	return cad_copia_cadena(buffer);
}
