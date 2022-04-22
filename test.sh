#!/usr/bin/bash

IP=127.0.0.1
PORT=8081


curl -X GET http://$IP:$PORT   # Devrait redoner l'index par default (server par default)
sleep 30
curl -X GET http://$IP:$PORT/error_pages   # Devrait redoner la page de directory
sleep 30
curl -X GET http://$IP:$PORT/nimp   # Devrait redoner les pages d'erreur
sleep 30
# curl -X DELETE http://$IP:$PORT/nimp   # Devrait delete le fichier ou dossier
# sleep 30
# curl -X GET http://$IP:$PORT/nimp   # Devrait delete le fichier ou dossier
