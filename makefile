SSWATCHER = watcher
SSWATCHERD = daemon
SHARED = shared

bindir = /usr/local/bin

all: build move clean

.PHONY: build sswatcher sswatcherd move clean install

build: sswatcher sswatcherd

sswatcher:
	cd $(SSWATCHER); make

sswatcherd:
	cd $(SSWATCHERD); make

move:
	mv $(SSWATCHER)/sswatcher sswatcher
	mv $(SSWATCHERD)/sswatcherd sswatcherd

clean:
	cd $(SSWATCHER); make clean
	cd $(SSWATCHERD); make clean

install:
	install sswatcher $(bindir)
	install sswatcherd $(bindir)
	cp -i ss-config.json /etc/shadowsocks.json
