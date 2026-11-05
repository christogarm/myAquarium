# Convención de commits

Este documento establece las convenciones para escribir mensajes de
commit en este repositorio. El objetivo es mantener un historial de
cambios claro, consistente y fácil de comprender, especialmente en
proyectos de software embebido y firmware.

Se utilizará **Conventional Commits** como referencia para estructurar
los mensajes, adaptando la redacción al español para facilitar la
colaboración entre desarrolladores hispanohablantes.

## 1. Estructura del mensaje

Los mensajes de commit deben seguir esta estructura:

``` text
<tipo>(<alcance>): <descripción>

[cuerpo opcional]

[pie opcional]
```

-   **Tipo (`type`):** identifica la naturaleza del cambio.
-   **Alcance (`scope`):** indica el módulo, componente o área afectada.
    Es opcional.
-   **Descripción (`description`):** resume brevemente el cambio
    realizado.
-   **Cuerpo (`body`):** explica el contexto, los motivos o los detalles
    técnicos cuando sea necesario.
-   **Pie (`footer`):** permite incluir referencias a incidencias o
    indicar cambios incompatibles.

## 2. Tipos de commits

  -----------------------------------------------------------------------
  Tipo                                Cuándo utilizarlo
  ----------------------------------- -----------------------------------
  `feat`                              Incorporación de una nueva
                                      funcionalidad.

  `fix`                               Corrección de errores o
                                      comportamientos incorrectos.

  `refactor`                          Reestructuración interna del código
                                      sin modificar su comportamiento
                                      externo.

  `test`                              Incorporación, modificación o
                                      corrección de pruebas.

  `docs`                              Cambios relacionados con la
                                      documentación.

  `build`                             Modificaciones en el sistema de
                                      compilación o en las dependencias.

  `ci`                                Cambios en los procesos de
                                      integración o entrega continua.

  `perf`                              Mejoras de rendimiento o
                                      eficiencia.

  `style`                             Cambios de formato o estilo que no
                                      afectan el comportamiento del
                                      código.

  `chore`                             Tareas generales de mantenimiento
                                      que no corresponden a las
                                      categorías anteriores.

  -----------------------------------------------------------------------


Los identificadores de tipo se mantienen en inglés porque forman parte
de la convención; las descripciones y el cuerpo del mensaje se redactan
en español.

### Ejemplos

``` text
feat: agregar monitoreo de temperatura
fix: corregir saturación de la salida del PID
refactor: simplificar la inicialización de sensores
test: agregar pruebas unitarias para el controlador PID
docs: actualizar instrucciones de instalación
build: configurar dependencias de FreeRTOS
ci: agregar flujo de compilación automatizada
perf: optimizar el filtrado de temperatura
style: aplicar formato al código fuente
chore: actualizar configuración del repositorio
```

## 3. Alcance (`scope`)

El alcance identifica el módulo o componente afectado. Se recomienda
utilizar nombres cortos, descriptivos y consistentes con la arquitectura
del proyecto.

  -----------------------------------------------------------------------
  Alcance                             Área
  ----------------------------------- -----------------------------------
  `application`                       Lógica de aplicación y coordinación
                                      de tareas.

  `domain`                            Lógica de negocio y algoritmos
                                      independientes del hardware.

  `services`                          Servicios y funcionalidades
                                      compartidas.

  `infrastructure`                    Implementaciones de hardware y
                                      sistemas externos.

  `hal`                               Capa de abstracción de hardware.

  `sensor`                            Gestión y adquisición de datos de
                                      sensores.

  `pid`                               Controladores PID.

  `logger`                            Registro y almacenamiento de datos.

  `diagnostics`                       Diagnósticos y detección de fallas.

  `config`                            Parámetros y configuración del
                                      sistema.

  `tests`                             Pruebas y su infraestructura.

  -----------------------------------------------------------------------

Estos alcances son ejemplos. Cada proyecto puede definir los suyos de
acuerdo con su arquitectura.

### Ejemplos

``` text
feat(sensor): agregar controlador para DS18B20
fix(pid): evitar la acumulación excesiva del término integral
refactor(domain): desacoplar la lógica de control del hardware
feat(application): agregar tarea de monitoreo de temperatura
test(pid): agregar pruebas unitarias del controlador
```

## 4. Reglas para redactar los mensajes

Para mantener la consistencia, se deben seguir estas reglas:

1.  Redactar las descripciones y el cuerpo del commit en español.
2.  Utilizar verbos en infinitivo para describir la acción: `agregar`,
    `corregir`, `actualizar`, `eliminar`, `optimizar`.
3.  Comenzar la descripción con letra minúscula, salvo nombres propios o
    identificadores.
4.  No colocar un punto al final de la descripción.
5.  Mantener la descripción breve y específica.
6.  Evitar mensajes genéricos que no expliquen el cambio realizado.
7.  Utilizar el cuerpo cuando sea necesario explicar decisiones técnicas
    o motivos del cambio.

### Ejemplos adecuados

``` text
feat(sensor): agregar interfaz genérica de temperatura
fix(pid): corregir acumulación del término integral
refactor(logger): separar el formato de datos del almacenamiento
test(sensor): agregar pruebas de detección de fallas
```

### Ejemplos inadecuados

``` text
actualizaciones
correcciones
cambios
nuevas modificaciones
trabajando en PID
versión final
```

Estos mensajes no describen claramente qué se modificó ni cuál fue el
propósito del cambio.

## 5. Cuerpo del commit

El cuerpo es opcional. Se recomienda utilizarlo cuando el cambio
requiera una explicación adicional, por ejemplo:

-   El problema que motivó el cambio.
-   La solución implementada.
-   Las decisiones técnicas relevantes.
-   Las limitaciones o consideraciones de implementación.

Ejemplo:

``` text
fix(pid): evitar acumulación integral durante la saturación

El término integral continuaba acumulando el error cuando
la salida del controlador alcanzaba sus límites máximo o mínimo.

Se agrega integración condicional para evitar una acumulación
excesiva del término integral durante la saturación.
```

## 6. Cambios incompatibles (`BREAKING CHANGE`)

Cuando un cambio modifica una interfaz, comportamiento o contrato
existente de manera que puede afectar a otros módulos o usuarios, debe
identificarse como un cambio incompatible.

Se puede agregar `!` después del tipo y alcance:

``` text
feat(sensor)!: modificar la interfaz de temperatura
```

También se puede agregar un pie para explicar el cambio:

``` text
feat(sensor)!: modificar la interfaz de temperatura

BREAKING CHANGE: se reemplaza la función de inicialización
anterior por una interfaz basada en funciones callback.
```

La etiqueta `BREAKING CHANGE` se mantiene exactamente así para respetar
la convención de Conventional Commits.

## 7. Buenas prácticas

Cada commit debe representar un cambio lógico y específico.

-   Evitar agrupar modificaciones que no estén relacionadas.
-   No separar artificialmente archivos que forman parte de una misma
    funcionalidad.
-   Revisar los cambios antes de confirmar el commit.
-   Evitar incluir archivos temporales, binarios generados o
    configuraciones locales.
-   Comprobar que el código compile y que pasen las pruebas relevantes.
-   Evitar commits que dejen el proyecto en un estado inconsistente.

**Importante:** un commit puede modificar varios archivos siempre que
todos formen parte de una misma unidad lógica de trabajo.

## 8. Flujo de trabajo recomendado

### 1. Revisar el estado del repositorio

``` bash
git status
```

### 2. Revisar los cambios realizados

``` bash
git diff
```

### 3. Agregar los archivos correspondientes

Agregar archivos específicos ayuda a evitar que se incluyan cambios
accidentales:

``` bash
git add src/pid.c
```

También se pueden agregar varios archivos relacionados:

``` bash
git add src/pid.c tests/test_pid.c
```

### 4. Revisar los cambios preparados

``` bash
git diff --staged
```

### 5. Realizar el commit

``` bash
git commit -m "fix(pid): corregir acumulación del término integral"
```

### 6. Verificar el historial

``` bash
git log --oneline --graph
```

### 7. Crear una etiqueta de versión de firmware (cuando corresponda)

No todos los commits necesitan una etiqueta. Se recomienda crear un
**Git tag** cuando un commit represente una versión identificable del
firmware y hardware, por ejemplo, una versión probada, liberada o entregada.

Se recomienda utilizar versionado semántico (`MAJOR.MINOR.PATCH`):

-   **MAJOR:** cambios incompatibles con versiones anteriores.
-   **MINOR:** incorporación de funcionalidades compatibles.
-   **PATCH:** correcciones compatibles con la versión anterior.

Ejemplos de etiquetas:

``` text
fw/v1.0.0 - hw/v1.0.0
fw/v1.0.1 - hw/v1.0.0
fw/v1.1.0 - hw/v1.0.1
```

Para crear una etiqueta anotada en el commit actual:

``` bash
git tag -a v1.0.0 -m "Versión de firmware 1.0.0"
```

Para consultar las etiquetas:

``` bash
git tag
```

Para consultar los detalles de una etiqueta:

``` bash
git show v1.0.0
```

Para publicar la etiqueta en el repositorio remoto:

``` bash
git push origin v1.0.0
```

Para publicar todas las etiquetas locales:

``` bash
git push origin --tags
```

**Recomendaciones para las versiones de firmware:**

-   Crear etiquetas únicamente cuando el commit corresponda a una
    versión que se desea identificar o conservar.
-   Preferir etiquetas anotadas (`git tag -a`) porque guardan
    información adicional, como el autor, la fecha y el mensaje.
-   Crear la etiqueta después de verificar que la versión compila y
    supera las pruebas definidas para el proyecto.
-   No reutilizar una etiqueta para señalar una versión diferente. Si
    una versión necesita correcciones, crear una nueva versión.
-   Mantener el código fuente, la configuración de compilación y los
    recursos necesarios para poder identificar y reproducir la versión
    etiquetada.

Una etiqueta identifica un commit específico; no crea un commit nuevo ni
modifica el historial. Por ello, la etiqueta debe apuntar al commit
exacto que contiene el firmware que se desea versionar.

## 9. Ejemplo de historial de commits

Un historial organizado podría verse así:

``` text
feat(sensor): agregar interfaz genérica de temperatura
feat(sensor): implementar controlador DS18B20
feat(domain): implementar filtrado de temperatura
feat(pid): agregar algoritmo de control de temperatura
feat(application): agregar tarea de monitoreo
test(pid): agregar pruebas unitarias del controlador
fix(pid): corregir prevención de acumulación integral
docs: documentar arquitectura del sistema
```

Después de validar una versión, el commit correspondiente podría
etiquetarse, por ejemplo, como `v1.0.0`.

## 10. Referencia

Este repositorio adopta como referencia el estándar **Conventional
Commits 1.0.0**:

https://www.conventionalcommits.org/es/v1.0.0/

**Objetivo:** mantener un historial de cambios claro, consistente y
trazable que facilite el mantenimiento, la revisión del código, la
colaboración entre desarrolladores y la identificación de versiones de
firmware.
