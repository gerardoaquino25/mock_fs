#include "ABM.h"

int main(int argc, char *argv[]) {
	contador_archivos = 0;
	archivos = list_create();
	nodos = list_create();
	bloques_nodos_archivos = dictionary_create();
	bloques_nodo_disponible = dictionary_create();
	directorios = malloc(sizeof(t_directorios_self));
	directorios->directorios = list_create();
	directorios->contador = 0;

	alta_nodo("A");
	alta_nodo("B");
	alta_nodo("C");

	copiar_archivo_a_mdfs(ARCHIVO_NUEVO, "/home/utnso/Escritorio/");

	copiar_archivo_a_local(ARCHIVO_NUEVO, "/home/utnso/Escritorio/Pruebas");
}
