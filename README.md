# ProyectoEmbebidos
Proyecto de la materia Sistemas Embebidos
## Tema: Dispensador de comida para gatos
## Justificación del proyecto 
<div align="justify"> 
El objetivo principal del proyecto es desarrollar un sistema automatizado para alimentar mascotas de manera remota y eficiente, utilizando tecnología WiFi para su control y un sensor de peso para detectar cuándo se ha agotado la comida. Este sistema pretende facilitar la vida de los propietarios de mascotas, garantizando una alimentación regular y monitoreada, incluso cuando no están en casa. 
</div>

## Descripción del Proyecto: 
<div align="justify"> 
El dispensador de comida para mascotas es un sistema embebido que utiliza una placa ESP32 por su capacidad de conectividad WiFi. El dispositivo incluye componentes electrónicos como un servomotor SG90 para controlar la compuerta de la comida, un LED y pantalla LCD I2C para indicaciones visuales, un sensor de peso para detectar cuando el alimento se ha agotado y un sensor ultrasónico o de movimiento con un módulo de cámara ESP32CAM que se active cuando detecte la mascota se encuentra en el dispensador, tome una foto y la envíe al usuario. El sistema se programa a través de Visual Studio Code y se controla mediante un bot de Telegram, permitiendo a los usuarios enviar comandos para dispensar comida y recibir notificaciones sobre el estado del dispensador y la foto cuando la mascota se encuentre en el dispensador. Adicional, se deberá enviar el peso del reservorio de comida a una plataforma en la nube, como ubidots, en donde se deberá ver una gráfica con el comportamiento de esta variable. Asi mismo, se utiliza mit app inventor para crear una aplicado que permita el punto de acceso de la esp32 que recibira datos de las veces que se ha alimentado a la mascota. El sistema de alimentación del dispensador debe ser autónomo mediante baterías de 3.7V en serie.
</div>
