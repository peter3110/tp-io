#!/bin/bash

# FILES="instancesInternet/david.col"

FILES="instancesInternet/david.col
instancesInternet/myciel3.col
instancesNuestras/input0.in"

semilla="456"

for nombreArchivo in $FILES
do
	# for algoritmo in "cb"
	for algoritmo in "cb" "bb"
	do
		for epsilonClique in "0.1"
		# for epsilonClique in "0" "0.1" "0.01" "0.001"
		do
			for epsilonAgujero in "0.1"
			# for epsilonAgujero in "0" "0.1" "0.01" "0.001"
			do
				for numeroModelo in "0" "1"
				do
					for recorridoArbol in "0" "1"
					do
						for variableCorte in "-1" "0" "1"
						do
							for random in "random" "notrandom"
							do
								if [ "${random}" == "random" ]
								then
									for proporcion in "0.2" "0.4" "0.8" "1"
										do
											./TP $nombreArchivo $random $proporcion $algoritmo $epsilonClique $epsilonAgujero $numeroModelo $recorridoArbol $variableCorte $semilla
										done
								else
									./TP $nombreArchivo $random 1 $algoritmo $epsilonClique $epsilonAgujero $numeroModelo $recorridoArbol $variableCorte $semilla
								fi
							done
						done
					done
				done
			done
		done
	done
done
