# sswatcher [![Build Status](https://travis-ci.org/meomoe/sswatcher.svg?branch=master)](https://travis-ci.org/meomoe/sswatcher)

sswatcher - Shadowsocks server watcher

sswatcher is a [Shadowsocks](https://shadowsocks.org/en/index.html) user management utility written in C++. sswatcher enables easy management of multiple users for a Shadowsocks server. It communicates with Shadowsocks through the [Shadowsocks Manager API](https://github.com/shadowsocks/shadowsocks/wiki/Manage-Multiple-Users).



## Features

- Add or remove users with no interruption
- Data usage monitoring
- Import and export configuration files
- The ability to use ports specified in Shadowsocks config file along with ports managed by sswatcher
- Minimal memory usage


## Requirements

- Linux or OS X
  - Note: sswatcher is tested on OS X 10.11, Ubuntu 14.04 LTS, and Debian 8.0. However, no guarantee is provided for its reliability on these platforms, or other Linux or Unix distributions.

- Shadowsocks Python 2.8

- [GCC](https://gcc.gnu.org) 4.7 or above, or [Clang](http://clang.llvm.org) 3.3 or above


## Installation

1. Install Shadowsocks Python (version 2.8.2 or above is preferred)  
   sswatcher currently does **not** work with Shadowsocks-libev. After you have installed [pip](https://pip.pypa.io/en/stable/installing/), install Shadowsocks:

   ```
   pip install shadowsocks
   ```

   If you have previously installed Shadowsocks, make sure that it provides the Manager API:

   ```
   ssserver --help | grep manager
   ```

   You should see the following output:

   ```
      --manager-address ADDR optional server manager UDP address, see wiki
   ```

2. Download sswatcher

   ```
   git clone https://github.com/meomoe/sswatcher.git
   ```

3. Install sswatcher

   ```
   cd sswatcher
   make && sudo make install
   ```


## Examples

The following examples assume that you have the default installation of sswatcher and Shadowsocks Python.

- Start sswatcher

   ```
   sudo sswatcher start
   ```

- Add port `20000` with `password1` to Shadowsocks

   ```
   sswatcher add 20000 password1
   ```

- Add port `30000` with `password2`, and set the data cap to 20 GB.

   ```
   sswatcher add 30000 password2 20
   ```

- Show statistics for current users

   ```
   sswatcher stat
   ```

   Result:

   ```
   port     password    data_cap        usage
   20000    password1   0 (unlimited)   19.05 GB
   30000    password2   20.00 GB        283.96 MB
   ```

- Remove port `30000`

   ```
   sswatcher remove 30000
   ```

- Load sswatcher config file from `~/users.tsv`

   Contents of `users.tsv`:

   ```
   10000	password1	0
   20000	password2	0
   30000	password3	0
   40000	password4	0
   ```

   Use the following command:

   ```
   sswatcher load ~/users.tsv
   ```

   Note: currently, loading a config file to sswatcher erases all previous entries; however, if a port exists both in the old configuration and the loaded config file, and its usage data was not present in the loaded file (as in this example), its usage data will be preserved.


## Details

You should understand the following details if you plan to use sswatcher:

- You can manage the sswatcher config file while ssserver is not running. However, these changes will only take effect the next time you start sswatcher using `sswatcher start`.

- Shadowsocks runs in parallel with sswatcher. sswatcher does **not** read your Shadowsocks config file, nor does Shadowsocks understand the sswatcher config file.

- By default, sswatcher uses the config file at `/etc/shadowsocks.json` to initialize Shadowsocks. If you already have have that file in place, you will be asked if you would like to overwrite that file during installation. If you want to specify another file, please refer to the [sswatcher Usage Reference](https://github.com/meomoe/sswatcher/wiki/sswatcher-Usage-Reference).

- The port `8383` is considered by sswatcher as a placeholder port in the Shadowsocks config file, and will be removed when sswatcher starts. If you plan to only use sswatcher, please leave this port in the Shadowsocks config file. However, if you have configured ports in the Shadowsocks config file, please do not use port `8383`. The reason for having a placeholder file is because Shadowsocks does not allow an empty `"port_password"` field.  
   To learn more about Shadowsocks config file, please refer to this [link](https://github.com/shadowsocks/shadowsocks/wiki/Configure-Multiple-Users).

- Passwords in sswatcher should not contain any spaces.


## Usage

See [sswatcher Usage Reference](https://github.com/meomoe/sswatcher/wiki/sswatcher-Usage-Reference).

## License

This program is release under the MIT License.
