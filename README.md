
# Project Telecom

Implementatie van het IGMP protocol, versie 3.

**Authors**: Stijn Vissers, Evert Heylen

## Structuur

  - De `src` map bevat alle code voor de elementen, en de inhoud hiervan kan dus in de `elements/local` map worden gezet.
  
  - De `scripts` map bevat alle nodige Click scripts. Standaard hebben we de Multicast Server geconfigureerd op `224.4.4.4`.
  

## Compilatie

Om te compileren gebruiken wij de volgende CXX flags om wél met C++11 te kunnen werken:

	CXXFLAGS="-Wno-narrowing -std=c++11" ./configure --disable-linuxmodule --enable-local --enable-etherswitch

Of, voor de clang liefhebbers:

	CXX=clang++ CXXFLAGS="-Wno-narrowing -Wno-string-plus-int -std=c++11 -g" ./configure --disable-linuxmodule --enable-local --enable-etherswitch


## Handlers

Elke router of client bevat een `igmp` en een `mc_table` element. Het `igmp` element heeft de volgende write handlers:

  - `join_group <IPAddress>`: zal de vermelde group joinen. Enkel van toepassing op clients.
  - `leave_group <IPAddress>`: zal de vermelde group leaven. Enkel van toepassing op clients.
  - `join_group_silent <IPAddress>`: zal de vermelde group joinen, zonder een `CHANGE_TO_EXCLUDE` report te sturen. Enkel van toepassing op clients, gebruikt om General Queries te testen.
  - `leave_group_silent <IPAddress>`: zal de vermelde group joinen, zonder een `CHANGE_TO_INCLUDE` report te sturen. Enkel van toepassing op clients, gebruikt om General Queries te testen.

Een `mc_table` element heeft één read handler:

  - `table`: print de multicast tabel af.
