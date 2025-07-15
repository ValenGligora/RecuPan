#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
// #include "../SolucionParcialBonificacionClientesTopicos/UtilitariasParcialBonificacionClientes.h"
// #include "../SolucionParcialBonificacionClientesTopicos/SolucionParcialBonificacionClientes.h"
// #include "../SolucionParcialBonificacionClientesTopicos/TiposArchivos.h"

// #define TDA_LISTA_IMPL_DINAMICA_VEC
// #include "../TDALista/TDALista.h"
#define NO_FILE -999
#define TAM_INI 5
#define INCREMENTO 2

#define TAM_COD_CLIENTE 16
#define TODO_OK 0
#define ERR_MEMORIA 1
#define ERR_ARCH 2

typedef struct
{
    void* vec;
    int ce;
    size_t cap;
    size_t tamElem;
}
Lista;

typedef struct
{
	char codigo[16];
	char nombre[51];
	float porcBonif;
}
Cliente;


typedef struct
{
	char codCliente[16];
	char codProd[16];
	float precioUnit;
	int cantidad;
}
Venta;


typedef struct
{
	char codCliente[16];
	int nroReg;
}
Indice;

typedef int (*Cmp)(const void* e1, const void* e2);

typedef int (*Condicion)(const void* elem);

typedef void (*Actualizar)(void* actualizado, const void* actualizador);

typedef void (*Accion)(void* elem, void* extra);

void bonificarClientes_ALU(const char* ClientesDatos, const char* ClientesIdx, const char* Ventas);
int cargarListaDesdeArchivo_ALU(Lista* lista, const char* nombreArch);
int listaInsertarAlFinal_ALU(Lista* lista, void* elem);
bool crearLista_ALU(Lista* lista, size_t tamElem);
void destruirLista_ALU(Lista* lista);

int actualizarBonificacion_ALU(const char* nombreArch,Lista* idx,Lista* ventas);
void ingresarBonificacion_ALU(FILE* arch,const void* i, float boni);
void* calcularTotal_ALU(const void* idx, void* i,size_t tamElem, Cmp cmp,FILE* arch);
int cmpCodCliente_ALU(const void* a,const void* b);
float aumentarTotal_ALU(void* a);

void mostrarArchivoClientes_ALU();
void mostrarArchivoVentas_ALU();

void mostrarListaClientes_ALU(Lista* lista);
int cmpBonificacion_ALU(const void* x, const void* y);
void listarClientesOrdXBonifDesc_ALU(const char* nombreArch);
void ordListaSeleccion_ALU(Lista* lista,Cmp cmp);
void* buscarMayor_ALU(void* inicio, void* ultimo, size_t tamElem,Cmp cmp);
void intercambiar_ALU(void* x, void* y, size_t tamElem);

int main()
{
    generarArchivoClientes();
    generarIndiceArchivoClientes();
    generarArchivoVentas();
    //A partir de aca debo hacer el resto de funciones
    //mostrarArchivoClientes();
    mostrarArchivoClientes_ALU();
    puts("\n");

    //mostrarArchivoVentas();
    mostrarArchivoVentas_ALU();

    puts("\n");


    //bonificarClientes("Clientes.dat", "Clientes.idx", "Ventas.dat");
    bonificarClientes_ALU("Clientes.dat", "Clientes.idx", "Ventas.dat");

	puts("Clientes Bonificados");
    //mostrarArchivoClientes();
	mostrarArchivoClientes_ALU();
	puts("\n");

    //listarClientesOrdXBonifDesc("Clientes.dat");
    listarClientesOrdXBonifDesc_ALU("Clientes.dat");

    return 0;
}

void listarClientesOrdXBonifDesc_ALU(const char* nombreArch){
    FILE* arch = fopen(nombreArch, "rb");
    if(!arch){
        printf("Error al abrir el archivo %s",nombreArch);
        return;
    }
    Lista listaClientes;
    if(!crearLista_ALU(&listaClientes, sizeof(Cliente))){
        puts("Error al crear Lista Cliente");
        fclose(arch);
        return;
    }
    if(cargarListaDesdeArchivo_ALU(&listaClientes,nombreArch) != TODO_OK){
        puts("Error al cargar lista Clientes");
        fclose(arch);
        destruirLista_ALU(&listaClientes);
        return;
    }
    //ordLista();
    //qsort(listaClientes.vec,listaClientes.ce,listaClientes.tamElem,cmpBonificacion_ALU);
    ordListaSeleccion_ALU(&listaClientes, cmpBonificacion_ALU);
    mostrarListaClientes_ALU(&listaClientes);

    fclose(arch);
    destruirLista_ALU(&listaClientes);
}

void ordListaSeleccion_ALU(Lista* lista, Cmp cmp){
    void* ult = lista->vec + lista->ce * lista->tamElem;
    void* mayor;
    for (void* i = lista->vec; i < ult; i+= lista->tamElem){
        mayor = buscarMayor_ALU(i,ult, lista->tamElem,cmp);
        intercambiar_ALU(mayor,i,lista->tamElem);
    }
}

void* buscarMayor_ALU(void* inicio, void* ultimo, size_t tamElem,Cmp cmp){
    void* mayor = inicio;
    for (void* i = inicio + tamElem; i < ultimo; i+=tamElem){
        if (cmp(mayor,i) <= 0)
            mayor = i;
    }
    return mayor;
}

void intercambiar_ALU(void* x, void* y, size_t tamElem){
    void* aux = malloc(tamElem);
    if(!aux){
        puts("No se pudo reservar memomoria en el ordenamiento");
        return;
    }
    memcpy(aux, x, tamElem);
    memcpy(x, y, tamElem);
    memcpy(y, aux, tamElem);
    free(aux);
}

void mostrarListaClientes_ALU(Lista* lista){
    Cliente* aux = (Cliente*) lista->vec;
    int cont= 0;
    puts("Clientes Ord. X Bonificacion");
    while (cont < lista->ce)
    {
        printf("%-*s%-*s%05.2f\n",TAM_COD_CLIENTE,aux->codigo,TAM_COD_CLIENTE,aux->nombre,aux->porcBonif);
        aux++;
        cont++;
    }
}

int cmpBonificacion_ALU(const void* x, const void* y){
    Cliente* c1 = (Cliente*) x;
    Cliente* c2 = (Cliente*) y;
    return c1->porcBonif-c2->porcBonif;
}

void mostrarArchivoVentas_ALU(){
    FILE* arch = fopen("Ventas.dat","rb");
    if (!arch){
        puts("Error al abrir el archivo Ventas.dat");
        return;
    }
    Venta aux;
    puts("Ventas");
    printf("%-*s%-*s%-*s%-s\n",TAM_COD_CLIENTE,"Cod. Cliente",TAM_COD_CLIENTE,"Cod. Prod.",TAM_COD_CLIENTE,"Precio Unit.","Cantidad");
    while (fread(&aux, sizeof(Venta),1,arch))
        printf("%-*s%-*s%08.2f\t%04d\n",TAM_COD_CLIENTE,aux.codCliente,TAM_COD_CLIENTE,aux.codProd,aux.precioUnit,aux.cantidad);
    fclose(arch);
}

void mostrarArchivoClientes_ALU(){
    FILE* arch = fopen("Clientes.dat","rb");
    if (!arch){
        puts("Error al abrir el archivo Clientes.dat");
        return;
    }
    Cliente aux;
    puts("Clientes");
    printf("%-*s%-*s%-s\n",TAM_COD_CLIENTE,"Codigo",TAM_COD_CLIENTE,"Nombre","Porc. Bonif.");
    while (fread(&aux,sizeof(Cliente),1,arch))
        printf("%-*s%-*s%05.2f\n",TAM_COD_CLIENTE,aux.codigo,TAM_COD_CLIENTE,aux.nombre,aux.porcBonif);
    fclose(arch);
}

void bonificarClientes_ALU(const char* ClientesDatos, const char* ClientesIdx, const char* Ventas){
    Lista listaIdx,listaVentas;
    if(!crearLista_ALU(&listaIdx,sizeof(Indice)))    {
        puts("Error al crear Lista Indice");
        return;
    }
    if(!crearLista_ALU(&listaVentas, sizeof(Venta))){
        destruirLista_ALU(&listaIdx);
        puts("Error al crear Lista Venta");
        return;
    }
    if(cargarListaDesdeArchivo_ALU(&listaIdx,ClientesIdx) != TODO_OK){
        destruirLista_ALU(&listaVentas);
        destruirLista_ALU(&listaIdx);
        puts("Error al cargar Lista indice");
        return;
    }
    if(cargarListaDesdeArchivo_ALU(&listaVentas,Ventas) != TODO_OK){
        destruirLista_ALU(&listaVentas);
        destruirLista_ALU(&listaIdx);
        puts("Error al cargar lista ventas");
        return;
    }
    if(actualizarBonificacion_ALU(ClientesDatos,&listaIdx,&listaVentas) != TODO_OK){
        destruirLista_ALU(&listaVentas);
        destruirLista_ALU(&listaIdx);
        puts("Error al actualizar la bonificacion");
        return;
    }
    destruirLista_ALU(&listaVentas);
    destruirLista_ALU(&listaIdx);
}

int actualizarBonificacion_ALU(const char* nombreArch,Lista* idx,Lista* ventas){
    FILE* arch = fopen(nombreArch,"r+b");
    if(!arch)   return ERR_ARCH;
    void* ultIdx = idx->vec + idx->ce * idx->tamElem;
    void* i = idx->vec;
    void* v = ventas->vec;
    while (i < ultIdx)
    {
        v = calcularTotal_ALU(i,v,ventas->tamElem,cmpCodCliente_ALU, arch);
        i += idx->tamElem;
    }
    fclose(arch);
    return TODO_OK;
}

void ingresarBonificacion_ALU(FILE* arch,const void* i, float boni){
    const Indice* idx = (Indice*) i;
    Cliente aux;
    fseek(arch,idx->nroReg * sizeof(Cliente), SEEK_SET);
    fread(&aux, sizeof(Cliente),1,arch);
    aux.porcBonif = boni;
    fseek(arch, idx->nroReg * sizeof(Cliente), SEEK_SET);
    fwrite(&aux, sizeof(Cliente), 1, arch);
}

int cmpCodCliente_ALU(const void* a,const void* b){
    const Indice* idx = (Indice*) a;
    const Venta* v = (Venta*) b;
    return strcmp(idx->codCliente,v->codCliente);
}

void* calcularTotal_ALU(const void* idx, void* i,size_t tamElem, Cmp cmp,FILE* arch){
    float boni,
        totalVenta = 0;
    while (cmp(idx,i) == 0){
        totalVenta += aumentarTotal_ALU(i);
        i+= tamElem;
    }
    if (totalVenta >= 300000){
        boni = 20;
    }else if (totalVenta >= 200000){
        boni = 10;
    }else if(totalVenta >= 100000){
        boni = 5;
    }else
        boni = 0;
    ingresarBonificacion_ALU(arch,idx,boni);
    return i;
}

float aumentarTotal_ALU(void* a){
    Venta* v = (Venta*) a;
    return v->cantidad * v->precioUnit;
}

int cargarListaDesdeArchivo_ALU(Lista* lista, const char* nombreArch){
    FILE* arch = fopen(nombreArch,"rb");
    if(!arch)   return ERR_ARCH;
    void* elem = malloc(lista->tamElem);
    if (!elem)
    {
        fclose(arch);
        return ERR_MEMORIA;
    }
    while (fread(elem,lista->tamElem,1,arch))
    {
        if(listaInsertarAlFinal_ALU(lista, elem)){
            fclose(arch);
            free(elem);
            return ERR_MEMORIA;
        }
    }
    free(elem);
    fclose(arch);
    return TODO_OK;
}

int listaInsertarAlFinal_ALU(Lista* lista, void* elem){
    if (lista->ce == lista->cap){
        int capNueva = lista->cap * INCREMENTO;
        void* aux = realloc(lista->vec, capNueva * lista->tamElem);
        if(!aux)    return ERR_MEMORIA;
        lista->cap = capNueva;
        lista->vec = aux;
    }
    void* ult = lista->vec + lista->ce * lista->tamElem;
    memcpy(ult,elem,lista->tamElem);
    lista->ce++;
    return TODO_OK;
}

bool crearLista_ALU(Lista* lista, size_t tamElem){
    lista->vec = malloc(tamElem*TAM_INI);
    if(!lista->vec) return false;
    lista->cap = TAM_INI;
    lista->ce = 0;
    lista->tamElem = tamElem;
    return true;
}

void destruirLista_ALU(Lista* lista){
    free(lista->vec);
    lista->vec = NULL;
    lista->ce = 0;
    lista->tamElem = 0;
    lista->cap = 0;
}
