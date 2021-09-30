#!/usr/bin/env python
# -*- coding: utf-8 -*-

print(""" 
    > Description:  Micro-service (REST API) that exposes the state of the smoke sensor, i.e. its analog values taken
    > Usage:        in browser http://Raspberrypi_IP:80/ VS in terminal #curl -X GET  http://Raspberrypi_IP:80/ 

    > Version:      2.0.1
    > Maintainer:   Nanamou
    > Last update:  06/08/2021
    > Status:       Test environnement

    > Pre-requisites:   GNU/Linux 
                        sudo apt-get update -y
                        sudo apt-get install python3.6 -y
                        sudo apt install python3-pip
                        sudo pip install Flask -y
                        sudo pip install Flask-RestPlus -y
                        pinout
                
    > Test command:     curl -X GET http://IP_raspberry:1234/

    """)


# Bibliothèques
import os
import time
import threading
import RPi.GPIO as GPIO
from flask import Flask, render_template, jsonify

# Variables globales
WAIT_TIME = 0.5 # Attendre 1/2 seconde
PIN_RASPBERRY= 40 # Attention au choix des ports; se référer au site https://fr.pinout.xyz/
CLIENT_API="0.0.0.0"
PORT_API=1234

states = {'value': 0} # La clé n'existe pas encore ->  c'est un ajout
app = Flask(__name__)


# Configuration de la broche du Raspberry Pi
GPIO.setwarnings(False) # Suppression des warnings
GPIO.setmode(GPIO.BOARD)  # Numéro du GPIO selon la numérotation physique. Par exemple 40 pour GPIO21
GPIO.setup(PIN_RASPBERRY, GPIO.IN, pull_up_down = GPIO.PUD_DOWN) # Activer/placer le contrôle du GPIO 40 en entree 


def process_WatchDetector():
    while True:
        state = GPIO.input(PIN_RASPBERRY) # Lit l'état actuel du GPIO, vrai ou 1 si fumée, faux ou 0 sinon
        states["value"] = state # La clé existe -> c'est une modification

        print(state)
        time.sleep(WAIT_TIME) 
        
        # vrai ou 1 si y'a pas de fumée, faux ou 0 sinon
        if (not state):
           print("[+] Smoke is detected: ")



@app.route('/', methods=['GET'])
def home():
    return jsonify(states) # La dernière valeur du dictionnaire est celle qui sera toujours renvoyée car cest la meme clef que nous modifions 

def process_AppFlask():
    app.run(debug=True, use_reloader=False, port=PORT_API, host=CLIENT_API) # Configuration du serveur Flask



if __name__ == '__main__':
    try:
        # Processus 1
        print("[+] Watch sensor detector is runnning...")
        th1=threading.Thread(target=process_WatchDetector).start()

        # Processus 2
        print("[+] Flask server is running...")
        th2=threading.Thread(target=process_AppFlask).start() 

    except KeyboardInterrupt:
        print("\n [!] Keyboard interrupt ...")

    except Exception as e:
        print("\n [!] Unexpected error: " + str(e))
