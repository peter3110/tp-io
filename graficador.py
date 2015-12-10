import ConfigParser
import sys
import matplotlib.pyplot as plt
import numpy as np

def dameDict(nombreArchivo):
    config = ConfigParser.ConfigParser()
    config.read(nombreArchivo)
    dictionary = {}
    for section in config.sections():
        dictionary[section] = {}
        for option in config.options(section):
            dictionary[section][option] = config.get(section, option)
    return dictionary

def main():

    # juntarRecorridoYVariable()
    # prepararSegunRecorridoArbol()
    # prepararSegunVariableCorte()

    CBvsBB()
    CBvsBBDavid()

def CBvsBB():
    archivos = ["instancesInternet/david.col", "instancesInternet/myciel3.col", "instancesNuestras/input0.in", "instancesInternet/anna.col", "instancesInternet/queen5_5.col", "instancesInternet/huck.col"]

    for nombreArchivo in archivos:
        listaValores = []
        for numeroModelo in ["0","1"]:
            for algoritmo in ["cb","bb"]:
                total = 0

                for variableCorte in ["0"]:
                    for recorridoArbol in ["1"]:
                        
                        for semilla in ["123", "456", "789"]:
                            arch = "salidas/" + nombreArchivo + "_notrandom_1_" + algoritmo + "_0.1_0.1_" + numeroModelo + "_" + recorridoArbol + "_" + variableCorte + "_" + semilla + ".txt"
                            total += float(dameDict(arch)["Resultados"]["tiempo total"])

                listaValores.append(total / 3.0)

        listaValores = tuple([float(x) for x in listaValores])
        graficarCBvsBB(listaValores, nombreArchivo.split("/")[1].split(".")[0] + "_notrandom_cbvsbb")

def CBvsBBDavid():
    archivos = ["instancesInternet/david.col", "instancesInternet/myciel3.col", "instancesNuestras/input0.in"]

    variableCorte = "0"
    recorridoArbol = "1"
    numeroModelo = "0"

    for nombreArchivo in archivos:
        listaValores = []
        for algoritmo in ["cb","bb"]:
            sublista = []
            
            total = 0
            for semilla in ["123", "456", "789"]:
                arch = "salidas/" + nombreArchivo + "_notrandom_1_" + algoritmo + "_0.1_0.1_" + numeroModelo + "_" + recorridoArbol + "_" + variableCorte + "_" + semilla + ".txt"
                total += float(dameDict(arch)["Resultados"]["tiempo total"])

            sublista.append(total / 3.0)

            for proporcion in ["0.2", "0.4", "0.8", "1"]:
                
                total = 0
                for semilla in ["123", "456", "789"]:
                    arch = "salidas/" + nombreArchivo + "_random_" + proporcion + "_" + algoritmo + "_0.1_0.1_" + numeroModelo + "_" + recorridoArbol + "_" + variableCorte + "_" + semilla + ".txt"
                    total += float(dameDict(arch)["Resultados"]["tiempo total"])

                sublista.append(total / 3.0)

            sublista = tuple([float(x) for x in sublista])
            listaValores.append(sublista)

        graficarCBvsBBDavid(listaValores, nombreArchivo.split("/")[1].split(".")[0] + "_comparandoRandomsYAlgoritmos")


def juntarRecorridoYVariable():
    archivos = ["instancesInternet/david.col", "instancesInternet/myciel3.col", "instancesNuestras/input0.in"]
    semilla = "123"

    for nombreArchivo in archivos:
        for algoritmo in ["cb","bb"]:
            for proporcion in ["0.2", "0.4", "0.8", "1"]:
                for numeroModelo in ["0","1"]:
                    listaValores = []

                    for variableCorte in ["-1","0","1"]:
                        for recorridoArbol in ["0","1"]:
                            total = 0
                            for semilla in ["123", "456", "789"]:
                                arch = "salidas/" + nombreArchivo + "_random_" + proporcion + "_" + algoritmo + "_0.1_0.1_" + numeroModelo + "_" + recorridoArbol + "_" + variableCorte + "_" + semilla + ".txt"
                                total += float(dameDict(arch)["Resultados"]["tiempo total"])

                            listaValores.append(total / 3.0)

                    listaValores = tuple([float(x) for x in listaValores])
                    graficarSegunJuntada(listaValores, nombreArchivo.split("/")[1].split(".")[0] + "_random_" + proporcion + "_" + algoritmo + "_" + numeroModelo + "_segunJuntada")

            for numeroModelo in ["0","1"]:
                listaValores = []

                for variableCorte in ["-1","0","1"]:
                    for recorridoArbol in ["0","1"]:
                        total = 0
                        for semilla in ["123", "456", "789"]:
                            arch = "salidas/" + nombreArchivo + "_notrandom_1_" + algoritmo + "_0.1_0.1_" + numeroModelo + "_" + recorridoArbol + "_" + variableCorte + "_" + semilla + ".txt"
                            total += float(dameDict(arch)["Resultados"]["tiempo total"])

                        listaValores.append(total / 3.0)

                listaValores = tuple([float(x) for x in listaValores])
                graficarSegunJuntada(listaValores, nombreArchivo.split("/")[1].split(".")[0] + "_notrandom_1_" + algoritmo + "_" + numeroModelo + "_segunJuntada")

def prepararSegunRecorridoArbol():
    archivos = ["instancesInternet/david.col", "instancesInternet/myciel3.col", "instancesNuestras/input0.in"]
    semilla = "123"

    for nombreArchivo in archivos:
        for algoritmo in ["cb","bb"]:
            for numeroModelo in ["0","1"]:
                for variableCorte in ["-1","0","1"]:
                    listaValores = []

                    notRandomFile = "salidas/" + nombreArchivo + "_notrandom_1_" + algoritmo + "_0.1_0.1_" + numeroModelo + "_" + "0" + "_" + variableCorte + "_" + semilla + ".txt"
                    notRandomFile2 = "salidas/" + nombreArchivo + "_notrandom_1_" + algoritmo + "_0.1_0.1_" + numeroModelo + "_" + "1" + "_" + variableCorte + "_" + semilla + ".txt"

                    listaValores.append(( dameDict(notRandomFile)["Resultados"]["tiempo total"], dameDict(notRandomFile2)["Resultados"]["tiempo total"] ))

                    for proporcion in ["0.2", "0.4", "0.8", "1"]:
                        randomFile = "salidas/" + nombreArchivo + "_random_" + proporcion + "_" + algoritmo + "_0.1_0.1_" + numeroModelo + "_" + "0" + "_" + variableCorte + "_" + semilla + ".txt"
                        randomFile2 = "salidas/" + nombreArchivo + "_random_" + proporcion + "_" + algoritmo + "_0.1_0.1_" + numeroModelo + "_" + "1" + "_" + variableCorte + "_" + semilla + ".txt"

                        listaValores.append((dameDict(randomFile)["Resultados"]["tiempo total"], dameDict(randomFile2)["Resultados"]["tiempo total"]))

                    graficarSegunRecorridoArbol(listaValores, nombreArchivo.split("/")[1].split(".")[0] + "_" + algoritmo + "_" + numeroModelo + "_" + variableCorte + "_segunRecorridoArbol")

def prepararSegunVariableCorte():
    archivos = ["instancesInternet/david.col", "instancesInternet/myciel3.col", "instancesNuestras/input0.in"]
    semilla = "123"

    for nombreArchivo in archivos:
        for algoritmo in ["cb","bb"]:
            for numeroModelo in ["0","1"]:
                for recorridoArbol in ["0","1"]:
                    listaValores = []

                    notRandomFile = "salidas/" + nombreArchivo + "_notrandom_1_" + algoritmo + "_0.1_0.1_" + numeroModelo + "_" + recorridoArbol + "_" + "0" + "_" + semilla + ".txt"
                    notRandomFile2 = "salidas/" + nombreArchivo + "_notrandom_1_" + algoritmo + "_0.1_0.1_" + numeroModelo + "_" + recorridoArbol + "_" + "1" + "_" + semilla + ".txt"
                    notRandomFile3 = "salidas/" + nombreArchivo + "_notrandom_1_" + algoritmo + "_0.1_0.1_" + numeroModelo + "_" + recorridoArbol + "_" + "-1" + "_" + semilla + ".txt"

                    listaValores.append(( dameDict(notRandomFile)["Resultados"]["tiempo total"], dameDict(notRandomFile2)["Resultados"]["tiempo total"],  dameDict(notRandomFile3)["Resultados"]["tiempo total"]))

                    for proporcion in ["0.2", "0.4", "0.8", "1"]:
                        randomFile = "salidas/" + nombreArchivo + "_random_" + proporcion + "_" + algoritmo + "_0.1_0.1_" + numeroModelo + "_" + recorridoArbol + "_" + "0" + "_" + semilla + ".txt"
                        randomFile2 = "salidas/" + nombreArchivo + "_random_" + proporcion + "_" + algoritmo + "_0.1_0.1_" + numeroModelo + "_" + recorridoArbol + "_" + "1" + "_" + semilla + ".txt"
                        randomFile3 = "salidas/" + nombreArchivo + "_random_" + proporcion + "_" + algoritmo + "_0.1_0.1_" + numeroModelo + "_" + recorridoArbol + "_" + "-1" + "_" + semilla + ".txt"

                        listaValores.append((dameDict(randomFile)["Resultados"]["tiempo total"], dameDict(randomFile2)["Resultados"]["tiempo total"], dameDict(randomFile3)["Resultados"]["tiempo total"]))

                    graficarSegunVariableCorte(listaValores, nombreArchivo.split("/")[1].split(".")[0] + "_" + algoritmo + "_" + numeroModelo + "_" + recorridoArbol + "_segunVariableCorte")

def graficarSegunRecorridoArbol(listaValores, nombreGrafico):
    n_groups = 2

    fig = plt.subplots()
    colores = ['b', 'g', 'r', 'black', 'y']
    nombres = ['not random', 'random 0.2', 'random 0.4', 'random 0.8', 'random 1']

    index = np.arange(n_groups)
    bar_width = 0.15
    opacity = 0.5

    for i, val in enumerate(listaValores):
        plt.bar(index + bar_width * i, val, bar_width, color=colores[i], alpha=opacity, label=nombres[i])

    plt.xlabel('Recorrido Arbol')
    plt.ylabel('Tiempo')
    plt.title('Tiempo segun recorrido arbol')
    plt.xticks(index + bar_width * 3, ("Depth-first search", "Best-bound search"))
    plt.legend()

    plt.savefig('informe/graficos/' + nombreGrafico + '.png')
    plt.close()

def graficarSegunVariableCorte(listaValores, nombreGrafico):
    n_groups = 3

    fig = plt.subplots()
    colores = ['b', 'g', 'r', 'black', 'y']
    nombres = ['not random', 'random 0.2', 'random 0.4', 'random 0.8', 'random 1']

    index = np.arange(n_groups)
    bar_width = 0.15
    opacity = 0.5

    for i, val in enumerate(listaValores):
        plt.bar(index + bar_width * i, val, bar_width, color=colores[i], alpha=opacity, label=nombres[i])

    plt.xlabel('Recorrido Arbol')
    plt.ylabel('Tiempo')
    plt.title('Tiempo segun recorrido arbol')
    plt.xticks(index + bar_width * 3, ("Minimum infeasibility", "CPLEX decide", "Maximum infeasibility"))
    plt.legend()

    plt.savefig('informe/graficos/' + nombreGrafico + '.png')
    plt.close()

def graficarSegunJuntada(listaValores, nombreGrafico):
    n_groups = 1

    fig = plt.figure()
    ax = plt.subplot(111)

    colores = ['b', 'g', 'r', 'black', 'y', "violet"]
    nombres = ["Minimum infeasibility, Depth-first search", "CPLEX decide, Depth-first search", "Maximum infeasibility, Depth-first search", "Minimum infeasibility, Best-bound search", "CPLEX decide, Best-bound search", "Maximum infeasibility, Best-bound search"]

    index = np.arange(n_groups)
    bar_width = 0.1
    opacity = 0.5

    for i, val in enumerate(listaValores):
        plt.bar(index + bar_width * i, val, bar_width, color=colores[i], alpha=opacity, label=nombres[i])

    # plt.xlabel('Variable de corte, Recorrido arbol')
    plt.ylabel('Tiempo (en segundos)')
    plt.tick_params(axis='x', which='both',bottom='off',top='off',labelbottom='off')

    # Shrink current axis by 20%
    box = ax.get_position()
    ax.set_position([box.x0, box.y0, box.width * 0.8, box.height])

    # Put a legend to the right of the current axis
    lgd = ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))

    # plt.legend()

    plt.savefig('informe/graficos/' + nombreGrafico + '.png', format='png', bbox_extra_artists=(lgd,), bbox_inches='tight')
    # fig.savefig('image_output.png', dpi=300, format='png', bbox_extra_artists=(lgd,), bbox_inches='tight')
    plt.close()

def graficarCBvsBB(listaValores, nombreGrafico):
    n_groups = 1

    fig = plt.figure()

    colores = ['b', 'g', 'r', 'black', 'y']
    nombres = ["Pedro CB", "Pedro BB", "Santiago CB", "Santiago BB"]

    index = np.arange(n_groups)
    bar_width = 0.1
    opacity = 0.5

    for i, val in enumerate(listaValores):
        plt.bar(index + bar_width * i, val, bar_width, color=colores[i], alpha=opacity, label=nombres[i])

    # plt.xlabel('modelo, algoritmo')
    plt.ylabel('Tiempo (en segundos)')
    plt.tick_params(axis='x', which='both',bottom='off',top='off',labelbottom='off')

    plt.legend(loc='upper left')

    plt.savefig('informe/graficos/' + nombreGrafico + '.png', format='png', dpi=300, bbox_inches='tight')
    plt.close()

def graficarCBvsBBDavid(listaValores, nombreGrafico):
    n_groups = 5

    fig = plt.subplots()
    colores = ['b', 'r']
    nombres = ['CB', 'BB']

    index = np.arange(n_groups)
    bar_width = 0.4
    opacity = 0.5

    for i, val in enumerate(listaValores):
        plt.bar(index + bar_width * i, val, bar_width, color=colores[i], alpha=opacity, label=nombres[i])

    plt.xlabel('Manera de particionar el grafo')
    plt.ylabel('Tiempo (en segundos)')
    plt.xticks(index + bar_width, ("notrandom", "random0.2", "random0.4", "random0.8", "random1"))
    plt.legend()

    plt.savefig('informe/graficos/' + nombreGrafico + '.png')
    plt.close()


if __name__ == '__main__':
    main()