#include "ABM.h"

t_archivo_self* find_archivo(char* nombre, t_list* archivosAux) {
	int i;
	for (i = 0; i < list_size(archivosAux); i += 1) {
		t_archivo_self * archivo = list_get(archivosAux, i);
		if (strcasecmp(archivo->nombre, nombre) == 0) {
			return archivo;
		}
	}
	return NULL;
}

char* generate_unique_id_to_nodo() {
	contador_nodo++;
	return string_itoa(contador_nodo);
}

char* generate_unique_id_to_directorio() {
	directorios->contador++;
	return string_itoa(directorios->contador);
}

char* generate_unique_id_to_archivo() {
	contador_archivos++;
	return string_itoa(contador_archivos);
}

t_archivo_self* find_archivo_by_id(char* archivo_id, t_list* archivosAux) {
	int i;
	for (i = 0; i < list_size(archivosAux); i += 1) {
		t_archivo_self * archivo = list_get(archivosAux, i);
		if (strcasecmp(archivo->id, archivo_id) == 0) {
			return archivo;
		}
	}
	return NULL;
}

int no_tiene_copia(t_archivo_self* archivo, int numero_bloque, t_nodo_self* nodo) {
	int i;
	t_bloque_archivo_self * bloque = dictionary_get(archivo->bloques, string_itoa(numero_bloque));

	if (bloque != NULL) {
		for (i = 0; i < list_size(bloque->copias); i++) {
			t_copia_self * copia = list_get(bloque->copias, i);
			if (strcasecmp(copia->nodo_bloque->nodo->nodo_id, nodo->nodo_id) == 0)
				return 0;
		}
	} else
		return 1;

	return 1;
}

t_nodo_self* find_nodo_disponible(int numero_bloque, t_archivo_self* archivo) {
	int espacio_disponible = 0;
	int i;
	t_nodo_self* selecto = NULL;
	for (i = 0; i < list_size(nodos); i++) {
		t_nodo_self * nodo = list_get(nodos, i);

		if (nodo->bloques_disponibles > espacio_disponible
				&& nodo->estado == 1 && no_tiene_copia(archivo, numero_bloque, nodo)) {
			espacio_disponible = nodo->bloques_disponibles;
			selecto = nodo;
		}
	}

	return selecto;
}

int get_bloque_disponible(t_nodo_self* nodo) {
	t_list* lista = ((t_list*) dictionary_get(bloques_nodo_disponible,
			nodo->nodo_id));

	if (list_size(lista)) {
		int disponible = list_get(lista, 0);
		list_remove(lista, 0);
		return disponible;
	}

	return -1;
}

t_nodo_self* find_nodo(char* nombre) {
	int i;
	for (i = 0; i < list_size(nodos); i += 1) {
		t_nodo_self * nodo = list_get(nodos, i);
		if (strcasecmp(nodo->nombre, nombre) == 0)
			return nodo;
	}
	return NULL;
}

void add_nodo_to_nodos(t_nodo_self* nodo) {
	list_add(nodos, nodo);
	iniciar_disponibles(nodo);
	t_list* bloque_archivo_control = list_create();
	dictionary_put(bloques_nodos_archivos, nodo->nodo_id,
			bloque_archivo_control);
}

void iniciar_disponibles(t_nodo_self*  nodo) {
	int i;
	t_list * lista = list_create();
	for (i = 1; i <= CANTIDAD_BLOQUES_NODO_DEFAULT; i++) {
		int* numero;
		numero = malloc(sizeof(int));
		numero = (int) i;
		list_add(lista, numero);
	}

	dictionary_put(bloques_nodo_disponible, nodo->nodo_id, lista);
}

t_archivo_self* archivo_no_disponible(t_archivo_self* archivo) {
	return archivo->estado == INVALIDO;
}

void chequear_alta_archivo(t_archivo_self* archivo, int numero_bloque) {
	if (archivo->estado == INVALIDO) {
		t_bloque_archivo_self* bloque = dictionary_get(archivo->bloques,
				string_itoa(numero_bloque));

		if (bloque->estado == INVALIDO) {
			t_list* copias = bloque->copias;

			int i;
			for (i = 0; i < list_size(copias); i++) {
				t_copia_self* copia = list_get(copias, i);
				if (copia->nodo_bloque->nodo->estado == VALIDO) {
					bloque->estado = VALIDO;
					archivo->bloques_invalidos -= 1;
					break;
				}
			}

			if (archivo->bloques_invalidos == 0) {
				archivo->estado = VALIDO;
			}
		}
	}
}

void reactivar_nodo(t_nodo_self* nodo) {
	if (nodo != NULL) {
		int i;
		nodo->estado = 1;
		t_list* archivos_nodo = dictionary_get(bloques_nodos_archivos,
				nodo->nodo_id);

		for (i = 0; i < list_size(archivos_nodo); i++) {
			t_bloque_archivo_control_self* bloque_archivo_control = list_get(
					archivos_nodo, i);
			t_archivo_self * archivo = find_archivo_by_id(
					bloque_archivo_control->archivo_id, archivos);
			chequear_alta_archivo(archivo,
					bloque_archivo_control->archivo_bloque);
		}
	}
}

void alta_nodo(char* nombre) {
	//TODO: iniciar conexión con nodo.
	t_nodo_self* nodoAux = find_nodo(nombre);
	if (nodoAux == NULL) {
		t_nodo_self* nodo;
		nodo = malloc(sizeof(t_nodo_self));
		nodo->estado = 1;
		nodo->nombre = nombre;
		nodo->nodo_id = generate_unique_id_to_nodo();
		nodo->bloques_disponibles = CANTIDAD_BLOQUES_NODO_DEFAULT;
		nodo->bloques = dictionary_create();
		add_nodo_to_nodos(nodo);

		txt_write_in_stdout("Se agrego el nodo ");
		txt_write_in_stdout(nombre);
		txt_write_in_stdout(".\n");
	} else {
		if (nodoAux->estado == INVALIDO) {
			reactivar_nodo(nodoAux);
			txt_write_in_stdout("Se reactivo el nodo ");
			txt_write_in_stdout(nombre);
			txt_write_in_stdout(".\n");
		} else {
			txt_write_in_stdout("El nodo ");
			txt_write_in_stdout(nombre);
			txt_write_in_stdout(" ya se encontraba activo.\n");
		}
	}
}

void chequear_baja_archivo(t_archivo_self* archivo, int archivo_bloque) {
	t_bloque_archivo_self* bloque = dictionary_get(archivo->bloques,
			string_itoa(archivo_bloque));

	if (bloque->estado == VALIDO) {
		int i;
		t_list* copias = bloque->copias;
		int desactivados = 0;

		for (i = 0; i < list_size(copias); i++) {
			t_copia_self* copia = list_get(copias, i);
			if (copia->nodo_bloque->nodo->estado == INVALIDO) {
				desactivados++;
			}
		}

		bloque->estado = desactivados != list_size(copias)
				&& list_size(copias) != 0;

		if (bloque->estado == INVALIDO) {
			archivo->bloques_invalidos += 1;
			archivo->estado = INVALIDO;
		}
	}
}

void baja_nodo(char* nombre) {
	//TODO: detener conexión con nodo.
	t_nodo_self* nodo = find_nodo(nombre);
	if (nodo != NULL) {
		if(nodo->estado != INVALIDO){
			int i;
			nodo->estado = 0;
			t_list* archivos_nodo = dictionary_get(bloques_nodos_archivos, nodo->nodo_id);

			if (archivos_nodo != NULL) {
				for (i = 0; i < list_size(archivos_nodo); i++) {
					t_bloque_archivo_control_self* bloque_archivo_control =
							list_get(archivos_nodo, i);
					t_archivo_self * archivo = find_archivo_by_id(
							bloque_archivo_control->archivo_id, archivos);
					chequear_baja_archivo(archivo,
							bloque_archivo_control->archivo_bloque);
				}
			}
			txt_write_in_stdout("El nodo ");
			txt_write_in_stdout(nombre);
			txt_write_in_stdout(" se ha dado de baja.\n");
		} else {
			txt_write_in_stdout("El nodo ");
			txt_write_in_stdout(nombre);
			txt_write_in_stdout(" ya se encontraba inactivo.\n");
		}
	} else {
		txt_write_in_stdout("No existe el nodo indicado.\n");
	}
}

char* create_directorio(char* nombre, int nivel, char* padre, char* ruta) {
	t_directorio_self* directorio;
	directorio = malloc(sizeof(t_directorio_self));
	directorio->nombre = string_duplicate(nombre);
	directorio->padre = string_duplicate(padre);
	directorio->nivel = nivel;
	directorio->ruta = string_duplicate(ruta);
	directorio->id = generate_unique_id_to_directorio();
	list_add(directorios->directorios, directorio);
	directorios->contador_directorios++;

	return directorio->id;
}

char * get_directorio_numero(char* nombre, int nivel, char* padre) {
	int i;
	char* res = "-1";
	for (i = 0; i < list_size(directorios->directorios); i++) {
		t_directorio_self* directorio = list_get(directorios->directorios, i);

		if (directorio->nivel == nivel) {
			if (strcasecmp(directorio->padre, padre) == 0) {
				if (strcasecmp(directorio->nombre, nombre) == 0)
					return directorio->id;
			}
		}
	}

	return res;
}

char* set_directorio(char* ruta, int extension) {
	int i = 0;
	int encontrado = 1;
	char* padre = "0";
	char** directorios_aux = string_split(ruta, "/");
	int sum = extension ? 1 : 0;
	char* ruta_aux = string_new();
	int total_directorios = 0;

	while (directorios_aux[i] != NULL) {
			total_directorios++;
			i++;
	}

	i = 0;
	while (1) {
		if (directorios_aux[i + sum] != NULL) {
			string_append(&ruta_aux, "/");
			string_append(&ruta_aux, directorios_aux[i]);
			if (encontrado) {
				char* padre_aux = get_directorio_numero(directorios_aux[i], i, padre);
				if (!(strcasecmp(padre_aux, "-1") == 0)) {
					padre = padre_aux;
					total_directorios--;
				} else {
					encontrado = 0;
					if (TAMANIO_MAXIMO_DIRECTORIOS >= directorios->contador_directorios + total_directorios)
						padre = create_directorio(directorios_aux[i], i, padre, ruta_aux);
					else {
						txt_write_in_stdout("No hay directorios disponibles.\n");
						break;
					}
				}
			} else {
				if (i > 0 || TAMANIO_MAXIMO_DIRECTORIOS >= directorios->contador_directorios + total_directorios)
					padre = create_directorio(directorios_aux[i], i, padre, ruta_aux);
				else {
					txt_write_in_stdout("No hay directorios disponibles.\n");
					break;
				}
			}
			i++;
		} else {
			break;
		}
	}

	return padre;
}

t_archivo_self* create_archivo(long tamanio, char* nombre) {
	t_archivo_self* archivo;
	archivo = malloc(sizeof(t_archivo_self));
	archivo->estado = 0;
	archivo->bloques = dictionary_create();
	archivo->tamanio = tamanio;
	archivo->nombre = nombre;
	archivo->bloques_invalidos = 0;
	archivo->id = generate_unique_id_to_archivo();
	archivo->ruta_id = set_directorio(nombre, true);

	return archivo;
}

t_bloque_archivo_self* create_bloque(int numero) {
	t_bloque_archivo_self* bloque;
	bloque = malloc(sizeof(t_bloque_archivo_self));
	bloque->copias = list_create();
	bloque->estado = 1;
	bloque->numero = numero;

	return bloque;
}

t_copia_self* create_copia(int bloque, t_nodo_self* nodo, char* archivo_id, int nodo_bloque_destino, int nodo_ocupado) {
	t_copia_self* copia;
	copia = malloc(sizeof(t_copia_self));
	copia->archivo_bloque = bloque;
	copia->archivo_id = string_duplicate(archivo_id);
	asignar_bloque_nodo_a_copia(copia, nodo, nodo_bloque_destino, nodo_ocupado);

	// Agrega el nodo_bloque usado a la lista de control.
	t_bloque_archivo_control_self* control;
	control = malloc(sizeof(t_bloque_archivo_control_self));
	control->archivo_bloque = bloque;
	control->archivo_id = string_duplicate(archivo_id);
	t_list* lista = dictionary_get(bloques_nodos_archivos, nodo->nodo_id);
	list_add(lista, control);

	return copia;
}

void asignar_bloque_nodo_a_copia(t_copia_self* copia, t_nodo_self* nodo, int nodo_bloque_destino, int nodo_ocupado) {
	t_nodo_bloque_self *nodo_bloque;
	nodo_bloque = malloc(sizeof(t_nodo_bloque_self));
	nodo_bloque->nodo = nodo;
	if (nodo_bloque_destino == NULL)
		nodo_bloque->bloque_nodo = get_bloque_disponible(nodo);
	else {
		nodo_bloque->bloque_nodo = nodo_bloque_destino;

		if (nodo_ocupado) {
			t_list* lista = ((t_list*) dictionary_get(bloques_nodo_disponible, nodo->nodo_id));
			int i;
			for (i = 0; i < lista->elements_count; i++) {
				int disponible = list_get(lista, i);
				if (disponible == nodo_bloque_destino) {
					list_remove(lista, i);
					break;
				}
			}
		}
	}
	dictionary_put(nodo->bloques, string_itoa(nodo_bloque->bloque_nodo), copia);
	copia->nodo_bloque = nodo_bloque;
	if (!nodo_ocupado)
		nodo->bloques_disponibles--;
}

void asignar_copia_a_bloque_archivo(t_bloque_archivo_self* bloque,
		t_copia_self* copia) {
	copia->numero = bloque->copias->elements_count;
	list_add(bloque->copias, copia);
}

void asignar_bloque_archivo_a_archivo(t_archivo_self* archivo,
		t_bloque_archivo_self* bloque) {
	dictionary_put(archivo->bloques, string_itoa(bloque->numero), bloque);
}

void add_archivo_to_archivos(t_archivo_self* archivo) {
	list_add(archivos, archivo);
}

t_archivo_datos_self* dividir_archivo(char* nombre) {
	t_archivo_datos_self* archivo;
	archivo = malloc(sizeof(archivo));

	FILE *file;
	long filelen;
	int i;
	archivo->bloques = list_create();
	int max = TAMANIO_BLOQUE_NODO;
	int maxRead = max - sizeof(char);

	file = fopen(nombre, "rb");
	if (file == NULL) {
		printf("No se encontro el archivo.");
		exit(1);
	}

	fseek(file, 0, SEEK_END);
	filelen = ftell(file);
	archivo->tamanio = filelen;
	rewind(file);

	int cantidadBloques = filelen / maxRead;
	int resto = filelen % maxRead;

	for (i = 0; i < cantidadBloques; i++) {
		char* segmento;
		segmento = (char *) malloc((max) * sizeof(char));
		fread(*&segmento, maxRead, 1, file);
		segmento = strcat(segmento, "\0");
		list_add(archivo->bloques, segmento);
	}

	if (resto != 0) {
		char* segmento;
		segmento = malloc(max);
		fread(*&segmento, max, 1, file);

		for (i = 0; i < resto; i++) {
			segmento = strcat(segmento, "\0");
		}
		list_add(archivo->bloques, segmento);
	}

	fclose(file);

	return archivo;
}

void borrado_fisico_nodo(char* nodo) {
	//TODO: Solicitud de borrado de informacion de nodo.
}

void formatear_mdfs() {
	//TODO: Test.
	int i, j;
	for (i = 0; i < list_size(nodos); i++) {
		t_nodo_self* nodo = list_get(nodos, i);
		borrado_fisico_nodo(nodo->nodo_id);
		t_list * bna = dictionary_get(bloques_nodos_archivos, nodo->nodo_id);
		t_list * bnd = dictionary_get(bloques_nodo_disponible, nodo->nodo_id);

		list_clean(bna);
		list_clean(bnd);
		dictionary_clean(nodo->bloques);

		iniciar_disponibles(nodo);
	}

	for (i = 0; i < list_size(archivos); i++) {
		t_archivo_self* archivo = list_get(archivos, i);
		t_list* bloques = archivo->bloques;
		for (i = 0; j < list_size(bloques); j++) {
			t_bloque_archivo_self* bloque = list_get(bloques, i);
			list_clean(bloque->copias);
		}
		list_clean(bloques);
	}
	list_clean(archivos);

	list_clean(directorios->directorios);
}

void mover_archivo(char* nombre, char* destino) {
	t_archivo_self* archivo = find_archivo(nombre, archivos);
	archivo->ruta_id = set_directorio(destino, true);
}

void renombrar_archivo(char * ruta, char* nombre) {
	t_archivo_self* archivo = find_archivo(ruta, archivos);
	int i = 0;
	char** ruta_aux = string_split(archivo->nombre, "/");
	char* destino = "/";

	while (ruta_aux[i + 1] != NULL) {
		string_append(&destino, ruta_aux[i]);
		string_append(&destino, "/");
		i++;
	}

	string_append(&destino, nombre);
	archivo->nombre = set_directorio(destino, true);
}

void eliminar_archivo(char * ruta) {
	int indice, i, j, k;
	t_archivo_self * archivo = NULL;
	for (indice = 0; indice < list_size(archivos); indice += 1) {
		t_archivo_self * archivo_aux = list_get(archivos, indice);
		if (strcasecmp(archivo_aux->nombre, ruta) == 0) {
			t_archivo_self * archivo = archivo_aux;
			break;
		}
	}

	if (archivo != NULL) {
		for (i = 0; i < dictionary_size(archivo->bloques); i++) {
			t_bloque_archivo_self* bloque = dictionary_get(archivo->bloques,
					string_itoa(i));
			for (j = 0; j < dictionary_size(bloque->copias); j++) {
				t_copia_self* copia = list_get(bloque->copias, i);
				t_nodo_bloque_self* bloque_nodo = copia->nodo_bloque;
				t_nodo_self * nodo = bloque_nodo->nodo;
				//TODO: Si el nodo no esta disponible, no se puede borrar los datos.
				nodo->bloques_disponibles++;
				dictionary_put(bloques_nodo_disponible, nodo->nodo_id,
						bloque_nodo->bloque_nodo);

				t_list* archivos_nodo = dictionary_get(bloques_nodos_archivos,
						nodo->nodo_id);

				for (k = list_size(archivos_nodo) - 1; k >= 0; k--) {
					t_bloque_archivo_control_self* bloque_archivo_control =
							list_get(archivos_nodo, i);
					if (strcasecmp(bloque_archivo_control->archivo_id,
							archivo->id) == 0)
						list_remove(archivos_nodo, k);
				}

			}
			list_destroy(bloque->copias);
		}

		list_remove(archivos, indice);
	}
}

void crear_directorio(char* ruta, char* nombre) {
	char * destino = string_new();
	if (ruta != NULL)
		string_append(&destino, ruta);

	string_append(&destino, "/");
	string_append(&destino, nombre);
	set_directorio(destino, false);
}

t_directorio_self* get_directorio_by_id(char* archivo_id) {
	int i;
	for (i = 0; i < list_size(directorios->directorios); i++) {
		t_directorio_self* directorio_aux = list_get(directorios->directorios,
				i);
		if (strcasecmp(archivo_id, directorio_aux->id) == 0)
			return directorio_aux;
	}

	return NULL;
}

char * get_ruta_directorio(t_directorio_self* directorio) {
	char* padre = directorio->padre;
	char* ultimo = strdup("0");
	char* ruta = string_new();
	string_append(&ruta, "/");
	string_append(&ruta, directorio->nombre);

	while (strcasecmp(padre, ultimo) != 0) {
		t_directorio_self* directorio_aux = get_directorio_by_id(padre);
		char* ruta_aux = strdup("/");
		string_append(&ruta_aux, directorio_aux->nombre);
		string_append(&ruta_aux, ruta);
		ruta = string_duplicate(ruta_aux);
		padre = string_duplicate(directorio_aux->padre);
	}

	return ruta;
}

t_directorio_self* get_directorio_by_ruta(char* ruta) {
	char** ruta_aux = string_split(ruta, "/");
	int i = 0;
	int j;
	char * padre = string_new();
	string_append(&padre, "0");
	t_directorio_self * resultado;
	resultado = malloc(sizeof(t_directorio_self));

	while (ruta_aux[i] != NULL) {
		resultado = NULL;
		for (j = 0; j < list_size(directorios->directorios); j++) {
			t_directorio_self * directorio = list_get(directorios->directorios,
					j);
			if (strcasecmp(directorio->nombre, ruta_aux[i]) == 0) {
				if (directorio->nivel == i) {
					if (strcasecmp(directorio->padre, padre) == 0) {
						padre = directorio->id;
						resultado = list_get(directorios->directorios, j);
						break;
					}
				}
			}
		}
		i++;
	}

	return resultado;
}

void renombrar_directorio(char* ruta, char* nombre) {
	char* ruta_aux = string_new();

	if (strncmp(ruta, "/", 1) == 0) {

	} else {
		string_append(&ruta_aux, "/");
	}
	string_append(&ruta_aux, ruta);

	t_directorio_self* directorio = get_directorio_by_ruta(ruta_aux);
	directorio->nombre = nombre;

	char** directorios_aux = string_split(ruta_aux, "/");
	int i = 0;
	char * ruta_final = string_new();

	while (i < directorio->nivel) {
		string_append(&ruta_final, "/");
		string_append(&ruta_final, directorios_aux[i]);
		i++;
	}

	string_append(&ruta_final, "/");
	string_append(&ruta_final, nombre);
	directorio->ruta = ruta_final;

	free(ruta_aux);
	free(directorios_aux);
}

char* get_nombre_archivo(char* ruta) {
	char** directorio = string_split(ruta, "/");
	int i = 0;

	while (directorio[i + 1] != NULL)
		i++;

	return directorio[i];
}

int get_espacio_disponible(){
	int i;
	int total = 0;
	for(i = 0; i < nodos->elements_count; i++){
		t_nodo_self* nodo = list_get(nodos, i);
		if(nodo->estado == VALIDO)
			total += nodo->bloques_disponibles;
	}

	return total;
}

int hay_espacio_disponible(int cantidad_bloques) {
	int i;
	int restantes = 0;
	int cantidad_necesaria = restantes = cantidad_bloques * 3;
	t_dictionary * nodos_aux = dictionary_create();

	for (i = 0; i < nodos->elements_count; i++) {
		t_nodo_self* nodo = list_get(nodos, i);
		if (nodo->estado == VALIDO) {

			if (nodo->bloques_disponibles > cantidad_bloques)
				restantes -= cantidad_bloques;
			else
				restantes -= nodo->bloques_disponibles;

			if (restantes <= 0)
				return true;
		}
	}

	if (restantes <= 0)
		return true;
	else
		return false;
}

void copiar_archivo_a_mdfs(char* ruta, char* destino_aux) {
//	t_archivo_datos_self* archivo_datos = dividir_archivo(ruta);

	FILE *file;
	long filelen;
	int i;
	int max = TAMANIO_BLOQUE_NODO;
	int maxRead = max - sizeof(char);

	file = fopen(ruta, "rb");

	if (file != NULL) {
		datos = malloc(sizeof(datos));
		datos->bloques = list_create();

		fseek(file, 0, SEEK_END);
		filelen = ftell(file);
		rewind(file);

		int cantidad_bloques = filelen / maxRead;
		int resto = filelen % maxRead;
		cantidad_bloques = cantidad_bloques + (resto != 0 ? 1 : 0);

		int espacio_disponible = hay_espacio_disponible(cantidad_bloques);

		if (espacio_disponible) {
			char * nombre_archivo = get_nombre_archivo(ruta);
			char * destino = string_duplicate(destino_aux);
			if (!string_ends_with(destino, "/"))
				string_append(&destino, "/");
			string_append(&destino, nombre_archivo);
			t_archivo_self * existe_archivo = find_archivo(destino, archivos);

			if (existe_archivo == NULL) {
				t_archivo_self* archivo = create_archivo(filelen, destino);
				archivo->cantidad_bloques = cantidad_bloques;

				int i;
				for (i = 0; i < cantidad_bloques; i++) {
					t_bloque_archivo_self* bloque = create_bloque(i);
					asignar_bloque_archivo_a_archivo(archivo, bloque);

					char* segmento;
					segmento = (char *) malloc((max) * sizeof(char));
					if (i - 1 < cantidad_bloques || resto == 0) {
						fread(*&segmento, maxRead, 1, file);
						segmento = strcat(segmento, "\0");
					} else {
						fread(*&segmento, max, 1, file);

						for (i = 0; i < resto; i++) {
							segmento = strcat(segmento, "\0");
						}
					}
					list_add(datos->bloques, segmento);

					//TODO: Enviar datos a nodos.
					t_copia_self* copia1 = create_copia(i, find_nodo_disponible(i, archivo), archivo->id, NULL, false);
					asignar_copia_a_bloque_archivo(bloque, copia1);

					t_copia_self* copia2 = create_copia(i, find_nodo_disponible(i, archivo), archivo->id, NULL, false);
					asignar_copia_a_bloque_archivo(bloque, copia2);

					t_copia_self* copia3 = create_copia(i, find_nodo_disponible(i, archivo), archivo->id, NULL, false);
					asignar_copia_a_bloque_archivo(bloque, copia3);
				}
				archivo->estado = VALIDO;

				add_archivo_to_archivos(archivo);
			} else {
				txt_write_in_stdout("Ya existe un archivo con el mismo nombre en la ruta indicada.\n");
			}
		} else {
			txt_write_in_stdout("No hay espacio disponible en el filesystem.\n");
		}
		fclose(file);
	} else {
		txt_write_in_stdout("No se encontro el archivo indicado.\n");
	}
}

void listar_nodos() {
	int i;

	for (i = 0; i < nodos->elements_count; i++) {
		t_nodo_self * nodo = list_get(nodos, i);
		txt_write_in_stdout("Nodo: ");
		txt_write_in_stdout(nodo->nombre);
		txt_write_in_stdout(" - Id: ");
		txt_write_in_stdout(string_itoa(nodo->nodo_id));
		txt_write_in_stdout(" - Bloques disponibles: ");
		txt_write_in_stdout(string_itoa(nodo->bloques_disponibles));
		txt_write_in_stdout(" - Estado: ");
		txt_write_in_stdout(string_itoa(nodo->estado));
		txt_write_in_stdout(".\n");
	}
}

void listar_bloques_archivo(char* nombre) {
	int j, k;

	t_archivo_self* archivo = find_archivo(nombre, archivos);

	if (archivo != NULL) {
		txt_write_in_stdout(get_nombre_archivo(archivo->nombre));
		txt_write_in_stdout("\n");
		txt_write_in_stdout("Cantidad de bloques: ");
		txt_write_in_stdout(string_itoa(archivo->cantidad_bloques));
		txt_write_in_stdout("\n");

		for (j = 0; j < archivo->cantidad_bloques; j++) {
			t_bloque_archivo_self * bloque = dictionary_get(archivo->bloques,
					string_itoa(j));
			if (bloque != NULL) {
				txt_write_in_stdout("	Bloque: ");
				txt_write_in_stdout(string_itoa(bloque->numero));
				txt_write_in_stdout(" - Estado: ");
				txt_write_in_stdout(string_itoa(bloque->estado));
				txt_write_in_stdout(" - Cantidad de copias: ");
				txt_write_in_stdout(
						string_itoa(bloque->copias->elements_count));
				txt_write_in_stdout("\n");

				for (k = 0; k < bloque->copias->elements_count; k++) {
					t_copia_self * copia = list_get(bloque->copias, k);
					txt_write_in_stdout("		Copia: ");
					txt_write_in_stdout(string_itoa(k));
					txt_write_in_stdout(" - Nodo: ");
					txt_write_in_stdout(copia->nodo_bloque->nodo->nombre);
					txt_write_in_stdout(" - Bloque: ");
					txt_write_in_stdout(
							string_itoa(copia->nodo_bloque->bloque_nodo));
					txt_write_in_stdout(".\n");
				}
			}
		}
	} else {
		txt_write_in_stdout("Archivo no encontrado.\n");
	}
}

void listar_todo() {
	int i, j, k;
	int cola = list_size(directorios->directorios);
	int encontrado = false;

	for (j = 0; j < cola; j++) {
		for (i = 0; i < list_size(directorios->directorios); i++) {
			t_directorio_self * directorio = list_get(directorios->directorios,
					i);
			if (directorio->nivel == j) {
				txt_write_in_stdout(string_itoa(directorio->nivel));
				txt_write_in_stdout(": ");
				txt_write_in_stdout(directorio->id);
				txt_write_in_stdout(" ");
				txt_write_in_stdout(directorio->padre);
				txt_write_in_stdout(" ");
				txt_write_in_stdout(directorio->nombre);
				txt_write_in_stdout(": ");
				txt_write_in_stdout(get_ruta_directorio(directorio));
				txt_write_in_stdout(".\n");
				encontrado = true;

				for (k = 0; k < archivos->elements_count; k++) {
					t_archivo_self * archivo = list_get(archivos, k);
					if (strcasecmp(archivo->ruta_id, directorio->id) == 0) {
						txt_write_in_stdout(
								get_nombre_archivo(archivo->nombre));
						txt_write_in_stdout(": ");
						txt_write_in_stdout(archivo->nombre);
						txt_write_in_stdout(" ");
						txt_write_in_stdout(string_itoa(archivo->tamanio));
						txt_write_in_stdout(" ");
						txt_write_in_stdout(string_itoa(archivo->estado));
						txt_write_in_stdout(".\n");
					}
				}
			}
		}
		if (encontrado)
			encontrado = false;
		else
			break;
	}
}

void listar_archivos() {
	int i;
	for (i = 0; i < archivos->elements_count; i++) {
		t_archivo_self * archivo = list_get(archivos, i);
		txt_write_in_stdout(get_nombre_archivo(archivo->nombre));
		txt_write_in_stdout(": ");
		txt_write_in_stdout(archivo->nombre);
		txt_write_in_stdout(" ");
		txt_write_in_stdout(string_itoa(archivo->tamanio));
		txt_write_in_stdout(" ");
		txt_write_in_stdout(string_itoa(archivo->estado));
		txt_write_in_stdout(".\n");
	}
}

void listar_directorios() {
	int i;
	int j;
	int cola = list_size(directorios->directorios);
	int encontrado = false;

	for (j = 0; j < cola; j++) {
		for (i = 0; i < list_size(directorios->directorios); i++) {
			t_directorio_self * directorio = list_get(directorios->directorios,
					i);
			if (directorio->nivel == j) {
				txt_write_in_stdout(string_itoa(directorio->nivel));
				txt_write_in_stdout(": ");
				txt_write_in_stdout(directorio->id);
				txt_write_in_stdout(" ");
				txt_write_in_stdout(directorio->padre);
				txt_write_in_stdout(" ");
				txt_write_in_stdout(directorio->nombre);
				txt_write_in_stdout(": ");
				txt_write_in_stdout(get_ruta_directorio(directorio));
				txt_write_in_stdout(".\n");
				encontrado = true;
			}
		}
		if (encontrado)
			encontrado = false;
		else
			break;
	}
}

void eliminar_directorio(char* ruta) {
	//TODO: Eliminar archivos contenidos en los directorios.
	t_directorio_self* directorio = get_directorio_by_ruta(ruta);
	t_list* directorios_a_eliminar = list_create();
	t_list* directorios_actuales = list_create();
	list_add(directorios_actuales, directorio);
	int i, j;

	for (i = list_size(directorios_actuales) - 1; i >= 0; i--) {
		t_directorio_self* directorio_padre = list_get(directorios_actuales, i);
		for (j = 0; j < list_size(directorios->directorios); j++) {
			t_directorio_self* directorio_aux = list_get(
					directorios->directorios, j);
			if (strcasecmp(directorio_aux->padre, directorio_padre->id) == 0) {
				list_add(directorios_a_eliminar, directorio_aux);
				list_add(directorios_actuales, directorio_aux);
			}
		}
		list_remove(directorios_actuales, i);
		i = list_size(directorios_actuales);
	}
	list_destroy(directorios_actuales);
	list_add(directorios_a_eliminar, directorio);

	for (i = 0; i < list_size(directorios_a_eliminar); i++) {
		t_directorio_self* directorio_a_eliminar = list_get(
				directorios_a_eliminar, i);
		for (j = 0; j < list_size(directorios->directorios); j++) {
			t_directorio_self* directorio_aux = list_get(
					directorios->directorios, j);
			if (strcasecmp(directorio_aux->id, directorio_a_eliminar->id)
					== 0) {
				list_remove(directorios->directorios, j);
				directorios->contador_directorios--;
				break;
			}
		}
	}

	list_destroy(directorios_a_eliminar);
}

int get_cantidad_niveles(char* ruta) {
	char* ruta_aux2 = string_duplicate(ruta);
	string_trim(&ruta_aux2);

	if (strcasecmp(ruta_aux2, "/") == 0 || string_is_empty(ruta_aux2))
		return 0;

	char** ruta_aux = string_split(ruta_aux2, "/");
	int contador = 0;

	while (ruta_aux[contador] != NULL)
		contador++;

	free(ruta_aux2);

	return contador;
}

int es_subdirectorio_by_ruta(char* ruta, char* destino) {
	t_directorio_self* directorio_origen = get_directorio_by_ruta(ruta);
	t_directorio_self* directorio_destino;

	int cantidad_niveles_ruta = get_cantidad_niveles(ruta);
	int cantidad_niveles_destino = get_cantidad_niveles(destino);
	if (cantidad_niveles_destino > 0) {
		set_directorio(destino, false);
		directorio_destino = get_directorio_by_ruta(destino);
	}

	return es_subdirectorio_by_directorio(directorio_origen, directorio_destino);
}

int es_subdirectorio_by_directorio(t_directorio_self* ruta,
		t_directorio_self* destino) {
	if (ruta == NULL || destino == NULL)
		return 0;

	if (strcasecmp(ruta->id, destino->id) == 0)
		return 1;

	if (strcasecmp(ruta->padre, "0") == 0
			&& strcasecmp(destino->padre, "0") == 0)
		return 0;

	if (strcasecmp(destino->padre, "0") == 0)
		return 0;

	if (ruta->nivel == destino->nivel)
		return 0;

	if (ruta->nivel > destino->nivel)
		return 0;

	t_directorio_self* directorio = get_directorio_by_id(destino->padre);

	while (strcasecmp(directorio->padre, "0") != 0) {
		if (strcasecmp(ruta->id, directorio->padre) == 0)
			return 1;
		directorio = get_directorio_by_id(directorio->padre);
	}

	return 0;
}

void mover_directorio(char* ruta, char* destino) {
	//TODO: Mover archivos contenidos en directorios.
	t_directorio_self* directorio_origen = get_directorio_by_ruta(ruta);
	t_directorio_self* directorio_destino = NULL;
	char* padre_final = strdup("0");
	int cantidad_niveles_ruta = get_cantidad_niveles(ruta);
	int cantidad_niveles_destino = get_cantidad_niveles(destino);
	if (cantidad_niveles_destino > 0) {
		set_directorio(destino, false);
		directorio_destino = get_directorio_by_ruta(destino);
		padre_final = directorio_destino->id;
	}

	if (!es_subdirectorio_by_directorio(directorio_origen, directorio_destino)
			&& strcasecmp(directorio_origen->padre, padre_final) != 0) {
		t_list* directorios_a_modificar = list_create();
		t_list* directorios_actuales = list_create();
		list_add(directorios_actuales, directorio_origen);
		int i, j;

		for (i = list_size(directorios_actuales) - 1; i >= 0; i--) {
			t_directorio_self* directorio_padre = list_get(directorios_actuales,
					i);
			for (j = 0; j < list_size(directorios->directorios); j++) {
				t_directorio_self* directorio_aux = list_get(
						directorios->directorios, j);
				if (strcasecmp(directorio_aux->padre, directorio_padre->id)
						== 0) {
					list_add(directorios_a_modificar, directorio_aux);
					list_add(directorios_actuales, directorio_aux);
				}
			}
			list_remove(directorios_actuales, i);
			i = list_size(directorios_actuales);
		}
		list_destroy(directorios_actuales);

		for (i = 0; i < list_size(directorios_a_modificar); i++) {
			t_directorio_self* directorio_a_eliminar = list_get(
					directorios_a_modificar, i);
			for (j = 0; j < list_size(directorios->directorios); j++) {
				t_directorio_self* directorio_aux = list_get(
						directorios->directorios, j);
				if (strcasecmp(directorio_aux->id, directorio_a_eliminar->id)
						== 0) {
					directorio_aux->nivel = directorio_aux->nivel
							+ (cantidad_niveles_destino - cantidad_niveles_ruta
									+ 1);
					break;
				}
			}
		}

		list_destroy(directorios_a_modificar);
		directorio_origen->nivel = directorio_origen->nivel
				+ (cantidad_niveles_destino - cantidad_niveles_ruta + 1);
		directorio_origen->padre = padre_final;
	}

}

int reconstruir_archivo(t_list* bloques, char * ruta, char* nombre) {
	//TODO: Reconstruir archivo.
	int i;
	char* ruta_aux = string_new();

	string_append(&ruta_aux, ruta);
	string_append(&ruta_aux, "/");
	string_append(&ruta_aux, nombre);

	FILE *f = fopen(ruta_aux, "w");
	if (f == NULL) {
		printf("Error opening file!\n");
		exit(1);
	}

	for (i = 0; i < list_size(bloques); i++) {
		fprintf(f, list_get(bloques, i));
	}

	fclose(f);

	return 1;
}

char* get_data_from_nodo(char* nombre, t_copia_self * bloque, int campo_temporal) {
	//TODO: Solicitar bloque a nodo.
	//TEMPORAL: Se va a agregar lo solicitado al nodo.
	return list_get(datos->bloques, campo_temporal);
}

int copiar_archivo_a_local(char* nombre, char* ruta) {
	t_list* bloques_archivo = list_create();
	t_archivo_self * archivo = find_archivo(nombre, archivos);

	int i, j, k;
	int completo = true;

	if (archivo->estado == VALIDO) {
		for (i = 0; i < dictionary_size(archivo->bloques); i++) {
			t_bloque_archivo_self* bloque = dictionary_get(archivo->bloques,
					string_itoa(i));

			int encontrado = false;
			for (j = 0; j < dictionary_size(bloque->copias); j++) {
				t_copia_self* copia = list_get(bloque->copias, j);
				t_nodo_bloque_self* bloque_nodo = copia->nodo_bloque;
				t_nodo_self * nodo = bloque_nodo->nodo;
				if (nodo->estado == VALIDO) {
					list_add(bloques_archivo,
							get_data_from_nodo(nodo->nodo_id, copia, i));
					encontrado = true;
					break;
				}
			}

			if (!encontrado) {
				completo = false;
				break;
			}
		}
	}

	if (completo) {
		reconstruir_archivo(bloques_archivo, ruta,
				get_nombre_archivo(archivo->nombre));
		list_destroy(bloques_archivo);
		return 1;
	} else {
		list_destroy(bloques_archivo);
		return 0;
	}
}

void get_MD5(char* nombre) {

}

t_bloque_archivo_self * get_bloque(char* nombre, int bloque_id) {
	t_archivo_self * archivo = find_archivo(nombre, archivos);

	if (archivo != NULL) {
		return dictionary_get(archivo->bloques, string_itoa(bloque_id));
	} else {
		return NULL;
	}
}

t_copia_self * get_copia_bloque_valida(t_bloque_archivo_self * bloque) {
	int i;
	for (i = bloque->copias->elements_count - 1; i >= 0; i--) {
		t_copia_self * copia = list_get(bloque->copias, i);

		if (copia->nodo_bloque->nodo->estado == VALIDO)
			return copia;
	}

	return NULL;
}

void ver_bloque(char* nombre, int bloque_id) {
	t_nodo_self * nodo = find_nodo(nombre);

	if (nodo != NULL) {
		t_copia_self * copia = dictionary_get(nodo->bloques, string_itoa(bloque_id));

		if (copia != NULL) {
			char * data = string_new();
			txt_write_in_stdout("CONTENIDO:\n");
			txt_write_in_stdout(get_data_from_nodo(nombre, copia, bloque_id));
			txt_write_in_stdout("\n");
		} else {
			txt_write_in_stdout("No hay datos dentro de ese nodo.\n");
		}
	} else {
		txt_write_in_stdout("No se encontro el nodo solicitado.\n");
	}
}

//void ver_bloque(char* nombre, int bloque_id) {
//	t_bloque_archivo_self * bloque = get_bloque(nombre, bloque_id);
//
//	if (bloque != NULL) {
//		t_copia_self * copia = get_copia_bloque_valida(bloque);
//
//		if (copia != NULL) {
//			char * data = string_new();
//			txt_write_in_stdout("CONTENIDO:\n");
//			txt_write_in_stdout(get_data_from_nodo(nombre, copia, bloque_id));
//			txt_write_in_stdout("\n");
//		} else {
//			txt_write_in_stdout("No se encontro una copia disponible.\n");
//		}
//	} else {
//		txt_write_in_stdout("No se encontro el bloque solicitado.\n");
//	}
//}

void borrado_fisico_bloque(){

}

void borrar_bloque(char* nombre, int bloque_nodo_id) {

	if (bloque_nodo_id <= CANTIDAD_BLOQUES_NODO_DEFAULT) {
		int i;
		t_nodo_self * nodo = find_nodo(nombre);

		if (nodo != NULL) {
			if (nodo->estado == VALIDO) {
				t_copia_self * copia = dictionary_get(nodo->bloques, string_itoa(bloque_nodo_id));

				if (copia != NULL) {
					borrado_fisico_bloque();	//TODO: borrado fisico de bloque del nodo.
					t_archivo_self* archivo = find_archivo_by_id(copia->archivo_id, archivos);
					t_bloque_archivo_self * bloque = dictionary_get(archivo->bloques, string_itoa(copia->archivo_bloque));
					int archivo_bloque = copia->archivo_bloque;
					int numero_copia = copia->numero;

					dictionary_put(bloques_nodo_disponible, nodo->nodo_id, copia->nodo_bloque->bloque_nodo);
					dictionary_remove(nodo->bloques, string_itoa(bloque_nodo_id));
					nodo->bloques_disponibles++;

					for (i = 0; i < bloque->copias->elements_count; i++) {
						t_copia_self * copia_aux = list_get(bloque->copias, i);
						if (copia_aux->numero == numero_copia) {
							list_remove(bloque->copias, i);
							break;
						}
					}

					t_list* archivos_nodo = dictionary_get(bloques_nodos_archivos, nodo->nodo_id);

					for (i = 0; i < archivos_nodo->elements_count; i++) {
						t_bloque_archivo_control_self* bloque_archivo_control = list_get(archivos_nodo, i);
						if (strcasecmp(bloque_archivo_control->archivo_id, archivo->id) == 0
								&& bloque_archivo_control->archivo_bloque == archivo_bloque) {
							list_remove(archivos_nodo, i);
							break;
						}
					}

					chequear_baja_archivo(archivo, archivo_bloque);

					txt_write_in_stdout("El bloque fue borrado exitosamente.\n");
				} else {
					txt_write_in_stdout("No hay informacion en el bloque indicado.\n");
				}
			} else {
				txt_write_in_stdout("El nodo se encuentra inactivo.\n");
			}
		} else {
			txt_write_in_stdout("No existe el nodo indicado.\n");
		}
	} else {
		txt_write_in_stdout("No existe el bloque indicado.\n");
	}
}

// Esto borraba un bloque de un archivo en general, lo que se hizo fue que borre un archivo de nodo.
//void borrar_bloque(char* nombre, int bloque_id) {
//	int i;
//	borrado_fisico_bloque();//TODO: borrado fisico de bloque del nodo.
//	t_archivo_self * archivo = find_archivo(nombre, archivos);
//	t_bloque_archivo_self * bloque = dictionary_get(archivo->bloques,string_itoa(bloque_id));
//
//	for (i = bloque->copias->elements_count - 1; i >= 0; i--) {
//		t_copia_self * copia = list_get(bloque->copias, i);
//		t_nodo_bloque_self * nodo_bloque = copia->nodo_bloque;
//		nodo_bloque->nodo->bloques_disponibles++;
//		free(nodo_bloque);
//	}
//	archivo->estado = INVALIDO;
//
//	list_destroy(bloque->copias);
//	dictionary_remove(archivo->bloques, string_itoa(bloque_id));
//}

//void copiar_bloque(char* nombre, int bloque_id, char* destino) {
//	//TODO: Copiar fisicamente al nodo indicado.
//	t_archivo_self * archivo = find_archivo(nombre, archivos);
//
//	if (archivo != NULL) {
//		t_bloque_archivo_self * bloque = get_bloque(nombre, bloque_id);
//
//		if (bloque != NULL) {
//			t_copia_self * copia = get_copia_bloque_valida(bloque);
//
//			if (copia != NULL) {
//				t_nodo_self* nodo_destino;
//				if (destino == NULL) {
//					nodo_destino = find_nodo_disponible(bloque_id, archivo);
//				} else {
//					nodo_destino = find_nodo(destino);
//				}
//
//				if (nodo_destino != NULL) {
//					if (no_tiene_copia(archivo, bloque_id, nodo_destino)) {
//						t_copia_self * copia = create_copia(bloque_id,
//								nodo_destino, archivo->id);
//						if (nodo_destino->bloques_disponibles > 0) {
//							asignar_copia_a_bloque_archivo(bloque, copia);
//							if (bloque->estado == INVALIDO) {
//								chequear_alta_archivo(archivo, bloque_id);
//
//								if (archivo->estado == VALIDO)
//									txt_write_in_stdout(
//											"Se activo un archivo inactivo.\n");
//							}
//						} else {
//							txt_write_in_stdout(
//									"El nodo indicado no tiene espacio disponible.\n");
//						}
//					} else {
//						txt_write_in_stdout(
//								"El nodo indicado ya tiene una copia del bloque.\n");
//					}
//				} else {
//					txt_write_in_stdout("No se encontro el nodo indicado.\n");
//				}
//			} else {
//				txt_write_in_stdout("No se encontro una copia disponible.\n");
//			}
//		} else {
//			txt_write_in_stdout("No se encontro el bloque solicitado.\n");
//		}
//	} else {
//		txt_write_in_stdout("No se encontro el archivo solicitado.\n");
//	}
//}

int existe_copia_en_nodo(t_bloque_archivo_self* bloque, t_nodo_self* nodo) {
	int i;
	for (i = 0; i < bloque->copias->elements_count; i++) {
		t_copia_self* copia = list_get(bloque->copias, i);
		if (strcasecmp(copia->nodo_bloque->nodo->nodo_id, nodo->nodo_id) == 0)
			return false;
	}

	return true;
}

void copiar_bloque(char* nombre_nodo_origen, int bloque_origen, char* nombre_nodo_destino, int bloque_destino) {
	//TODO: Copiar fisicamente al nodo indicado.

	if (bloque_origen <= TAMANIO_BLOQUE_NODO && bloque_destino <= TAMANIO_BLOQUE_NODO) {
		t_nodo_self * nodo_origen = find_nodo(nombre_nodo_origen);

		if (nodo_origen != NULL) {

			if (nodo_origen->estado == VALIDO) {
				t_copia_self * copia_origen = dictionary_get(nodo_origen->bloques, string_itoa(bloque_origen));

				if (copia_origen != NULL) {
					t_nodo_self* nodo_destino = find_nodo(nombre_nodo_destino);

					if (nodo_destino != NULL) {

						if (strcasecmp(nodo_destino->nodo_id, nodo_origen->nodo_id) == 0) {

							if (nodo_destino->estado == VALIDO) {
								int ocupado = false;
								t_archivo_self* archivo_origen = find_archivo_by_id(copia_origen->archivo_id, archivos);
								t_bloque_archivo_self* bloque_archivo = dictionary_get(archivo_origen->bloques,
										string_itoa(copia_origen->archivo_bloque));
								t_copia_self * copia_destino = dictionary_get(nodo_destino->bloques, string_itoa(bloque_destino));

								if (copia_destino != NULL) {

									if (existe_copia_en_nodo(bloque_archivo, nodo_destino)) {
										int i;
										t_archivo_self* archivo_destino = find_archivo_by_id(copia_destino->archivo_id, archivos);
										t_bloque_archivo_self * bloque_archivo_destino = dictionary_get(archivo_destino->bloques,
												string_itoa(copia_destino->archivo_bloque));
										int archivo_bloque_destino = copia_destino->archivo_bloque;
										int numero_copia_destino = copia_destino->numero;

										dictionary_put(bloques_nodo_disponible, nodo_destino->nodo_id,
												copia_destino->nodo_bloque->bloque_nodo);
										dictionary_remove(nodo_destino->bloques, string_itoa(bloque_destino));

										for (i = 0; i < bloque_archivo_destino->copias->elements_count; i++) {
											t_copia_self * copia_aux = list_get(bloque_archivo_destino->copias, i);
											if (copia_aux->numero == numero_copia_destino) {
												list_remove(bloque_archivo_destino->copias, i);
												break;
											}
										}

										t_list* archivos_nodo = dictionary_get(bloques_nodos_archivos, nodo_destino->nodo_id);

										for (i = 0; i < archivos_nodo->elements_count; i++) {
											t_bloque_archivo_control_self* bloque_archivo_control = list_get(archivos_nodo, i);
											if (strcasecmp(bloque_archivo_control->archivo_id, archivo_destino->id) == 0
													&& bloque_archivo_control->archivo_bloque == archivo_bloque_destino) {
												list_remove(archivos_nodo, i);
												break;
											}
										}
										chequear_baja_archivo(archivo_destino, archivo_bloque_destino);
									} else {
										ocupado = true;
									}

									t_copia_self * duplicado = create_copia(copia_origen->archivo_bloque, nodo_destino,
											copia_origen->archivo_id, bloque_destino, ocupado);
									asignar_copia_a_bloque_archivo(bloque_archivo, duplicado);

									txt_write_in_stdout("El bloque fue copiado exitosamente.\n");
								} else {
									txt_write_in_stdout("El bloque ya existe en el nodo indicado.\n");
								}
							} else {
								txt_write_in_stdout("El nodo destino esta inactivo.\n");
							}
						} else {
							txt_write_in_stdout("El nodo destino y origen son el mismo.\n");
						}
					} else {
						txt_write_in_stdout("No se encontro el nodo destino indicado.\n");
					}
				} else {
					txt_write_in_stdout("No se encontro informacion en el bloque indicado.\n");
				}
			} else {
				txt_write_in_stdout("El nodo origen esta inactivo.\n");
			}
		} else {
			txt_write_in_stdout("No se encontro el nodo origen indicado.\n");
		}
	} else {
		txt_write_in_stdout("El bloque indicado no existe.\n");
	}
}
