import ConfigParser
import sys
import matplotlib.pyplot as plt
import numpy as np

# config = ConfigParser.ConfigParser()
# config.read(sys.argv[1])

# Uno en particular
# nombreSeccion = "Datos del problema"
# nombreDato = "epsilonClique"

# Los lee como texto
# print config.get(nombreSeccion, nombreDato)

# Pasado a numero
# print float(config.get(nombreSeccion, nombreDato))

# Leer todos y meterlos en dictionary (los imprime tmb)
# dictionary = {}
# for section in config.sections():
#     print "[" + section + "]"
#     dictionary[section] = {}
#     for option in config.options(section):
#         dictionary[section][option] = config.get(section, option)
#         print option + " = " + dictionary[section][option] 



n_groups = 2

valoresNotRandom = (1,2)
valoresRandom02 = (2,3)
valoresRandom04 = (4,5)
valoresRandom08 = (1,6)
valoresRandom10 = (4,2)

fig = plt.subplots()
colores = ['b', 'g', 'r', 'black', 'y']
nombres = ['not random', 'random 0.2', 'random 0.4', 'random 0.8', 'random 1']

index = np.arange(n_groups)
bar_width = 0.15
opacity = 0.5

# plt.bar(index, valoresNotRandom, bar_width,alpha=opacity,color='b',label='not random')
# plt.bar(index + bar_width, valoresRandom02, bar_width,alpha=opacity,color='r',label='valoresRandom02')

for i, val in enumerate([valoresNotRandom, valoresRandom02, valoresRandom04, valoresRandom08, valoresRandom10]):
    plt.bar(index + bar_width * i, val, bar_width, color=colores[i], alpha=opacity, label=nombres[i])

plt.xlabel('Recorrido Arbol')
plt.ylabel('Tiempo')
plt.title('Tiempo segun recorrido arbol')
plt.xticks(index + bar_width * 3, ("Depth-first search", "Best-bound search"))
plt.legend()

# plt.tight_layout()
plt.show()