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


## Installation

1. Install Shadowsocks Python (version 2.8.2 or above is preferred)
   sswatcher currently does NOT work with Shadowsocks-libev. After you have installed [pip](https://pip.pypa.io/en/stable/installing/), install Shadowsocks:

   ```
   pip install shadowsocks
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

1. Start sswatcher

   ```
   sudo sswatcher start
   ```

2. Add port `20000` with `password1` to Shadowsocks

   ```
   sswatcher add 20000 password1
   ```

3. Add port `30000` with `password2`, and set the data cap to 20 GB.

   ```
   sswatcher add 30000 password2 20000000000
   ```

4. Show statistics for current users

   ```
   sswatcher usage
   ```

   The result is a tab separated table, the same as sswatcher config file:

   ```
   port		password	data_cap	usage
   20000	password1	0			19048204
   30000	password2	20000000000	283957184
   ```

5. Remove port `30000`

   ```
   sswatcher remove 30000
   ```

6. Load sswatcher config file from `~/users.tsv`

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

- By default, sswatcher uses the config file at `/etc/shadowsocks.json` to initialize Shadowsocks. If you already have have that file in place, you will be asked if you would like to overwrite that file during installation.

- The port `8383` is considered by sswatcher as a placeholder port in the Shadowsocks config file, and will be removed when sswatcher starts. If you plan to only use sswatcher, please leave this port in the Shadowsocks config file. However, if you have configured ports in the Shadowsocks config file, please do not use port `8383`. The reason for having a placeholder file is because Shadowsocks does not allow an empty `"port_password"` field.

   To learn more about Shadowsocks config file, please refer to this [link](https://github.com/shadowsocks/shadowsocks/wiki/Configure-Multiple-Users).

- Passwords should not contain any spaces.


## Usage

    sswatcher <command> [arguments]

You can optionally prefix each command with two hyphens ("--") for clarity.

    sswatcher --usage


### Commands

- `status`
  - Shows the current status of ssserver and sswatcherd, the sswatcher daemon

- `usage`
  - Shows current usage data

- `add <port> <password> [datacap]`
  - Adds `port` with `password` to Shadowsocks. Optionally, you can specify `datacap` in bytes for this user.

- `update <port> <password> [datacap]`
  - Behaves the same as `add`. Exists for semantic purposes.

- `update_limit <port> <datacap>`
  - Updates the `datacap` (bytes) for user at `port`. The specified port has to be maintained by sswatcher.

- `update_limit_all <datacap>`
  - Updates the `datacap` for all users.

- `reset_usage <port>`
  - Resets the usage statistics for user at `port` to 0.

- `reset_usage_all`
  - Resets all users' usage statistics to 0.

- `remove <port>`
  - Removes the `port` from Shadowsocks, and delete from sswatcher.

- `load`
  - Loads sswatcher configuration from standard input.

- `load_file <filename>`
  - Loads sswatcher configuration file at `filename`.

- `export_file <file>`
  - Exports sswatcher configuration and statistics to `file`.

- `default_datacap`
  - Prints the default data cap in bytes.

- `set_default_datacap <datacap>`
  - Sets the default datacap to `datacap` (bytes).

- `version`
  - Prints sswatcher version.


The following instructions **requires** superuser privilege:

- `start [user <run_as_user>] `
  - Starts Shadowsocks server `ssserver` and sswatcher daemon `sswatcherd`. If `run_as_user` is specified, `sswatcherd` will drop privileges to run_as_user after it is started.

- `stop`
- `restart`

## TODO

- Convert statistics units to KB, MB, GB, etc.

- Attach note to each port

- Connect to database

## License

This program is release under the MIT License.

