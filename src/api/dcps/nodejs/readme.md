# Vortex DCPS API for NodeJS

## Developer getting started

Details on development processes and tools are found on [Confluence](https://confluence.prismtech.com/display/TOOLRM/Development+tools).

### Linux Requisites

You will need the following software installed:

* NodeJS 8.11.1 (or later) - Stick to a Node 8 version, though.
* npm 5.6.0 (or later)
* Python 2.7 (v3.x.x is not supported)
* make
* C/C++ compiler toolchain like GCC 

### Windows Requisites

You will need the following software installed:

* NodeJS 8.11.1 (or later) - Stick to a Node 8 version, though.
* npm 5.6.0 (or later)
* Python 2.7 (v3.x.x is not supported)
* Visual C++ build tools (VS 2015 or VS 2017)

### First time actions

Once you have checked out the project, change to this directory, and execute:

```
npm install
```

This will install all required node packages

### Before running tests

Ensure that you have configured your OSPL environment (i.e. `OSPL_HOME`, `OSPL_URI` and `LD_LIBRARY_PATH` have been set).

### Common commands

Make sure your JavaScript conforms to coding standards:

```
npm run lint
```

You can fix automatically fixable problems found by `lint` by:

```
npm run lint-fix
```

Run unit tests:

```
npm test
```

Note that unit tests include code coverage information. Use it to improve your tests.

Generate API documentation (using JSDOC):

```
npm run docs
```

Finally, you can run unit tests, and create results output suitable for OSPL DBTs:

```
npm run dbt
```


