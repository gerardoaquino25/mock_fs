#include "ABM.h"

int es_ruta(char* ruta) {
	if (ruta == NULL)
		return 0;

	if (string_length(ruta) < 1)
		return 0;

	return string_starts_with(ruta, "/") == true;
}

void* consola() {
	char * word = string_new();
	char * ruta = string_new();
	char * ruta_aux = string_new();
	string_append(&ruta, "/");

	while (1) {
		char word_aux[256];

		txt_write_in_stdout("Ingrese una instruccion:\n");
		fgets(word_aux, sizeof(word_aux), stdin);

		word = string_duplicate(word_aux);
		string_trim(&word);
		strcpy(word, string_substring(word, 0, string_length(word)));

		char** comandos = string_split(word, " ");

		int i = 0;
		while (comandos[i] != NULL) {
			i++;
		}

		if (strcmp(comandos[0], "dir") == 0) {
			txt_write_in_stdout(ruta);
			txt_write_in_stdout("\n");
		}

		if (strcmp(comandos[0], "cd") == 0) {
			if (comandos[1] != NULL) {
				if (strcmp(comandos[1], "..") == 0) {
					if (strcmp(ruta, "/") != 0) {
						char** split_ruta = string_split(ruta, "/");
						int j = 0;
						while (split_ruta[j] != NULL) {
							j++;
						}
						int lenght = string_length(ruta) - string_length(split_ruta[j - 1]);

						ruta = string_substring(ruta, 0, lenght);
					}
				} else {
					if (es_ruta(comandos[1])) {
						ruta = string_duplicate(comandos[1]);
					} else {
						ruta_aux = string_new();
						ruta_aux = string_duplicate(ruta);
						string_append(&ruta_aux, comandos[1]);
						string_append(&ruta_aux, "/");

						if (get_directorio_by_ruta(ruta_aux) != NULL)
							ruta = string_duplicate(ruta_aux);
						else
							txt_write_in_stdout("No existe la ruta indicada.\n");
					}
				}
			}
		}

		if (strcmp(comandos[0], "start") == 0) {
			if (strcmp(comandos[1], "nodo") == 0) {
				if (comandos[2] != NULL) {
					alta_nodo(comandos[2]);
				} else {
					txt_write_in_stdout("Debe indicar un nombre para el nodo.\n");
				}
			}
		}

		if (strcmp(comandos[0], "stop") == 0) {
			if (strcmp(comandos[1], "nodo") == 0) {
				if (comandos[2] != NULL) {
					baja_nodo(comandos[2]);
				} else {
					txt_write_in_stdout("Debe indicar un nombre para el nodo.\n");
				}
			}
		}

		if (strcmp(comandos[0], "copy") == 0) {
			if (strcmp(comandos[1], "file") == 0)
				if (comandos[2] != NULL)
					copiar_archivo_a_mdfs(comandos[2], comandos[3]);

			if (strcmp(comandos[1], "bloque") == 0)
				if (comandos[2] != NULL && comandos[3])
					copiar_bloque(comandos[2], (int) strtol(comandos[3], (char **) NULL, 10), comandos[4],
							(int) strtol(comandos[5], (char **) NULL, 10));
		}

		if (strcmp(comandos[0], "export") == 0)
			if (strcmp(comandos[1], "file") == 0)
				if (comandos[2] != NULL)
					copiar_archivo_a_local(comandos[2], comandos[3]);

		if (strcmp(comandos[0], "create") == 0)
			if (strcmp(comandos[1], "dir") == 0)
				if (comandos[2] != NULL)
					crear_directorio(comandos[3], comandos[2]);

		if (strcmp(comandos[0], "rename") == 0)
			if (strcmp(comandos[1], "dir") == 0)
				if (comandos[2] != NULL)
					renombrar_directorio(comandos[3], comandos[2]);

		if (strcmp(comandos[0], "get") == 0)
			if (strcmp(comandos[1], "bloque") == 0)
				if (comandos[2] != NULL && comandos[3] != NULL)
					ver_bloque(comandos[2], (int) strtol(comandos[3], (char **) NULL, 10));

		if (strcmp(comandos[0], "delete") == 0) {
			if (strcmp(comandos[1], "bloque") == 0) {
				if (comandos[2] != NULL && comandos[3] != NULL)
					borrar_bloque(comandos[2], (int) strtol(comandos[3], (char **) NULL, 10));
			} else if (strcmp(comandos[1], "dir") == 0) {
				if (comandos[2] != NULL)
					eliminar_directorio(comandos[2]);
			} else if (strcmp(comandos[1], "file") == 0) {
				if (comandos[2] != NULL)
					eliminar_archivo(comandos[2]);
			}
		}

		if (strcmp(comandos[0], "format") == 0) {
			txt_write_in_stdout("Esta seguro que quiere formatear la unidad? [s/n].\n");
			char formatear_confirmacion_aux[256];
			fgets(formatear_confirmacion_aux, sizeof(formatear_confirmacion_aux), stdin);
			char* formatear_confirmacion = string_duplicate(formatear_confirmacion_aux);
			string_trim(&formatear_confirmacion);

			if (strcmp(formatear_confirmacion, "s") == 0)
				formatear_mdfs();
		}

		if (strcmp(comandos[0], "ls") == 0) {
			if (comandos[1] == NULL)
				listar_todo();
			else {

				if (strcmp(comandos[1], "dirs") == 0)
					listar_directorios();

				if (strcmp(comandos[1], "files") == 0)
					listar_archivos();

				if (strcmp(comandos[1], "bloques") == 0)
					if (comandos[2] != NULL)
						listar_bloques_archivo(comandos[2]);

				if (strcmp(comandos[1], "nodos") == 0)
					listar_nodos();
			}
		}

		if (strcmp(comandos[0], "exit") == 0)
			exit(1);
	}
}

int main(int argc, char *argv[]) {
	txt_write_in_stdout("Bienvenido!\n");
	iniciar();
	consola();
}
