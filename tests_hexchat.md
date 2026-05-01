# 🧪 TESTS HEXCHAT - ft_irc Validation Complète

**Client de référence:** HexChat
**Serveur:** ./ircserv 6667 password

---

## 📋 PRÉPARATION

### Lancer le serveur
```bash
cd irc/
make re
./ircserv 6667 password
```

### Configurer HexChat
1. Ouvrir HexChat
2. Network List → Add
3. Serveur: `localhost/6667`
4. Password: `password`
5. Nick: `alice`
6. Décocher "Use SSL"

---

## ✅ TEST 1 - Authentification (PASS/NICK/USER)

### Scénario
Connexion basique au serveur avec authentification.

### Étapes
1. Connecter avec HexChat
2. Observer les messages de bienvenue

### Résultats attendus
- ✅ Connexion réussie
- ✅ RPL_WELCOME (001) : "Welcome to the ft_irc Network alice"
- ✅ RPL_YOURHOST (002) : Informations serveur
- ✅ RPL_CREATED (003) : Date création
- ✅ RPL_MYINFO (004) : Modes disponibles
- ✅ MOTD affiché

### Commandes testées
- PASS
- NICK
- USER

### Erreurs à vérifier
- Mauvais mot de passe → ERR_PASSWDMISMATCH
- Nickname déjà utilisé → ERR_NICKNAMEINUSE

---

## ✅ TEST 2 - JOIN + Communication de Base

### Scénario
Rejoindre un canal et envoyer des messages.

### Étapes HexChat
```
/join #test
/msg #test Bonjour tout le monde !
/me teste une action
```

### Résultats attendus
- ✅ Canal #test créé
- ✅ alice devient opérateur (@alice)
- ✅ RPL_TOPIC ou RPL_NOTOPIC
- ✅ RPL_NAMREPLY avec @alice
- ✅ RPL_ENDOFNAMES
- ✅ Messages visibles dans le canal
- ✅ Action (/me) fonctionne

### Commandes testées
- JOIN
- PRIVMSG

---

## ✅ TEST 3 - Rejoindre avec 2 clients

### Scénario
Tester multi-utilisateurs dans un canal.

### Étapes
**Client 1 (alice):**
```
/join #general
/msg #general Hello from alice
```

**Client 2 (bob) - Nouvelle instance HexChat:**
```
/server localhost/6667
Password: password
Nick: bob
/join #general
/msg #general Hello from bob
```

### Résultats attendus
- ✅ Bob voit alice dans la liste des membres
- ✅ Alice reçoit le message de bob
- ✅ Bob reçoit le message d'alice
- ✅ Les deux voient @alice (opérateur)
- ✅ Broadcast JOIN fonctionnel

### Commandes testées
- JOIN (multi-utilisateurs)
- PRIVMSG (broadcast)

---

## ✅ TEST 4 - PRIVMSG Direct (User to User)

### Scénario
Message privé entre utilisateurs.

### Étapes HexChat
```
/query bob
Hello bob, message privé !
```

### Résultats attendus
- ✅ Onglet privé ouvert avec bob
- ✅ Message reçu par bob
- ✅ Bob peut répondre
- ✅ Pas de broadcast au canal

### Commandes testées
- PRIVMSG (user to user)

---

## ✅ TEST 5 - PART (Quitter canal)

### Scénario
Quitter un canal avec raison.

### Étapes HexChat
```
/join #test
/part #test Goodbye everyone!
```

### Résultats attendus
- ✅ alice quitte #test
- ✅ Message de départ visible par les autres
- ✅ Raison affichée ("Goodbye everyone!")
- ✅ alice n'est plus dans la liste des membres

### Commandes testées
- PART

---

## ✅ TEST 6 - MODE +i (Invite-only)

### Scénario
Tester le mode invite-only avec INVITE.

### Étapes
**Client 1 (alice - opérateur):**
```
/join #private
/mode #private +i
/invite bob #private
```

**Client 2 (bob):**
```
/join #private
```

**Client 3 (charlie - non invité):**
```
/join #private
```

### Résultats attendus
- ✅ Mode +i activé sur #private
- ✅ Bob reçoit l'invitation
- ✅ Bob peut rejoindre #private (invité)
- ✅ Charlie reçoit ERR_INVITEONLYCHAN (473) "Cannot join channel (+i)"
- ✅ Charlie NE PEUT PAS rejoindre

### Commandes testées
- MODE +i
- INVITE
- Vérification invite-only dans JOIN

---

## ✅ TEST 7 - MODE +t (Topic restricted)

### Scénario
Seuls les opérateurs peuvent changer le topic.

### Étapes
**alice (opérateur):**
```
/join #rules
/mode #rules +t
/topic #rules Welcome to #rules - Be nice!
```

**bob (non-opérateur):**
```
/join #rules
/topic #rules I try to change
```

### Résultats attendus
- ✅ Mode +t activé
- ✅ alice change le topic → Succès
- ✅ Broadcast du nouveau topic
- ✅ bob reçoit ERR_CHANOPRIVSNEEDED (482)
- ✅ Topic reste inchangé

### Commandes testées
- MODE +t
- TOPIC (avec/sans privilèges)

---

## ✅ TEST 8 - MODE +k (Channel key/password)

### Scénario
Canal avec mot de passe.

### Étapes
**alice (opérateur):**
```
/join #secret
/mode #secret +k mypassword
```

**bob (sans mot de passe):**
```
/join #secret
```

**bob (avec mot de passe):**
```
/join #secret mypassword
```

### Résultats attendus
- ✅ Mode +k activé avec clé "mypassword"
- ✅ bob sans clé → ERR_BADCHANNELKEY (475)
- ✅ bob avec clé → JOIN réussi

### Tester removal de clé:
```
/mode #secret -k
```
- ✅ Clé supprimée
- ✅ bob peut rejoindre sans mot de passe

### Commandes testées
- MODE +k / -k

---

## ✅ TEST 9 - MODE +o (Operator privilege)

### Scénario
Donner/retirer privilèges opérateur.

### Étapes
**alice (opérateur):**
```
/join #team
(bob rejoint)
/mode #team +o bob
```

**bob (nouveau opérateur):**
```
/mode #team +i
```

**alice:**
```
/mode #team -o bob
```

**bob (plus opérateur):**
```
/mode #team -i
```

### Résultats attendus
- ✅ bob reçoit @ après +o
- ✅ bob peut utiliser MODE (est opérateur)
- ✅ bob perd @ après -o
- ✅ bob reçoit ERR_CHANOPRIVSNEEDED après -o

### Commandes testées
- MODE +o / -o

---

## ✅ TEST 10 - MODE +l (User limit)

### Scénario
Limite de membres sur un canal.

### Étapes
**alice (opérateur):**
```
/join #small
/mode #small +l 2
```

**bob:**
```
/join #small
```

**charlie:**
```
/join #small
```

### Résultats attendus
- ✅ Limite fixée à 2 membres
- ✅ alice + bob rejoignent (2/2)
- ✅ charlie reçoit ERR_CHANNELISFULL (471)
- ✅ charlie NE PEUT PAS rejoindre

### Tester removal limite:
```
/mode #small -l
/join #small (en tant que charlie)
```
- ✅ Limite supprimée
- ✅ charlie peut rejoindre

### Commandes testées
- MODE +l / -l

---

## ✅ TEST 11 - KICK

### Scénario
Expulser un utilisateur d'un canal.

### Étapes
**alice (opérateur):**
```
/join #general
(bob rejoint)
/kick #general bob Spam not allowed
```

**bob (non-opérateur) essaie:**
```
/join #general
(charlie rejoint)
/kick #general charlie Test
```

### Résultats attendus
- ✅ alice peut kick bob
- ✅ bob voit "You were kicked by alice (Spam not allowed)"
- ✅ bob quitte le canal
- ✅ bob peut rejoindre après kick
- ✅ bob reçoit ERR_CHANOPRIVSNEEDED (pas opérateur)
- ✅ charlie reste dans le canal

### Commandes testées
- KICK

---

## ✅ TEST 12 - QUIT

### Scénario
Déconnexion propre du serveur.

### Étapes HexChat
```
/join #test
/quit Goodbye everyone!
```

### Résultats attendus
- ✅ Message QUIT broadcast à tous les canaux
- ✅ Raison affichée ("Goodbye everyone!")
- ✅ Déconnexion propre
- ✅ Pas de crash serveur

### Commandes testées
- QUIT

---

## ✅ TEST 13 - Multi-canaux

### Scénario
Rejoindre plusieurs canaux simultanément.

### Étapes HexChat
```
/join #chan1,#chan2,#chan3
/msg #chan1 Message pour chan1
/msg #chan2 Message pour chan2
/part #chan1,#chan3
```

### Résultats attendus
- ✅ 3 canaux rejoints en une commande
- ✅ Messages envoyés aux bons canaux
- ✅ PART de 2 canaux en une commande
- ✅ #chan2 reste actif

### Commandes testées
- JOIN (multi-canaux)
- PART (multi-canaux)

---

## ✅ TEST 14 - TOPIC sans mode +t

### Scénario
Tout le monde peut changer le topic (mode +t désactivé).

### Étapes
**alice (opérateur):**
```
/join #free
/topic #free Initial topic
```

**bob (non-opérateur):**
```
/join #free
/topic #free Bob changed the topic
```

### Résultats attendus
- ✅ alice change le topic → OK
- ✅ bob change le topic → OK (pas de +t)
- ✅ Topic mis à jour pour tous
- ✅ Pas d'erreur pour bob

### Commandes testées
- TOPIC (sans restriction)

---

## ✅ TEST 15 - Affichage MODE actuel

### Scénario
Voir les modes actifs sur un canal.

### Étapes
```
/join #modes
/mode #modes +itl 10
/mode #modes
```

### Résultats attendus
- ✅ RPL_CHANNELMODEIS (324) affiché
- ✅ Affiche "+itl" (ou similaire)
- ✅ Pas de crash

### Commandes testées
- MODE (affichage)

---

## 🔥 TEST 16 - Stress Test

### Scénario
Plusieurs clients, plusieurs canaux, actions simultanées.

### Étapes
**3 clients HexChat (alice, bob, charlie):**

**Tous:**
```
/join #general
/join #random
/join #test
```

**Alternance rapide:**
```
/msg #general Spam 1
/msg #random Spam 2
/msg #test Spam 3
(répéter 20 fois)
```

### Résultats attendus
- ✅ Aucun message perdu
- ✅ Pas de crash serveur
- ✅ Ordre des messages cohérent
- ✅ Tous les broadcasts reçus

---

## 🧪 TEST 17 - Cas limites

### Scénario
Tester les edge cases.

### Étapes
```
# Paramètres vides
/topic #test
/kick #test
/invite
/mode

# Canaux inexistants
/msg #doesnotexist Hello
/topic #fake New topic
/part #nope

# Utilisateurs inexistants
/msg fakeuserabc Hello
/invite fakeuserabc #test

# Modes invalides
/mode #test +xyz
/mode #test +o
/mode #test +l abc
```

### Résultats attendus
- ✅ ERR_NEEDMOREPARAMS pour paramètres manquants
- ✅ ERR_NOSUCHCHANNEL pour canaux inexistants
- ✅ ERR_NOSUCHNICK pour users inexistants
- ✅ ERR_UNKNOWNMODE pour modes invalides
- ✅ Pas de crash

---

## 📊 CHECKLIST FINALE

### Commandes de base
- [ ] PASS - Authentification
- [ ] NICK - Changement pseudo
- [ ] USER - Enregistrement
- [ ] JOIN - Rejoindre canal
- [ ] PART - Quitter canal
- [ ] PRIVMSG - Messages canaux
- [ ] PRIVMSG - Messages users
- [ ] QUIT - Déconnexion

### Opérateurs
- [ ] KICK - Expulsion
- [ ] INVITE - Invitation
- [ ] TOPIC - Topic sans +t
- [ ] TOPIC - Topic avec +t

### Modes
- [ ] MODE +i - Invite-only
- [ ] MODE -i - Remove invite-only
- [ ] MODE +t - Topic restricted
- [ ] MODE -t - Remove topic restriction
- [ ] MODE +k - Channel key
- [ ] MODE -k - Remove key
- [ ] MODE +o - Give operator
- [ ] MODE -o - Remove operator
- [ ] MODE +l - User limit
- [ ] MODE -l - Remove limit

### Edge cases
- [ ] Canaux inexistants
- [ ] Users inexistants
- [ ] Paramètres manquants
- [ ] Modes invalides
- [ ] Multi-canaux JOIN/PART

### Robustesse
- [ ] 3+ clients simultanés
- [ ] Messages rapides (spam)
- [ ] Déconnexions brutales
- [ ] Reconnexions

---

## 📝 RAPPORT DE TEST (Template)

```
# TESTS HEXCHAT - Résultats

Date: _____________
Testeur: _____________

## Résumé
- Tests réussis: ___/17
- Tests échoués: ___/17
- Bugs trouvés: ___

## Détails

### TEST 1 - Authentification
Status: [ ] PASS [ ] FAIL
Notes: _______________

### TEST 2 - JOIN + Communication
Status: [ ] PASS [ ] FAIL
Notes: _______________

[etc...]

## Bugs trouvés
1. _______________
2. _______________

## Conclusion
_______________
```

---

**17 TESTS COMPLETS POUR VALIDATION MANDATORY** 🚀
