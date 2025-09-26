# Robot MR-25
# Auteur : Mace Robotics
# Date dernière modification: 23/09/2025
# Firmware du robot MR-25 (carte Raspberry Pi Pico)
# Version : 0.42

#lib
import time, re
import robot
import uasyncio as asyncio
from machine import UART, Pin, I2C
import gc


# CONSTANTES
VERSION_FIRMWARE = 0.41


# variables globales
_data_simple = 0 # si reception d'une commande simple sans paramètre (exemple : #BAT! ou #FV!)
deux_para = 0 # si reception d'une commande avec 2 paramètres
send_data = 0
data_send = 0
cmd_recu = 0

# configuration du l'UART, communication avec le Pi
uart0 = UART(0, baudrate=921600 , tx=Pin(16), rx=Pin(17))

# broche GP25 ou WL_GPI00 en sortie (led de la pico)
led_int = Pin(25, Pin.OUT)  

# led low batterie
led_low_bat = Pin(22, Pin.OUT)  
led_low_bat.value(0)

# bip de start
print("Robot MR-25 Start")
robot.buzzer(500, 65_536/2)
time.sleep(0.5)
robot.buzzerStop()

'''
robot.turnAngle(45)
# TEST
while 1:
    print("orientation = ", robot.orientation())
    print("positionX = ", robot.positionX())
    print("positionY = ", robot.positionY())
    time.sleep(2)
'''



# reception UART
async def task1():
    global send_data, data_send
    writer = asyncio.StreamWriter(uart0, {})
    while True:
        if(send_data == 1):
          send_data = 0
          writer.write(str(data_send)+'\n')
          #print(str(data_send))
          
          await writer.drain()
        await asyncio.sleep(0.2)



async def uart_receiver():
    global _data_simple, deux_para, send_data, data_send
    reader = asyncio.StreamReader(uart0)
    while True:
        try:
          message = await reader.readline()

          data = message.decode('utf-8')#conversion en chaine de caractère
          print("data recu:", data)
          ################################
          # extraction de la commande :
          try:
            cmd_recu = re.search(r'#([A-Za-z]+),', data)
            cmd_recu = str(cmd_recu.group(1)) # conversion
            print("cmd_recu :", cmd_recu)
            _data_simple = 0
          except: # c'est une commande avec un paramètre
            _data_simple = 1 # reception d'une commande simple, sans paramètre
          
          
          if (_data_simple == 1):
            # reception d'une commande simple sans paramètre
            cmd_recu = re.search(r'#([A-Za-z]+)!', data)
            cmd_recu = str(cmd_recu.group(1)) # conversion
          else:
            ################################
            # extraction du paramètre n°1 :
            #parametre1_recu = re.search(r",(\d+)", data)
            #print("Extraction du paramètre n°1")
            try:
              parametre1_recu = re.search(r',\s*(-?\d+)', data)
              print("Paramètre n°1 :", parametre1_recu.group(1))
              deux_para = 1
              parametre1_recu = str(parametre1_recu.group(1))
              print("parametre1_recu:", parametre1_recu)
              print("Commande avec 2 paramètres")
            except:
              # S'il y a 1 paramètre : 
              parametre1_recu = re.search(r',([0-9]+)!', data)
              parametre1_recu = str(parametre1_recu.group(1))
              #print("parametre1_recu:", parametre1_recu)
            ################################
            # extraction du paramètre n°2 :
            if (deux_para == 1):
              try:
                parametre2_recu = re.search(r',([0-9]+)!', data)
                parametre2_recu = int(parametre2_recu.group(1))
                #print("parametre2_recu", parametre2_recu)
              except:
                print("Pas de paramètre n°2")
				
        except:
          print("Erreur reception")
          
        
        #print('CMD RECU:', cmd_recu)

        if(cmd_recu == "FV"):
            send_data = 1
            data_send = VERSION_FIRMWARE

        if(cmd_recu == "BAT"):
            send_data = 1
            data_send = robot.battery()

        if(cmd_recu == "PR"):
            send_data = 1
            data_send = robot.proxRead(parametre1_recu)
            print("send data")

        if(cmd_recu == "MF"):
            robot.forward(int(parametre1_recu))

        if(cmd_recu == "FM"):
            robot.forwardmm(int(parametre1_recu))
            
        if(cmd_recu == "TA"):
            print("parametre1_recu = ", int(parametre1_recu))
            robot.turnAngle(int(parametre1_recu))

        if(cmd_recu == "MB"):
            robot.back(int(parametre1_recu))
            print("send data")

        if(cmd_recu == "STP"):
            send_data = 0
            robot.stop()

        if(cmd_recu == "TR"):
            robot.turnRight(int(parametre1_recu))
            
        if(cmd_recu == "TL"):
            robot.turnLeft(int(parametre1_recu))           

        if(cmd_recu == "MOTR"):
            robot.motorRight(int(parametre1_recu), int(parametre2_recu))
            
        if(cmd_recu == "MOTL"):
            robot.motorLeft(int(parametre1_recu), int(parametre2_recu))
        
        # controle led RGB
        if(cmd_recu == "RGB"):
            print("parametre1_recu :", parametre1_recu)
            chiffres =  [int(c) for c in str(parametre1_recu) if c in ['0', '1']]
            print("chiffres",chiffres)
            robot.ledRgb(chiffres[0],chiffres[1],chiffres[2])
            
        # lecture encodeur left
        if(cmd_recu == "EDL"):
            send_data = 1
            data_send = robot.encoderLeft()

        # lecture encodeur right
        if(cmd_recu == "EDR"):
            send_data = 1
            data_send = robot.encoderRight()
            
        # Encodeur reset
        if(cmd_recu == "ERZ"):
            data_send = robot.encoderReset()
            
        # controle buzzer
        if(cmd_recu == "BUZ"):
            robot.buzzer(int(parametre1_recu), 65536/2)
            
        # stop buzzer
        if(cmd_recu == "BUZS"):
            robot.buzzerStop()
            
        # contole de la led low batt
        if(cmd_recu == "LEDB"):
            if (int(parametre1_recu) == 1):
              led_low_bat.value(1)
            else:
              led_low_bat.value(0)
            
        #print("fin reception")
            
async def main():
    results = await asyncio.gather(uart_receiver(), task1())
    print(results)

asyncio.run(main())


# End of file


