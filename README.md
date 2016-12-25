---
title: Project Telecom
subtitle: Implementatie van het IGMP protocol, versie 3.
author: Evert Heylen
---

# Structuur

  - De `elements` map bevat alle code voor de elementen, en de inhoud hiervan kan dus in de `elements/local` map worden gezet.
  
  - De `scripts` map bevat alle nodige Click scripts. Standaard hebben we de Multicast Server geconfigureerd op `224.4.4.4`.
  
  
# Compilatie

Om te compileren gebruik ik de volgende CXX flags om wél met C++11 te kunnen werken:

```bash
CXXFLAGS="-Wno-narrowing -std=c++11" 
	./configure --disable-linuxmodule --enable-local --enable-etherswitch
```

Of, voor de clang liefhebbers:

```bash
CXX=clang++ CXXFLAGS="-Wno-narrowing -Wno-string-plus-int -std=c++11 -g"
	./configure --disable-linuxmodule --enable-local --enable-etherswitch
```

# Handlers

Elke router of client bevat een `igmp` en een `mc_table` element. Het `igmp` element heeft de volgende **write** handlers (enkel voor clients):

  - `change_mode <mode> <group> <source>*`: Zal de modus aanpassen, en ook een Report sturen naar de router
    - `<mode>` is `INCLUDE` of `EXCLUDE`
    - `<group>` is een multicast group adres (class D)
    - `<source>` is een source adres
  - `silent_change_mode <mode> <group> <source>*`: Doet het hetzelfde als hierboven, maar zonder hiervan de router op de hoogte te stellen. Nuttig om timers te testen.
  - `allow <source>+`: Laat de gegeven source addressen door, en communiceert dat met de server.
  - `block <source>+`: Houdt de gegeven source addressen tegen, en communiceert dat met de server.

Een `mc_table` element heeft één **read** handler:

  - `table`: print de multicast tabel af.

  
# Tests

Ik heb mijn eigen test framework geschreven in Python, op basis van de modules `unittest` en `pexpect`. De tests testen voornamelijk op het aankomen van de paketten (door de output van click na te gaan). Correcte checksums etc moeten manueel nagegaan worden in Wireshark. Om de tests ietwat snel te doen lopen zijn er bepaalde veronderstellingen gemaakt over lagere default waardes van timers (zie constants.hh). De tests werken niet tenzij de typfout in `ipnetwork.click` op lijn 70 wordt aangepast (een spatie te weinig). Er zijn ook extra scripts geschreven om te testen, die allemaal in de folder `scripts` staan.

Om de tests te draaien (na het installeren van de nodige libraries):

```bash
cd tests
python3 tests.py
```

Het uitvoeren van de tests zelf is niet getest op de referentie-PC's.
