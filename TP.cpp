#include <ilcplex/ilocplex.h>
#include <ilcplex/cplex.h>
ILOSTLBEGIN
#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm>

#define TOL 0.1
#define TIEMPO_LIMITE 300

/* INPUT: recibe 3 parametros: 
 * i)  archivo .in del cual leer, en el formato especificado.
 * ii) "random" o "notrandom", dependiendo de si quiero que las P particiones sean
 *     random o quiero que se asigne 1 particion distinta a cada nodo
 * iii)proporcion de particiones / cantNodos (en caso de haber elegido notrandom SE IGNORA) --> 0 <= num <= 1
 * iv) "bb" o "cb", para elegir el algoritmo con el cual resolver
 * v) epsilonClique --> 0 <= num <= 1
 * vi) epsilonAgujero --> 0 <= num <= 1
 * vii) numeroDeModelo --> 0 = Pedro, 1 = Santiago
*/

/* Defino las constantes del problema */
int N; // CANTIDAD DE VERTICES
int E; // CANTIDAD DE ARISTAS
int P; // CANTIDAD DE PARTICIONES
float porcentajeParticiones; // porcentaje de particiones que paso por parametro
string algoritmo;
int CANT_RESTR_CLIQUES = 5;
int CANT_RESTR_AGUJEROS = 5;

vector <vector <int> > M; // M_v1_v2 = 1 si (v1,v2) in E; 0 sino.
vector <vector <int> > S; // en S_p estan los vertices de la particion p.

// para leer archivos en los que hay que asignar una particion a cada vertice.
/* Formato:
 *  c FILE: anna.col
    c Translated from Stanford GraphBase File: anna.gb
    c Stanford GraphBase ID: book(?anna?,138,0,1,239,0,0,0)
    p edge |V| |E|*2
    e 1 36
    e 2 45
    ... etc.
*/

/*
 *
    Las variables se guardan:
    w0,w1,...,w(P-1),x00,x01,...,x0(P-1),x10,x11,...,x1(P-1),..,x(N-1)0,x(N-1)1,...,x(N-1)(P-1)
    Es decir, primero los w que representan los colores.
    Luego ordenamos por variable, y dentro de cada variable ordenamos por color.

 */

///Al principio me muevo en los indices del color por lo que lo muevo ese offset
///Luego, usamos que a lo sumo uso P colores (P es la cantidad de particiones, entonces a lo sumo tengo un P coloreo)
///Para ir al indice correcto, me voy hasta cantColoresPorVariable * indiceVariable
///Luego, sumo el indice del color para moverme al color correcto dentro de la variable
int xijIndice(int indiceVariable, int indiceColor){
    return P + P * indiceVariable + indiceColor;
}


void mostrameValores(CPXENVptr env, CPXLPptr lp){
    
    int cantVariables = P + N*P;

    double *sol = new double[cantVariables];
    CPXgetx(env, lp, sol, 0, cantVariables - 1);

    cout << "Colores: " << endl;
    for(int v = 0; v < P; v++){    
        cout << sol[v] << " ";
    }
    cout << endl;

    cout << "Nodos:" << endl;
    for(int i = 0; i < N; i++){
        std::cout << std::setw(2) << std::setfill('0') << i << " ";
        for(int color = 0; color < P; color++){
            cout << sol[xijIndice(i, color)] << " ";
        }
        cout << endl;
    }
    
    /*for(int j=0; j<P; j++) {
        if(sol[j] > 0.0001) {
            cout << "W_" << j << " = " << sol[j] << endl;
        }
        else{
            cout << "W_" << j << " = 0" << endl;
        }
    }
    for(int i=0; i<N; i++) {
        for(int j=0; j<P; j++) {
            if(sol[P + P*i + j] > 0.0001) {
                cout << "X_" << i << "_" << j << " = " << sol[P+P*i+j] << endl;
            }
            else{
                cout << "X_" << i << "_" << j << " = 0" << endl;
            }
        }
    }
*/
    delete [] sol;
}


/// Ordena de menor a mayor
bool ordenamientoPeso(const pair<int,double> &A, const pair<int,double> &B) {
    return (A.second < B.second);
}

////
vector<int> dameClique(double *sol, int color) {
    vector<int> clique;
    vector< pair< int, double > > vecinosPotenciales;
    int indiceDelMasPesado = 0;
    double pesoMax = 0;
    for(int nodo=0; nodo<N; nodo++) {
        if (sol[xijIndice(nodo,color)] > pesoMax) {
            pesoMax = sol[xijIndice(nodo,color)];
            indiceDelMasPesado = nodo;
        }
    }
    //cout << pesoMax << endl;
    if (pesoMax == 0) {return clique; }

    double pesoAcumulado = pesoMax;
    for(int vecino=0; vecino < N; vecino++) {
        if(M[vecino][indiceDelMasPesado] == 1){
            vecinosPotenciales.push_back(make_pair(vecino, sol[xijIndice(vecino,color)]));
        }
    }
    sort(vecinosPotenciales.begin(), vecinosPotenciales.end(), ordenamientoPeso);
    clique.push_back(indiceDelMasPesado);
    while( not vecinosPotenciales.empty()) {
        // agrego nuevo nodo a la clique
        pair<int,double> vecinoAgregar = vecinosPotenciales[vecinosPotenciales.size() - 1];
        clique.push_back( vecinoAgregar.first);
        pesoAcumulado += vecinoAgregar.second;
        vecinosPotenciales.pop_back();
        // saco los nodos que nos son vecinos del nuevo nodo del 'vecinosPotenciales'
        vector< pair<int, double> >::iterator it = vecinosPotenciales.begin();
        while( it != vecinosPotenciales.end() ) {
            if(M[it->first][vecinoAgregar.first] == 0) {
                it = vecinosPotenciales.erase(it);
            } else { 
                it++; 
            }
        }
    }
    if (pesoAcumulado > 1.0 + TOL and clique.size() > 2) {
        return clique;
    } else {
        return vector<int>(); 
    }
}

// =================================================================================
void read(string randomness) {  // debo dividir a los vertices en 'porcentajeParticiones' * N particiones

    // me salteo las 3 primeras filas (asumo que todas las instancias vienen de esta forma)
    string line; char c;
    // (segun el formato de los casos de prueba) - salteo las 'c', la 'p' y el 'edge'
    while(cin>>line) { if (line == "p") break; }
    cin >> line;
    
    cin >> N >> E;

    if(randomness == "random") {  // random -> me pasaron por parametro cuantas particiones se 
        P = porcentajeParticiones * N;    //       aceptan como maximo
    }
    if(randomness == "notrandom") { P = N; }  // notrandom -> una particion por nodo
    
    // Asignacion de particiones (puede ser al azar o una particion para cada nodo)
    if(randomness == "random") {
        S.resize(P);  // S guarda que vertices pertenecen a cada particion
        for(int v=0; v<N; v++) {  // asigno particiones al azar. Pueden haber particiones no asignadas
            int vp = rand() % P;
            S[vp].push_back(v);
        }
        // tengo que eliminar los slots de S que quedan vacios, y decrementar P de forma acorde
        int count =0;
        vector< vector<int> >::iterator it = S.begin();
        while( it != S.end() ) {
            if(it->empty()) {
                it = S.erase(it);
                count++;
            } else { it++; }
        }
        P = P - count;
    
    } else if (randomness == "notrandom") {
        S.resize(N);
        for(int v=0; v<N; v++) {
            S[v].push_back(v);
            //cout << v << " " << vp << endl;
        }
    } else {
        cout << "ERROR: parametros mal ingresados!" << endl;
    }
    cout << "Cantidad final de particiones: " << P << endl;

    M.resize(N);
    for(int i=0; i<N; i++) { M[i].resize(N); } // M in NxN.
    // v_i = 1...N ----> v_i = 0...N-1
    int v1, v2;
    for(int e=0; e<E; e++) {  // para cada eje, guardo los nodos de sus extremos.
        cin >> c >> v1 >> v2; // (formato) - leo el caracter extra que hay adelante
        //cout << v1 << " " << v2 << endl;
        M[v1-1][v2-1] = 1;
        M[v2-1][v1-1] = 1;
    }
}

int dameParticion(int vertice){
    for(unsigned int i=0; i < S.size(); ++i){
        if ( std::find(S[i].begin(), S[i].end(), vertice) != S[i].end() ){
            return i;
        }
    }
    ///en caso de error
    return -1;
}

void agregarRestriccionClique(CPXENVptr env, CPXLPptr lp, std::vector<int> indicesClique, int numeroColor){

    // En total, son P + N*P variables ( las W[j] y las X[i][j] )
    int cantVariables = P + N*P;

    int ccnt = 0;
    int rcnt = 1; //Agrego una sola restriccion

    int nzcnt = 0;  // al ppio es cero (para cada valor q agrego, lo voy a incrementar en 1)

    char sense[rcnt]; // Sentido de la desigualdad. 'G' es mayor o igual y 'E' para igualdad, 'L' menor o igual

    double *rhs = new double[rcnt]; // Termino independiente de las restricciones.
    int *matbeg = new int[rcnt];    //Posicion en la que comienza cada restriccion en matind y matval.
    int *matind = new int[rcnt*cantVariables];       // Array con los indices de las variables con coeficientes != 0 en la desigualdad.
    double *matval = new double[rcnt*cantVariables]; // Array que en la posicion i tiene coeficiente ( != 0) 

    ///Sumatoria de xij - wj
    matbeg[0] = nzcnt;
    rhs[0]    = 0;
    sense[0]  = 'L';

    matind[nzcnt] = numeroColor;
    matval[nzcnt] = -1 * P;
    nzcnt++;

    for(int i = 0; i < indicesClique.size(); i++) {
        matind[nzcnt] = xijIndice(indicesClique[i], numeroColor);
        matval[nzcnt] = 1;
        nzcnt++;
    }

    int status = CPXaddrows(env, lp, ccnt, rcnt, nzcnt, rhs, sense, matbeg, matind, matval, NULL, NULL);

    if (status) {
        cerr << "Problema agregando restricciones." << endl;
        exit(1);
    }

    delete[] rhs;
    delete[] matbeg;
    delete[] matind;
    delete[] matval;
}

void agregarRestriccionAgujero(CPXENVptr env, CPXLPptr lp, std::vector<int> indicesAgujero, int numeroColor){

    // En total, son P + N*P variables ( las W[j] y las X[i][j] )
    int cantVariables = P + N*P;

    int ccnt = 0;
    int rcnt = 1; //Agrego una sola restriccion

    int nzcnt = 0;  // al ppio es cero (para cada valor q agrego, lo voy a incrementar en 1)

    char sense[rcnt]; // Sentido de la desigualdad. 'G' es mayor o igual y 'E' para igualdad, 'L' menor o igual

    double *rhs = new double[rcnt]; // Termino independiente de las restricciones.
    int *matbeg = new int[rcnt];    //Posicion en la que comienza cada restriccion en matind y matval.
    int *matind = new int[rcnt*cantVariables];       // Array con los indices de las variables con coeficientes != 0 en la desigualdad.
    double *matval = new double[rcnt*cantVariables]; // Array que en la posicion i tiene coeficiente ( != 0) 

    ///Sumatoria de xij - wj
    matbeg[0] = nzcnt;
    rhs[0]    = 0;
    sense[0]  = 'L';

    matind[nzcnt] = numeroColor;
    matval[nzcnt] = -1 * ((indicesAgujero - 1) / 2);
    nzcnt++;

    for(int i = 0; i < indicesAgujero.size(); i++) {
        matind[nzcnt] = xijIndice(indicesAgujero[i], numeroColor);
        matval[nzcnt] = 1;
        nzcnt++;
    }

    int status = CPXaddrows(env, lp, ccnt, rcnt, nzcnt, rhs, sense, matbeg, matind, matval, NULL, NULL);

    if (status) {
        cerr << "Problema agregando restricciones." << endl;
        exit(1);
    }

    delete[] rhs;
    delete[] matbeg;
    delete[] matind;
    delete[] matval;
}

// ================================================================================

int main(int argc, char **argv) {

    if(not freopen(argv[1], "r", stdin)) {
        return 1;
    }

    // freopen("output.out", "w", stdout);

    char ejes[100];
    char labels[100];
    char test[100];
    string randomness     = argv[2];
    porcentajeParticiones = atof(argv[3]);
    algoritmo             = argv[4];
    double epsilonClique  = atof(argv[5]);
    double epsilonAgujero = atof(argv[6]);
    int numeroDeModelo    = atoi(argv[7]);

    sprintf(ejes, "ejes.out");
    sprintf(labels, "labels.out");
    if(randomness == "notrandom") { 
        sprintf(test, "%s%s", argv[1], argv[2]);
    }
    else if (randomness == "random") {
        sprintf(test, "%s%s", argv[1], argv[3]);
    }else {
        cout << "Paramtros mal introducidos" << endl;
        return 0;
    }

    read(randomness); // cada elemento de la particion conformado por un unico nodo

    // Le paso por parametro el algoritmo a implementar: bb = branch and bound, cb = cut and branch
    if(algoritmo != "bb" && algoritmo != "cb") {
        cout << "Error introduciendo parametro de algoritmo a ser aplicado " << endl;
        return 0;
    }

    // ==============================================================================================

    // Genero el problema de cplex.
    int status;
    // Creo el entorno.
    CPXENVptr env = CPXopenCPLEX(&status); // Puntero al entorno.
    CPXLPptr lp; // Puntero al LP
     
    if (env == NULL) {
        cerr << "Error creando el entorno" << endl;
        exit(1);
    }
        
    // Creo el LP.
    lp = CPXcreateprob(env, &status, "instancia coloreo de grafo particionado");
     
    if (lp == NULL) {
        cerr << "Error creando el LP" << endl;
        exit(1);
    }

    // Definimos las variables. En total, son P + N*P variables ( las W[j] y las X[i][j] )
    int cantVariables = P + N*P;
    double *ub, *lb, *objfun; // Cota superior, cota inferior, coeficiente de la funcion objetivo.
    char *xctype, **colnames; // tipo de la variable , string con el nombre de la variable.
    ub       = new double[cantVariables]; 
    lb       = new double[cantVariables];
    objfun   = new double[cantVariables];
    xctype   = new char[cantVariables];
    colnames = new char*[cantVariables];

    for (int i = 0; i < cantVariables; i++) {
        ub[i] = 1.0; // seteo upper y lower bounds de cada variable
        lb[i] = 0.0;
        if(i < P) {  // agrego el costo en la funcion objetivo de cada variables
            objfun[i] = 1;  // busco minimizar Sum(W_j) para j=0..P (la cantidad de colores utilizados).
        } else {
            objfun[i] = 0;  // los X[i][j] no contribuyen a la funcion objetivo
        }
        xctype[i] = 'B';  // 'C' es continua, 'B' binaria, 'I' Entera.
        colnames[i] = new char[10];
    }

    /* Defino el tipo de variable BoolVarMatrix, que sera utilizado en la resolucion
     * recordar: X_v_j = 1 sii el color j es asignado al vertice v
     * recordar: W_j = 1 si X_v_j = 1 para al menos un vertice v
    */
    for(int j=0; j<P; j++) {
        sprintf(colnames[j], "W_%d", j);
        //cout << colnames[j] << endl;
    }
    for(int i=1; i<=N; i++) {
        for(int j=0; j<P; j++) {
            sprintf(colnames[i*P+j], "X_%d_%d", i-1, j);
            //cout << colnames[i*P+j] << endl;
        }
    }


    // ========================== Agrego las columnas. =========================== //
    if(algoritmo == "cb" ) {
        // si quiero resolver la relajacion, agregar los cortes y despues resolver el MIP, no agrego xctype
        status = CPXnewcols(env, lp, cantVariables, objfun, lb, ub, NULL, colnames);
    } else if (algoritmo == "bb") {
        // si quiero hacer MIP, directamente, con brancha and bound, agrego xctype
        status = CPXnewcols(env, lp, cantVariables, objfun, lb, ub, xctype, colnames);
    } else {
        cout << "Error: parametro de algoritmo bb/cb mal introducido" << endl;
        return 0;
    }
    
    if (status) {
        cerr << "Problema agregando las variables CPXnewcols" << endl;
        exit(1);
    }
    
    // Libero las estructuras.
    for (int i = 0; i < cantVariables; i++) {
        delete[] colnames[i];
    }

    delete[] ub;
    delete[] lb;
    delete[] objfun;
    delete[] xctype;
    delete[] colnames;

    // CPLEX por defecto minimiza. Le cambiamos el sentido a la funcion objetivo si se quiere maximizar.
    // CPXchgobjsen(env, lp, CPX_MAX);

    // ================================================================================================ //
    // ===================================== Restricciones ============================================ //

    // i)   Asigno exactamente un color a exactamente un vertice de cada particion ( P restricciones )
    // ii)  Dos vertices adyacentes no pueden tener el mismo color ( E restricciones )
    // iii) Los W_j estan bien armados, en funcion de X_v_j ( 2*P restricciones )

    // ccnt = numero nuevo de columnas en las restricciones.
    // rcnt = cuantas restricciones se estan agregando.
    // nzcnt = # de coeficientes != 0 a ser agregados a la matriz. Solo se pasan los valores que no son cero.
 
    int ccnt = 0;
    int rcnt;
    if(numeroDeModelo == 0){
        rcnt = P + (E*P)/2 + 2*P;  // Cota maxima a la cantidad de restricciones
    }
    else{
        rcnt = P + (E*P)/2 + N*P;
    }
                                    // (E/2 porque en la entrada se supone que en la entrada me pasan 2 veces cada eje)
    int nzcnt = 0;  // al ppio es cero (para cada valor q agrego, lo voy a incrementar en 1)

    char sense[rcnt]; // Sentido de la desigualdad. 'G' es mayor o igual y 'E' para igualdad, 'L' menor o igual

    double *rhs = new double[rcnt]; // Termino independiente de las restricciones.
    int *matbeg = new int[rcnt];    //Posicion en la que comienza cada restriccion en matind y matval.
    //cout << "hola" << endl;
    int *matind = new int[rcnt*cantVariables];       // Array con los indices de las variables con coeficientes != 0 en la desigualdad.
    //cout << "chau" << endl;
    double *matval = new double[rcnt*cantVariables]; // Array que en la posicion i tiene coeficiente ( != 0) de la variable matind[i] en la restriccion.

    // CPLEX va a leer hasta la cantidad nzcnt que le pasemos.
    int cantRestricciones = 0;  // r = numero de restriccion

    // i) P restricciones - exactamente un color a cada vertice (una restriccion por cada particion)
    for(int p = 0; p < P; p++) {
        matbeg[cantRestricciones] = nzcnt;
        rhs[cantRestricciones]    = 1  ;
        sense[cantRestricciones]  = 'E';
        for(int i=0; i<N; i++) {  // para cada conjunto dentro de la particion, 'e' es el indice de un elemento en el cjto
            for(int e=0; e < S[p].size() && S[p][e] == i; e++) {  // si el nodo i esta en el conjunto S[p] de la particion,
                for(int j=0; j<P; j++) {
                    matind[nzcnt] = xijIndice(i, j);
                    matval[nzcnt] = 1;
                    nzcnt++;
                }
            }
        }
        cantRestricciones++;
    }

    // ii) (E*P)/2 restricciones mas (para cada eje, para cada color pero solo cuando i < j)
    int cuenta = 0;
    for(int k=0; k<P; k++) {  // para cada color k
        for(int i=0; i<N; i++) {
            for(int j=i+1; j<N; j++) {  // para cada par de nodos
                if(M[i][j] == 1 and dameParticion(i) != dameParticion(j)) {     // si estan unidos por un eje
                    matbeg[cantRestricciones] = nzcnt;
                    rhs[cantRestricciones]    = 1;
                    sense[cantRestricciones]  = 'L';

                    matind[nzcnt] = xijIndice(i,k);
                    matval[nzcnt] = 1;
                    nzcnt++;
                    matind[nzcnt] = xijIndice(j,k);
                    matval[nzcnt] = 1;
                    nzcnt++;
                    cantRestricciones++;
                    cuenta++;
                }
            }
        }
    }

    // P + i*P + k
    // k son la cantidad de colores
    // P + i*P me salteo al casillero de la matriz
    if(numeroDeModelo == 0){

        // iii) 2*P restricciones mas
        // r = numero actual de restriccion
        for(int k=0; k<P; k++) {  // para cada color
            matbeg[cantRestricciones] = nzcnt;
            rhs[cantRestricciones] = 0;
            sense[cantRestricciones] = 'L';
            matind[nzcnt] = k;
            matval[nzcnt] = -1 * P;
            nzcnt++;
            for(int i=0; i<N; i++) {
                matind[nzcnt] = xijIndice(i,k);
                matval[nzcnt] = 1;
                nzcnt++;
            }
            cantRestricciones++;
        }

        for(int k=0; k<P; k++) {
            matbeg[cantRestricciones] = nzcnt;
            rhs[cantRestricciones] = 0;
            sense[cantRestricciones] = 'G';
            matind[nzcnt] = k;
            matval[nzcnt] = -1;
            nzcnt++;
            for(int i=0; i<N; i++) {
                matind[nzcnt] = xijIndice(i,k);
                matval[nzcnt] = 1;
                nzcnt++;
            }
            cantRestricciones++;
        }

    }
    else{
        for(int color = 0; color < P; color++) { 
            for(int i = 0; i < N; i++) {
                matbeg[cantRestricciones] = nzcnt;
                rhs[cantRestricciones] = 0;
                sense[cantRestricciones] = 'L';
                /// -wj + xij
                matind[nzcnt] = color;
                matval[nzcnt] = -1;
                nzcnt++;
                matind[nzcnt] = xijIndice(i, color);
                matval[nzcnt] = 1;
                nzcnt++; ///xij
                cantRestricciones++;
            }
        }

    }

    //Actualizo rcnt.
    rcnt = cantRestricciones;


    // ===================================================================================================
    
    // Agregamos las restricciones al lp.
    status = CPXaddrows(env, lp, ccnt, rcnt, nzcnt, rhs, sense, matbeg, matind, matval, NULL, NULL);

    if (status) {
        cerr << "Problema agregando restricciones." << endl;
        exit(1);
    }
            
    delete[] rhs;
    delete[] matbeg;
    delete[] matind;
    delete[] matval;


    // ============================================================================================== //
    // ================================== Optimizamos el problema. ================================== //
    // Seteo de algunos parametros.

    // Para desactivar la salida poner CPX_OFF.
    status = CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);
        
    if (status) {
        cerr << "Problema seteando SCRIND" << endl;
        exit(1);
    }
        
    // Setea el tiempo limite de ejecucion.
    status = CPXsetdblparam(env, CPX_PARAM_TILIM, TIEMPO_LIMITE);  // setear limite de tiempo en 3600 !!!!!!!
    
    if (status) {
        cerr << "Problema seteando el tiempo limite" << endl;
        exit(1);
    }
 
    // Escribimos el problema a un archivo .lp.
    status = CPXwriteprob(env, lp, "test.lp", NULL);

    if (status) {
        cerr << "Problema escribiendo modelo" << endl;
        exit(1);
    }
        
    // Tomamos el tiempo de resolucion utilizando CPXgettime.
    double inittime, endtime;
    status = CPXgettime(env, &inittime);

    // Seteamos algunos parametros para resolver con branch and bound
    CPXsetintparam(env, CPX_PARAM_MIPSEARCH, CPX_MIPSEARCH_TRADITIONAL);

    // Para facilitar la comparación evitamos paralelismo:
    CPXsetintparam(env, CPX_PARAM_THREADS, 1);

    //Para que no se adicionen planos de corte:
    CPXsetintparam(env,CPX_PARAM_EACHCUTLIM, 0);
    CPXsetintparam(env, CPX_PARAM_FRACCUTS, -1);
    CPXsetintparam(env, CPX_PARAM_LANDPCUTS, -1);

    // Para que no haga preprocesamientos
    CPXsetintparam(env, CPX_PARAM_PRESLVND, -1);
    CPXsetintparam(env, CPX_PARAM_REPEATPRESOLVE, 0);
    CPXsetintparam(env, CPX_PARAM_RELAXPREIND, 0);
    CPXsetintparam(env, CPX_PARAM_REDUCE, 0);





    // =========================================================================================================
    // resuelvo con cut and branch (con los cortes definidos por nosotros) o con branch and bound (y sin cortes)
    // =========================================================================================================
    if(algoritmo == "cb") {
        
        // while (algo) ... resolver el lp, chequear si la restr inducida por la clique actual es violada. Seguir
        //cout << "antes" << endl;
        
        status = CPXlpopt(env, lp);
        
        //cout << "despues" << endl;
        double objval;
        status = CPXgetobjval(env, lp, &objval);
        // Aca, deberia agregar los cortes requeridos, en funcion de "cliques" y "objval"

        mostrameValores(env, lp);

        double *sol = new double[cantVariables];
        CPXgetx(env, lp, sol, 0, cantVariables - 1);
/*
        for(int v=0; v<N; v++){
            estaColoreado = false;
            for(int j=0; j<P; j++){
                if (sol[P + P*v + j] > 0) {
                    cout << v+1 << " " << j+1 << " " << sol[xijIndice(v,j)] << endl;
                    estaColoreado = true;
                }
            }
            if(not estaColoreado) {
                cout << v+1 << " " << 0 << endl;
            }
        }
*/

        // CPXwriteprob (env, lp, "antesDeClique.lp", "LP");
        for(int color=0; color<P; color++) {
            vector < vector<int> > agregados;
            for(int i=0; i<CANT_RESTR_CLIQUES; i++) {
                vector<int> clique = dameClique(sol, color);
                sort(clique.begin(), clique.end());
                bool incluido = find(agregados.begin(), agregados.end(), clique) != agregados.end();

                if (not incluido and (not clique.empty())) {
                    agregados.push_back(clique);
                    agregarRestriccionClique(env, lp, clique, color);
                    cout << "AGREGO RESTRICCION de color #"<< color << ": ";
                    for(int j=0; j<clique.size(); j++) {
                        cout << clique[j] << " ";
                    }
                    cout << endl;
                }
            }
        }
        delete [] sol;
        

        ///Cuando salimos, pasamos a binaria y corremos un branch and bound
        char *ctype = new char[cantVariables];
        for (int i = 0; i < cantVariables; i++) {
            ctype[i] = 'B';
        }

        cout << "Antes cambiar tipo" << endl;
        status = CPXcopyctype (env, lp, ctype);
        cout << "Despues cambiar tipo" << endl;
        delete[] ctype;

        cout << "ANTES" << endl;
        // CPXwriteprob (env, lp, "antesDeMip.lp", "LP");
        status = CPXmipopt(env,lp);
        cout << "DESPUES" << endl;

    }
    else if (algoritmo == "bb") {
        status = CPXmipopt(env,lp);
    }
    else {
        return 0;
    }
    
    status = CPXgettime(env, &endtime);

    if (status) {
        cerr << "Problema optimizando CPLEX" << endl;
        exit(1);
    }

    // Chequeamos el estado de la solucion.
    int solstat;
    char statstring[510];
    CPXCHARptr p;
    solstat = CPXgetstat(env, lp);
    p = CPXgetstatstring(env, solstat, statstring);
    string statstr(statstring);
    cout << endl << "Resultado de la optimizacion: " << statstring << endl;
    
    if(solstat!=CPXMIP_OPTIMAL && solstat!=CPXMIP_OPTIMAL_TOL && solstat!=CPXMIP_NODE_LIM_FEAS && solstat!=CPXMIP_TIME_LIM_FEAS){
        cout << "No hay solucion" << endl;
    }
    else{
        double objval;
        status = CPXgetobjval(env, lp, &objval);
            
        if (status) {
            cerr << "Problema obteniendo valor de mejor solucion." << endl;
            exit(1);
        }

        cout << "Datos de la resolucion: " << "\t" << objval << "\t" << (endtime - inittime) << endl; 

        
        //std::string outputfile = "dieta.sol";
        //ofstream solfile(outputfile.c_str());

        // Tomamos los valores de todas las variables. Estan numeradas de 0 a n-1.
        double *sol = new double[cantVariables];
        status = CPXgetx(env, lp, sol, 0, cantVariables - 1);

        if (status) {
            cerr << "Problema obteniendo la solucion del LP." << endl;
            exit(1);
        }

            
        // Solo escribimos las variables distintas de cero (tolerancia, 1E-05).
        //solfile << "Status de la solucion: " << statstr << endl;
        for(int j=0; j<P; j++) {
            if(sol[j] > TOL) {
                //cout << "W_" << j << " = " << sol[j] << endl;
            }
        }
        for(int i=0; i<N; i++) {
            for(int j=0; j<P; j++) {
                if(sol[P + P*i + j] > TOL) {
                    //cout << "X_" << i << "_" << j << " = " << sol[P+P*i+j] << endl;
                }
            }
        }
        
        //solfile.close();

        // ==================== Devuelvo el grafo resultante coloreado, para graficar! ====================== //
        ofstream streamEjes, streamLabels;
        //ofstream streamParticiones;
        // Tomamos los valores de la solucion y los escribimos a un archivo.
        streamEjes.open(ejes);
        for(int v1=0; v1<N; v1++) {
            for(int v2=v1+1; v2<N; v2++) {
                if (M[v1][v2] == 1) { 
                    streamEjes << v1+1 << " " << v2+1 << endl;
                }
            }
        } streamEjes.close();
        cout << ejes << endl;
        
        streamLabels.open(labels);
        bool estaColoreado;
        for(int v=0; v<N; v++){
            estaColoreado = false;
            for(int j=0; j<P; j++){
                if (sol[P + P*v + j] == 1) {
                    streamLabels << v+1 << " " << j+1 << endl;
                    estaColoreado = true;
                }
            }
            if(not estaColoreado) {
                streamLabels << v+1 << " " << 0 << endl;
            }
        }
        streamLabels.close();

        delete [] sol;
    }

    return 0;
}
