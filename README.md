# Proyecto Programación Avanzada

WEB SERVER 

## Características:
- Basado en HTTP 1.0.
- Implementa los métodos HEAD, GET, POST y algunos encabezados.
- Genera Content-Type y Content-Length correcto para archivos html, css, js, gif, jpg, png, .ico, .txt (como mínimo).
- Por default corre en modo multihilos.
- Responde con los mensajes correctos cuando el recurso pedido se encuentra o no.
- En un módulo dinámico activado por switch de línea de comandos implementa CGI mediante GET y POST para ejecutar archivos .php (o bien el lenguaje de tu elección).
- Es un demonio.
- Existen un razonable números de commits (+15?) en el historial del proyecto que evidencían el proceso de desarrollo.

## Entrega:
- Se hará una demo el haciendo apuntar cualquier navegador a la dirección donde estará instalado tu servidor (en AWS de preferencia).
- Evaluación según la siguiente tabla:
    - 40 - Entrega solo .html.
    - 10 - Entrega páginas completas (con imágenes).
    -  5 - usa syslog para loggear todo request/error.
    - 15 - Multithread/multiproceso, sin memory leaks/zombies.
    - 20 - Ejecuta  CGI mediante GET/POST.
    - 10 - recibe archivos usando input de type file (y se pueden consultar con un directorio virtual).
    - 10 - No es multithreaded ni multiproceso, usa select para atender eventos asíncronos.
