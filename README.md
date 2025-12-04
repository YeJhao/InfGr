# Práctica 1 - Path Tracing

---

## Estructura del código

El código está estructurado en varias carpetas, cada una conteniendo el código correspondiente a su módulo:

- **geometry**: Módulo de geometrías. Contiene las formas geométricas utilizadas en nuestro Path Tracing, la definición de la clase Color, así como un fichero auxiliar con funciones de refracción y reflexión (bsdf_utils)
- **imaging**: Módulo encargado de aplicar tone mapping al render final de forma que se pueda ver la imagen en formato PNG.
- **light**: Módulo encargado de la definición de luces puntuales.
- **ray**: Módulo encargado de la definición de los rayos.

A parte de estos módulos, se pueden encontrar dos ficheros sueltos:

- **generate_image_PT**: Fichero encargado de generar el ejecutable. Contiene el _main_ donde se le pide al usuario toda información y se le da un menú en el que puede elegir la escena que quiere renderizar.
- **path_tracing**: Fichero encargado de calcular la Ecuación de Render, teniendo en cuenta tanto luz directa como indirecta, la generación de nuevos rayos, etc.

---

## Instrucciones de compilación

Se supone que tiene compatibilidad de compilación y ejecución tanto en Windows como en Linux, pero sólo se ha podido llevar a cabo la experimentación en el segundo sistema operativo. Para poder compilar y ejecutar este proyecto se deben seguir los siguientes pasos:

1. Creamos una carpeta llamada "_build_":
   `mkdir build`
2. Nos metemos dentro de este nuevo directorio y ejecutamos el siguiente comando:
   `cmake ..`
   Este comando nos genererá los ficheros correspondientes al directorio de trabajo para poder compilar y generar los ejecutables.
3. Para compilar el trabajo se puede hacer de dos formas:
   3.1. Dentro del directorio "_build_" ejecutamos:
   `make`
   3.2. Fuera del directorio ejecutamos:
   `cmake --build build`

De esta forma generaremos un ejecutable `generate_image_PT` en el directorio "_executables_".
Para poder ejecutarlo, escribimos en la terminal:
`./ruta_a_executables/generate_image_PT`
