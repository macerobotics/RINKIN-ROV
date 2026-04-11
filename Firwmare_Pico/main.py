import utime
from machine import Pin, PWM, UART

# --- Initialisation du matériel ---

# Lecture de la tension batterie via ADC sur la broche GP27
batt = machine.ADC(Pin(27, mode=Pin.IN))

# Bouton de démarrage des ESC (actif à l'état bas, résistance pull-up interne)
fireup_pin = machine.Pin(18, Pin.IN, Pin.PULL_UP)

# LED intégrée au Pico (GP25)
led_bultin = machine.Pin(25, machine.Pin.OUT)

# LEDs externes
led1 = machine.Pin(7, machine.Pin.OUT)
led2 = machine.Pin(8, machine.Pin.OUT)

# Communication série UART0 avec le Raspberry Pi (9600 bauds, TX=GP16, RX=GP17)
# Brochage côté Raspi : 3e pin droite = GND | 4 = TX | 5 = RX
raspi = machine.UART(0, baudrate=9600, tx=Pin(16), rx=Pin(17))

# Création de 5 canaux PWM pour les ESC (broches GP0 à GP4, fréquence 50 Hz)
esc = []
for n in range(5):
    esc.append(PWM(Pin(n, mode=Pin.OUT), freq=50))

# Variables globales :
# moters : valeurs de consigne des 5 moteurs
# leds   : état des 2 LEDs
# trig   : déclencheur (non utilisé ici)
# count  : compteur de commandes reçues
moters, leds, trig, count = [0] * 5, [0] * 2, 0, 0

print("\nNico sub projet 11/03/26\n")


# --- Fonctions ---

def fireup_ESC():
    """Séquence d'armement des ESC :
    1. Gaz au minimum pendant 5 secondes
    2. Gaz au milieu pendant 7 secondes
    Les ESC sont ainsi calibrés et armés."""
    for n in range(5):
        esc[n].duty_u16(2700)           # Position gaz minimum (≈1000 µs)
    print("throttle in bottom position")
    for n in range(5, 1, -1):           # Compte à rebours 5..2
        print(n)
        utime.sleep(1)

    print("throttle in middle position")
    for n in range(5):
        esc[n].duty_u16(4950)           # Position gaz milieu (≈1500 µs)
    for n in range(7, 1, -1):           # Compte à rebours 7..2
        print(n)
        utime.sleep(1)

    print("ESC fired up")
    # Clignotement de la LED intégrée x3 pour signaler la fin de l'armement
    for n in range(3):
        led_bultin.value(1)
        utime.sleep(0.1)
        led_bultin.value(0)
        utime.sleep(0.1)


def map(value, istart, istop, ostart, ostop):
    """Ré-échelle une valeur d'une plage d'entrée vers une plage de sortie.
    Équivalent de la fonction map() d'Arduino."""
    return int(ostart + (ostop - ostart) * ((value - istart) / (istop - istart)))


def is_valid_command(text):
    """Valide et décode une commande reçue depuis le Raspberry Pi.
    Format attendu : #XYZ! (commence par '#', se termine par '!')
    Longueur totale : 2 à 6 caractères."""
    try:
        text = text.decode('utf-8').strip()
    except:
        print("rubbish from Raspi", text)   # Données illisibles / corrompues
        return

    temp = len(text)

    if temp == 0:
        print("0 chars from raspi")          # Trame vide
        return
    if temp > 6:
        print("chars > 6")                   # Trame trop longue
        return
    if text[0] != '#':
        print("no #")                        # Pas de marqueur de début
        return
    if text[-1] != '!':
        print("no !")                        # Pas de marqueur de fin
        return

    print('>' + text + '<', end=" \t  ")    # Commande valide, affichage debug

    cmd = text[1:-1]                         # Extraction du contenu (sans '#' et '!')

    if cmd[1] == 'm':                        # Commande moteur (ex: #0m5!)
        get_moter_int(text[1:-1])

    if cmd[1] == 'l':                        # Commande LED (ex: #0l1!)
        wh = int(text[1])                    # Numéro de la LED ciblée
        if cmd[0] == '0':                    # '0' = éteindre la LED
            leds[0] = int(cmd[-1])
            led1.value(leds[0])
        else:                                # Autre = allumer la LED
            leds[1] = int(cmd[-1])
            led2.value(leds[1])
        disp_data()

    if cmd[1] == 'b':                        # Commande batterie : renvoie la tension au Raspi
        batt_volt = str(batt.read_u16())
        print("battery voltage =", batt_volt)
        batt_volt += '\n'
        raspi.write(batt_volt.encode('utf-8'))


def get_moter_int(comm):
    """Extrait le numéro de moteur et la valeur de consigne depuis la commande,
    met à jour le tableau moters[] et applique la vitesse à l'ESC correspondant.
    Format de comm (sans # et !) : '0m5' → moteur 0, consigne 5"""
    global moters
    wh = int(comm[0])          # Numéro du moteur (0 à 4)
    value = int(comm[2:])      # Valeur de consigne (ex: -9 à 9)
    moters[wh] = value
    duty = ESC_speed(value)    # Conversion en cycle utile PWM
    esc[wh].duty_u16(duty)
    disp_data()


def ESC_speed(speed):
    """Convertit une consigne de vitesse [-9 ; +9] en valeur de cycle utile
    pour le signal PWM ESC (plage 3650 à 6350 sur 16 bits, soit ≈1100 à 1900 µs)."""
    temp = map(speed, -9, 9, 3650, 6350)
    return temp


def disp_data():
    """Affiche l'état de tous les moteurs, des LEDs et le compteur de commandes."""
    global count
    count += 1
    for n in range(5):
        print("m%d =%3d" % (n + 1, moters[n]), end='   ')
    print("\tled1 =%3d  led2 =%3d  counter = %4d" % (leds[0], leds[1], count))


# --- Programme principal ---

# Lancement de la séquence d'armement des ESC au démarrage
# (le test sur fireup_pin est commenté → armement systématique)
# if not fireup_pin.value():
fireup_ESC()

raspi.flush()   # Vide le buffer UART avant d'entrer dans la boucle principale

# Boucle principale : écoute les commandes UART toutes les 30 ms
while True:
    if raspi.any() > 5:             # Au moins 6 octets disponibles dans le buffer
        data = raspi.readline()     # Lecture d'une ligne complète
        is_valid_command(data)      # Traitement de la commande
    utime.sleep_ms(30)              # Pause 30 ms pour ne pas surcharger le CPU