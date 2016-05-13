WATCHER_DIR = watcher
DAEMON_DIR = daemon
RES_DIR = resources
SSWATCHER = sswatcher
SSWATCHERD = sswatcherd
SHARED = shared

bindir = /usr/local/bin

all: build move clean

.PHONY: build $(SSWATCHER) $(SSWATCHERD) move clean install uninstall

build: $(SSWATCHER) $(SSWATCHERD)

$(SSWATCHER):
	cd $(WATCHER_DIR); make

$(SSWATCHERD):
	cd $(DAEMON_DIR); make

move:
	mv $(WATCHER_DIR)/$(SSWATCHER) $(SSWATCHER)
	mv $(DAEMON_DIR)/$(SSWATCHERD) $(SSWATCHERD)

clean:
	cd $(WATCHER_DIR); make clean
	cd $(DAEMON_DIR); make clean

install:
	install $(SSWATCHER) $(bindir)
	install $(SSWATCHERD) $(bindir)
	cp -i $(RES_DIR)/ss-config.json /etc/shadowsocks.json

uninstall:
	rm $(bindir)/$(SSWATCHER)
	rm $(bindir)/$(SSWATCHERD)
