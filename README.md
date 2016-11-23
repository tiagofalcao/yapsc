Yet Another Parallel SystemC Library
=====


License
-------
 - This module is distributed under the GNU Affero General Public License v3.0.
   See the [License](LICENSE.TXT) file for details.

Build
------------
This module require a SystemC installation with pkg-config support.

1. Build
```bash
make
```

2. Build and Test
```bash
make check
```

Installation
------------

```bash
make install PREFIX=/usr/local/
```

Use
---

```C
// Configure the environment
YAPSC_INIT(ARGC, ARGV);

// Execute a code block if this is the domain of MODULE_NAME
YAPSC_MODULE(MODULE_NAME){}

// Register a payload extension
YAPSC_PAYLOAD_EXTENSION(EXT, ENCODE, DECODE);

// Register a Target socket with a name
YAPSC_TARGET_REG(TARGET_MODULE, TARGET_NAME, TARGET);

// Register a Initiator socket and the name of the target module and socket
YAPSC_INITIATOR_REG(INIT, TARGET_MODULE, TARGET_NAME);

// Start the simulation
YAPSC_START();

// Finalize the library
YAPSC_FINALIZE();
```


More
----

You can find language overview, models, and documentation at
http://www.archc.org

Thanks for the interest. We hope you enjoy using ArchC!

The ArchC Team
Computer Systems Laboratory (LSC)
IC-UNICAMP
http://www.lsc.ic.unicamp.br
