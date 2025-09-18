# projet ROV RINKIN
# version : 0.1
# date modification : 12/09/25

from machine import Pin, ADC
import time

MIN_DUTY_TURBINE=3000

# Définir le pin PWM 
pwm_pin = machine.Pin(28)

# configuration pin 27 en entrée analogique
# lecture de la tensio batterie
batt_voltage = ADC(Pin(27))

print("Valeur adc = ", batt_voltage.read_u16())
time.sleep(1)



# Créer un objet PWM 
pwm = machine.PWM(pwm_pin)

# Paramétrer la fréquence de 50 Hz pour ESC
pwm.freq(50)

# Fonction pour envoyer un signal PWM avec un rapport cyclique (duty) 
def set_speed(duty_cycle):
    # Le duty_cycle doit être entre 0 et 65535
    pwm.duty_u16(duty_cycle)
    
# Fonction pour inverser la direction (si l'ESC supporte cette fonctionnalité)
def reverse_direction():
    # Exemple de signal pour inverser la direction (voir documentation ESC)
    set_speed(0)  # Arrêter le moteur
    time.sleep(3)
    print("Inversion de la direction...")
    # Envoi d'une impulsion courte à faible duty pour inverser (ajustez selon l'ESC)
    set_speed(2300)  # Court signal pour inverser la direction
    time.sleep(1)


# Envoyer un signal à 100% pour initier l'ESC.
print("Initialisation de l'ESC.")
set_speed(65536)
time.sleep(5)  # Laisser l'ESC s'initialiser pendant 2 secondes

# Initialisation de l'ESC


# Démarrer à faible vitesse la turbine
print("Démarrage moteur à faible vitesse...")
set_speed(MIN_DUTY_TURBINE)
time.sleep(5)

set_speed(0)
time.sleep(5)

# Augmenter la vitesse progressivement
print("Augmentation de la vitesse...")
'''for duty in range(3000, 7000, 100):  # Augmentation du duty de 50 à 1023
    set_speed(duty)
    time.sleep(1)  # Attendre 1 seconde avant de changer la vitesse
    print(duty)'''
    
# Inverser la direction
reverse_direction()

for duty in range(2300, 2000, -10):  
    set_speed(duty)
    time.sleep(1)  # Attendre 1 seconde avant de changer la vitesse
    print(duty)

# Arrêter le moteur
print("Arrêt du moteur...")
set_speed(0)  # Mettre le duty cycle à 0 pour arrêter le moteur


# end file