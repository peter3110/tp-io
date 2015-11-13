
# 
crear_funcion_de_mapeo <- function(numeritos_de_nodos,maximo_color){
  numeritos_sin_repetir = unique(numeritos_de_nodos)
  N = length(numeritos_sin_repetir)
  res <- function(numerito_de_nodo){
    for(i in 1:N){
      if(numeritos_sin_repetir[i] == numerito_de_nodo){
        return(floor((i - 1) * 1.0 / (N - 1) * (maximo_color - 1)) + 1)
      }
    }
  }
  return(res)
}
#Ejemplo
# a = c(4,1,10,6,10,1,4)
# f <- crear_funcion_de_mapeo(a,123)
# f(1) # da 41
# f(4) # da 1
# f(6) # da 123
# f(10) # da 82

#########################################################
library(igraph)

edges   <- read.table("ejes.out")
labels  <- read.table("labels.out")
nodos   <- labels$V1 # indice de cada nodo
colores <- labels$V2 # para cada nodo, que color le asigno
G  = graph.data.frame(edges, FALSE, nodos)

nodos2 = subset(labels, labels$V2 > 0)
G2     = induced.subgraph(G,nodos2$V1)
nodos2
# aplico f para obtener nodos de colores bien diferentes.
f <- crear_funcion_de_mapeo(nodos2$V2,655)
colores = nodos2$V2
for (i in 1:nrow(nodos2)) {
  colores[i] = colors()[f(nodos2$V2[i])]
}

# armo el grafo inducido
plot(G2,vertex.size=5, vertex.label = NA, 
     vertex.color=colores, layout=layout.lgl,
     rescale=TRUE, edge.arrow.size = 0.5)
## FIN

