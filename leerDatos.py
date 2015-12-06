import ConfigParser
import sys

config = ConfigParser.ConfigParser()
config.read(sys.argv[1])

# Uno en particular
# nombreSeccion = "Datos del problema"
# nombreDato = "epsilonClique"

# Los lee como texto
# print config.get(nombreSeccion, nombreDato)

# Pasado a numero
# print float(config.get(nombreSeccion, nombreDato))

# Leer todos y meterlos en dictionary (los imprime tmb)
dictionary = {}
for section in config.sections():
    print "[" + section + "]"
    dictionary[section] = {}
    for option in config.options(section):
        dictionary[section][option] = config.get(section, option)
        print option + " = " + dictionary[section][option] 