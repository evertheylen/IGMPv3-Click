
# Project Telecom

Implementation of the IGMP protocol, version 3.

Om te compileren gebruiken wij de volgende CXX flags om wel met C++11 te kunnen werken:

	CXXFLAGS="-Wno-narrowing -std=c++11" ./configure --disable-linuxmodule --enable-local 
--enable-etherswitch

Of, voor de clang liefhebbers:

	CXX=clang++ CXXFLAGS="-Wno-narrowing -Wno-string-plus-int -std=c++11" ./configure 
--disable-linuxmodule --enable-local --enable-etherswitch

**Authors**: Stijn Vissers, Evert Heylen
