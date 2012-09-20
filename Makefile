all:
	$(MAKE) -C src
	cp src/poller .

clean:
	$(MAKE) -C src clean
