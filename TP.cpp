#include <ilcplex/ilocplex.h>
#include <ilcplex/cplex.h>
ILOSTLBEGIN
#include <string>
#include <vector>

#define TOL 1E-05

/* INPUT: recibe 3 parametros: 
 * i)  archivo .in del cual leer, en el formato especificado.
 * ii) "random" o "notrandom", dependiendo de si quiero que las P particiones sean
 *     random o quiero que se asigne 1 particion distinta a cada nodo
 * iii)proporcion de particiones / cantNodos (en caso de haber elegido random) --> 0 <= num <= 1
 * iv) "bb" o "cb", para elegir el algoritmo con el cual resolver
*/

/* Defino las constantes del problema */
int N; // CANTIDAD DE VERTICES
int E; // CANTIDAD DE ARISTAS
int P; // CANTIDAD DE PARTICIONES
float porcentajeParticiones; // porcentaje de particiones que paso por parametro
string algoritmo;

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
  int vp;
  if(randomness == "random") {
    S.resize(P);  // S guarda que vertices pertenecen a cada particion
    for(int v=0; v<N; v++) {  // asigno particiones al azar. Pueden haber particiones no asignadas
      vp = rand() % P;
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
      vp = v;
      S[vp].push_back(v);
      //cout << v << " " << vp << endl;
    }
  } else {
    cout << "ERROR: parametros mal ingresados!" << endl;
  }
  //cout << "Cantidad final de particiones: " << P << endl;

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

// ================================================================================

int main(int argc, char **argv) {


  freopen(argv[1], "r", stdin);
  ofstream streamEjes, streamLabels, streamParticiones;
  // freopen("output.out", "w", stdout);

  char ejes[100];
  char labels[100];
  char test[100];
  string randomness = argv[2];
  if(randomness == "notrandom") { 
    sprintf(ejes, "ejes.out");
    sprintf(labels, "labels.out");
    sprintf(test, "%s%s", argv[1], argv[2]);
    read(randomness);   // cada elemento de la particion conformado por un unico nodo
    algoritmo = argv[3];
  } else if (randomness == "random") {
    sprintf(ejes, "ejes.out");
    sprintf(labels, "labels.out");
    sprintf(test, "%s%s", argv[1], argv[3]);
    porcentajeParticiones = atof(argv[3]);
    read(randomness);
    algoritmo = argv[4];
  } else {
    cout << "Paramtros mal introducidos" << endl;
    return 0;
  }
  // Le paso por parametro el algoritmo a implementar: bb = branch and bound, cb = cut and branch
  if(algoritmo != "bb" && algoritmo != "cb") {
    cout << "Error introduciendo parametro de algoritmo a ser aplicado " << endl;
    return 0;
  }

  // ==============================================================================================

  // Genero el problema de cplex.
  int status;
  CPXENVptr env; // Puntero al entorno.
  CPXLPptr lp; // Puntero al LP
   
  // Creo el entorno.
  env = CPXopenCPLEX(&status);

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
  int n = P + N*P;
  double *ub, *lb, *objfun; // Cota superior, cota inferior, coeficiente de la funcion objetivo.
  char *xctype, **colnames; // tipo de la variable , string con el nombre de la variable.
  ub = new double[n]; 
  lb = new double[n];
  objfun = new double[n];
  xctype = new char[n];
  colnames = new char*[n];

  for (int i = 0; i < n; i++) {
    ub[i] = 1.0; // seteo upper y lower bounds de cada variable
    lb[i] = 0.0;
    if(i < P) {  // agrego el costo en la funcion objetivo de cada variables
      objfun[i] = 1;  // busco minimizar Sum(W_j) para j=0..P (la cantidad de colores utilizados).
    } else {
      objfun[i] = 0;  // los X[i][j] no contribuyen a la funcion objetivo
    }
    xctype[i] = 'I';  // 'C' es continua, 'B' binaria, 'I' Entera.
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
    status = CPXnewcols(env, lp, n, objfun, lb, ub, NULL, colnames);
  } else if (algoritmo == "bb") {
    // si quiero hacer MIP, directamente, con brancha and bound, agrego xctype
    status = CPXnewcols(env, lp, n, objfun, lb, ub, xctype, colnames);
  } else {
    cout << "Error: parametro de algoritmo bb/cb mal introducido" << endl;
    return 0;
  }
  
  if (status) {
    cerr << "Problema agregando las variables CPXnewcols" << endl;
    exit(1);
  }
  
  // Libero las estructuras.
  for (int i = 0; i < n; i++) {
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
  int rcnt = P + (E/2)*P + 2*P;  // cantidad de restricciones !!!!! 
                  // (E/2 porque en la entrada se supone que en la entrada me pasan 2 veces cada eje)
  int nzcnt = 0;  // al ppio es cero (para cada valor q agrego, lo voy a incrementar en 1)

  char sense[rcnt]; // Sentido de la desigualdad. 'G' es mayor o igual y 'E' para igualdad, 'L' menor o igual
  double *rhs = new double[rcnt]; // Termino independiente de las restricciones.
  int *matbeg = new int[rcnt];    //Posicion en la que comienza cada restriccion en matind y matval.
  int *matind = new int[rcnt*n];       // Array con los indices de las variables con coeficientes != 0 en la desigualdad.
  double *matval = new double[rcnt*n]; // Array que en la posicion i tiene coeficiente ( != 0) de la variable matind[i] en la restriccion.

  // CPLEX va a leer hasta la cantidad nzcnt que le pasemos.

  // i) P restricciones - exactamente un color a cada vertice (una restriccion por cada particion)
  for(int p=0; p<P; p++) {
    matbeg[p] = nzcnt;
    rhs[p]    = 1  ;
    sense[p]  = 'E';
    for(int i=0; i<N; i++) {  // para cada conjunto dentro de la particion, 'e' es el indice de un elemento en el cjto
      for(int e=0; e<S[p].size() && S[p][e] == i; e++) {  // si el nodo i esta en el conjunto S[p] de la particion,
        for(int j=0; j<P; j++) {
          matind[nzcnt] = P + i*P + j;
          matval[nzcnt] = 1;
          nzcnt++;
        }
      }
    }
  }

  // ii) E*P restricciones mas (para cada eje, para cada color)
  int r = P;  // r = numero de restriccion
  int cuenta = 0;
  for(int k=0; k<P; k++) {  // para cada color k
    for(int i=0; i<N; i++) {
      for(int j=i+1; j<N; j++) {  // para cada par de nodos
        if(M[i][j] == 1) {     // si estan unidos por un eje
          matbeg[r] = nzcnt;
          rhs[r]    = 1;
          sense[r]  = 'L';

          matind[nzcnt] = P + i*P + k; matval[nzcnt] = 1;
          nzcnt++;
          matind[nzcnt] = P + j*P + k; matval[nzcnt] = 1;
          nzcnt++;
          r++;
          cuenta++;
        }
      }
    }
  }


  // iii) 2*P restricciones mas
  // r = numero actual de restriccion
  for(int k=0; k<P; k++) {  // para cada color
    matbeg[r] = nzcnt;
    rhs[r] = 0;
    sense[r] = 'L';
    matind[nzcnt] = k; matval[nzcnt] = -1 * P; nzcnt++;
    for(int i=0; i<N; i++) {
      matind[nzcnt] = P + i*P + k; matval[nzcnt] = 1;
      nzcnt++;

    }
    r++;
  }
  for(int k=0; k<P; k++) {
    matbeg[r] = nzcnt;
    rhs[r] = 0;
    sense[r] = 'G';
    matind[nzcnt] = k; matval[nzcnt] = -1; nzcnt++;
    for(int i=0; i<N; i++) {
      matind[nzcnt] = P + i*P + k; matval[nzcnt] = 1;
      nzcnt++;
    }
    r++;
  }



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
  status = CPXsetdblparam(env, CPX_PARAM_TILIM, 50);  // setear en 3600 !!!!!!!
  
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


  // =========================================================================================================
  // resuelvo con cut and branch (con los cortes definidos por nosotros) o con branch and bound (y sin cortes)
  // =========================================================================================================
  if(algoritmo == "cb") {
    
    vector< vector<int> > cliques;  // cuando miro este vector tengo que chequear que cada entrada no sea vacia:
          // puede pasar que el grafo analizado tengo menos que "cantCliquesBuscadas" cliques !
    int cantCliquesBuscadas = 40;   // me devuelve 41 cliques
    //cliques = find_Cliques(...);

    cout << "La cantidad de cliques que tengo es: " << cliques.size() << endl;
    
    // while (algo) ... resolver el lp, chequear si la restr inducida por la clique actual es violada. Seguir
      status = CPXlpopt(env, lp);
      double objval;
      status = CPXgetobjval(env, lp, &objval);
      // Aca, deberia agregar los cortes requeridos, en funcion de "cliques" y "objval"

    // y, al salir del while, hacer status = CPXmipopt(env,lp) --> donde a env,lp le agregue los cortes
    // status = CPXnewcols(env, lp, n, objfun, lb, ub, xctype, colnames);
    // status = CPXchgprobtype(env, lp, CPXPROB_FIXEDMILP);
    // status = CPXmipopt(env,lp);

  } else if (algoritmo == "bb") {
    status = CPXmipopt(env,lp);
  } else {
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
  
  if(solstat!=CPX_STAT_OPTIMAL){
     //exit(1);
  }  

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
  double *sol = new double[n];
  status = CPXgetx(env, lp, sol, 0, n - 1);

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
  
  delete [] sol;
  //solfile.close();

  // ==================== Devuelvo el grafo resultante coloreado, para graficar! ====================== //
  // Tomamos los valores de la solucion y los escribimos a un archivo.
  streamEjes.open (ejes);
  for(int v1=0; v1<N; v1++) {
    for(int v2=v1+1; v2<N; v2++) {
      if (M[v1][v2] == 1) { 
        streamEjes << v1+1 << " " << v2+1 << endl;
      }
    }
  } streamEjes.close();
  cout << ejes << endl;
  
  streamLabels.open (labels);
  int estaColoreado;
  for(int v=0; v<N; v++) {
      estaColoreado = 0;
      for(int j=0; j<P; j++) {
        if (sol[P + P*v + j] == 1) {
          streamLabels << v+1 << " " << j+1 << endl;
          estaColoreado = 1;
        }
      }
      if(estaColoreado==0) { streamLabels << v+1 << " " << 0 << endl; }
    } streamLabels.close();

  return 0;
}
